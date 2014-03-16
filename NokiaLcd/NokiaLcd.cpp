#include "NokiaLcd.h"





PCF8833Lcd::PCF8833Lcd(uint8_t rst,uint8_t cs,uint8_t sck,uint8_t sdio)
{
  _rst_pin = rst;
  _cs_pin = cs;
  _sck_pin = sck;
  _sdio_pin = sdio;

}


PCF8833Lcd::~PCF8833Lcd()
{

}


void PCF8833Lcd::sendCmd(uint8_t cmd)
{
  uint8_t tmpChar;
			
  digitalWrite(_cs_pin, LOW);
  digitalWrite(_sck_pin, LOW);
  digitalWrite(_sdio_pin, LOW);//  output low on data out (9th bit low = command), p0.19
  digitalWrite(_sck_pin, HIGH); // send clock pulse
  for (tmpChar = 0; tmpChar < 8; tmpChar++)
  {
    digitalWrite(_sck_pin, LOW);	// send clock pulse
    if ((cmd & 0x80) == 0x80)digitalWrite(_sdio_pin, HIGH);
    else digitalWrite(_sdio_pin, LOW);
    cmd <<= 1;
    digitalWrite(_sck_pin, HIGH);
  }
  digitalWrite(_cs_pin, HIGH);	// disable



}


void PCF8833Lcd::sendData(uint8_t data)
{
  uint8_t tmpChar;

  digitalWrite(_cs_pin, LOW);
  digitalWrite(_sck_pin, LOW);
  digitalWrite(_sdio_pin, HIGH);// output high on data out (9th bit high = data), p0.19
  digitalWrite(_sck_pin, HIGH); // send clock pulse
  for (tmpChar = 0; tmpChar < 8; tmpChar++)
  {
	digitalWrite(_sck_pin, LOW);	// send clock pulse
	if ((data & 0x80) == 0x80)digitalWrite(_sdio_pin, HIGH);
	else digitalWrite(_sdio_pin, LOW);
    data <<= 1;
	digitalWrite(_sck_pin, HIGH);
  }
  digitalWrite(_cs_pin, HIGH);	// disable
}


void PCF8833Lcd::init(bool colorSwap)
{
  digitalWrite(_rst_pin, HIGH);
  digitalWrite(_cs_pin, HIGH);
  digitalWrite(_sck_pin, HIGH);
  digitalWrite(_sdio_pin, HIGH);
  delay(10);
  digitalWrite(_rst_pin, LOW);
  delay(200);
  digitalWrite(_rst_pin, HIGH);
  delay(200);

  sendCmd(SLEEPOUT);	
  sendCmd(BSTRON);	
  sendCmd(DISPON);	
  // 12-bit color pixel format:
  sendCmd(COLMOD);
  sendData(0x03);
  sendCmd(MADCTL);	
  if(colorSwap)sendData(0x08);
  else sendData(0x00);
  sendCmd(SETCON);	
  sendData(0x30);	
  sendCmd(NOPP);	
}


void PCF8833Lcd::begin(void)
{
  CurrentX = 0;
  CurrentY = 0;

  pinMode(_rst_pin,OUTPUT);
  pinMode(_cs_pin, OUTPUT);
  pinMode(_sck_pin, OUTPUT);
  pinMode(_sdio_pin, OUTPUT);
  memset(backGroundColor,0,sizeof(backGroundColor));
  init();

}




void PCF8833Lcd::SetPixel(uint16_t color, unsigned char x, unsigned char y)
{
  sendCmd(PASETP);	// page start/end ram	
  sendData(x);	
  sendData(x);	
  sendCmd(CASETP);	
  sendData(y);
  sendCmd(y);	// column start/end ram
	
  sendCmd(RAMWRP);	// write

  sendData((unsigned char)((color>>4)&0x00FF));
  sendData((unsigned char)((color&0x0F)<<4));
}



void PCF8833Lcd::ClearScring(uint16_t color)
{
  uint8_t x;
  uint8_t y;
  uint8_t color0;
  uint8_t color1;
  uint8_t color2;

  sendCmd(PASETP);	
  sendData(0);	
  sendData(NOKIA_LCD_WIDTH);	
  sendCmd(CASETP);	
  sendData(0);
  sendCmd(NOKIA_LCD_HEIGHT);	
	
  sendCmd(RAMWRP);	// write

  //RG-BR-GB//
  color0 = (unsigned char)((color>>4)&0x00FF);
  color1 = (unsigned char)(((color>>8)&0x000F)|((color<<4) & 0x00F0));
  color2 = (unsigned char)(color&0x00FF);
  for(y=0;y<ROW_LENGTH;y++)
  {
    for(x=0;x<(COL_HEIGHT/2);x++)
    {
       sendData(color0);
       sendData(color1);
	   sendData(color2);
    } 
  }
  

}

void PCF8833Lcd::PutChar(unsigned char  x,unsigned char  y,char data,uint16_t color,uint16_t backGround)
{
  uint8_t  tmpX;
  uint8_t  tmpY;
  uint8_t  table;
  
  
  // Bounds check
  if(((x+FONT_WIDE) > NOKIA_LCD_WIDTH) || ((y+FONT_HEIGHT) > NOKIA_LCD_HEIGHT))
    return;
  if(!(backGround & BACKGROUND_REPLACE))
  {
  	
  }
  else 
  {
    for(tmpY=0; tmpY < FONT_HEIGHT; tmpY++) 
	{
	   for(tmpX = 0;tmpX < FONT_WIDE; tmpX++)
	   {
	      backGroundColor[tmpY][tmpX] = backGround;
	   }	
	}
  }
}


void PCF8833Lcd::printf(unsigned char  x,unsigned char  y,int color,uint16_t backGround,const char* fmt,...)
{



}




