#ifndef _FFT_H
#define _FFT_H

#include "main.h"
#include "delay.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "arm_math.h"
#include "myusart1.h"
#include "OLED_I2C.h"
#include "WindowFunction.h"

void	ADC_Init(void);
void 	ADC_Get(void);
void 	Fre_Control(uint32_t Fre);
double 	Get_amp(void);
void 	Get_Show_THD(void);
void 	wave_determine(double THD);
int 	fft_getpeak(float *inputx,float *input,float *output,uint16_t inlen,uint8_t x,uint8_t N,float y);
void 	fft(void);

#endif
