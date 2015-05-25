/*
 * mars_make_servo_moter.h
 *
 *  Created on: 2013/12/18
 *      Author: Shuhei
 */

#ifndef MARS_MAKE_SERVO_MOTER_H_
#define MARS_MAKE_SERVO_MOTER_H_
#include "mars_stm32.h"

#ifdef __cplusplus
 extern "C" {
#endif


#define DC_SERVO_DT (0.0001)        //微小時間
/**
 * DCモータをサーボとして扱うためにPWM出力ピンとADCピンの関連付けや他のパラメータの関連付けとなる構造体
 *
 * @Author			AOKI
 */
typedef struct DC_SERVO{
	PIN_NUMBER pwm_pin;	//PWM出力するピン
	PIN_NUMBER adc_pin;	//ADCするピン
	int max_degree;		//物理的限界角度
	int middle_degree;
	int ccw_lpv;		//反時計回りでのモータが駆動できるPWMの最低値
	int cw_lpv;			//時計回りでのモータが駆動できるPWMの最低値
	int diff[2];		//
	float integral;		//
	float p;				//
	float i;				//
	float d;				//
	float kp;
	float ki;
	float kd;
	int (*Hash)(int);
}DC_SERVO;

int DC_SERVO_StructInitialize(DC_SERVO *dc_servo,PIN_NUMBER pwm_pin,PIN_NUMBER adc_pin,int middle_degree,int max_degree,int ccw_lpv,int cw_lpv);
int DC_SERVO_SetKP_KI_KD(DC_SERVO *dc_servo,float kp,float ki,float kd);
int DC_SERVO_RegisterHash(DC_SERVO *dc_servo,int (*Hash)(int degree));
int DC_SERVO_Change(DC_SERVO *dc_servo,int target_degree);
int DC_SERVO_ClearIntegral(DC_SERVO *dc_servo);
static int TestHash(int degree);

#ifdef __cplusplus
 }
#endif

#endif /* MARS_MAKE_SERVO_MOTER_H_ */
