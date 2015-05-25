/*
 * slave_flow.hpp
 *
 *  Created on: 2014/02/05
 *      Author: Shuhei
 */

#ifndef SLAVE_FLOW_HPP_
#define SLAVE_FLOW_HPP_

#include"mars_stm32.h"
#include"mars_USART.h"

#define ARM_NUM (3)
#define FOOT_NUM (4)

#define MASTER_READ_LEN (28)

#define TIME_OUT (200)


class SlaveFlow{
	typedef short data;
	//ピン番号
	PIN_NUMBER l_arm[ARM_NUM];
	PIN_NUMBER r_arm[ARM_NUM];
	PIN_NUMBER l_foot[FOOT_NUM];
	PIN_NUMBER r_foot[FOOT_NUM];
	USART_TypeDef* master;
	//ピンデータ
	data l_arm_d[ARM_NUM];
	data r_arm_d[ARM_NUM];
	data l_foot_d[FOOT_NUM];
	data r_foot_d[FOOT_NUM];
	char master_read[MASTER_READ_LEN];

public :
	bool connected;
public :
	SlaveFlow();
	void Input();
	void Output();
private :
	void Decode();
	void Calc();	//いらないと思う
	void Write();
	void PinAssign();

};



#endif /* SLAVE_FLOW_HPP_ */
