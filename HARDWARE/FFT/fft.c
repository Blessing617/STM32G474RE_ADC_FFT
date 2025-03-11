#include "FFT.h"

#define  sampledot 		 1024
#define  FFT_LENGTH		 1024 				//FFT长度，默认是1024点FFT	
#define  FRE			 1024000

uint16_t ADC_GetValue	[1024] = {0};
float fft_inputbuf	[FFT_LENGTH*2	] = {0};			//FFT输入数组
float fft_outputbuf	[FFT_LENGTH		] = {0};			//FFT输出数组

arm_cfft_radix4_instance_f32 scfft;

void Fre_Control(uint32_t Fre)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

	uint32_t MaxData;
	uint16_t div=1;	
	while( (SystemCoreClock/Fre/div)>65535 )
	{
		div++;
	}
	MaxData =  SystemCoreClock/Fre/div - 1;	
	
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = div-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = MaxData;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

void ADC_Init(void)
{
	Fre_Control(FRE);
	HAL_TIM_Base_Start(&htim3);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_GetValue, 1024); 	
}

void ADC_Get(void)
{
	Fre_Control(FRE);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_GetValue, 1024); 
}

double Get_amp(void)
{
	uint16_t i;
	double max=0,min=4096;
	double amp;
	for(i=0;i<1023;i++)
	{
		if((uint16_t)(ADC_GetValue[i])>max)
				max=(uint16_t)(ADC_GetValue[i]);
		if((uint16_t)(ADC_GetValue[i])<min)
				min=(uint16_t)(ADC_GetValue[i]);
	}
	amp=(max-min)*3.3/4095.0f;
	//OLED_ShowFloat(0,4,max*3.3/4095.0f,2);	
	//OLED_ShowFloat(80,4,min*3.3/4095.0f,2);
	return amp;
}

void Get_Show_THD(void)
{
	float max[5],k[5];
	double THD;
	uint32_t i,j,l,l1;
	for (i=2;i<1022;i++)
	{
		if ((fft_outputbuf[i]>200)&&(fft_outputbuf[i]>2*fft_outputbuf[i-1])&&(fft_outputbuf[i]>=fft_outputbuf[i+1]))
		{
			break;
		}
	}
	max[0]=fft_outputbuf[i];
	for (j=1;j<5;j++)
	{
		double buf;
		k[0] = fft_outputbuf[(j+1)*i-2];
		k[1] = fft_outputbuf[(j+1)*i-1];
		k[2] = fft_outputbuf[(j+1)*i];
		k[3] = fft_outputbuf[(j+1)*i+1];
		k[4] = fft_outputbuf[(j+1)*i+2];
		for (l=0; l<4 ; ++l)  								//比较n-1轮
		{
			for (l1=0; l1<4-l; ++l1)  						//每轮比较n-1-i次,
			{
				if (k[l1] < k[l1+1])
				{
					buf = k[l1];
					k[l1] = k[l1+1];
					k[l1+1] = buf;
				}
			}
		}
	max[j]=k[0];
	}
	for (i=0;i<5;i++)
	{
		if (max[i]<8)
		{
			max[i]=0;
		}
		if (max[i]>50000)
		{
			max[i]=0;
		}
	}
	THD = sqrt(max[1]*max[1]+ max[2]*max[2] + max[3]*max[3] + max[4]*max[4] );
	THD = THD/max[0];
	THD = THD*100;
	wave_determine(THD);
	OLED_ShowFloat(40,6,THD,2);
}

int fft_getpeak(float *inputx,float *input,float *output,uint16_t inlen,uint8_t x,uint8_t N,float y) //  intlen 输入数组长度，x寻找长度
{
	int i,i2;
	uint32_t idex;  
	float datas;
	float sum;
	int outlen=0;
	for(i=0;i<inlen-x;i+=x)
	{
		arm_max_f32(input+i,x,&datas,&idex);
		if( (input[i+idex]>=input[i+idex+1])&&(input[i+idex]>=input[i+idex-1])&&( (2*datas)/FFT_LENGTH )>y)
		   {
			   sum=0;
			   for(i2=i+idex-N;i2<i+idex+N;i2++)
			   {
				   sum+=input[i2];
			   }
			   if(1.5f*sum/(2*N)<datas)
			   {					
				     output[3*outlen] = 1.0f*FRE*(i+idex+1)/FFT_LENGTH;//计算频率
					 outlen++;
			   }
                else continue;
		   }
		else continue;
	}
	return outlen;
}

void wave_determine(double THD)
{
	if(THD < 5)
	{
		wave_show(sine);
	}
	else if(THD > 5 && THD <20)
	{
		wave_show(tri);
	}
	else if(THD > 20 && THD <45)
	{
		wave_show(squ);
	}
}

float freamp[50];//获取各次谐波频率和幅

void fft(void)
{  
	uint16_t i,freamplen;
	float HZ,amp;
	
	arm_cfft_radix4_instance_f32 scfft;
	arm_cfft_radix4_init_f32(&scfft,FFT_LENGTH,0,1);
	
//	for(i=0;i<FFT_LENGTH;i++)
//	{
//		printf("%d\r\n",ADC_GetValue[i]);
//	}

	for(i=0;i<FFT_LENGTH;i++)//生成信号序列
	{
		fft_inputbuf[2*i] = (float)ADC_GetValue[ i ]*3.3f/4096.0f;	//实部为ADC采样值
		fft_inputbuf[2*i+1]=0;										//虚部全部为0
	}
	
	arm_cfft_radix4_f32(&scfft,fft_inputbuf);		
	arm_cmplx_mag_f32(fft_inputbuf,fft_outputbuf,FFT_LENGTH);		//把运算结果复数求模得幅值 	
	
	for(i=1;i<FFT_LENGTH/1;i++)
	{
		printf("%f\r\n",fft_outputbuf[i]);
	}	
	
	Get_Show_THD();
	
	freamplen=fft_getpeak(fft_inputbuf,fft_outputbuf+1,freamp,FFT_LENGTH/2,10,5,0.2);//寻找基波和谐波
	
	amp=Get_amp();
	HZ=freamp[0];
	freamp[0]=0;	
	
	OLED_ShowFloat(32,2,HZ,2);
	OLED_ShowFloat(40,4,amp,2);
}
