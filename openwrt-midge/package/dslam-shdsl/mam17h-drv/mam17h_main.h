#ifndef MAM17H_H
#define MAM17H_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/firmware.h>
#include <asm/io.h>
#include <linux/time.h>

#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/vermagic.h>
#include <linux/version.h>

#include <asm/types.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/kobject.h>


#define MAM17H_PCI_VENDOR 0x55
#define MAM17H_PCI_DEVICE 0x95
#define MAM17H_MODNAME "mam17h"
#define MAM17_IOMEM_SIZE 0x1000


#define TWO_WIRE 0
#define SYMETRIC 0
#define EOC_ENABLED_IDC 1
#define NO_POWER 0
#define DATA_OR_NETWORK_CLK 0x04
#define LP_DISABLE       0x01
#define LP_ENABLE        0x02
#define SELECT_16_PAM    0x01
#define SELECT_32_PAM    0x02
#define NEW_STYLE_CAPLIST 0x0
#define  PMMS_OFF 0x00


#define MASTER 1
#define SLAVE 0

#define ANNEX_A 0
#define ANNEX_B 1
#define ANNEX_A_B 2
#define ANNEX_AB 3

#define AUTO_PAM_SELECT 0

#define TCPAM4		1
#define TCPAM8		2
#define TCPAM16		3
#define TCPAM32		4
#define TCPAM64		5
#define TCPAM128	6



#define GHS_TRNS_00 0
#define GHS_TRNS_01 1
#define GHS_TRNS_10 2
#define GHS_TRNS_11 3
#define STARTUP_LOCAL 0
#define STARTUP_FAREND 0x80
#define PBO_NORMAL 0
#define PBO_FORCED 4
#define PMMS_NORMAL 0
#define EPL_ENABLED 0x08
#define EPL_DISABLED 0
#define SHDSL_CLK_MODE_1 0x1
#define SHDSL_CLK_MODE_2 0x2
#define SHDSL_CLK_MODE_3a 0x4
#define SHDSL_CLK_MODE_3b 0x8
#define SDI_LOW 0
#define SDI_FALLING 0
#define SDI_HIGH 1
#define SDI_RISING 1
#define SDI_NO 0
#define SDI_YES 1
#define SLIP_FAST 0
#define SLIP_NORMAL4 1
#define SLIP_NORMAL8 2





// DFE commands
#define CMD_CFG_SDI_TX 0x841F
#define CMD_CFG_SDI_RX 0x842F
#define CMD_CFG_GHS_MODE 0x2422
#define CMD_CFG_CAPLIST_SHORT_VER_2 0x2452
#define CMD_CFG_GHS_EXTENDED_PAM_MODE 0x2472

#define CMD_SDI_REMOTE_LOOP_ENABLE 0x8C1F

#define DFE 1
#define IDC 2

#define SDFE4_FIFO8	128
#define SDFE4_FIFO32 SDFE4_FIFO8 / 4
#define PEF24624_ADR_HOST 0xF3
#define Embedded_IDC 0x01
#define Embedded_DFE 0x02
#define PEF24624_ADR_DEV 0xF0

#define PEF24624_ADR_AUX	        0x09
#define PEF24624_ADR_RAMSHELL 0x05
#define PEF24624_ADR_IDC_RAMSHELL 0x08

#define HDLC_HDR_SZ		0x02
#define MSG_AUX_CMDHDR_SZ	0x01
#define MSG_AUX_ACKHDR_SZ	0x00
#define AUX_CMDHDR_SZ		(HDLC_HDR_SZ+MSG_AUX_CMDHDR_SZ)
#define AUX_ACKHDR_SZ		(HDLC_HDR_SZ+MSG_AUX_ACKHDR_SZ)



#define MSG_RAM_CMDHDR_SZ	0x02
#define MSG_RAM_ACKHDR_SZ	0x00
#define RAM_CMDHDR_SZ		(HDLC_HDR_SZ+MSG_RAM_CMDHDR_SZ)
#define RAM_ACKHDR_SZ		(HDLC_HDR_SZ+MSG_RAM_ACKHDR_SZ)

