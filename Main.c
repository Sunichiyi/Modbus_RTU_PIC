#include "Main.h"

// 設定微處理器相關結構位元
#pragma config	OSC=HS, BOREN=OFF, BORV = 2, PWRT=OFF, WDT=OFF, LVP=OFF

#define OSC_CLOCK		16//震盪器16MHz
#define Slave_Address	0x02//Modbus位址
#define ModbusIO		PORTDbits.RD7
#define FuncModbus		readModbus[1]
#define Sensor			2
#define Sensorlength	2

unsigned char Data[Sensor*Sensorlength]={0};
unsigned char readModbus[8]={0};
unsigned char writeModbus[Sensorlength<<3]={0};

//************************************************
//*       #pragma Interrupt Declarations         *
//************************************************
//************************************************
//* Function: isr_high_direct                    *
//*   - Direct execution to the actual           *
//*     high-priority interrupt code.            *
//************************************************
#pragma code isrhighcode = 0x0008

void isr_high_direct(void)	
{  								
     _asm 			//begin in-line assembly
     goto isr_high	//go to isr_high function
     _endasm 		//end in-line assembly
}
#pragma code

//************************************************
//* Function: isr_high(void)                     *     
//*   High priority interrupt:	                 *
//*   - Received a serial data from RS-232       *
//*     Save the received data to buffer Rec_Data*
//*   - Timer 2 Interrupt : 5 mS 				 *                     
//************************************************
#pragma interrupt isr_high

void isr_high(void)
{
	char i=0,arraylength=0,check=0,for_count=0,data_count=0;
	int readStart=0,readCount=0,readCRC=0;
	
	readModbus[0]=RCREG;
	if(readModbus[0]==Slave_Address)//確認呼叫對象
	{
		while(!PIR1bits.RCIF);
		FuncModbus=RCREG;
		if((FuncModbus==0x03)||(FuncModbus==0x04))//確認Function
		{
			for(i=2;i<8;i++)
			{
				while(!PIR1bits.RCIF);
				readModbus[i] = RCREG;
			}
			check=crcVertify(readModbus,6);
			if(check==0)
			{
				writeModbus[0]=Slave_Address;
				writeModbus[1]=FuncModbus;
				readStart=readModbus[2];
				readStart=(readStart<<8)|readModbus[3];
				if(readStart<0x02)
				{
					readCount=readModbus[4];
					readCount=(readCount<<8)|readModbus[5];
					if((readCount<=Sensor)&(readCount!=0)&((readStart*Sensorlength)+(readCount*Sensorlength))<=Sensor*Sensorlength)
					{
						for_count=(readStart+readCount)*Sensorlength;
						for(i=3,data_count=0,arraylength=readStart*Sensorlength;arraylength<for_count;i++,data_count++,arraylength++)
						{
							writeModbus[i]=Data[arraylength];//資料提取並計算總筆數
						}
						writeModbus[2]=data_count;//傳回資料筆數
						readCRC=crcGenerator(writeModbus,(data_count+3));//轉換CRC(陣列,總傳回筆數)
						writeModbus[data_count+4]=readCRC;//高位元CRC
						writeModbus[data_count+3]=readCRC>>8;//低位元CRC
						
						ModbusIO=1;
						for_count=data_count+5;
						for(i=0;i<for_count;i++)
						{
							TXREG = writeModbus[i];//WriteUSART();
							while(!TXSTAbits.TRMT);//BusyUSART()
						}
						ModbusIO=0;
					}
					else
					{
						err_Modbus(0x03);
					}
				}
				else
				{
					err_Modbus(0x02);
				}
			}	
		}
		else
		{
			err_Modbus(0x01);
		}	
	}
}

void main(void)
{
	InitializePORT();
	InitializeAD();
	InitializeUSART();
	InitializeINT();
	PORTBbits.RB0=0;
	while(1)
	{
		ADCON0bits.CHS0=0;
		ConvertADC();
		while(BusyADC());
		Data[0] = ADRESH;
		Data[1] = ADRESL;
		
		ADCON0bits.CHS0=1;
		ConvertADC();
		while(BusyADC());
		Data[2] = ADRESH;
		Data[3] = ADRESL;
	}	
}

//毫秒延遲
void delay_ms(long A)
{
//This function is only good for OSC_CLOCK higher than 4MHz
	int i,us2TCY;
	us2TCY=(10*OSC_CLOCK)>>2;
	for(i=0;i<A;i++)
	{
		Delay100TCYx(us2TCY);
	}	
}

void err_Modbus(unsigned char err_Code)
{
	unsigned char err_ModbusCode;
	
	PORTBbits.RB0=1;
	if(err_Code==0x01)
	{
		delay_ms(5);
	}
	err_ModbusCode=FuncModbus|0x80;
	TXREG = err_ModbusCode;
	while(!TXSTAbits.TRMT);
	TXREG = err_Code;
	while(!TXSTAbits.TRMT);
	PORTBbits.RB0=0;
}	
