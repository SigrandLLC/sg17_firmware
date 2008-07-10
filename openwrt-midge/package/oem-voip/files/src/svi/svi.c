#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include "drv_tapi_io.h"	/* from TAPI_HL_driver */
#include "vinetic_io.h" 	/* from Vinetic_LL_driver */
#include "ab_ioctl.h"		/* from ATA_Board_driver */

#define ERR_SUCCESS 0
#define ERR_MEMORY_FULL 1
#define ERR_UNKNOWN_OPTION 2
#define ERR_COULD_NOT_OPEN_FILE 3
#define ERR_IOCTL_FAILS 4

#define ERR_MSG_SIZE 256

/*
COMMAND LINE KEYS:
  -h, --help		display this help and exit
  -V, --version		show version and exit
*/

struct _startup_options
{
	unsigned help : 1;
	unsigned version : 1;
} g_so;

unsigned char startup_init( int argc, char ** argv );

unsigned char g_err_no;
unsigned char g_err_tag;
char g_err_msg[ ERR_MSG_SIZE ];


const unsigned char g_programm[ ] = "svi";
const unsigned char g_version[ ] = "0.01-beta";
const unsigned char g_author[ ] = "Vladimir Luchko";
const unsigned char g_email[ ] = "vlad.luch@mail.ru";

#define AB_FW_PRAM_NAME "/lib/firmware/pramfw.bin"
#define AB_FW_DRAM_NAME "/lib/firmware/dramfw.bin"
#define AB_FW_CRAM_FXS_NAME "/lib/firmware/cramfw_fxs.bin"
#define AB_FW_CRAM_FXO_NAME "/lib/firmware/cramfw_fxo.bin"

#define AB_DRIVER_DEV_NODE "/dev/sgatab"

static char * fw_pram = NULL;
static char * fw_dram = NULL;
static char * fw_cram_fxs = NULL;
static char * fw_cram_fxo = NULL;
static unsigned long fw_pram_size = 0;
static unsigned long fw_dram_size = 0;
static unsigned long fw_cram_fxs_size = 0;
static unsigned long fw_cram_fxo_size = 0;
static ab_init_params_t params;
static ab_boards_presence_t bp;

static void Error_message( int argc, char ** argv );
static void show_help( void );
static void show_version( void );
static void show_version( void );
static void run( void );

unsigned char 
startup_init( int argc, char ** argv )
{
	int option_IDX;
	int option_rez;
	char * short_options = "hVs:";
	struct option long_options[ ] = {
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
		};

	/* INIT WITH DEFAULT VALUES */
	g_so.help = 0;
	g_so.version = 0;

	opterr = 0;
	while ((option_rez = getopt_long (
			argc, argv, short_options, 
			long_options, &option_IDX)) != -1) {
		switch( option_rez ){
			case 'h': {
				g_so.help = 1;
				return 0;
			}
			case 'V': {
				g_so.version = 1;
				return 0;
			}
			case '?' :{
				/* unknown option found */
				g_err_no = ERR_UNKNOWN_OPTION;
				g_err_tag = optind;
				return g_err_no;
			}
		}
	}
	return 0;
};

int 
main( int argc, char ** argv )
{
	g_err_no = ERR_SUCCESS;
	g_err_tag = 0;
	strcpy( g_err_msg, "" );
	int err = 0;

	err = startup_init( argc, argv );
	if( err ){
		goto main_exit;
	}

	if( g_so.help ){
		show_help( );
	} else if( g_so.version ){
		show_version( );
	} else {
		/* engine start */
		run();
	}

main_exit:
	Error_message( argc, argv );

	return g_err_no;
};

/////////////////////////////////////////////////////////////////
static int 
params_get( void )
{
	int ab_fd;
	int err = 0;

	memset (&bp, 0, sizeof(bp));
	memset (&params, 0, sizeof(params));

	/* Get board mount info */
	ab_fd = open(AB_DRIVER_DEV_NODE, O_RDWR, 0x644);
	if (ab_fd < 0) {
		g_err_no = ERR_COULD_NOT_OPEN_FILE;
		strcpy(g_err_msg, "opening board device node");
		goto init_params_get__exit;
	}

	err = ioctl(ab_fd, SGAB_GET_BOARDS_PRESENCE, &bp);
	if(err) {
		g_err_no = ERR_IOCTL_FAILS;
		strcpy(g_err_msg, "SGAB_GET_BOARDS_PRESENCE fails");
		goto init_params_get__exit;
	}
	
	params.requested_board_slot = bp.slots[0];

	err = ioctl(ab_fd, SGAB_GET_INIT_PARAMS, &params);
	if(err) {
		g_err_no = ERR_IOCTL_FAILS;
		strcpy(g_err_msg, "SGAB_GET_INIT_PARAMS fails");
		goto init_params_get__exit;
	}

	close (ab_fd);
	return 0;

init_params_get__exit:
	close (ab_fd);
	return -1;
};

