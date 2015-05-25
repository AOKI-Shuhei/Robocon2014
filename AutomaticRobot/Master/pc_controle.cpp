/*
 * pc_controle.cpp
 *
 *  Created on: 2014/02/05
 *      Author: Shuhei
 */


#include"pc_controle.hpp"


PCControle::PCControle(Params *params) : BaseFlow(params){
	connected = false;
	stopWatchNumber0 = STOPWATCH_NUMBER_0;
	stopWatchNumber1 = STOPWATCH_NUMBER_1;
	PC_PORT = USART1;
}

void PCControle::Initialize(){
	PinMode(PC8,IO_OUTPUT);
	PinMode(PC9,IO_OUTPUT);
	RestartStopWatch(stopWatchNumber0);
}

void PCControle::InputFromPC(){
	if(USART_Readnbyte(PC_PORT,pc_read,PC_READ_LEN) == 0){
		RestartStopWatch(stopWatchNumber0);
		StopStopWatch(stopWatchNumber1);
		connected = true;
	}else{
		if(GetStopWatchMillis(stopWatchNumber0) > TIME_OUT){
			connected = false;
			if(!IsRunningStopWatch(stopWatchNumber1)){
				RestartStopWatch(stopWatchNumber1);
			}else{
				if(GetStopWatchMillis(stopWatchNumber1) > 400){
					USART_Close(PC_PORT);
					Delay(100);
					USART_Begin(PC_PORT,9600);
					ResetStopWatch(stopWatchNumber1);
				}
			}
		}
	}
}


/**
 * in  : アームの目的の値
 * out : 従マイコンへの送信データ
 */
void PCControle::Calc(){
	InputFromPC();
	if(connected){
		int i=0;
		//left_arm 0 1 2 ...
		for(int j=0;j<ARM_NUM && i<PC_READ_LEN;j++){
			params->leftArms[j] = pc_read[i++];
			params->leftArms[j] = params->leftArms[j] << 7;
			params->leftArms[j] |= pc_read[i++];
			params->leftArms[j]-=90;
		}
		//right_arm 0 1 2 ...
		for(int j=0;j<ARM_NUM && i<PC_READ_LEN;j++){
			params->rightArms[j] = pc_read[i++];
			params->rightArms[j] = params->rightArms[j] << 7;
			params->rightArms[j] |= pc_read[i++];
			params->rightArms[j]-=90;
		}
		//left_foot 0 1 2 ...
		for(int j=0;j<FOOT_NUM && i<PC_READ_LEN;j++){
			params->leftFoots[j] = pc_read[i++];
			params->leftFoots[j] = params->leftFoots[j] << 7;
			params->leftFoots[j] |= pc_read[i++];
			params->leftFoots[j]-=90;
		}
		//right_foot 0 1 2 ...
		for(int j=0;j<FOOT_NUM && i<PC_READ_LEN;j++){
			params->rightFoots[j] = pc_read[i++];
			params->rightFoots[j] = params->rightFoots[j] << 7;
			params->rightFoots[j] |= pc_read[i++];
			params->rightFoots[j] -=90;
		}
		
		if(pc_read[PC_READ_LEN-1] == 0x7f){
			params->relay = HIGH;
		}else if(pc_read[PC_READ_LEN-1] == 0x00){
			params->relay = LOW;
		}
	}
	if(connected){
		IO_Write(PC9,HIGH);
		IO_Write(PC8,LOW);
	}else{
		IO_Write(PC8,HIGH);
		IO_Write(PC9,LOW);
	}
}


