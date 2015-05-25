/*

 * mars_make_servo_moter.c
 *
 *  Created on: 2013/12/18
 *      Author: Shuhei
 */

#include "mars_make_servo_moter.h"

/**
 * DC���[�^���T�[�{�Ƃ��Ĉ������߂̍\���̂̏������֐�
 * @param dc_servo		�Ώۂ̍\����
 * @param pwm_pin		PWM�o�͂���s��
 * @param adc_pin		ADC����s��
 * @param max_degree	�����I���E�p�x
 * @param ccw_lpv		�����v���ł̃��[�^���쓮�ł���PWM�̍Œ�l
 * @param cw_lpv		���v���ł̃��[�^���쓮�ł���PWM�̍Œ�l
 *
 * @return 				-1�Ȃ�ݒ�Ɏ��s
 *
 * @Author				AOKI
 */
int DC_SERVO_StructInitialize(DC_SERVO *dc_servo,PIN_NUMBER pwm_pin,PIN_NUMBER adc_pin,int middle_degree,int max_degree,int ccw_lpv,int cw_lpv){
	dc_servo->pwm_pin = pwm_pin;
	dc_servo->adc_pin = adc_pin;
	dc_servo->middle_degree = middle_degree;
	dc_servo->max_degree = max_degree;
	dc_servo->ccw_lpv = ccw_lpv;
	dc_servo->cw_lpv = cw_lpv;
	dc_servo->diff[0] = 0;
	dc_servo->diff[1] = 0;
	dc_servo->integral = 0.0;
	dc_servo->p = 0;
	dc_servo->i = 0;
	dc_servo->d = 0;
	dc_servo->Hash = NULL;
	if(PinMode(dc_servo->pwm_pin,PWM) == -1){
		return -1;
	}
	if(PinMode(dc_servo->adc_pin,ADC) == -1){
		return -1;
	}
	return 0;
}

/**
 * DC���[�^���T�[�{�Ƃ��Ĉ������߂̔��W���H�̐ݒ�
 * DC_SERVO_StructInitialize()�ƕʂ����͈̂����������̂�������������
 * @param dc_servo		�Ώۂ̍\����
 * @param kp			KP
 * @param ki			KI
 * @param kd			KD
 * @return				���ɂȂ�
 *
 * @Author				AOKI
 */
int DC_SERVO_SetKP_KI_KD(DC_SERVO *dc_servo,float kp,float ki,float kd){
	dc_servo->kp = kp;
	dc_servo->ki = ki;
	dc_servo->kd = kd;
	return 0;
}

int DC_SERVO_RegisterHash(DC_SERVO *dc_servo, int (*Hash)(int degree)){
	dc_servo->Hash = Hash;
	return 0;
}

/**
 * DC���[�^���T�[�{�Ƃ��Ĉ������߂̊֐�
 * @param dc_servo			�Ώۂ̃��[�^�AADC�Z�b�g
 * @param target_degree		�ڕW�p�x
 *
 * @return 					���ɂȂ�
 *
 * @Author					AOKI
 *
 * ���i��y���������R�[�h���\�����������̂ł�
 * ����͂ł̃R�s�[�Ȃ̂ŊԈႦ�Ă���\��������
 */
int DC_SERVO_Change(DC_SERVO *dc_servo,int target_degree){
	int pid_ad,pid_duty;
	int sensor_value;
	int mid_advalue;
	int max_advalue;
	int min_advalue;
	int target_advalue;
	int nuetral_ad_span;
	
	if(dc_servo->Hash == NULL){
		PWM_Write(dc_servo->pwm_pin,500);
		return -1;
	}
	
	sensor_value = ADC_Read(dc_servo->adc_pin);
	mid_advalue = dc_servo->Hash(dc_servo->middle_degree);
	max_advalue = mid_advalue - dc_servo->Hash(dc_servo->max_degree + dc_servo->middle_degree);
	min_advalue = mid_advalue - dc_servo->Hash(dc_servo->middle_degree - dc_servo->max_degree);
	target_advalue = dc_servo->Hash(dc_servo->middle_degree + target_degree);
	nuetral_ad_span = (dc_servo->Hash(dc_servo->middle_degree + target_degree + 1) - dc_servo->Hash(dc_servo->middle_degree + target_degree - 1));
	nuetral_ad_span = nuetral_ad_span > 0 ? nuetral_ad_span : -nuetral_ad_span;
	nuetral_ad_span /=2;
	nuetral_ad_span = 100;


	dc_servo->diff[0] = dc_servo->diff[1];										//��O�̖ڕW�l�Ƃ̍����i�[

	dc_servo->diff[1] = sensor_value - target_advalue;	//����ʂ̌v�Z
	dc_servo->integral += (float)((dc_servo->diff[1] + dc_servo->diff[0]) / 2)*DC_SERVO_DT;		// DT �͌v�Z�����i�������ԁj�B�ϕ��v�Z
	if(dc_servo->diff[0] * dc_servo->diff[1] <= 0){
		DC_SERVO_ClearIntegral(dc_servo);
	}

	dc_servo->p = dc_servo->kp * dc_servo->diff[1];
	dc_servo->i = dc_servo->ki * dc_servo->integral;
	dc_servo->d = dc_servo->kd * ((dc_servo->diff[1] - dc_servo->diff[0]) / DC_SERVO_DT);			//�����v�Z

	pid_ad = (int)((dc_servo->p)+(dc_servo->i)+(dc_servo->d));						//�����炭AD�l�I�Ȃ��̂ł͂Ȃ�

	if(pid_ad  > max_advalue ){													//ad�l���S���ϒ�R�̒��S�ɂ���Ɖ���B�Z���T�̑g�݂��ɂ��ύX�̕K�v����
		pid_ad  = max_advalue;
	}
	if(pid_ad < min_advalue){																//pid���}max_advalue�͈͓̔���
		pid_ad = min_advalue;
	}
	if(pid_ad > nuetral_ad_span){
		pid_duty = (dc_servo->ccw_lpv) + pid_ad * (999 - (dc_servo->ccw_lpv)) / max_advalue;	//pwm 500�`999�Ő��] ok
		
	}else if(pid_ad < -nuetral_ad_span){
		pid_duty = dc_servo->cw_lpv  + (pid_ad * dc_servo->cw_lpv) / (-min_advalue);			//pwm 0�`499�ŋt�]
	}else{
		pid_duty = 500;
	}
	PWM_Write(dc_servo->pwm_pin,pid_duty);									
	return sensor_value;
}



int DC_SERVO_ClearIntegral(DC_SERVO *dc_servo){
	dc_servo->integral = 0;
	return 0;
}


static int TestHash(int degree){
	static int max = 76;
	static int hash[] = {2569,2553,2553,2540,2516,2516,2505,2497,2483,2470,2458,2432,2421,2420,2405,2392,2382,2365,2356,2332,2331,2313,2294,2295,2280,2265,2251,2243,2226,2214,2203,2191,2178,2163,2156,2138,2129,2117,2101,2089,2072,2057,2041,2030,2018,2005,1994,1980,1967,1955,1941,1929,1916,1899,1892,1880,1865,1856,1844,1828,1819,1804,1787,1776,1761,1747,1737,1722,1711,1696,1683,1663,1654,1641,1631,1616};
	degree = degree > 0 ? ( degree < max ? degree : max - 1) : 0;
	return hash[degree];
}

