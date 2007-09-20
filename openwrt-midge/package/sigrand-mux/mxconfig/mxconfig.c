#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <malloc.h>
#include <string.h>

#include "dev_interface.h"
#include "mxobjects.h"
#include "mxconfig.h"

#define IFS_ROOT "/home/artpol/work/tmp/ifs-root"
//#define IFS_ROOT "/sys/class/net"

typedef enum { help,setup,info,full_info,chck } action_t;

void print_usage(char *name)
{
    printf("Usage: %s [Options]\n"
	    "Options:\n"
    	    "  -i, --iface\tinterface name\n"
    	    "  -l, --list\tdevice multiplexing settings\n"
    	    "  -c, --check\tcheck multiplexing configuration\n"
    	    "  -r, --rline\tTransmit multiplexer bus line\n"
    	    "  -t, --tline\tReceive multiplexer bus line\n"
	    "  -a, --mxrate\tSet multiplexing rate (use for SHDSL)\n"
    	    "  -o, --mxsmap\tSlopmap used for multiplexing (use for E1)\n"
    	    "  -j, --tfs\tTransmit Frame Start (TFS)\n"
    	    "  -g, --rfs\tReceive Frame Start (RFS)\n"
    	    "  -e, --mxen\tEnable(1)/disable(0) multiplexing on interface\n"
    	    "  -m, --clkm\tSet interface as clock master(1)/slave(0)\n"
    	    "  -d, --clkab\tSet clock domain: A(0), B(1)\n"
    	    "  -u, --clkr\tClock master uses local(0) or remote(1) oscillator\n",
	    name);
}

