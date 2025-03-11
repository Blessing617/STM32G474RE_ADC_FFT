#include "OLED_I2C.h"
#include "delay.h"
#include "myiic.h"
#include "codetab.h"
#include "stdio.h"
#include "string.h"

uint8_t RevState=1;
unsigned char brightness=0xff;

void I2C_WriteByte(uint8_t addr,uint8_t data)
{
	IIC_Start();
	IIC_Send_Byte(OLED_ADDRESS);
	IIC_Wait_Ack();
	IIC_Send_Byte(addr);
	IIC_Wait_Ack();
	IIC_Send_Byte(data);
	IIC_Wait_Ack();
	IIC_Stop();
}

void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_WriteByte(0x00, I2C_Command);
}

void WriteDat(unsigned char I2C_Data)//写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}

void OLED_Init(void)
{
	IIC_Init();
	delay_ms(100); //这里的延时很重要
	
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(brightness); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	if(RevState)
		WriteCmd(0xa6);
	else
		WriteCmd(0xa7);
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
    {
      WriteDat(fill_Data);
    }
	}
}

void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y,unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          : 
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}


//显示一个字符
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch, unsigned char TextSize)
{
	unsigned char i = 0;
	switch(TextSize)
	{
		case 1:
		{
			ch -= 32;
			OLED_SetPos(x,y);
			for(i=0;i<6;i++)
				WriteDat(F6x8[ch][i]);
			x += 6;
		}break;
		case 2:
		{
			ch -= 32;
			OLED_SetPos(x,y);
			for(i=0;i<8;i++)
				WriteDat(F8X16[ch*16+i]);
			OLED_SetPos(x,y+1);
			for(i=0;i<8;i++)
				WriteDat(F8X16[ch*16+i+8]);
			x += 8;
		}break;
	}
}

//显示int型数据
int OLED_ShowInt(unsigned char x, unsigned char y, int val, unsigned char TextSize)
{
	unsigned char str[12], ch;
	int i, n;
	if (val == 0)
	{
		OLED_ShowChar(x, y, '0', TextSize);
		return 1;
	}
	else
	{
		if (val < 0)
		{
			str[0] = '-';
			val = -val;
		}
		else
			str[0] = ' ';
		for (n = 1; val > 0; val /= 10, n++)
			str[n] = (val % 10) + '0';
		str[n] = '\0';
		for (i = 1; i <= n / 2; i++)
		{
			ch = str[i];
			str[i] = str[n - i];
			str[n - i] = ch;
		}
		if (str[0] == '-')
		{
			OLED_ShowStr(x, y, str, TextSize);
			return n;
		}
		else
		{
			OLED_ShowStr(x, y, str + 1, TextSize);
			return (n-1);
		}
	}
}

//显示字符串反色
void OLED_ShowRevStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(~F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(~F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(~F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}


void OLED_ShowRevChar(unsigned char x, unsigned char y, unsigned char ch, unsigned char TextSize)
{
	unsigned char i = 0;
	switch(TextSize)
	{
		case 1:
		{
			ch -= 32;
			OLED_SetPos(x,y);
			for(i=0;i<6;i++)
				WriteDat(~F6x8[ch][i]);
			x += 6;
		}break;
		case 2:
		{
			ch -= 32;
			OLED_SetPos(x,y);
			for(i=0;i<8;i++)
				WriteDat(~F8X16[ch*16+i]);
			OLED_SetPos(x,y+1);
			for(i=0;i<8;i++)
				WriteDat(~F8X16[ch*16+i+8]);
			x += 8;
		}break;
	}
}

//0x00-0xff
void OLED_SetBrightness(unsigned char val)
{
	WriteCmd(0xAE);
	WriteCmd(0x81);
	WriteCmd(val);
	WriteCmd(0xAf);
}
//全局反色
void OLED_Reverse(void)
{
	RevState=!RevState;
	if(RevState)
		WriteCmd(0xa6);
	else
		WriteCmd(0xa7);
}

int OLED_ShowFloat(unsigned char x, unsigned char y, double val, unsigned char TextSize)
{
	char out[16];
	sprintf(out,"%.2lf",val);
	out[15]='\0';
	OLED_ShowStr(x,y,(unsigned char *)out,TextSize);
	return(strlen(out));
}

//画点
// x: 0-127  y: 0-63
unsigned char Point_Data[128][8]={0};
void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t)
{
  unsigned char y_pos,temp=0;
  y_pos=y/8;
  
  //位置地址
  WriteCmd(0xb0+y_pos);         //页地址
	WriteCmd(((x&0xf0)>>4)|0x10); //列地址 高四位
	WriteCmd((x&0x0f)|0x00);      //列地址 低四位
  
  //写数据
  temp=1<<(y%8);
 	if(t)Point_Data[x][y_pos]|=temp;
	else Point_Data[x][y_pos]&=~temp;
  
  WriteDat(Point_Data[x][y_pos]);//写数据  
}


//滚动显示


uint8_t isScrolling = 0;
void OLED_StopScroll(void)
{
  WriteCmd(0x2E);
  isScrolling = 0;
}
void OLED_Scroll(uint8_t direction, uint8_t startPage, uint8_t endPage, uint8_t hSpeed, uint8_t vSpeed)
{
  static const uint8_t hSpeedReg[8] = {0x3, 0x2, 0x1, 0x6, 0x0, 0x5, 0x4, 0x7}; // framePerT: 256, 128, 64, 25, 5, 4, 3, 2
  if (startPage > 7)
    startPage = 7;
  if (endPage > 7)
    endPage = 7;
  if (endPage < startPage)
    endPage = startPage;
  hSpeed &= 0x7;  // & 3'b111
  vSpeed &= 0x3F; // & 6'b111111
  
  OLED_StopScroll();
  WriteCmd(0x28 | (direction & 0x03));
  WriteCmd(0x00);
  WriteCmd(startPage);
  WriteCmd(hSpeedReg[hSpeed]);
  WriteCmd(endPage);
  WriteCmd(vSpeed);
  WriteCmd(0x2F); // Start Scroll
  isScrolling = 1;
}

//矩形区域填充 x:0-127,y:0-63 
void OLED_Fill_Area(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,unsigned char fill_Data)
{
	unsigned char m,n;
  
  OLED_SetPos(x1,y1); //设置起始点坐标
	for(m=0;m<y2-y1+1;m++)
	{
    OLED_SetPos(x1,y1+m);
		for(n=x1;n<x2+1;n++)
    {
      WriteDat(fill_Data);
    }
	}
}


void BackGround(void)
{
	OLED_ShowCN(0,0,2);			//波
	OLED_ShowCN(16,0,3);		//形
	
	OLED_ShowCN(0,2,4);			//频
	OLED_ShowCN(16,2,5);		//率	
	
	OLED_ShowCN(0,4,6);			//幅
	OLED_ShowCN(16,4,8);		//值
	
	OLED_ShowStr(104,2,(uint8_t*)"Hz",2);
	OLED_ShowChar(90,4,'V',2);
	OLED_ShowChar(90,6,'%',2);
	
	OLED_ShowStr(0,6,(uint8_t *)"THD",2);	
}

void wave_show (int wave)
{
	switch (wave)
	{
		case sine : OLED_ShowCN(45,0,12); 		//正
					OLED_ShowCN(45+16,0,13); 	//弦
					break;
		
		case tri : 	OLED_ShowCN(45,0,15); 		//三
					OLED_ShowCN(45+16,0,16); 	//角
					break;
		
		case squ :	OLED_ShowCN(45,0,14); 		//方
					OLED_ShowCN(45+16,0,2); 	//波
					break;  					
	}
}
