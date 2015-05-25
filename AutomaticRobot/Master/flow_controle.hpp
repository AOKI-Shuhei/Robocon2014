/*
 * flow_controle.hpp
 *
 *  Created on: 2014/05/09
 *      Author: Shuhei
 */

#ifndef FLOW_CONTROLE_HPP_
#define FLOW_CONTROLE_HPP_

#include "base_flow.hpp"




#ifdef _EMERGENCY
class Emergency{
private:
	int lastTime;
	int LEDTime;
	IO_VALUE on;
	int count;
	long totalBatteryVoltage;

PIN_NUMBER	on_when_voltage_low,
			battery;

public:
	Emergency();
	void Update();
	void Initialize();
};
#endif




class FlowControle{

public:

#ifdef _PC_CONNECT
	BaseFlow *pcControle;
#else
	BaseFlow *seesawFlow,
	*swingFlow,
	*poleFlow,
	*jungleFlow;
#endif

	Params *params;
private:
	BaseFlow *correntFlow;
#ifdef _EMERGENCY
	Emergency emergency;
#endif
	PIN_NUMBER LEFT_ARM,
	RIGHT_ARM,
	LEFT_FOOT,
	RIGHT_FOOT,
	LEFT_ARM_S,
	RIGHT_ARM_S,
	LEFT_FOOT_S,
	RIGHT_FOOT_S,
	SABAASH[2],
	RELAY;
	USART_TypeDef *SHUDOUKI,
	*SLAVE;
	int slaveSendTimingStopwatch;
public:
#ifdef _PC_CONNECT
	FlowControle(BaseFlow *pc);
#else
	FlowControle(BaseFlow *seesaw,BaseFlow *swing,BaseFlow *pole,BaseFlow *jungle);
#endif
	void Initialize();
	void Start(Params *params);
private:
	void Assignment();
	void Loop();
	void ScanSensor();
	void Write();
	void InputFromShudouki();
	void WriteStep(int step);
	void WriteMode(MODE_SELECT mode);
	bool TryGetMode(char *str,MODE_SELECT *mode);
};




#endif /* FLOW_CONTROLE_HPP_ */