action_t
process_args(int argc,char **argv,char **ifname,devsetup_t *settings)
{
    action_t type = help;
    char *endp;


    // Default settings
    settings->type = unknown_ts;
    settings->mx_slotmap = 0;
    settings->mxrate = -1;
    settings->rline = settings->tline = -1;
    settings->rfs = settings->tfs = -1;    
    settings->clkm = settings->clkr = settings->clkab = -1;
    settings->mxen = -1;
    
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
    	    {"iface", 1, 0, 'i'},
    	    {"list", 0, 0, 'l'},
    	    {"check", 0, 0, 'c'},
    	    {"rline", 1, 0, 'r'},
    	    {"tline", 1, 0, 't'},
    	    {"mxrate", 1, 0, 'a'},
    	    {"mxsmap", 1, 0, 'o'},
    	    {"tfs", 1, 0, 'j'},
    	    {"rfs", 1, 0, 'g'},
    	    {"mxen", 1, 0, 'e'},
    	    {"clkr", 1, 0, 'u'},
    	    {"clkab", 1, 0, 'd'},
    	    {"clkm", 1, 0, 'm'},
    	    {0, 0, 0, 0}
	};

	int c = getopt_long (argc, argv, "i:slr:t:a:b:c:d:e:f:g:m:",
                long_options, &option_index);
        if (c == -1)
    	    break;
	switch (c) {
        case 'i':
	    *ifname = strdup(optarg);
            break;
        case 'l':
	    type = info;
    	    break;
	case 'c':
	    switch( type ){
	    case setup:
	    case help:
	    case info:
		type = chck;
		break;
	    }
	    break;
	case 'r':
	    type = setup;
	    settings->rline = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --rline option: %s not integer\n",optarg);
		return 0;
	    }else if( settings->rline > 15 || settings->rline < 0 ){
		printf("Error: --rline must be in [0..15]\n");
		return 0;
	    }
	    break;
	case 't':
	    type = setup;
	    settings->tline = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --tline option: %s not integer\n",optarg);
		return 0;
	    }else if( settings->tline > 15 || settings->tline < 0 ){
		printf("Error: --tline must be in [0..15]\n");
		return 0;
	    }
	    break;
	case 'a':
	    type = setup;
	    if( settings->type != unknown_ts ){
		printf("Error: using both timeslots and continual modes for one device\n");
		return 0;
	    }
	    settings->mxrate = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --mxrate option: %s not integer\n",optarg);
		return 0;
	    }
	    settings->type = continual_ts;
	    break;
	case 'o':
	{
	    type = setup;
	    int err;
	    if( settings->type != unknown_ts ){
		printf("Error: using both timeslots and continual modes for one device\n");
		return 0;
	    }
	    settings->mx_slotmap = str2slotmap(optarg,strlen(optarg),&err);
	    if( err ){
		printf("Error: in --mxsmap mask: %s",optarg);
		return 0;
	    }
	    settings->type = selective_ts;
	    break;
	}
	case 'j':
	    type = setup;
	    settings->tfs = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --tfs option: %s not integer\n",optarg);
		return 0;
	    }else if( settings->tfs > 255 || settings->tfs < 0){
		printf("Error: --tfs must be in [0..255]\n");
		return 0;
	    }
	    break;
	case 'g':
	    type = setup;
	    settings->rfs = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --rfs option: %s not integer\n",optarg);
		return 0;
	    }else if( settings->rfs > 255 || settings->rfs < 0){
		printf("Error: --rfs must be in [0..255]\n");
		return 0;
	    }
	    break;
	case 'e':
	    type = setup;
	    settings->mxen = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --mxen option: %s not integer\n",optarg);
		return 0;
	    }else if( settings->mxen > 1 || settings->mxen < 0){
		printf("Error: --mxen must be in 0 or 1\n");
		return 0;
	    }
	    break;
	case 'u':
	    type = setup;
	    settings->clkr = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --clkr option: %s not integer\n",optarg);
		return 0;
	    }else if( settings->clkr > 1 || settings->clkr < 0){
		printf("Error: --clkr must be in 0 or 1\n");
		return 0;
	    }
	    break;
	case 'm':
	    type = setup;
	    settings->clkm = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --clkm option: %s not integer\n",optarg);
		return 0;
	    }else if( settings->clkm > 1 || settings->clkm < 0){
		printf("Error: --clkm must be in 0 or 1\n");
		return 0;
	    }
	    break;
	case 'd':
	    type = setup;
	    settings->clkab = strtoul(optarg,&endp,0);
	    if( endp == optarg ){
		printf("Error: in --clkab option: %s not integer\n",optarg);
		return 0;
	    }else if( settings->clkab > 1 || settings->clkab < 0){
		printf("Error: --clkab must be in 0 or 1\n");
		return 0;
	    }
	    break;
	}
    }

    return type;
}

void print_device(ifdescr_t *ifd)
{
    devsetup_t *settings = &ifd->settings;

    printf("  %s: mxen=%d clkm=%d clkab=%d clkr=%d rline=%d tline=%d rfs=%d tfs=%d ",
	ifd->name,settings->mxen,settings->clkm,settings->clkab,settings->clkr,
	settings->rline,settings->tline,settings->rfs,settings->tfs);

    switch( settings->type ){
    case selective_ts:
	{
	    char buf[BUF_SIZE];
	    int err;
	    slotmap2str(settings->mx_slotmap,buf);
	    printf(" mxsmap=%s\n",buf);
	}
	break;
    case continual_ts:
	printf(" mxrate=%d\n",settings->mxrate);
	break;
    }
} 

void
mxline_print(mxline_t *l,int lind,char *sname)
{
    int i;
    printf("Line%d %s devices:\n",lind,sname);
    for(i=0;i<l->devcnt;i++){
	print_device(l->devs[i].ifd);
    }
}

inline int 
is_crossing(u8 start1,u8 width1,u8 start2,u8 width2)
{
    return !( (start1+(width1-1))<start2 || (start2+(width2-1))<start1 );
}			    

inline int 
is_contain(u8 start1,u8 width1,u8 start2,u8 width2)
{
    return !( (start1<start2) && (start1+(width1-1))<(start2+(width2-1)) );
}			    

