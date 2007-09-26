#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <dirent.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "dev_interface.h"

#define MAX_TS_BIT 32

int
check_for_mxsupport(char *conf_path,ifdescr_t *ifd) 
{
    DIR *dir;
    int ocnt = 0,i;
    struct dirent *ent;
    char d[MAX_FNAME];
    char *opts[] = { "mx_clkab","mx_clkm","mx_clkr","mx_enable",
			"mx_rline","mx_rxstart","mx_tline","mx_txstart" };
    int optsize = sizeof(opts)/sizeof(char *);
    iftype_t type;
    
    snprintf(d,MAX_FNAME,"%s/%s/sg_multiplexing",conf_path,ifd->name);
    
    if( !(dir = opendir(d)) ){
	PDEBUG(DERR,"Cannot open dir: %s",d);
	return -1;
    }

    type = unknown_ts;
    while( ent = readdir(dir) ){
        int flen = strlen(ent->d_name);
	int optlen = strlen("mxrate");
	if( flen == optlen && !strncmp("mxrate",ent->d_name,flen) ){
	    if( type != unknown_ts ){
		closedir(dir);
		return -1;
	    }
	    type = continual_ts;
	    continue;
	}

	optlen = strlen("mx_slotmap");
	if( flen == optlen && !strncmp("mx_slotmap",ent->d_name,flen) ){
	    if( type != unknown_ts ){
		closedir(dir);
		return -1;
	    }
	    type = selective_ts;
	    continue;
	}

	for(i=0;i<optsize;i++){
	    int optlen = strlen(opts[i]);
	    if( flen != optlen )
		continue; // not equal
	    PDEBUG(DFULL,"check equal of: %s - %s",opts[i],ent->d_name);
	    if( !strncmp(opts[i],ent->d_name,flen) ){
		PDEBUG(DFULL,"%s - %s is equal",opts[i],ent->d_name);
		ocnt++;
		break;
	    }
	}
    }
    closedir(dir);
    ifd->settings.type = type;
    PDEBUG(DFULL,"ocnt = %d, opts_num = %d",ocnt,opts_num);
    return (ocnt == optsize ) ? 0 : -1;	
}

int
fill_iflist(char *conf_path,ifdescr_t **iflist,int ifcnt)
{
    DIR *dir;
    struct dirent *ent;
    int cnt = 0,i;
    char fname[MAX_FNAME];
    struct stat statbuf;
    devsetup_t settings;
    
    for(i=0;i<ifcnt;i++){
	iflist[i] = NULL;
    }

    if( !(dir = opendir(conf_path)) ){
	PDEBUG(DERR,"Cannot open dir: %s",conf_path);
	return -1;
    }

    while( ent = readdir(dir) ){
	iflist[cnt] = malloc(sizeof(ifdescr_t));
        iflist[cnt]->name = strdup(ent->d_name);
	if( check_for_mxsupport(conf_path,iflist[cnt]) ){
	    free(iflist[cnt]->name);
	    free(iflist[cnt]);
	    iflist[cnt] = NULL;
	}else{
	    if( accum_settings(conf_path,iflist[cnt]) ){
		free(iflist[cnt]->name);
		free(iflist[cnt]);
		iflist[cnt] = NULL;
		continue;
	    }
	    cnt++;
	}
    }

    closedir(dir);
    return cnt;
}

unsigned int
str2slotmap(char *str,size_t size,int *err)
{
    char *e,*s=str;
    unsigned int fbit,lbit,ts=0;
    int i;

    PDEBUG(4,"start");	
    for (;;) {
        fbit=lbit=strtoul(s, &e, 0);
        if (s == e)
            break;
        if (*e == '-') {
            s = e+1;
            lbit = strtoul(s, &e, 0);
        }

        if (*e == ','){
            e++;
        }

        if( !(fbit < MAX_TS_BIT && lbit < MAX_TS_BIT) )
            break;
        
	for (i=fbit; i<=lbit;i++){
	    ts |= 1L << i;
	}
        s=e;
    }
    PDEBUG(4,"str=%08x, s=%08x,size=%d",(u32)str,(u32)s,size);
    *err=0;	
    if( s != (str+size) && s != str )
        *err=1;
    return ts;
}


