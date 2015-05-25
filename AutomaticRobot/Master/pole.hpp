/*
 * pole.hpp
 *
 *  Created on: 2014/02/06
 *      Author: Shuhei
 */

#ifndef POLE_HPP_
#define POLE_HPP_

#include"base_flow.hpp"

class PoleFlow : public BaseFlow{
public :
	PoleFlow(Params *parmas);
	void Initialize();
	void Calc();
private:
	enum{
		MOTION_NUM = 224,
		STEP_NUM = 226,
	};
	int stopWatchNumber;

	int SetArms(int step);

};


#endif /* POLE_HPP_ */
