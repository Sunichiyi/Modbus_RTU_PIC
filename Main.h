#include <p18f4525.h> 	//納入微控制器定義檔
#include <delays.h>		//納入時間延遲函式庫定義檔
//#include <timers.h>		//納入計時器函式庫定義檔
#include <usart.h>		//納入USART函式庫定義檔
#include <adc.h>
#include <string.h>

void isr_high_direct(void);	
void isr_high(void);

void InitializePORT(void);
void InitializeAD(void);
void InitializeUSART(void);
void InitializeINT(void);

void delay_ms(long);

void err_Modbus(unsigned char err_Code);
unsigned int crcGenerator(unsigned char *crcdata,char sizeofarray);//產生CRC
char crcVertify(unsigned char *crcdata,char sizeofarray);//驗證CRC
