/*
 * main.c
 *
 *  Created on: 2014/05/12
 *      Author: Shuhei
 */

#include "main.hpp"



/**
 * arms limits
 */
int Arm1Constrain(int x){
	return ConstrainInt(x,-30,30);
}
int Arm2Constrain(int x){
	return ConstrainInt(x,-88,89);
}
int Arm3Constrain(int x){
	return ConstrainInt(x,-90,90);
}
int Arm4Constrain(int x){
	return ConstrainInt(x,-90,0);
}


void main(void){



	PinAssignmentInitialize();
	InitializePilot();

#ifdef _DEBUG
	PCDebugInitialize();
#endif
	while(1){
		/**
		 * input from controler
		 *
		 */
		if(USART_Readnbyte(CONTROLER,controlerReadData,CONTROLER_READ_LENGTH) == 0){
			RestartStopWatch(controlerStopWatch);
			controlerConnected = 1;
			controlerArrived = 1;

		}else{
			controlerArrived = 0;
			if(GetStopWatchMillis(controlerStopWatch) > TIME_OUT){controlerConnected = 0;}
		}
#ifdef _DEBUG
		switch(correntControle){
		case 0:
#endif
			//decode controlerReadData[0] when data arrived
			if(controlerArrived){
				PreFlow();
				if(isRunning){
					switch(correntMode){
					case SEESAW:
						SeesawFlow();
						break;
					case SWING:
						SwingFlow();
						break;
					case POLE:
						PoleFlow();
						break;
					case JUNGLE:
						JungleFlow();
						break;
					case START_UP:
						StartUpFlow();
						break;
					}
					//override param if manual
					if(isManual){
						ManualFlow();
					}
					//make datas for sending to controler and send
				}
			}
			//write pin value
			if(controlerConnected && isRunning){
				ParamOutput();
			}else{
				DisconnectedParamOutput();
			}
			UpdateJidouki();
			UpdateOperation();
			WriteModePilot();
			UpdatePilotLED();
#ifdef _DEBUG
			break;
		case 1:
			if(controlerArrived){
				bool output1 = controlerReadData[2] & CONTROLER_R2;
				bool output2 = controlerReadData[2] & CONTROLER_L2;
				if(controlerReadData[0] & CONTROLER_SELECT){
					coilHighLow = (IO_VALUE)!coilHighLow;
					DC_SERVO_ClearIntegral(&ARM1_AS_SERVO);
					//DC_SERVO_ClearIntegral(&ARM2_AS_SERVO);
				}
				if(output1){
					arm1Output = arm1Duty;
				}else{
					arm1Output = 500;
				}
				if(output2){
					arm2Output = arm2Duty;
				}else{
					arm2Output = 500;
				}

			}
			if(controlerConnected){
				ADC_Read(ARM1_VOLUME);
				//ADC_Read(ARM2_VOLUME);
				PWM_Write(LEFT_FOOT,500);
				PWM_Write(RIGHT_FOOT,500);
				PWM_Write(ARM0_MOTOR,500);
				DC_SERVO_ClearIntegral(&ARM1_AS_SERVO);
				//DC_SERVO_ClearIntegral(&ARM2_AS_SERVO);
				SERVO_Write(ARM3_SERVO,arm3Degree);
				SERVO_Write(ARM4_SERVO,arm4Degree);
				IO_Write(COIL,coilHighLow);
				PWM_Write(ARM1_MOTOR,arm1Output);
				//PWM_Write(ARM2_MOTOR,arm2Output);
			}else{
				DisconnectedParamOutput();
			}
			break;
		}

		PCDebug();
#endif
	}

}


/**
 * about jidouki
 */
void JidoukiWrite(char str[]){
	Log(str,1);
#ifndef _NO_CONNECTION
	USART_Print(JIDOUKI,str);
#endif
}

int TryGetMode(char str[],MODE_SELECT *select){
	if(strcmp(str,"seesaw") == 0){
		*select = SEESAW;
	}else if(strcmp(str,"swing") == 0){
		*select = SWING;
	}else if(strcmp(str,"pole") == 0){
		*select = POLE;
	}else if(strcmp(str,"jungle") == 0){
		*select = JUNGLE;
	}else{
		return 0;
	}
	return 1;
}

void JidoukiModeWrite(MODE_SELECT select){
	char str[20] = "mode ";
	switch(select){
	case SEESAW:
		strcat(str,"seesaw");
		break;
	case SWING:
		strcat(str,"swing");
		break;
	case POLE:
		strcat(str,"pole");
		break;
	case JUNGLE:
		strcat(str,"jungle");
	}
	strcat(str,";");
	JidoukiWrite(str);
}

