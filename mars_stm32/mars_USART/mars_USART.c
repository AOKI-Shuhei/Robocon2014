/*
 * mars_USART.c
 *
 *  Created on: 2013/12/05
 *      Author: Shuhei
 */


#include"mars_USART.h"






/**
 * USARTからデータを受信してデータが正しいかを検討する関数
 * @param USARTx			対象のUSART
 * @param data				受信データを入れる配列
 * @param length			受信データ幅
 * @return 					1なら受信バッファ < length +1
 * 							-1なら受信データがおかしい
 *
 * この関数による受信データは、
 * 0xffが先頭バイト
 * 以下MSBが0のデータがlength個続く
 *
 * なので実際に受信するデータlength+1byteです。先頭の0xffは配列dataに渡す情報には含めません
 *
 * @Author					AOKI
 */
int USART_Readnbyte(USART_TypeDef* USARTx, char* data,short length){
	
	char tempdata[USART_MAX_DATA_SIZE];
	int i;
	char temp[1];
	if(USART_GetRxBufferSize(USARTx) < (length+1)){
		return 1;
	}
	
	USART_Read(USARTx,temp,1);
	
	if(temp[0] == 0xff){
		for(i=0;i<length;i++){
			USART_Read(USARTx,temp,1);
			if((temp[0] & 0x80) != 0x00){
				return -1;
			}
			tempdata[i] = temp[0];
		}
		for(i=0;i<length;i++){
			data[i] = tempdata[i];
		}
		return 0;
	}
	
	return -1;
}




