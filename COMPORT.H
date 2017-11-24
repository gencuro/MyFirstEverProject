#ifndef __MSDOS__

#include <time.h>
#include "variable.h"
DCB dcb;
HANDLE hCom;

//----------------------------------------------------------------
int OpenPort( char* Nport, int  baudrate, int sbit, int nbit, int parity, FILE *file)
{
	char port[5] = "COM";
	strcat(port, Nport);

	BOOL Success;

	hCom = CreateFile( port,
					   GENERIC_READ | GENERIC_WRITE,
                       0, 
                       NULL, 
                       OPEN_EXISTING, 
                       0, 
                       NULL 
                     );

if (hCom == INVALID_HANDLE_VALUE) {  //если не удалось открыть
  printf_rus ("Ошибка! Не удалось открыть COM порт.\n"); 
  fprintf(file, "Ошибка. COM порт не открыт. \n");
return (OPEN_COM_ERR);
}
else 
{	
	printf_rus("Com порт открыт. \n");
	fprintf(file, "COM порт открыт. \n");
}

Success = GetCommState(hCom, &dcb);

if (!Success) {
  printf ("GetCommState failed with error.\n");
return (GETCOMSTAT_ERR);
}

dcb.BaudRate = baudrate;
dcb.StopBits = (BYTE) sbit;
dcb.ByteSize = nbit;
dcb.Parity = (BYTE) parity;


Success = SetCommState( hCom, &dcb);

if (!Success) { //неправильное значение для установки
  printf ("SetCommState failed with error. \n");
  fprintf(file, "Ошибка, неправильное значение для установки. \n");
return (SETCOMSTAT_ERR);
}
printf_rus("Com порт успешно переконфигурирован. \n");
  fprintf(file, "COM порт переконфигурирован. \n");
 
  return NO_ERR;
};
//----------------------------------------------------------------
int write_COM(char *data, unsigned long count)
{
	DWORD charsWrite = 0;

	if(!WriteFile ( hCom
			    , (LPVOID) data
				, (DWORD) count
                , &charsWrite
                , NULL
                )				
	   )
		    {
		printf_rus("Ошибка! Запись не удалась.\n");
			};	
return NO_ERR;
};

//----------------------------------------------------------------
int read_COM(char *data, unsigned long count)
{
	DWORD charsRead = 0;

	if(!ReadFile ( hCom
				  , (LPVOID) data
				  , (DWORD) count
				  , &charsRead
				  , NULL
				  )
	   )
	{
		printf_rus("Ошибка! Чтение не удалось.\n");
	};	
return NO_ERR;
};
//----------------------------------------------------------------
void ClosePort(FILE *file, int errorlevel)
{
	time_t timee;
	time(&timee);

	 if(CloseHandle( hCom )) 
	 {
		 fprintf(file, "\nCOM порт закрыт. \n");
		 printf_rus ("Com порт закрыт. \n");
	 }
	 else
	 {
		 printf_rus ("Ошибка! Не удалось закрыть Com порт.\n");
		 fprintf(file, "\n Ошибка! Не удалось закрыть Com порт.\n");
		 errorlevel = COM_CLOSE_ERR;
	 }
	 /*************************************************************************************/
	time(&timee);
	printf("%s", ctime(&timee));

	if(errorlevel == NO_ERR) fprintf(file, "Работа завершена без ошибок. \n");
	else fprintf(file, "Работа завершена с ошибкой:%d. \n", errorlevel);

	fprintf(file, "Завершение работы:  %s", ctime(&timee));
	fprintf(file, "|*************************************************************************************|\n");
	fclose( file );
};

#endif



#ifdef __MSDOS__

#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <BA435\variable.h>

void printf_rus(char str[])
{
		for(int i = 0; str[i] != '\0'; i++)
	{
		unsigned char cstr = str[i];

		if((cstr >= 0xE0) && (cstr < 0xF0))
		{
			cstr -= 0x40;
			printf("%c", cstr);
		}
		else if((cstr >= 0xF0) && (cstr <= 0xFF))
		{
			cstr -= 0x10;
			printf("%c", cstr);
		}
		else if((cstr >= 0xC0) && (cstr <= 0xDF))
		{
			cstr -= 0x40;
			printf("%c", cstr);
		}
		else printf("%c", cstr);
	}
}
/*---------------------------------------------------*/
/******************************************************/
int	RS232_base_ports[MAX_RS232] = {0x3F8, 0x2F8, 0x3E8, 0x2E8};

int nbit_array[4][2] = {{0, 5},
						{1, 6},
						{2, 7},
						{3, 8},
						};

int base_port;

char RS232_receivebyte(int num)
{
	return inportb(RS232_base_ports[num]);
}

