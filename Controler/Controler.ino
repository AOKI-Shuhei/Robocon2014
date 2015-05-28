
/*
コントローラのソースは消えちゃったっぽい。ので古いソースをもとに復旧してみる。動くかどうか未知数。
*/
// http://pspunch.com/pd/article/arduino_lib_gpsx.html

#include <GPSX.h>
#include <GPSXClass.h>

//#define _DEBUG

#indef _DEBUG
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#endif

typedef enum SERIAL_DATA{
	D_RUNNING = 0x40,
	D_SELECT = 0x20,
	D_START = 0x10,
	D_LEFT = 0x08,
	D_UP = 0x04,
	D_RIGHT = 0x02,
	D_DOWN = 0x01,

	D_RECT = 0x40,
	D_TRIANGLE = 0x20,
	D_CIRCLE = 0x10,
	D_CROSS = 0x08,
	D_L1 = 0x04,
	D_R1 = 0x02,

	D_L2 = 0x40,
	D_R2 = 0x20,

	D_NUMBER = 0x3f,
	D_MINUS = 0x40,
}SERIAL_DATA;

const int REVISE_STICK = 10;
const int L_REVERSE = 1,R_REVERSE = -1, TURN_REVERSE = -1;
const int PRESSED_TIME = 400;

byte serialData[6] = {0xff,0,0,0,0,0};
const SERIAL_DATA_LENGTH = 6;

long milliTimes[8];

void getButtons(){
	if(PRESSED_STICK_LEFT(PSX_PAD1) && IS_DOWN_STICK_RIGHT(PSX_PAD1) || PRESSED_STICK_RIGHT(PSX_PAD1) && IS_DOWN_STICK_LEFT(PSX_PAD)){
		serialData[1] ^= D_RUNNING;
	}
	serialData[1] &= D_RUNNING;
	if(PRESSED_SELECT(PSX_PAD1)){
		serialData[1] |= D_SELECT;
	}
	if(PRESSED_START(PSX_PAD1)){
		serialData[1] |= D_START;
	}
	serialData[2] = 0;
	if(PRESSED_SQUARE(PSX_PAD1)){
		serialData[2] |= D_RECT;
	}
	if(PRESSED_CROSS(PSX_PAD1)){
		serialData[2] |= D_CROSS;
	}
	if(PRESSED_CIRCLE(PSX_PAD1)){
		serialData[2] |= D_CIRCLE;
	}
	if(PRESSED_TRIANGE(PSX_PAD1)){
		serialData[2] |= D_TRIANGLE;
	}
	if(PRESSED_L1(PSX_PAD1)){
		serialData[2] |= D_L1;
	}
	if(PRESSED_R1(PSX_PAD1)){
		serialData[2] |= D_R1;
	}
	if(IS_DOWN_R2(PSX_PAD1)){
		serialData[3] |= D_R2;
	}
	if(IS_DOWN_L2(PSX_PAD1)){
		serialData[3] |= D_L2;
	}
	if(PRESSED_UP(PSX_PAD1)){
		serialData[1] |= D_UP;
	}
	if(PRESSED_DOWN(PSX_PAD1)){
		serialData[1] |= D_DOWN;
	}
	if(PRESSED_RIGHT(PSX_PAD1)){
		serialData[1] |= D_RIGHT;
	}
	if(PRESSED_LEFT(PSX_PAD1)){
		serialData[1] |= D_LEFT;
	}
}

void getAnalog(){
	int left = ANALOG_LEFT_Y(PSX_PAD1);
	int right = ANALOG_RIGHT(PSX_PAD1);
	serialData[4] = 0;
	serialData[5] = 0;
	if(left > REVISE_STICK + 127){
		serialData[4] = 0x3f&(byte)((left-127)/2);
	}else if(left < 127 - REVISE_STICK){
		serialData[4] = 0x3f&(byte)((127-left)/2);
		serialData[4] |= 0x40;
	}
	if(right > REVISE_STICK + 127){
		serialData[4] = 0x3f&(byte)((right-127)/2);
		serialData[4] |= 0x40;
	}else if(right < 127 - REVISE_STICK){
		serialData[4] = 0x3f&(byte)((127-right)/2);
	}
}
