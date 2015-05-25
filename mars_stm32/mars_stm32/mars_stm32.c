#include "mars_stm32.h"

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * 諸々の初期設定を行う関数。ローカル変数の宣言が終わったら最初に一度呼んでください。
 * 現状、使いそうなモジュールは全て起動しているので省電力化などはやっていないです。
 * この関数を変更する場合は変更点と変更者名をここのコメントに残してください
 *
 * @Author			AOKI
 *
 * USART割込みの優先度を設定？	AOKI
 */
void RCC_Configuration(void)
{
	NVIC_InitTypeDef nvic;
	//よくわからないけどとりあえず書いてる
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div1);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	RCC_ADCCLKConfig(RCC_PCLK2_Div2);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 |	//TIM2起動
						RCC_APB1Periph_TIM3 |		//TIM3起動
						RCC_APB1Periph_TIM4 |		//TIM4起動
						RCC_APB1Periph_USART2 |		//USART2起動
						RCC_APB1Periph_USART3,		//USART3起動
						ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |	//ADC起動
						RCC_APB2Periph_GPIOA |		//GPIOA起動
						RCC_APB2Periph_GPIOB |		//GPIOB起動
						RCC_APB2Periph_GPIOC |		//GPIOC起動
						RCC_APB2Periph_GPIOD |		//GPIOD起動
						RCC_APB2Periph_AFIO |		//Alternate Function起動
						RCC_APB2Periph_USART1 |		//USART1起動
						RCC_APB2Periph_TIM1,		//TIM1起動
						ENABLE);
	//Systickタイマの割込みの間隔を設定
	if(SysTick_Config(SystemCoreClock / SYSTICK_INTERRUPT_TIMING)){
		while(1);
	}
	//割込みの優先度がなんちゃら。書いてみただけ
	nvic.NVIC_IRQChannel = USART1_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
	nvic.NVIC_IRQChannel = USART2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 2;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
	nvic.NVIC_IRQChannel = USART3_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 3;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
	InitStopWatch();
}

/**
 * ピン設定を行う関数
 * ここで設定できるのはGPIO、PWM、ADCに関して
 * @param pin_number    ピン番号
 * @param mode          ピン設定
 * @return              -1なら設定に失敗
 *
 * @Author              AOKI
 *
 * 動作確認済み
 */
int PinMode(PIN_NUMBER pin_number, PIN_MODE mode){
	switch(mode){
	case IO_INPUT_PD:
	case IO_INPUT_PU:
	case IO_OUTPUT:
		return GPIO_PinMode(pin_number,mode);
	case ADC:
		return ADC_PinMode(pin_number);
	case PWM:
		return PWM_PinMode(pin_number);
	case SERVO:
		return SERVO_PWM_PinMode(pin_number);
	default:
		return -1;
	}
}

/**
 * 指定したピンからvalueの出力を行う
 * PinModeでOutPutに指定したピンで行う
 * @param pin_number		ピン番号
 * @param value				出力
 * @return					-1なら設定失敗
 *
 * @Author					AOKI
 *
 * PC14とPC15とPD0とPD1は使用禁止(発振器がついてる）
 * Remapの確認はしてないがそれ以外は動作する
 */
int IO_Write(PIN_NUMBER pin_number,IO_VALUE value){
	GPIO_TypeDef *GPIOx;
	uint16_t pin;
	if(DecodePin(pin_number,&GPIOx,&pin) == -1){
		return -1;
	}
	switch(value){
	case HIGH:
		GPIO_SetBits(GPIOx,pin);
		break;
	case LOW:
		GPIO_ResetBits(GPIOx,pin);
		break;
	default:
		return -1;
	}
	return 0;
}

/**
 * 指定したピンの状態を得る
 * PinModeでINPUTを指定したピンで行う
 * @param pin_number			ピン番号
 * @return 						ピンの状態。HIGHなら1、LOWなら0。-1なら取得に失敗
 *
 * @Author						AOKI
 *
 * PC14とPC15とPD0とPD1は使用禁止(発振器がついてる）
 * Remapの確認はしてないがそれ以外は動作する
 */
int IO_Read(PIN_NUMBER pin_number){
	GPIO_TypeDef *GPIOx;
	uint16_t pin;
	if(DecodePin(pin_number,&GPIOx,&pin) == -1){
		return -1;
	}
	return GPIO_ReadInputDataBit(GPIOx,pin) != 0 ? 1 : 0;
}

/**
 * 	指定したピンでADCを行う関数
 * 	@param pin_number		ピン番号
 * 	@return 				-1ならADC失敗（ピン番号が違う）それ以外ならADC値
 *
 * 	@Author					AOKI
 *
 * 	全て動作確認済み。ただしADC16は内部の温度計に接続されているので考慮していない
 */
int ADC_Read(PIN_NUMBER pin_number){
	uint8_t channel;
	if(ADC_Decode(pin_number,&channel) == -1){
		return -1;
	}
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_13Cycles5);
	// Start the conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	// Wait until conversion completion
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	// Get the conversion value
	return ADC_GetConversionValue(ADC1);
}

