/*
 * BaseFlow.hpp
 *
 *  Created on: 2014/02/03
 *      Author: AOKI
 *      It's master controler.
 */

#ifndef BASEFLOW_HPP_
#define BASEFLOW_HPP_


#include"mars_stm32.h"
#include"mars_USART.h"
#include <string.h>
#include <stdlib.h>

#define SLAVE_SEND_TIMING_STOPWATCH		(0)
#define CONNECTION_STOPWATCH			(1)
#define STOPWATCH_NUMBER_0				(2)
#define STOPWATCH_NUMBER_1				(3)


#ifdef _EMERGENCY
#define LOW_BATTERY_BORDER	(7400)
#endif
/**



 * 抽象クラス
 * これを継承してCalcとInitializeは最低限実装。
 */



enum {
		ARM_NUM = 4,
		FOOT_NUM = 5,
};




class Connection{
private:
	bool doCall;
	bool calling;
	int stopWatchNumber;
	bool connected;
	int timeOut;
public:
	bool replyed;
	Connection(int timeOut);
	void UpdateCallState(USART_TypeDef *shudouki);
	void StartCall(bool defaltState);
	void StopCall();
	bool GetConnection();
};

/**
 * モード設定用
 */
typedef enum MODE_SELECT{
	SEESAW,SWING,POLE,JUNGLE,
}MODE_SELECT;


struct Params{
	typedef short data;
	typedef char s_data;
	data
	leftArms[ARM_NUM],
	rightArms[ARM_NUM],
	leftFoots[FOOT_NUM],
	rightFoots[FOOT_NUM],
	sabaash[2];
	IO_VALUE
	leftArmSensor,
	rightArmSensor,
	leftFootSensor,
	rightFootSensor,
	relay;

	MODE_SELECT
	correntMode,
	nextMode;

	int correntStep;
	int lastStep;
	Connection *connection;
	Params(Connection *connection);
};




class BaseFlow{
protected :
	enum{
		TOTAL_ARMS_NUM = 19,
	};
	Params *params;

public :
	virtual void Initialize() = 0;
	virtual void Calc() = 0;
protected :
	BaseFlow(Params *params){
		this->params = params;
	}
};



#endif /* BASEFLOW_HPP_ */
