#include "mars_stm32.h"

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * ���X�̏����ݒ���s���֐��B���[�J���ϐ��̐錾���I�������ŏ��Ɉ�x�Ă�ł��������B
 * ����A�g�������ȃ��W���[���͑S�ċN�����Ă���̂ŏȓd�͉��Ȃǂ͂���Ă��Ȃ��ł��B
 * ���̊֐���ύX����ꍇ�͕ύX�_�ƕύX�Җ��������̃R�����g�Ɏc���Ă�������
 *
 * @Author			AOKI
 *
 * USART�����݂̗D��x��ݒ�H	AOKI
 */
void RCC_Configuration(void)
{
	NVIC_InitTypeDef nvic;
	//�悭�킩��Ȃ����ǂƂ肠���������Ă�
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div1);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	RCC_ADCCLKConfig(RCC_PCLK2_Div2);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 |	//TIM2�N��
						RCC_APB1Periph_TIM3 |		//TIM3�N��
						RCC_APB1Periph_TIM4 |		//TIM4�N��
						RCC_APB1Periph_USART2 |		//USART2�N��
						RCC_APB1Periph_USART3,		//USART3�N��
						ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |	//ADC�N��
						RCC_APB2Periph_GPIOA |		//GPIOA�N��
						RCC_APB2Periph_GPIOB |		//GPIOB�N��
						RCC_APB2Periph_GPIOC |		//GPIOC�N��
						RCC_APB2Periph_GPIOD |		//GPIOD�N��
						RCC_APB2Periph_AFIO |		//Alternate Function�N��
						RCC_APB2Periph_USART1 |		//USART1�N��
						RCC_APB2Periph_TIM1,		//TIM1�N��
						ENABLE);
	//Systick�^�C�}�̊����݂̊Ԋu��ݒ�
	if(SysTick_Config(SystemCoreClock / SYSTICK_INTERRUPT_TIMING)){
		while(1);
	}
	//�����݂̗D��x���Ȃ񂿂��B�����Ă݂�����
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
 * �s���ݒ���s���֐�
 * �����Őݒ�ł���̂�GPIO�APWM�AADC�Ɋւ���
 * @param pin_number    �s���ԍ�
 * @param mode          �s���ݒ�
 * @return              -1�Ȃ�ݒ�Ɏ��s
 *
 * @Author              AOKI
 *
 * ����m�F�ς�
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
 * �w�肵���s������value�̏o�͂��s��
 * PinMode��OutPut�Ɏw�肵���s���ōs��
 * @param pin_number		�s���ԍ�
 * @param value				�o��
 * @return					-1�Ȃ�ݒ莸�s
 *
 * @Author					AOKI
 *
 * PC14��PC15��PD0��PD1�͎g�p�֎~(���U�킪���Ă�j
 * Remap�̊m�F�͂��ĂȂ�������ȊO�͓��삷��
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
 * �w�肵���s���̏�Ԃ𓾂�
 * PinMode��INPUT���w�肵���s���ōs��
 * @param pin_number			�s���ԍ�
 * @return 						�s���̏�ԁBHIGH�Ȃ�1�ALOW�Ȃ�0�B-1�Ȃ�擾�Ɏ��s
 *
 * @Author						AOKI
 *
 * PC14��PC15��PD0��PD1�͎g�p�֎~(���U�킪���Ă�j
 * Remap�̊m�F�͂��ĂȂ�������ȊO�͓��삷��
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
 * 	�w�肵���s����ADC���s���֐�
 * 	@param pin_number		�s���ԍ�
 * 	@return 				-1�Ȃ�ADC���s�i�s���ԍ����Ⴄ�j����ȊO�Ȃ�ADC�l
 *
 * 	@Author					AOKI
 *
 * 	�S�ē���m�F�ς݁B������ADC16�͓����̉��x�v�ɐڑ�����Ă���̂ōl�����Ă��Ȃ�
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
 * �w�肵���s������PWM�o�͂��s���֐�
 * @param pin_number		�s���ԍ�
 * @param value				�f���[�e�B�[��	0-999
 * @return 					-1��PWM�̃s�������������@0�͐���I���@1�͒l��pulse�ύX�Ȃ��ł̏I��
 *
 * @Author					AOKI
 *
 * Remap�̃`�F�b�N�͂܂�
 * TIM2��ch3�ȊO�͓���m�F�ρBTIM2��ch3�����삵�Ȃ������͂킩��Ȃ��̂ŕۗ�
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
 * �w�肵���s������T�[�{�p��PWM�o�͂��s���֐�
 * @param pin_number		�s���ԍ�
 * @param degree			�p�x
 * @return 					-1��PWM�̃s�������������@0�͐���I���@1�͒l��pulse�ύX�Ȃ��ł̐���I��
 *
 * @Author					AOKI
 *
 * Remap�̃`�F�b�N�͂܂�
 * TIM2��ch3�ȊO�͓���m�F�ρBTIM2��ch3�����삵�Ȃ������͂킩��Ȃ��̂ŕۗ�
 *
 * ���݊p�x�͈�-45~45�B-90~90�ɂ���ɂ�degree*=2���폜
 */
int SERVO_Write(PIN_NUMBER pin_number,int degree){
	TIM_OCInitTypeDef OC_Init;
	void (*channelsInit[])(TIM_TypeDef*,TIM_OCInitTypeDef*) = {TIM_OC1Init,TIM_OC2Init,TIM_OC3Init,TIM_OC4Init};
	uint16_t (*TIM_Pulse_Capture[])(TIM_TypeDef*) = {TIM_GetCapture1,TIM_GetCapture2,TIM_GetCapture3,TIM_GetCapture4};
	TIM_TypeDef *TIMx;
	uint16_t channel;
	uint16_t capture_pulse;
	int value = 900;
	//degree �� duty��ϊ�
	//����duty�� = degree�ɂ���
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
 * GPIO�̐ݒ���s���֐�
 * PinMode����Ă΂��
 * @param pin_number    �s���ԍ�
 * @param mode          �s���ݒ�
 * @return              -1�Ȃ�ݒ�Ɏ��s
 *
 * @Author              AOKI
 *
 * PC14��PC15��PD0��PD1�͎g�p�֎~(���U�킪���Ă�j
 * Remap�̊m�F�͂��ĂȂ�������ȊO�͓��삷��
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
 * ADC�̐ݒ���s���֐�
 * PinMode����Ă΂��
 * @param pin_number	�s���ԍ�
 * @return 				-1�Ȃ�ݒ�Ɏ��s
 *
 * @Author				AOKI
 *
 * ����m�F��
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
 * PWM�̐ݒ���s���֐�
 * PinMode����Ă΂��
 * @param				�s���ԍ�
 * @return 				-1�Ȃ�ݒ�Ɏ��s
 *
 * @Author				AOKI
 *
 * Remap�̃`�F�b�N�͂܂�
 * TIM2��ch3�ȊO�͓���m�F�ρBTIM2��ch3�����삵�Ȃ������͂킩��Ȃ��̂ŕۗ�
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
 * �T�[�{�p��PWM�̐ݒ���s���֐�
 * ���ʂ�PWM���o�͂���ꍇ��20kHz���������ł�50Hz�ƂȂ�
 * ����^�C�}����͓������g�������o�Ȃ��̂Œ���
 * PinMode����Ă΂��
 * @param				�s���ԍ�
 * @return 				-1�Ȃ�ݒ�Ɏ��s
 *
 * @Author				AOKI
 *
 * Remap�̃`�F�b�N�͂܂�
 * TIM2��ch3�ȊO�͓���m�F�ρBTIM2��ch3�����삵�Ȃ������͂킩��Ȃ��̂ŕۗ�
 * ���g����M���������Ȃ̂ő��v�Ȃ͂�
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
 * �s���ԍ�����|�[�g�ƃs���𒊏o����֐�
 * @param pin_number    �s���ԍ�
 * @param port          �|�[�g
 * @param pin           �s�����l
 * @return              -1�Ȃ�ݒ�Ɏ��s
 *
 * @Author              AOKI
 *
 * ����m�F��
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
 * �s���ԍ�����^�C�}�ƃ`���l���𒊏o����֐�
 * @param			�s���ԍ�
 * @TIMx			�^�C�}
 * @channel			�`���l��
 * @return			-1�Ȃ璊�o���s
 *
 * @Author			AOKI
 *
 * remap�ȊO����m�F��
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
 * �s���ԍ�����Ή�����ADC�`�����l�������擾����֐�
 * @param pin_number			�s���ԍ�
 * @param channel				�`���l��
 * @return 						-1�Ȃ�擾���s
 *
 * @param						AOKI
 *
 * ����m�F��
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
 * USART�̐ݒ���s���֐�
 * @param USARTx					USART�̔ԍ��BUSART1,USART2,USART3����I��
 * @oaram mode						�ʐM���[�h�BSEND,READ,SEND_AND_READ����I���iSEND_AND_READ��(SEND | READ)�Ɠ����j
 * @param remap						���}�b�v���s�����ǂ����BENABLE/DISABLE�B
 * @return							-1�Ȃ�ݒ莸�s
 *
 * @Author							AOKI
 *
 * remap�ȊO����m�F�ς�
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
 * USART���J�n����֐�
 * @param USARTx					USART�̔ԍ��BUSART1,USART2,USART3����I��
 * @param baudrate					�{�[���[�g
 * @return							���ɂȂ�
 *
 * @Author							AOKI
 *
 * remap�ȊO����m�F�ς�
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
 * USART��1byte���M����֐�
 * @param USARTx				USART�̔ԍ�
 * @param tx_data				���M�f�[�^
 * @return 						-1�Ȃ瑗�M�o�b�t�@����t
 * 								-2�Ȃ瑗�M�ݒ�ɂ���Ă��Ȃ�����
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
 * USART��1byte��M����֐�
 * @param USARTx				USART�̔ԍ�
 * @retuern						��M�f�[�^�B�ǂ����~���Ă�0�Ȃ�ǂ����Ŏ��s���������Ă���\��������܂��B
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
 * USART�ŕ���byte���M����֐��B���M�t�H�[�}�b�g�͓��Ɏw��Ȃ�
 * @param USARTx					USART�̔ԍ��BUSART1,USART2,USART3����I��
 * @param tx_data					���M�f�[�^��
 * @param length					���M�f�[�^��
 * @return 							-1�Ȃ瑗�M�o�b�t�@���I�[�o�t���[
 * 									-2�Ȃ瑽�����M�ɐݒ肳��ĂȂ�
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
 * USART�ŕ���byte���M����֐��B�������z�肵�Ă��ďI�[�ɏI�[����'\0'���Ȃ��Ƒ�ςȂ��ƂɂȂ�܂��B
 * ���M�f�[�^�ɂ�'\0'�͊܂�
 * @param USARTx				USART�̔ԍ�
 * @param data					���M�f�[�^
 *
 * @return 						-1�Ȃ瑗�M�o�b�t�@�I�[�o�t���[
 * 								-2�Ȃ瑽�����M�ݒ肳��Ă��Ȃ�
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
 * USART�Ŏ�M����֐�
 * @param USARTx					USART�̔ԍ��BUSART1,USART2,USART3����I��
 * @param rx_data					��M�f�[�^������z��
 * @param length					�z�肷���M�f�[�^��
 * @return 							-1��USARTx�ɓn����������������
 * 									-2�Ȃ瑽����M���ݒ肳��ĂȂ�
 * 									1�Ȃ��M�o�b�t�@��length���܂��Ă��Ȃ�
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
 * ��M�o�b�t�@�̃T�C�Y���擾����֐�
 * @param USARTx					USART�̔ԍ�
 * @return							�o�b�t�@�T�C�Y�B-1�Ȃ��������������
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
 * ��M�o�b�t�@�������֐�
 * @param USARTx					USART�̔ԍ�
 * @return 							-1�Ȃ��������������
 *
 * @Author							AOKI
 *
 * �܂������ĂȂ��ł�
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
 * ���M�o�b�t�@�������֐�
 * @param USARTx					USART�̔ԍ�
 * @return 							-1�Ȃ��������������
 *
 * @Author							AOKI
 *
 * �܂������ĂȂ��ł�
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
 *�@���͂��ꂽ���ԏ������~�߂�֐�
 *�@�����������݂͎��s�����
 *
 *�@@param msec			�������~�߂鎞�ԁD�~���b
 *
 *�@@Author				AOKI
 *
 * ����m�F�ς�
 */
void Delay(int msec){
	Timing_delay = msec > 0 ? msec : 0;
	for(;Timing_delay > 0;);
}

/**
 * �v���O�������n�����Ă���̌o�ߎ��Ԃ�Ԃ��֐�
 * �P�ʂ̓~���b
 * 596���Ԃ��炢�ŃI�[�o�t���[����̂Œ���
 *
 * @return				�v���O�����n������̌o�ߎ��ԁi�~���b�j
 *
 * @Author				AOKI
 */
int Millis(){
	return Now_milli_second;
}

/**
 * �����O�o�b�t�@��1byte�����֐�
 * @param l_buffer			�Ώۂ̃����O�o�b�t�@
 * @param data				�f�[�^
 * @return					-1�Ȃ�I�[�o�t���[
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
 * �����O�o�b�t�@����1byte���o���֐�
 * @param l_buffer			�Ώۂ̃o�b�t�@�����O
 * @param data				�f�[�^������ϐ�
 * @return 					-1�Ȃ�f�[�^�Ȃ��A���̓o�b�t�@���L���ł͂Ȃ�
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
 * �����O�o�b�t�@�\���̂̏������֐�
 * @param l_buffer				�Ώۂ̃����O�o�b�t�@
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
 * �����݃n���h���B�C�ɂ��Ȃ��Ă����ł�
 * SysTick_Handler��stm32f10x_it.o�ƂȂ񂿂����ăR���p�C���ɓ{��ꂽ��stm32f10x_it.c�̓����̊֐����폜���Ă�������
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
 * USART1�̊����݃n���h���B�C�ɂ��Ȃ��Ă����ł��B�ȉ����������̊֐��́T
 * �R���p�C���ɓ{���Ă烁�b�Z�[�W��ǂ�őΏ����Ă��������B
 *
 *�@@Author						AOKI
 * @����Ă邱��
 * ������M�����炱�̊֐��������݂ŌĂ΂�܂��B���̎��Ɏ�M�o�b�t�@�����O�Ɏ�M�f�[�^�����܂�
 * ���M�\�ɂȂ��ĂĂ����̊֐����Ă΂�܂��B���M�o�b�t�@�����O����1byte���o���đ��M���܂��B���M�o�b�t�@�����O�̃T�C�Y��0�Ȃ瑗�M�̊����݂͂�����܂���
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