int
check_xline(mxline_t *l,int bindex,char *sname)
{
    int i,j;
    for(i=0;i<l->devcnt;i++){
	mxelem_t *cur =&l->devs[i];
	for(j=i+1;j<l->devcnt;j++){
	    mxelem_t *tmp =&l->devs[j];
	    u32 end1 = cur->xfs+(cur->mxrate-1);
	    u32 end2 = tmp->xfs+(tmp->mxrate-1);
	    if( is_crossing(cur->xfs,cur->mxrate,tmp->xfs,tmp->mxrate) ){
		mxerror("Line %d: %s timeslots crossing - %s(%d-%d) and %s(%d-%d)",
			bindex,sname,cur->ifd->name,cur->xfs,end1,
			tmp->ifd->name,tmp->xfs,end2 );
	    }
	}
    }
}


int
check(ifdescr_t **iflist,int iflsize)
{
    int i;
    mxline_t *rlines[MX_LINES],*tlines[MX_LINES];
    int clkAm[MAX_IFS],clkBm[MAX_IFS];
    u16 clkAm_cnt=0,clkBm_cnt=0,clkA_cnt=0,clkB_cnt=0;
    int derr = 0;

    memset(rlines,0,sizeof(rlines));
    memset(tlines,0,sizeof(tlines));
    memset(clkAm,0,sizeof(clkAm));
    memset(clkBm,0,sizeof(clkBm));

    // Fill lines
    for(i=0;i<iflsize;i++){
	u32 mxrate = 0;
	mxelem_t el;
	devsetup_t *set = &iflist[i]->settings;
	if( !set->mxen )
	    continue;
	switch( set->clkab ){
	case clkA:
	    clkA_cnt++;
	    break;
	case clkB:
	    clkB_cnt++;
	    break;
	}
	
	if( set->clkm ){
	    switch( set->clkab ){
	    case clkA:
		clkAm[clkAm_cnt] = i;
		clkAm_cnt++;
		break;
	    case clkB:
		clkBm[clkBm_cnt] = i;
		clkBm_cnt++;
		break;
	    }
	}else if( set->clkr ){
	    mxwarn("%d: clkR option is ignored - not clock master");
	}
	
	switch( set->type ){
	case continual_ts:
	    mxrate = set->mxrate;
	    break;
	case selective_ts:
	    mxrate = slotmap2mxrate(set->mx_slotmap);
	    break;
	}
	
	el.ifd = iflist[i];
	
	el.xfs = set->rfs;
	el.mxrate = mxrate;
	if( !rlines[set->rline] ){
	    rlines[set->rline] = mxline_init();
	}
	if( mxline_add(rlines[set->rline],set->clkab,el) ){
	    mxerror("Cannot add device %s to line %d, too many interfaces",
		iflist[i]->name,set->rline);
	}

	el.xfs = set->tfs;
	el.mxrate = mxrate;
	if( !tlines[set->tline] ){
	    tlines[set->tline] = mxline_init();
	}
	if( mxline_add(tlines[set->tline],set->clkab,el) ){
	    mxerror("Cannot add device %s to line %d, too many interfaces",
		iflist[i]->name,set->tline);
	}
    }
    
    // 1. In each domain one and only one master 
    if( !clkAm_cnt && clkA_cnt )
	mxerror("No clock master in domain A");
    if( clkAm_cnt>1 ){
	int i;
	mxerror("More than one clock master in domain A:");
	for(i=0;i<clkAm_cnt;i++){
	    print_device(iflist[clkAm[i]]);
	}
    }
    if( !clkBm_cnt && clkB_cnt)
	mxerror("No clock master in domain B");
    if( clkBm_cnt>1 ){
	int i;
	mxerror("More than one clock master in domain B:");
	for(i=0;i<clkBm_cnt;i++){
	    print_device(iflist[clkBm[i]]);
	}
    }

    // 2. All devices of line in one domain
    for(i=0;i<MX_LINES;i++){
	int rerr=0,terr=0;
	if( !rlines[i] && !tlines[i] )
	    continue; // line is not used
	if( !rlines[i] )
	    mxwarn("Line %d: writing but not reading");
	if( !tlines[i] )
	    mxwarn("Line %d: reading but not writing");
	
	if( rlines[i] && rlines[i]->domain_err ){
	    mxerror("Line %d: not all writing devices in same domain");
	    mxline_print(rlines[i],i,"read");
	    derr = 1;
	}
	if( tlines[i] && tlines[i]->domain_err){
	    mxerror("Line %d: not all reading devices in same domain");
	    mxline_print(tlines[i],i,"write");
	    derr = 1;
	}
    }

    // 3. Check that timeslots inside line do not cross
    for(i=0;i<MX_LINES;i++){
	if( !rlines[i] && !tlines[i] )
	    continue;
	if( rlines[i] ){
	    check_xline(rlines[i],i,"rline");
	}
	if( tlines[i] ){    
	    check_xline(tlines[i],i,"tline");
	}
    }
    
    
    // 3. Проверить что на линии один мастер
    // 4. Проверить что у всех устройств шины один домен
    // 5. ?? Проверить что устройства обмениваются ПОПАРНО, а не второем, например
    //
    //
}

