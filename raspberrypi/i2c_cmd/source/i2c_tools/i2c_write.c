
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

void send_regs(int fd,  const struct sensor_regs *regs, int num_regs)
{
	int i;
	for (i=0; i<num_regs; i++)
	{
		if (regs[i].reg == 0xFFFF)
		{
			if (ioctl(fd, I2C_SLAVE_FORCE, regs[i].data) < 0)
			{
				printf("Failed to set I2C address to %02X", regs[i].data);
			}
		}
		else if (regs[i].reg == 0xFFFE)
		{
			usleep(regs[i].data);
		}
		else
		{
			unsigned char msg[4] = {regs[i].reg>>8, regs[i].reg, regs[i].data};
			int len = 3;

			if (write(fd, msg, len) != len)
			{
				printf("Failed to write register index %d", i);
			}
			
		}
	}
}

int main(int argc, char *argv[])
{
	U32 I2C_port;
	U32 device_addr;
	U32 reg_addr;
	U32 new_data;
	U8 value;
	struct sensor_regs regs;
	
	int result;
	if (argc < 5)
	{
		printf("usage: %s <bus_num> <device address 8bits> <register address 16bits> <value>. sample: %s 0x0 0xA0 0x10 0x40\n", argv[0], argv[0]);
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

	if(StrToNumber(argv[4], &new_data) != HI_SUCCESS ) {
		printf("Please input len like 0x100\n");
		return -1;
	}
	value = (U8)new_data;
	printf("====I2C write:<%#x> <%#x> <%#x>====\n", device_addr, reg_addr, new_data);
	
	snprintf(i2c_device_name, sizeof(i2c_device_name), "/dev/i2c-%d", I2C_port);
	//printf("Using i2C device %s\n", i2c_device_name);
	
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
	regs.reg = reg_addr;
	regs.data = new_data;
	
	send_regs(fd, &regs, 1);
	close(fd);
	
	return 0;

}

