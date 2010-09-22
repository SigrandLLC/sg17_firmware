/*      dslam_sw.c:  Sigrand DSLAM switches control driver for linux (kernel 2.6.x)
 *
 *	Written 2010 by Scherbakov Mihail <m.u.x.a@mail.ru>
 *
 *	This driver provides control of DSLAM swiches for Sigrand SG-17 routers 
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 */
 
#include <linux/module.h>
#include <linux/config.h>
#include <linux/init.h>
#include <asm/am5120/adm5120.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>

#include "dslam_sw.h"
#include "dslam_sw_debug.h"

MODULE_DESCRIPTION ( "dslam switch control driver\n" );
MODULE_AUTHOR ( "Scherbakov Mihail\n" );
MODULE_LICENSE ( "GPL" );
MODULE_VERSION ( "1.0" );


/* --------------------------------------------------------------------------
 *      IC+ entry point
 * -------------------------------------------------------------------------- */

struct dslam_env {
	unsigned long io_outen;
	unsigned long io_imask;
	unsigned long io_omask;
	unsigned long c_outen;
	unsigned long c_imask;
	unsigned long c_omask;
	unsigned long regnum;
};


struct dslam_env chips[] = {
    { GPIO6_OUTPUT_EN,GPIO6_INPUT_MASK,GPIO6_OUTPUT_HI,GPIO7_OUTPUT_EN,GPIO7_INPUT_MODE,GPIO7_OUTPUT_HI, 0xf2},
    { GPIO1_OUTPUT_EN,GPIO1_INPUT_MASK,GPIO1_OUTPUT_HI,GPIO0_OUTPUT_EN,GPIO0_INPUT_MODE,GPIO0_OUTPUT_HI, 0xf2}
};

#define chips_num (sizeof(chips)/sizeof(struct dslam_env))

unsigned long ic_readpreamble = 0x58;
unsigned char ic_readpreamble_sz = 7; // 7 bit wide
unsigned long ic_writepreamble = 0x54;
unsigned char ic_writepreamble_sz = 7; // 7 bit wide
unsigned long ic_pause = 0x2;
unsigned char ic_pause_sz = 2; // 2 bit wide

static inline void
enable_bits(unsigned long bits) {
	ADM5120_SW_REG(GPIO_conf0_REG) |= bits;
}

static inline void
disable_bits(unsigned long bits) {
	ADM5120_SW_REG(GPIO_conf0_REG) &= (~bits);
}

static inline unsigned char
read_bit(unsigned long bit) {
	if( ADM5120_SW_REG(GPIO_conf0_REG) & bit )
		return 1;
	else
		return 0;
}


static inline void
dslam_write_mode(struct dslam_env *env) {
    // enable write on GPIO: MDIO=1(write mode) MDC=1 (write mode)
    enable_bits(env->io_outen | env-> io_omask | env->c_outen | env->c_omask);
}

static inline void 
dslam_read_mode(struct dslam_env *env) {
    // enable read on GPIO: MDIO (read mode)
    disable_bits(env->io_outen);
    enable_bits(env->c_outen | env->c_omask);
}

static inline void
dslam_empty_clock(struct dslam_env *env) {
    // Empty clock ticks between read & write
	disable_bits(env->c_omask);
	udelay(1);
	enable_bits(env-> c_omask);
	udelay(1);
	disable_bits(env->c_omask);
	udelay(1);
	enable_bits(env-> c_omask);
	udelay(1);
}

static void
dslam_write_bits(struct dslam_env *env, unsigned short outdata,unsigned char outbits) {
	int i;
	udelay(1);
	for(i=outbits-1;i>=0;i--) {
		if( (outdata>>i) & 0x1 ) {
			enable_bits(env->io_omask);
		} else {
			disable_bits(env->io_omask);
		}
		disable_bits(env->c_omask);
		udelay(1);
		enable_bits(env-> c_omask);
		udelay(1);
	}
}

