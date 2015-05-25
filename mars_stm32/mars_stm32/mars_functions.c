#include"mars_functions.h"

int ConstrainInt(int value,int low,int high){
  return value > low ? (value < high ? value : high) : low;
}

int ShiftFromM90_90To0_180(int value){
	return value+90;
}
int ShiftFrom0_180ToM90_90(int value){
	return value - 90;

}

