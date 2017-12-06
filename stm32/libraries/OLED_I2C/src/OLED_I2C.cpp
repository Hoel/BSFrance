/*
  OLED_I2C.cpp - Arduino/chipKit library support for 128x64 pixel SSD1306 OLEDs
  Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
  
  This library has been made to make it easy to use 128x64 pixel OLED displays
  based on the SSD1306 controller chip with an Arduino or a chipKit.

  You can always find the latest version of the library at 
  http://www.RinkyDinkElectronics.com/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
  
  add support ssd1306_12832 by huaweiwx@sina.com 2016.11.8
  change class OLED to class OLED_I2C,print to printxy
  add class OLED_12864 and class OLED_12832
  modify stm32 12864 unword prob.  
*/

#include "Arduino.h"
#include "OLED_I2C.h"

#if defined(__AVR__)
	#include "hardware/avr/HW_AVR_defines.h"
	#include "hardware/avr/HW_AVR.h"
#elif defined(__PIC32MX__)
	#include "WProgram.h"
	#include "hardware/pic32/HW_PIC32_defines.h"
	#include "hardware/pic32/HW_PIC32.h"
#elif defined(STM32GENERIC)
    #include "hardware/arm/HW_STM32HAL_defines.h"
	#include "hardware/arm/HW_STM32.h"
#elif defined (__STM32F1__) ||defined (STM32F2)||defined (STM32F4)
    #include "hardware/arm/HW_STM32_defines.h"
	#include "hardware/arm/HW_STM32.h"
#elif defined(__arm__)
	#include "hardware/arm/HW_ARM_defines.h"
	#include "hardware/arm/HW_ARM.h"
#else
	#error "board notdefined"
#endif

OLED_I2C::OLED_I2C(uint8_t data_pin, uint8_t sclk_pin, uint8_t rst_pin)
{ 
	_sda_pin = data_pin;
	_scl_pin = sclk_pin;
	_rst_pin = rst_pin;
}

void OLED_I2C::init()
{   

	if (_rst_pin != RST_NOT_IN_USE)
	{
		pinMode(_rst_pin, OUTPUT);
		digitalWrite(_rst_pin, HIGH);
		delay(1);
		digitalWrite(_rst_pin, LOW);
		delay(10);
		digitalWrite(_rst_pin, HIGH);
	}
#if defined(__AVR__)|| defined(__PIC32MX__)

#if defined(SDA1) & defined(SCL1)
	if (((_sda_pin == SDA) and (_scl_pin == SCL)) or ((_sda_pin == SDA1) and (_scl_pin == SCL1)))
#else
	if ((_sda_pin == SDA) and (_scl_pin == SCL))
#endif
	{
		_use_hw = true;
		_initTWI();
	}
	else
#endif		
	{
		_use_hw = false;
		pinMode(_scl_pin, OUTPUT);
	}

    //huaweiwx@sina.com  2016.11.15	
	delay(200);//http://www.openedv.com/posts/list/54584.htm

	_sendTWIcommand(SSD1306_DISPLAY_OFF);  //0xAE
    _sendTWIcommand(SSD1306_SET_DISPLAY_CLOCK_DIV_RATIO); //0xD5
    _sendTWIcommand(0x80);
    _sendTWIcommand(SSD1306_SET_MULTIPLEX_RATIO);//0xA8
    _sendTWIcommand(0x3F);
    _sendTWIcommand(SSD1306_SET_DISPLAY_OFFSET);//0xD3
    _sendTWIcommand(0x0);
    _sendTWIcommand(SSD1306_SET_START_LINE | 0x0);//0x40
    _sendTWIcommand(SSD1306_CHARGE_PUMP);//0x8D
//	_sendTWIcommand(0x10); //Vcc supplied Externally
	_sendTWIcommand(0x14); //VCC Generated by Internal DC/DC Circuit
    _sendTWIcommand(SSD1306_MEMORY_ADDR_MODE);//0x20
    _sendTWIcommand(0x00);
    _sendTWIcommand(SSD1306_SET_SEGMENT_REMAP | 0x1);//0xA0|1=A1
    _sendTWIcommand(SSD1306_COM_SCAN_DIR_DEC);//0xC8

    _sendTWIcommand(SSD1306_SET_COM_PINS);//0xDA
//    _sendTWIcommand(0x12);
    if (_max_high == 32) _sendTWIcommand(SET_COMVAL_12832);  //add support ssd1306_12832 by huaweiwx@sina.com
    else _sendTWIcommand(SET_COMVAL_12864);
    		
    _sendTWIcommand(SSD1306_SET_CONTRAST_CONTROL);//0x81
	_sendTWIcommand(0xCF); //9F Vcc Externally /CF Vcc dc_dc
    _sendTWIcommand(SSD1306_SET_PRECHARGE_PERIOD);//0xD9
	_sendTWIcommand(0xF1); //0x22 vcc externally //F1 internal dc_dc
    _sendTWIcommand(SSD1306_SET_VCOM_DESELECT);//0xDB
    _sendTWIcommand(0x40);
    _sendTWIcommand(SSD1306_DISPLAY_ALL_ON_RESUME);//0xA4
    _sendTWIcommand(SSD1306_NORMAL_DISPLAY);//0xA6
	_sendTWIcommand(SSD1306_DISPLAY_ON);//0xAF

	clrScr();
	update();
	cfont.font=0;
}