/**
 * 指定したピンからPWM出力を行う関数
 * @param pin_number		ピン番号
 * @param value				デューティー比	0-999
 * @return 					-1はPWMのピンがおかしい　0は正常終了　1は値のpulse変更なしでの終了
 *
 * @Author					AOKI
 *
 * Remapのチェックはまだ
 * TIM2のch3以外は動作確認済。TIM2のch3が動作しない原因はわからないので保留
 */
int PWM_Write(PIN_NUMBER pin_number,int value){
	TIM_OCInitTypeDef OC_Init;
	void (*channelsInit[])(TIM_TypeDef*,TIM_OCInitTypeDef*) = {TIM_OC1Init,TIM_OC2Init,TIM_OC3Init,TIM_OC4Init};
	uint16_t (*TIM_Pulse_Capture[])(TIM_TypeDef*) = {TIM_GetCapture1,TIM_GetCapture2,TIM_GetCapture3,TIM_GetCapture4};
	int period = ((SystemCoreClock/PWM_PRESCALAR) / PWM_CLOCK) - 1;
	TIM_TypeDef *TIMx;
	uint16_t channel;
	uint16_t capture_pulse;
	if(PWM_Decode(pin_number,&TIMx,&channel) == -1){
		return -1;
	}
	if(3 < channel){
		return -1;
	}
	capture_pulse = TIM_Pulse_Capture[channel](TIMx);
	value = value > (PWM_DUTY -1) ? (PWM_DUTY -1) : (value < 0 ? 1 : value);
	value = (value*period)/(PWM_DUTY-1);
	if(value != capture_pulse){
		TIM_OCStructInit(&OC_Init);
		OC_Init.TIM_OCMode = TIM_OCMode_PWM1;
		OC_Init.TIM_OutputState = TIM_OutputState_Enable;
		OC_Init.TIM_OCPolarity = TIM_OCPolarity_High;
		OC_Init.TIM_Pulse = value;
		channelsInit[channel](TIMx,&OC_Init);
		return 0;
	}
	return 1;
}

/**
 * 指定したピンからサーボ用のPWM出力を行う関数
 * @param pin_number		ピン番号
 * @param degree			角度
 * @return 					-1はPWMのピンがおかしい　0は正常終了　1は値のpulse変更なしでの正常終了
 *
 * @Author					AOKI
 *
 * Remapのチェックはまだ
 * TIM2のch3以外は動作確認済。TIM2のch3が動作しない原因はわからないので保留
 *
 * 現在角度範囲-45~45。-90~90にするにはdegree*=2を削除
 */
int SERVO_Write(PIN_NUMBER pin_number,int degree){
	TIM_OCInitTypeDef OC_Init;
	void (*channelsInit[])(TIM_TypeDef*,TIM_OCInitTypeDef*) = {TIM_OC1Init,TIM_OC2Init,TIM_OC3Init,TIM_OC4Init};
	uint16_t (*TIM_Pulse_Capture[])(TIM_TypeDef*) = {TIM_GetCapture1,TIM_GetCapture2,TIM_GetCapture3,TIM_GetCapture4};
	TIM_TypeDef *TIMx;
	uint16_t channel;
	uint16_t capture_pulse;
	int value = 900;
	//degree → duty比変換
	//今はduty比 = degreeにする
	//degree*=2;
	degree += 90;
	degree = ConstrainInt((int)(degree*6.666667),0,1200);
	value += degree;
	if(PWM_Decode(pin_number,&TIMx,&channel) == -1){
		return -1;
	}
	if(3 < channel){
		return -1;
	}
	capture_pulse = TIM_Pulse_Capture[channel](TIMx);
	if(value != capture_pulse){
		TIM_OCStructInit(&OC_Init);
		OC_Init.TIM_OCMode = TIM_OCMode_PWM1;
		OC_Init.TIM_OutputState = TIM_OutputState_Enable;
		OC_Init.TIM_OCPolarity = TIM_OCPolarity_High;
		OC_Init.TIM_Pulse = value;
		channelsInit[channel](TIMx,&OC_Init);
		return 0;
	}
	return 1;
}

/**
 * GPIOの設定を行う関数
 * PinModeから呼ばれる
 * @param pin_number    ピン番号
 * @param mode          ピン設定
 * @return              -1なら設定に失敗
 *
 * @Author              AOKI
 *
 * PC14とPC15とPD0とPD1は使用禁止(発振器がついてる）
 * Remapの確認はしてないがそれ以外は動作する
 */
