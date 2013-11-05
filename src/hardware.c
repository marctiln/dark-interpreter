/* switching signal paths and hardware setup */

/* TODO:

- maybe do all hardware init/setup in one go - as maybe problem with so many
- structures try also with BSRR or with GPIO_WriteBit(GPIO_TypeDef*
- GPIOx, uint16_t GPIO_Pin, BitAction BitVal);

- lmpwm is ringing (1-1000)

- clean up for functions below and test all combinations:

  functions:

- justfilter
- justdistortion
- filterthendistortion
- distortionthenfilter
- distortioninfilter on/off (could be done fast also)
- setfiltfeedbackpath - digital/lm13700
- feedback (leave for now)
- setfloating (enum list)
- setallfloating (or how to set diff ones but not just one)

- setlmpwm
- setmaximpwm
- set40106power

////

- test leave all hanging= GPIO_Mode_IN_FLOATING

///

so we have switchwise:

1on/2off/ ????of make sense-> /4/5/6which datagen?

INPUT/parallel/always: 

feedback on/off - jackin-> - lm358in->

1-feedon jackin xx
2-feedon xx     lmin
3-feedoff jackin xx
4-feedoff xx     lmin

// or do in all hanging?

5-hangfeed jackin xx
6-hangfeed xx     lmin
7-feedon hangin
8-feedoff hangin

421=8 options

first 3 bits (of 12)

supplemented by rest:

otherwise:

1-straightout
2-unhang all [where to re-hang?]+clocks question?

hang all, 

3-just40106 -> 40106 clock options=1hang,2-dg1,3-dg2,4-dg3

40106thenfilter-->|filterpath=1lm/2lmoneside/3digital-flagup!---distortioninfilter

lm - 1hang,2-dg1,3-dg2,4-dg3 * distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3
lmone - 1hang,2-dg1,3-dg2,4-dg3 * distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3
dig - nodist - 40106clock=1hang,2-dg1,3-dg2,4-dg3 * distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3

filterthen40106-->|- all clocks 

lm - 1hang,2-dg1,3-dg2,4-dg3 * distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3
lmone - 1hang,2-dg1,3-dg2,4-dg3 * distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3
dig - nodist - 40106clock=1hang,2-dg1,3-dg2,4-dg3 * distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3

-justfilter------->|

lm - 1hang,2-dg1,3-dg2,4-dg3 * distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3
lmone - 1hang,2-dg1,3-dg2,4-dg3 * distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3
dig - nodist - distortion - 40106clock=1hang,2-dg1,3-dg2,4-dg3


*/

#include "hardware.h"
#include "misc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_pwr.h"

extern __IO uint16_t adc_buffer[10];

int duty_cycle;

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t CCR4_Val = 333;
uint16_t PrescalerValue = 0;

uint16_t fakep;

void TIM_Config(void);


void TIM2_IRQHandler(void)
{
  static int flag=0;
if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
{
TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//GPIO_ToggleBits(GPIOC, GPIO_Pin_14);
 if (flag>10){
   GPIO_ResetBits(GPIOC, GPIO_Pin_14);
   if (flag>(10+fakep)) flag=0;
 }
 else GPIO_SetBits(GPIOC, GPIO_Pin_14);
 flag++;

}
}


/* what are pins we need to be switching?

- SW0 PB0 - out to 40106
- SW1 PC11 - out to filter
- SW2 PB2 - straightout to jack ***
- SW3 PB3 - filter to 40106
- SW4 PB4 - filter to jack
- SW5 PB5 - 40106 to filter
- SW6 PB6 - 40106 to jack

// 4053:

- SW7 PB7 - input
- SW8 PB8 - filterin
- SW9 PB9 - filterout
- feedbacksw-PC8
- filterdistort-PC10 1 to stop distortion

4053:

X0/jackin->X-14-LINEINR
X1/lm358

Y0/13700out->Y-15-filtin
Y1/LINEOUTL

Z0/13700in->Z-4-filtout
Z1/LINEINL

A-PB7=switchesX
B-PB8=Y
C-PB9=Z


are any of these swapped???

*/

/* first step switch - SW2 PB2 - out to jack ***/


#define JACKOUT (1 << 2) /* port B, pin 2 */
#define FILTIN (1 << 4) /* port B, pin 4 */
#define LINEINN (1 << 7) /* port B, pin 7 */

void setup_switches(void)
{

  //    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);    
  //    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);   


    	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  //	GPIOB->MODER |= (1 << (2 * 2)) | (1 << (4 * 2)) | (1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2)) | (1 << (6 * 2)) | (1 << (0 * 2)) ;	// JACK

	// try this way as PB4 has problem with JTRST - but somehow
	// this screws up rest settings sometimes
	// somehow funny one either/or works???

  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 ; // this works for pin 4 (should use for all pins?)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  //GPIO_SetBits(GPIOB, GPIO_Pin_4);
  	

	// switch off PC8/feedback???
  
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	//	GPIOC->MODER |= (GPIO_MODER_MODER8_0) | (GPIO_MODER_MODER11_0) | (GPIO_MODER_MODER10_0) ; // another way to do it

	GPIOC->MODER |= (1 << (8 * 2)) | (1 << (11 * 2)) | (1 << (10 * 2));
	GPIOC->ODR = 0;
}

void switchalloff(void)
{
  GPIOC->ODR = 0;
  GPIOB->ODR = 0;
}

void switch_jack(void)
{
  // clear first 3 and JACKOUT is on

  // try also with BSRR

  //  GPIO_SetBits(GPIOB, GPIO_Pin_2); 

  //  GPIO_SetBits(GPIOC, GPIO_Pin_8); // feedback on PC8

  GPIOB->ODR &= ~(7); // should clear more than this?
  GPIOB->ODR = JACKOUT;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

}


