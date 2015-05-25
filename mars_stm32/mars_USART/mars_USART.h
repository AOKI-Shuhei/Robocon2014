/*
 * mars_USART.h
 *
 *  Created on: 2013/12/05
 *      Author: Shuhei
 */

#ifndef MARS_USART_H_
#define MARS_USART_H_
#include "mars_stm32.h"

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * mars_stm32�̕��̃V���A���ʐM�֘A�̐݌v�ύX�ɔ����A�L���[�i�����O�o�b�t�@���Ă�����ł��ˁj�͕K�v�Ȃ��Ȃ����̂ł��̕ӂ͑啝�폜�ɂ��܂����B
 * �c�����֐���USART_Readnbyte�݂̂ŁA����͑�1�������ς�����݂̂ŗv���d�l�͂��Ƃ̂܂܂ł��B
 */



//��M����f�[�^�̍ő吔
#define USART_MAX_DATA_SIZE (40)

int USART_Readnbyte(USART_TypeDef* USARTx, char* data,short length);






#ifdef __cplusplus
 }
#endif



#endif /* MARS_USART_H_ */