int GPIO_PinMode(PIN_NUMBER pin_number,PIN_MODE mode){
	GPIO_TypeDef *GPIOx;
	uint16_t pin;
	GPIO_InitTypeDef IO_Init;
	switch(mode){
	case IO_INPUT_PD:
		IO_Init.GPIO_Mode = GPIO_Mode_IPD;
		break;
	case IO_INPUT_PU:
		IO_Init.GPIO_Mode = GPIO_Mode_IPU;
		break;
	case IO_OUTPUT:
		IO_Init.GPIO_Mode = GPIO_Mode_Out_PP;
		break;
	default:
		return -1;
	}
	DecodePin(pin_number,&GPIOx,&pin);
	IO_Init.GPIO_Pin = pin;
	IO_Init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&IO_Init);
	return 0;
}

/**
 * ADCの設定を行う関数
 * PinModeから呼ばれる
 * @param pin_number	ピン番号
 * @return 				-1なら設定に失敗
 *
 * @Author				AOKI
 *
 * 動作確認済
 */
int ADC_PinMode(PIN_NUMBER pin_number){
	GPIO_TypeDef *GPIOx;
	uint16_t pin;
	GPIO_InitTypeDef IO_Init;
	ADC_InitTypeDef ADC_InitStructure;
	uint8_t channel;
	if(ADC_Decode(pin_number,&channel) == -1){
		return -1;
	}
	if(DecodePin(pin_number,&GPIOx, &pin) == -1){
		return -1;
	}
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_Init(ADC1,&ADC_InitStructure);
	IO_Init.GPIO_Pin = pin;
	IO_Init.GPIO_Mode = GPIO_Mode_AIN;
	IO_Init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&IO_Init);
	ADC_Cmd(ADC1,ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	return 0;
}

/**
 * PWMの設定を行う関数
 * PinModeから呼ばれる
 * @param				ピン番号
 * @return 				-1なら設定に失敗
 *
 * @Author				AOKI
 *
 * Remapのチェックはまだ
 * TIM2のch3以外は動作確認済。TIM2のch3が動作しない原因はわからないので保留
 */
int PWM_PinMode(PIN_NUMBER pin_number){
	void (*channelsConfig[])(TIM_TypeDef*,uint16_t) = {TIM_OC1PreloadConfig,TIM_OC2PreloadConfig,TIM_OC3PreloadConfig,TIM_OC4PreloadConfig};
	GPIO_InitTypeDef IO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_Inittructure;
	TIM_TypeDef *TIMx;
	uint16_t channel;
	GPIO_TypeDef *GPIOx;
	uint16_t pin;
	if(PWM_Decode(pin_number,&TIMx,&channel) == -1){
		return -1;
	}
	if(DecodePin(pin_number,&GPIOx,&pin) == -1){
		return -1;
	}
	IO_InitStructure.GPIO_Pin = pin;
	IO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	IO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&IO_InitStructure);
	TIM_TimeBaseStructInit(&TIM_Inittructure);
	TIM_Inittructure.TIM_Prescaler = PWM_PRESCALAR - 1;
	TIM_Inittructure.TIM_Period = ((SystemCoreClock/PWM_PRESCALAR) / PWM_CLOCK) - 1;
	TIM_TimeBaseInit(TIMx,&TIM_Inittructure);
	PWM_Write(pin_number,0);
	if(TIMx == TIM1){
		TIM_CtrlPWMOutputs(TIMx,ENABLE);
	}
	channelsConfig[channel](TIMx,TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIMx,ENABLE);
	TIM_Cmd(TIMx,ENABLE);
	return 0;
}

/**
 * サーボ用のPWMの設定を行う関数
 * 普通にPWMを出力する場合は20kHzだがここでは50Hzとなる
 * 同一タイマからは同じ周波数しか出ないので注意
 * PinModeから呼ばれる
 * @param				ピン番号
 * @return 				-1なら設定に失敗
 *
 * @Author				AOKI
 *
 * Remapのチェックはまだ
 * TIM2のch3以外は動作確認済。TIM2のch3が動作しない原因はわからないので保留
 * 周波数を弄っただけなので大丈夫なはず
 */
int SERVO_PWM_PinMode(PIN_NUMBER pin_number){
	void (*channelsConfig[])(TIM_TypeDef*,uint16_t) = {TIM_OC1PreloadConfig,TIM_OC2PreloadConfig,TIM_OC3PreloadConfig,TIM_OC4PreloadConfig};
	GPIO_InitTypeDef IO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_Inittructure;
	TIM_TypeDef *TIMx;
	uint16_t channel;
	GPIO_TypeDef *GPIOx;
	uint16_t pin;
	if(PWM_Decode(pin_number,&TIMx,&channel) == -1){
		return -1;
	}
	if(DecodePin(pin_number,&GPIOx,&pin) == -1){
		return -1;
	}
	IO_InitStructure.GPIO_Pin = pin;
	IO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	IO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&IO_InitStructure);
	TIM_TimeBaseStructInit(&TIM_Inittructure);
	TIM_Inittructure.TIM_Prescaler = SERVO_PWM_PRESCALAR - 1;
	TIM_Inittructure.TIM_Period = ((SystemCoreClock/SERVO_PWM_PRESCALAR) / SERVO_PWM_CLOCK) - 1;
	TIM_TimeBaseInit(TIMx,&TIM_Inittructure);
	SERVO_Write(pin_number,0);
	if(TIMx == TIM1){
		TIM_CtrlPWMOutputs(TIMx,ENABLE);
	}
	channelsConfig[channel](TIMx,TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIMx,ENABLE);
	TIM_Cmd(TIMx,ENABLE);
	return 0;
}

