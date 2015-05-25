/*
 * flow_controle.cpp
 *
 *  Created on: 2014/05/10
 *      Author: Shuhei
 */

#include"flow_controle.hpp"

void FlowControle::Assignment(){
	LEFT_ARM = PB9;
	RIGHT_ARM = PB8;
	LEFT_FOOT = PB6;
	RIGHT_FOOT = PB7;
	LEFT_ARM_S = PC0;
	RIGHT_ARM_S = PC1;
	LEFT_FOOT_S = PC2;
	RIGHT_FOOT_S = PC3;
	SHUDOUKI = USART1;
	SLAVE = USART3;
	SABAASH[0] = PC6;
	SABAASH[1] = PC7;
	RELAY = PC13;
	slaveSendTimingStopwatch = SLAVE_SEND_TIMING_STOPWATCH;
}


#ifdef _PC_CONNECT
FlowControle::FlowControle(BaseFlow *pc){
	pcControle = pc;
	correntFlow = pcControle;
}
#else
FlowControle::FlowControle(BaseFlow *seesaw,BaseFlow *swing,BaseFlow *pole,BaseFlow *jungle){
	this->seesawFlow = seesaw;
	this->swingFlow = swing;
	this->poleFlow = pole;
	this->jungleFlow = jungle;
	this->correntFlow = this->seesawFlow;


}

#endif

void FlowControle::Initialize(){
	Assignment();
	RCC_Configuration();
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE);
	PinMode(RIGHT_ARM,SERVO);
	PinMode(LEFT_ARM,SERVO);
	PinMode(RIGHT_FOOT,SERVO);
	PinMode(LEFT_FOOT,SERVO);
	PinMode(LEFT_ARM_S,IO_INPUT_PD);
	PinMode(RIGHT_ARM_S,IO_INPUT_PD);
	PinMode(LEFT_FOOT_S,IO_INPUT_PD);
	PinMode(RIGHT_FOOT_S,IO_INPUT_PD);
	PinMode(SABAASH[0],SERVO);
	PinMode(SABAASH[1],SERVO);
	PinMode(RELAY,IO_OUTPUT);
	USART_Mode(SHUDOUKI,SEND_AND_READ,DISABLE);
	USART_Mode(SLAVE,SEND,DISABLE);
	USART_Begin(SHUDOUKI,9600);
	USART_Begin(SLAVE,9600);
	StartStopWatch(slaveSendTimingStopwatch);
#ifdef _EMERGENCY
	emergency.Initialize();


#endif


}
void FlowControle::Start(Params *params){
	this->params = params;
	Initialize();

#ifdef _ZERO_POINT
	params->relay = HIGH;
	while(1){
		Write();
	}
#endif

	while(1){
		correntFlow->Initialize();
		Loop();
	}
}

void FlowControle::Loop(){
	while(1){
#ifdef _EMERGENCY
		emergency.Update();
#endif
#ifndef _PC_CONNECT
		ScanSensor();
		InputFromShudouki();
		if(params->correntStep == 0){
			params->relay = LOW;
		}else{
			params->relay = HIGH;
		}
		if(params->nextMode != params->correntMode || params->correntStep != params->lastStep){
			params->connection->StopCall();

		}
#endif
		correntFlow->Calc();
		Write();
#ifndef _PC_CONNECT

		if(params->nextMode != params->correntMode){
			params->correntMode = params->nextMode;
			switch(params->correntMode){
			case SEESAW:
				correntFlow = seesawFlow;
				return;
			case SWING:
				correntFlow = swingFlow;
				return;
			case JUNGLE:
				correntFlow = jungleFlow;
				return;
			case POLE:
				correntFlow = poleFlow;
				return;
			}
			params->correntStep = 0;
		}
#endif
	}
}
void FlowControle::ScanSensor(){
	params->leftArmSensor = (IO_VALUE)IO_Read(LEFT_ARM_S);
	params->rightArmSensor = (IO_VALUE)IO_Read(RIGHT_ARM_S);
	params->leftArmSensor = (IO_VALUE)IO_Read(LEFT_FOOT_S);
	params->rightFootSensor = (IO_VALUE)IO_Read(RIGHT_ARM_S);
}