int
slotmap2str(unsigned int smap, char *buf,int offset)
{
    int start = -1,end, i;
    char *p=buf;

    if( !smap ){
	buf[0] = 0;
	return 0;
    }
    
    for(i=0;i<32;i++){
	if( start<0 ){
	    start = ((smap >> i) & 0x1) ? i : -1;
	    if( start>0 && i==31 )
		p += sprintf(p,"%d",start+offset);
	}else if( !((smap >> i) & 0x1) || i == 31){
	    end = ((smap >> i) & 0x1) ? i : i-1;
	    if( p>buf )
		p += sprintf(p,",");
	    p += sprintf(p,"%d",start+offset);
	    if( start<end )
		p += sprintf(p,"-%d",end+offset);
	    start=-1;
	}
    }
    return strlen(buf);
}

u8 
slotmap2mxrate(u32 smap)
{
    int mxrate = 0;
    while(smap){
	if(smap&0x01)
	    mxrate++;
	smap = (smap>>1) &0x7fffffff;
    }
    return mxrate;
}


int 
set_int_option(char *conf_path,char *name,int val)
{
    int fd;
    int len,cnt;
    char fname[MAX_FNAME];
    char buf[BUF_SIZE];
	
    PDEBUG(DFULL,"set_dev_option(%s,%s)",name,val); 
    snprintf(fname,MAX_FNAME,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_WRONLY)) < 0 ){
	PDEBUG(DERR,"set_dev_option: Cannot open %s",fname);
	return -1;
    }
    snprintf(buf,BUF_SIZE,"%d",val);
    len = strlen(buf);
    cnt = write(fd,buf,len);
    PDEBUG(DFULL,"set_dev_option: write %d, written %d",len,cnt);
    close(fd);
    if( cnt != len )
	return -1;
    return 0;
}

int 
set_char_option(char *conf_path,char *name,char *str)
{
    int fd;
    int len,cnt;
    char fname[MAX_FNAME];
	
    PDEBUG(DFULL,"set_dev_option(%s,%s)",name,val); 
    snprintf(fname,MAX_FNAME,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_WRONLY)) < 0 ){
	PDEBUG(DERR,"set_dev_option: Cannot open %s",fname);
	return -1;
    }
    len = strlen(str)+1;
    cnt = write(fd,str,len);
    PDEBUG(DFULL,"set_dev_option: write %d, written %d",len,cnt);
    close(fd);
    if( cnt != len )
	return -1;
    return 0;
}

int
get_int_option(char *conf_path,char *name,int *val)
{
    int fd;
    int cnt;
    char fname[MAX_FNAME];
    char buf[BUF_SIZE];
    char *endp;
    int tmp=0;
    
    PDEBUG(DFULL,"get_dev_option(%s)",name); 
    snprintf(fname,MAX_FNAME,"%s/%s",conf_path,name);

    if( (fd = open(fname,O_RDONLY)) < 0 ){
	PDEBUG(DERR,"get_dev_option: Cannot open %s",fname);
	return -1;
    }

    cnt = read(fd,buf,BUF_SIZE);
    PDEBUG(DFULL,"get_dev_option: readed %d",cnt);

    close(fd);

    if( cnt < 0 || !cnt){
	return -1;
    }
    buf[cnt] = 0;

    *val = strtoul(buf,&endp,0);
    if (endp == buf )
	return -1;

    return 0;
}

