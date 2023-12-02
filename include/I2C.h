#ifndef I2C_HEADER
#define I2C_HEADER
#include <stdint.h>
#define I2C_NG	(-1)
#define I2C_OK	(0)

typedef enum _SEND_ORDER {
	ORDER_DO_NOT_CHANGE,
	ORDER_DO_CHANGE,
	ORDER_OPERATION_NUM
} SEND_ORDER;

#ifdef  __cplusplus
extern  "C" {
#endif
int32_t i2c_open(int32_t *p_fd, const char* dev_name);
int32_t i2c_read(int32_t *p_fd, uint8_t sla_addr, SEND_ORDER reg_order, uint8_t *p_reg, uint16_t reg_len, uint8_t *p_data, uint16_t data_len);
int32_t i2c_write(int32_t *p_fd, uint8_t sla_addr, SEND_ORDER reg_order, uint8_t *p_reg, uint16_t reg_len, SEND_ORDER data_order, uint8_t *p_data, uint16_t data_len);
void i2c_close(int32_t *p_fd);
#ifdef __cpluscplus
}
#endif
#endif