void JidoukiStepWrite(int step){
	char str[20];
	sprintf(str,"step %d;",step);
	JidoukiWrite(str);
}

int GetJidoukiStep(void){
	return jidoukiStep;
}
MODE_SELECT GetJidoukiMode(void){
	return jidoukiMode;
}
int GetJidoukiRxEdge(void){
	int temp = jidoukiRxEdge;
	jidoukiRxEdge = 0;
	return temp;
}
OPERATION_TYPE GetJidoukiType(){
	return jidoukiOpeType;
}

void SetJidoukiReply(){
	if(!jidoukiReply){
		RestartStopWatch(jidoukiStopWatch);
	}
	jidoukiReply = 1;

}
int GetJidoukiReply(){
	if(jidoukiReply){
		if(GetStopWatchMillis(jidoukiStopWatch) > JIDOUKI_REPLY_DELAY){
			jidoukiReply = 0;
			return 1;
		}
	}
	return 0;
}

void UpdateJidouki(){
	int rxGet;
	char rxData[20];
	char *opcode;
	char *operand;
	int opeSize = 0;

	/**
	 * It's automatically done replying "yes"
	 */
	if(GetJidoukiReply()){
		JidoukiWrite("yes;");
	}

	rxGet = USART_ReadSplit(JIDOUKI,';',rxData,20);
	if(USART_GetRxBufferSize(JIDOUKI) > 20){
		USART_ClearRxBuffer(JIDOUKI);
	}
	rxGet = rxGet < 0 ? 0 : rxGet;
	if(rxGet){
		Log(rxData,2);
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
			if(strcmp(opcode,"hello") == 0){
				SetJidoukiReply();

			}else{
				jidoukiOpeType = O_MISSED;
				jidoukiRxEdge = 1;
			}
			break;
		case 2:
			if(strcmp(opcode,"cmpmode") == 0){
				if(TryGetMode(operand,&jidoukiMode)){
					jidoukiOpeType = O_MODE;
					jidoukiRxEdge = 1;
				}else{
					jidoukiOpeType = O_MISSED;
					jidoukiRxEdge = 1;
				}
			}else if(strcmp(opcode,"cmpstep") == 0){
				jidoukiStep = atoi(operand);
				if(jidoukiStep == 0 && *operand != '0'){
					jidoukiOpeType = O_MISSED;
					jidoukiRxEdge = 1;
				}else{
					jidoukiOpeType = O_STEP;
					jidoukiRxEdge = 1;
				}
			}else{
				jidoukiOpeType = O_MISSED;
				jidoukiRxEdge = 1;
			}
			break;
		default:
			jidoukiOpeType = O_MISSED;
			jidoukiRxEdge = 1;
			break;
		}
	}
}



