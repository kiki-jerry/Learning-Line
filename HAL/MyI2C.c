#include "stm32f1xx_hal.h"
#include "gpio.h"

void MyI2C_W_SCL(GPIO_PinState x)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, x);
//	Delay_us(10);
}

void MyI2C_W_SDA(GPIO_PinState x)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, x);
//	Delay_us(10);
}

uint8_t MyI2C_R_SDA(void)
{
	uint8_t BitValue;
	BitValue = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3);
//	Delay_us(10);
	return BitValue;
}

void MyI2C_Init(void)
{
	MX_GPIO_Init();
	
	MyI2C_W_SCL(GPIO_PIN_SET);
	MyI2C_W_SDA(GPIO_PIN_SET);
}

void MyI2C_Start(void)
{
	MyI2C_W_SDA(GPIO_PIN_SET);
	MyI2C_W_SCL(GPIO_PIN_SET);
	MyI2C_W_SDA(GPIO_PIN_RESET);
	MyI2C_W_SCL(GPIO_PIN_RESET);
}

void MyI2C_Stop(void)
{
	MyI2C_W_SDA(GPIO_PIN_RESET);
	MyI2C_W_SCL(GPIO_PIN_SET);
	MyI2C_W_SDA(GPIO_PIN_SET);
}

void MyI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i ++)
	{
		MyI2C_W_SDA( ((Byte & (0x80 >> i)) != 0) ? GPIO_PIN_SET : GPIO_PIN_RESET );
		MyI2C_W_SCL(GPIO_PIN_SET);
		MyI2C_W_SCL(GPIO_PIN_RESET);
	}
}

uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;
	MyI2C_W_SDA(GPIO_PIN_SET);
	for (i = 0; i < 8; i ++)
	{
		MyI2C_W_SCL(GPIO_PIN_SET);
		if (MyI2C_R_SDA()){Byte |= (0x80 >> i);}
		MyI2C_W_SCL(GPIO_PIN_RESET);
	}
	return Byte;
}

void MyI2C_SendAck(uint8_t AckBit)
{
	MyI2C_W_SDA( (AckBit == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET );
	MyI2C_W_SCL(GPIO_PIN_SET);
	MyI2C_W_SCL(GPIO_PIN_RESET);
}

uint8_t MyI2C_ReceiveAck(void)
{
	uint8_t AckBit;
	MyI2C_W_SDA(GPIO_PIN_SET);
	MyI2C_W_SCL(GPIO_PIN_SET);
	AckBit = MyI2C_R_SDA();
	MyI2C_W_SCL(GPIO_PIN_RESET);
	return AckBit;
}