int main(int argc, char *argv[] )
{
    char *ifname = NULL;
    int iflen = 0;
    char *pname = strdup(argv[0]);
    action_t type = help;    
    devsetup_t settings;
    iftype_t iftype = unknown_ts;
    ifdescr_t *iflist[MAX_IFS];    
    int cnt = fill_iflist(IFS_ROOT,iflist,MAX_IFS);

    type = process_args(argc,argv,&ifname,&settings);

    if( ifname )
	iflen = strlen(ifname);
    
    switch( type ){
    case help:
	print_usage(pname);
	break;
    case info:
	if( ifname ){
	    printf("Display short info about %s\n",ifname);
	    int i;
	    for(i=0;i<cnt;i++){
	        int tmplen = strlen(iflist[i]->name);
	        if( tmplen != iflen )
	    	    continue;
		if( !strcmp(ifname,iflist[i]->name) ){
		    print_device(iflist[i]);
		    break;
		}
	    }
	}else{
	    int i;
	    for(i=0;i<cnt;i++){
		print_device(iflist[i]);
	    }
	}
	return 0;
    case full_info:
	if( ifname ){
	    printf("Display short info about %s\n",ifname);
	    int i;
	    for(i=0;i<cnt;i++){
	        int tmplen = strlen(iflist[i]->name);
	        if( tmplen != iflen )
	    	    continue;
		if( !strcmp(ifname,iflist[i]->name) ){
//		    print_full(IFS_ROOT,iflist[i]);
		    break;
		}
	    }
	}else{
	    int i;
	    for(i=0;i<cnt;i++){
//		print_full(IFS_ROOT,iflist[i]);
	    }
	}
	return 0;
    case setup:
	{
	    devsetup_t *set = NULL;
	    int i;
	    for(i=0;i<cnt;i++){
		set = &iflist[i]->settings;
		int tmplen = strlen(iflist[i]->name);
		if( tmplen != iflen )
		    continue;
		if( !strcmp(ifname,iflist[i]->name) )
		    break;
	    }
	    if( i == cnt || !set){
		printf("Error: cannot find interface %s\n",ifname);
		return -1;
	    }

/*	    
	    char err_str[256];
	    switch( set->type ){
	    case continual_ts:
	        strcpy(err_str,"mxrate");
	        break;
	    case selective_ts:
	        strcpy(err_str,"mxsmap");
	        break;
	    }    
	    printf("Error: %s: for this type of interface you must specify %s\n",ifname,err_str);
	    return 0;
*/
	    iflist[i]->settings = settings;	    
	    apply_settings(IFS_ROOT,iflist[i]);
	}
	break;
    case chck:
	check(iflist,cnt);
	break;
    }
    
    return 0;
}

