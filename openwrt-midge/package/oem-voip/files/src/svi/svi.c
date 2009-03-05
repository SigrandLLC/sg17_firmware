/* INCLUDE {{{*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/kdev_t.h>
#include <errno.h>

#include "drv_tapi_io.h"	/* from TAPI_HL_driver */
#include "vinetic_io.h" 	/* from Vinetic_LL_driver */
#include "ab_ioctl.h"		/* from ATA_Board_driver */
/*}}}*/
/* DEFINE {{{*/
#define ERR_SUCCESS 0
#define ERR_MEMORY_FULL 1
#define ERR_UNKNOWN_OPTION 2
#define ERR_COULD_NOT_OPEN_FILE 3
#define ERR_IOCTL_FAILS 4
#define ERR_OTHER 5

#define ERR_MSG_SIZE 256

#define AB_FW_PRAM_NAME "/lib/firmware/pramfw.bin"
#define AB_FW_DRAM_NAME "/lib/firmware/dramfw.bin"
#define AB_FW_CRAM_FXS_NAME "/lib/firmware/cramfw_fxs.bin"
#define AB_FW_CRAM_FXO_NAME "/lib/firmware/cramfw_fxo.bin"

#define AB_SGATAB_DEV_NODE "/dev/sgatab"
#define AB_SGATAB_MAJOR_FILE "/proc/driver/sgatab/major"
#define VIN_DEV_NODE_PREFIX "/dev/vin"
#define VINETIC_MAJOR 121
#define FXO_OSI_MAX 600
/*}}}*/
/*{{{ Global VARS */
/*
COMMAND LINE KEYS:
  -h, --help		display this help and exit
  -V, --version		show version and exit
  -m, --skip-modules	skip modules reloading
  -d, --skip-devices	skip devices basicdev init
  -c, --skip-channels	skip channels firmware download
*/
struct _startup_options
{
	unsigned help : 1;
	unsigned version : 1;
	unsigned modules : 1;
	unsigned devices : 1;
	unsigned channels : 1;
} g_so;
static unsigned char g_err_no;
static unsigned char g_err_tag;
static char g_err_msg[ ERR_MSG_SIZE ];

static const unsigned char g_programm[ ] = "svi";
static const unsigned char g_version[ ] = "0.01-beta";
static const unsigned char g_author[ ] = "Vladimir Luchko";
static const unsigned char g_email[ ] = "vlad.luch@mail.ru";

static unsigned char * fw_pram = NULL;
static unsigned char * fw_dram = NULL;
static unsigned char * fw_cram_fxs = NULL;
static unsigned char * fw_cram_fxo = NULL;
static unsigned long fw_pram_size = 0;
static unsigned long fw_dram_size = 0;
static unsigned long fw_cram_fxs_size = 0;
static unsigned long fw_cram_fxo_size = 0;
/*}}}*/
/*{{{ Global FUNCTIONS */
static unsigned char startup_init( int argc, char ** argv );
static void run( void );

static int voip_in_slots( void );
static int load_modules( void );
static int init_voip( void );

static int board_iterator (int (*func)(ab_board_params_t const * const bp));
static int create_vinetic_nodes (void);
static int create_vin_board (ab_board_params_t const * const bp);
static int board_init (ab_board_params_t const * const bp);
static int dev_init(int const dev_idx, 
		ab_dev_params_t const * const dp, long const nIrqNum);
static int basicdev_init( int const dev_idx, ab_dev_params_t const * const dp, 
		long const nIrqNum);
static int chan_init (int const dev_idx, int const chan_idx, 
		dev_type_t const dt);
static int chan_init_tune (int const rtp_fd, int const chan_idx, 
		int const dev_idx, dev_type_t const dtype);
static int pd_ram_load( void );
static int cram_fxs_load( void );
static int cram_fxo_load( void );
static int fw_masses_init_from_path (unsigned char ** const fw_buff, 
		unsigned long * const buff_size, char const * const path );
static void fw_masses_free( void );

static void Error_message( int argc, char ** argv );
static void show_last_err(char * msg, int fd);
static void show_help( void );
static void show_version( void );
/*}}}*/

int 
main( int argc, char ** argv )
{/*{{{*/
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
		run();
	}

main_exit:
	Error_message( argc, argv );

	return g_err_no;
}/*}}}*/