void OLED_I2C::clrScr()
{
	memset(scrbuf, 0, buf_size);
}

void OLED_I2C::fillScr()
{
	memset(scrbuf, 255, buf_size);
}

void OLED_I2C::setBrightness(uint8_t value)
{
	_sendTWIcommand(SSD1306_SET_CONTRAST_CONTROL);
	_sendTWIcommand(value);
}

void OLED_I2C::invert(bool mode)
{
	if (mode==true)
		_sendTWIcommand(SSD1306_INVERT_DISPLAY);
	else
		_sendTWIcommand(SSD1306_NORMAL_DISPLAY);
}

void OLED_I2C::setPixel(uint16_t x, uint16_t y)
{
	int by, bi;

//	if ((x>=0) and (x<128) and (y>=0) and (y<_max_high))
	if ((x<128) and (y<_max_high))
	{
		by=((y/8)*128)+x;
		bi=y % 8;

		scrbuf[by] = scrbuf[by] | (1<<bi);
	}
}

void OLED_I2C::clrPixel(uint16_t x, uint16_t y)
{
	int by, bi;

//	if ((x>=0) and (x<128) and (y>=0) and (y<_max_high))
	if ((x<128) and (y<_max_high))
	{
		by=((y/8)*128)+x;
		bi=y % 8;

		scrbuf[by] = scrbuf[by] & ~(1<<bi);
	}
}

void OLED_I2C::invPixel(uint16_t x, uint16_t y)
{
	int by, bi;

//	if ((x>=0) and (x<128) and (y>=0) and (y<_max_high))
	if ((x<128) and (y<_max_high))
	{
		by=((y/8)*128)+x;
		bi=y % 8;

		if ((scrbuf[by] & (1<<bi))==0)
			scrbuf[by]=scrbuf[by] | (1<<bi);
		else
			scrbuf[by]=scrbuf[by] & ~(1<<bi);
	}
}

void OLED_I2C::invertText(bool mode)
{
	if (mode==true)
		cfont.inverted=1;
	else
		cfont.inverted=0;
}

void OLED_I2C::printxy(char *st, int x, int y)
{
//	unsigned char ch;
	int stl;

	stl = strlen(st);
	if (x == RIGHT)
		x = 128-(stl*cfont.x_size);
	if (x == CENTER)
		x = (128-(stl*cfont.x_size))/2;

	for (int cnt=0; cnt<stl; cnt++)
			_print_char(*st++, x + (cnt*(cfont.x_size)), y);
}

void OLED_I2C::printxy(String st, int x, int y)
{
	char buf[st.length()+1];

	st.toCharArray(buf, st.length()+1);
	printxy(buf, x, y);
}

void OLED_I2C::printNumI(long num, int x, int y, int length, char filler)
{
	char buf[25];
	char st[27];
	boolean neg=false;
	int c=0, f=0;
  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-1); c++)
				st[c]=filler;
			st[c]=48;
			st[c+1]=0;
		}
		else
		{
			st[0]=48;
			st[1]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=true;
			num=-num;
		}
	  
		while (num>0)
		{
			buf[c]=48+(num % 10);
			c++;
			num=(num-(num % 10))/10;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg))
		{
			for (int i=0; i<(length-c-neg); i++)
			{
				st[i+neg]=filler;
				f++;
			}
		}

		for (int i=0; i<c; i++)
		{
			st[i+neg+f]=buf[c-i-1];
		}
		st[c+neg+f]=0;

	}

	printxy(st,x,y);
}


