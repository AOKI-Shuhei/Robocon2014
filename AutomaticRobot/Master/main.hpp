/*
 * main.hpp
 *
 *  Created on: 2014/02/03
 *      Author: FMV
 */

#ifndef MAIN_HPP_
#define MAIN_HPP_


#include "mars_stm32.h"
#include "flow_controle.hpp"

#ifdef _PC_CONNECT

#include "pc_controle.hpp"

#else

#include "seesaw.hpp"
#include "swing.hpp"
#include "pole.hpp"
#include "jungle.hpp"

#endif





#endif /* MAIN_HPP_ */