int KataHash(int degree){
	int hash[] = {3655,3628,3580,3535,3479,3436,3389,3348,3296,3232,3195,3128,3101,3052,3006,2960,2908,2878,2813,2761,2725,2675,2634,2567,2540,2500,2462,2394,2341,2281,2241,2187,2126,2091,2054,1993,1952,1890,1816,1781,1731,1678,1631,1577,1532,1501,1436,1430,1405,1281,1272,1232,1163,1107,1059,1020,994,948,861,833,813,717,668,640,568,560,507,454,416,385,362};
	int max = 70;
	degree = ConstrainInt(degree,0,max-1);
	return hash[degree];
}
int HijiHash(int degree){
	degree = ConstrainInt(degree,5,295);
	degree = 300-degree;
	return degree*13;
}
void PinAssignmentInitialize(void){
	int i;
	RCC_Configuration();
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE);

	PinMode(LEFT_FOOT,PWM);
	PinMode(RIGHT_FOOT,PWM);
	PinMode(ARM0_MOTOR,PWM);
	DC_SERVO_StructInitialize(&ARM1_AS_SERVO,ARM1_MOTOR,ARM1_VOLUME,KATA_MIDDLE,KATA_SPAN,KATA_CCW,KATA_CW);
	DC_SERVO_SetKP_KI_KD(&ARM1_AS_SERVO,KATA_KP,KATA_KI,KATA_KD);
	DC_SERVO_RegisterHash(&ARM1_AS_SERVO,&KataHash);
	/*
	DC_SERVO_StructInitialize(&ARM2_AS_SERVO,ARM2_MOTOR,ARM2_VOLUME,HIJI_MIDDLE,HIJI_SPAN,HIJI_CCW,HIJI_CW);
	DC_SERVO_SetKP_KI_KD(&ARM2_AS_SERVO,HIJI_KP,HIJI_KI,HIJI_KD);
	DC_SERVO_RegisterHash(&ARM2_AS_SERVO,&HijiHash);
	*/
	PinMode(ARM2_SERVO,SERVO);
	PinMode(ARM3_SERVO,SERVO);
	PinMode(ARM4_SERVO,SERVO);
	PinMode(COIL,IO_OUTPUT);
	USART_Mode(JIDOUKI,SEND_AND_READ,DISABLE);
	USART_Mode(CONTROLER,SEND_AND_READ,DISABLE);
	USART_Begin(JIDOUKI,USART_BAUDRATE);
	USART_Begin(CONTROLER,USART_BAUDRATE);

	for(i=0;i<CONTROLER_READ_LENGTH;i++){
		controlerReadData[i] = 0;
	}
	InitializeOperationBuffer();

	DisconnectedParamOutput();
}
void ParamOutput(void){

	PWM_Write(LEFT_FOOT,leftFootDuty);
	PWM_Write(RIGHT_FOOT,rightFootDuty);
	PWM_Write(ARM0_MOTOR,arm0Duty);
	//PWM_Write(ARM1_MOTOR,500);
	arm1Degree = Arm1Constrain(arm1Degree);
	arm2Degree = Arm2Constrain(arm2Degree);
	arm3Degree = Arm3Constrain(arm3Degree);
	arm4Degree = Arm4Constrain(arm4Degree);
	DC_SERVO_Change(&ARM1_AS_SERVO,arm1Degree);
	//DC_SERVO_Change(&ARM2_AS_SERVO,arm2Degree);
	//PWM_Write(ARM2_MOTOR,arm2Degree);
	SERVO_Write(ARM2_SERVO,arm2Degree);
	SERVO_Write(ARM3_SERVO,arm3Degree);
	SERVO_Write(ARM4_SERVO,arm4Degree);
	IO_Write(COIL,coilHighLow);

}
void DisconnectedParamOutput(void){
	ADC_Read(ARM1_VOLUME);
	//ADC_Read(ARM2_VOLUME);
	PWM_Write(LEFT_FOOT,500);
	PWM_Write(RIGHT_FOOT,500);
	PWM_Write(ARM0_MOTOR,500);
	PWM_Write(ARM1_MOTOR,500);
	arm1Degree = Arm1Constrain(arm1Degree);
	arm2Degree = Arm2Constrain(arm2Degree);
	arm3Degree = Arm3Constrain(arm3Degree);
	arm4Degree = Arm4Constrain(arm4Degree);
	//PWM_Write(ARM2_MOTOR,500);
	DC_SERVO_ClearIntegral(&ARM1_AS_SERVO);
	//DC_SERVO_ClearIntegral(&ARM2_AS_SERVO);
	SERVO_Write(ARM2_SERVO,arm2Degree);
	SERVO_Write(ARM3_SERVO,arm3Degree);
	SERVO_Write(ARM4_SERVO,arm4Degree);
	IO_Write(COIL,coilHighLow);
}
void ManualFlow(void){
	/**
	 * get arms change
	 * can inverse controles
	 */
	int temp;
	temp = (controlerReadData[0] & CONTROLER_UP ? -1 : 0) + (controlerReadData[0] & CONTROLER_DOWM ? 1 : 0);
	arm1Degree	= Arm1Constrain(arm1Degree+temp);
	temp = (controlerReadData[0] & CONTROLER_RIGHT ? 1 : 0) + (controlerReadData[0] & CONTROLER_LEFT ? -1 : 0);
	arm2Degree	= Arm2Constrain(arm2Degree+temp);
	temp = (controlerReadData[1] & CONTROLER_RECT ? 1 : 0) + (controlerReadData[1] & CONTROLER_CIRCLE ? -1 : 0);
	arm3Degree	= Arm3Constrain(arm3Degree+temp);
	temp = (controlerReadData[1] & CONTROLER_TRIANGLE ? -1 : 0) + (controlerReadData[1] & CONTROLER_CROSS ? 1 : 0);
	arm4Degree	= Arm4Constrain(arm4Degree+temp);
}
int ByteToDuty(char data,float gain){
	int rslt = (data & CONTROLER_MINUS) ? -1 : 1;
	rslt *= ((((500-DUTY_LIMIT)*(data&CONTROLER_NUMBER))*gain)/63);
	return rslt + 500;
}
void PreFlow(void){
	/**
	 * get isManual toggle
	 * is running or not
	 * get coil toggle
	 * get mode
	 * get step change
	 * get foots param
	 * get turn table param
	 */
	if(controlerReadData[0] & CONTROLER_SELECT){
		coilHighLow = (IO_VALUE)!coilHighLow;
		DC_SERVO_ClearIntegral(&ARM1_AS_SERVO);
		//DC_SERVO_ClearIntegral(&ARM2_AS_SERVO);
	}
	isRunning = controlerReadData[0] & CONTROLER_RUNNING;
	if(controlerReadData[0] & CONTROLER_START){
		isManual = !isManual;
	}
	if(isRunning){
		if(!isManual){
			bool tempB = true;
			MODE_SELECT nextMode;
			if(controlerReadData[1] & CONTROLER_CROSS){
				nextMode = SEESAW;
			}else if(controlerReadData[1] & CONTROLER_CIRCLE){
				nextMode = SWING;
			}else if(controlerReadData[1] & CONTROLER_RECT){
				nextMode = POLE;
			}else if(controlerReadData[1] & CONTROLER_TRIANGLE){
				nextMode = JUNGLE;
			}else{
				tempB = false;
			}

			if(tempB){
				correntMode = nextMode;
				correntStep = 0;
				lastStep = 0;
				modeChangedEdge = 1;
				ClearOperation();
				AddModeOperation(correntMode);
			}
		}
		correntStep += (controlerReadData[1] & CONTROLER_R1 ? 1 : 0);
		correntStep -= (controlerReadData[1] & CONTROLER_L1 ? 1 : 0);
		if(lastStep != correntStep){
			stepChangedEdge = 1;
			lastStep = correntStep;
		}else{
			stepChangedEdge = 0;
		}
		leftFootDuty	= ByteToDuty(controlerReadData[3],leftFootGain);
		rightFootDuty	= ByteToDuty(controlerReadData[4],rightFootGain);
		arm0Duty		= 500 + (controlerReadData[2] & CONTROLER_R2 ? (coilHighLow == HIGH ? 300 : 150) : 0) + (controlerReadData[2] & CONTROLER_L2 ? (coilHighLow == HIGH ? -300 : -150) : 0);
	}else{
		if(correntMode != START_UP){
			correntStep = 0;
			lastStep = 0;
			modeChangedEdge = 1;
			ClearOperation();
			AddModeOperation(SEESAW);
			correntMode = START_UP;
		}
	}
}

