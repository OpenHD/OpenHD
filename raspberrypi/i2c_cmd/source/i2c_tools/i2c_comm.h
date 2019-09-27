
#ifndef _I2C_COMM_H
#define _I2C_COMM_H

#define LOCALFUNC
#define HI_RET int
#define IN
#define OUT
#define U32 unsigned int
#define U8 unsigned char
#define CHAR char
#define UCHAR unsigned char
#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t unsigned char

#define HI_SUCCESS 0
#define HI_FAILURE (-1)


struct sensor_regs {
	uint16_t reg;
	uint16_t data;
};

HI_RET StrToNumber(IN CHAR *str , OUT U32 * pulValue);

#endif

