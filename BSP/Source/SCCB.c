#include "SCCB.h"

void SCCBInitialize(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStruct;
	
	/* 使能I2C时钟 */
	RCC_APB1PeriphClockCmd(SCCB_I2C_CLK, ENABLE);
	
	/* 使能I2C使用的GPIO时钟 */
	RCC_AHB1PeriphClockCmd(SCCB_I2C_SCL_GPIO_CLK|SCCB_I2C_SDA_GPIO_CLK, ENABLE);

	/* 初始化GPIO */
	GPIO_InitStructure.GPIO_Pin = SCCB_I2C_SCL_PIN ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(SCCB_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SCCB_I2C_SDA_PIN ;
	GPIO_Init(SCCB_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	/* 配置引脚源 */
	GPIO_PinAFConfig(SCCB_I2C_SCL_GPIO_PORT, SCCB_I2C_SCL_SOURCE, SCCB_I2C_SCL_AF);
	GPIO_PinAFConfig(SCCB_I2C_SDA_GPIO_PORT, SCCB_I2C_SDA_SOURCE, SCCB_I2C_SDA_AF);

	/*初始化I2C模式 */
	I2C_DeInit(SCCB_I2C); 

	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0xFE;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = 200000;

	/* 写入配置 */
	I2C_Init(SCCB_I2C, &I2C_InitStruct);

	/* 使能I2C */
	I2C_Cmd(SCCB_I2C, ENABLE);

	Delay(50);
}

uint8_t SCCBWriteReg(uint8_t DeviceAddr, uint8_t RegAddr, uint8_t RegVal){
	uint32_t timeout = 1000;

	/* Generate the Start Condition */
	I2C_GenerateSTART(SCCB_I2C, ENABLE);

	/* Test on SCCB_I2C EV5 and clear it */
	timeout = 10000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		/* If the timeout delay is exeeded, exit with error code */
		if ((timeout--) == 0) return 0x01;
	}

	/* Send DCMI selcted device slave Address for write */
	I2C_Send7bitAddress(SCCB_I2C, DeviceAddr, I2C_Direction_Transmitter);

	/* Test on SCCB_I2C EV6 and clear it */
	timeout = 10000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		/* If the timeout delay is exeeded, exit with error code */
		if ((timeout--) == 0) return 0x02;
	}

	/* Send SCCB_I2C location address LSB */
	I2C_SendData(SCCB_I2C, (uint8_t)(RegAddr));

	/* Test on SCCB_I2C EV8 and clear it */
	timeout = 10000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		/* If the timeout delay is exeeded, exit with error code */
		if ((timeout--) == 0) return 0x03;
	}

	/* Send Data */
	I2C_SendData(SCCB_I2C, RegVal);

	/* Test on SCCB_I2C EV8 and clear it */
	timeout = 10000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		/* If the timeout delay is exeeded, exit with error code */
		if ((timeout--) == 0) return 0x04;
	}  

	/* Send SCCB_I2C STOP Condition */
	I2C_GenerateSTOP(SCCB_I2C, ENABLE);
	while(I2C_GetFlagStatus(SCCB_I2C, I2C_FLAG_BUSY))
	{
		if ((timeout--) == 0) return 0x05;
	}
	
	/* If operation is OK, return 0 */
	return 0;
}

uint8_t SCCBReadReg(uint8_t DeviceAddr, uint8_t RegAddr, uint8_t * RegVal){
	uint32_t timeout = 1000;
	
	while(I2C_GetFlagStatus(SCCB_I2C, I2C_FLAG_BUSY))
	{
		if ((timeout--) == 0) return 0x01;
	}
	
	/* Generate the Start Condition */
	I2C_GenerateSTART(SCCB_I2C, ENABLE);

	/* Test on SCCB_I2C EV5 and clear it */
	timeout = 1000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if ((timeout--) == 0) return 0x02;
	}

	/* Send DCMI selcted device slave Address for write */
	I2C_Send7bitAddress(SCCB_I2C, DeviceAddr, I2C_Direction_Transmitter);

	/* Test on SCCB_I2C EV6 and clear it */
	timeout = 1000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		/* If the timeout delay is exeeded, exit with error code */
		if ((timeout--) == 0) return 0x03;
	}

	/* Send SCCB_I2C location address LSB */
	I2C_SendData(SCCB_I2C, RegAddr);

	/* Test on SCCB_I2C EV8 and clear it */
	timeout = 1000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		/* If the timeout delay is exeeded, exit with error code */
		if ((timeout--) == 0) return 0x04;
	}

	I2C_GenerateSTOP(SCCB_I2C, ENABLE);
	while(I2C_GetFlagStatus(SCCB_I2C, I2C_FLAG_BUSY))
	{
		if ((timeout--) == 0) return 0x05;
	}

	/* Generate the Start Condition */
	I2C_GenerateSTART(SCCB_I2C, ENABLE);

	/* Test on SCCB_I2C EV6 and clear it */
	timeout = 1000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		/* If the timeout delay is exeeded, exit with error code */
		if ((timeout--) == 0) return 0x06;
	} 

	/* Send DCMI selcted device slave Address for write */
	I2C_Send7bitAddress(SCCB_I2C, DeviceAddr, I2C_Direction_Receiver);

	/* Test on SCCB_I2C EV6 and clear it */
	timeout = 1000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
		if ((timeout--) == 0) return 0x07;
	}

	/* Prepare an NACK for the next data received */
	I2C_AcknowledgeConfig(SCCB_I2C, DISABLE);

	/* Test on SCCB_I2C EV7 and clear it */
	timeout = 1000; /* Initialize timeout value */
	while(!I2C_CheckEvent(SCCB_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
		if ((timeout--) == 0) return 0x08;
	}

	/* Prepare Stop after receiving data */
	I2C_GenerateSTOP(SCCB_I2C, ENABLE);

	/* Receive the Data */
	*RegVal = I2C_ReceiveData(SCCB_I2C);
	
	return 0x00;
}