/**
 * ピン番号からポートとピンを抽出する関数
 * @param pin_number    ピン番号
 * @param port          ポート
 * @param pin           ピン数値
 * @return              -1なら設定に失敗
 *
 * @Author              AOKI
 *
 * 動作確認済
 */
int DecodePin(PIN_NUMBER pin_number,GPIO_TypeDef **port, uint16_t* pin){
	switch(pin_number & GETPORT){
	case PIN_NUMBER_PORTA:
		*port = GPIOA;
		break;
	case PIN_NUMBER_PORTB:
		*port = GPIOB;
		break;
	case PIN_NUMBER_PORTC:
		*port = GPIOC;
		break;
	case PIN_NUMBER_PORTD:
		*port = GPIOD;
		break;
	default:
		return -1;
	}
	*pin = pin_number & GETPIN;
	return 0;
}

/**
 * ピン番号からタイマとチャネルを抽出する関数
 * @param			ピン番号
 * @TIMx			タイマ
 * @channel			チャネル
 * @return			-1なら抽出失敗
 *
 * @Author			AOKI
 *
 * remap以外動作確認済
 */
int PWM_Decode(PIN_NUMBER pin_number,TIM_TypeDef **TIMx,uint16_t *channel){
	(*channel) = 4;
	switch(pin_number){
	case PA8:
		(*channel)--;
	case PA9:
		(*channel)--;
	case PA10:
		(*channel)--;
	case PA11:
		(*channel)--;
		*TIMx = TIM1;
		break;
	case PA0:
	case PA15:
		(*channel)--;
	case PA1:
	case PB3:
		(*channel)--;
	case PA2:
	case PB10:
		(*channel)--;
	case PA3:
	case PB11:
		(*channel)--;
		*TIMx = TIM2;
		break;
	case PA6:
	case PC6:
	case PB4:
		(*channel)--;
	case PA7:
	case PC7:
	case PB5:
		(*channel)--;
	case PB0:
	case PC8:
		(*channel)--;
	case PB1:
	case PC9:
		(*channel)--;
		*TIMx = TIM3;
		break;
	case PB6:
		(*channel)--;
	case PB7:
		(*channel)--;
	case PB8:
		(*channel)--;
	case PB9:
		(*channel)--;
		*TIMx = TIM4;
		break;
	default:
		return -1;
	}
	return 0;
}

/**
 * ピン番号から対応するADCチャンネルをし取得する関数
 * @param pin_number			ピン番号
 * @param channel				チャネル
 * @return 						-1なら取得失敗
 *
 * @param						AOKI
 *
 * 動作確認済
 */
int ADC_Decode(PIN_NUMBER pin_number,uint8_t* channel){
	switch(pin_number){
	case PA0:
		(*channel) = ADC_Channel_0;
		break;
	case PA1:
		(*channel) = ADC_Channel_1;
		break;
	case PA2:
		(*channel) = ADC_Channel_2;
		break;
	case PA3:
		(*channel) = ADC_Channel_3;
		break;
	case PA4:
		(*channel) = ADC_Channel_4;
		break;
	case PA5:
		(*channel) = ADC_Channel_5;
		break;
	case PA6:
		(*channel) = ADC_Channel_6;
		break;
	case PA7:
		(*channel) = ADC_Channel_7;
		break;
	case PB0:
		(*channel) = ADC_Channel_8;
		break;
	case PB1:
		(*channel) = ADC_Channel_9;
		break;
	case PC0:
		(*channel) = ADC_Channel_10;
		break;
	case PC1:
		(*channel) = ADC_Channel_11;
		break;
	case PC2:
		(*channel) = ADC_Channel_12;
		break;
	case PC3:
		(*channel) = ADC_Channel_13;
		break;
	case PC4:
		(*channel) = ADC_Channel_14;
		break;
	case PC5:
		(*channel) = ADC_Channel_15;
		break;
	default:
		return -1;
	}
	return 0;
}

/**
 * USARTの設定を行う関数
 * @param USARTx					USARTの番号。USART1,USART2,USART3から選択
 * @oaram mode						通信モード。SEND,READ,SEND_AND_READから選択（SEND_AND_READは(SEND | READ)と同じ）
 * @param remap						リマップを行うかどうか。ENABLE/DISABLE。
 * @return							-1なら設定失敗
 *
 * @Author							AOKI
 *
 * remap以外動作確認済み
 */
