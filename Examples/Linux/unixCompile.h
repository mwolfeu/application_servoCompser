#ifdef SC_DBG_UNIX

#ifndef SC_COMPILE_ISSUES
#define SC_COMPILE_ISSUES

#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

#include <stdio.h>
#include <time.h>

#include<cstdlib>
#include<iostream>
using namespace std;

void nothing(void);
void nothing1(int x);

#define boolean bool
#define uint8_t unsigned char
#define cli nothing
#define sei nothing
#define SREG 0
#define __iRestore nothing1

#endif

extern void setup(void);
extern void loop(void);

#endif