void SeesawFlow(void){


	/*
	int arm1StepParams[]	= {-29,6,20,-16,-16,19,-9,18,23};
	int arm2StepParams[]	= {89,89,89,89,-88,-88,-88,89,89};
	int arm3StepParams[]	= {-4,-4,-4,-4,-4,-4,-4,-9,-9};
	int arm4StepParams[]	= {0,-53,-63,-63,-63,-63,-63,-54,-32};
	int motionNum			= 9;
	*/
//13
	int arm1StepParams[]	= {-24,24,-13,-13,17,-13,-13,18,24,-26};
	int arm2StepParams[]	= {89,89,89,-88,-88,-88,89,87,87,87};
	int arm3StepParams[]	= {0,0,0,0,0,0,0,0,0,0};
	int arm4StepParams[]	= {-21,-63,-22,-22,-22,-22,-22,-54,-54,-26};
	int motionNum			= 10;
	int stepNum = 12;

	int innerStep;
	bool jidoukiHaveToSend = false;


	int jidoukiOpeStep = 0;

	correntStep = ConstrainInt(correntStep,0,stepNum-1);



	//about inner hash

	switch(correntStep){
	case 0:
	case 1:
		innerStep = 0;
		break;
	case 2:
	case 3:
		innerStep = 1;
		break;
	case 4:
		if(stepChangedEdge && coilHighLow == HIGH){
			coilHighLow = LOW;
		}
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		innerStep = correntStep -2;
		break;
	case 10:
		if(stepChangedEdge && coilHighLow == LOW){
			coilHighLow = HIGH;
		}
	case 11:
		innerStep = correntStep -2;
		break;

	}
	//about send to jidouki

	switch(correntStep){
	case 0:
	case 1:
		jidoukiOpeStep = correntStep;
		jidoukiHaveToSend = true;
		break;
	case 3:
		jidoukiHaveToSend = true;
		jidoukiOpeStep = 2;
		break;
	case 11:
		jidoukiHaveToSend = true;
		jidoukiOpeStep = 5;
	}
	if(stepChangedEdge && jidoukiHaveToSend){
		AddStepOperation(jidoukiOpeStep);
	}


	innerStep = ConstrainInt(innerStep,0,motionNum-1);


	if(!isManual){
		arm1Degree = arm1StepParams[innerStep];
		arm2Degree = arm2StepParams[innerStep];
		arm3Degree = arm3StepParams[innerStep];
		arm4Degree = arm4StepParams[innerStep];
	}
}

