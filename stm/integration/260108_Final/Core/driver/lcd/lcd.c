/*
 * lcd.c
 *
 *  Created on: Dec 17, 2025
 *      Author: rhoblack
 */
#include "lcd.h"

uint8_t lcdData = 0;
I2C_HandleTypeDef *hLcdI2C;

void LCD_CmdMode()
{
	lcdData &= ~(1<<LCD_RS);
}

void LCD_DataMode()
{
	lcdData |= (1<<LCD_RS);
}

void LCD_WriteMode()
{
	lcdData &= ~(1<<LCD_RW);
}

void LCD_SendData(uint8_t data)
{
	// send data to I2C interface
	HAL_I2C_Master_Transmit(hLcdI2C, 0x27<<1, &data, 1, 1000);
}

void LCD_E_High()
{
	lcdData |= (1<<LCD_E);
	LCD_SendData(lcdData);
	//HAL_Delay(1);
}

void LCD_E_Low()
{
	lcdData &= ~(1<<LCD_E);
	LCD_SendData(lcdData);
	//HAL_Delay(1);
}

void LCD_WriteNibble(uint8_t data)
{
	LCD_E_High();
	lcdData = (lcdData & 0x0f) | (data & 0xf0);
	LCD_SendData(lcdData);
	LCD_E_Low();
}

void LCD_WriteByte(uint8_t data)
{
	LCD_WriteNibble(data);
	data <<= 4;
	LCD_WriteNibble(data);
}

void LCD_WriteCmdData(uint8_t data)
{
	LCD_CmdMode();
	LCD_WriteMode();
	LCD_WriteByte(data);
}

void LCD_WriteCharData(uint8_t data)
{
	LCD_DataMode();
	LCD_WriteMode();
	LCD_WriteByte(data);
}

void LCD_BackLightOn()
{
	lcdData |= (1<<LCD_BL);
	LCD_WriteByte(lcdData);
}

void LCD_BackLightOff()
{
	lcdData &= ~(1<<LCD_BL);
	LCD_WriteByte(lcdData);
}

void LCD_Init(I2C_HandleTypeDef *phi2c)
{
	hLcdI2C = phi2c;
	HAL_Delay(40);
	LCD_CmdMode();
	LCD_WriteMode();
	LCD_WriteNibble(0x30);
	HAL_Delay(5);
	LCD_WriteNibble(0x30);
	HAL_Delay(1);
	LCD_WriteNibble(0x30);
	LCD_WriteNibble(0x20);
	LCD_WriteByte(LCD_4BIT_FUNCTION_SET); // 0x28
	LCD_WriteByte(LCD_DISPLAY_OFF); // 0x08
	LCD_WriteByte(LCD_DISPLAY_CLEAR); // 0x01
	LCD_WriteByte(LCD_ENTRY_MODE_SET); // 0x06
	LCD_WriteByte(LCD_DISPLAY_ON); // 0x0C
	LCD_BackLightOn();
}

void LCD_gotoXY(uint8_t row, uint8_t col)
{
	col %= 16;
	row %= 2;

	uint8_t lcdRegisterAddress = (0x40 * row) + col;
	uint8_t command = 0x80 + lcdRegisterAddress;
	LCD_WriteCmdData(command);
}

void LCD_WriteString(char *str)
{
	for (int i=0; str[i]; i++) {
		LCD_WriteCharData(str[i]);
	}
}

void LCD_WriteStringXY(uint8_t row, uint8_t col, char *str)
{
	LCD_gotoXY(row, col);
	LCD_WriteString(str);
}