#define  CMD_WR_REG_AUX_SCI_IF_MODE  0x00A9
#define  CMD_WR_RAM_RS               0x0003
#define  CMD_WR_REG_RS_FWSTART       0x0061
#define  CMD_RD_REG_RS_FWSTART       0x0060
#define  CMD_WR_REG_RS_FWCTRL        0x0001
#define  CMD_WR_REG_RS_FWDTPNT       0x0041
#define  CMD_RD_REG_RS_FWCRC         0x0020

#define FWGP1_IFETCH                   0x00000002
#define FWGP1_MWAIT                    0x00000004
#define FWGP1_RAMLOADED                0x00000001

#define  FWCTRL_CHK                  0x00000100
#define  FWCTRL_SWITCH               0x00010000
#define  FWCTRL_PROTECT              0x01000000
#define  FWCTRL_VALID                0x00000001





#ifndef IO_READ_WRITE
#       define iowrite8(val,addr)  writeb(val,addr)
#       define iowrite16(val,addr)  writeb(val,addr)
#       define iowrite32(val,addr)  writel(val,addr)
#       define ioread8(addr) readb(addr)
#       define ioread16(addr) readb(addr)
#       define ioread32(addr) readl(addr)
#endif

/* bits descriptions of microprocessor interface resiters */

// MPI_CON
#define EFFLU 0x02
#define INFFLU 0x01
// MPI_EFSTAT
#define EFFL  0x20 //00111111
// MPI_EINT
#define INTSCI 0x02
#define INTIDC 0x01
// MPI_EINT_EN
#define MPILEV 0x04
#define SCIEN  0x02
#define IDCEN  0x01
// MPI_IINT_EN MPI_IINT
#define R 0x01
// SCI_CTRL_L
#define RMC       0x10
#define Daisy_Res 0x04
#define RRES      0x02
#define XRES      0x01
// SCI_CTRL_H
#define XME 0x80
#define XTF 0x40
// SCI_CFG_L
#define WBM    0xC0 //00
#define RBM    0x30 //00
#define CLKGAT 0x08
#define ARBIT  0x04 //0
#define DUPLEX 0x02 //0
#define LOOP   0x01 //0
// SCI_CFG_H
#define RIL     0xC0 //recommeneded 00
#define SRA     0x20 //1
#define RAC     0x10 //1
#define RCRC    0x08 //recommeneded 0
#define XCRC    0x04 //1
#define CLKPOL  0x01 //recommeneded 0
// SCI_REPORT_L
#define RBC_L 0xFF
// SCI_REPORT_H
#define OVF   0x80
#define RACI  0x40
#define XACI  0x20
#define RBC_H 0x0F
// SCI_CLKCFG
#define SCI_GEN_EN 0x80
#define SCI_CLK_F  0x07
// SCI_INTEN && SCI_INT
#define XDOV 0x40
#define XPR  0x20
#define XMR  0x10
#define XDU  0x08
#define RPF  0x04
#define RME  0x02
#define RFO  0x01


/* bits descriptions of mr17bh4 resiters */

// CRA && SR
#define PWRF0 0x01
#define PWRF1 0x02
#define XINT  0x20
#define XRST  0x80
// PWRR0 && PWRR1
#define PWRON 0x01
#define PWRIE 0x02
#define OVL   0x04
#define UNB   0x08
// CRB
#define RXDE  0x08
#define LLED0 0x40
#define LLED1 0x80


