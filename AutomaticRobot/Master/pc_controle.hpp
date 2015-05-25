/*
 * pc_controle.hpp
 *
 *  Created on: 2014/02/05
 *      Author: Shuhei
 */

#ifndef PC_CONTROLE_HPP_
#define PC_CONTROLE_HPP_


#include"base_flow.hpp"

class PCControle : public BaseFlow{
private :
	enum{
		PC_READ_LEN = 37,
		TIME_OUT = 100,

	};
	char pc_read[PC_READ_LEN];
	bool connected;
	int stopWatchNumber0;
	int stopWatchNumber1;
	USART_TypeDef *PC_PORT;
public :
	PCControle(Params *params);
	void Initialize();
private :
	void InputFromPC();
	void Calc();
};



#endif /* PC_CONTROLE_HPP_ */