static void
dslam_read_bits(struct dslam_env *env, unsigned long *indata,unsigned char inbits) {
	int i;
	unsigned long in = 0;
	udelay(1);
	for(i=0;i<inbits;i++){
		disable_bits(env->c_omask);
		udelay(1);
		in |= read_bit(env->io_imask);
		enable_bits(env-> c_omask);
		in <<= 1;
		udelay(1);
	}
	// remove last shift
	in >>= 1;
	*indata = in;
}

int write_reg(int sw_num, unsigned long reg_num, unsigned long reg_val) {
	struct dslam_env *env;
	if (sw_num > chips_num) {
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n", sw_num, chips_num);
		return -1;
	}
	env = &chips[sw_num];

	dslam_write_mode(env);
	dslam_write_bits(env,ic_writepreamble,ic_writepreamble_sz);	
	dslam_write_bits(env,reg_num,8);
	dslam_write_bits(env,ic_pause,ic_pause_sz);
	dslam_write_bits(env,reg_val,16);
	dslam_write_mode(env);

	return 0;
}

unsigned long read_reg(int sw_num, unsigned long reg_num) {
	struct dslam_env *env;
	unsigned long reg_val;
	if (sw_num > chips_num) {
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n", sw_num, chips_num);
		return 0;
	}
	env = &chips[sw_num];

	dslam_write_mode(env);
	dslam_write_bits(env,ic_readpreamble,ic_readpreamble_sz);
	dslam_write_bits(env,reg_num,8);
	dslam_read_mode(env);
	dslam_empty_clock(env);
	dslam_read_bits(env,&reg_val,16);
	dslam_write_mode(env);
	
	return reg_val;
}


static int 
read_reg_read(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	struct dslam_env *env;
	unsigned long reg_val = 0;
	
	env = &chips[0];

	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	
	env = &chips[sw_num];

	dslam_write_mode(env);
	dslam_write_bits(env,ic_readpreamble,ic_readpreamble_sz);
	dslam_write_bits(env,env->regnum,8);
	dslam_read_mode(env);
	dslam_empty_clock(env);
	dslam_read_bits(env,&reg_val,16);
	dslam_write_mode(env);
	
	return snprintf(buf,count,"0x%02lx 0x%04lx",env->regnum,reg_val);
}

static int
store_reg_read(struct file *file,const char *buffer,unsigned long count,void *data) {
	char *ptr=(char*)buffer;
	int sw_num = *(short*)data;
	struct dslam_env *env;
	char *endp;
	unsigned long reg_num;

	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	env = &chips[sw_num];
	
	ptr[count-1] = '\0';
	reg_num = simple_strtoul(ptr,&endp,16);
	env->regnum = reg_num;
	return count;
}

static int 
store_reg_write(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	struct dslam_env *env;
	char *endp;
	unsigned long reg_num,reg_val;
	
	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	env = &chips[sw_num];

	reg_num = simple_strtoul(buffer,&endp,16);
	while( *endp == ' '){
		endp++;
	}
	reg_val = simple_strtoul(endp,&endp,16);

	dslam_write_mode(env);
	dslam_write_bits(env,ic_writepreamble,ic_writepreamble_sz);	
	dslam_write_bits(env,reg_num,8);
	dslam_write_bits(env,ic_pause,ic_pause_sz);
	dslam_write_bits(env,reg_val,16);
	dslam_write_mode(env);
	
	return count;
}


static int 
store_default(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	
	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}

	if (simple_strtoul(buffer,&endp,16) == 1)
	{
		write_reg(sw_num, 0x3b, 0xffff);
		write_reg(sw_num, 0x3c, 0x07ff);
		write_reg(sw_num, 0x01, 0x0412);
		write_reg(sw_num, 0x3b, 0xffff);
		write_reg(sw_num, 0xff, 0x0080);
		write_reg(sw_num, 0xf9, 0x1e38);
		
		write_reg(sw_num, 0xd8, 0xffff);
		if (sw_num == 0)
			write_reg(sw_num, 0xd9, 0x37ff);
		else 
			write_reg(sw_num, 0xd9, 0x17ff);
	}
//	printk(KERN_NOTICE"default settings is set\n");
//	printk(KERN_NOTICE"0xd8=%lx\n", read_reg(sw_num, 0xd8));
	
	return count;
}

