#ifndef _MYIIC_H
#define _MYIIC_H
#include "main.h"

#define SDA_IN()  {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=0<<9*2;}	//输入模式
#define SDA_OUT() {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=1<<9*2;} //输出模式

//方便G4官方版直插 OLED_12864
#if defined (STM32G4)
    #define IIC_SCL_0 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_RESET)
    #define IIC_SCL_1 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_SET)
    #define IIC_SDA_0 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8, GPIO_PIN_RESET)
    #define IIC_SDA_1 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8, GPIO_PIN_SET)
    #define READ_SDA  HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_8)
#endif
#if defined (STM32F407xx)  
    #define IIC_SCL_0 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8, GPIO_PIN_RESET)
    #define IIC_SCL_1 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8, GPIO_PIN_SET)
    #define IIC_SDA_0 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_RESET)
    #define IIC_SDA_1 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_SET)
    #define READ_SDA  HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)
#endif

void IIC_Init(void);                      //初始化IIC的IO口				 
void IIC_Start(void);				              //发送IIC开始信号
void IIC_Stop(void);	  			            //发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);			    //IIC发送一个字节
uint8_t IIC_Read_Byte(unsigned char ack); //IIC读取一个字节
uint8_t IIC_Wait_Ack(void); 				      //IIC等待ACK信号
void IIC_Ack(void);					              //IIC发送ACK信号
void IIC_NAck(void);				              //IIC不发送ACK信号

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	 
#endif

