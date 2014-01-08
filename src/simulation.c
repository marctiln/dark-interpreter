// gcc simulation.c -osim -lm
// was datagentest.c previously

/* All simulation data generators: IFS, rossler, secondrossler, fitz,
   oregon, spruce, brussel, simpleSIR, sier, */

//+ inc,dec,left,right and so on

/* 

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301 USA

Based in part on SLUGens by Nicholas Collins.

*/

#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#else
#include <malloc.h>
#include <math.h>
#define rand() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
#endif

#include "simulation.h"

/* TODO:

- clean ups for inits/each function and test all
- does NaN cause problems or not?

- check memory use! also use of free and how we will do all inits

/////resolved/////

[- somehow declare offset for settings, or we store this somewhere]
[- should howmuch be int or is u8 ok? probably leave as u8]
[- return count+i or i? - count+i in all cases]

*/

//////////////////////////////////////////////////////////

// sine

void sineinit(struct siney* unit, uint16_t *workingbuffer){
  unit->del=unit->cc=0;
  float pi= 3.141592;
  float w;    // ψ
  float yi;
  float phase;
  int sign_samp,i;
  w= 2*pi;
  w= w/256;
    for (i = 0; i <= 256; i++)
    {
      yi= 2047*sinf(phase);
      phase=phase+w;
      sign_samp=2047+yi;     // dc offset translated for a 12 bit DAC
      unit->sin_data[i]=sign_samp; // write value into array
    }
}

uint16_t runsine(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct siney* unit){
  u8 i=0;
  if (unit->del++==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i]=unit->sin_data[unit->cc%256];
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+(u16)i]);
#endif
    unit->cc++;
  }
    unit->del=0;
  }
  return count+i;
}


//////////////////////////////////////////////////////////

// generic arithmetik datagens 

void geninit(struct generik* unit, uint16_t *workingbuffer){
  unit->del=0;
  unit->cop=workingbuffer[0]; 
}

uint16_t runinc(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i]=unit->cop++;
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+(u16)i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}

uint16_t rundec(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+i]=unit->cop--;
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}

uint16_t runleft(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i]=workingbuffer[count+(u16)i]<<=1;
#ifdef PCSIM
    printf("%d %d\n",count+(u16)i,workingbuffer[count+(u16)i]);
#endif
  }
  unit->del=0;
  }
  return count+(u16)i;
}

uint16_t runright(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i]=workingbuffer[count+(u16)i]>>=1;
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}

uint16_t runswap(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0; u16 temp;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    temp=workingbuffer[count+(u16)i];
    workingbuffer[count+(u16)i]=workingbuffer[count+(u16)i+1];
    workingbuffer[count+(u16)i+1]=temp;
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}

uint16_t runnextinc(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i]=workingbuffer[count+(u16)i]+1;
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}

uint16_t runnextdec(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i]=workingbuffer[count+(u16)i]-1;
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}

uint16_t runnextmult(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i]*=workingbuffer[count+(u16)i+1];
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}

uint16_t runnextdiv(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    if ((workingbuffer[count+(u16)i+1])>0)   workingbuffer[count+(u16)i]/=workingbuffer[count+(u16)i+1];
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}


uint16_t runcopy(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i+1]=workingbuffer[count+(u16)i];
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}

uint16_t runzero(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct generik* unit){
  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    workingbuffer[count+(u16)i+1]=0;
#ifdef PCSIM
    printf("%d\n",workingbuffer[count+i]);
#endif
  }
    unit->del=0;
  }
  return count+i;
}



//////////////////////////////////////////////////////////

// first SIR:

/*

This is the C version of program 2.1 from page 19 of
"Modeling Infectious Disease in humans and animals"
by Keeling & Rohani.

It is the simple SIR epidemic without births or deaths.

see also: http://homepages.warwick.ac.uk/~masfz/ModelingInfectiousDiseases/

 */

