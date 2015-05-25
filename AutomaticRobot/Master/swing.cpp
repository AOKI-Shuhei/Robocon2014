/*
 * swing.cpp
 *
 *  Created on: 2014/02/06
 *      Author: Shuhei
 */



#include "swing.hpp"

SwingFlow::SwingFlow(Params *params) : BaseFlow(params){
	stopWatchNumber = STOPWATCH_NUMBER_0;
};

void SwingFlow::Calc(){
	if(params->lastStep != params->correntStep){
		ResetStopWatch(stopWatchNumber);
		params->lastStep = params->correntStep;
	}
	params->correntStep = ConstrainInt(params->correntStep,0,STEP_NUM -1);
	int motionStep;
	switch(params->correntStep){
	case 2:
		motionStep = 1;
		break;
	default:
		motionStep = ConstrainInt(params->correntStep-2,0,MOTION_NUM-1);
		break;
	}
	int motionTime = SetArms(motionStep);

	switch(params->correntStep){
	case 0:
		break;
	case 4:
		params->connection->StartCall(true);
		if(!params->connection->GetConnection()){
			params->correntStep++;
			params->connection->StopCall();
		}
		break;
	case 17:
		params->relay = LOW;
		break;
	default:
		if(motionTime > 0){
			if(!IsRunningStopWatch(stopWatchNumber)){
				StartStopWatch(stopWatchNumber);
			}
			if(GetStopWatchMillis(stopWatchNumber) > motionTime){
				params->correntStep++;
			}
		}
		break;
	}

};

void SwingFlow::Initialize(){
	params->lastStep = 0;
	StopStopWatch(stopWatchNumber);
	ResetStopWatch(stopWatchNumber);
}

int SwingFlow::SetArms(int step){
#ifndef _IS_BLUE
	Params::data motion[MOTION_NUM][TOTAL_ARMS_NUM] = {
		{26,-50,-17,-49,0,49,1,-42,90,-90,-3,-90,47,-21,34,0,50,-90,0},
		{26,-50,-17,61,0,49,1,-42,90,-90,-3,-90,47,-21,34,0,50,-90,0},
		{26,-50,-17,61,0,49,1,-42,90,-61,-3,13,47,-21,34,0,16,-3,0},
		{61,-33,-17,52,0,49,1,-42,90,19,-3,13,47,-61,0,0,90,89,500},
		{61,-33,-17,52,0,49,1,-42,90,19,-3,13,47,-61,-51,0,90,90,500},
		{84,-47,-17,60,-84,67,1,-67,90,17,-3,13,47,0,-52,0,90,80,500},
		{90,-47,-17,60,-90,67,1,-90,90,17,-3,13,47,-14,-29,6,-25,80,500},
		{84,-47,-17,80,-84,67,1,-90,0,17,-3,13,47,-14,-29,6,-25,80,500},
		{84,-47,-17,80,0,45,1,-90,0,25,-3,13,47,-9,-33,6,-25,80,500},
		{84,-46,-17,60,0,20,1,22,31,-23,30,13,47,-9,-33,6,-25,80,20000},
		{90,-68,-17,5,0,45,1,-90,0,66,-3,13,47,-9,-35,6,-33,90,500},
		{20,-69,-17,5,0,45,1,-90,0,66,-3,13,47,-9,-33,6,-25,90,500},
		{20,-69,-17,5,0,45,1,-90,0,66,-3,13,47,-9,43,6,-25,90,500},
		{20,-69,-17,5,0,45,1,-90,0,-35,-3,13,47,-9,43,6,-25,80,0},
		{20,-69,-17,-60,0,45,1,-90,0,-35,-3,13,47,-9,43,6,-7,18,500},
		{20,-69,-17,-60,0,45,1,-90,0,-35,-3,13,47,-9,43,6,-7,18,0},

	};
#else
	Params::data motion[MOTION_NUM][TOTAL_ARMS_NUM] = {
		{0,-49,-1,42,-26,50,17,49,21,-34,0,-50,90,-90,90,3,90,-47,0},
		{0,-49,-1,42,-26,50,17,-61,21,-34,0,-50,90,-90,90,3,90,-47,0},
		{0,-49,-1,42,-26,50,17,-61,21,-34,0,-16,3,-90,61,3,-13,-47,0},
		{0,-49,-1,42,-61,33,17,-52,61,0,0,-90,-89,-90,-19,3,-13,-47,500},
		{0,-49,-1,42,-61,33,17,-52,61,51,0,-90,-90,-90,-19,3,-13,-47,500},
		{84,-67,-1,67,-84,47,17,-60,0,52,0,-90,-80,-90,-17,3,-13,-47,500},
		{90,-67,-1,90,-90,47,17,-60,14,29,-6,25,-80,-90,-17,3,-13,-47,500},
		{84,-67,-1,90,-84,47,17,-80,14,29,-6,25,-80,0,-17,3,-13,-47,500},
		{0,-45,-1,90,-84,47,17,-80,9,33,-6,25,-80,0,-25,3,-13,-47,500},
		{0,-20,-1,-22,-84,46,17,-60,9,33,-6,25,-80,-31,23,-30,-13,-47,20000},
		{0,-45,-1,90,-90,68,17,-5,9,35,-6,33,-90,0,-66,3,-13,-47,500},
		{0,-45,-1,90,-20,69,17,-5,9,33,-6,25,-90,0,-66,3,-13,-47,500},
		{0,-45,-1,90,-20,69,17,-5,9,-43,-6,25,-90,0,-66,3,-13,-47,500},
		{0,-45,-1,90,-20,69,17,-5,9,-43,-6,25,-80,0,35,3,-13,-47,0},
		{0,-45,-1,90,-20,69,17,60,9,-43,-6,7,-18,0,35,3,-13,-47,500},
		{0,-45,-1,90,-20,69,17,60,9,-43,-6,7,-18,0,35,3,-13,-47,0},
	};	
#endif
	
	if(step != ConstrainInt(step,0,MOTION_NUM-1)){
		params->relay = LOW;
		return -1;
	}
	int i=0;
	for(int j=0;j<ARM_NUM;j++,i++){
		params->leftArms[j] = motion[step][i];
	}
	for(int j=0;j<ARM_NUM;j++,i++){
		params->rightArms[j] = motion[step][i];
	}
	for(int j=0;j<FOOT_NUM;j++,i++){
		params->leftFoots[j] = motion[step][i];
	}
	for(int j=0;j<FOOT_NUM;j++,i++){
		params->rightFoots[j] = motion[step][i];
	}
	return motion[step][TOTAL_ARMS_NUM-1];

}


