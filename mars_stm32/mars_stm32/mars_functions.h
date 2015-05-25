
#ifndef MARS_FUNCTIONS_H_
#define MARS_FUNCTIONS_H_

#include<stdio.h>

#ifdef __cplusplus
 extern "C" {
#endif
int ConstrainInt(int value,int low,int high);
int ShiftFromM90_90To0_180(int value);
int ShiftFrom0_180ToM90_90(int value);
#ifdef __cplusplus
 }
#endif
#endif /* MARS_FUNCTIONS_H_ */
