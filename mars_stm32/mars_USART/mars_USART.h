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
 * mars_stm32の方のシリアル通信関連の設計変更に伴い、キュー（リングバッファっていうんですね）は必要なくなったのでこの辺は大幅削除にしました。
 * 残った関数はUSART_Readnbyteのみで、これは第1引数が変わったのみで要求仕様はもとのままです。
 */



//受信するデータの最大数
#define USART_MAX_DATA_SIZE (40)

int USART_Readnbyte(USART_TypeDef* USARTx, char* data,short length);






#ifdef __cplusplus
 }
#endif



#endif /* MARS_USART_H_ */