void OLED_I2C::printNumF(double num, byte dec, int x, int y, char divider, int length, char filler)
{
	char st[27];
	boolean neg=false;

	if (num<0)
		neg = true;

	_convert_float(st, num, length, dec);

	if (divider != '.')
	{
		for (unsigned int i=0; i<sizeof(st); i++)
			if (st[i]=='.')
				st[i]=divider;
	}

	if (filler != ' ')
	{
		if (neg)
		{
			st[0]='-';
			for (unsigned int i=1; i<sizeof(st); i++)
				if ((st[i]==' ') || (st[i]=='-'))
					st[i]=filler;
		}
		else
		{
			for (unsigned int i=0; i<sizeof(st); i++)
				if (st[i]==' ')
					st[i]=filler;
		}
	}

	printxy(st,x,y);
}

void OLED_I2C::_print_char(unsigned char c, int x, int y)
{
	if ((cfont.y_size % 8) == 0)
	{
		int font_idx = ((c - cfont.offset)*(cfont.x_size*(cfont.y_size/8)))+4;
		for (int rowcnt=0; rowcnt<(cfont.y_size/8); rowcnt++)
		{
			for(int cnt=0; cnt<cfont.x_size; cnt++)
			{
				for (int b=0; b<8; b++)
					if ((fontbyte(font_idx+cnt+(rowcnt*cfont.x_size)) & (1<<b))!=0)
						if (cfont.inverted==0)
							setPixel(x+cnt, y+(rowcnt*8)+b);
						else
							clrPixel(x+cnt, y+(rowcnt*8)+b);
					else
						if (cfont.inverted==0)
							clrPixel(x+cnt, y+(rowcnt*8)+b);
						else
							setPixel(x+cnt, y+(rowcnt*8)+b);
			}
		}
	}
	else
	{
		int font_idx = ((c - cfont.offset)*((cfont.x_size*cfont.y_size/8)))+4;
		int cbyte=fontbyte(font_idx);
		int cbit=7;
		for (int cx=0; cx<cfont.x_size; cx++)
		{
			for (int cy=0; cy<cfont.y_size; cy++)
			{
				if ((cbyte & (1<<cbit)) != 0)
					if (cfont.inverted==0)
						setPixel(x+cx, y+cy);
					else
						clrPixel(x+cx, y+cy);
				else
					if (cfont.inverted==0)
						clrPixel(x+cx, y+cy);
					else
						setPixel(x+cx, y+cy);
				cbit--;
				if (cbit<0)
				{
					cbit=7;
					font_idx++;
					cbyte=fontbyte(font_idx);
				}
			}
		}
	}
}

void OLED_I2C::setFont(uint8_t* font)
{
	cfont.font=font;
	cfont.x_size=fontbyte(0);
	cfont.y_size=fontbyte(1);
	cfont.offset=fontbyte(2);
	cfont.numchars=fontbyte(3);
	cfont.inverted=0;
}

void OLED_I2C::drawHLine(int x, int y, int l)
{
	int by, bi;

	if ((x>=0) and (x<128) and (y>=0) and (y<_max_high))
	{
		for (int cx=0; cx<l; cx++)
		{
			by=((y/8)*128)+x;
			bi=y % 8;

			scrbuf[by+cx] |= (1<<bi);
		}
	}
}

void OLED_I2C::clrHLine(int x, int y, int l)
{
	int by, bi;

	if ((x>=0) and (x<128) and (y>=0) and (y<_max_high))
	{
		for (int cx=0; cx<l; cx++)
		{
			by=((y/8)*128)+x;
			bi=y % 8;

			scrbuf[by+cx] &= ~(1<<bi);
		}
	}
}

#pragma GCC diagnostic ignored "-Wunused-variable"
void OLED_I2C::drawVLine(int x, int y, int l)
{
	int by, bi;

	if ((x>=0) and (x<128) and (y>=0) and (y<_max_high))
	{
		for (int cy=0; cy<l; cy++)
		{
			setPixel(x, y+cy);
		}
	}
}

void OLED_I2C::clrVLine(int x, int y, int l)
{
	int by, bi;

	if ((x>=0) and (x<128) and (y>=0) and (y<_max_high))
	{
		for (int cy=0; cy<l; cy++)
		{
			clrPixel(x, y+cy);
		}
	}
}