void Diff(struct simpleSIR* unit,float Pop[3])
{
  float tmpS, tmpI, tmpR;
  tmpS=Pop[0]; tmpI=Pop[1]; tmpR=Pop[2];

  unit->dPop[0] = - unit->beta*tmpS*tmpI;              // dS/dt
  unit->dPop[1] = unit->beta*tmpS*tmpI - unit->gamm*tmpI;   // dI/dt
  unit->dPop[2] = unit->gamm*tmpI;                    // dR/dt
}

void Runge_Kutta(struct simpleSIR* unit)
{
  unsigned char i;
  float dPop1[3], dPop2[3], dPop3[3], dPop4[3];
  float tmpPop[3], initialPop[3];

  /* Integrates the equations one step, using Runge-Kutta 4
     Note: we work with arrays rather than variables to make the
     coding easier */

  initialPop[0]=unit->S; initialPop[1]=unit->I; initialPop[2]=unit->R;

  Diff(unit,initialPop);
  for(i=0;i<3;i++)
    {
      dPop1[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop1[i]/2;
    }

  Diff(unit,tmpPop);
  for(i=0;i<3;i++)
    {
      dPop2[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop2[i]/2;  
    }

  Diff(unit,tmpPop);
  for(i=0;i<3;i++)
    {
      dPop3[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop3[i]; 
    }

  Diff(unit,tmpPop);

  for(i=0;i<3;i++)
    {
      dPop4[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+(dPop1[i]/6 + dPop2[i]/3 + dPop3[i]/3 + dPop4[i]/6)*unit->step;
    }

  unit->S=tmpPop[0]; unit->I=tmpPop[1]; unit->R=tmpPop[2];
  //printf("%g %g %g\n",unit->S,unit->I,unit->R);

  return;
}

void simplesirinit(struct simpleSIR* unit, uint16_t *workingbuffer){

  //  unit->t=0;

  //TODO: init with workingbuffer

  //  unit->beta=520.0/365.0;
  //  unit->gamm=1.0/7.0;
  unit->beta=(float)workingbuffer[0]/65536.0;
  unit->gamm=(float)workingbuffer[1]/65536.0;
  unit->S0=1.0-1e-6;
  unit->I0=1e-6;
  unit->step=0.01/((unit->beta+unit->gamm)*unit->S0);
  unit->S=unit->S0; unit->I=unit->I0; unit->R=1-unit->S-unit->I;
  // what else in init?
}

uint16_t runsimplesir(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct simpleSIR* unit){

  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    Runge_Kutta(unit);//  unit->t+=step;
    workingbuffer[count+i]=unit->I;
    //    printf("%c",unit->I);
    
  }
  unit->del=0;
  }
  return count+i;

}

//////////////////////////////////////////////////////////

// SEIR. SIR

void seirinit(struct SEIR* unit, uint16_t *workingbuffer){
  unsigned char i;
  //unit->beta=17/5;
  //unit->gamm=1.0/13;
  unit->beta=(float)workingbuffer[0]/65536.0;
  unit->gamm=(float)workingbuffer[1]/65536.0;
  unit->n=13;
  unit->m=8;
  unit->mu=1.0/(55*365);
  unit->S0=0.05;
  unit->I0=0.00001;
  unit->step=0.01/(unit->beta+unit->gamm*unit->n+unit->mu);

  unit->S=unit->S0;
  for(i=0;i<unit->n;i++)
    {
      unit->I[i]=unit->I0/unit->n;
    }

}

void seirDiff(struct SEIR* unit,float Pop[MAX_GROUPS+1])
{
  int i;
  float Inf, tmpS;

  /* Set up some temporary variables to make things easier.
     Note I_i = Pop[i]  */
  
  tmpS=Pop[0];
  Inf=0;
  for(i=unit->m+1;i<=unit->n;i++)
    {
      Inf+=Pop[i];
    }
  
  unit->dPop[0]= unit->mu - unit->beta*Inf*tmpS - unit->mu*tmpS;
  unit->dPop[1]= unit->beta*Inf*tmpS - unit->gamm*unit->n*Pop[1] - unit->mu*Pop[1];

  for(i=2;i<=unit->n;i++)
    {
      unit->dPop[i]= unit->gamm*unit->n*Pop[i-1] - unit->gamm*unit->n*Pop[i] - unit->mu*Pop[i];
    }

  return;
}

void seir_Runge_Kutta(struct SEIR* unit)
{
  int i;
  float dPop1[MAX_GROUPS], dPop2[MAX_GROUPS], dPop3[MAX_GROUPS], dPop4[MAX_GROUPS];
  float tmpPop[MAX_GROUPS],InitialPop[MAX_GROUPS];

  /* Integrates the equations one step, using Runge-Kutta 4
     Note: we work with arrays rather than variables to make the
     coding easier */

  InitialPop[0]=unit->S;
  for(i=0;i<unit->n;i++)
    {
      InitialPop[i+1]=unit->I[i];
    }

  seirDiff(unit,InitialPop);
  for(i=0;i<=unit->n;i++)
    {
      dPop1[i]=unit->dPop[i];
      tmpPop[i]=InitialPop[i]+unit->step*dPop1[i]/2;
    }

  seirDiff(unit,tmpPop);
  for(i=0;i<=unit->n;i++)
    {
      dPop2[i]=unit->dPop[i];
      tmpPop[i]=InitialPop[i]+unit->step*dPop2[i]/2;  
    }

  seirDiff(unit,tmpPop);
  for(i=0;i<=unit->n;i++)
    {
      dPop3[i]=unit->dPop[i];
      tmpPop[i]=InitialPop[i]+unit->step*dPop3[i]; 
    }

  seirDiff(unit,tmpPop);

  for(i=0;i<unit->n;i++)
    {
      dPop4[i+1]=unit->dPop[i+1];
      unit->I[i]=unit->I[i]+(dPop1[i+1]/6 + dPop2[i+1]/3 + dPop3[i+1]/3 + dPop4[i+1]/6)*unit->step;
    }
  dPop4[0]=unit->dPop[0];
  unit->S=unit->S+(dPop1[0]/6 + dPop2[0]/3 + dPop3[0]/3 + dPop4[0]/6)*unit->step;

  return;
}


uint16_t runseir(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct SEIR* unit){

  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    seir_Runge_Kutta(unit);//  unit->t+=step;
    workingbuffer[count+i]=unit->S;
    //    printf("%c",unit->S);
  }
  unit->del=0;
  }
  return count+i;
}