int USART_Mode(USART_TypeDef* USARTx, USART_MODE mode, FunctionalState remap){
	PIN_NUMBER pin_tx,pin_rx;
	GPIO_InitTypeDef IO_Init;
	GPIO_TypeDef *GPIOx;
	uint16_t pin;
	USART_LING_BUFFER *u_l_buffer;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
		if(remap != ENABLE){
			pin_tx = PA9;
			pin_rx = PA10;
			GPIO_PinRemapConfig(GPIO_Remap_USART1,DISABLE);
		}else{
			pin_tx = PB6;
			pin_rx = PB7;
			GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
		}
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
		if(remap != ENABLE){
			pin_tx = PA2;
			pin_rx = PA3;
			GPIO_PinRemapConfig(GPIO_Remap_USART2,DISABLE);
		}else{
			return -1;
		}
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
		if(remap != ENABLE){
			pin_tx = PB10;
			pin_rx = PB11;
			GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,DISABLE);
		}else{
			pin_tx = PC10;
			pin_rx = PC11;
			GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);
		}
	}else{
		return -1;
	}
	//tx
	if(mode & SEND){
		if(DecodePin(pin_tx,&GPIOx,&pin) == -1){
			return -1;
		}
		IO_Init.GPIO_Pin = pin;
		IO_Init.GPIO_Speed = GPIO_Speed_50MHz;
		IO_Init.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOx,&IO_Init);
		LingBuffer_Initialize(&(u_l_buffer->tx_buffer));
	}
	//rx
	if(mode & READ){
		if(DecodePin(pin_rx,&GPIOx,&pin) == -1){
			return -1;
		}
		IO_Init.GPIO_Pin = pin;
		IO_Init.GPIO_Speed = GPIO_Speed_50MHz;
		IO_Init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOx,&IO_Init);
		LingBuffer_Initialize(&(u_l_buffer->rx_buffer));
	}
	return 0;
}

/**
 * USARTを開始する関数
 * @param USARTx					USARTの番号。USART1,USART2,USART3から選択
 * @param baudrate					ボーレート
 * @return							特になし
 *
 * @Author							AOKI
 *
 * remap以外動作確認済み
 */
int USART_Begin(USART_TypeDef* USARTx, int baudrate){
	USART_InitTypeDef U_Init;
	U_Init.USART_BaudRate = baudrate;
	U_Init.USART_WordLength = USART_WordLength_8b;
	U_Init.USART_StopBits = USART_StopBits_1;
	U_Init.USART_Parity = USART_Parity_No;
	U_Init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	U_Init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USARTx,&U_Init);
	USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
	USART_Cmd(USARTx,ENABLE);
	return 0;
}
						    
int USART_Close(USART_TypeDef* USARTx){
    USART_ITConfig(USARTx,USART_IT_RXNE,DISABLE);
    USART_Cmd(USARTx,DISABLE);
    USART_ClearRxBuffer(USARTx);
    USART_ClearTxBuffer(USARTx);
    return 0;
}
						    

int USART_ParityBegin(USART_TypeDef* USARTx, int baudrate){
	USART_InitTypeDef U_Init;
	U_Init.USART_BaudRate = baudrate;
	U_Init.USART_WordLength = USART_WordLength_9b;
	U_Init.USART_StopBits = USART_StopBits_1;
	U_Init.USART_Parity = USART_Parity_Even;
	U_Init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	U_Init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USARTx,&U_Init);
	USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
	USART_Cmd(USARTx,ENABLE);
	return 0;
}

/**
 * USARTで1byte送信する関数
 * @param USARTx				USARTの番号
 * @param tx_data				送信データ
 * @return 						-1なら送信バッファが一杯
 * 								-2なら送信設定にされていないかも
 *
 * @Author						AOKI
 */
int USART_Putc(USART_TypeDef* USARTx,char tx_data){
	USART_LING_BUFFER *u_l_buffer;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	if(u_l_buffer->tx_buffer.enable == 0){
		return -2;
	}
	if((LING_BUFFER_SIZE - u_l_buffer->tx_buffer.size) < 1){
		return -1;
	}
	USART_ITConfig(USARTx,USART_IT_TXE,DISABLE);
	LingBuffer_In(&(u_l_buffer->tx_buffer),tx_data);
	USART_ITConfig(USARTx,USART_IT_TXE,ENABLE);
	return 0;
}

/**
 * USARTで1byte受信する関数
 * @param USARTx				USARTの番号
 * @retuern						受信データ。どう足掻いても0ならどこかで失敗が発生している可能性があります。
 *
 * @Author						AOKI
 */
char USART_Getc(USART_TypeDef* USARTx){
	USART_LING_BUFFER *u_l_buffer;
	char rx_data;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return 0;
	}
	if(u_l_buffer->rx_buffer.enable != 1){
		return 0;
	}
	if(u_l_buffer->rx_buffer.size == 0){
		return 0;
	}
	USART_ITConfig(USARTx,USART_IT_RXNE,DISABLE);
	LingBuffer_Out(&(u_l_buffer->rx_buffer),&rx_data);
	USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
	return rx_data;
}