void SwingFlow(void){
	/**
	 * 0:	hold				j:0
	 * 1:	approach			j:1
	 * 2:	ready and pose
	 * 3:	pose				j:2
	 * 4:	disconnect and up	j:3	and when disconnected, step 4
	 * 5:	reverse
	 * 6:	down
	 * 7:	reverse and up and approach to catch
	 * 8:	catch				j:sending hello, and when connected,step xx
	 * 9:	up					j:xx+1
	 */
	int arm1StepParams[]	= {-14,-2,7,16,-3,2,-30};
	int arm2StepParams[]	= {88,88,-1,-2,88,88,88};
	int arm3StepParams[]	= {0,0,0,0,0,0,0};
	int arm4StepParams[]	= {-5,-44,-9,-9,-17,-26,-17};
	int motionNum = 7;
	int stepNum = 11;
	int innerStep;
	int jidoukiOpeStep;
	bool jidoukiHaveToSend = false;
	correntStep = ConstrainInt(correntStep,0,stepNum-1);
	switch(correntStep){
	case 5:
		if(stepChangedEdge && coilHighLow == HIGH){
			coilHighLow = LOW;
		}
	case 0:
	case 1:
	case 2:
		innerStep = 0;
		break;
	case 3:
	case 4:
		innerStep = 1;
		break;
	case 9:
		if(stepChangedEdge && coilHighLow == LOW){
			coilHighLow = HIGH;
		}
	case 6:
	case 7:
	case 8:

	case 10:
		innerStep = correntStep -4;
		break;
	default:
		correntStep = 0;
		return;
	}

	switch(correntStep){
	case 0:
	case 1:
	case 2:
	case 4:
		jidoukiOpeStep = correntStep;
		jidoukiHaveToSend = true;
		break;
	case 10:
		jidoukiOpeStep = 16;		//?
		jidoukiHaveToSend = true;
		break;
	}
	if(stepChangedEdge && jidoukiHaveToSend){
		AddStepOperation(jidoukiOpeStep);
	}
	innerStep = ConstrainInt(innerStep,0,motionNum-1);


	if(!isManual){
		arm1Degree = arm1StepParams[innerStep];
		arm2Degree = arm2StepParams[innerStep];
		arm3Degree = arm3StepParams[innerStep];
		arm4Degree = arm4StepParams[innerStep];
	}

}

void PoleFlow(void){
	int arm1StepParams[]	= {-14,-2,7,16,-3,2,-30};
	int arm2StepParams[]	= {88,88,-1,-2,88,88,88};
	int arm3StepParams[]	= {0,0,0,0,0,0,0};
	int arm4StepParams[]	= {-5,-44,-9,-9,-17,-26,-17};
	int motionNum = 3;
	int stepNum = 6;
	int innerStep;
	int jidoukiOpeStep;
	bool jidoukiHaveToSend = false;
	correntStep = ConstrainInt(correntStep,0,stepNum-1);
	switch(correntStep){
	case 0:
		innerStep = 0;
		break;
	case 3:
		if(stepChangedEdge && coilHighLow == HIGH){
			coilHighLow = LOW;
		}
	case 1:
	case 2:

		innerStep = 1;
		break;
	case 4:
	case 5:
		innerStep = 2;

	}
	switch(correntStep){
	case 0:
	case 1:
	case 2:
	case 3:
		jidoukiOpeStep = correntStep;
		jidoukiHaveToSend = true;
		break;
	case 5:
		jidoukiOpeStep = 225;		//?
		jidoukiHaveToSend = true;
		break;
	}
	if(stepChangedEdge && jidoukiHaveToSend){
		AddStepOperation(jidoukiOpeStep);
	}
	innerStep = ConstrainInt(innerStep,0,motionNum-1);

/*
	if(!isManual){
		arm1Degree = arm1StepParams[innerStep];
		arm2Degree = arm2StepParams[innerStep];
		arm3Degree = arm3StepParams[innerStep];
		arm4Degree = arm4StepParams[innerStep];
	}
	*/
}

void JungleFlow(void){
	int arm1StepParams[]	= {0,0};
	int arm2StepParams[]	= {0,0};
	int arm3StepParams[]	= {0,0};
	int arm4StepParams[]	= {0,0};
	int stepNum				= 2;
}

void StartUpFlow(void){
	if(!isManual){
	arm1Degree = 0;
	arm2Degree = 88,
	arm3Degree = 0;
	arm4Degree = 0;
	}
}

