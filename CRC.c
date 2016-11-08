#include "Main.h"

//產生CRC
unsigned int crcGenerator(unsigned char *crcdata,char sizeofarray)
{
	char j=0,k=0;
	//char sizeofarray=sizeof(crcdata); //計算長度
	unsigned int reg_crc=0xFFFF; //CRC初始化
	unsigned int outCRC;
	for(j=0;j<sizeofarray;j++)//CRC運算
	{
		reg_crc ^= crcdata[j];
		for(k=0;k<8;k++)
		{
			if(reg_crc & 0x01)
				reg_crc=(reg_crc>>1) ^ 0xA001;
			else
				reg_crc=reg_crc >>1;
		}
	}
	//CRC高低位元交換(先高再低)
	outCRC=((reg_crc<<8)&0xFF00)|((reg_crc>>8)&0x00FF);
	
	return outCRC;
}

//驗證CRC
char crcVertify(unsigned char *crcdata,char sizeofarray)
{
	unsigned int checkCRC=0,HighCRC=0,LowCRC=0,CRC=0;
	
	checkCRC=crcGenerator(crcdata,sizeofarray);//產生傳來的CRC
	HighCRC=crcdata[sizeofarray];//低位元CRC
	LowCRC=crcdata[sizeofarray+1];//高位元CRC
	CRC=((HighCRC<<8)&0xFF00)|(LowCRC&0x00FF);//合併原始CRC
	
	if(checkCRC==CRC)//檢查CRC
		return 0;
	else
		return 1;
}