//////////////////////////////////////////////////////////

// SICR. SIR

void sicrinit(struct SICR* unit, uint16_t *workingbuffer){
  //unit->beta=0.2;
  //unit->epsilon=0.1;
//unit->gamm=1.0/100.0;
//unit->Gamm=1.0/1000.0;
unit->beta=(float)workingbuffer[0]/65536.0;
unit->epsilon=(float)workingbuffer[1]/655360.0;
unit->gamm=(float)workingbuffer[2]/655360.0;
unit->Gamm=(float)workingbuffer[3]/655360.0;
unit->mu=1.0/(50.0*365.0);
unit->q=0.4;
unit->S0=0.1;
unit->I0=1e-4;
unit->C0=1e-3;

unit->S=unit->S0; unit->I=unit->I0; unit->C=unit->C0; unit->R=1-unit->S-unit->I-unit->C0;
unit->step=0.01/((unit->beta+unit->gamm+unit->mu+unit->Gamm)*unit->S0);

}

void sicrdiff(struct SICR* unit,float Pop[3])
{
  float tmpS, tmpI, tmpC;
  tmpS=Pop[0]; tmpI=Pop[1]; tmpC=Pop[2];
  unit->dPop[0] = unit->mu - unit->beta*tmpS*(tmpI + unit->epsilon*tmpC) - unit->mu*tmpS;
  unit->dPop[1] = unit->beta*tmpS*(tmpI + unit->epsilon*tmpC) - unit->gamm*tmpI -unit->mu*tmpI;
  unit->dPop[2] = unit->gamm*unit->q*tmpI - unit->Gamm*tmpC - unit->mu*tmpC;
  return;
}