int Ope_LingBuffer_In(OPERATION_TYPE type,MODE_SELECT select,int selectStep){
	if(opeBuffer.size >= OPERATION_BUFFER_SIZE){
		return -1;
	}
	opeBuffer.buffer[opeBuffer.end].type = type;
	opeBuffer.buffer[opeBuffer.end].mode = select;
	opeBuffer.buffer[opeBuffer.end].step = selectStep;
	opeBuffer.size++;
	opeBuffer.end = (opeBuffer.end + 1) % OPERATION_BUFFER_SIZE;
	return 0;
}
int Ope_LingBuffer_Out(OPERATION *ope){
	if(opeBuffer.size == 0){
		return -1;
	}
	ope->type = opeBuffer.buffer[opeBuffer.start].type;
	ope->mode = opeBuffer.buffer[opeBuffer.start].mode;
	ope->step = opeBuffer.buffer[opeBuffer.start].step;
	opeBuffer.size--;
	opeBuffer.start = (opeBuffer.start + 1) % OPERATION_BUFFER_SIZE;
	return 0;
}
void Ope_LingBuffer_Clear(){
	opeBuffer.start = 0;
	opeBuffer.end = 0;
	opeBuffer.size = 0;
}
void AddModeOperation(MODE_SELECT select){
	Ope_LingBuffer_In(O_MODE,select,0);
}



static void InitializeOperationBuffer(void){
	opeBuffer.start = 0;
	opeBuffer.size = 0;
	opeBuffer.end = 0;
}
void AddStepOperation(int selectStep){
	Ope_LingBuffer_In(O_STEP,SEESAW,selectStep);
}
void ClearOperation(void){
	Ope_LingBuffer_Clear();
	correntOperation.type = O_NOP;
	oState = OS_NOP;
}
void UpdateOperation(void){
	if(opeBuffer.size == 0 && correntOperation.type == O_NOP){
		oState = OS_NOP;
	}
	switch(oState){
	case OS_NOP:
		if(opeBuffer.size > 0){
			Ope_LingBuffer_Out(&correntOperation);
			oState = OS_SEND;
		}
		break;
	case OS_SEND:
		if( correntOperation.type != O_NOP){
			switch(correntOperation.type){
			case O_MODE:
				JidoukiModeWrite(correntOperation.mode);
				break;
			case O_STEP:
				JidoukiStepWrite(correntOperation.step);
				break;
			}
			RestartStopWatch(opeStopWatch);
#ifdef _NO_CONNECTION
			oState = OS_CMP;
#else
			oState = OS_CATCH;
#endif
		}else{
			oState = OS_NOP;
		}
		break;
	case OS_CATCH:
		if(correntOperation.type != O_NOP){
			if(GetJidoukiRxEdge()){
				if(correntOperation.type == GetJidoukiType()){
					switch(correntOperation.type){
					case O_MODE:
						if(correntOperation.mode == GetJidoukiMode()){
							oState = OS_CMP;
						}else{
							oState = OS_RE;
						}
						break;
					case O_STEP:
						if(correntOperation.step == GetJidoukiStep()){
							oState = OS_CMP;
						}else{
							oState = OS_RE;
						}
						break;
					}
				}else{
					oState = OS_RE;
				}
			}else if(GetStopWatchMillis(opeStopWatch) > JIDOUKI_OUT){
				oState = OS_RE;
			}
		}else{
			oState = OS_NOP;
		}
		break;
	case OS_CMP:
		correntOperation.type = O_NOP;
		oState = OS_NOP;
		break;
	case OS_RE:
		oState = OS_SEND;
	}
}




#ifdef _DEBUG

void PCDebugInitialize(void){
	USART_Mode(USART3,SEND_AND_READ,DISABLE);
	USART_Begin(USART3,9600);
}