void RS232_sendbyte(int num, char byte)
{
	outportb(RS232_base_ports[num], byte);
}

/********************************************************/

int hCom;
int num;

int OpenPort(char* Nport, int baudrate, int sbit, int nbit, int parity, FILE *file)
{
	/****************************************************/
	char format_byte;
	register unsigned divisor;
    int word_length;

    num = atoi(Nport) - 1;

    base_port = RS232_base_ports[num];

	int i = 0;
	for(i = 0; i < 4; i++)
	{
		if(nbit_array[i][1] == nbit)
		{
			word_length = nbit_array[i][0];
			break;
		}
	}

	// Errors control:
	if ((num > MAX_RS232-1)||(word_length > 3)||(sbit > 1)||(parity > 7)||(!baudrate)) return RS232_INVALID_PARAM;

	format_byte = 0x3F & ((word_length)|(sbit << 2)|(parity << 3));


	outportb(RS232_base_ports[num] + 1, 0);   /* Turn off interrupts - Port1 */
    outportb(RS232_base_ports[num] + 3, 0x80);/* SET DLAB ON */

//set baudrate
	/* ДЕЛИТЕЛЬ = 1.8432 МГц / (baud_rate*16) */
//	divisor = (unsigned)(115200l / baudrate);
//	outp(RS232_base_ports[0] + 0, divisor & 0xff);    /* МЛАДШИЙ БАЙТ ДЕЛИТЕЛЯ     */
//	outp(RS232_base_ports[0] + 1, divisor >> 8);  /* СТАРШИЙ БАЙТ ДЕЛИТЕЛЯ     */

	  outportb(RS232_base_ports[num] + 0, 0x01);/* Set Baud rate - Divisor Latch Low Byte */
							   /* Default 0x03 =  38,400 BPS */
							   /*         0x01 = 115,200 BPS */
							   /*         0x02 =  57,600 BPS */
							   /*         0x06 =  19,200 BPS */
							   /*         0x0C =   9,600 BPS */
							   /*         0x18 =   4,800 BPS */
							   /*         0x30 =   2,400 BPS */
    outportb(RS232_base_ports[num] + 1, 0x00);/* Set Baud rate - Divisor Latch High Byte */

    outportb(RS232_base_ports[num] + 3, format_byte);//configure com port

	outportb(RS232_base_ports[num] + 1, 0x01); // Разрешаем прерывания по приему

    outportb(RS232_base_ports[num] + 2, 0xC7);/* FIFO Control Register */
    outportb(RS232_base_ports[num] + 4, 0x0B);/* Turn on DTR, RTS, and OUT2 */

	printf_rus("Com порт открыт. \n");
	fprintf(file, "COM порт открыт. \n"); //Com port otkrit

return NO_ERR;
}

int write_COM(char *data, unsigned long count)
{
	inportb(RS232_base_ports[num] + 0); // Clear

	register int tm = TIME_OUT;//timeout
	int i;
	for(i = 0; i < count; i++)
	{
		
		while((inportb(RS232_base_ports[num] + 5) & 0x20) == 0)
			if (tm++ > 0) 			
				return TIME_OUT_ERR;
			
		RS232_sendbyte(num, data[i]);
	}
return NO_ERR;
}

int read_COM(char *data, unsigned long count)
{
	register int tm = TIME_OUT;//timeout
	int i;
	for(i = 0; i < count; i++)
	{
		while((inportb(RS232_base_ports[num] + 5) & 0x1) == 0)
			if (tm++ > 0)
				return TIME_OUT_ERR;

		data[i] = RS232_receivebyte(num);
	}
return NO_ERR;
}

void ClosePort(FILE *cf, int errorlevel)
{
	outp(RS232_base_ports[num] + 1,0);   /* ЗАПРЕЩЕНИЕ АППАРАТНЫХ ПРЕРЫВАНИЙ */
	outp(RS232_base_ports[num] + 4,0);   /* МАСКИРОВКА СИГНАЛОВ              */
	
	printf_rus("Com порт закрыт. \n");
	fprintf(cf, "\nCOM порт закрыт. \n");	//Com port zakrit

/*************************************************************************************/
time_t timee;
time(&timee);
printf("%s", ctime(&timee));

if(errorlevel == 0) fprintf(cf, "Работа завершена без ошибок. \n");//rabota zaversena bez osibok
else fprintf(cf, "Работа завершена с ошибкой:%d. \n", errorlevel);//rabota zaversena s osibkoy

fprintf(cf, "Завершение работы:  %s", ctime(&timee));//zaversenie raboti
fprintf(cf, "|*************************************************************************************|\n");
fclose( cf );
/*************************************************************************************/
}
#endif