void OLED_I2C::drawLine(int x1, int y1, int x2, int y2)
{
	int tmp;
	double delta, tx, ty;
	double m, b, dx, dy;
	
	if (((x2-x1)<0))
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
    if (((y2-y1)<0))
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	if (y1==y2)
	{
		if (x1>x2)
		{
			tmp=x1;
			x1=x2;
			x2=tmp;
		}
		drawHLine(x1, y1, x2-x1);
	}
	else if (x1==x2)
	{
		if (y1>y2)
		{
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		drawVLine(x1, y1, y2-y1);
	}
	else if (abs(x2-x1)>abs(y2-y1))
	{
		delta=(double(y2-y1)/double(x2-x1));
		ty=double(y1);
		if (x1>x2)
		{
			for (int i=x1; i>=x2; i--)
			{
				setPixel(i, int(ty+0.5));
        		ty=ty-delta;
			}
		}
		else
		{
			for (int i=x1; i<=x2; i++)
			{
				setPixel(i, int(ty+0.5));
        		ty=ty+delta;
			}
		}
	}
	else
	{
		delta=(float(x2-x1)/float(y2-y1));
		tx=float(x1);
        if (y1>y2)
        {
			for (int i=y2+1; i>y1; i--)
			{
		 		setPixel(int(tx+0.5), i);
        		tx=tx+delta;
			}
        }
        else
        {
			for (int i=y1; i<y2+1; i++)
			{
		 		setPixel(int(tx+0.5), i);
        		tx=tx+delta;
			}
        }
	}

}

void OLED_I2C::clrLine(int x1, int y1, int x2, int y2)
{
	int tmp;
	double delta, tx, ty;
	double m, b, dx, dy;
	
	if (((x2-x1)<0))
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
    if (((y2-y1)<0))
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	if (y1==y2)
	{
		if (x1>x2)
		{
			tmp=x1;
			x1=x2;
			x2=tmp;
		}
		clrHLine(x1, y1, x2-x1);
	}
	else if (x1==x2)
	{
		if (y1>y2)
		{
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		clrVLine(x1, y1, y2-y1);
	}
	else if (abs(x2-x1)>abs(y2-y1))
	{
		delta=(double(y2-y1)/double(x2-x1));
		ty=double(y1);
		if (x1>x2)
		{
			for (int i=x1; i>=x2; i--)
			{
				clrPixel(i, int(ty+0.5));
        		ty=ty-delta;
			}
		}
		else
		{
			for (int i=x1; i<=x2; i++)
			{
				clrPixel(i, int(ty+0.5));
        		ty=ty+delta;
			}
		}
	}
	else
	{
		delta=(float(x2-x1)/float(y2-y1));
		tx=float(x1);
        if (y1>y2)
        {
			for (int i=y2+1; i>y1; i--)
			{
		 		clrPixel(int(tx+0.5), i);
        		tx=tx+delta;
			}
        }
        else
        {
			for (int i=y1; i<y2+1; i++)
			{
		 		clrPixel(int(tx+0.5), i);
        		tx=tx+delta;
			}
        }
	}

}

void OLED_I2C::drawRect(int x1, int y1, int x2, int y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	drawHLine(x1, y1, x2-x1);
	drawHLine(x1, y2, x2-x1);
	drawVLine(x1, y1, y2-y1);
	drawVLine(x2, y1, y2-y1+1);
}

void OLED_I2C::clrRect(int x1, int y1, int x2, int y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	clrHLine(x1, y1, x2-x1);
	clrHLine(x1, y2, x2-x1);
	clrVLine(x1, y1, y2-y1);
	clrVLine(x2, y1, y2-y1+1);
}

void OLED_I2C::drawRoundRect(int x1, int y1, int x2, int y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		setPixel(x1+1,y1+1);
		setPixel(x2-1,y1+1);
		setPixel(x1+1,y2-1);
		setPixel(x2-1,y2-1);
		drawHLine(x1+2, y1, x2-x1-3);
		drawHLine(x1+2, y2, x2-x1-3);
		drawVLine(x1, y1+2, y2-y1-3);
		drawVLine(x2, y1+2, y2-y1-3);
	}
}

void OLED_I2C::clrRoundRect(int x1, int y1, int x2, int y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		clrPixel(x1+1,y1+1);
		clrPixel(x2-1,y1+1);
		clrPixel(x1+1,y2-1);
		clrPixel(x2-1,y2-1);
		clrHLine(x1+2, y1, x2-x1-3);
		clrHLine(x1+2, y2, x2-x1-3);
		clrVLine(x1, y1+2, y2-y1-3);
		clrVLine(x2, y1+2, y2-y1-3);
	}
}

