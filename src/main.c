/*

 Oh, eternity with outstretched wings, that broodest over the secret
 truths in whose roots lie the mysteries of man—his whence, his
 whither—have I searched thee, and struck a right key on thy dreadful
 organ!

 [Thomas de Quincey. The Dark Interpreter] 

*/

/*
 PATH=~/sat/bin:$PATH
 PATH=~/stm32f4/stlink/flash:$PATH
 make stlink_flash
*/

#define MAX_EXE_STACK 4
#define VILLAGE_SIZE (STACK_SIZE*2) // 128

#ifdef LACH
#define SETSIZE 32
#define SETSHIFT 11
#else
#define SETSIZE 64
#define SETSHIFT 10
#endif

#ifdef PCSIM
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "audio.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
#include "settings.h"
uint16_t *adc_buffer;
//typedef float float32_t;
u8 digfilterflag;
//int16_t src[BUFF_LEN], dst[BUFF_LEN];

int16_t *villager,*stacker,*stackery,*settingsarray;
int16_t *src, *dst;
u8 *village_effects;// [VILLAGE_SIZE/2];
u8 *settingsarrayattached; //64
u8 *settingsarrayinfected; //64

void  dohardwareswitch(u8 one,u8 two){
  // nothing
}

void set40106pwm(u16 one){
}

void setmaximpwm(u16 one){
}

void setlmpwm(u16 one, u16 two){
}

#else
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "codec.h"
#include "i2s.h"
#include "adc.h"
#include "audio.h"
#include "hardware.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
#include "settings.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

u16 villager[VILLAGE_SIZE];
u8 village_effects[STACK_SIZE]; // is half village_size
u16 stackery[STACK_SIZE*4]; // 64*4 MAX now
u16 stacker[STACK_SIZE*4]; // 64*4 MAX
u16 settingsarray[64];
#endif

// for knobwork
// TENE: 2,0,3,4,1 // else: 3,0,2,4,1

#ifdef TENE
#define FIRST 2
#define SECOND 0
#define THIRD 3
#define FOURTH 4
#define FIFTH 1
#define UP 6
#define DOWN 8
#define LEFT 5
#define RIGHT 7
#else
#define FIRST 3
#define SECOND 0
#define THIRD 2
#define FOURTH 4
#define FIFTH 1
#define UP 5
#define DOWN 6
#define LEFT 8
#define RIGHT 7
#endif

u8 EFFECTWRITE;
#ifdef LACH
u8 EFFECTREAD;
#endif

signed char direction[2]={-1,1};
u8 wormflag[10]={0,0,0,0,0,0,0,0,0,0};
u8 setwalk[8]={239,240,1,17,16,15,254,238}; 
u8 inp;
u16 villagestackpos=0;
u16 *buf16;

#define delay()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#define delayxx()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#ifndef PCSIM
extern int16_t audio_buffer[AUDIO_BUFSZ];
u8* datagenbuffer = (u8*)0x10000000;
#define randi() (adc_buffer[9]) // 12 bits
#else //PCSIM
#define randi() (rand()%4096)
u8* datagenbuffer;
extern int16_t* audio_buffer;
#endif
extern u8 digfilterflag;

//u8 testdirection;
u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array

u8 exestackpush(u8 exenum, u8* exestack, u8 exetype){
  if (exenum<MAX_EXE_STACK){
    exestack[exenum]=exetype;
    exenum++;
  }
  return exenum;
}

u8 exestackpop(u8 exenum){
  if (exenum>0){
    exenum--;
  }
  return exenum;
  }

u16 villagepush(u16 villagepos, u16 start, u16 wrap,u8 effect){
  if (villagepos<(VILLAGE_SIZE-2)) /// size -2
    {
      village_effects[villagepos/2]=effect;
      villager[villagepos++]=start;
      villager[villagepos++]=wrap;
}
  return villagepos;
}

u16 villagepop(u16 villagepos){
  if (villagepos>2)
    {
      villagepos-=2;
    }
  return villagepos;
}

u8 fingerdir(void){
  u8 handleft, handright, up=0,down=0,left=0,right=0;
  u8 handupp, handdown;
  u8 result=5;

  for (u8 x=0;x<8;x++){
  handupp=adc_buffer[UP]>>8; 
  handdown=adc_buffer[DOWN]>>8;
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handupp>2) up++;
  if (handdown>2) down++;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (up>4 && up>down && up>left && up>right) {
    result=0;
  }
  else if (down>4 && down>left && down>right) {
    result=2; 
  }
  else if (left>4 && left>right) {
    result=3;
  }
  else if (right>4) {
    result=1;
  }
  }
  return result;
}

