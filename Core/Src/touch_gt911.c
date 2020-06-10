/*
 * low level driver for the touch screen Goodix GT911
 * four pins:
 * INT for signaling when data (low active)
 * RST to reset, (when low)
 * SCL/SDA for a plain vanila I2C slave
 * sadly the board i'm using is bit-banging I2C transaction!
 */
#include <stdio.h>
#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "main.h"
#include "touch_gt911.h"

#if 0
// no need for EXTI IRQ callback for TOUCH_INT as i can poll easily the value at TouchGfx refresh (50/60 Hertz)
void HAL_GPIO_EXTI_Callback( uint16_t pin )
{
	if (pin == GPIO_PIN_15)
		printf(".");
}
#endif

/****************************************************
 * Low level fn related to I2C bus bit banging!
 */
void Touch_IIC_Delay(uint32_t a)
{
#if 0
// no active loop as we are on freeRtos! save resources
	while (a --)
		osDelay(1);
#endif
	int i;
	while (a --)
	{
		for (i = 0; i < 10; i++);
	}
}

void Touch_IIC_Start(void)
{
	Touch_IIC_SDA(1);
	Touch_IIC_SCL(1);
	Touch_IIC_Delay(IIC_DelayValue);

	Touch_IIC_SDA(0);
	Touch_IIC_Delay(IIC_DelayValue);
	Touch_IIC_SCL(0);
	Touch_IIC_Delay(IIC_DelayValue);
}
void Touch_IIC_Stop(void)
{
	Touch_IIC_SCL(0);
	Touch_IIC_Delay(IIC_DelayValue);
	Touch_IIC_SDA(0);
	Touch_IIC_Delay(IIC_DelayValue);

	Touch_IIC_SCL(1);
	Touch_IIC_Delay(IIC_DelayValue);
	Touch_IIC_SDA(1);
	Touch_IIC_Delay(IIC_DelayValue);
}
uint8_t Touch_IIC_WaitACK(void)
{
	Touch_IIC_SDA(1);
    Touch_IIC_Delay(IIC_DelayValue);
    Touch_IIC_SCL(1);
    Touch_IIC_Delay(IIC_DelayValue);
    Touch_IIC_SCL(0);
    return (HAL_GPIO_ReadPin(TOUCH_SDA_GPIO_Port, TOUCH_SDA_Pin) != GPIO_PIN_RESET) ? ERROR : SUCCESS;
}

uint8_t Touch_IIC_WriteByte(uint8_t IIC_Data)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		Touch_IIC_SDA(IIC_Data & 0x80);
        Touch_IIC_Delay( IIC_DelayValue );
        Touch_IIC_SCL(1);
        Touch_IIC_Delay( IIC_DelayValue );
        Touch_IIC_SCL(0);
        if(i == 7) {
        	Touch_IIC_SDA(1);
        }
        IIC_Data <<= 1;
	}
	return Touch_IIC_WaitACK();
}

void Touch_IIC_ACK(void) {
	Touch_IIC_SCL(0);
	Touch_IIC_Delay(IIC_DelayValue);
	Touch_IIC_SDA(0);
	Touch_IIC_Delay(IIC_DelayValue);
	Touch_IIC_SCL(1);
	Touch_IIC_Delay(IIC_DelayValue);

	Touch_IIC_SCL(0);
	Touch_IIC_SDA(1);

	Touch_IIC_Delay(IIC_DelayValue);
}

void Touch_IIC_NoACK(void)
{
	Touch_IIC_SCL(0);
	Touch_IIC_Delay(IIC_DelayValue);
	Touch_IIC_SDA(1);
	Touch_IIC_Delay(IIC_DelayValue);
	Touch_IIC_SCL(1);
	Touch_IIC_Delay(IIC_DelayValue);

	Touch_IIC_SCL(0);
	Touch_IIC_Delay(IIC_DelayValue);
}

uint8_t Touch_IIC_ReadByte(uint8_t ACK_Mode) {
	uint8_t IIC_Data = 0;
	uint8_t i = 0;

	for (i = 0; i < 8; i++) {
		IIC_Data <<= 1;
		Touch_IIC_SCL(1);
		Touch_IIC_Delay( IIC_DelayValue );
		IIC_Data |= (HAL_GPIO_ReadPin(TOUCH_SDA_GPIO_Port, TOUCH_SDA_Pin) != GPIO_PIN_RESET) ? 0x01 : 0x00;
		Touch_IIC_SCL(0);
		Touch_IIC_Delay( IIC_DelayValue );
	}

	if ( ACK_Mode == 1 )
		Touch_IIC_ACK();
	else
		Touch_IIC_NoACK();

	return IIC_Data;
}
/****************************************************/