void sicr_Runge_Kutta(struct SICR* unit)
{
  u8 i=0;
  float dPop1[3], dPop2[3], dPop3[3], dPop4[3];
  float tmpPop[3], initialPop[3];

  initialPop[0]=unit->S; initialPop[1]=unit->I; initialPop[2]=unit->C;

  sicrdiff(unit,initialPop);
  for(i=0;i<3;i++)
    {
      dPop1[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop1[i]/2;
    }

  sicrdiff(unit,tmpPop);
  for(i=0;i<3;i++)
    {
      dPop2[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop2[i]/2;  
    }

  sicrdiff(unit,tmpPop);
  for(i=0;i<3;i++)
    {
      dPop3[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop3[i]; 
    }

  sicrdiff(unit,tmpPop);

  for(i=0;i<3;i++)
    {
      dPop4[i]=unit->dPop[i];

      tmpPop[i]=initialPop[i]+(dPop1[i]/6 + dPop2[i]/3 + dPop3[i]/3 + dPop4[i]/6)*unit->step;
    }


  unit->S=tmpPop[0]; unit->I=tmpPop[1]; unit->C=tmpPop[2];  unit->R=1-unit->S-unit->I-unit->C;
 
  return;
}

uint16_t runsicr(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct SICR* unit){

  u8 i=0;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    sicr_Runge_Kutta(unit);//  unit->t+=step;
    workingbuffer[count+i]=unit->S;
    //    printf("%d %d\n",count+i,unit->S);
  }
  unit->del=0;
  }
  return count+i;
}




//////////////////////////////////////////////////////////

// IFS

void ifsinit(struct IFS* unit, uint16_t *workingbuffer){
  u8 i,iter;
  u8 column = 6, row = 4;
  unit->p1.x=0.1;
  unit->p1.y=0.1;         

  for (iter=0;iter<row;iter++){
    for (i=0;i<column;i++){
      //      iter=rand()%row;
      //      i=rand()%column;

#ifdef PCSIM
      unit->coeff[iter][i]=((float)rand()/(float)(RAND_MAX));
      if (((float)rand()/(float)(RAND_MAX))>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
      unit->prob[iter]=((float)rand()/(float)(RAND_MAX));
#else
      unit->coeff[iter][i]=((float)rand()/4096.0f);
      if (((float)rand()/4096.0f)>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
      unit->prob[iter]=((float)rand()/4096.0f);
#endif

  unit->prob[0]=(float)workingbuffer[0]/65536.0;
  unit->prob[1]=(float)workingbuffer[1]/65536.0;
  unit->prob[2]=(float)workingbuffer[2]/65536.0;
  unit->prob[3]=(float)workingbuffer[3]/65536.0;
  unit->prob[4]=(float)workingbuffer[4]/65536.0;
    }
  }
}

uint16_t runifs(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct IFS* unit){

  float random_num;
  u8 iter,i,it,x;
  u8 column = 6, row = 4;

  /*  ifs->prob[0]=0.0;
  ifs->prob[1]=0.85; 
  ifs->prob[2]=0.92; 
  ifs->prob[3]=0.99; 
  ifs->prob[4]=1.0; 
  */
  if (++unit->del==delay){

#ifdef PCSIM
  random_num = (float)rand()/(float)(RAND_MAX);
#else
  random_num = (float)rand()/4096.0;
#endif

  for (x=0;x<howmuch;x++){
  for(i = 0; i < row; i++){
    if ( BET(random_num,unit->prob[i],unit->prob[i+1]) ){
      unit->p2.x = unit->coeff[i][0]*unit->p1.x + unit->coeff[i][1]*unit->p1.y + unit->coeff[i][4];
      unit->p2.y = unit->coeff[i][2]*unit->p1.x + unit->coeff[i][3]*unit->p1.y + unit->coeff[i][5];
      break;
    }
  }
  unit->p1=unit->p2;  
					
  //  if (unit->p2.x>0.0)
    //    unit->returnvalx=(int)((unit->p2.x)*1024);
  //  if (unit->p2.y>0.0)
    //   unit->returnvaly=(int)((unit->p2.y)*1024);
  workingbuffer[count+x]=unit->p2.x;
  //  printf("%c",unit->p2.x);
  /*    iter=rand()%row;
    i=rand()%column;
    unit->coeff[iter][i]=((float)rand()/(float)(RAND_MAX));
    if (((float)rand()/(float)(RAND_MAX))>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
    unit->prob[iter]=((float)rand()/(float)(RAND_MAX));
    unit->p1.x=0.5;
    unit->p1.y=0.5;*/
}
  unit->del=0;
  }
  return count+x;
}

//////////////////////////////////////////////////////////

// ROSSLER

void rosslerinit(struct Rossler* unit, uint16_t *workingbuffer) {
  /*  unit->h = 0.1;
  unit->a = 0.3;
  unit->b = 0.2;
  unit->c = 5.8;*/

  unit->h = (float)workingbuffer[0]/120536.0;
  unit->a = (float)workingbuffer[1]/122536.0;
  unit->b = (float)workingbuffer[2]/100536.0;
  unit->lx0 = 0.1;
  unit->ly0 = 0;
  unit->lz0 = 0;
}

uint16_t runrossler(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct Rossler* unit){
  float lx0,ly0,lz0,lx1,ly1,lz1;
  //  float h,a,b,c;
  u8 i=0;

  /* which unit to vary according to workingbuffer */
  // leave as so!

  float h = unit->h;
  float a = unit->a;
  float b = unit->b;
  float c = unit->c;

  if (++unit->del==delay){

  for (i=0; i<howmuch; i++) {

  lx0 = unit->lx0;
  ly0 = unit->ly0;
  lz0 = unit->lz0;
  lx1 = lx0 + h * (-ly0 - lz0);
  ly1 = ly0 + h * (lx0 + (a * ly0));
  lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
  unit->lx0 = lx1;
  unit->ly0 = ly1;
  unit->lz0 = lz1;
  //  printf("%d",lz1);
  workingbuffer[count+i]=lx1;
  //  workingbuffer[i+1]=ly1;
  //  workingbuffer[i+2]=lz1;
  }
  unit->del=0;
  }

  return count+i;
}

//////////////////////////////////////////////////////////

// 2nd rossler from: MCLDChaosUGens.cpp

void secondrosslerinit(struct secondRossler* unit, uint16_t *workingbuffer){
  
  unit->a = (float)workingbuffer[0]/65536.0;
  unit->b = (float)workingbuffer[1]/65536.0;
  unit->c = (float)workingbuffer[2]/65536.0;
  unit->h = (float)workingbuffer[3]/65536.0;
  unit->x0 = (float)workingbuffer[4]/65536.0;
  unit->y0 = (float)workingbuffer[5]/65536.0;
  unit->z0 = (float)workingbuffer[6]/65536.0;

  }

uint16_t runsecondrossler(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct secondRossler* unit){

  u8 i=0;

  if (++unit->del==delay){
    
    float a=unit->a;
    float b=unit->b;
    float c=unit->b;
    float h=unit->h;
    float x0=unit->x0;
    float y0=unit->y0;
    float z0=unit->z0;

      float xn = unit->xn;
    float yn = unit->yn;
    float zn = unit->zn;
  float xnm1 = unit->xnm1;
  float ynm1 = unit->ynm1;
  float znm1 = unit->znm1;
  float dx = xn - xnm1;
  float dy = yn - ynm1;
  float dz = zn - znm1;

	for (i=0; i<howmuch; ++i) {

			xnm1 = xn;
			ynm1 = yn;
			znm1 = zn;

			float k1x, k2x, k3x, k4x,
				k1y, k2y, k3y, k4y,
				k1z, k2z, k3z, k4z,
				kxHalf, kyHalf, kzHalf;

			// 4th order Runge-Kutta approximate solution for differential equations
			k1x = - (h * (ynm1 + znm1));
			k1y = h * (xnm1 + a * ynm1);
			k1z = h * (b + znm1 * (xnm1 - c));
			kxHalf = k1x * 0.5;
			kyHalf = k1y * 0.5;
			kzHalf = k1z * 0.5;

			k2x = - (h * (ynm1 + kyHalf + znm1 + kzHalf));
			k2y = h * (xnm1 + kxHalf + a * (ynm1 + kyHalf));
			k2z = h * (b + (znm1 + kzHalf) * (xnm1 + kxHalf - c));
			kxHalf = k2x * 0.5;
			kyHalf = k2y * 0.5;
			kzHalf = k2z * 0.5;

			k3x = - (h * (ynm1 + kyHalf + znm1 + kzHalf));
			k3y = h * (xnm1 + kxHalf + a * (ynm1 + kyHalf));
			k3z = h * (b + (znm1 + kzHalf) * (xnm1 + kxHalf - c));

			k4x = - (h * (ynm1 + k3y + znm1 + k3z));
			k4y = h * (xnm1 + k3x + a * (ynm1 + k3y));
			k4z = h * (b + (znm1 + k3z) * (xnm1 + k3x - c));

			xn = xn + (k1x + 2.0*(k2x + k3x) + k4x) * ONESIXTH;
			yn = yn + (k1y + 2.0*(k2y + k3y) + k4y) * ONESIXTH;
			zn = zn + (k1z + 2.0*(k2z + k3z) + k4z) * ONESIXTH;

			dx = xn - xnm1;
			dy = yn - ynm1;
			dz = zn - znm1;
	
	/*		ZXP(xout) = (xnm1 + dx) * 0.5f;
		ZXP(yout) = (ynm1 + dy) * 0.5f;
		ZXP(zout) = (znm1 + dz) * 1.0f;*/
			workingbuffer[count+i]=xnm1+dx;
			//			printf("%c",(xnm1+dx));
	}
	unit->xn = xn;
	unit->yn = yn;
	unit->zn = zn;
	unit->xnm1 = xnm1;
	unit->ynm1 = ynm1;
	unit->znm1 = znm1;
	unit->del=0;
  }
  return count+i;
}

//////////////////////////////////////////////////////////

// BRUSSELATOR

void brusselinit(struct Brussel* unit, uint16_t *workingbuffer) {
  unit->x = 0.5f; 
  unit->y = 0.5f; 
  unit->delta = (float)workingbuffer[0]/65536.0;
  unit->mu = (float)workingbuffer[1]/65536.0;
  unit->gamma = (float)workingbuffer[2]/65536.0;
  unit->del=0;
}

uint16_t runbrussel(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct Brussel* unit){
    
  float dx, dy; 
  
  u8 i=0;

  if (++unit->del==delay){
  float muplusone = 1.0f+unit->mu; 
  float x= unit->x; 
  float y= unit->y;  
    for (i=0; i<howmuch; ++i) {
		
      float temp = x*x*y; 
        
        dx = temp - (muplusone*x) + unit->gamma;
        dy =  (unit->mu*x)  - temp; 
        
        x += unit->delta*dx; 
        y += unit->delta*dy; 
	workingbuffer[count+(u16)i]=x*65536.0;
	}
#ifdef PCSIM
    //    printf("brussels: x %f y %f\n",x,y); 
    printf("%d\n",workingbuffer[count+(u16)i]); 
#endif
    unit->del=0;
    unit->x = x; 
    unit->y = y;
  }
  return count+i;
}

//////////////////////////////////////////////////////////

// spruceworm

void spruceinit(struct Spruce* unit, uint16_t *workingbuffer) {
	
  unit->x = 0.9f; 
  unit->y = 0.1f; 
  unit->k1 = (float)workingbuffer[0]/65536.0;
  unit->k2 = (float)workingbuffer[1]/65536.0;
  unit->alpha = (float)workingbuffer[2]/65536.0;
  unit->beta = (float)workingbuffer[3]/65536.0;
  unit->mu = (float)workingbuffer[4]/65536.0;
  unit->rho = (float)workingbuffer[5]/65536.0;
  unit->delta = (float)workingbuffer[6]/65536.0;
}

uint16_t runspruce(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct Spruce* unit){

  float dx, dy; 
  u8 i=0;

  if (++unit->del==delay){
    float x= unit->x; 
    float y= unit->y;  

	for (i=0; i<howmuch; ++i) {
		
        float temp = y*y; 
        float temp2 = unit->beta*x;
        
        dx = (unit->k1* x* (1.0-x)) - (unit->mu*y);
        dy = (unit->k2*y*(1.0- (y/(unit->alpha*x))))  - (unit->rho*(temp/(temp2*temp2 +  temp))); 
        x += unit->delta*dx; 
        y += unit->delta*dy; 
	}
	
  unit->del=0;
  unit->x = x; 
  unit->y = y;
#ifdef PCSIM
  	printf("spruce: x %f y %f z %f\n",x,y); 
#endif
  }
  return count+i;
}

//////////////////////////////////////////////////////////

// OREGONATOR

void oregoninit(struct Oregon* unit, uint16_t *workingbuffer) {
    unit->x = 0.5f; 
    unit->y = 0.5f; 
    unit->z = 0.5f; 
    unit->delta = (float)workingbuffer[0]/65536.0;
    unit->epsilon = (float)workingbuffer[1]/65536.0;
    unit->mu = (float)workingbuffer[2]/65536.0;
    unit->q = (float)workingbuffer[3]/65536.0;

}

uint16_t runoregon(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct Oregon* unit){
  
  float dx, dy, dz; 
  u8 i=0;
  
  if (++unit->del==delay){

  float x= unit->x; 
  float y= unit->y; 
  float z= unit->z; 
	for (i=0; i<howmuch; ++i) {
		
        dx = unit->epsilon*((unit->q*y) -(x*y) + (x*(1-x))); 
	dy = unit->mu* (-(unit->q*y) -(x*y) + z); 
        dz = x-y; 
        
        x += unit->delta*dx; 
        y += unit->delta*dy; 
        z += unit->delta*dz; 
        
	//	output1[i]= x; 
	//        output2[i]= y; 
	//        output3[i]= z; 
	workingbuffer[count+(u16)i]=x*65536.0;
	}
#ifdef PCSIM
	printf("Oregonator: x %f y %f z %f\n",x,y,z); 
#endif
	unit->x = x; 
	unit->y = y;
	unit->z = z;
	unit->del=0;
}
return count+i;
}

//////////////////////////////////////////////////////////

// FITZHUGH - writes into buffer 3xhowmuch, how to store local floats?

void fitzinit(struct Fitz* unit, uint16_t *workingbuffer) {
	unit->u=0.0;
	unit->w=0.0;
	//	unit->b0= 1.4;
	//	unit->b1= 1.1;
	unit->b0=(float)workingbuffer[0]/32768.0;
	unit->b1=(float)workingbuffer[1]/32768.0;
}

uint16_t runfitz(uint16_t count, uint16_t delay, uint16_t *workingbuffer, uint8_t howmuch, struct Fitz* unit){

  /* SETTINGS */

  float urate= 0.7;
  float wrate= 1.7;
  float u,w;
  u8 x=0;

  if (++unit->del==delay){

  u=unit->u;
  w=unit->w;

  for (x=0;x<howmuch;x++){

    float dudt= urate*(u-(0.33333*u*u*u)-w);
    float dwdt= wrate*(unit->b0+unit->b1*u-w);
	  
    u+=dudt;
    w+=dwdt;
    //assumes fmod works correctly for negative values
    if ((u>1.0) || (u<-1.0)) u=fabs(fmod((u-1.0),4.0)-2.0)-1.0;

    int z=((float)(u)*1500);
    //    int zz=((float)(w)*1500);
    workingbuffer[count+x]=z;//workingbuffer[x+2]=zz;
#ifdef PCSIM
    //    printf("brussels: x %f y %f\n",x,y); 
    printf("%d\n",workingbuffer[count+x]); 
#endif

  }
  count+=x;

  for (x=0;x<howmuch;x++){

    float dudt= urate*(u-(0.33333*u*u*u)-w);
    float dwdt= wrate*(unit->b0+unit->b1*u-w);

    u+=dudt;
    w+=dwdt;
    //assumes fmod works correctly for negative values
        if ((u>1.0) || (u<-1.0)) u=fabs(fmod((u-1.0),4.0)-2.0)-1.0;

    int z=((float)(u)*700);
    int zz=((float)(w)*700);
    workingbuffer[count+x]=z;//deltay[x]=zz;
  }
  count+=x;
  for (x=0;x<howmuch;x++){

    float dudt= urate*(u-(0.33333*u*u*u)-w);
    float dwdt= wrate*(unit->b0+unit->b1*u-w);

    u+=dudt;
    w+=dwdt;
    //assumes fmod works correctly for negative values
        if ((u>1.0) || (u<-1.0)) u=fabs(fmod((u-1.0),4.0)-2.0)-1.0;

    int z=((float)(u)*3600);
    //		workingbuffer[x+(howmuch*2)]=z;
    workingbuffer[count+x]=z;
  }
  unit->u=u;
  unit->w=w;
  unit->del=0;
  }
  return count+x;
  }

#ifdef PCSIM

void main(void)
{
  //  int cuu=atoi(argv[1]), pll=atoi(argv[2]);
  int x;
  uint16_t xxx[MAX_SAM+12],result;
  uint16_t count=0;
  srand(time(NULL));

  for (x=0;x<MAX_SAM;x++){
    xxx[x]=rand()%65536;
  }

  // for Fitz? de-alloc?
    struct Fitz *unit=malloc(sizeof(struct Fitz));
  //    struct Oregon *unit=malloc(sizeof(struct Oregon));
  //    struct Spruce *unit=malloc(sizeof(struct Spruce));
  //    struct Brussel *unit=malloc(sizeof(struct Brussel));
  //  struct Rossler *unit=malloc(sizeof(struct Rossler));
  //  struct secondRossler *unit=malloc(sizeof(struct secondRossler));
  //  struct IFS *unit=malloc(sizeof(struct IFS));
  //  struct simpleSIR *unit=malloc(sizeof(struct simpleSIR));
  //    struct SEIR *unit=malloc(sizeof(struct SEIR));
  //      struct SICR *unit=malloc(sizeof(struct SICR));
  //  struct siney *unit=malloc(sizeof(struct siney));
  //  spruceinit(unit,xxx); 
    fitzinit(unit,xxx); 
    //  brusselinit(unit,xxx); 

        while(1){ 
	  //	  count=runoregon(count,1,xxx,1,unit);
	  count=runfitz(count,1,xxx,1,unit);

	  //	  count=runseir(count,10,xxx,10,unit);
	  //	  count=runsicr(count,10,xxx,10,unit);
	  //	  count=runsine(count,1,xxx,10,unit);
	  //	  	  printf("count %d\n",count);
	  /*	  for (x=3;x<13;x++){
	    printf("%c",xxx[x]>>8);
	    }*/
	    
    }
}
#endif