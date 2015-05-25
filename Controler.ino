
/*
コントローラのソースは消えちゃったっぽい。ので古いソースをもとに復旧してみる。動くかどうか未知数。
*/
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
	if(PRESSED_SELECT(PSX_PAD1)){
		serialData[]
	}
}