void FlowControle::Write(){
	int i=0;
	Params::data slaveData[ARM_NUM*2 + FOOT_NUM*2 - 4];
	char slaveSendData[(ARM_NUM*2 + FOOT_NUM*2 -4)*2 + 1];
	int slaveDataLength = ARM_NUM*2 + FOOT_NUM*2 - 4;
	int slaveSendDataLength = (ARM_NUM*2 + FOOT_NUM*2 -4)*2 + 1;

	for(int j=0;j<ARM_NUM-1;j++,i++){
		slaveData[i] = params->leftArms[j];
	}
	for(int j=0;j<ARM_NUM-1;j++,i++){
		slaveData[i] = params->rightArms[j];
	}
	for(int j=0;j<FOOT_NUM-1;j++,i++){
		slaveData[i] = params->leftFoots[j];
	}
	for(int j=0;j<FOOT_NUM-1;j++,i++){
		slaveData[i] = params->rightFoots[j];
	}
	IO_Write(RELAY,params->relay);
	if(GetStopWatchMillis(slaveSendTimingStopwatch) > 80){
		slaveSendData[0] = 0xff;
		for(int j=0,i=1;j<slaveDataLength;j++){
			slaveData[j]+=90;
			slaveSendData[i++] = (slaveData[j] >> 7) & 0x7f;
			slaveSendData[i++] = slaveData[j] & 0x7f;
		}
		USART_Write(SLAVE,slaveSendData,slaveSendDataLength);
		ResetStopWatch(slaveSendTimingStopwatch);
	}
	SERVO_Write(LEFT_ARM,params->leftArms[ARM_NUM-1]);
	SERVO_Write(RIGHT_ARM,params->rightArms[ARM_NUM-1]);
	SERVO_Write(LEFT_FOOT,params->leftFoots[FOOT_NUM-1]);
	SERVO_Write(RIGHT_FOOT,params->rightFoots[FOOT_NUM-1]);
	SERVO_Write(SABAASH[0],params->sabaash[0]);
	SERVO_Write(SABAASH[1],params->sabaash[1]);
}


void FlowControle::InputFromShudouki(){
	int rxGet;
	char rxData[30];
	char *opcode,*operand;
	int opeSize = 0;
	rxGet = USART_ReadSplit(SHUDOUKI,';',rxData,30);
	if(USART_GetRxBufferSize(SHUDOUKI) > 20){
		USART_ClearRxBuffer(SHUDOUKI);
	}
	if(rxGet > 0){
		opcode = strtok(rxData," ");
		operand = strtok(NULL," ");
		if(opcode != NULL){
			opeSize++;
		}
		if(operand != NULL){
			opeSize++;
		}
		switch(opeSize){
		case 1:
			if(strcmp(opcode,"yes") == 0){
				params->connection->replyed = true;
			}
			break;
		case 2:
			if(strcmp(opcode,"mode") == 0){
				MODE_SELECT select;
				if(TryGetMode(operand,&select)){
					params->nextMode = select;
					params->correntStep = 0;
					WriteMode(params->nextMode);
				}
			}else if(strcmp(opcode,"step") == 0){
				int select = atoi(operand);
				params->correntStep = select;
				WriteStep(params->correntStep);
			}
			break;
		}
	}
	params->connection->UpdateCallState(SHUDOUKI);
}

void FlowControle::WriteStep(int step){
	char tempStr[20];
	sprintf(tempStr,"cmpstep %d;",step);
	USART_Print(SHUDOUKI,tempStr);
}

void FlowControle::WriteMode(MODE_SELECT mode){
	char tempStr0[25],tempStr1[10];
	switch(mode){
	case SEESAW:
		strcpy(tempStr1,"seesaw");
		break;
	case SWING:
		strcpy(tempStr1,"swing");
		break;
	case POLE:
		strcpy(tempStr1,"pole");
		break;
	case JUNGLE:
		strcpy(tempStr1,"jungle");
		break;
	}
	sprintf(tempStr0,"cmpmode %s;",tempStr1);
	USART_Print(SHUDOUKI,tempStr0);
}


bool FlowControle::TryGetMode(char *str,MODE_SELECT *mode){
	if(strcmp(str,"seesaw") == 0){
		*mode = SEESAW;
	}else if(strcmp(str,"swing") == 0){
		*mode = SWING;
	}else if(strcmp(str,"pole") == 0){
		*mode = POLE;
	}else if(strcmp(str,"jungle") == 0){
		*mode = JUNGLE;
	}else{
		return false;
	}
	return true;
}


#ifdef _EMERGENCY
Emergency::Emergency(){

}

void Emergency::Initialize(){
	lastTime = Millis();
	LEDTime = 0;
	on = LOW;
	on_when_voltage_low = PC10;
	battery = PA2;
	PinMode(on_when_voltage_low,IO_OUTPUT);
	PinMode(battery,ADC);
	IO_Write(on_when_voltage_low,LOW);
	count = 0;
	totalBatteryVoltage = 0;
}

void Emergency::Update(){
	totalBatteryVoltage+= ADC_Read(battery);
	count++;
	if(Millis() - lastTime > 1000){
		if(count == 0)count=1;
		int batteryVoltage = totalBatteryVoltage/count;
		batteryVoltage*=1000;
		batteryVoltage/=430;
		if(batteryVoltage < LOW_BATTERY_BORDER){
			on = HIGH;
			LEDTime = Millis();
		}else{
			on = LOW;
		}
		lastTime = Millis();
		totalBatteryVoltage = 0;
		count = 0;
	}
	IO_Write(on_when_voltage_low,on);
}
#endif
