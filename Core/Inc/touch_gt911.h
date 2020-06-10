#ifndef _GT911_H
#define _GT911_H

#define IIC_DelayValue  20
#define TOUCH_MAX  5

typedef struct
{
  uint8_t num;
  uint16_t X[TOUCH_MAX];
  uint16_t Y[TOUCH_MAX];
} TS_StateTypeDef;


#define Touch_IIC_RADDR         0xBB    // touch IIC addr
#define Touch_IIC_WADDR         0xBA
#define Touch_ID_ADDR           0x8140
#define Touch_READ_ADDR         0x814E

#define GT5688_CFG_ADDR         0x8050
#define GT5688_Chksum_ADDR 0x813C

#define GT9XX_CFG_ADDR     0x8047  // GT911
#define GT9XX_Chksum_ADDR  0X80FF  // GT911

#define Touch_IIC_SCL(a)	if (a) \
		HAL_GPIO_WritePin(TOUCH_SCL_GPIO_Port, TOUCH_SCL_Pin, GPIO_PIN_SET); \
	else \
		HAL_GPIO_WritePin(TOUCH_SCL_GPIO_Port, TOUCH_SCL_Pin, GPIO_PIN_RESET);

#define Touch_IIC_SDA(a)	if (a) \
		HAL_GPIO_WritePin(TOUCH_SDA_GPIO_Port, TOUCH_SDA_Pin, GPIO_PIN_SET); \
	else \
		HAL_GPIO_WritePin(TOUCH_SDA_GPIO_Port, TOUCH_SDA_Pin, GPIO_PIN_RESET);

uint8_t GT911_Init( void );
uint8_t GT911_GetState( TS_StateTypeDef* );
#endif