// mpi commands
#define CMD_PMD_Reset 0x160
#define CMD_PMD_CO_PortSubTypeSelect 0x60
#define CMD_PMD_SpanProfileGroupConfig 0x64
#define CMD_PMD_Control 0x161
#define CMD_LinkControl 0x80
#define CMD_TC_FlowModify 0x8
#define CMD_HDLC_TC_LinkModify 0x20
#define CMD_xMII_Modify 0x1A
#define CMD_PMD_AlarmControl 0x166
#define CMD_TNL_PMD_0_Message 0x180
#define CMD_TNL_PMD_1_Message 0x181
#define CMD_TNL_PMD_2_Message 0x182
#define CMD_TNL_PMD_3_Message 0x183
#define CMD_PMD_StatusGet 0x162
#define CMD_CFG_SYM_DSL_MODE 0x0404
#define CMD_HDLC_TC_LinkPmParamGet 0x143
#define CMD_HDLC_TC_LinkCorruptPacketControl 0x140
#define CMD_TC_LayerLoopControl 0x91

#define CMD_SystemInterfaceLoopControl 0x90

#define ALM_PMD_TC_LayerMismatch 0x604
#define EVT_InitializationComplete 0x680
#define EVT_EOC_LinkState 0x688

#define ALM_PMD_StatusChanged 0x603

// channel states
#define DOWN_NOT_READY 0
#define DOWN_READY 4
#define STOP_DOWN_READY 0x14
#define INITIALIZING 1
#define UP_DATA_MODE 3

// mpi ACK
#define ACK_PMD_CO_PortSubTypeSelect 0x260
#define ACK_PMD_SpanProfileGroupConfig 0x264
#define ACK_PMD_Control 0x361
#define ACK_TC_FlowModify 0x208
#define ACK_HDLC_TC_LinkModify 0x220

#define BIT_STUFFING 1
#define HDLC_IF_CH_FFH 0xFF
#define IFX_ENABLE 1
#define IFX_DISABLE 0
#define NO_CLK 0
#define FORCE_LINK_DOWN 0
#define ENABLE_LINK 1
#define START_TRAINING 4
#define HDLC_TC_LAYER 0
#define START_AFTER_INIT 3
#define STOP_AFTER_INIT 2
#define MII_100BT 1
#define FULL_DUPLEX 1
#define NORMAL 0
#define PWRBO_FORCED 1
#define PWRBO_NORMAL 0

#define  TPS_TC_A		     0x66
#define  TPS_TC_B		     0x68

#define  CMD_GHS_CAP_GET	     0x2822
#define  CMD_PERF_STATUS_GET         0x9432


struct channel {
	u32 mode : 2;
	u32 state;
	u32 need_reset;
	u32 tcpam : 3;
	u16 rate;
	u32 annex : 2;
	u32 clkmode : 2;
	u32 pbo_mode : 1;
	u8 pbo_vals[16];
	u8 pbo_vnum;
	u8 crc16;
	u8 fill_7e;
	char name[16];
};

struct mam17_card {
	int number;
	int if_num;
	int mpair;
	int mpair_mode;
	int state;
	u8 pwr_source;
	char name[40];
	atomic_t locked; // if some of cards interfaces are up
	// PCI card fields
	struct pci_dev *pdev;
	void *mem_base;

	spinlock_t chip_lock;
	wait_queue_head_t wait;
	struct work_struct work;
	struct regs_str *regs;
	struct channel channels[4];

//	struct sg17_sci sci;
//	struct sdfe4 hwdev;
	// netdev fields
	struct net_device *ndevs[4];
	
};

struct statistics {
	u8 SNR_Margin_dB;
	u8 LoopAttenuation_dB;
	u8 ES_count;
	u8 SES_count;
	u16 CRC_Anomaly_count;
	u8 LOSWS_count;
	u8 UAS_Count;
	u16 SegmentAnomaly_Count;
	u8 SegmentDefectS_Count;
	u8 CounterOverflowInd;
	u8 CounterResetInd;
};

