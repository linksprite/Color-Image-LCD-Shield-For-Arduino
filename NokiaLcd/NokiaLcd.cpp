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


void PCF8833Lcd::Contrast(char setting)
{
	setting &= 0x7F;	// msb is not used, mask it out
	sendCmd(SETCON); // contrast command (PHILLIPS)
	sendData(setting);	// volume (contrast) setting - course adjustment,  -- original was 24
}



// Added by Steve Sparks @ Big Nerd Ranch.
// This swaps the Epson RGB order into the Philips RGB order. (Or, vice versa, I suppose.)
uint16_t PCF8833Lcd::SwapColors(uint16_t in) 
{
    return ((in & 0x000F)<<8)|(in & 0x00F0)|((in & 0x0F00)>>8);
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
  //CurrentX = 0;
  //CurrentY = 0;

  pinMode(_rst_pin,OUTPUT);
  pinMode(_cs_pin, OUTPUT);
  pinMode(_sck_pin, OUTPUT);
  pinMode(_sdio_pin, OUTPUT);
  //memset(backGroundColor,0,sizeof(backGroundColor));
  init();

}




void PCF8833Lcd::SetPixel(uint16_t color, unsigned char x, unsigned char y)
{
  y	=	(COL_HEIGHT - 1) - y;
  x = (ROW_LENGTH - 1) - x;
  sendCmd(PASETP);	// page start/end ram	
  sendData(x);	
  sendData(x);	
  sendCmd(CASETP);	
  sendData(y);
  sendData(y);	// column start/end ram
	
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


void PCF8833Lcd::DrawCircle (int x0, int y0, int radius, int color)
{
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	SetPixel(color, x0, y0 + radius);
	SetPixel(color, x0, y0 - radius);
	SetPixel(color, x0 + radius, y0);
	SetPixel(color, x0 - radius, y0);

	while(x < y)
	{
		if(f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;

		SetPixel(color, x0 + x, y0 + y);
		SetPixel(color, x0 - x, y0 + y);
		SetPixel(color, x0 + x, y0 - y);
		SetPixel(color, x0 - x, y0 - y);
		SetPixel(color, x0 + y, y0 + x);
		SetPixel(color, x0 - y, y0 + x);
		SetPixel(color, x0 + y, y0 - x);
		SetPixel(color, x0 - y, y0 - x);
	}
}



void PCF8833Lcd::PutChar(char c, int x, int y, int fColor, int bColor)
{
	y	=	(COL_HEIGHT - 1) - y; // make display "right" side up
	x	=	(ROW_LENGTH - 2) - x;

	int             i,j;
	unsigned int    nCols;
	unsigned int    nRows;
	unsigned int    nBytes;
	unsigned char   PixelRow;
	unsigned char   Mask;
	unsigned int    Word0;
	unsigned int    Word1;
	unsigned char   *pFont;
	unsigned char   *pChar;

	// get pointer to the beginning of the selected font table
	pFont = (unsigned char *)font8_16;
	// get the nColumns, nRows and nBytes
	nCols = *pFont;
	nRows = *(pFont + 1);
	nBytes = *(pFont + 2);
	// get pointer to the last byte of the desired character
	pChar = pFont + (nBytes * (c - 0x1F)) + nBytes - 1;

	
    fColor = SwapColors(fColor);
    bColor = SwapColors(bColor);

    // Row address set (command 0x2B)
	sendCmd(PASETP);
	sendData(x);
	sendData(x + nRows - 1);
    // Column address set (command 0x2A)
	sendCmd(CASETP);
	sendData(y);
	sendData(y + nCols - 1);
	
	sendCmd(RAMWRP);
	pChar+=nBytes-1;  // stick pChar at the end of the row - gonna reverse print on phillips
	for (i = nRows - 1; i >= 0; i--) 
	{
		PixelRow = *pChar--;
		Mask = 0x01;  // <- opposite of epson
		for (j = 0; j < nCols; j += 2) 
		{
			if ((PixelRow & Mask) == 0)Word0 = bColor;
			else Word0 = fColor;
			Mask = Mask << 1; // <- opposite of epson
			if ((PixelRow & Mask) == 0)Word1 = bColor;
			else Word1 = fColor;
			Mask = Mask << 1; // <- opposite of epson
			sendData((Word0 >> 4) & 0xFF);
			sendData(((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));
			sendData(Word1 & 0xFF);
		}
     }
}


void PCF8833Lcd::PutStr(char *pString, int x, int y, int fColor, int bColor)
{
	x = x + 16;
	y = y + 8;
    int originalY = y;

	while (*pString != '\0') 
	{
		if((*pString == '\n')||(*pString == '\r'))
	    {
	      y = 0;
	      x += 16;
	      if((x+16) > 131)x = 0;
		  continue;
	    }

		PutChar(*pString++, x, y, fColor, bColor);
		y = y + 8;
		if (y > 131) 
		{
            x = x + 16;
            y = 0;
        }
        if (x > 123) x = 0;
	}
}

void PCF8833Lcd::DrawLine(int x0, int y0, int x1, int y1, int color)
{
	int dy = y1 - y0; // Difference between y0 and y1
	int dx = x1 - x0; // Difference between x0 and x1
	int stepx, stepy;

	if (dy < 0)
	{
		dy = -dy;
		stepy = -1;
	}
	else stepy = 1;

	if (dx < 0)
	{
		dx = -dx;
		stepx = -1;
	}
	else stepx = 1;

	dy <<= 1; // dy is now 2*dy
	dx <<= 1; // dx is now 2*dx
	SetPixel(color, x0, y0);

	if (dx > dy) 
	{
		int fraction = dy - (dx >> 1);
		while (x0 != x1)
		{
			if (fraction >= 0)
			{
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			SetPixel(color, x0, y0);
		}
	}
	else
	{
		int fraction = dx - (dy >> 1);
		while (y0 != y1)
		{
			if (fraction >= 0)
			{
				x0 += stepx;
				fraction -= dy;
			}
		    y0 += stepy;
		    fraction += dx;
		    SetPixel(color, x0, y0);
		}
	}
}


void PCF8833Lcd::DrawRect(int x0, int y0, int x1, int y1, unsigned char fill, int color)
{
	// check if the rectangle is to be filled
	if (fill == 1)
	{
		int xDiff;
	
		if(x0 > x1)xDiff = x0 - x1; //Find the difference between the x vars
		else xDiff = x1 - x0;
		while(xDiff > 0)
		{
			DrawLine(x0, y0, x0, y1, color);
			if(x0 > x1)x0--;
			else x0++;
			xDiff--;
		}
	}
	else 
	{
		// best way to draw an unfilled rectangle is to draw four lines
		DrawLine(x0, y0, x1, y0, color);
		DrawLine(x0, y1, x1, y1, color);
		DrawLine(x0, y0, x0, y1, color);
		DrawLine(x1, y0, x1, y1, color);
	}
}


void PCF8833Lcd::OFF(void)
{
   sendCmd(DISPOFF);
}

void PCF8833Lcd::ON(void)
{
  sendCmd(DISPON);
}






void PCF8833Lcd::printf(unsigned char  x,unsigned char  y,uint16_t color,uint16_t backGround,const char* fmt,...)
{



}