u8 fingerdirr(void){

  u8 handleft, handright, up=0,down=0,left=0,right=0;
  u8 handupp, handdown;
  signed char result=0;

  for (u8 x=0;x<16;x++){
  handupp=adc_buffer[UP]>>8; 
  handdown=adc_buffer[DOWN]>>8;
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handupp>2) up++;
  if (handdown>2) down++;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (up>8 && up>down && up>left && up>right) {
    result=-8;
  }
  else if (down>8 && down>left && down>right) {
    result=+9; 
  }
  else if (left>8 && left>right) {
    result=-1;
  }
  else if (right>8) {
    result=1;
  }
  }
  return result;
}

u8 fingerdirleftright(void){

  u8 handleft, handright, left=0,right=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    return 0;
  }
  else if (right>8 && right>left) {
    return 1;
  }
  }
  return 2;
}

u8 fingerdirupdown(void){

  u8 handleft, handright, left=0,right=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;

  if (left>8 && left>right) {
    return 0;
  }
  else if (right>8 && right>left) {
    return 1;
  }
  }
  return 2;
}

signed char fingerdirupdownn(void){
  u8 handleft, handright, left=0,right=0;
  signed char result=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    result=-1;
  }
  else if (right>8 && right>left) {
    result=1;
  }
  }
  return result;
}

signed char fingerdirleftrightt(void){
  u8 handleft, handright, left=0,right=0;
  signed char result=0;

  for (u8 x=0;x<8;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>4 && left>right) {
    result=-1;
  }
  else if (right>4 && right>left) {
    result=1;
  }
  }
  return result;
}

signed char fingerdirleftrighttx(signed char vall){
  u8 handleft, handright, left=0,right=0;
  static signed char result=0; signed char tmp;

  for (u8 x=0;x<8;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>4 && left>right) {
    tmp=-1;
  }
  else if (right>4 && right>left) {
    tmp=1;
  }
  }
  //  return tmp;
  result+=tmp;
  if (result<=(-vall)) {
    result=0;
    tmp=-1;
  }
  else if (result>=vall) {
    result=0;
    tmp=1;
  }
  else tmp=0;
  return tmp;
}

signed char fingerdirupdownx(signed char vall){
  u8 handleft, handright, left=0,right=0;
  static signed char result=0; signed char tmp;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    tmp=-1;
  }
  else if (right>8 && right>left) {
    tmp=1;
  }
  }
  //  return tmp;
  result+=tmp;
  if (result<=(-vall)) {
    result=0;
    tmp=-1;
  }
  else if (result>=vall) {
    result=0;
    tmp=1;
  }
  else tmp=0;
  return tmp;
}

float32_t fingerdirleftrighttf(void){ 

  u8 handleft, handright, left=0,right=0;
  float32_t result=0.0f;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    result=-0.001f;  // TODO tuning
  }
  else if (right>8 && right>left) {
    result=0.001f;
  }
  }
  return result;
}


u16 fingervalup16bits(u16 tmpsetting, u8 inc){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;

  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) tmpsetting+=inc;
  else if (ttss<sstt) tmpsetting-=inc;
  return tmpsetting;
}

u8 fingervalup(u8 tmpsetting){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;

  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) tmpsetting+=1;
  else if (ttss<sstt) tmpsetting-=1;
  return tmpsetting;
}

u8 fingervaleff(u8 tmpsetting){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){
  handup=adc_buffer[RIGHT]>>8;
  handdown=adc_buffer[LEFT]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    if (tmpsetting>=8) tmpsetting=1;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==0) tmpsetting=7; // wraps
  }
  return tmpsetting;
}

u8 fingervalright(u8 tmpsetting, u8 wrap){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){ 
    handup=adc_buffer[RIGHT]>>8; // 4bits=16
    handdown=adc_buffer[LEFT]>>8;
    if (handup>2) ttss++;// was 8 - TODO: tweak
    else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    tmpsetting=tmpsetting%wrap;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==255) tmpsetting=wrap-1;
  }
  return tmpsetting;
}

u8 fingervalupwrap(u8 tmpsetting, u8 wrap){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    tmpsetting=tmpsetting%wrap;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==255) tmpsetting=wrap-1;
  }
  return tmpsetting;
}

////////////////////////////////////////////////////////////////////////////////

void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 coo,x,addr,tmp=0,tmphardware=0;u8 HARDWARE=0;
  u8 del=0,machine_count=0,tmpacht=0,villagepos=0,settingspos=0,m1flag=0,villagerdest,cpur; 
  u8 exestack[MAX_EXE_STACK];
  u16 tmper,foldy;

u8 settingsarrayinfected[64];
u8 settingsarrayattached[64];
u8 stackerattached[64];
u8 stackeryattached[64];
u8 villagerattached[128];
u8 villagereffattached[64];
u8 cpuattached[64];

  inittable(3,4,randi());
  //  wormdir=0;
  const float32_t pi= 3.141592;
  float32_t w;
  float32_t yi;
  float32_t phase=0;
  int sign_samp,i;
  inp=0; // STRAIGHT IN
  w= 2*pi;
  w= w/256;
  for (i = 0; i <= 256; i++)
    {
      yi= 32767*sinf(phase); // was 2047???
      phase=phase+w;
      sign_samp=32767+yi;     // dc offset
      sin_data[i]=sign_samp; // write value into array
    }

