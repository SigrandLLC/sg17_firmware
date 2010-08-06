#ifndef MAM_17H_SOCKRATE_H
#define MAM_17H_SOCKRATE_H

#include <asm/types.h>
#include "mam17h_main.h"
#include "mam17h_mpair.h"


// firmware defines
#define FW_DFE_NAME "dfe.bin"//"mam17h_dfe.bin"//"mr17_dfe.bin"//
#define FW_IDC_NAME "idc.bin"//"mam17h_idc.bin"//"mr17_idc.bin"//
#define PKG_SIZE 32

#define FW_DFE_CODE_OFFSET 0x0
#define FW_DFE_DATA_OFFSET 0x24000
#define FW_DFE_CODE_SIZE   0x24000
#define FW_DFE_DATA_SIZE   0xC000
#define FW_DFE_CRC_OFFSET  0x30000

#define FW_IDC_CODE_OFFSET 0x40
#define FW_IDC_DATA_OFFSET 0x24040
#define FW_IDC_CODE_SIZE   0x24000
#define FW_IDC_DATA_SIZE   0x8000
#define FW_IDC_CRC_OFFSET  0x2C040 + 4



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

int mam17_socrate_init(struct mam17_card *card);
int sdfe4_download_DFE_fw(u8 * fw, struct mam17_card *card);
int sdfe4_download_IDC_fw(u8 * fw, struct mam17_card *card);

struct regs_str {
// micriprocessor interface registers
	u8 uu1[8];
	u8 MPI_CON, MPI_EGRESS, MPI_INGRESS;
	u8 MPI_EFSTAT, MPI_EINT, MPI_EINT_EN, MPI_IINT_EN, MPI_IINT, SCI_INGRESS, SCI_EGRESS;
	u8 SCI_CTRL_L, SCI_CTRL_H, SCI_CFG_L, SCI_CFG_H, SCI_REPORT_L, SCI_REPORT_H, SCI_ACFG0, SCI_ACFG1, SCI_ACFG2, SCI_ACFG3;
	u8 uu2[1];
	u8 SCI_CLKCFG, SCI_INTEN, SCI_INT;
// mr17bh4 registers
	u8 CRA, SR, PWRR0, PWRR1, CRB0, CRB1, CRB2, CRB3;
};

struct sdfe4_ret {
	u32 l;
	u8 val[4*SDFE4_FIFO32];
};




#endif


