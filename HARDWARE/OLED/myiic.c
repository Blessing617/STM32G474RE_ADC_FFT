#include "myiic.h"
#include "delay.h"
	
//IIC初始化
void IIC_Init(void)
{
//    GPIO_InitTypeDef GPIO_Initure;
//    
//    __HAL_RCC_GPIOB_CLK_ENABLE();   //使能GPIOB时钟
//    
//    GPIO_Initure.Pin=GPIO_PIN_8|GPIO_PIN_9;
//    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
//    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
//    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //快速
//    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
  
    IIC_SDA_1;
    IIC_SCL_1;  
}

//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     
	IIC_SDA_1;	  	  
	IIC_SCL_1;
	delay_us(4);
 	IIC_SDA_0;
	delay_us(4);
	IIC_SCL_0;
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SCL_0;
	IIC_SDA_0;
 	delay_us(4);
	IIC_SCL_1; 
	IIC_SDA_1;
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	SDA_IN();       
	IIC_SDA_1;
  delay_us(1);	   
	IIC_SCL_1;
  delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_0; 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL_0;
	SDA_OUT();
	IIC_SDA_0;
	delay_us(2);
	IIC_SCL_1;
	delay_us(2);
	IIC_SCL_0;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL_0;
	SDA_OUT();
	IIC_SDA_1;
	delay_us(2);
	IIC_SCL_1;
	delay_us(2);
	IIC_SCL_0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(uint8_t txd)
{                        
  uint8_t t;   
  SDA_OUT(); 	    
  IIC_SCL_0;
  for(t=0;t<8;t++)
  {              
    if (txd & 0x80)
      IIC_SDA_1;
    else
      IIC_SDA_0;
    txd<<=1; 	  
    delay_us(2);   
    IIC_SCL_1;
    delay_us(2); 
    IIC_SCL_0;	
    delay_us(2);
  }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();
  for(i=0;i<8;i++ )
	{
    IIC_SCL_0; 
    delay_us(2);
    IIC_SCL_1;
    receive<<=1;
    if(READ_SDA)receive++;   
		delay_us(1); 
   }					 
    if (!ack)
        IIC_NAck();
    else
        IIC_Ack(); 
    return receive;
}


