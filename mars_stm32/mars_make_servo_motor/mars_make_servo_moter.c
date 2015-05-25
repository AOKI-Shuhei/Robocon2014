/*

 * mars_make_servo_moter.c
 *
 *  Created on: 2013/12/18
 *      Author: Shuhei
 */

#include "mars_make_servo_moter.h"

/**
 * DCモータをサーボとして扱うための構造体の初期化関数
 * @param dc_servo		対象の構造体
 * @param pwm_pin		PWM出力するピン
 * @param adc_pin		ADCするピン
 * @param max_degree	物理的限界角度
 * @param ccw_lpv		反時計回りでのモータが駆動できるPWMの最低値
 * @param cw_lpv		時計回りでのモータが駆動できるPWMの最低値
 *
 * @return 				-1なら設定に失敗
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
 * DCモータをサーボとして扱うための比例係数？の設定
 * DC_SERVO_StructInitialize()と別けたのは引数が長いのが嫌だったから
 * @param dc_servo		対象の構造体
 * @param kp			KP
 * @param ki			KI
 * @param kd			KD
 * @return				特になし
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
 * DCモータをサーボとして扱うための関数
 * @param dc_servo			対象のモータ、ADCセット
 * @param target_degree		目標角度
 *
 * @return 					特になし
 *
 * @Author					AOKI
 *
 * 藤司先輩が書いたコードを構造化したものです
 * 手入力でのコピーなので間違えている可能性もあり
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


	dc_servo->diff[0] = dc_servo->diff[1];										//一つ前の目標値との差を格納

	dc_servo->diff[1] = sensor_value - target_advalue;	//操作量の計算
	dc_servo->integral += (float)((dc_servo->diff[1] + dc_servo->diff[0]) / 2)*DC_SERVO_DT;		// DT は計算周期（微小時間）。積分計算
	if(dc_servo->diff[0] * dc_servo->diff[1] <= 0){
		DC_SERVO_ClearIntegral(dc_servo);
	}

	dc_servo->p = dc_servo->kp * dc_servo->diff[1];
	dc_servo->i = dc_servo->ki * dc_servo->integral;
	dc_servo->d = dc_servo->kd * ((dc_servo->diff[1] - dc_servo->diff[0]) / DC_SERVO_DT);			//微分計算

	pid_ad = (int)((dc_servo->p)+(dc_servo->i)+(dc_servo->d));						//おそらくAD値的なものではない

	if(pid_ad  > max_advalue ){													//ad値中心が可変抵抗の中心にあると仮定。センサの組みつけにより変更の必要あり
		pid_ad  = max_advalue;
	}
	if(pid_ad < min_advalue){																//pidを±max_advalueの範囲内に
		pid_ad = min_advalue;
	}
	if(pid_ad > nuetral_ad_span){
		pid_duty = (dc_servo->ccw_lpv) + pid_ad * (999 - (dc_servo->ccw_lpv)) / max_advalue;	//pwm 500～999で正転 ok
		
	}else if(pid_ad < -nuetral_ad_span){
		pid_duty = dc_servo->cw_lpv  + (pid_ad * dc_servo->cw_lpv) / (-min_advalue);			//pwm 0～499で逆転
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

