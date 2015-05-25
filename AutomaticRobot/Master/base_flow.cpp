/*
 * BaseFlow.cpp
 *
 *  Created on: 2014/02/03
 *      Author: FMV
 */

#include "base_flow.hpp"




Connection::Connection(int timeOut){
	doCall = false;
	calling = false;
	stopWatchNumber =CONNECTION_STOPWATCH;
	connected = false;
	this->timeOut = timeOut;
}
void Connection::UpdateCallState(USART_TypeDef *shudouki){
	if(doCall){
		if(!calling){
			USART_Print(shudouki,"hello;");
			ResetStopWatch(stopWatchNumber);
			StartStopWatch(stopWatchNumber);
			calling = true;
		}else{
			if(replyed){
				calling = false;
				connected = true;
				replyed = false;
			}else if(GetStopWatchMillis(stopWatchNumber) > timeOut){
				connected = false;
				calling = false;
			}
		}
	}else{
		calling = 0;
	}
}
void Connection::StartCall(bool defaltState){
	if(!doCall){
		connected = defaltState;
		calling = 0;
	}
	doCall = true;
}
void Connection::StopCall(){
	doCall = false;
	replyed = false;
}
bool Connection::GetConnection(){
	return connected;
}

Params::Params(Connection *connection){
	for(int i=0;i<ARM_NUM;i++){
		leftArms[i] = 0;
	}
	for(int i=0;i<ARM_NUM;i++){
		rightArms[i] = 0;
	}
	for(int i=0;i<FOOT_NUM;i++){
		leftFoots[i] = 0;
	}
	for(int i=0;i<FOOT_NUM;i++){
		rightFoots[i] = 0;
	}
	sabaash[0] = 0;
	sabaash[1] = 0;
	leftArmSensor = LOW;
	rightArmSensor = LOW;
	leftFootSensor = LOW;
	rightFootSensor = LOW;
	relay = LOW;
	correntMode = SEESAW;
	nextMode = SEESAW;
	correntStep = 0;
	lastStep = 0;
	this->connection = connection;
}
