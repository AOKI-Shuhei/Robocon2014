/*
 * seesaw.cpp
 *
 *  Created on: 2014/02/03
 *      Author: FMV
 */


#include "seesaw.hpp"

SeesawFlow::SeesawFlow(Params *params) : BaseFlow(params){
	stopWatchNumber = STOPWATCH_NUMBER_0;
};

void SeesawFlow::Calc(){
	if(params->lastStep != params->correntStep){
		ResetStopWatch(stopWatchNumber);
		params->lastStep = params->correntStep;
	}
	params->correntStep = ConstrainInt(params->correntStep,0,STEP_NUM-1);
	int motionStep;
	motionStep = ConstrainInt(params->correntStep-1,0,MOTION_NUM-1);


	int motionTime = SetArms(motionStep);
	//motion now testing
	switch(params->correntStep){
	case 0:
	case 1:
		break;
	case 2:
		params->connection->StartCall(true);
		if(!params->connection->GetConnection()){
			params->correntStep++;
			params->connection->StopCall();
		}
		break;
	case 3:
		params->connection->StartCall(false);
		if(params->connection->GetConnection()){
			params->correntStep++;
			params->connection->StopCall();
		}
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

void SeesawFlow::Initialize(){
	params->lastStep = 0;
	StopStopWatch(stopWatchNumber);
	ResetStopWatch(stopWatchNumber);
}

int SeesawFlow::SetArms(int step){
	Params::data motion[STEP_NUM][TOTAL_ARMS_NUM] = {
			{-33,-7,-80,-60,-76,23,0,0,0,0,0,0,-38,0,0,0,0,42,0},
			{-33,-7,-80,50,-76,23,0,0,0,0,0,0,-38,0,0,0,0,42,0},
			{-23,-49,-80,50,-37,90,0,0,0,0,0,0,-43,0,0,0,0,39,0},
			{-27,-37,-80,-60,-49,51,0,0,0,0,0,0,-52,0,0,0,0,25,0},
			{2,-26,-80,-60,-49,51,0,0,0,0,0,0,0,0,0,0,0,0,0},

	};
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