static void 
run( void )
{/*{{{*/
	int err = 0;

	/* test the voip modules presence */
	err = voip_in_slots();
	if(err){
		goto __exit;
	}

	/* load drivers stack and create node files */
	err = load_modules();
	if(err){
		goto __exit;
	}

	/* init devices and channels */
	err = init_voip();
	if(err){
		goto __exit_ioctl_msg;
	}
__exit:
	return;
__exit_ioctl_msg:
	return;
}/*}}}*/

static int 
voip_in_slots( void )
{/*{{{*/
	int err;
	err = system("cat /proc/pci | grep 0055:009c > /dev/null || exit 1");
	return err;
}/*}}}*/

static int 
load_modules( void )
{/*{{{*/
	char sgatab_major_str[20];
	int sgatab_major;
	int fd;
	mode_t uma;
	int err;

	uma = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	if(g_so.modules){
		goto __exit_success;
	}

	/* remove all modules and they dev nodes */
	system("modprobe -r drv_sgatab");
	system("modprobe -r drv_daa");
	system("modprobe -r drv_vinetic");
	system("modprobe -r drv_tapi");
	system("rm -f "VIN_DEV_NODE_PREFIX"*");
	remove(AB_SGATAB_DEV_NODE);

	/* load sgatab module */
	err = system("modprobe drv_sgatab");
	if(err){
		g_err_no = ERR_OTHER;
		sprintf(g_err_msg, "%s() svi ERROR : drv_sgatab load",__func__);
		goto __exit_fail;
	}

	/* create sgatab dev node */
	fd = open(AB_SGATAB_MAJOR_FILE, O_RDONLY);
	if(fd == -1){
		g_err_no = ERR_COULD_NOT_OPEN_FILE;
		sprintf(g_err_msg, "%s() ERROR : drv_sgatab major file open",
				__func__);
		goto __exit_fail;
	}
	err = read(fd, sgatab_major_str, sizeof(sgatab_major_str));
	close(fd);
	if(!err){
		g_err_no = ERR_OTHER;
		sprintf(g_err_msg, "%s() ERROR : drv_sgatab major file empty",
				__func__);
		goto __exit_fail;
	} else if(err == -1){
		g_err_no = ERR_OTHER;
		sprintf(g_err_msg, "%s() ERROR : reading drv_sgatab major "
				"file : \n\t%s", __func__, strerror(errno));
		goto __exit_fail;
	}

	sgatab_major = strtol(sgatab_major_str, NULL, 10);
	err = mknod(AB_SGATAB_DEV_NODE, S_IFCHR | uma, MKDEV(sgatab_major,0));
	if(err){
		g_err_no = ERR_OTHER;
		sprintf(g_err_msg, "%s() ERROR : drv_sgatab dev file create :\n"
				"\t%s\n", __func__, strerror(errno));
		goto __exit_fail;
	}

	/* load infineon modules stack */
	err = system("modprobe drv_tapi");
	if(err){
		g_err_no = ERR_OTHER;
		sprintf(g_err_msg, "%s() ERROR : drv_tapi load",__func__ );
		goto __exit_fail;
	}
	err = system("modprobe drv_vinetic");
	if(err){
		g_err_no = ERR_OTHER;
		sprintf(g_err_msg, "%s() ERROR : drv_vinetic load",__func__ );
		goto __exit_fail;
	}
	err = system("modprobe drv_daa");
	if(err){
		g_err_no = ERR_OTHER;
		sprintf(g_err_msg, "%s() ERROR : drv_daa load",__func__ );
		goto __exit_fail;
	}

	/* create vinetic nodefiles */
	err = create_vinetic_nodes();
	if(err){
		goto __exit_fail;
	}

__exit_success:
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

static int
create_vinetic_nodes( void )
{/*{{{*/
	int err;
	err = board_iterator(create_vin_board);
	if(err){
		goto __exit_fail;
	}
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

static int 
init_voip( void )
{/*{{{*/
	int err;
	err = board_iterator(board_init);
	if(err){
		goto __exit_fail;
	}
	fw_masses_free();
	return 0;
__exit_fail:
	fw_masses_free();
	return -1;
}/*}}}*/

static int
board_iterator (int (*func)(ab_board_params_t const * const bp))
{/*{{{*/
	ab_board_params_t bp;
	int bc;
	int fd;
	int i;
	int err;

	static int boards_count_changed_tag = 0;
	/*  0 - first run
	 *  >0 && !=bc - count of boards changes 
	 * */

	fd = open(AB_SGATAB_DEV_NODE, O_RDWR);
	if(fd==-1){
		g_err_no = ERR_COULD_NOT_OPEN_FILE;
		sprintf(g_err_msg, "%s() ERROR : drv_sgatab dev file open",
				__func__ );
		goto __exit_fail;
	}

	err = ioctl(fd, SGAB_GET_BOARDS_COUNT, &bc);
	if(err){
		g_err_no = ERR_IOCTL_FAILS;
		sprintf(g_err_msg, "%s() ERROR : SGAB_GET_BOARDS_COUNT ioctl",
				__func__ );
		goto __exit_fail_close;
	}

	if( boards_count_changed_tag == 0){
		/*first run of board_iterator()*/
		boards_count_changed_tag = bc;
	} else if(boards_count_changed_tag != bc){
		/* not first run & count of boards changed */
		g_err_no = ERR_OTHER;
		sprintf(g_err_msg, "%s() ERROR : count of boards changed",
				__func__ );
		goto __exit_fail_close;
	}

	/* do it for all boards */
	for (i=0; i<bc; i++){
		/* get board params */
		memset(&bp, 0, sizeof(bp));
		bp.board_idx = i;
		err = ioctl(fd, SGAB_GET_BOARD_PARAMS, &bp);
		if(err){
			g_err_no = ERR_IOCTL_FAILS;
			sprintf(g_err_msg, "%s() ERROR : SGAB_GET_BOARD_PARAMS" 
					" ioctl", __func__ );
			goto __exit_fail_close;
		} else if((!bp.is_present) || bp.is_count_changed){
			g_err_no = ERR_OTHER;
			sprintf(g_err_msg, "%s() ERROR : SGAB_GET_BOARD_PARAMS"
					" boards count changed", __func__ );
			goto __exit_fail_close;
		}
		/* execute func on current board */
		err = func (&bp);
		if(err){
			goto __exit_fail_close;
		}
	}

	close (fd);
	return 0;
__exit_fail_close:
	close (fd);
__exit_fail:
	return -1;
}/*}}}*/

static int 
board_init (ab_board_params_t const * const bp)
{/*{{{*/
	int i;
	int err;
	static int dev_idx = 0;

	for(i=0; i<DEVS_PER_BOARD_MAX; i++){
		if(bp->devices[i].type != dev_type_ABSENT){
			/* init device and it`s channels */
			err = dev_init (dev_idx, &bp->devices[i], bp->nIrqNum);
			if(err){
				goto __exit_fail;
			}
			dev_idx++;
		}
	}
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

static int 
dev_init (int const dev_idx, ab_dev_params_t const * const dp, 
		long const nIrqNum)
{/*{{{*/
	int j;
	int err;

	/* dev basic init */
	err = basicdev_init(dev_idx, dp, nIrqNum);
	if(err){
		goto __exit_fail;
	}

	if(g_so.channels){
		goto __exit_success;
	}

	/* download fw and chans init */
	for(j=0; j<CHANS_PER_DEV; j++){
		/* channel init */
		err = chan_init(dev_idx, j, dp->type);
		if(err){
			goto __exit_fail;
		}
	}

__exit_success:
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

static int 
basicdev_init( int const dev_idx, ab_dev_params_t const * const dp, 
		long const nIrqNum)
{ /*{{{*/
	IFX_int32_t err;
	VINETIC_BasicDeviceInit_t binit;
	int cfg_fd;
	char dev_node[ 50 ];

	if(g_so.devices){
		goto __exit_success;
	}

	memset(&binit, 0, sizeof(binit));
	binit.AccessMode = dp->AccessMode;
	binit.nBaseAddress = dp->nBaseAddress;
	binit.nIrqNum = nIrqNum;

	sprintf(dev_node,VIN_DEV_NODE_PREFIX"%d0", dev_idx+1);

	cfg_fd = open(dev_node, O_RDWR);
	if(cfg_fd==-1){
		g_err_no = ERR_COULD_NOT_OPEN_FILE;
		sprintf(g_err_msg, "%s() opening vinetic device node '%s'",
				__func__, dev_node);
		goto __exit_fail;
	}

	err = ioctl(cfg_fd, FIO_VINETIC_DEV_RESET, 0);
	if(err){
		g_err_no = ERR_IOCTL_FAILS;
		sprintf(g_err_msg, "%s() device reset fails (ioctl)",__func__);
		show_last_err(">>", cfg_fd);
		goto __exit_fail_close;
	}

	err = ioctl (cfg_fd, FIO_VINETIC_BASICDEV_INIT, &binit);
	if(err){
		g_err_no = ERR_IOCTL_FAILS;
		sprintf(g_err_msg, "%s() BasicDev init fails (ioctl)",__func__);
		show_last_err(">>", cfg_fd);
		goto __exit_fail_close;
	}

	close (cfg_fd);
__exit_success:
	return 0;
__exit_fail_close:
	close (cfg_fd);
__exit_fail:
	return -1;
}/*}}}*/

static int
chan_init(int const dev_idx, int const chan_idx, dev_type_t const dt)
{/*{{{*/
	IFX_TAPI_CH_INIT_t init;
	VINETIC_IO_INIT vinit;
	char cnode[ 50 ];
	int cfd;
	int err;

	/* Initialize channel */
	sprintf(cnode, "/dev/vin%d%d", dev_idx+1, chan_idx+1);

	cfd = open(cnode, O_RDWR);
	if (cfd==-1){
		g_err_no = ERR_COULD_NOT_OPEN_FILE;
		sprintf(g_err_msg,"%s() opening vinetic channel node",__func__);
		goto __exit_fail;
	}

	memset(&vinit, 0, sizeof(vinit));

	err = pd_ram_load();
	if(err){
		goto __exit_fail_close;
	}

	vinit.pPRAMfw = fw_pram;
	vinit.pram_size = fw_pram_size;
	vinit.pDRAMfw = fw_dram;
	vinit.dram_size = fw_dram_size;

	if(dt==dev_type_FXO){
		cram_fxo_load();
		vinit.pCram     = fw_cram_fxo;
		vinit.cram_size = fw_cram_fxo_size;
		vinit.pBBDbuf   = fw_cram_fxo;
		vinit.bbd_size  = fw_cram_fxo_size;
	} else if(dt==dev_type_FXS){
		cram_fxs_load();
		vinit.pCram     = fw_cram_fxs;
		vinit.cram_size = fw_cram_fxs_size;
		vinit.pBBDbuf   = fw_cram_fxs;
		vinit.bbd_size  = fw_cram_fxs_size;
	}

	/* Set the pointer to the VINETIC dev specific init structure */
	memset(&init, 0, sizeof(IFX_TAPI_CH_INIT_t));
	init.pProc = (IFX_void_t *) &vinit;
	/* Init the VoIP application */
	init.nMode = IFX_TAPI_INIT_MODE_VOICE_CODER;

	/* Initialize channel */
	err = ioctl(cfd, IFX_TAPI_CH_INIT, (IFX_uint32_t) &init);
	if( err ){
		g_err_no = ERR_IOCTL_FAILS;
		sprintf(g_err_msg,"%s() initilizing channel with firmware (ioctl)",
				__func__);
		show_last_err(">>", cfd);
		goto __exit_fail_close;
	}

	err = chan_init_tune( cfd, chan_idx, dev_idx, dt );
	if( err ){
		goto __exit_fail_close;
	}

	close(cfd);
	return 0;
__exit_fail_close:
	close(cfd);
__exit_fail:
	return -1;
}/*}}}*/

static int 
chan_init_tune( int const rtp_fd, int const chan_idx, int const dev_idx,
		dev_type_t const dtype )
{/*{{{*/
	IFX_TAPI_MAP_DATA_t datamap;
	IFX_TAPI_LINE_TYPE_CFG_t lineTypeCfg;
	IFX_TAPI_SIG_DETECTION_t dtmfDetection;
	IFX_TAPI_SIG_DETECTION_t faxSig;
	int err = 0;

	memset(&datamap, 0, sizeof (datamap));
	memset(&lineTypeCfg, 0, sizeof (lineTypeCfg));

	/* Map channels */	
	datamap.nDstCh = chan_idx; 
	datamap.nChType = IFX_TAPI_MAP_TYPE_PHONE;
	err = ioctl(rtp_fd, IFX_TAPI_MAP_DATA_ADD, &datamap);
	if( err ){
		g_err_no = ERR_IOCTL_FAILS;
		strcpy(g_err_msg, "mapping channel to it`s own data (ioctl)");
		show_last_err(">>", rtp_fd);
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
		show_last_err(">>", rtp_fd);
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
			show_last_err(">>", rtp_fd);
			goto ab_chan_init_tune__exit;
		}
		/* ENABLE detection of FAX signals */
		memset (&faxSig, 0, sizeof(faxSig));
		faxSig.sig = IFX_TAPI_SIG_CEDRX | IFX_TAPI_SIG_CEDTX |
			IFX_TAPI_SIG_CEDENDRX | IFX_TAPI_SIG_CEDENDTX;
		err = ioctl (rtp_fd,IFX_TAPI_SIG_DETECT_ENABLE,&faxSig);
		if(err){
			g_err_no = ERR_IOCTL_FAILS;
			strcpy(g_err_msg, "trying to enable FAX signal detection (ioctl)" );
			show_last_err(">>", rtp_fd);
			goto ab_chan_init_tune__exit;
		}
	} else if(dtype == dev_type_FXO) {
		/* DISABLE detection of DTMF tones 
		 * from local interface (ALM X) */
		IFX_TAPI_FXO_OSI_CFG_t osi_cfg;
		memset(&dtmfDetection, 0, sizeof (dtmfDetection));
		dtmfDetection.sig = IFX_TAPI_SIG_DTMFTX;
		err = ioctl (rtp_fd,IFX_TAPI_SIG_DETECT_DISABLE,&dtmfDetection);
		if( err ){
			g_err_no = ERR_IOCTL_FAILS;
			strcpy(g_err_msg, "trying to disable DTMF ALM signal "
					"detection (ioctl)" );
			show_last_err(">>", rtp_fd);
			goto ab_chan_init_tune__exit;
		}
		/* set OSI timing */
		memset(&osi_cfg, 0, sizeof (osi_cfg));
		osi_cfg.nOSIMax = FXO_OSI_MAX;
		err = ioctl(rtp_fd, IFX_TAPI_FXO_OSI_CFG_SET, &osi_cfg);
		if( err ){
			g_err_no = ERR_IOCTL_FAILS;
			strcpy(g_err_msg, "trying to set OSI timing (ioctl)" );
			show_last_err(">>", rtp_fd);
			goto ab_chan_init_tune__exit;
		}
	}
	return 0;
ab_chan_init_tune__exit:
	return -1;
};/*}}}*/

static int
pd_ram_load( void )
{/*{{{*/
	int err;
	if( !fw_pram){
		err = fw_masses_init_from_path (&fw_pram, &fw_pram_size, 
				AB_FW_PRAM_NAME );
		if(err){
			goto __exit_fail;
		}
	}
	if( !fw_dram){
		err = fw_masses_init_from_path (&fw_dram, &fw_dram_size, 
				AB_FW_DRAM_NAME );
		if(err){
			goto __exit_fail;
		}
	}
	return 0; 
__exit_fail:
	return -1;
}/*}}}*/

static int
cram_fxs_load( void )
{/*{{{*/
	int err;
	if( !fw_cram_fxs){
		err = fw_masses_init_from_path (&fw_cram_fxs, &fw_cram_fxs_size, 
				AB_FW_CRAM_FXS_NAME);
		if(err){
			goto __exit_fail;
		}
	}
	return 0; 
__exit_fail:
	return -1;
}/*}}}*/

static int
cram_fxo_load( void )
{/*{{{*/
	int err;
	if( !fw_cram_fxo){
		err = fw_masses_init_from_path (&fw_cram_fxo, &fw_cram_fxo_size, 
				AB_FW_CRAM_FXO_NAME);
		if(err){
			goto __exit_fail;
		}
	}
	return 0; 
__exit_fail:
	return -1;
}/*}}}*/

static int 
fw_masses_init_from_path (unsigned char ** const fw_buff, 
		unsigned long * const buff_size, char const * const path )
{/*{{{*/
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
}/*}}}*/

static void 
fw_masses_free( void )
{/*{{{*/
	if(fw_pram) {
		free(fw_pram);
		fw_pram_size = 0;
		fw_pram = NULL;
	}
	if(fw_dram) {
		free(fw_dram);
		fw_dram_size = 0;
		fw_dram = NULL;
	}
	if(fw_cram_fxs) {
		free(fw_cram_fxs);
		fw_cram_fxs_size = 0;
		fw_cram_fxs = NULL;
	}
	if(fw_cram_fxo) {
		free(fw_cram_fxo);
		fw_cram_fxo_size = 0;
		fw_cram_fxo = NULL;
	}
}/*}}}*/

static int 
create_vin_board (ab_board_params_t const * const bp)
{/*{{{*/
	int i;
	int err;
	mode_t uma;
	static int dev_idx = 0;

	uma = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	for(i=0; i<DEVS_PER_BOARD_MAX; i++){
		if(bp->devices[i].type != dev_type_ABSENT){
			int j;
			char dev_name[50];
			memset(dev_name, 0, sizeof(dev_name));
			snprintf (dev_name, sizeof(dev_name), "%s%d0",
					VIN_DEV_NODE_PREFIX, dev_idx+1);
			err = mknod(dev_name, S_IFCHR|uma,MKDEV(VINETIC_MAJOR,
					(dev_idx+1)*10));
			if(err){
				g_err_no = ERR_OTHER;
				sprintf(g_err_msg, "%s() ERROR : drv_vinetic "
						"dev file create :\n\t%s\n",
						__func__, strerror(errno));
				goto __exit_fail;
			}
			for(j=0; j<CHANS_PER_DEV; j++){
				char ch_name[50];
				memset(ch_name, 0, sizeof(ch_name));
				snprintf (ch_name, sizeof(ch_name), "%s%d%d",
						VIN_DEV_NODE_PREFIX, 
						dev_idx+1, j+1);
				err = mknod(ch_name, S_IFCHR|uma, MKDEV(
						VINETIC_MAJOR,(dev_idx+1)*10+
						j+1));
				if(err){
					g_err_no = ERR_OTHER;
					sprintf(g_err_msg, "%s() ERROR : "
							"drv_vinetic chan "
							"file create :\n\t%s\n",
							__func__, 
							strerror(errno));
					goto __exit_fail;
				}
			}
			dev_idx++;
		}
	}
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

static unsigned char 
startup_init( int argc, char ** argv )
{/*{{{*/
	int option_IDX;
	int option_rez;
	char * short_options = "hVmdc";
	struct option long_options[ ] = {
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V' },
		{ "skip-modules", no_argument, NULL, 'm' },
		{ "skip-devices", no_argument, NULL, 'd' },
		{ "skip-channels", no_argument, NULL, 'c' },
		{ NULL, 0, NULL, 0 },
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
			case 'm': {
				g_so.modules = 1;
				break;
			}
			case 'd': {
				g_so.devices = 1;
				break;
			}
			case 'c': {
				g_so.channels = 1;
				break;
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
}/*}}}*/

static void 
Error_message( int argc, char ** argv )
{/*{{{*/
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
		case ERR_COULD_NOT_OPEN_FILE:{
			fprintf( stderr, "%s : file open error\n", g_programm );
			break;	
		}
		case ERR_IOCTL_FAILS:{
			fprintf( stderr, "%s : some ioctl fails\n",g_programm );
			break;	
		}
	}
	if( strcmp( g_err_msg, ""  ) ){
		fprintf( stderr,"%s > %s\n", g_programm, g_err_msg );
	}

	fprintf( stderr,"Try '%s --help' for more information.\n", 
		g_programm );
}/*}}}*/

static void
show_last_err(char * msg, int fd)
{/*{{{*/
	int error;
	ioctl (fd, FIO_VINETIC_LASTERR, &error);
	fprintf (stderr,"%s: 0x%X\n", msg, error);
}/*}}}*/

static void 
show_help( )
{/*{{{*/
	fprintf( stdout, 
"\
Usage: %s [OPTION]\n\
SVI. SIP VoIP cards Initializer. Loads firmwire to chips and do\n\
		other necessary actions.\n\
\n
It can get some options :)\n\
  -h, --help     	display this help and exit\n\
  -V, --version  	displey current version and license info\n\
  -m, --skip-modules	skip modules reloading \n\
  -d, --skip-devices	skip devices basicdev init \n\
  -c, --skip-channels	skip channels firmware init \n\
\n\
Report bugs to <%s>.\n\
"
			, g_programm, g_email );
}/*}}}*/

static void 
show_version( )
{/*{{{*/
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
}/*}}}*/

