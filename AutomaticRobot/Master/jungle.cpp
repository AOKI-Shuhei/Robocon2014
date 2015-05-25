/*
 * jungle.cpp
 *
 *  Created on: 2014/02/06
 *      Author: Shuhei
 */

#include "jungle.hpp"

JungleFlow::JungleFlow(Params *params) : BaseFlow(params){

	stopWatchNumber = STOPWATCH_NUMBER_0;
};

void JungleFlow::Calc(){
	if(params->lastStep != params->correntStep){
		ResetStopWatch(stopWatchNumber);
		params->lastStep = params->correntStep;
	}
	params->correntStep = ConstrainInt(params->correntStep,0,STEP_NUM);
	int motionStep;
	//
	motionStep = params->correntStep-1;

	int motionTime = SetArms(motionStep);

	//
	if(params->correntStep == 0){

	}else{
		if(motionTime > 0 && GetStopWatchMillis(stopWatchNumber) > motionTime){
			params->correntStep++;
		}
	}
};

void JungleFlow::Initialize(){
	params->lastStep = 0;
	StopStopWatch(stopWatchNumber);
	ResetStopWatch(stopWatchNumber);

}


int JungleFlow::SetArms(int step){
	Params::data motion[MOTION_NUM][TOTAL_ARMS_NUM] = {
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{84,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000},
			{84,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000},
			{84,70,-80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

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