#ifndef PCSIM
  //	ADC1_Initonce();
  ADC1_Init((uint16_t *)adc_buffer);

#ifndef LACH
  setup_switches();
#endif

  // maintain order
  Audio_Init();
  Codec_Init(32000); // was 48000
  delay();

#ifndef LACH
  initpwm(); 	
#endif		

  I2S_Block_Init();
  I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);

#ifndef LACH
  dohardwareswitch(0,0);
#endif
#endif // for ifndef PCSIM

  machine *m=(machine *)malloc(sizeof(machine));
  m->m_threadcount=0;
  m->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk FIXED

#ifdef PCSIM
  datagenbuffer=(u8*)malloc(65536);
  audio_buffer=(int16_t*)malloc(32768*sizeof(int16_t));
  settingsarray=malloc(64*sizeof(int16_t));
  villager=malloc(VILLAGE_SIZE*sizeof(int16_t));
  stacker=malloc(4*64*sizeof(int16_t));
  stackery=malloc(4*64*sizeof(int16_t));
  adc_buffer=malloc(10*sizeof(int16_t));
  initaudio();
  srandom(time(0));
  src=malloc(BUFF_LEN*sizeof(int16_t));
  dst=malloc(BUFF_LEN*sizeof(int16_t));
  village_effects=malloc(VILLAGE_SIZE/2);
  settingsarrayattached=malloc(64);
  settingsarrayinfected=malloc(64);

  for (x=0;x<(BUFF_LEN);x++){
    src[x]=rand()%65536;
    dst[x]=rand()%65536;
  }

  for (x=0;x<32768;x++){
    audio_buffer[x]=rand()%65536;
  }
#endif

  u8 hwdel=0;
  u16 hwpos=0;
  signed char stack_pos=0;
  signed char stack_posy=0;
  u16 start,wrap;
  u8 exenums=0;

  struct stackey stackyy[STACK_SIZE];
  struct stackeyyy stackyyy[STACK_SIZE];
  buf16 = (u16*) datagenbuffer;
  u8 leakiness=randi()%255;
  u8 infection=randi()%255;

  for (x=0;x<64;x++){
    if ((rand()%255) > (adc_buffer[SECOND]>>4)) settingsarrayinfected[x]=1; // infected
    else settingsarrayinfected[x]=0;
  }

  // fill datagenbuffer???

  for (x=0;x<32768;x++){
     buf16[x]=randi()<<4;
    delayxx();
  }

  for (x=0;x<64;x++){
    settingsarrayattached[x]=0;
  }

  //****** setup code for walkers
#ifdef LACH
  for (x=0;x<6;x++){
    settingsarray[x]=0;
  }//start

  for (x=6;x<12;x++){
    settingsarray[x]=65535;
  }//wrap

  for (x=12;x<18;x++){
    settingsarray[x]=511; //>>8
  }//step

  for (x=18;x<20;x++){
    settingsarray[x]=511;
  }//speed

  EFFECTWRITE=0;
  EFFECTREAD=0;

  for (x=20;x<23;x++){
    settingsarray[x]=32768;
  }//fmods

  for (x=23;x<25;x++){
    settingsarray[x]=0; 
  }//expand

  for (x=25;x<31;x++){
    settingsarray[x]=32768;//>>15 = 1
  }//DIR and speed
#else
  for (x=0;x<14;x++){
    settingsarray[x]=0;
  }//start

  for (x=14;x<28;x++){
    settingsarray[x]=65535;
  }//wrap

  // new hardware offsets
  for (x=28;x<32;x++){
    settingsarray[x]=2;
  }

  for (x=32;x<42;x++){
    settingsarray[x]=511; //>>8
  }//step

  for (x=42;x<46;x++){
    settingsarray[x]=511;
  }//speed

  settingsarray[46]=0; // EFFROFFSET
  settingsarray[47]=0; // EFFFOFFSET

  EFFECTWRITE=0;

  for (x=48;x<51;x++){
    settingsarray[x]=32768;
  }//fmods

  for (x=51;x<54;x++){
    settingsarray[x]=0; 
  }//expand

  for (x=54;x<64;x++){
    settingsarray[x]=32768;//>>15 = 1
  }//DIR
