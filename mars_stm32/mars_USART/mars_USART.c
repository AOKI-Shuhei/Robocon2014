/*
 * mars_USART.c
 *
 *  Created on: 2013/12/05
 *      Author: Shuhei
 */


#include"mars_USART.h"






/**
 * USART����f�[�^����M���ăf�[�^��������������������֐�
 * @param USARTx			�Ώۂ�USART
 * @param data				��M�f�[�^������z��
 * @param length			��M�f�[�^��
 * @return 					1�Ȃ��M�o�b�t�@ < length +1
 * 							-1�Ȃ��M�f�[�^����������
 *
 * ���̊֐��ɂ���M�f�[�^�́A
 * 0xff���擪�o�C�g
 * �ȉ�MSB��0�̃f�[�^��length����
 *
 * �Ȃ̂Ŏ��ۂɎ�M����f�[�^length+1byte�ł��B�擪��0xff�͔z��data�ɓn�����ɂ͊܂߂܂���
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




