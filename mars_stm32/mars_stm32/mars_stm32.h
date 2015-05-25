

#ifndef MARS_STM32_H_
#define MARS_STM32_H_
#include"stm32f10x.h"
#include"stm32f10x_conf.h"
#include"mars_functions.h"

#ifdef __cplusplus
 extern "C" {
#endif
   



/**
 * PWM�̃N���b�N�֘A�BPWM_CLOCK��PWM�̎��g��
 * PWM_DUTY�̓f���[�e�B�[��̕���(���ۂ�PWM_DUTY-1������j
 * ((SystemCoreClock/PWM_PRESCALAR) / PWM_CLOCK) - 1 �� 65535 �������悤��PWM_PRESCALAR�𒲐߂���
 * SystemCoreClock��24M
 */
#define PWM_CLOCK (20000)
#define PWM_PRESCALAR (1)
#define PWM_DUTY (1000)

#define SERVO_PWM_CLOCK (50)
#define SERVO_PWM_PRESCALAR (24)
#define SERVO_PWM_DUTY (20000)

/**
 * SysTick�^�C�}�̊����݊Ԋu
 * (1/SYSTICK_INTERRUPT_TIMING)�b��1��SysTick�^�C�}���荞�݂���������B
 */
#define SYSTICK_INTERRUPT_TIMING (1000)


   
#define PIN_NUMBER_PORTA (0x10000)
#define PIN_NUMBER_PORTB (0x20000)
#define PIN_NUMBER_PORTC (0x40000)
#define PIN_NUMBER_PORTD (0x80000)

#define STOPWATCH_NUM (10)



static int Timing_delay = 0;	//Delay(int)�p�̕ϐ�
static int Now_milli_second = 0;	//Millis()�p�̕ϐ�

/**
 * �s���ԍ��B
 *
 * @Author		AOKI
 *
 * ����m�F�ς�
 */
typedef enum PIN_NUMBER{
  PA0 = 0x0001 + 0x10000,
  PA1 = 0x0002 + 0x10000,
  PA2 = 0x0004 + 0x10000,
  PA3 = 0x0008 + 0x10000,
  PA4 = 0x0010 + 0x10000,
  PA5 = 0x0020 + 0x10000,
  PA6 = 0x0040 + 0x10000,
  PA7 = 0x0080 + 0x10000,
  PA8 = 0x0100 + 0x10000,
  PA9 = 0x0200 + 0x10000,
  PA10 = 0x0400 + 0x10000,
  PA11 = 0x0800 + 0x10000,
  PA12 = 0x1000 + 0x10000,
  PA13 = 0x2000 + 0x10000,
  PA14 = 0x4000 + 0x10000,
  PA15 = 0x8000 + 0x10000,
  
  PB0 = 0x0001 + 0x20000,
  PB1 = 0x0002 + 0x20000,
  PB2 = 0x0004 + 0x20000,
  PB3 = 0x0008 + 0x20000,
  PB4 = 0x0010 + 0x20000,
  PB5 = 0x0020 + 0x20000,
  PB6 = 0x0040 + 0x20000,
  PB7 = 0x0080 + 0x20000,
  PB8 = 0x0100 + 0x20000,
  PB9 = 0x0200 + 0x20000,
  PB10 = 0x0400 + 0x20000,
  PB11 = 0x0800 + 0x20000,
  PB12 = 0x1000 + 0x20000,
  PB13 = 0x2000 + 0x20000,
  PB14 = 0x4000 + 0x20000,
  PB15 = 0x8000 + 0x20000,
  
  PC0 = 0x0001 + 0x40000,
  PC1 = 0x0002 + 0x40000,
  PC2 = 0x0004 + 0x40000,
  PC3 = 0x0008 + 0x40000,
  PC4 = 0x0010 + 0x40000,
  PC5 = 0x0020 + 0x40000,
  PC6 = 0x0040 + 0x40000,
  PC7 = 0x0080 + 0x40000,
  PC8 = 0x0100 + 0x40000,
  PC9 = 0x0200 + 0x40000,
  PC10 = 0x0400 + 0x40000,
  PC11 = 0x0800 + 0x40000,
  PC12 = 0x1000 + 0x40000,
  PC13 = 0x2000 + 0x40000,
  PC14 = 0x4000 + 0x40000,
  PC15 = 0x8000 + 0x40000,
  
  PD0 = 0x0001 + 0x80000,
  PD1 = 0x0002 + 0x80000,
  PD2 = 0x0004 + 0x80000,
  
  GETPORT = 0xf0000,
  GETPIN = 0xffff
  
  
}PIN_NUMBER;

typedef enum IO_VALUE{
	HIGH = 1,
	LOW = 0
}IO_VALUE;

/**
 * PinMode�֐��Ŏg�����
 */
typedef enum PIN_MODE{
  IO_INPUT_PU,	//�v���A�b�v����
  IO_INPUT_PD,	//�v���_�E������
  IO_OUTPUT,	//�f�B�W�^���o��
  PWM,			//PWM�o��
  ADC,			//AD�ϊ�����
  SERVO			//�T�[�{�pPWM�o��
}PIN_MODE;

/**
 * USART�̃��[�h
 */
typedef enum USART_MODE{
  SEND = 0x0001,	//���M�̂�
  READ = 0x0002,	//��M�̂�
  SEND_AND_READ = 0x0003,	//����
}USART_MODE;

/**
 * �����O�o�b�t�@�̃T�C�Y�͑���Ȃ��Ȃ����瑝�₵�Ă�������
 */
#define LING_BUFFER_SIZE (1024)

/**
 * �����O�o�b�t�@�p�̍\���́B�C�ɂ��Ȃ��Ă����ł�
 *
 * @Author				AOKI
 */
typedef struct LING_BUFFER{
	char buffer[LING_BUFFER_SIZE];
	short start;
	short end;
	short size;
	short enable;
}LING_BUFFER;

/**
 * ����M���ꂼ��Ƀ����O�o�b�t�@���\��
 *
 * @Author				AOKI
 */
typedef struct USART_LING_BUFFER{
	LING_BUFFER rx_buffer;
	LING_BUFFER tx_buffer;
}USART_LING_BUFFER;

/**
 * USART���ꂼ��ɑ���M�����O�o�b�t�@�𐶐�
 *
 * @Author				AOKI
 */
static USART_LING_BUFFER USART1_buffer;
static USART_LING_BUFFER USART2_buffer;
static USART_LING_BUFFER USART3_buffer;




void RCC_Configuration(void);
int PinMode(PIN_NUMBER pin_number, PIN_MODE mode);
int IO_Write(PIN_NUMBER pin_number,IO_VALUE value);
int IO_Read(PIN_NUMBER pin_number);
int ADC_Read(PIN_NUMBER pin_number);
int PWM_Write(PIN_NUMBER pin_number,int value);
int SERVO_Write(PIN_NUMBER pin_number,int degree);

int USART_Mode(USART_TypeDef* USARTx, USART_MODE mode, FunctionalState remap);
int USART_Begin(USART_TypeDef* USARTx, int baudrate);
int USART_Close(USART_TypeDef* USARTx);
int USART_ParityBegin(USART_TypeDef* USARTx, int baudrate);
int USART_Putc(USART_TypeDef* USARTx,char tx_data);
char USART_Getc(USART_TypeDef* USARTx);
int USART_Write(USART_TypeDef* USARTx, char *data, short length);
int USART_Print(USART_TypeDef* USARTx,char *data);
int USART_Read(USART_TypeDef* USARTx, char* data, short length);
int USART_ReadSplit(USART_TypeDef* USARTx, char split, char* data, short length);
int USART_GetTxBufferSize(USART_TypeDef* USARTx);
int USART_GetRxBufferSize(USART_TypeDef* USARTx);
int USART_ClearRxBuffer(USART_TypeDef* USARTx);
int USART_ClearTxBuffer(USART_TypeDef* USARTx);


static int DecodePin(PIN_NUMBER pin_number,GPIO_TypeDef **port, uint16_t* pin);
static int GPIO_PinMode(PIN_NUMBER pin_number,PIN_MODE mode);
static int ADC_PinMode(PIN_NUMBER pin_number);
static int PWM_PinMode(PIN_NUMBER pin_number);
static int SERVO_PWM_PinMode(PIN_NUMBER pin_number);
static int PWM_Decode(PIN_NUMBER pin_number,TIM_TypeDef **TIMx,uint16_t* channel);
static int ADC_Decode(PIN_NUMBER pin_number,uint8_t* channel);

static int LingBuffer_In(LING_BUFFER *l_buffer,char data);
static int LingBuffer_Out(LING_BUFFER *l_buffer,char *data);
static void LingBuffer_Initialize(LING_BUFFER *l_buffuer);
static int LingBuffer_Contains(LING_BUFFER *l_buffer, char data);



void Delay(int msec);
int Millis(void);

static int stopWatchMillis[STOPWATCH_NUM];
static int stopWatchStopMillis[STOPWATCH_NUM];
static char stopWatchIsRunning[STOPWATCH_NUM];
static char stopWatchResetFlag[STOPWATCH_NUM];
static void InitStopWatch();
int StartStopWatch(int number);
int StopStopWatch(int number);
int ResetStopWatch(int number);
int RestartStopWatch(int number);
int IsRunningStopWatch(int number);
int GetStopWatchMillis(int number);


#ifdef __cplusplus
 }
#endif

#endif /* MARS_STM32_H_ */
