/*
 * main.hpp
 *
 *  Created on: 2014/02/03
 *      Author: FMV
 *  This is C++ file, but C format.
 */

#ifndef MAIN_HPP_
#define MAIN_HPP_



#include "mars_stm32.h"
#include "mars_USART.h"
#include "mars_functions.h"
#include "mars_make_servo_moter.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * pid parameters
 */
#define KATA_MIDDLE		(35)
#define KATA_SPAN		(30)
#define KATA_CCW		(560)
#define KATA_CW			(410)
#define KATA_KP			(3)
#define KATA_KI			(8)
#define KATA_KD			(0)

#define HIJI_MIDDLE		(130)
#define HIJI_SPAN		(90)
#define HIJI_CCW		(580)
#define HIJI_CW			(420)
#define HIJI_KP			(3.5)
#define HIJI_KI			(3)
#define HIJI_KD			(0)



#define DUTY_LIMIT		(150)
#define USART_BAUDRATE	(9600)
#define TIME_OUT	(100)

#define CONTROLER_READ_LENGTH	(5)



#define JIDOUKI_REPLY_DELAY	(100)
#define JIDOUKI_OUT	(400)
#define OPERATION_BUFFER_SIZE (10)

static float			leftFootGain = 0.95,
rightFootGain = 1.0;




typedef enum MODE_SELECT{
	SEESAW	= 0x00,
	SWING	= 0x04,
	POLE	= 0x08,
	JUNGLE	= 0x0c,
	START_UP,
}MODE_SELECT;


/**
 * Controler's paramater.
 */
typedef enum CONTROLER_DATA{
	CONTROLER_RUNNING =	0x40,
	CONTROLER_SELECT =	0x20,
	CONTROLER_START =	0x10,
	CONTROLER_LEFT =	0x08,
	CONTROLER_UP = 		0x04,
	CONTROLER_RIGHT =	0x02,
	CONTROLER_DOWM =	0x01,

	CONTROLER_RECT =	0x40,
	CONTROLER_TRIANGLE =0x20,
	CONTROLER_CIRCLE =	0x10,
	CONTROLER_CROSS =	0x08,
	CONTROLER_L1 =		0x04,
	CONTROLER_R1 = 		0x02,

	CONTROLER_L2 =		0x40,
	CONTROLER_R2 =		0x20,

	CONTROLER_NUMBER =	0x3f,
	CONTROLER_MINUS =	0x40,
}CONTROLER_DATA;

static char			controlerReadData[CONTROLER_READ_LENGTH];
static int				controlerConnected = 0,
		controlerArrived   = 0;
static int controlerStopWatch = 4;


/**
 * Pin assignments.
 */
static PIN_NUMBER		LEFT_FOOT =		PC9,
		RIGHT_FOOT =	PA8,
		ARM0_MOTOR =	PC8,
		ARM1_MOTOR =	PC7,
		ARM2_SERVO =	PB6,
		ARM3_SERVO =	PB7,
		ARM4_SERVO =	PB8,
		ARM1_VOLUME =	PC0,
		//ARM2_VOLUME =	PC3,
		COIL		=	PB1;
static USART_TypeDef	*JIDOUKI =		USART2,
		*CONTROLER =	USART1;

static int				leftFootDuty =	500,
		rightFootDuty =	500,
		arm0Duty =		500,
		arm1Degree =	0,
		arm2Degree =	90,
		arm3Degree =	0,
		arm4Degree =	0;

static IO_VALUE			coilHighLow =	HIGH;

static DC_SERVO			ARM1_AS_SERVO;

static MODE_SELECT		correntMode		= START_UP;
static int 			correntStep		= 0;
static int lastStep = 0;

static int				isRunning	=	0,
		isManual	=	1;

static int				modeChangedEdge	=	0,
		stepChangedEdge	=	0;

static void PinAssignmentInitialize(void);
static void ParamOutput(void);
static void DisconnectedParamOutput(void);
static void ManualFlow(void);
static void SeesawFlow(void);
static void SwingFlow(void);
static void PoleFlow(void);
static void JungleFlow(void);
static void StartUpFlow(void);

static int GetModeChangedEdge();
static int GetStepChagedEdge();

static void PreFlow(void);






 typedef enum OPERATION_TYPE{
	 O_MODE,O_STEP,O_MISSED,O_NOP
 }OPERATION_TYPE;

 typedef struct OPERATION{
	 OPERATION_TYPE type;
	 MODE_SELECT mode;
	 int step;
 }OPERATION;


 typedef enum OPERATION_STATE{
	 OS_NOP,OS_SEND,OS_CATCH,OS_CMP,OS_RE,
 }OPERATION_STATE;


 static int opeStopWatch = 3;



 typedef struct OPERATION_LING_BUFFER{
	 OPERATION buffer[OPERATION_BUFFER_SIZE];
	 short start;
	 short end;
	 short size;

 }OPERATION_LING_BUFFER;


 static OPERATION_LING_BUFFER opeBuffer;

 static OPERATION correntOperation;

 static OPERATION_STATE oState = OS_NOP;
 static void InitializeOperationBuffer(void);
 static void AddModeOperation(MODE_SELECT select);
 static void AddStepOperation(int selectStep);
 static void ClearOperation(void);
 static void UpdateOperation(void);


 /**
  * About automatic robot.
  */

 static MODE_SELECT 				jidoukiMode =	SEESAW;
 static int						jidoukiStep	=	0;
 static int 					jidoukiRxEdge =	0;
 static OPERATION_TYPE			jidoukiOpeType = O_NOP;
 static int						jidoukiReply = 0;
 static int						jidoukiStopWatch = 5;



 static void JidoukiWrite(char str[]);
 static void JidoukiModeWrite(MODE_SELECT select);
 static void JidoukiStepWrite(int step);
 static int GetJidoukiRxEdge();
 static OPERATION_TYPE GetJidoukiType();
 static MODE_SELECT GetJidoukiMode();
 static int GetJidoukiStep();
 static void UpdateJidouki();



static void Log(char *str,int type = 0);
#ifdef _DEBUG
 static void PCDebugInitialize(void);
 static void PCDebug(void);
 static int correntControle = 0;
 static int arm1Duty = 500,arm2Duty = 500;
 static int arm1Output = 500,arm2Output = 500;
 static bool communicationOn = true;
static int time;
static int running = 0;

#endif

static PIN_NUMBER	pilot[5] = {PB0,PC5,PC4,PA7,PA6};
static IO_VALUE		pilotValue[5] = {LOW};
static int pilotFlash[5] = {0};
static int 		pilotNum = 5;
static void InitializePilot();
static void UpdatePilotLED();
static void AddPilotPattern(int index,int millis);
static void WriteModePilot();



#endif /* MAIN_HPP_ */