void PCDebug(void){

	char data[100];
	int count;
	float tempf;
	char *op0, *op1;
	int opSize = 0;
	char tempS[200];
	count = USART_ReadSplit(USART3,';',data,100);
	if(USART_GetRxBufferSize(USART3) > 100){
		USART_ClearRxBuffer(USART3);
	}
	if(count > 0){
		op0 = strtok(data," ");
		op1 = strtok(NULL," ");
		if(op0 != NULL){
			opSize++;
		}
		if(op1 != NULL){
			opSize++;
		}
		switch(opSize){
		case 1:

			if(strcmp(op0,"hello") == 0){
				USART_Print(USART3,"---Hello---------\n\r");
				USART_Print(USART3,"-arm1 param\n");
				sprintf(data,"KP  = %1.3f\n\rKI  = %1.3f\n\rKD  = %1.3f\n\rCW  = %d\n\rCCW = %d\n\r",ARM1_AS_SERVO.kp,ARM1_AS_SERVO.ki,ARM1_AS_SERVO.kd,ARM1_AS_SERVO.cw_lpv,ARM1_AS_SERVO.ccw_lpv);
				USART_Print(USART3,data);
				USART_Print(USART3,"-Left foot gain = ");
				sprintf(data,"%2.2f\n\r",leftFootGain);
				USART_Print(USART3,data);
				USART_Print(USART3,"-Right foot gain = ");
				sprintf(data,"%2.2f\n\r",rightFootGain);
				USART_Print(USART3,data);
				return;
			}else if(strcmp(op0,"print") == 0){
				running = 1;
				time = Millis();
				return;
			}else if(strcmp(op0,"stop") == 0){
				running = 0;
				return;
			}else if(strcmp(op0,"mode") == 0){
				USART_Print(USART3,"Auto/Manual : ");
				if(isManual){
					USART_Print(USART3,"Manual\n\r");
				}else{
					USART_Print(USART3,"Auto\n\r");
				}
				USART_Print(USART3,"Mode : ");
				switch(correntMode){
				case SEESAW:
					USART_Print(USART3,"Seesaw\n\r");
					break;
				case SWING:
					USART_Print(USART3,"Swing\n\r");
					break;
				case POLE:
					USART_Print(USART3,"Pole\n\r");
					break;
				case JUNGLE:
					USART_Print(USART3,"Jungle\n\r");
					break;
				case START_UP:
					USART_Print(USART3,"Start up\n\r");
					break;
				}
			}else{
				USART_Print(USART3,"Invalid operator\n\r");
				return;
			}
			break;
		case 2:

			tempf = atof(op1);

			if(strcmp(op0,"leftfootgain") == 0){
				leftFootGain = tempf;
				sprintf(tempS,"Set Left Foot Gain : %f\n\r",leftFootGain);
			}else if(strcmp(op0,"rightfootgain") == 0){
				rightFootGain = tempf;
				sprintf(tempS,"Set Right Foot Gain : %f\n\r",rightFootGain);
			}else if(strcmp(op0,"arm1kp") == 0){
				DC_SERVO_SetKP_KI_KD(&ARM1_AS_SERVO,tempf,ARM1_AS_SERVO.ki,ARM1_AS_SERVO.kd);
				sprintf(tempS,"Set Arm1 KP : %f\n\r",ARM1_AS_SERVO.kp);
			}else if(strcmp(op0,"arm1ki") == 0){
				DC_SERVO_SetKP_KI_KD(&ARM1_AS_SERVO,ARM1_AS_SERVO.kp,tempf,ARM1_AS_SERVO.kd);
				sprintf(tempS,"Set Arm1 KI : %f\n\r",ARM1_AS_SERVO.ki);
			}else if(strcmp(op0,"arm1kd") == 0){
				DC_SERVO_SetKP_KI_KD(&ARM1_AS_SERVO,ARM1_AS_SERVO.kp,ARM1_AS_SERVO.ki,tempf);
				sprintf(tempS,"Set Arm1 KD : %f\n\r",ARM1_AS_SERVO.kd);
			}else if(strcmp(op0,"arm1cw") == 0){
				ARM1_AS_SERVO.cw_lpv = (int)tempf;
				sprintf(tempS,"Set Arm1 CW_LPV : %d\n\r",ARM1_AS_SERVO.cw_lpv);
			}else if(strcmp(op0,"arm1ccw") == 0){
				ARM1_AS_SERVO.ccw_lpv = (int)tempf;
				sprintf(tempS,"Set Arm1 CCW_LPV : %d\n\r",ARM1_AS_SERVO.ccw_lpv);
			}else if(strcmp(op0,"switch") == 0){
				if(strcmp(op1,"armsduty") == 0){
					correntControle = 1;
					sprintf(tempS,"Switch : Direct controle duty\n\r");
				}else if(strcmp(op1,"normal") == 0){
					correntControle = 0;
					sprintf(tempS,"Swith : Normal controle\n\r");
				}else{
					USART_Print(USART3,"Invalid operator\n\r");
					return;
				}
			}else if(strcmp(op0,"arm1") == 0){
				if(correntControle == 1){
					if(strcmp(op1,"++") == 0){
						arm1Duty = arm1Duty + 1 < 999 ? arm1Duty + 1 : 999;
					}else if(strcmp(op1,"+++") == 0){
						arm1Duty = arm1Duty + 5 < 999 ? arm1Duty + 5 : 999;
					}else if(strcmp(op1,"--") == 0){
						arm1Duty = arm1Duty - 1 > 0 ? arm1Duty - 1 : 0;
					}else if(strcmp(op1,"---") == 0){
						arm1Duty = arm1Duty - 5 > 0 ? arm1Duty - 5 : 0;
					}else{
						arm1Duty =  ConstrainInt((int)tempf,0,999);
					}
					sprintf(tempS,"Set Arm1 duty : %d\n\r",arm1Duty);
				}else{
					sprintf(tempS,"Cannot controle arms with duty\n\r");
				}
			}else if(strcmp(op0,"arm2") == 0){
				if(correntControle == 1){
					if(strcmp(op1,"++") == 0){
						arm2Duty = arm2Duty + 1 < 999 ? arm2Duty + 1 : 999;
					}else if(strcmp(op1,"+++") == 0){
						arm2Duty = arm2Duty + 5 < 999 ? arm2Duty + 5 : 999;
					}else if(strcmp(op1,"--") == 0){
						arm2Duty = arm2Duty - 1 > 0 ? arm2Duty - 1 : 0;
					}else if(strcmp(op1,"---") == 0){
						arm2Duty = arm2Duty - 5 > 0 ? arm2Duty - 5 : 0;
					}else{
						arm2Duty =  ConstrainInt((int)tempf,0,999);
					}
					sprintf(tempS,"Set Arm2 duty : %d\n\r",arm2Duty);
				}else{
					sprintf(tempS,"Cannot controle arms with duty\n\r");
				}
			}else if(strcmp(op0,"com") == 0){
				if(strcmp(op1,"on") == 0){
					communicationOn = true;
					Log("Communication log : on");
					return;
				}else if(strcmp(op1,"off") == 0){
					communicationOn = false;
					Log("Communication log : off");
					return;
				}else{
					USART_Print(USART3,"Invalid operator\n\r");
					return;
				}
			}else


			{
				USART_Print(USART3,"Invalid operator\n\r");
				return;
			}
			USART_Print(USART3,tempS);
			break;

		default:
			USART_Print(USART3,"Invalid operator\n");
			return;
		}
	}
	if(running){
		if(Millis() - time > 500){
			sprintf(data,"arm1: %4d, arm2: %4d, arm3: %4d, arm4: %4d, step: %3d, ",arm1Degree,arm2Degree,arm3Degree,arm4Degree,correntStep);
			USART_Print(USART3,data);
			USART_Print(USART3,"Mode: ");
			switch(correntMode){
			case SEESAW:
				USART_Print(USART3,"Seesaw, ");
				break;
			case SWING:
				USART_Print(USART3,"Swing, ");
				break;
			case POLE:
				USART_Print(USART3,"Pole, ");
				break;
			case JUNGLE:
				USART_Print(USART3,"Jungle, ");
				break;
			case START_UP:
				USART_Print(USART3,"Start up, ");
				break;
			}
			Log(isManual ? "Manual" : "Auto");
			time = Millis();
		}
	}

}