/**
 * USARTで複数byte送信する関数。送信フォーマットは特に指定なし
 * @param USARTx					USARTの番号。USART1,USART2,USART3から選択
 * @param tx_data					送信データ列
 * @param length					送信データ幅
 * @return 							-1なら送信バッファがオーバフロー
 * 									-2なら多分送信に設定されてない
 * @Author							AOKI
 */
int USART_Write(USART_TypeDef* USARTx, char *tx_data, short length){
	USART_LING_BUFFER *u_l_buffer;
	int i;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	if(u_l_buffer->tx_buffer.enable == 0){
		return -2;
	}
	if((LING_BUFFER_SIZE - u_l_buffer->tx_buffer.size) <length){
		return -1;
	}
	USART_ITConfig(USARTx,USART_IT_TXE,DISABLE);
	for(i=0;i<length;i++){
		LingBuffer_In(&(u_l_buffer->tx_buffer),tx_data[i]);
	}
	USART_ITConfig(USARTx,USART_IT_TXE,ENABLE);
	return 0;
}

/**
 * USARTで複数byte送信する関数。文字列を想定していて終端に終端文字'\0'がないと大変なことになります。
 * 送信データには'\0'は含む
 * @param USARTx				USARTの番号
 * @param data					送信データ
 *
 * @return 						-1なら送信バッファオーバフロー
 * 								-2なら多分送信設定されていない
 *
 * @Author						AOKI
 */
int USART_Print(USART_TypeDef* USARTx,char *data){
	int sum=0;
	for(;data[sum]!= '\0';sum++);
	sum++;
	return USART_Write(USARTx,data,sum);
}

/**
 * USARTで受信する関数
 * @param USARTx					USARTの番号。USART1,USART2,USART3から選択
 * @param rx_data					受信データを入れる配列
 * @param length					想定する受信データ幅
 * @return 							-1はUSARTxに渡す引数がおかしい
 * 									-2なら多分受信が設定されてない
 * 									1なら受信バッファにlengthたまっていない
 *
 * @Author							AOKI
 */
int USART_Read(USART_TypeDef* USARTx, char* rx_data, short length){
	USART_LING_BUFFER *u_l_buffer;
	int i;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	if(u_l_buffer->rx_buffer.enable != 1){
		return -2;
	}
	if(u_l_buffer->rx_buffer.size < length){
		return 1;
	}
	USART_ITConfig(USARTx,USART_IT_RXNE,DISABLE);
	for(i=0;i<length;i++){
		LingBuffer_Out(&(u_l_buffer->rx_buffer),(rx_data+i));
	}
	USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
	return 0;
}

int USART_ReadSplit(USART_TypeDef* USARTx, char split, char* data, short length){
	int count;
	USART_LING_BUFFER *u_l_buffer;
	char temp;
	int i;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	if(u_l_buffer->rx_buffer.enable != 1){
		return -2;
	}



	USART_ITConfig(USARTx,USART_IT_RXNE,DISABLE);
	count = LingBuffer_Contains(&(u_l_buffer->rx_buffer),split);
	if(count != -1){
		do{
			LingBuffer_Out(&(u_l_buffer->rx_buffer),&temp);
			count--;
		}while(temp == 0);
		i=0;
		count++;
		if(temp == split || count+1 > length){
			return 0;
		}
		do{
			*(data+i) = temp;
			LingBuffer_Out(&(u_l_buffer->rx_buffer),&temp);
			i++;
		}while(temp != split);
		*(data + i) = 0;
		return count + 1;
	}
	USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
	return 0;

}


/*
int USART_ReadSplit(USART_TypeDef* USARTx, char split, char* data, short length){
	int count;
	USART_LING_BUFFER *u_l_buffer;
	char temp;
	int i;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	if(u_l_buffer->rx_buffer.enable != 1){
		return -2;
	}
	
	

	USART_ITConfig(USARTx,USART_IT_RXNE,DISABLE);
	count = LingBuffer_Contains(&(u_l_buffer->rx_buffer),split);
	if(count != -1 && count+1 < length){
		for(i=0;i<count;i++){
			LingBuffer_Out(&(u_l_buffer->rx_buffer),(data+i));
		}
		LingBuffer_Out(&(u_l_buffer->rx_buffer),&temp);
		USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
		*(data+i) = 0;
		return count+1;
	}
	USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
	return 0;
							    
}
*/
/**
 * 受信バッファのサイズを取得する関数
 * @param USARTx					USARTの番号
 * @return							バッファサイズ。-1なら引数がおかしい
 *
 * @Author							AOKI
 */
int USART_GetRxBufferSize(USART_TypeDef* USARTx){
	USART_LING_BUFFER *u_l_buffer;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	return u_l_buffer->rx_buffer.size;
} 

int USART_GetTxBufferSize(USART_TypeDef* USARTx){
	USART_LING_BUFFER *u_l_buffer;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	return u_l_buffer->tx_buffer.size;
} 


