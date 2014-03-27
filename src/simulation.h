#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#endif

#define true 1
#define false 0
#define MAX_SAM 32768
#define floor(x) ((int)(x))
#define ONESIXTH 0.1666666666666667
#define BET(A, B, C)  (((A>=B)&&(A<=C))?1:0)    /* a between [b,c] */
#define MAX_GROUPS 16
#define STACK_SIZE 16
#define NUM_FUNCS 28

#define CONVY 0
#define SINEY 1
#define INCY 2
#define DECY 3
#define LEFTY 4
#define RIGHTY 5
#define SWAPPY 6
#define NEXTINCY 7
#define NEXTDECY 8
#define NEXTMULTY 9
#define NEXTDIVY 10
#define COPYY 11
#define ZEROY 12
#define FULLY 13
#define RANDY 14
#define KNOBY 15
#define SWAPAUDIOY 16
#define ORAUDIOY 17
#define SIMPLESIRY 18
#define SEIRY 19
#define SICRY 20
#define IFSY 21
#define ROSSLERY 22
#define SECONDROSSLERY 23
#define BRUSSELY 24
#define SPRUCEY 25
#define OREGONY 26
#define FITZY 27

typedef unsigned char u8;
typedef uint16_t u16;

struct siney{
  u16 sin_data[256];  // sine LUT Array
  u16 cc;
};

struct CONV{
float c0, c1, c2;
};

struct generik{
  u16 cop;
};

struct simpleSIR{
  float beta;//=520.0/365.0;
  float gamm;//=1.0/7.0;
  float S0;//=1.0-1e-6;
  float I0;//=1e-6;
  float step;
  float S,I,R;
  float dPop[3];
};

struct SEIR {
  float beta;
  float step;
  float gamm;
  int n;
  int m;
  float mu;
  float S0,I0;
  float S,I[MAX_GROUPS]; // 4x8x16=512bytes
  float dPop[MAX_GROUPS+1];//4x9=36bytes
};

struct SICR {
  float beta;
  float epsilon;
  float gamm;
  float Gamm; 
  float mu;
  float q;
  float S0;
  float I0;
  float C0;
  float t,S,I,C,R;
  float dPop[3];
  float step;
};

typedef struct{ float x, y; } Point;

struct IFS {
  float prob[5];
  float coeff[4][6];
  Point p1,p2;
	};

struct Rossler{
  float h,a,b,c,lx0,ly0,lz0;
};

struct secondRossler{
  float z0, zn, znm1;
  float a,b,c,h;
  float x0, y0, xn, yn, xnm1, ynm1;
};

struct Brussel{
  float x,y; 
  float delta,mu,gamma;
};

struct Spruce{
  float x, y; 
  float k1,k2,alpha,beta,mu,rho,delta;
};

struct Oregon
{
  float x, y, z; 
  float delta,epsilon,mu,q;
};

struct Fitz
{
  float u,w,b0,b1;
};


void inittable(u8 r, u8 k, int rule, u8 *table);

struct stackey{
  u16 (*functione) (uint16_t count, u16 *workingbuffer, uint8_t howmuch, void * unity);
  u8 howmuch;
  void* unit;
  };


signed char func_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u16* buffer, u8 stack_pos, u8 howmuch);
void func_runall(struct stackey stack[STACK_SIZE], u16* buffer, u8 stack_pos);
signed char func_pop(struct stackey stack[STACK_SIZE], u8 stack_pos);

void ca_runall(struct stackey stack[STACK_SIZE], u8* buffer, u8 stack_posy);
signed char ca_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u8* buffer, u8 stack_posy, u8 howmuch);
signed char ca_pop(struct stackey stack[STACK_SIZE], u8 stack_posy);