static int 
basicdev_inits( void )
{ 
	IFX_int32_t err;
	VINETIC_BasicDeviceInit_t binit;
	int i;
	int dev_N;
	unsigned long region_size = params.region_size;

	memset(&binit, 0, sizeof(binit));
	binit.AccessMode = params.AccessMode;
	binit.nIrqNum = params.nIrqNum;
	binit.nBaseAddress = params.nBaseAddress;

	for (dev_N=1, i=0; i<DEVS_PER_BOARD_MAX; i++) {
		char dev_node[ 50 ];
		int cfg_fd;

		if(params.devices[i] == dev_type_ABSENT){
			binit.nBaseAddress += region_size;
			continue;
		}

		sprintf(dev_node,"/dev/vin%d0", dev_N );

		cfg_fd = open(dev_node, O_RDWR, 0x644);
		if( cfg_fd <= 0 ){
			g_err_no = ERR_COULD_NOT_OPEN_FILE;
			strcpy(g_err_msg, "opening vinetic device node");
			goto basicdev_inits__exit;
		}

		err = ioctl(cfg_fd, FIO_VINETIC_DEV_RESET, 0);
		if(err){
			g_err_no = ERR_IOCTL_FAILS;
			strcpy(g_err_msg, "Device reset fails (ioctl)");
			close (cfg_fd);
			goto basicdev_inits__exit;
		}
		err = ioctl (cfg_fd, FIO_VINETIC_BASICDEV_INIT, &binit);
		if(err){
			g_err_no = ERR_IOCTL_FAILS;
			strcpy(g_err_msg, "BasicDev init fails (ioctl)");
			close (cfg_fd);
			goto basicdev_inits__exit;
		}
		binit.nBaseAddress += region_size;
		dev_N++;
		close (cfg_fd);
	}

	return 0;

basicdev_inits__exit:
	return -1;
};

static void 
fw_masses_free( )
{
	if(fw_pram) {
		free(fw_pram);
		fw_pram = NULL;
	}
	if(fw_dram) {
		free(fw_dram);
		fw_dram = NULL;
	}
	if(fw_cram_fxs) {
		free(fw_cram_fxs);
		fw_cram_fxs = NULL;
	}
	if(fw_cram_fxo) {
		free(fw_cram_fxo);
		fw_cram_fxo = NULL;
	}
};

static int 
fw_masses_init_from_path (char ** const fw_buff, 
		unsigned long * const buff_size, char const * const path )
{
	int fd;

	fd = open(path, O_RDONLY);
	if( fd <= 0 ) {
		g_err_no = ERR_COULD_NOT_OPEN_FILE;
		strcpy(g_err_msg, "Opening firmware file");
		goto fw_masses_init_from_path__exit;
	}
	*buff_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	(*fw_buff) = malloc(*buff_size);
	if( ! (*fw_buff) ){
		g_err_no = ERR_MEMORY_FULL;
		goto fw_masses_init_from_path__open_file;
	}

	if(read(fd, (*fw_buff), *buff_size) != *buff_size){
		g_err_no = ERR_COULD_NOT_OPEN_FILE;
		strcpy(g_err_msg, "Reading firmware file");
		goto fw_masses_init_from_path__alloc_mem;
	}

	close(fd);
	return 0;

fw_masses_init_from_path__alloc_mem:
	free((char*)(*fw_buff));
	(*fw_buff) = NULL;
fw_masses_init_from_path__open_file:
	close(fd);
	*buff_size = 0;
fw_masses_init_from_path__exit:
	return -1;
};

static int 
fw_masses_init( void )
{
	int i;
	int fxs_inited = 0;
	int fxo_inited = 0;
	int err;

	for (i=0; i<DEVS_PER_BOARD_MAX; i++){
		if(fxs_inited && fxo_inited){
			break;
		}
		if(!fxo_inited && params.devices[i] == dev_type_FXO){
			err = fw_masses_init_from_path(
					&fw_cram_fxo, &fw_cram_fxo_size, 
					AB_FW_CRAM_FXO_NAME );
			if( err ){
				goto __free_masses;
			}
			fxo_inited = 1;
		} else if (!fxs_inited && params.devices[i] == dev_type_FXS){
			err = fw_masses_init_from_path(
					&fw_cram_fxs, &fw_cram_fxs_size, 
					AB_FW_CRAM_FXS_NAME );
			if( err ){
				goto __free_masses;
			}
			fxs_inited = 1;
		}
	}

	err = fw_masses_init_from_path(
			&fw_pram, &fw_pram_size, AB_FW_PRAM_NAME );
	if( err ){
		goto __free_masses;
	}

	err = fw_masses_init_from_path(
			&fw_dram, &fw_dram_size, AB_FW_DRAM_NAME );
	if( err ){
		goto __free_masses;
	}

	return 0;

__free_masses:
	fw_masses_free( );
	return -1;
};