/**
 * 受信バッファを消す関数
 * @param USARTx					USARTの番号
 * @return 							-1なら引数がおかしい
 *
 * @Author							AOKI
 *
 * まだ試してないです
 */
int USART_ClearRxBuffer(USART_TypeDef* USARTx){
	USART_LING_BUFFER *u_l_buffer;
	char temp;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	USART_ITConfig(USARTx,USART_IT_RXNE,DISABLE);
	for(;LingBuffer_Out(&(u_l_buffer->rx_buffer),&temp) != -1;);
	USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
	return 0;
}

/**
 * 送信バッファを消す関数
 * @param USARTx					USARTの番号
 * @return 							-1なら引数がおかしい
 *
 * @Author							AOKI
 *
 * まだ試してないです
 */
int USART_ClearTxBuffer(USART_TypeDef* USARTx){
	USART_LING_BUFFER *u_l_buffer;
	char temp;
	if(USARTx == USART1){
		u_l_buffer = &USART1_buffer;
	}else if(USARTx == USART2){
		u_l_buffer = &USART2_buffer;
	}else if(USARTx == USART3){
		u_l_buffer = &USART3_buffer;
	}else{
		return -1;
	}
	USART_ITConfig(USARTx,USART_IT_TXE,DISABLE);
	for(;LingBuffer_Out(&(u_l_buffer->tx_buffer),&temp) != -1;);
	USART_ITConfig(USARTx,USART_IT_TXE,ENABLE);
	return 0;
}

/**
 *　入力された時間処理を止める関数
 *　ただし割込みは実行される
 *
 *　@param msec			処理を止める時間．ミリ秒
 *
 *　@Author				AOKI
 *
 * 動作確認済み
 */
void Delay(int msec){
	Timing_delay = msec > 0 ? msec : 0;
	for(;Timing_delay > 0;);
}

/**
 * プログラムが始動してからの経過時間を返す関数
 * 単位はミリ秒
 * 596時間ぐらいでオーバフローするので注意
 *
 * @return				プログラム始動からの経過時間（ミリ秒）
 *
 * @Author				AOKI
 */
int Millis(){
	return Now_milli_second;
}

/**
 * リングバッファに1byte入れる関数
 * @param l_buffer			対象のリングバッファ
 * @param data				データ
 * @return					-1ならオーバフロー
 *
 * @Author					AOKI
 */
int LingBuffer_In(LING_BUFFER *l_buffer,char data){
	if(l_buffer->size >= LING_BUFFER_SIZE){
		return -1;
	}
	l_buffer->buffer[l_buffer->end] = data;
	l_buffer->size++;
	l_buffer->end = (l_buffer->end+1)%LING_BUFFER_SIZE;
	return 0;
}

/**
 * リングバッファから1byte取り出す関数
 * @param l_buffer			対象のバッファリング
 * @param data				データを入れる変数
 * @return 					-1ならデータなし、或はバッファが有効ではない
 *
 * @Author					AOKI
 */
int LingBuffer_Out(LING_BUFFER *l_buffer,char *data){
	if(l_buffer->size == 0){
		return -1;
	}
	*data = l_buffer->buffer[l_buffer->start];
	l_buffer->size--;
	l_buffer->start = (l_buffer->start+1)%LING_BUFFER_SIZE;
	return 0;
}
						    
int LingBuffer_Contains(LING_BUFFER *l_buffer, char data){
	int i;
	int count = 0;
	if(l_buffer->size == 0){
		return -1;
	}
	for(i=l_buffer->start;i != l_buffer->end;i = (i+1)%LING_BUFFER_SIZE,count++){
		if(data == l_buffer->buffer[i]){
			return count;
		}
	}
	
	return -1;
}

/**
 * リングバッファ構造体の初期化関数
 * @param l_buffer				対象のリングバッファ
 *
 * @Author						AOKI
 */
void LingBuffer_Initialize(LING_BUFFER *l_buffuer){
	int i;
	for(i=0;i<LING_BUFFER_SIZE;i++){
		l_buffuer->buffer[i] = 0;
	}
	l_buffuer->size = 0;
	l_buffuer->start = 0;
	l_buffuer->enable = 1;
	l_buffuer->end = 0;
}

/**
 * 割込みハンドラ。気にしなくていいです
 * SysTick_Handlerがstm32f10x_it.oとなんちゃらってコンパイラに怒られたらstm32f10x_it.cの同名の関数を削除してください
 *
 * @Author				AOKI
 */
void SysTick_Handler(void){
	if(Timing_delay > 0){
		Timing_delay--;
	}
	Now_milli_second++;
}

/**
 * USART1の割込みハンドラ。気にしなくていいです。以下同じ感じの関数はゝ
 * コンパイラに怒られてらメッセージを読んで対処してください。
 *
 *　@Author						AOKI
 * @やってること
 * 何か受信したらこの関数が割込みで呼ばれます。その時に受信バッファリングに受信データを入れます
 * 送信可能になっててもこの関数が呼ばれます。送信バッファリングから1byte取り出して送信します。送信バッファリングのサイズが0なら送信の割込みはかかりません
 */
