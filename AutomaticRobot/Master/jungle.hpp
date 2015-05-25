/*
 * jungle.hpp
 *
 *  Created on: 2014/02/06
 *      Author: Shuhei
 */

#ifndef JUNGLE_HPP_
#define JUNGLE_HPP_

#include "base_flow.hpp"

class JungleFlow : public BaseFlow{
public :
	JungleFlow(Params *params);
	void Initialize();
	void Calc();
private:
	enum{
		MOTION_NUM = 4,
		STEP_NUM = 4,
	};


	int stopWatchNumber;

	int SetArms(int step);

};



#endif /* JUNGLE_HPP_ */