static int 
read_statistics(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	int i, j=0;

	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	j += snprintf(&buf[j], count, "port |   TX  |   RX\n");
	for (i = 0; i < 27*2; i+=2)
	{
		write_reg(sw_num, 0x38, 0x0100+(i&0xFF));
		if (i == 24*2) {
			j += snprintf(&buf[j], count, " 26  |%6lu |", (read_reg(sw_num, 0x39) << 16) + read_reg(sw_num, 0x3a));
		} else {
			if (i == 25*2) {
				j += snprintf(&buf[j], count, " 24  |%6lu |", (read_reg(sw_num, 0x39) << 16) + read_reg(sw_num, 0x3a));
			} else {
				if (i == 26*2) {
					j += snprintf(&buf[j], count, " 25  |%6lu |", (read_reg(sw_num, 0x39) << 16) + read_reg(sw_num, 0x3a));
				} else {
					j += snprintf(&buf[j], count, " %2i  |%6lu |", i/2, (read_reg(sw_num, 0x39) << 16) + read_reg(sw_num, 0x3a));
				}
			} 
		}

		write_reg(sw_num, 0x38, 0x0100+(i&0xFF)+1);
		j += snprintf(&buf[j], count, "%6lu\n", (read_reg(sw_num, 0x39) << 16) + read_reg(sw_num, 0x3a));
	}
	j += snprintf(&buf[j], count, "port |   TX  |   RX\n");
	j += snprintf(&buf[j], count, "port 24 - PHY(Gbit), 25 - GRMII(another switch), 26 - MII(CPU)\n");
	
	return j;
}
static int store_disable_learning(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	int port;
	int reg;
	if( sw_num > chips_num ) {
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	port = simple_strtoul(buffer,&endp,10);
	if (port <= 15) {
		reg = read_reg(sw_num, 0x45);
		write_reg(sw_num, 0x45, reg & ~(1<<port));
	} else {
		reg = read_reg(sw_num, 0x46);
		write_reg(sw_num, 0x46, reg & ~(1<<(port-16)));
	}
	return count;	
}
static int store_enable_learning(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	int port;
	int reg;
	if( sw_num > chips_num ) {
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	port = simple_strtoul(buffer,&endp,10);
	if (port <= 15) {
		reg = read_reg(sw_num, 0x45);
		write_reg(sw_num, 0x45, reg | 1<<port);
	} else {
		reg = read_reg(sw_num, 0x46);
		write_reg(sw_num, 0x46, reg | 1<<(port-16));
	}
	return count;	
}

static int store_mac(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	char port;
	int i = 0, j = 0;
	char mac_str[15], tmp;
	unsigned long long mac;
	
	if( sw_num > chips_num ) {
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}	
	strncpy(mac_str, buffer, 5);
	mac_str[5] = '\0';
	if (strcmp(mac_str, "clear") == 0) {
		for (i = 0; i < 4096; i++)
		{
			write_reg(sw_num, 0xC5, 0);
			write_reg(sw_num, 0xC6, 0);
			write_reg(sw_num, 0xC7, 0);
			write_reg(sw_num, 0xC8, 0);
			write_reg(sw_num, 0xC5, (i & 0xFFF) + (7 << 12));
		}
		printk(KERN_ERR"dslam_sw: LUT was creared\n");
		return count;
	}
	
	port = simple_strtoul(buffer,&endp,10);
	while( *endp == ' '){
		endp++;
	}
	if (port > 26) {
		printk(KERN_ERR"port must be from 0 to 26 (%d)\n", port);
		return count;
	}
	while ((endp[i] != ' ') && (endp[i] != '\0') && (i < count) && (j < 12)) {
		if (endp[i] == ':') {
			i++;
			continue;
		}
		mac_str[j] = endp[i];
		i++; j++;
	}
	mac_str[j] = '\0';
	
	tmp = mac_str[8];
	mac_str[8]='\n';
	mac = (unsigned long long)simple_strtoul(mac_str,&endp,16) << 16;
	mac_str[8] = tmp;
	mac += (unsigned long long)simple_strtoul(&mac_str[8],&endp,16);
//	printk(KERN_ERR"mac = %llx\n", mac);

	write_reg(sw_num, 0xC5, 0);
	write_reg(sw_num, 0xC6, (mac >> 12) & 0xFFFF);
	write_reg(sw_num, 0xC7, (mac >> 28) & 0xFFFF);
	write_reg(sw_num, 0xC8, 0x3800 + (port << 4) + ((mac >> 44) & 0xF));
	write_reg(sw_num, 0xC5, (mac & 0xFFF) + (7 << 12));

//	printk(KERN_ERR"%03x: %04x %04x %04x\n", (int)(mac & 0xFFF) + 1, reg_C8, reg_C7, reg_C6);
//	printk(KERN_ERR"mac %s, port %i\n", mac_str, port);

	if (!(read_reg(sw_num, 0xC5) && 0x8000))
		printk(KERN_ERR"ERROR write in LUT!!! reg 0xC5 = %04lx\n", read_reg(sw_num, 0xC5));
//	else
//		printk(KERN_ERR"added MAC %s behind port %i\n", mac_str, port);
	
	return count;
}


static int 
read_mac(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	int i;
	unsigned long j=0;
	unsigned long reg_c8, reg_c7, reg_c6;
	unsigned int reg_45, reg_46;
	
	if (offset > 0) {
		*eof = 1;
		return 0;
	}

	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	reg_45 = read_reg(sw_num, 0x45);
	reg_46 = read_reg(sw_num, 0x46);
	write_reg(sw_num, 0x45, 0x0000);
	write_reg(sw_num, 0x46, 0x0000);
	j += snprintf(&buf[j], count-j, "{");
	for (i = 0; i < 4096; i++)
	{
		write_reg(sw_num, 0xC5, 0x4000+i);
		reg_c8 = read_reg(sw_num, 0xC8);
		if (reg_c8 != 0) {
			reg_c7 = read_reg(sw_num, 0xC7);
			reg_c6 = read_reg(sw_num, 0xC6);
			j += snprintf(&buf[j], count-j, "\n%03x: %04lx %04lx %04lx\n", i, read_reg(sw_num, 0xC8), read_reg(sw_num, 0xC7), read_reg(sw_num, 0xC6));
			j += snprintf(&buf[j], count-j, "\t\"%01lx%01lx:%02lx:%01lx%01lx:%02lx:%01lx%01x:%02x\":\"%lu\",\n", 
			reg_c8&0xF, (reg_c7>>12)&0xF, (reg_c7>>4)&0xFF, reg_c7&0xF,
			(reg_c6>>12)&0xF, (reg_c6>>4)&0xFF, reg_c6&0xF, ((i-1)>>8)&0xF, ((i-1)&0xFF), (reg_c8>>4)&31);
		}

	}
	if (j != 1) j -= 1;
	j += snprintf(&buf[j], count-j, "\n}\n");

	write_reg(sw_num, 0x45, reg_45);
	write_reg(sw_num, 0x46, reg_46);
	return j;
}

static int 
read_status(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	int i, j=0, k;
	unsigned long val;

	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	j += snprintf(&buf[j], count, " ");
	for (i = 0; i < 27; i++)
		j += snprintf(&buf[j], count, "%2i  ", i);
	j += snprintf(&buf[j], count, "\n");
	
	for (i = 0; i < 24/3; i++)
	{
		val = read_reg(sw_num, (0xDE + (unsigned)i));
		for (k = 0; k < 3; k++)
		{
			if (val & (0x10 << (5*k))) {
				j += snprintf(&buf[j], count, " up ");
			} else {
				j += snprintf(&buf[j], count, " dwn ");
			}
		}
	}
	j += snprintf(&buf[j], count, "\n");
	
	return j;
}

static int store_vlan_mode(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	char mode;
	unsigned long tmp;
	
	if( sw_num > chips_num ) {
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	mode = simple_strtoul(buffer,&endp,10);
	
	tmp = read_reg(sw_num, 0x40);
	if (mode == PORT_BASED) {
		write_reg(sw_num, 0x40, tmp & 0xFFBF);
	}
	if (mode == TAG_BASED) {
		unsigned long tmp = read_reg(sw_num, 0x40);
		write_reg(sw_num, 0x40, tmp | 0x40);
	}
	return count;
}
static int read_vlan_mode(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	unsigned long tmp;
	
	if( sw_num > chips_num ) {
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	
	tmp = read_reg(sw_num, 0x40);
	if (!(tmp & 0x40)) {
		return(snprintf(buf, count, "port based\n"));
	}
	if (tmp & 0x40) {
		return(snprintf(buf, count, "tag based\n"));
	}
	return(snprintf(buf, count, "undefined\n"));	
}

static int 
read_vlan(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	int i, j=0;
	unsigned long val;

	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	j += snprintf(&buf[j], count, "VLAN settings:\n");
	for (i = 0; i < 27; i++)
	{
		val = read_reg(sw_num, (0x80 + (unsigned)i*2)) + (read_reg(sw_num, (0x81 + (unsigned)i*2)) << 16);
		j += snprintf(&buf[j], count, "p%02i=0x%08lx\n", i, val);
	}
	return j;
}

static int 
store_vlan(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	char port;
	unsigned long settings;
	
	if( sw_num > chips_num ) {
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}

	port = simple_strtoul(buffer,&endp,10);
	if (port > 26) {
		printk(KERN_ERR"dslam: ERROR port must be from 0 to 26, port=%d\n",port);
		return count;
	}
	while( *endp == ' '){
		endp++;
	}
	settings = simple_strtoul(endp,&endp,16);
	if (settings > 0x07ffffff) {
		printk(KERN_ERR"dslam: ERROR settings must be from 0 to 0x7ffffff, setting=%lx\n",settings);
		return count;
	}
	
//	printk(KERN_NOTICE"port=%d settings=%lx\n", port, settings);

	write_reg(sw_num, 0x80+2*port, settings & 0xffff);
	write_reg(sw_num, 0x81+2*port, (settings >> 16) & 0x07ff);
	
	return count;
}

// echo "entry_num vid val"
static int 
store_vid_table_entry(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	char entry_num;
	unsigned long vid;
	unsigned long val;
	
	entry_num = simple_strtoul(buffer,&endp,10);
	while( *endp == ' '){
		endp++;
	}
	if (entry_num > 31) {
		printk(KERN_ERR"entry_num must be from 0 to 31 (%d)\n", entry_num);
		return count;
	}
	vid = simple_strtoul(endp,&endp,10);
	while( *endp == ' '){
		endp++;
	}
	if (entry_num > 31) {
		printk(KERN_ERR"entry_num must be from 0 to 31 (%d)\n", entry_num);
		return count;
	}
	val = simple_strtoul(endp,&endp,16);
	
	write_reg(sw_num, 0x60+entry_num, vid);
	write_reg(sw_num, 0x80+2*entry_num, val & 0xFFFF);
	write_reg(sw_num, 0x80+1+2*entry_num, (val >> 16) & 0xFFFF);
//	printk(KERN_NOTICE"val = %lx high = %lx low = %lx\n", val, (val >> 16) & 0xFFFF, val & 0xFFFF);
	
	return count;	
}
static int 
read_vid_table_entry(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	int i, j=0;
	
	for (i = 0; i < 32; i++)
	{
		j += snprintf(&buf[j], count, "%i VID=%lu VLAN=%04lx%04lx\n", i, read_reg(sw_num, 0x60+i), read_reg(sw_num, 0x80+2*i+1), read_reg(sw_num, 0x80+2*i));
	}
	return j;
}

static int 
store_pvid(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	char port_num;
	unsigned pvid;
	int reg_num;
	
	port_num = simple_strtoul(buffer,&endp,10);
	while( *endp == ' '){
		endp++;
	}
	if (port_num > 26) {
		printk(KERN_ERR"port must be from 0 to 26 (%d)\n", port_num);
		return count;
	}
	pvid = simple_strtoul(endp,&endp,10);
	while( *endp == ' '){
		endp++;
	}
	if (pvid > 31) {
		printk(KERN_ERR"pvid must be from 0 to 31 (%d)\n", pvid);
		return count;
	}
	
	reg_num = (int)(port_num / 3);
//	printk(KERN_NOTICE"port_num = %i reg_num = %i port_num mod 3 = %i\n", port_num, reg_num, port_num % 3);
	
	switch (port_num % 3) {
		case 0:
			write_reg(sw_num, 0x53+reg_num, (read_reg(sw_num, 0x53+reg_num) & 0x7FE0) | pvid);
		break;
		case 1:
			write_reg(sw_num, 0x53+reg_num, (read_reg(sw_num, 0x53+reg_num) & 0x7C1F) | ((pvid << 5) & 0x3E0));
		break;
		case 2:
			write_reg(sw_num, 0x53+reg_num, (read_reg(sw_num, 0x53+reg_num) & 0x03FF) | ((pvid << 10) & 0x7C00));
		break;
	}
	return count;	
}
static int 
read_pvid(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	int i, j=0;
	unsigned pvid;
	int reg_num;
	
	for (i = 0; i <= 26; i++)
	{
		reg_num = (int)(i / 3);
	
		switch (i % 3) {
			case 0:
				pvid = read_reg(sw_num, 0x53+reg_num) & 0x1F;
			break;
			case 1:
				pvid = (read_reg(sw_num, 0x53+reg_num) >> 5) & 0x1F;
			break;
			case 2:
				pvid = (read_reg(sw_num, 0x53+reg_num)  >> 10) & 0x1F;
			break;
		}
		j += snprintf(&buf[j], count, "port %i PVID=%u\n", i, pvid);
	}
	return j;
}
static int 
store_tag(struct file *file,const char *buffer,unsigned long count,void *data) {
	int sw_num = *(short*)data;
	char *endp;
	char port_num;
	unsigned tag;
	
	port_num = simple_strtoul(buffer,&endp,10);
	while( *endp == ' '){
		endp++;
	}
	if (port_num > 26) {
		printk(KERN_ERR"port must be from 0 to 26 (%d)\n", port_num);
		return count;
	}
	tag = simple_strtoul(endp,&endp,10);
	while( *endp == ' '){
		endp++;
	}
	switch (tag) {
		case 0: // none
			if (port_num <= 15) {
				write_reg(sw_num, 0x5C, read_reg(sw_num, 0x5C) & (~(0x1 << port_num)));
				write_reg(sw_num, 0x5E, read_reg(sw_num, 0x5E) & (~(0x1 << port_num)));
			} else {
				write_reg(sw_num, 0x5D, read_reg(sw_num, 0x5D) & (~(0x1 << (port_num - 16))));
				write_reg(sw_num, 0x5F, read_reg(sw_num, 0x5F) & (~(0x1 << (port_num - 16))));
			}
		break;
		case 1: // tagging
			if (port_num <= 15) {
				write_reg(sw_num, 0x5C, read_reg(sw_num, 0x5C) | (0x1 << port_num));
				write_reg(sw_num, 0x5E, read_reg(sw_num, 0x5E) & (~(0x1 << port_num)));
			} else {
				write_reg(sw_num, 0x5D, read_reg(sw_num, 0x5D) | (0x1 << (port_num - 16)));
				write_reg(sw_num, 0x5F, read_reg(sw_num, 0x5F) & (~(0x1 << (port_num - 16))));
			}
		break;
		case 2: // untagging
			if (port_num <= 15) {
				write_reg(sw_num, 0x5C, read_reg(sw_num, 0x5C) & (~(0x1 << port_num)));
				write_reg(sw_num, 0x5E, read_reg(sw_num, 0x5E) | (0x1 << port_num));
			} else {
				write_reg(sw_num, 0x5D, read_reg(sw_num, 0x5D) & (~(0x1 << (port_num - 16))));
				write_reg(sw_num, 0x5F, read_reg(sw_num, 0x5F) | (0x1 << (port_num - 16)));
			}
		break;		
	}
	return count;	
}
static int 
read_tag(char *buf, char **start, off_t offset, int count, int *eof, void *data) {
	int sw_num = *(short*)data;
	int i, j=0;
	int tag=0, untag=0;
	
	for (i = 0; i <= 26; i++)
	{
		if (i <= 15) {
			tag = read_reg(sw_num, 0x5C) & (0x1 << i);
			untag = read_reg(sw_num, 0x5E) & (0x1 << i);
		} else {
			tag = read_reg(sw_num, 0x5D) & (0x1 << (i - 16));
			untag = read_reg(sw_num, 0x5F) & (0x1 << (i - 16));
		}
		if (tag) {
			j += snprintf(&buf[j], count, "port %i: tagging\n", i);
		} else {
			if (untag) {
				j += snprintf(&buf[j], count, "port %i: untagging\n", i);
			} else {
				if (!tag & !untag) {
					j += snprintf(&buf[j], count, "port %i: none\n", i);
				} else {
					j += snprintf(&buf[j], count, "port %i: error\n", i);
				}
			}
		}
	}
	return j;
}



/* --------------------------------------------------------------------------
 *      PROCFS initialisation/cleanup
 * -------------------------------------------------------------------------- */

#define MAX_PROC_STR 256

#define PFS_SW0 0
#define PFS_SW1 1

char dslam_procdir[]="sys/net/dslam_sw";
struct proc_dir_entry *dslam_entry;
struct proc_dir_entry *dslam_sw0_entry;
struct proc_dir_entry *dslam_sw1_entry;

struct dev_entrie {
	char *name;
	int mark;
	struct proc_dir_entry *pent;
	mode_t mode;
	read_proc_t *fread;
	write_proc_t *fwrite;
};

static int read_reg_read(char *buf, char **start, off_t offset, int count, int *eof, void *data);
static int store_reg_read(struct file *file,const char *buffer,unsigned long count,void *data);

//static int read_reg_write(char *buf, char **start, off_t offset, int count, int *eof, void *data);
static int store_reg_write(struct file *file,const char *buffer,unsigned long count,void *data);

static int store_default(struct file *file,const char *buffer,unsigned long count,void *data);


static struct dev_entrie entries_sw0[]={
	{ "default",  PFS_SW0, NULL, 0200, NULL, store_default },
	{ "statistics",  PFS_SW0, NULL, 0400, read_statistics, NULL },
	{ "status",  PFS_SW0, NULL, 0400, read_status, NULL },
	{ "mac",  PFS_SW0, NULL, 0600, read_mac, store_mac },
	{ "vlan_mode",  PFS_SW0, NULL, 0600, read_vlan_mode, store_vlan_mode },
	{ "vlan",  PFS_SW0, NULL, 0600, read_vlan, store_vlan },
	{ "pvid",  PFS_SW0, NULL, 0600, read_pvid, store_pvid },
	{ "tag",  PFS_SW0, NULL, 0600, read_tag, store_tag },
	{ "vid_table",  PFS_SW0, NULL, 0400, read_vid_table_entry, store_vid_table_entry },
	{ "regread",  PFS_SW0, NULL, 0600, read_reg_read, store_reg_read },
	{ "regwrite", PFS_SW0, NULL, 0200, NULL , store_reg_write },
	{ "disable_learning", PFS_SW0, NULL, 0200, NULL, store_disable_learning },
	{ "enable_learning", PFS_SW0, NULL, 0200, NULL, store_enable_learning },
};

static struct dev_entrie entries_sw1[]={
	{ "default",  PFS_SW1, NULL, 0200, NULL, store_default },
	{ "statistics",  PFS_SW1, NULL, 0400, read_statistics, NULL },
	{ "status",  PFS_SW1, NULL, 0400, read_status, NULL },
	{ "mac",  PFS_SW1, NULL, 0600, read_mac, store_mac },
	{ "vlan_mode",  PFS_SW1, NULL, 0600, read_vlan_mode, store_vlan_mode },
	{ "vlan",  PFS_SW1, NULL, 0600, read_vlan, store_vlan },
	{ "pvid",  PFS_SW1, NULL, 0600, read_pvid, store_pvid },
	{ "tag",  PFS_SW1, NULL, 0600, read_tag, store_tag },
	{ "vid_table",  PFS_SW1, NULL, 0400, read_vid_table_entry, store_vid_table_entry },
	{ "regread",  PFS_SW1, NULL, 0600, read_reg_read, store_reg_read },
	{ "regwrite", PFS_SW1, NULL, 0200, NULL, store_reg_write },
	{ "disable_learning", PFS_SW1, NULL, 0200, NULL, store_disable_learning },
	{ "enable_learning", PFS_SW1, NULL, 0200, NULL, store_enable_learning },
};

#define PFS_ENTS  (sizeof(entries_sw0)/sizeof(struct dev_entrie))

static int set_entry(struct proc_dir_entry *ent,read_proc_t *fread, write_proc_t *fwrite,int mark)
{
	short *mk;
	if( !( mk=(short *)kmalloc( sizeof( short ),GFP_KERNEL ) ) )
		return -1;
	*mk=mark;
	ent->data=(void *)mk;
	ent->owner=THIS_MODULE;
	ent->read_proc=fread;
	ent->write_proc=fwrite;
	return 0;
}


static int dslam_procfs_init(void) {
	int i;
	
	ADM5120_SW_REG(GPIO_conf0_REG) = 0;
	ADM5120_SW_REG(GPIO_conf2_REG) = 0;
	
	dslam_entry = proc_mkdir("sys/net/dslam_sw", NULL);
	if (dslam_entry == NULL)
		return -ENOMEM;
	dslam_sw0_entry = proc_mkdir("sys/net/dslam_sw/sw0", NULL);
	if (dslam_sw0_entry == NULL)
		return -ENOMEM;
	dslam_sw1_entry = proc_mkdir("sys/net/dslam_sw/sw1", NULL);
	if (dslam_sw1_entry == NULL)
		return -ENOMEM;
	
	for (i=0; i<PFS_ENTS; i++) {
		if ( !(entries_sw0[i].pent = create_proc_entry(entries_sw0[i].name,entries_sw0[i].mode,dslam_sw0_entry) ) )
			return -1;
		if ( !(entries_sw1[i].pent = create_proc_entry(entries_sw1[i].name,entries_sw1[i].mode,dslam_sw1_entry) ) )
			return -1;
		if ( set_entry(	entries_sw0[i].pent,entries_sw0[i].fread, entries_sw0[i].fwrite,entries_sw0[i].mark) )
			return -1;
		if ( set_entry(	entries_sw1[i].pent,entries_sw1[i].fread, entries_sw1[i].fwrite,entries_sw1[i].mark) )
			return -1;
	}
	
	return 0;
	
	dslam_write_mode(&chips[0]);
	dslam_write_mode(&chips[1]);
	PDEBUG(0,"conf0=%lx",ADM5120_SW_REG(GPIO_conf0_REG));

	return 0;
}        

static void dslam_procfs_remove(void) {
	int j;

	for ( j=0; j<PFS_ENTS; j++ ) {
		if( entries_sw0[j].pent->data )
			kfree(entries_sw0[j].pent->data);
		if( entries_sw1[j].pent->data )
			kfree(entries_sw1[j].pent->data);
	}

	for ( j=0; j<PFS_ENTS; j++ ) {
		remove_proc_entry(entries_sw0[j].name,dslam_sw0_entry);
		remove_proc_entry(entries_sw1[j].name,dslam_sw1_entry);
	}
	remove_proc_entry("sys/net/dslam_sw/sw0",NULL);
	remove_proc_entry("sys/net/dslam_sw/sw1",NULL);
	remove_proc_entry("sys/net/dslam_sw",NULL);
}        


/* --------------------------------------------------------------------------
 *      Module initialisation/cleanup
 * -------------------------------------------------------------------------- */

int __devinit
dslam_gpio_init( void ) {
	printk(KERN_NOTICE"Load DSLAM switch control driver\n");
	return dslam_procfs_init();
}

void __devexit
dslam_gpio_exit( void ) {
	printk(KERN_NOTICE"Unload DSLAM switch control driver\n");
	dslam_procfs_remove();
}


module_init(dslam_gpio_init);
module_exit(dslam_gpio_exit);