struct cmd_pmd_spanprofilegroupconfig {
	u32 LinkNo;
	u32 wireinterface;
	u32 minlinerate;
	u32 maxlinerate;
	u32 minlinesubrat;
	u32 maxlinesubrat;
	u32 psd;
	u32 transmod;
	u32 remoteenabled;
	u32 powerfeeding;
	u32 cc_targetmargindown;
	u32 wc_targetmargindown;
	u32 cc_targetmarginup;
	u32 wc_targetmarginup;
	u32 usedtargetmargins;
	u32 refcloc;//DATA_OR_NETWORK_CLK????????????????????????????????
	u32 lineprobe;
	u32 pam_constellation;
	u32 capliststyl;
	u32 pbo_mode;
	u32 epl_mode;
	u32 pbo_valu;
};

struct ack_ghs_cap_get {
	u8 ClType;
	u8 ClParam;
	u8 ClOctetNrNPar;
	u8 ClOctetNrSPar;
	u8 rsvd0;
	u8 rsvd1;
	u8 rsvd2;
	u8 rsvd3;
	u8 ClData[4];
};	

struct cmd_ghs_cap_get {
	u8 ClType;
	u8 ClParam;
	u8 rsvd0;
	u8 rsvd1;
	u8 rsvd2;
	u8 rsvd3;
	u8 rsvd4;
	u8 rsvd5;
};	

struct cmd_cfg_sym_dsl_mode
{
       u8 mode;
       u8 repeater;
       u8 standard;
       u8 rsvd0;
       u8 rsvd1;
       u8 rsvd2;
       u8 rsvd3;
       u8 rsvd4;
       u8 rsvd5;
       u8 rsvd6;
       u8 rsvd7;
};
#define TERMINATOR 0
#define REPEATER   1
#define SHDSL      1

#define STU_C 1
#define STU_R 2

struct cmd_cfg_ghs_mode{
	u8 transaction;
	u8 startup_initialization;
	u8 pbo_mode;
	u8 pmms_margin_mode;
	u8 epl_mode;
	u8 rsvd1;
	u8 rsvd2;
	u8 rsvd3;
	u8 rsvd4;
	u8 rsvd5;
	u8 rsvd6;
	u8 rsvd7;
};
struct cmd_cfg_caplist_short_ver_2{
	u8 clock_mode;
	u8 annex;
	u8 psd_mask;
	u8 pow_backoff;
	u16 base_rate_min;
	u16 base_rate_max;
	u16 base_rate_min16;
	u16 base_rate_max16;
	u16 base_rate_min32;
	u16 base_rate_max32;
	u8 sub_rate_min;
	u8 sub_rate_max;
	u8 enable_pmms;
	u8 pmms_margin;
	u8 rsvd0;
	u8 rsvd1;
	u8 rsvd2;
	u8 rsvd3;
	u8 octet_no_0;
	u8 octet_val_0;
	u8 octet_no_1;
	u8 octet_val_1;
	u8 octet_no_2;
	u8 octet_val_2;
	u8 octet_no_3;
	u8 octet_val_3;
	u8 octet_no_4;
	u8 octet_val_4;
	u8 octet_no_5;
	u8 octet_val_5;
	u8 octet_no_6;
	u8 octet_val_6;
	u8 octet_no_7;
	u8 octet_val_7;
	u8 octet_no_8;
	u8 octet_val_8;
	u8 octet_no_9;
	u8 octet_val_9;
	u8 octet_no_10;
	u8 octet_val_10;
	u8 octet_no_11;
	u8 octet_val_11;
	u8 octet_no_12;
	u8 octet_val_12;
	u8 octet_no_13;
	u8 octet_val_13;
	u8 octet_no_14;
	u8 octet_val_14;
	u8 octet_no_15;
	u8 octet_val_15;
};
struct cmd_cfg_sdi_tx {
	s32 data_shift;
	s8 frame_shift;
	u8 sp_level;
	u8 sp_sample_edg;
	u8 data_sample_edg;
	s32 lstwr_1strd_dly;
	u8 slip_mode;
	u8 rsvd1;
	u8 align;
	u8 rsvd3;
};
struct  cmd_cfg_sdi_rx {
	s32 data_shift;
	s8 frame_shift;
	u8 sp_level;
	u8 driving_edg;
	u8 data_shift_edg;
	s32 lstwr_1strd_dly;
	u8 slip_mode;
	u8 rsvd1;
	u8 align;
	u8 rsvd3;
};
struct cmd_cfg_ghs_extended_pam_mode {
	u8 ext_pam_mode;
	u8 bits_per_symbol;
	u16 speed_rate;
};
struct  cmd_tc_flowmodify{
    u32 dsl0_ts:8;
    u32 dsl1_ts:8;
	u32 dsl2_ts:8;
    u32 dsl3_ts:8;
};

