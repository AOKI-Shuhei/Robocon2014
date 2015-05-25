/*
 * seesaw.hpp
 *
 *  Created on: 2014/02/03
 *      Author: FMV
 */

#ifndef SEESAW_HPP_
#define SEESAW_HPP_

#include"base_flow.hpp"

class SeesawFlow : public BaseFlow{
private :
	enum{
		STEP_NUM = 6,
		MOTION_NUM = 5,
	};

	int stopWatchNumber;

public :
	SeesawFlow(Params *params);
	void Initialize();
	void Calc();
private :

	int SetArms(int step);

};

#endif /* SEESAW_HPP_ */
