#ifndef SG_PI_H
#define SG_PI_H

#include "mam17h_main.h"

#define CMD_DSL_PARAM_GET 0x4802
#define EVT_PMD_MultiWireMapping 0x691
#define EVT_PMD_MpairStatus 0x692
#define EVT_PMD_Channel_0_Down 0x6C0
#define ALM_DelayMeasurementAborted 0x60A
#define EVT_PMD_DelayCompState 0x693

typedef struct message_t_ {
	unsigned LENGTH : 5;
	unsigned MSGID : 11;
//	unsigned ID : 7;
//	unsigned CAT : 2;
//	unsigned TYPE : 2;
	unsigned TCID : 11;
	unsigned RC : 4;
	unsigned M : 1;
} message_t;

typedef struct ack_t_ {
	int len;
	u32 buf32[32];
} ack_t;

void led_off(struct mam17_card * card, int ch);
void led_blink(struct mam17_card * card, int ch);
void led_fast_blink(struct mam17_card * card, int ch);
void led_on(struct mam17_card * card, int ch);
int mpi_recv(struct mam17_card *card);
int mpi_cmd(struct mam17_card *card, u16 opcode, u32 * payload, int plen, ack_t *ack);
int tunnel_cmd(struct mam17_card *card, u16 opcode, u32 * payload, int plen, int dfe_num, ack_t * ack);

extern int ack_wait;

#endif