struct cmd_hdlc_tc_link_linkmodify{
   u32 linkNo;// DSL0
   u32 bitdyte;
   u32 interframe_ch;
   u32 sharedflags;
   u32 fcs;
   u32 acf_insert;
   u32 txaddrctrl;
   u32 li_m_pairports;
   u32 clockingmode;
};

struct cmd_xmii_modify {

   u32 linkNo;
   u32 speed;
   u32 duplex;
   u32 smii_syncmode;
   u32 altcollision;
   u32 rxduringtx;
   u32 collisiontype;
};
struct cmd_pmd_control {

   u32 LinkNo;
   u32 LinkControl;
   u32 ActivationState;
};

#define MAIN_INIT      0
#define MAIN_PRE_ACT   1
#define MAIN_CORE_ACT  2
#define MAIN_DATA_MODE 3
#define MAIN_EXCEPTION 5
#define MAIN_TEST      6

#define CMD_CONNECT_CTRL 0x0c04

struct cmd_connect_ctrl
{
	u8 state;
	u8 rsvd1;
	u16 rsvd2;
};

struct cmd_cfg_sdi_settings
{
       u8 input_mode;
       u8 output_mode;
       u16 frequency;
       u16 payload_bits;
       u8 frames;
       u8 loop;
       u8 ext_clk8k;
       u8 dpll4bclk;
       u8 refclkin_freq;
       u8 refclkout_freq;
};

struct ack_dsl_param_get
{
	u8 stu_mode;
	u8 repeater;
	u8 annex;
	u8 clk_ref;
	u16 base_rate;
	u8 sub_rate;
	u8 psd_mask;
	u8 frame_mode;
	u8 rsvd2;
	u16 tx_sync_word;
	u16 rx_sync_word;
	u8 tx_stuff_bits;
	u8 rx_stuff_bits;
	s8 pow_backoff;
	s8 pow_backoff_farend;
	u8 ghs_pwr_lev_carr;
	u8 bits_p_symbol;
};

#define SDI_TDMCLK_TDMSP 1
#define SDI_TDMSP_TDMMSP 2
#define SDI_NO_LOOP         0
#define SDI_REMOTE_LOOP     1
#define SDI_REMOTE_CLK_ONLY 2
#define SDI_LOCAL_LOOP      3
#define SDI_NODPLL 0
#define SDI_DPLL4IN 1
#define TIM_REF_CLK_IN_8192KHZ 5
#define TIM_DATA_CLK_8192KHZ 15
#define CMD_CFG_SDI_SETTINGS 0x840f
#define CONNECTED 11

#define CMD_Dbg_SDI_Settings 0x75
#define CMD_Dbg_SDI_Tx 0x76
#define CMD_Dbg_SDI_Rx 0x77

void mam17_card_remove(struct mam17_card *card);
int __devinit mam17_init (void);
void __devexit mam17_exit (void);
irqreturn_t interrupt ( int  irq,  void  * dev_id,  struct pt_regs  * regs_ );
void dump(void);
int recv(u8 * msg, int len);
int xmit(u8 * msg, int len, u8 * ret);

int configure_channel(struct mam17_card *card, int ch);
void def_conf(struct mam17_card *card, int ch);
int load_cfg(struct mam17_card *card, int ch);
int get_statistic(u8 ch, struct mam17_card * card, struct statistics *stat);

void monitor(void *data);

#endif