static int 
chan_init_fw( int const rtp_fd, dev_type_t dtype )
{
	IFX_TAPI_CH_INIT_t init;
	VINETIC_IO_INIT vinit;
	int err = 0;

	memset(&vinit, 0, sizeof(VINETIC_IO_INIT));

	vinit.pPRAMfw = (IFX_uint8_t *) fw_pram;
	vinit.pram_size = fw_pram_size;
	vinit.pDRAMfw = (IFX_uint8_t *) fw_dram;
	vinit.dram_size = fw_dram_size;
	/* FXS / FXO personal */
	if(dtype == dev_type_FXS) {
		vinit.pCram = (IFX_uint8_t *) fw_cram_fxs;
		vinit.cram_size = fw_cram_fxs_size;
		vinit.pBBDbuf = (IFX_uint8_t *) fw_cram_fxs;
		vinit.bbd_size = fw_cram_fxs_size;
	} else if(dtype == dev_type_FXO){
		vinit.pCram = (IFX_uint8_t *) fw_cram_fxo;
		vinit.cram_size = fw_cram_fxo_size;
		vinit.pBBDbuf = (IFX_uint8_t *) fw_cram_fxo;
		vinit.bbd_size = fw_cram_fxo_size;
	}
	
	/* Set the pointer to the VINETIC dev specific init structure */
	memset(&init, 0, sizeof(IFX_TAPI_CH_INIT_t));
	init.pProc = (IFX_void_t *) &vinit;
	/* Init the VoIP application */
	init.nMode = IFX_TAPI_INIT_MODE_VOICE_CODER;

	/* Initialize channel */
	err = ioctl(rtp_fd, IFX_TAPI_CH_INIT, (IFX_uint32_t) &init);
	if( err ){
		g_err_no = ERR_IOCTL_FAILS;
		strcpy(g_err_msg, "initilizing channel with firmware (ioctl)");
		goto __exit_fail;
	}

	return 0;

__exit_fail:
	return -1;
};

static int 
chan_init_tune( int const rtp_fd, int const chan_idx, int const dev_idx,
		dev_type_t const dtype )
{
	IFX_TAPI_MAP_DATA_t datamap;
	IFX_TAPI_LINE_TYPE_CFG_t lineTypeCfg;
	IFX_TAPI_SIG_DETECTION_t dtmfDetection;
	int err = 0;

	memset(&datamap, 0, sizeof (datamap));
	memset(&lineTypeCfg, 0, sizeof (lineTypeCfg));

	/* Map channels */	
	datamap.nDstCh = chan_idx; 
	datamap.nChType = IFX_TAPI_MAP_TYPE_PHONE;
	err = ioctl(rtp_fd, IFX_TAPI_MAP_DATA_ADD, &datamap);
	if( err ){
		g_err_no = ERR_IOCTL_FAILS;
		strcpy(g_err_msg, "mapping channel to it`s "
				"own data (ioctl)");
		goto ab_chan_init_tune__exit;
	} 

	/* Set channel type */	
	if(dtype == dev_type_FXS) {
		lineTypeCfg.lineType = IFX_TAPI_LINE_TYPE_FXS_NB;
	} else if(dtype == dev_type_FXO) {
		lineTypeCfg.lineType = IFX_TAPI_LINE_TYPE_FXO_NB;
		lineTypeCfg.nDaaCh = dev_idx * CHANS_PER_DEV + chan_idx;
	}

	err = ioctl (rtp_fd, IFX_TAPI_LINE_TYPE_SET, &lineTypeCfg);
	if( err ){
		g_err_no = ERR_IOCTL_FAILS;
		strcpy(g_err_msg, "setting channel type (ioctl)" );
		goto ab_chan_init_tune__exit;
	} 

	if(dtype == dev_type_FXS) {
		/* ENABLE detection of DTMF tones 
		 * from local interface (ALM X) */
		memset(&dtmfDetection, 0, sizeof (dtmfDetection));
		dtmfDetection.sig = IFX_TAPI_SIG_DTMFTX;
		err = ioctl (rtp_fd,IFX_TAPI_SIG_DETECT_ENABLE,&dtmfDetection);
		if( err ){
			g_err_no = ERR_IOCTL_FAILS;
			strcpy(g_err_msg, "trying to enable DTMF ALM signal "
					"detection (ioctl)" );
			goto ab_chan_init_tune__exit;
		}
	} else if(dtype == dev_type_FXO) {
		/* DISABLE detection of DTMF tones 
		 * from local interface (ALM X) */
		memset(&dtmfDetection, 0, sizeof (dtmfDetection));
		dtmfDetection.sig = IFX_TAPI_SIG_DTMFTX;
		err = ioctl (rtp_fd,IFX_TAPI_SIG_DETECT_DISABLE,&dtmfDetection);
		if( err ){
			g_err_no = ERR_IOCTL_FAILS;
			strcpy(g_err_msg, "trying to disable DTMF ALM signal "
					"detection (ioctl)" );
			goto ab_chan_init_tune__exit;
		}
	}
	
	return 0;

ab_chan_init_tune__exit:
	return -1;
};