int
get_char_option(char *conf_path,char *name,char **val)
{
    int fd;
    int cnt;
    char fname[MAX_FNAME];
    char buf[BUF_SIZE];
    char *endp;
    
    PDEBUG(DFULL,"get_dev_option(%s)",name); 
    snprintf(fname,MAX_FNAME,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_RDONLY)) < 0 ){
	PDEBUG(DERR,"get_dev_option: Cannot open %s",fname);
	return -1;
    }
    cnt = read(fd,buf,BUF_SIZE);
    PDEBUG(DFULL,"get_dev_option: readed %d",cnt);
    close(fd);
    if( cnt < 0 ){
	return -1;
    }
    buf[cnt] = 0;
    *val = strdup(buf);
    return 0;
}

int
apply_settings(char *conf_path,ifdescr_t *ifd)
{
    char conf[MAX_FNAME];
    char buf[BUF_SIZE];
    devsetup_t *set = &ifd->settings;
    
    snprintf(conf,MAX_FNAME,"%s/%s/sg_multiplexing",conf_path,ifd->name);
    
    if( set->type == continual_ts && set->mxrate>=0 ){
	if( set_int_option(conf,"mxrate",set->mxrate) )
	    return -1;
    }else if( set->type == selective_ts ){
	slotmap2str(set->mx_slotmap,buf,0);
	if( set_char_option(conf,"mx_slotmap",buf) )
	    return -1;
    }

    if( set->rline >=0 ){
	set_int_option(conf,"mx_rline",set->rline);
    }

    if( set->tline >=0 ){
	set_int_option(conf,"mx_tline",set->tline);
    }

    if( set->rfs >=0 ){
	set_int_option(conf,"mx_rxstart",set->rfs);
    }
    
    if( set->tfs >=0 ){
	set_int_option(conf,"mx_txstart",set->tfs);
    }

    if( set->clkm >=0 ){
	set_int_option(conf,"mx_clkm",set->clkm);
    }

    if( set->clkab >=0 ){
	set_int_option(conf,"mx_clkab",set->clkab);
    }

    if( set->clkr >=0 ){
	set_int_option(conf,"mx_clkr",set->clkr);
    }

    if( set->mxen >=0 ){
	set_int_option(conf,"mx_enable",set->mxen);
    }

}

int
accum_settings(char *conf_path,ifdescr_t *ifd)
{
    char conf[MAX_FNAME];
    char *buf;
    int size,err;
    devsetup_t *set = &ifd->settings;
    int tmp=0;
    
    snprintf(conf,MAX_FNAME,"%s/%s/sg_multiplexing",conf_path,ifd->name);
    
    // Read used timeslots information
    switch( set->type ){
    case continual_ts: 
	if( get_int_option(conf,"mxrate",&tmp) )
	    return -1;
	set->mxrate = tmp;
	break;
    case selective_ts:
	if( get_char_option(conf,"mx_slotmap",&buf) )
	    return -1;
	set->mx_slotmap = str2slotmap(buf,strlen(buf),&err);
	if( err && strlen(buf) )
	    return -1;
	break;
    }
    
    // 
    if( get_int_option(conf,"mx_rline",&tmp) ){
	return -1;
    }
    set->rline = tmp;

    if( get_int_option(conf,"mx_tline",&tmp) ){
	return -1;
    }
    set->tline = tmp;
    
    if( get_int_option(conf,"mx_rxstart",&tmp) ){
	return -1;
    }
    set->rfs = tmp;
    
    if( get_int_option(conf,"mx_txstart",&tmp) ){
	return -1;
    }
    set->tfs = tmp;

    if( get_int_option(conf,"mx_clkm",&tmp) ){
	return -1;
    }
    set->clkm = tmp;

    if( get_int_option(conf,"mx_clkab",&tmp) ){
	return -1;
    }
    set->clkab= tmp;

    if( get_int_option(conf,"mx_clkr",&tmp) ){
	return -1;
    }
    set->clkr = tmp;

    if( get_int_option(conf,"mx_enable",&tmp) ){
	return -1;
    }
    set->mxen = tmp;
    return 0;
}


