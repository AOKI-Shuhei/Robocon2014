/*
 * swing.hpp
 *
 *  Created on: 2014/02/06
 *      Author: Shuhei
 */

#ifndef SWING_HPP_
#define SWING_HPP_

#include"base_flow.hpp"

class SwingFlow : public BaseFlow{
public :
	SwingFlow(Params *params);
	void Initialize();
	void Calc();
private :
	int stopWatchNumber;
	enum{
		STEP_NUM = 18,
		MOTION_NUM = 16,
	};

	int SetArms(int step);


};
/**
 * 0:0 = constrain(corrent-2, 0 ,max)
 * 1:0 = constrain(corrent-2, 0, max)
 * 2:1
 * 3:1 = corrent-2	wait disconnect and when disconnected, step 4
 * 4: corrent - 2 = 2
 * :
 * :
 * 13:corrent - 2 = 11
 * 14:12	wait connect and when connected, step 15
 * 15:13	= corrent - 2
 * 16:14
 *
 *
 */




#endif /* SWING_HPP_ */