static int 
all_chans_init( void )
{
	int dev_N;
	int dev_idx;
	int chan_idx;
	int err = 0;

	for (dev_N=1, dev_idx=0; dev_idx<DEVS_PER_BOARD_MAX; dev_idx++){
		if (params.devices [dev_idx] == dev_type_ABSENT){
			continue;
		}
		for (chan_idx=0; chan_idx<CHANS_PER_DEV; chan_idx++){
			int fd_chan;
			char dev_node[ 50 ];

			/* Initialize channel */
			sprintf(dev_node, "/dev/vin%d%d", 
					dev_N, chan_idx+1);

			fd_chan = open(dev_node, O_RDWR, 0x644);
			if (fd_chan<=0){
				g_err_no = ERR_COULD_NOT_OPEN_FILE;
				strcpy(g_err_msg, 
						"opening vinetic channel node");
				goto __exit_fail;
			}
			err = chan_init_fw (fd_chan, params.devices [dev_idx]);
			if(err){
				close (fd_chan);
				goto __exit_fail;
			}
			err = chan_init_tune (fd_chan, chan_idx, dev_N-1,
					params.devices [dev_idx]);
			close (fd_chan);
			if(err){
				goto __exit_fail;
			}
		}
		dev_N++;
	}

	fw_masses_free( );
	return 0;

__exit_fail:
	fw_masses_free( );
	return -1;
};

static void 
run( void )
{
	int err = 0;

	/* get mount info */
	err = params_get();
	if( err ) {
		fprintf(stderr,"\nsvi ERROR : params_get()\n");
		return;
	}

	/* basicdev init for every device */
	err = basicdev_inits();
	if( err ) {
		fprintf(stderr,"\nsvi ERROR : basicdev_inits()\n");
		return;
	}

	/* Load PRAM, DRAM and CRAM */
	err = fw_masses_init();
	if( err ) {
		fprintf(stderr,"\nsvi ERROR : fw_masses_init()\n");
		return;
	}

	/* Channels init with PRAM, DRAM and CRAM and tune media */
	err = all_chans_init();
	if( err ) {
		fprintf(stderr,"\nsvi ERROR : all_chans_init()\n");
		return;
	}
};

static void 
Error_message( int argc, char ** argv )
{
	if( g_err_no == ERR_SUCCESS ){
		return;	
	}
	
	switch( g_err_no ){
		case ERR_MEMORY_FULL :{
			fprintf( stderr, "%s : out of memory\n", g_programm );
			break;	
		}	
		case ERR_UNKNOWN_OPTION :{
			fprintf( stderr, "%s : invalid option\n", g_programm );
			break;	
		}	
	}
	if( strcmp( g_err_msg, ""  ) ){
		fprintf( stderr,"%s > %s\n", g_programm, g_err_msg );
	}

	fprintf( stderr,"Try '%s --help' for more information.\n", 
		g_programm );
};


//------------------------------------------------------------

static void 
show_help( )
{
	fprintf( stdout, 
"\
Usage: %s [OPTION]\n\
SVI. SIP VoIP cards initializer. Loads firmwire to chips and do\n\
		other necessary actions.\n\
\n\
It can get some options :)\n\
  -h, --help     display this help and exit\n\
  -V, --version  displey current version and license info\n\
\n\
Report bugs to <%s>.\n\
"
			, g_programm, g_email );
};

//------------------------------------------------------------

static void 
show_version( )
{
	fprintf( stdout, 
"\
%s-%s, built [%s]-[%s]\n\n\
Copyright (C) 2007 Free Software Foundation, Inc.\n\
This is free software.  You may redistribute copies of it under the terms of\n\
the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by %s. <%s>\n\
"
		, g_programm, g_version, 
		__DATE__, __TIME__, g_author, g_email);
};

//------------------------------------------------------------