void USART1_IRQHandler(void){
	char temp;
	//Receive
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET){
		LingBuffer_In(&(USART1_buffer.rx_buffer),(char)USART_ReceiveData(USART1));
		return;
	}
	//Send
	if(USART_GetITStatus(USART1,USART_IT_TXE) != RESET){
		if(USART1_buffer.tx_buffer.size > 0){
			LingBuffer_Out(&(USART1_buffer.tx_buffer),&temp);
			USART_SendData(USART1,temp);
		}else{
			USART_ITConfig(USART1,USART_IT_TXE,DISABLE);
		}
		return;
	}
	//Interrupts which is not permitted raised!
	//This happens when so many packetes are received.
	//Something is wrong... reset USART module.
	USART_Cmd(USART1, DISABLE);
	USART_Cmd(USART1, ENABLE);
}

void USART2_IRQHandler(void){
	char temp;
	//Receive
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET){
		LingBuffer_In(&(USART2_buffer.rx_buffer),(char)USART_ReceiveData(USART2));
		return;
	}
	//Send
	if(USART_GetITStatus(USART2,USART_IT_TXE) != RESET){
		if(USART2_buffer.tx_buffer.size > 0){
			LingBuffer_Out(&(USART2_buffer.tx_buffer),&temp);
			USART_SendData(USART2,temp);
		}else{
			USART_ITConfig(USART2,USART_IT_TXE,DISABLE);
		}
		return;
	}
	//Interrupts which is not permitted raised!
	//This happens when so many packetes are received.
	//Something is wrong... reset USART module.
	USART_Cmd(USART2, DISABLE);
	USART_Cmd(USART2, ENABLE);
}

void USART3_IRQHandler(void){
	char temp;
	//Receive
	if(USART_GetITStatus(USART3,USART_IT_RXNE) != RESET){
		LingBuffer_In(&(USART3_buffer.rx_buffer),(char)USART_ReceiveData(USART3));
		return;
	}
	//Send
	if(USART_GetITStatus(USART3,USART_IT_TXE) != RESET){
		if(USART3_buffer.tx_buffer.size > 0){
			LingBuffer_Out(&(USART3_buffer.tx_buffer),&temp);
			USART_SendData(USART3,temp);
		}else{
			USART_ITConfig(USART3,USART_IT_TXE,DISABLE);
		}
		return;
	}
	//Interrupts which is not permitted raised!
	//This happens when so many packetes are received.
	//Something is wrong... reset USART module.
	USART_Cmd(USART3, DISABLE);
	USART_Cmd(USART3, ENABLE);
}

void InitStopWatch(){
  int i;
  for(i=0;i<STOPWATCH_NUM;i++){
    stopWatchMillis[i] = 0;
    stopWatchStopMillis[i] = 0;
    stopWatchIsRunning[i] = 0;
    stopWatchResetFlag[i] = 1;
  }
}

int StartStopWatch(int number){
  if(ConstrainInt(number,0,STOPWATCH_NUM-1) != number){
    return -1;
  }
  if(!IsRunningStopWatch(number)){
    stopWatchMillis[number] = stopWatchResetFlag[number] != 0 ? Millis() : Millis() - (stopWatchStopMillis[number] - stopWatchMillis[number]);
  }
  stopWatchIsRunning[number] = 1;
  stopWatchResetFlag[number] = 0;

  return 0;
}

int StopStopWatch(int number){
  if(ConstrainInt(number,0,STOPWATCH_NUM-1) != number){
    return -1;
  }
  if(IsRunningStopWatch(number)){
    stopWatchIsRunning[number] = 0;
    stopWatchStopMillis[number] = Millis();
  }
  return 0;
}

int ResetStopWatch(int number){
  if(ConstrainInt(number,0,STOPWATCH_NUM-1) != number){
    return -1;
  }
  stopWatchResetFlag[number] = 1;
  stopWatchMillis[number] = Millis();
  stopWatchStopMillis[number] = stopWatchMillis[number];
  return 0;
}
int GetStopWatchMillis(int number){
  if(ConstrainInt(number,0,STOPWATCH_NUM-1) != number){
    return -1;
  }
  if(IsRunningStopWatch(number)){
    return Millis() - stopWatchMillis[number];
  }else{
    return stopWatchStopMillis[number] - stopWatchMillis[number];
  }
}
int RestartStopWatch(int number){
	if(ConstrainInt(number,0,STOPWATCH_NUM-1) != number){
	    return -1;
	  }
	StopStopWatch(number);
	ResetStopWatch(number);
	StartStopWatch(number);
	return 0;
}

int IsRunningStopWatch(int number){
  if(ConstrainInt(number,0,STOPWATCH_NUM-1) != number){
    return -1;
  }
  return stopWatchIsRunning[number] != 0 ? 1 : 0;
}


#ifdef __cplusplus
 }
#endif
