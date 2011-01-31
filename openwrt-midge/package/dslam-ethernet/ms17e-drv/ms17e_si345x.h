#ifndef MS17E_SI245X_H
#define MS17E_SI245X_H

// registers
#define INT           0x00
#define INT_MASK      0x01
#define PORT1_EVENTS  0x02
#define PORT2_EVENTS  0x03
#define PORT3_EVENTS  0x04
#define PORT4_EVENTS  0x05
#define PORT1_STATUS  0x06
#define PORT2_STATUS  0x07
#define PORT3_STATUS  0x08
#define PORT4_STATUS  0x09
#define PORT1_CONFIG  0x0A
#define PORT2_CONFIG  0x0B
#define PORT3_CONFIG  0x0C
#define PORT4_CONFIG  0x0D
#define PORT1_I_CUT   0x0E
#define PORT2_I_CUT   0x0F
#define PORT3_I_CUT   0x10
#define PORT4_I_CUT   0x11
#define COMMAND_REG   0x12
#define VEE_MSB       0x13
#define VEE_LSB       0x14
#define CUR_P1_MSB    0x15
#define CUR_P1_LSB    0x16
#define CUR_P2_MSB    0x17
#define CUR_P2_LSB    0x18
#define CUR_P3_MSB    0x19
#define CUR_P3_LSB    0x1A
#define CUR_P4_MSB    0x1B
#define CUR_P4_LSB    0x1C
#define DEVICE_STATUS 0x1D
#define HW_REV        0x60
#define FIRMWARE_REV1 0x61
#define FIRMWARE_REV2 0x62
#define FIRMWARE_REV3 0x63

// defines for chip commands
#define CMD_PORT_POWER_ON  0x04 // CMD_PORT_POWER_ON | port_num
#define CMD_PORT_POWER_OFF 0x08 // CMD_PORT_POWER_OFF | port_num
#define CMD_PORT_RESET     0x0C // CMD_PORT_RESET | port_num
#define CMD_TOGGLE_DETECTION_BACKOFF_TIMING  0x10
#define CMD_RESET_CHIP     0x14
#define CMD_GET_VEE        0x18
#define CMD_GET_CURRENT    0x1C // CMD_GET_CURRENT | port_num
#define CMD_OFF_LOW_PORTS  0x20

#endif
