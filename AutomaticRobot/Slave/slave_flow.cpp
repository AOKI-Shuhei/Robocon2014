/*
 * slave_flow.cpp
 *
 *  Created on: 2014/02/05
 *      Author: Shuhei
 */


#include "slave_flow.hpp"

SlaveFlow::SlaveFlow(){
	r_arm[0] = PA3;
	r_arm[1] = PA1;
	r_arm[2] = PA0;

	l_arm[0] = PB7;
	l_arm[1] = PB8;
	l_arm[2] = PB9;

	r_foot[0] = PA6;
	r_foot[1] = PA7;
	r_foot[2] = PB0;
	r_foot[3] = PB1;

	l_foot[0] = PB6;
	l_foot[1] = PA11;
	l_foot[2] = PA10;
	l_foot[3] = PA8;

	master = USART3;

	connected = false;

	for(int i=0;i<ARM_NUM;i++){
		l_arm_d[i] = 0;
		r_arm_d[i] = 0;
	}
	for(int i=0;i<FOOT_NUM;i++){
		l_foot_d[i] = 0;
		r_foot_d[i] = 0;
	}

	PinAssign();


}

void SlaveFlow::PinAssign(){
	RCC_Configuration();
	for(int i=0;i<ARM_NUM;i++){
		PinMode(l_arm[i],SERVO);
		PinMode(r_arm[i],SERVO);
	}
	for(int i=0;i<FOOT_NUM;i++){
		PinMode(l_foot[i],SERVO);
		PinMode(r_foot[i],SERVO);
	}
	USART_Mode(master,READ,DISABLE);
	USART_Begin(master,9600);
}
/**
 * in  : USART
 */
void SlaveFlow::Input(){
	static int time_out[] = {0,0};
	
	if(USART_Readnbyte(master,master_read,MASTER_READ_LEN) == 0){
		time_out[0] = 0;
		connected = true;
	}else{
		if(time_out[0] == 0){
			time_out[1] = Millis();
		}
		time_out[0] = 1;
		if((Millis() - time_out[1]) > TIME_OUT){
			connected = false;
		}
	}
	Decode();
}

void SlaveFlow::Decode(){
	if(connected){
		int i=0;
		//left_arm 0 1 2 ...
		for(int j=0;j<ARM_NUM && i<MASTER_READ_LEN;j++){
			l_arm_d[j] = master_read[i++];
			l_arm_d[j] = l_arm_d[j] << 7;
			l_arm_d[j] |= master_read[i++];
			l_arm_d[j] = ShiftFrom0_180ToM90_90(l_arm_d[j]);
		}
		//right_arm 0 1 2 ...
		for(int j=0;j<ARM_NUM && i<MASTER_READ_LEN;j++){
			r_arm_d[j] = master_read[i++];
			r_arm_d[j] = r_arm_d[j] << 7;
			r_arm_d[j] |= master_read[i++];
			r_arm_d[j] = ShiftFrom0_180ToM90_90(r_arm_d[j]);
		}
		//left_foot 0 1 2 ...
		for(int j=0;j<FOOT_NUM && i<MASTER_READ_LEN;j++){
			l_foot_d[j] = master_read[i++];
			l_foot_d[j] = l_foot_d[j] << 7;
			l_foot_d[j] |= master_read[i++];
			l_foot_d[j] = ShiftFrom0_180ToM90_90(l_foot_d[j]);
		}
		//right_foot 0 1 2 ...
		for(int j=0;j<FOOT_NUM && i<MASTER_READ_LEN;j++){
			r_foot_d[j] = master_read[i++];
			r_foot_d[j] = r_foot_d[j] << 7;
			r_foot_d[j] |= master_read[i++];
			r_foot_d[j] = ShiftFrom0_180ToM90_90(r_foot_d[j]);
		}
	}
}

void SlaveFlow::Output(){
	static int count = Millis();
	int lastCount = count;
	count = Millis();
	
	
	Calc();
	Write();
}

void SlaveFlow::Calc(){

}

void SlaveFlow::Write(){
	
	for(int i=0;i<ARM_NUM;i++){
		SERVO_Write(l_arm[i],l_arm_d[i]);
		SERVO_Write(r_arm[i],r_arm_d[i]);
	}
	
	for(int i=0;i<FOOT_NUM;i++){
		SERVO_Write(l_foot[i],l_foot_d[i]);
		SERVO_Write(r_foot[i],r_foot_d[i]);
	}
}