#endif //lACH

  // CPUintrev3:
  for (x=0; x<64; x++) // was 100
    {
      addr=randi()<<3;
      cpustackpush(m,addr,randi()<<4,randi()%CPU_TOTAL,randi()%24); // was <<3
    }

  //pureleak

  for (x=0;x<100;x++){
    addr=randi()<<3;
    cpustackpushhh(datagenbuffer,addr,randi()<<4,randi()%CPU_TOTAL,randi()%24);// was <<3
  }

  // CA
  for (x=0;x<(STACK_SIZE);x++){
    //      start=0; wrap=32768; // TESTY!
          start=randi()<<4;
          wrap=randi()<4;
	  stack_posy=ca_pushn(stackyyy,randi()<<4,datagenbuffer,stack_posy,randi()<<4,start,wrap); 
  }

  //simulationforstack:	
    for (x=0;x<STACK_SIZE;x++){
  //  for (x=0;x<2;x++){ // TESTY!
                start=randi()<<4;
      //      start=0; wrap=32767; // TESTY!
            wrap=randi()<<4;
      stack_pos=func_pushn(stackyy,randi()<<4,buf16,stack_pos,randi()<<4,start,wrap);
            villagestackpos=villagepush(villagestackpos,start,wrap,randi()%16);
      //      villagestackpos=villagepush(villagestackpos,start,wrap,);
	//	        printf("TESTY:%d\n",wrap);
  }

    // execution stack - TESTER!
        for (x=0;x<MAX_EXE_STACK;x++){
	        exenums=exestackpush(exenums,exestack,randi()%5); //exetype=0-3 TESTY!
	  //      exenums=exestackpush(exenums,exestack,); //exetype=0-3 TESTY!
      }

	u8 mainmode,groupstart,groupwrap; signed char fingerspeed;
	u8 xx,cpupos,villageepos,dirpos,groupsel,attachpos,groupstartt,wormstart,wormpos,foldposy,foldpos,stackpos;
	u16 foldposl,settingsposl,datagenpos,stackerposl;

	    m->m_leakiness=leakiness;
	    m->m_infectprob=infection;
	    m->m_memory=datagenbuffer;

  while(1)
    {

#ifdef TEST_STRAIGHT
      // do nothing
#else

#ifdef TEST_EEG
      //write ADC9 into buf16
  for (x=0;x<32768;x++){
    buf16[x]=randi()<<4; // 16 bits
  }
#else

#ifdef PCSIM
      // randomise adc_buffer
      for (x=0;x<10;x++){
	adc_buffer[x]+=(randi()%4096);
	adc_buffer[x]=adc_buffer[x]%4096;
      }

      I2S_RX_CallBack(src, dst, BUFF_LEN/2); 
#endif
      
      for (x=0;x<exenums;x++){
	switch(exestack[x]){
	case 0:
	  func_runall(stackyy,stack_pos); // simulations
	  break;
	case 1:
	  ca_runall(stackyyy,stack_posy); // CA
	  break;
	case 2:
	  machine_count++;
	  if (machine_count>=MACHINESPEED){
	    machine_run(m); //cpu
	    	    machine_count=0;
	    	  }
	  break;
	case 3:
	    machine_runnn(datagenbuffer); // pureleak
	  break;
	case 4:
	  break;
	}
      }

      /////////////////////////////////////
	      
      //MODECODE      /////////////////////////////////////

      EFFECTWRITE=adc_buffer[FOURTH]>>6; // now 6 bits///=64 was7 bitsrest of effects as offsets///
      // top bit as wormcode
      mainmode=adc_buffer[FIRST]>>8; // 5 bits = 32 // TESTY! TODO! now 4 bits=16
      fingerspeed=((adc_buffer[FIRST]>>2)%32)+1; // 32/*32=1024 = 10 bits
      // also some other major setting on this
      //      mainmode=1; // testy!!!

      switch(mainmode){
#ifdef LACH // TODO!
      case 0:
	settingspos+=fingerdirleftrighttx(12);
	settingspos=settingspos%SETSIZE;
	xx=fingerdirupdown();
	// a hole
	if (xx==1){
	settingsarrayattached[settingspos]=0; 
	  settingsarray[settingspos]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	settingsarrayattached[settingspos]=0; 
	  settingsarray[settingspos]=adc_buffer[UP]<<4;
	}
	break;
#else
      case 0: // up/down/left/right as INPUT
	xx=fingerdir();
	if (xx!=5) inp=xx;
	break;
#endif

      case 1:
	/*
	  select stack (left/right) (knob and set)
	*/
	stackpos+=fingerdirleftrighttx(16);
	stackpos=stackpos%4;
	xx=fingerdirupdown();
	if (xx==1){
	  if (stackpos==0) m->m_threadcount=0;//adc_buffer[SECOND]>>6; // 6bits
	  else if (stackpos==1) stack_posy=adc_buffer[SECOND]>>6; // 6bits
	  else if (stackpos==2) stack_pos=adc_buffer[SECOND]>>6; // 6bits
	  else 
	    {
	      villagestackpos=(adc_buffer[SECOND]>>6)*2; // 6bits
	      if (villagestackpos==0) villagestackpos=2;
	    }
	}
	else if (xx==0){
	  if (stackpos==0) m->m_threadcount=0;//adc_buffer[UP]>>6; // 6bits
	  else if (stackpos==1) stack_posy=adc_buffer[UP]>>6; // 6bits
	  else if (stackpos==2) stack_pos=adc_buffer[UP]>>6; // 6bits
	  else {
	    villagestackpos=(adc_buffer[UP]>>6)*2; // 6bits
	    if (villagestackpos==0) villagestackpos=2;
	  }
	}
	  break;
      case 2: // directions - redone...
	dirpos=adc_buffer[SECOND]>>8; // 4 bits
#ifdef LACH
	dirpos=dirpos%6;
	xx=fingerdir();
	if (xx==0) wormflag[dirpos]=1; // up
	else if (xx==1) { settingsarray[25+dirpos]=(1<<15); wormflag[dirpos]=0;} //right
	else if (xx==3) { settingsarray[25+dirpos]=0; wormflag[dirpos]=0;} // left=0
	else if (xx==2) { settingsarray[25+dirpos]=adc_buffer[DOWN]<<4; wormflag[dirpos]=0;} 
#else
	dirpos=dirpos%10;
	xx=fingerdir();
	if (xx==0) wormflag[dirpos]=1; // up
	else if (xx==1) { settingsarray[54+dirpos]=(1<<15); wormflag[dirpos]=0;} //right
	else if (xx==3) { settingsarray[54+dirpos]=0; wormflag[dirpos]=0;} // left=0
	else if (xx==2) { settingsarray[54+dirpos]=adc_buffer[DOWN]<<4; wormflag[dirpos]=0;} 
#endif
	break; 


      case 3: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupwrap=adc_buffer[SECOND]>>6; // 6bits
	// 6 bits groupstart by way of finger LEFT/RIGHT
	groupstart+=fingerdirleftrighttx(8);
	groupstart=groupstart%SETSIZE;
	groupsel+=fingerdirupdownx(64); // TODO-redo as leftrighttx
	groupsel=groupsel%4;

	for (x=0;x<groupwrap;x++){
	switch(groupsel){
	case 0:
	  settingsarray[(groupstart+x)%SETSIZE]=buf16[((FOLDOFFSET>>1)+(coo%((groupwrap)+1)))%32768];
	  settingsarrayattached[(groupstart+x)%SETSIZE]=1;//up
	  coo++;
	  break;
	case 1:
#ifdef TENE
	  settingsarray[(groupstart+x)%SETSIZE]=adc_buffer[9]<<4;//right
#else
	  settingsarray[(groupstart+x)%SETSIZE]=adc_buffer[DOWN]<<4;
#endif
	  settingsarrayattached[(groupstart+x)%SETSIZE]=2;
	  break;
	case 2:
	  settingsarray[(groupstart+x)%SETSIZE]=adc_buffer[THIRD]<<4;
	  settingsarrayattached[(groupstart+x)%SETSIZE]=3;
	  break;
	case 3: 
	  // DEtach
	  settingsarrayattached[(groupstart+x)%SETSIZE]=0;
	  break;
	}
	}
	break;

      case 4: //SETTED! 	//expand for 4stacker/5stackery//6CPU//7villager/8village_effects
	groupwrap=adc_buffer[SECOND]>>6; // 6bits
	// 6 bits groupstart by way of finger LEFT/RIGHT
	groupstart+=fingerdirleftrighttx(8);
	groupstart=groupstart%STACK_SIZE;
	groupsel+=fingerdirupdownx(64); // TODO-redo as leftrighttx
	groupsel=groupsel%4;

	for (x=0;x<groupwrap;x++){
	switch(groupsel){
	case 0:
	  stackerattached[(groupstart+x)%SETSIZE]=1;//up
	  coo++;
	  break;
	case 1:
	  stackerattached[(groupstart+x)%SETSIZE]=2;
	  break;
	case 2:
	  stackerattached[(groupstart+x)%SETSIZE]=3;
	  break;
	case 3: 
	  // DEtach
	  stackerattached[(groupstart+x)%SETSIZE]=0;
	  break;
	}
	}
	break;

      case 5: //SETTED! 	//expand for 4stacker/5stackery//6CPU//7villager/8village_effects
	groupwrap=adc_buffer[SECOND]>>6; // 6bits
	// 6 bits groupstart by way of finger LEFT/RIGHT
	groupstart+=fingerdirleftrighttx(8);
	groupstart=groupstart%STACK_SIZE;
	groupsel+=fingerdirupdownx(64); // TODO-redo as leftrighttx
	groupsel=groupsel%4;

	for (x=0;x<groupwrap;x++){
	switch(groupsel){
	case 0:
	  stackeryattached[(groupstart+x)%SETSIZE]=1;//up
	  coo++;
	  break;
	case 1:
	  stackeryattached[(groupstart+x)%SETSIZE]=2;
	  break;
	case 2:
	  stackeryattached[(groupstart+x)%SETSIZE]=3;
	  break;
	case 3: 
	  // DEtach
	  stackeryattached[(groupstart+x)%SETSIZE]=0;
	  break;
	}
	}
	break;

      case 6: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupwrap=adc_buffer[SECOND]>>5; // 7bits
	// 6 bits groupstart by way of finger LEFT/RIGHT
	groupstart+=fingerdirleftrighttx(8);
	groupstart=groupstart%(STACK_SIZE);
	groupsel+=fingerdirupdownx(64); // TODO-redo as leftrighttx
	groupsel=groupsel%4;

	for (x=0;x<groupwrap;x++){
	switch(groupsel){
	case 0:
	  cpuattached[(groupstart+x)%SETSIZE]=1;//up
	  coo++;
	  break;
	case 1:
	  cpuattached[(groupstart+x)%SETSIZE]=2;
	  break;
	case 2:
	  cpuattached[(groupstart+x)%SETSIZE]=3;
	  break;
	case 3: 
	  // DEtach
	  cpuattached[(groupstart+x)%SETSIZE]=0;
	  break;
	}
	}
	break;

      case 7: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupwrap=adc_buffer[SECOND]>>5; // 7bits
	// 6 bits groupstart by way of finger LEFT/RIGHT
	groupstart+=fingerdirleftrighttx(8);
	groupstart=groupstart%(STACK_SIZE*2);
	groupsel+=fingerdirupdownx(64); // TODO-redo as leftrighttx
	groupsel=groupsel%4;

	for (x=0;x<groupwrap;x++){
	switch(groupsel){
	case 0:
	  villagerattached[(groupstart+x)%SETSIZE]=1;//up
	  coo++;
	  break;
	case 1:
	  villagerattached[(groupstart+x)%SETSIZE]=2;
	  break;
	case 2:
	  villagerattached[(groupstart+x)%SETSIZE]=3;
	  break;
	case 3: 
	  // DEtach
	  villagerattached[(groupstart+x)%SETSIZE]=0;
	  break;
	}
	}
	break;

      case 8: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupwrap=adc_buffer[SECOND]>>5; // 7bits
	// 6 bits groupstart by way of finger LEFT/RIGHT
	groupstart+=fingerdirleftrighttx(8);
	groupstart=groupstart%(STACK_SIZE*2);
	groupsel+=fingerdirupdownx(64); // TODO-redo as leftrighttx
	groupsel=groupsel%4;

	for (x=0;x<groupwrap;x++){
	switch(groupsel){
	case 0:
	  villagereffattached[(groupstart+x)%SETSIZE]=1;//up
	  coo++;
	  break;
	case 1:
	  villagereffattached[(groupstart+x)%SETSIZE]=2;
	  break;
	case 2:
	  villagereffattached[(groupstart+x)%SETSIZE]=3;
	  break;
	case 3: 
	  // DEtach
	  villagereffattached[(groupstart+x)%SETSIZE]=0;
	  break;
	}
	}
	break;

	///////////////////////////////////////////
	// swops now 
	// settingsarray<->settingsarray
	// villager<->villager
	// stacker/y<->villager and vice versa

      case 9:
#ifdef LACH
	foldy=adc_buffer[SECOND]>>7; // howmuch-32
#else
	foldy=adc_buffer[SECOND]>>6; // howmuch-64
#endif
	foldposy+=fingerdirupdownx(8); 
	foldposy=foldposy%SETSIZE;
	foldpos+=fingerdirleftrighttx(8);
	foldpos=foldpos%SETSIZE;
	for (x=0;x<(foldy);x++){
	  settingsarray[(foldpos+x)%SETSIZE]=settingsarray[(foldpos+foldposy+x)%SETSIZE];
	}
	break;

      case 10:
	foldy=adc_buffer[SECOND]>>5; // howmuch-128
	foldposy+=fingerdirupdownx(8);
	foldposy=foldposy%128;
	foldpos+=fingerdirleftrighttx(8);
	foldpos=foldpos%64;
	for (x=0;x<(foldy);x++){
	  villager[(foldpos+x)%VILLAGE_SIZE]=villager[(foldpos+foldposy+x)%VILLAGE_SIZE];
	}
	break;
	///
      case 11: // various stack and villager exchanges - 
	// starts and ends only of stacks (not CPU) -> villagers
	
	foldy=adc_buffer[SECOND]>>6; // howmuch-64
	foldposy+=fingerdirupdownx(8);
	foldposy=foldposy%64;
	foldpos+=fingerdirleftrighttx(8);
	foldpos=foldpos%128;
	///////
	for (x=0;x<(foldy);x++){ // 64 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point // 7 bits=128
	  villagerdest=(((foldpos)>>1)+x+foldposy)%(VILLAGE_SIZE/2); // village entry
	  if (tmper<STACK_SIZE){
	    // deal with stacker
	    villager[villagerdest*2]=stacker[tmper*4];  
	    villager[(villagerdest*2)+1]=stacker[(tmper*4)+1];  
	  }
	  else {
	    tmper-=STACK_SIZE;
	    // deal with stackery
	    villager[villagerdest*2]=stackery[tmper*4];  
	    villager[(villagerdest*2)+1]=stackery[(tmper*4)+1];  
	  }
	}
	break;

      case 12: // various stack and villager exchanges - 
	// other way round
	
	foldy=adc_buffer[SECOND]>>5; // howmuch-128
	foldposy+=fingerdirupdownx(8); // TODO-redo as leftrighttx
	foldposy=foldposy%64;
	foldpos+=fingerdirleftrighttx(8); // 16 bits
	foldpos=foldpos%128;

	for (x=0;x<(foldy);x++){ 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point // 7 bits=128
	  villagerdest=(((foldpos)>>1)+x+foldposy)%(VILLAGE_SIZE/2); // village entry

	  if (tmper<STACK_SIZE){
	    // deal with stacker
	    stacker[tmper*4]=villager[villagerdest*2];  
	    stacker[(tmper*4)+1]=villager[(villagerdest*2)+1];  
	  }
	  else {
	    tmper-=STACK_SIZE;
	    // deal with stackery
	    stackery[tmper*4]=villager[villagerdest*2];  
	    stackery[(tmper*4)+1]=villager[(villagerdest*2)+1];  
	  }
	}
	break;

      case 13: // dump (all) to datagen//back
	// test leftrightt	settingsarray[15]+=fingerdirleftrightt(); // 16 bits
	
	foldy=adc_buffer[SECOND]>>2; // howmuch=10 bits=1024
	foldpos+=fingerdirleftrightt(); // 16 bits

	xx=fingerdirupdown();
	if (xx==1) { //down

	for (x=0;x<foldy;x++){ // 10 bits
#ifdef LACH
	  tmper=((foldpos>>6)+x)%736; // full house//10 bits=1024
	  if (tmper<32) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<288) buf16[(foldpos+x)%32768]=stacker[tmper-32];
	  else if (tmper<544) buf16[(foldpos+x)%32768]=stackery[tmper-288];
	  else if (tmper<608) buf16[(foldpos+x)%32768]=m->m_threads[tmper-544].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-608];
#else
	  tmper=((foldpos>>6)+x)%768; // full house//10 bits=1024
	  if (tmper<64) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<320) buf16[(foldpos+x)%32768]=stacker[tmper-64];
	  else if (tmper<576) buf16[(foldpos+x)%32768]=stackery[tmper-320];
	  else if (tmper<640) buf16[(foldpos+x)%32768]=m->m_threads[tmper-576].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-640];