#endif


void Log(char *str,int type){
#ifdef _DEBUG
	switch(type){
	case 1:
		if(communicationOn){
			USART_Print(USART3,"S- ");
		}
		break;
	case 2:
		if(communicationOn){
			USART_Print(USART3,"J- ");
		}
		break;
	default:
		break;
	}
	if(communicationOn || (type != 1 && type != 2)){
		USART_Print(USART3,str);
		USART_Print(USART3,"\n");
	}
#endif
}


void InitializePilot(){
	for(int i=0;i<pilotNum;i++){
		PinMode(pilot[i],IO_OUTPUT);
	}
}
void UpdatePilotLED(){
	for(int i=0;i<pilotNum;i++){
		IO_Write(pilot[i],pilotValue[i]);
	}
}

void AddPilotPattern(int index,IO_VALUE value,int millis){
	pilotValue[index] = value;
}

void WriteModePilot(){
	for(int i=1;i<5;i++){
		AddPilotPattern(i,LOW,0);
	}
	switch(correntMode){
	case START_UP:
		break;
	case SEESAW:
		AddPilotPattern(1,HIGH,0);
		break;
	case SWING:
		AddPilotPattern(2,HIGH,0);
		break;
	case POLE:
		AddPilotPattern(3,HIGH,0);
		break;
	case JUNGLE:
		AddPilotPattern(4,HIGH,0);
		break;
	}
	if(isManual){
		AddPilotPattern(0,LOW,0);
	}else{
		AddPilotPattern(0,HIGH,0);
	}
}