void OLED_I2C::drawCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
	char ch, cl;
	
	setPixel(x, y + radius);
	setPixel(x, y - radius);
	setPixel(x + radius, y);
	setPixel(x - radius, y);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		setPixel(x + x1, y + y1);
		setPixel(x - x1, y + y1);
		setPixel(x + x1, y - y1);
		setPixel(x - x1, y - y1);
		setPixel(x + y1, y + x1);
		setPixel(x - y1, y + x1);
		setPixel(x + y1, y - x1);
		setPixel(x - y1, y - x1);
	}
}

void OLED_I2C::clrCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
	char ch, cl;
	
	clrPixel(x, y + radius);
	clrPixel(x, y - radius);
	clrPixel(x + radius, y);
	clrPixel(x - radius, y);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		clrPixel(x + x1, y + y1);
		clrPixel(x - x1, y + y1);
		clrPixel(x + x1, y - y1);
		clrPixel(x - x1, y - y1);
		clrPixel(x + y1, y + x1);
		clrPixel(x - y1, y + x1);
		clrPixel(x + y1, y - x1);
		clrPixel(x - y1, y - x1);
	}
}

void OLED_I2C::drawBitmap(int x, int y, uint8_t* bitmap, int sx, int sy)
{
	int bit;
	byte data;

	for (int cy=0; cy<sy; cy++)
	{
		bit= cy % 8;
		for(int cx=0; cx<sx; cx++)
		{
			data=bitmapbyte(cx+((cy/8)*sx));
			if ((data & (1<<bit))>0)
				setPixel(x+cx, y+cy);
			else
				clrPixel(x+cx, y+cy);
		}
	}      
}

// Private
void OLED_I2C::_sendStart(byte addr)
{
	pinMode(_sda_pin, OUTPUT);
	digitalWrite(_sda_pin, HIGH);
	digitalWrite(_scl_pin, HIGH);
	digitalWrite(_sda_pin, LOW);
	digitalWrite(_scl_pin, LOW);
	shiftOut(_sda_pin, _scl_pin, MSBFIRST, addr);
	}

void OLED_I2C::_sendStop()
{
	pinMode(_sda_pin, OUTPUT);
	digitalWrite(_sda_pin, LOW);
	digitalWrite(_scl_pin, HIGH);
	digitalWrite(_sda_pin, HIGH);
	pinMode(_sda_pin, INPUT);
}

void OLED_I2C::_sendNack()
{
	pinMode(_sda_pin, OUTPUT);
	digitalWrite(_scl_pin, LOW);
	digitalWrite(_sda_pin, HIGH);
	digitalWrite(_scl_pin, HIGH);
	digitalWrite(_scl_pin, LOW);
	pinMode(_sda_pin, INPUT);
}

void OLED_I2C::_sendAck()
{
	pinMode(_sda_pin, OUTPUT);
	digitalWrite(_scl_pin, LOW);
	digitalWrite(_sda_pin, LOW);
	digitalWrite(_scl_pin, HIGH);
	digitalWrite(_scl_pin, LOW);
	pinMode(_sda_pin, INPUT);
}

void OLED_I2C::_waitForAck()
{
	pinMode(_sda_pin, INPUT);
	digitalWrite(_scl_pin, HIGH);
	uint16_t times = 1000;
	while (times-- && (digitalRead(_sda_pin)==HIGH)) {}
	digitalWrite(_scl_pin, LOW);
}

void OLED_I2C::_writeByte(uint8_t value)
{
	pinMode(_sda_pin, OUTPUT);
	shiftOut(_sda_pin, _scl_pin, MSBFIRST, value);
}


void OLED_12864::begin(void)
{   
	scrbuf = this->bufs;
	buf_size = 1024;
	_max_high = 64;
	init();
}

void OLED_12832::begin(void)
{   
	scrbuf = this->bufs;
	buf_size = 512;
	_max_high = 32;
    init();
}
