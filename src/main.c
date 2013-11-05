/*

Oh, eternity with outstretched wings, that broodest over the secret
truths in whose roots lie the mysteries of man—his whence, his
whither—have I searched thee, and struck a right key on thy dreadful
organ!

[Thomas de Quincey. The Dark Interpreter] 

*/

/* first steps for the dark interpreter based on work by: */
/* E. Brombaugh 07-31-2012                                        */
/* M. Howse 08.2013 */

 /*
PATH=~/sat/bin:$PATH
PATH=~/stm32f4/stlink/flash:$PATH

make stlink_flash
 */

#include "stm32f4xx.h"
#include "codec.h"
#include "i2s.h"
#include "adc.h"
#include "audio.h"
#include "hardware.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

#define delay()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 1000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

#define delay2()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 10000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)


void main(void)
{
	uint32_t state;
	int32_t idx, rcount,wcount;
	uint16_t data,x,y;
		
#if 1
	Audio_Init();
	ADC1_Init((uint16_t *)adc_buffer);
	setup_switches();
	//	switch_jack();
		test_filter();
	//			test_40106andfilt(); //???
	//test_filtand40106(); // works nicely
	//switchalloff();
	//	test_40106(); 

	Codec_Init(48000);
	delay();	// needed to allow codec to settle?
		retryagainpwm(); 	
		//		setup40106power();
	//	test_40106(); //that's working! is it???

	
	I2S_Block_Init();
	
	I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);
#else
	for(state=0;state<8;state++)
	{
		for(idx=0;idx<BUFF_LEN/2;idx++)
		{
			tx_buffer[2*idx+0] = state*BUFF_LEN/2 + idx;
			tx_buffer[2*idx+1] = state*BUFF_LEN/2 + idx;
		}
		I2S_RX_CallBack(tx_buffer, rx_buffer, BUFF_LEN);
	}
#endif
	x=rcount=wcount=1;
	
	while(1)
	{
	  // top down knobs: 2,0,3,4,1 
	  	  	  y=adc_buffer[2]; 
			  x=adc_buffer[0]; 
			  // setmaximpwm(200);
			  setmaximpwm(200+y); // from 200 to 4800 (lowest)
			  //setlmpwm(200+y, x); // from 1 to 1000
			  //			  set40106power(10-(x/100)); // should be from 0-20 say or 0-10
			  // set40106power(x);
	  x++;
	  delay2();
	  if (x>1000) x=1;
	}
}

#ifdef  USE_FULL_ASSERT

#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

#if 1
/* exception handlers - so we know what's failing */
void NMI_Handler(void)
{ 
	while(1){};
}

void HardFault_Handler(void)
{ 
	while(1){};
}

void MemManage_Handler(void)
{ 
	while(1){};
}

void BusFault_Handler(void)
{ 
	while(1){};
}

void UsageFault_Handler(void)
{ 
	while(1){};
}

void SVC_Handler(void)
{ 
	while(1){};
}

void DebugMon_Handler(void)
{ 
	while(1){};
}

void PendSV_Handler(void)
{ 
	while(1){};
}
#endif
