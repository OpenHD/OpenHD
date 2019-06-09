

#include <ctype.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "i2c_comm.h"

#define I2C_DEVICE_NAME_LEN 13	// "/dev/i2c-XXX"+NULL
static char i2c_device_name[I2C_DEVICE_NAME_LEN];

static int i2c_rd(int fd, uint8_t i2c_addr, uint16_t reg, uint8_t *values, uint32_t n)
{
	int err;
	int i = 0;
	uint8_t buf[2] = { reg >> 8, reg & 0xff };
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[2] = {
		{
			 .addr = i2c_addr,
			 .flags = 0,
			 .len = 2,
			 .buf = buf,
		},
		{
			.addr = i2c_addr,
			.flags = I2C_M_RD,
			.len = n,
			.buf = values,
		},
	};

	msgset.msgs = msgs;
	msgset.nmsgs = 2;

	err = ioctl(fd, I2C_RDWR, &msgset);
	printf("Read i2c addr %02X\n", i2c_addr);
	if (err != msgset.nmsgs)
		return -1;
	for(i = 0; i < n;++i)
	{
		printf("addr %04x : value %02x \n",reg+i,values[i]);
	}
	
	return 0;
}


int main(int argc, char *argv[])
{
	U32 I2C_port;
	U32 device_addr;
	U32 reg_addr;
	U32 new_data;
	U8 value[100];
	U32 num = 1;
	int result;
	if (argc < 4)
	{
		printf("usage: %s <bus_num> <device address(8bit)> <register address(16bits)> <len default:1 max100>. sample: %s 0x0 0xA0 0x1000 0x2\n", argv[0], argv[0]);
		return -1;
	}

	if(StrToNumber(argv[1], &I2C_port) != HI_SUCCESS ) {
		printf("Please input i2c port like 0x100 or 256.\r\n");
		return -1;
	}

	if(StrToNumber(argv[2], &device_addr) != HI_SUCCESS ) {
		printf("Please input dev addr like 0x100 or 256.\r\n");
		return -1;
	}

	if(StrToNumber(argv[3], &reg_addr) != HI_SUCCESS ) {
		printf("Please input reg addr like 0x100 0r 256.\r\n");
		return -1;
	}
	if(argc >=5)
	{
		if(StrToNumber(argv[4], &num) != HI_SUCCESS ) {
			printf("Please input reg addr like 0x100 0r 256.\r\n");
			return -1;
		}
	}
	snprintf(i2c_device_name, sizeof(i2c_device_name), "/dev/i2c-%d", I2C_port);
	printf("Using i2C device %s\n", i2c_device_name);
	
	printf("====I2C read:<%#x> <%#x> <%#x>====\n", device_addr, reg_addr, num);
	
	int fd;
	fd = open(i2c_device_name, O_RDWR);
	if (!fd)
	{
		printf("Couldn't open I2C device\n");
		return -1;
	}
	if (ioctl(fd, I2C_SLAVE_FORCE, device_addr) < 0)
	{
		printf("Failed to set I2C address\n");
		return -1;
	}

	
	i2c_rd(fd, device_addr,reg_addr, &value, num);
	
	close(fd);
	
	
	return value[0];
	
}