#endif
	}
	}
	else if (xx==0) {
	for (x=0;x<foldy;x++){ // 10 bits
#ifdef LACH
	  tmper=((foldpos>>6)+x)%736; // full house//10 bits=1024
	  if (tmper<32) settingsarray[tmper]=buf16[(foldpos+x)%32768];
	  else if (tmper<288) stacker[tmper-32]=buf16[(foldpos+x)%32768];
	  else if (tmper<544) stackery[tmper-288]buf16[(foldpos+x)%32768];
	  else if (tmper<608) m->m_threads[tmper-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	    else villager[tmper-608]buf16[(foldpos+x)%32768];
#else
	  tmper=((foldpos>>6)+x)%768; // full house//10 bits=1024
	  if (tmper<64) settingsarray[tmper]=buf16[(foldpos+x)%32768];
	  else if (tmper<320) stacker[tmper-64]=buf16[(foldpos+x)%32768];
	  else if (tmper<576) stackery[tmper-320]=buf16[(foldpos+x)%32768];
	  else if (tmper<640) m->m_threads[tmper-576].m_CPU=buf16[(foldpos+x)%32768]>>11;
	    else villager[tmper-640]=buf16[(foldpos+x)%32768];
#endif
	}
	}
	break;
	////////////////
      case 14: // infection
	///infection across buffer: knobs; speed,probability,buffer
	//set according to probability
	if ((adc_buffer[SECOND]>>5)==0){
	  for (x=0;x<SETSIZE;x++){
	    if ((rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x]=1; // infected
	  else settingsarrayinfected[x]=0;
	  } // reset!

	  // run infection at speed eff[0] 

	  for (x=0;x<SETSIZE;x++){
	    // infection - how many infected (not dead) round each one?
	    if (++del==(adc_buffer[SECOND]>>5)){
	      tmpacht=(x-1)%SETSIZE;
		if (settingsarrayinfected[x]==0 && ((settingsarrayinfected[tmpacht]>=1 && settingsarrayinfected[tmpacht]<128) || (settingsarrayinfected[(x+1)%SETSIZE]>=1 && settingsarrayinfected[(x+1)%SETSIZE]<128)) && (rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x]=1;
	    // inc
	    if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128) settingsarrayinfected[x]++;
	    del=0;
	    }

	  // overmap onto buffer eff[0]: 0=stay same/infect=reduce by days/dead=128=zero
	    //0/settingsarray 1/villager 2/3/4//stacksandCPU 5/foldback	    
	    switch(adc_buffer[SECOND]>>9) // 8 cases
	      {
	      case 0:
	      default:
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	settingsarray[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) settingsarray[x]=0;
		break;
	      case 1:
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	stacker[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) stacker[x]=0;
		break;
	      case 2:
 		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	stackery[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) stackery[x]=0;
		break;
	      case 4:
 		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	{
		  cpur=x%(m->m_threadcount);
		  m->m_threads[cpur].m_CPU-=settingsarrayinfected[x];//cpu
		  m->m_threads[cpur].m_CPU%=31;
		}
		else if (settingsarrayinfected[x]>127) {
		  m->m_threads[x%(m->m_threadcount)].m_CPU=0;//cpu
		}
		break;
		/*	      case 5:
		if (x<14){
		  if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	FOLDD[x]-=settingsarrayinfected[x]; // foldd max
		else if (settingsarrayinfected[x]>127) FOLDD[x%FOLD_SIZE]=0; // foldd max
		}
		break;*/
	      }
	    /////
	  }
	}
	break;
	// TODO: add one more mode!!!
      }

      //END MODECODE      /////////////////////////////////////
      /// DEAL with settingsattach and other attachs....

      for (x=0;x<SETSIZE;x++){
	switch(settingsarrayattached[x]){
	case 0:
	  break;
	case 1:
	  settingsarray[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  settingsarray[x]=adc_buffer[9]<<4;
#else
	  settingsarray[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  settingsarray[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  settingsarray[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<64;x++){
	switch(stackerattached[x]){
	case 0:
	  break;
	case 1:
	  stacker[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  stacker[x]=adc_buffer[9]<<4;
#else
	  stacker[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  stacker[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  stacker[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<64;x++){
	switch(villagereffattached[x]){
	case 0:
	  break;
	case 1:
	  village_effects[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  village_effects[x]=adc_buffer[9]<<4;
#else
	  village_effects[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  village_effects[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  village_effects[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<128;x++){
	switch(villagerattached[x]){
	case 0:
	  break;
	case 1:
	  villager[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  villager[x]=adc_buffer[9]<<4;
#else
	  villager[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  villager[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  villager[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }


      for (x=0;x<64;x++){
	switch(stackerattached[x]){
	case 0:
	  break;
	case 1:
	  stacker[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  stacker[x]=adc_buffer[9]<<4;
#else
	  stacker[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  stacker[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  stacker[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }




      //// DEAL last with hardware:
#ifdef LACH
      // TODO! as EFFECTREAD!
      EFFECTREAD=adc_buffer[FIFTH]>>6; // 6 bits
#else
      tmphardware=0;
      for (x=0;x<256;x++){ // was 256
	tmphardware+=adc_buffer[FIFTH]>>7; // 5 bits now!
      }
      HARDWARE=tmphardware>>8; //was >>8 to divide average
#endif
      /////////////////////////////////////
      //// MODE/KNOB CODE END

#ifndef LACH
      /////////////////////////////////////
      // 4-hardware operations
      
      /// general HW walk in/as tmp
      if (++hwdel>=HWSPEED){
	if (wormflag[0]) hwpos+=direction[wormdir];
	    else hwpos+=(HWSTEP*direction[HWDIR]);
	tmp=HWSTART+(hwpos%HWWRAP); //to cover all directions
	hwdel=0;
      }

      if (digfilterflag&16){
	//	if (HDGENERCONS==0) settingsarray[24]=256; //SET HDGENERCONS=1
	dohardwareswitch(HARDWARE,HDGENERBASE+(datagenbuffer[tmp]%HDGENERCONS));
      }
      else
	{
	  dohardwareswitch(HARDWARE,0);
	}
	     		   
      // just leave this running
		     		     
      set40106pwm(F0106ERBASE+(buf16[(tmp+F0106EROFFSET)%32768]%F0106ERCONS)); // constrain all to base+constraint

      if (digfilterflag&4){
	setlmpwm(LMERBASE+(buf16[(tmp+LMEROFFSET)%32768]%LMERCONS),LMERBASE+(buf16[(tmp+LMEROFFSETTWO)%32768]%LMERCONS)); 
      }
	  
      if (digfilterflag&8){
	setmaximpwm(MAXIMERBASE+(buf16[(tmp+MAXIMEROFFSET)%32768]%MAXIMERCONS));
      }
     
#endif
#endif
#endif
      //#endif 
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
