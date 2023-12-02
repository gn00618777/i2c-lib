/*

Copyright @ 2023 Bernie Chen
Permission is hereby granted, free of charge, to any persion obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit person whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS, OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PURTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "I2C.h"

#define TAG			"I2C_LIB"
#define LERR(format, ...)	fprintf(stderr, "[%s] " format, TAG, ##__VA_ARGS__)
#define LWAR(format, ...)	fprintf(stdout, "[%s] " format, TAG, ##__VA_ARGS__)

#define MAX_WRITE_LEN		(8)

#define I2C_READ_RETRY		(3)
#define I2C_WRITE_RETRY		(3)

#define I2C_GET_7BIT_ADDR(x)	(x >>= 1)

//static pthread_mutex_t i2c_mutex = PTHREAD_MUTEX_INITIALIZER;

int32_t i2c_open(int32_t *p_fd, const char *p_dev)
{
	int32_t fd = -1;

	if(!p_fd) {
		LERR("p_fd is NULL\n");
		return I2C_NG;
	}

	if(*p_fd != -1) {
		LERR("fd is opened before\n");
		return I2C_NG;
	}

	if(!p_dev) {
		LERR("p_dev is NULL\n");
		return I2C_NG;
	}

	fd = open(p_dev, O_RDWR);
	if(fd < 0) {
		perror("I2C open:");
		return I2C_NG;
	}

	return I2C_OK;
}

int32_t i2c_read(int32_t *p_fd, uint8_t sla_addr, SEND_ORDER reg_order, uint8_t *p_reg,
			uint16_t reg_len, uint8_t *p_data, uint16_t data_len)
{
	int32_t try = 0, i = 0, j = 0;
	struct i2c_msg messages[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	unsigned char buffer[MAX_WRITE_LEN];
	unsigned char temp = 0;

	memset(messages, 0, sizeof(messages));
	memset(&ioctl_data, 0, sizeof(ioctl_data));
	memset(buffer, 0, sizeof(buffer));

	if(!p_fd) {
		LERR("p_fd is NULL\n");
		return I2C_NG;
	}

	if(!p_reg) {
		LERR("p_reg is NULL\n");
		return I2C_NG;
	}

	if(reg_order >= ORDER_OPERATION_NUM) {
		LERR("argument of order is invalid\n");
		return I2C_NG;
	}

	if(!p_data) {
		LERR("p_data is invalid\n");
		return I2C_NG;
	}

	if(reg_len == 0 || data_len == 0) {
		LERR("length of reg and data can't be zero\n");
		return I2C_NG;
	}

	if((reg_len + data_len) >= MAX_WRITE_LEN) {
		LERR("sum of reg length and data length exceeds limit:%d\n", MAX_WRITE_LEN);
		return I2C_NG;
	}

	I2C_GET_7BIT_ADDR(sla_addr);

	memcpy(buffer, p_reg, reg_len);

	if(reg_order == ORDER_DO_CHANGE) {
		for(i = 0, j = reg_len - 1 ; i < j ; ++i, --j) {
			temp = buffer[i];
			buffer[i] = buffer[j];
			buffer[j] = temp;
		}
	}

	//send register address reading
	messages[0].addr = sla_addr;
	messages[0].flags = 0;		//write
	messages[0].len = reg_len;
	messages[0].buf = p_reg;

	messages[1].addr = sla_addr;
	messages[1].flags = I2C_M_RD;
	messages[1].len = data_len;
	messages[1].buf = p_data;

	ioctl_data.nmsgs = 2;
	ioctl_data.msgs = messages;

	for(try = 0 ; try < I2C_READ_RETRY ; try++) {
		if(ioctl(*p_fd, I2C_RDWR, &ioctl_data) < 0) {
			perror("I2C read:");
			return I2C_NG;
		} else {
			break;
		}
	}

	return I2C_OK;
}

int32_t i2c_write(int32_t *p_fd, uint8_t sla_addr, SEND_ORDER reg_order, uint8_t *p_reg,
			uint16_t reg_len, SEND_ORDER data_order, uint8_t *p_data, uint16_t data_len)
{
	int32_t try = 0, i = 0, j = 0;
	struct i2c_msg messages[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	unsigned char buffer[MAX_WRITE_LEN];
	unsigned char temp = 0;

	memset(messages, 0, sizeof(messages));
        memset(&ioctl_data, 0, sizeof(ioctl_data));
	memset(buffer, 0, sizeof(buffer));

	if(!p_fd < 0) {
		LERR("p_fd is NULL\n");
		return I2C_NG;
	}

	if(!p_reg) {
		LERR("p_reg is NULL\n");
		return I2C_NG;
	}

	if(!p_data) {
		LERR("p_data is NULL\n");
		return I2C_NG;
	}

	if(reg_order >= ORDER_OPERATION_NUM || data_order >= ORDER_OPERATION_NUM) {
		LERR("argument of order is invalid\n");
                return I2C_NG;
	}

	if(reg_len == 0 || data_len == 0) {
		LERR("length of reg and data can't be zero\n");
		return I2C_NG;
	}

	if((reg_len + data_len) >= MAX_WRITE_LEN) {
		LERR("sum of reg length and data length exceeds limit:%d\n", MAX_WRITE_LEN);
		return I2C_NG;
	}

	I2C_GET_7BIT_ADDR(sla_addr);

	memcpy(buffer, p_reg, reg_len);

	if(reg_order == ORDER_DO_CHANGE) {
		for(i = 0, j = reg_len - 1 ; i < j ; ++i, --j) {
			temp = buffer[i];
			buffer[i] = buffer[j];
			buffer[j] = temp;
		}
	}

	memcpy(buffer + reg_len, p_data, data_len);

	if(data_order == ORDER_DO_CHANGE) {
		for(i = reg_len, j = (reg_len + data_len) - 1 ; i < j ; ++i, --j) {
			temp = buffer[i];
			buffer[i] = buffer[j];
			buffer[j] = temp;
		}
	}


	//send register address writing
	messages[0].addr = sla_addr;
	messages[0].flags = 0;          //write
	messages[0].len = reg_len + data_len;
	messages[0].buf = buffer;

	ioctl_data.nmsgs = 1;
	ioctl_data.msgs = messages;

	for(try = 0 ; try < I2C_WRITE_RETRY ; try++) {
		if(ioctl(*p_fd, I2C_RDWR, &ioctl_data) < 0) {
			perror("I2C write:");
			return I2C_NG;
		} else {
			break;
		}
	}

	return I2C_OK;
}

void i2c_close(int32_t *p_fd)
{
	if(!p_fd) {
		LERR("p_fd is NULL\n");
		return;
	}

	close(*p_fd);

	return;
}