void test_filter(void)
{
  // connect the filter in signal path:

  //- SW1 PC11 - out to filter PC10 also 1

  GPIOC->ODR |= (1<<11) | (1<<10); //test distortion in filter = remove 1<<10

  GPIOB->ODR &= ~(7); ///???
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));

  //- SW4 PB4 - filter to jack

  GPIOB->ODR |= FILTIN;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in
  GPIOB->ODR |= (1<<8) | (1<<9);// test switch - puts left in/out across
  // without left in/out across we have ringing on LM!

}

void test_40106(void)
{



  //  GPIOC->ODR |= (1<<10); //test distortion in filter = remove 1<<10

  //- SW0 PB0 - out to 40106
  //- SW6 PB6 - 40106 to jack

  GPIOB->ODR &= ~(7); ///???
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));


  GPIOB->ODR = 1 | (1<<6);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

  // was |=

}

/* fill in rest of switch functions - also to leave all floating for
   pads as HW option FLOATIT */

void test_filtand40106(void)
{

  /*
- SW3 PB3 - filter to 40106
- SW6 PB6 - 40106 to jack
- SW1 PC11 - out to filter
  */

  // connect the filter in signal path:

  //- SW1 PC11 - out to filter PC10 also 1

  GPIOC->ODR |= (1<<11) | (1<<10);   //test distortion in filter = remove 1<<10


  //- SW4 PB3 and PB6

  GPIOB->ODR &= ~(7);
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));
  GPIOB->ODR |= (1<<8) | (1<<9);// test switch - puts left in/out across

  GPIOB->ODR |= (1<<3) | (1<<6);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in
}

void test_40106andfilt(void)
{
  /*
- SW0 PB0 - out to 40106
- SW4 PB4 - filter to jack
- SW5 PB5 - 40106 to filter
  */

  GPIOC->ODR |= (1<<10); //test distortion in filter = remove 1<<10


  GPIOB->ODR &= ~(7);
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));
  GPIOB->ODR |= (1<<8) | (1<<9);// test switch - puts left in/out across

  GPIOB->ODR |= (1<<0 | (1<<4)) | (1<<5);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

}


void retryagainpwm(void){ // THIS ONE WORKS! 

  TIM_Config();

  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // 28 MHz

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000; // was 665 was 2400 - this can change

  // 717 should give us 39 KHz if we start from 28 MHz

  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  TIM_OCStructInit(&TIM_OCInitStructure);


  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM3, &TIM_OCInitStructure);

  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);

  /* do TIM8 channel and enables */

  TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

  // new structure???

  TIM_OCStructInit(&TIM_OCInitStructure);

  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM8, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);

  TIM_CtrlPWMOutputs(TIM8, ENABLE);

  /* TIM8 enable counter */
  TIM_Cmd(TIM8, ENABLE);


}

void setlmpwm(uint16_t value, uint16_t value2){

  TIM3->CCR4 = value2;
  //  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // was by 28 MHz
  PrescalerValue = 2;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = value; // was 665
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  /* do TIM8 channel and enables */

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  // new structure???

  TIM_OCStructInit(&TIM_OCInitStructure);

  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 100;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM3, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);

  TIM_CtrlPWMOutputs(TIM3, ENABLE);

  /* TIM8 enable counter */
  TIM_Cmd(TIM3, ENABLE);



}

void setmaximpwm(uint16_t value){

  // uint32_t SystemCoreClock = 168000000;

  //  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // 28 MHz
  PrescalerValue = 2;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = value; // was 665
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  /* do TIM8 channel and enables */
  TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
    
  TIM_OCStructInit(&TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 100;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM8, &TIM_OCInitStructure);
      
  TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);
  
  TIM_CtrlPWMOutputs(TIM8, ENABLE);
  TIM_Cmd(TIM8, ENABLE);
  //  TIM8->CCR4 = value;
  

}

void TIM_Config(void) 
{

  /*
filterpwm-PB1 (is for lm13700)
filterclock-PC9 (is for maxim)

-PB1 can be TIM3_CH4, PC9 as TIM8_CH4, 

  */

  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* TIM8 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE); // APB2?

  /* GPIOC and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // AHB1???

  
  /* GPIOB Configuration:  TIM3 CH4 (PB1) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 

  /* Connect TIM3 pins to PB1 */  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);

  /* TIM8 pins to PC9 */  

  /* GPIOC Configuration: TIM8_CH4 (PC9) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ; //was UP
  GPIO_Init(GPIOC, &GPIO_InitStructure); 

  /* Connect TIM8 pins to PC9 */  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM8);

}

void setup40106power(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;


  PWR_BackupAccessCmd(ENABLE); // Enable access to LSE
  RCC_LSEConfig(RCC_LSE_OFF); // PC14 PC15 as GPIO
  //  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

NVIC_InitTypeDef NVIC_InitStructure;
/* Enable the TIM2 gloabal Interrupt */
NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);

TIM_OCStructInit(&TIM_OCInitStructure);
 
/* TIM2 clock enable */
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
/* Time base configuration */
 TIM_TimeBaseStructure.TIM_Period = 1; // 1000 - 1 // 1 MHz down to 1 KHz (1 ms)
 TIM_TimeBaseStructure.TIM_Prescaler =10-1;// 84 - 1; // was 84 - 1// 24 MHz Clock down to 1 MHz (adjust per your clock)

TIM_TimeBaseStructure.TIM_ClockDivision = 0;
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
/* TIM IT enable */
TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
/* TIM2 enable counter */
TIM_Cmd(TIM2, ENABLE);
}

void set40106power(uint16_t value){

  fakep=value;

}
