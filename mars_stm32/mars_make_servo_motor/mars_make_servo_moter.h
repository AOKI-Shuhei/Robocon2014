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


#define DC_SERVO_DT (0.0001)        //��������
/**
 * DC���[�^���T�[�{�Ƃ��Ĉ������߂�PWM�o�̓s����ADC�s���̊֘A�t���⑼�̃p�����[�^�̊֘A�t���ƂȂ�\����
 *
 * @Author			AOKI
 */
typedef struct DC_SERVO{
	PIN_NUMBER pwm_pin;	//PWM�o�͂���s��
	PIN_NUMBER adc_pin;	//ADC����s��
	int max_degree;		//�����I���E�p�x
	int middle_degree;
	int ccw_lpv;		//�����v���ł̃��[�^���쓮�ł���PWM�̍Œ�l
	int cw_lpv;			//���v���ł̃��[�^���쓮�ł���PWM�̍Œ�l
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