void Touch_Reset(void)
{
    //Touch_INT_Out(); // no need to change dir as always out can read too!
	HAL_GPIO_WritePin(TOUCH_INT_GPIO_Port, TOUCH_INT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(TOUCH_RST_GPIO_Port, TOUCH_RST_Pin, GPIO_PIN_SET);
	Touch_IIC_Delay(40000);

    //      INT to make the other I2C address 0XBA/0XBB
	HAL_GPIO_WritePin(TOUCH_RST_GPIO_Port, TOUCH_RST_Pin, GPIO_PIN_RESET);
    Touch_IIC_Delay(1000000);
    HAL_GPIO_WritePin(TOUCH_RST_GPIO_Port, TOUCH_RST_Pin, GPIO_PIN_SET);
    Touch_IIC_Delay(350000);
    //Touch_INT_In(); // it's out opendrain but can read the value, nevertheless!
    Touch_IIC_Delay(350000);
}

uint8_t Touch_WriteHandle (uint16_t addr)
{
	uint8_t status = ERROR;
	Touch_IIC_Start();
	if( Touch_IIC_WriteByte(Touch_IIC_WADDR) == SUCCESS)
		if( Touch_IIC_WriteByte((uint8_t)(addr >> 8)) == SUCCESS)
			if( Touch_IIC_WriteByte((uint8_t)(addr)) == SUCCESS )
				status = SUCCESS;
	return status;
}


uint8_t Touch_WriteData (uint16_t addr, uint8_t value)
{
	uint8_t status = ERROR;
	Touch_IIC_Start();

	if( Touch_WriteHandle(addr) == SUCCESS)
		if (Touch_IIC_WriteByte(value) == SUCCESS)
			status = SUCCESS;
	Touch_IIC_Stop();
	return status;
}

uint8_t Touch_WriteReg (uint16_t addr, uint8_t cnt, uint8_t *value)
{
	uint8_t status;
	uint8_t i;

	Touch_IIC_Start();

	if( Touch_WriteHandle(addr) == SUCCESS) {
		for(i = 0 ; i < cnt; i++)
			Touch_IIC_WriteByte(value[i]);
		Touch_IIC_Stop();
		status = SUCCESS;
	} else {
		Touch_IIC_Stop();
		status = ERROR;
	}
	return status;
}

uint8_t Touch_ReadReg(uint16_t addr, uint8_t cnt, uint8_t *value) {
	uint8_t status = ERROR;
	uint8_t i;
	Touch_IIC_Start();

	if( Touch_WriteHandle(addr) == SUCCESS) {
		Touch_IIC_Start();

		if (Touch_IIC_WriteByte(Touch_IIC_RADDR) == SUCCESS) {
			for(i = 0 ; i < cnt; i++) {
				if (i == (cnt - 1))
					value[i] = Touch_IIC_ReadByte(0);
				else
                	value[i] = Touch_IIC_ReadByte(1);
			}
			Touch_IIC_Stop();
			status = SUCCESS;
		}
	}
	Touch_IIC_Stop();
	return (status);
}

uint8_t GT911_Init(void) {
	uint8_t Touch_ID[4];
	uint8_t Touch_CFG[5];

	//Touch_IIC_GPIO_Config();
	Touch_Reset();
	Touch_ReadReg (Touch_ID_ADDR,4,Touch_ID);

	if( Touch_ID[0] == '5' ) {
		Touch_ReadReg (GT5688_CFG_ADDR,5,Touch_CFG);

		printf("Touch ID: GT%.4s \r\n",Touch_ID);
		printf("IC config version: 0X%.2x \r\n",Touch_CFG[0]);
		printf("Resolution: %d * %d\r\n",(Touch_CFG[2]<<8) + Touch_CFG[1],(Touch_CFG[4]<<8) +Touch_CFG[3]);

		return SUCCESS;
	} else
	if( Touch_ID[0] == '9') {
		Touch_ReadReg (GT9XX_CFG_ADDR,5,Touch_CFG);
		printf("Touch ID: GT%.4s \r\n",Touch_ID);
		printf("IC config version: 0X%.2x \r\n",Touch_CFG[0]);
		printf("Resolution: %d * %d\r\n",(Touch_CFG[2]<<8) + Touch_CFG[1],(Touch_CFG[4]<<8) +Touch_CFG[3]);
		return SUCCESS;
	} else
	{
		printf("Touch Error\r\n");
		return ERROR;
	}
}

// filter out the second touch event (release) on the same cohordinates
uint16_t prevX, prevY;
uint8_t GT911_GetState( TS_StateTypeDef* state )
{

    uint8_t touchData[2 + 8 * TOUCH_MAX ];
    uint8_t i;
	if (!state)
		return 0;
	Touch_ReadReg( Touch_READ_ADDR, sizeof(touchData) ,touchData);
    Touch_WriteData( Touch_READ_ADDR,0);
    state->num = touchData[0] & 0x0f;

    if ((state->num >= 1) && (state->num <=TOUCH_MAX)) {
    	for( i=0; i< state->num; i++) {
    		state->Y[i] = (touchData[5+8*i]<<8) | touchData[4+8*i];
    		state->X[i] = (touchData[3+8*i]<<8) | touchData[2+8*i];
    	}
        // HACK to filter out second same X/Y event (release event?)
        if (state->X[0] == prevX && state->Y[0] == prevY)
        	state->num = 0;
        prevX = state->X[0];
        prevY = state->Y[0];
    } else
    	state->num = 0;

    return state->num;
}


