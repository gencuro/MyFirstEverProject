#include "stdafx.h"//Dos-
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>

#ifndef __MSDOS__
#include <windows.h>//Dos-
#include "KZYop.h"
#include "variable.h"
/**********************/
int ahtoi(char * str)
{
int nRet = 0;
 for(int i = 0; str[i] != 0; i++){
  nRet <<= 4;
  if(str[i] >= '0' && str[i] <= '9')
   nRet |= str[i] - '0';
  else if(str[i] >= 'A' && str[i] <= 'F')
   nRet |= str[i] - ('A' - 10);
  else if(str[i] >= 'a' && str[i] <= 'f')
   nRet |= str[i] - ('a' - 10);
 }
 return nRet;
}

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

#endif


#ifdef __MSDOS__
#include <conio.h>
#include <dir.h>
#include <BA435\KZYOP.H>
#include <BA435\VARIABLE.H>

unsigned long ahtoi(char * str)
{
 unsigned long nRet = 0;
 for(int i = 0; str[i] != 0; i++){
  nRet <<= 4;
  if(str[i] >= '0' && str[i] <= '9')
   nRet |= str[i] - '0';
  else if(str[i] >= 'A' && str[i] <= 'F')
   nRet |= str[i] - ('A' - 10);
  else if(str[i] >= 'a' && str[i] <= 'f')
   nRet |= str[i] - ('a' - 10);
 }
 return nRet;
}
#endif

/*---------------------------------------------------*/

int findcmd(char * str)
{
	int command = 0;
	char comm[4];

	if(str[0] == '-')		
	{
		for(int i = 1; str[i] != 0; i++)
				comm[i - 1] = str[i];
	}

	if(strncmp(comm, "com", 3) == 0) command = 1;//-com
	if(strncmp(comm, "op", 2) == 0) command = 2;//-operation
	if(strncmp(comm, "sa", 2) == 0) command = 3;//-start adress
	if(strncmp(comm, "l", 1) == 0) command = 4;//-long
	if(strncmp(comm, "mt", 2) == 0) command = 5;//-memType
	if(strncmp(comm, "mid", 3) == 0) command = 6;//-memId
	if(strncmp(comm, "fn", 2) == 0) command = 7;//-filename
	if(strncmp(comm, "wa", 2) == 0) command = 8;//-wraparound
	if(strncmp(comm, "lp", 2) == 0) command = 9;//-logpath
	if(strncmp(comm, "h", 1) == 0) command = 10;//-help

return command;
}
/*---------------------------------------------------*/
int main(int argc, char* argv[])
{
	int cmd;
	int errorlevel = NO_ERR;
/*-----------------default-----------------------*/	
    int wa = 0;    //false;

	char  * portName = "1";
	int  baudrate = DEFAULT_BAUDRATE, nbit = DEFAULT_NBIT;
	int  sbit = DEFAULT_SBIT, parity = DEFAULT_PARITY;

	unsigned long addr = DEFAULT_ADDR
				 ,llong = DEFAULT_LLONG;

    char filename[PATHFILE_MAX_SIZE];
	char logpath[PATHFILE_MAX_SIZE];
	char memid[MEMID_MAX_SIZE] = "report" ;

	char * memtype;

	char * op; 
	int Nop;

	int lp = 0; // temp for logpath
	int tt; //temp for path
	char dir[PATHFILE_MAX_SIZE];
	FILE *cf;
/*-----------------------------------------------*/
	for(int i = 1; i < argc; i++)
	{		
		cmd = findcmd(argv[i]);

		if(cmd == 1) //-com
		{
			portName = argv[i + 1];
			baudrate = atoi(argv[i + 2]);
			sbit =  atoi(argv[i + 3]);
			nbit = atoi(argv[i + 4]);
			parity = atoi(argv[i + 5]);
			i += 5;
		}
		
		if(cmd == 2) //-operation
		{
			op = argv[i + 1];

			if(op[0] == 'w') Nop = 2;
			else if(op[0] == 'r') Nop = 1;
				else Nop = 3;

			i += 1;
		}
		
		if(cmd == 3) //-start adress
		{
			addr = ahtoi(argv[i + 1]); 
			i += 1;
		}

		if(cmd == 4) //-long
		{
			llong = ahtoi(argv[i + 1]); 
			i += 1;			
		}

		if(cmd == 5) //-memType
		{
			memtype = (char*) argv[i + 1];
			i += 1;
		}

		if(cmd == 6) //-memId
		{
			strcpy(memid, argv[i + 1]);
			i += 1;
		}

		if(cmd == 7) //-filename
		{			
			strcpy(dir, argv[i + 1]);

            i += 1;
			
			for(tt = 0; dir[tt] != '\0'; tt++)
			{
				if((dir[tt] == '/') || (dir[tt] == '*') || (dir[tt] == '?') || (dir[tt] == '"') ||
					(dir[tt] == '<') || (dir[tt] == '>')) 
				{
					printf_rus("Имя файла введено неверно. \n");					
					return errorlevel;
				}	
			}

			tt = 3;
			if((dir[1] == ':') && (dir[2] == '\\'))
				{					
					for(tt; dir[tt] != '\0'; tt++)
					{
						if(dir[tt] == '\\')
						{
							#ifndef __MSDOS__
							strncpy(filename, dir, tt);	
							filename[tt] = '\\';
							filename[tt + 1] = '\0';
							CreateDirectory(filename, NULL);	
							#endif

							#ifdef __MSDOS__
							strncpy(filename, dir, tt);								
							filename[tt] = '\0';							
							mkdir(filename);
							filename[tt] = '\\';
							#endif

						}						
					}
					strcpy(filename, dir);
				}
			else
				{
					///
					for(tt = 0; dir[tt] != '\0'; tt++)
					{
						if((dir[tt] == ':') || (dir[tt] == '\\'))
						{
							printf_rus("Имя файла введено неверно. \n");							
							return errorlevel;
						}	
					}
					///
				}			
		}		

		if(cmd == 8) //-wraparound
		{
			wa = 1; //true
		}

		if(cmd == 9) //-logpath
		{
			lp = 1;
            strcpy(dir, argv[i + 1]);			
			i += 1;

			for(tt = 0; dir[tt] != '\0'; tt++)
			{
				if((dir[tt] == '/') || (dir[tt] == '*') || (dir[tt] == '?') || (dir[tt] == '"') ||
					(dir[tt] == '<') || (dir[tt] == '>')) 
				{
					printf_rus("Имя файла введено неверно. \n");					
					return errorlevel;
				}	
			}
			
			tt = 3;
			if((dir[1] == ':') && (dir[2] == '\\'))
				{					
					for(tt = 0; dir[tt] != '\0'; tt++)
					{
						if(dir[tt] == '\\')
						{
							#ifndef __MSDOS__
							strncpy(logpath, dir, tt);	
							logpath[tt] = '\\';
							logpath[tt + 1] = '\0';
							CreateDirectory(logpath, NULL);	
							#endif

							#ifdef __MSDOS__
							strncpy(logpath, dir, tt);								
							logpath[tt] = '\0';							
							mkdir(filename);
							logpath[tt] = '\\';
							#endif
						}						
					}	
					strcpy(logpath, dir);
				}	
				else
				{
					///
					for(tt = 0; dir[tt] != '\0'; tt++)
					{
						if((dir[tt] == ':') || (dir[tt] == '\\'))
						{
							printf_rus("Имя файла введено неверно. \n");							
							return errorlevel;
						}	
					}
					///
				}			
		 }
		 if(cmd == 10) //-help
		 {
			 printf_rus("Ключи: \n 1) -com \n Используется для установки параметров порта.  \n -com [N_port] [budrate] [stop_bits] [n_bits] [parity] \n");
			 printf_rus("\n 2) -op \n Операция. \n -op <операция>  (операции: r - чтение, w - запись, v - верификация) \n");
			 printf_rus("\n 3) -sa \n Начальный адрес(по умолчанию = 0). \n -sa <начальный_адрес> (вводится в шестнадцатиричной форме с 0x или без)  \n");
			 printf_rus("\n 4) -l \n Объем информационной части(по умолчанию = 0). \n -l <объем_информационной_части> (вводится в шестнадцатиричной форме с 0x или без)\n");
			 printf_rus("\n 5) -mt \n Тип КЗУ. \n -mt <тип_КЗУ> (вводится прописными буквами) \n");
			 printf_rus("\n 6) -mid \n Идентификационный номер КЗУ \n -mid <идентификационный_номер_КЗУ> \n");
			 printf_rus("\n 7) -fn \n Имя файла для чтения, записи и сравнения. \n -fn a\\b\\c\\имя_файла (a, b,c -некоторые каталоги, если путь содержит пробелы его необходимо вводить в кавычках). Если каталог не существует, то он будет создан \n");
			 printf_rus("\n 8) -wa \n Круговая запись(чтение). \n -wa \n");
			 printf_rus("\n 9) -lp \n Путь .log файла. \n -lp a\\b\\c (a, b,c -некоторые каталоги)  \n");
			 printf_rus("\n 10) -h \n Помощь \n -h \n");

			 return NO_ERR;			
		 }
	}//end for

if(lp == 1)
{
	strcat(logpath, "\\");
	strcat(logpath, memid);
	strcat(logpath, ".log");
	cf = fopen(logpath, "at" );
}
else
{
	strcat(memid, ".log");
	cf = fopen(memid, "at" );
}
/*************************************************************************************/
#ifdef __MSDOS__
clrscr();
#endif

fprintf(cf, " BA435_loader_v01 \n");
time_t timee;
time(&timee);

printf("%s", ctime(&timee));
fprintf(cf, "Начало работы:  %s\n", ctime(&timee));
/*********************************Командная строка*********************************/
fprintf(cf, "Командная строка: \n");
for(int ij = 1; ij < argc; ij++)
	fprintf(cf, "%s ", argv[ij]);

fprintf(cf, "\n \n \n");
/*************************************************************************************/
	errorlevel = OpenPort( portName, baudrate, sbit, nbit, parity, cf );
	if (errorlevel != NO_ERR) 
	{
		ClosePort(cf, errorlevel);
		return errorlevel;
	}	
	
	int mt = 0;
	char _k4m[] = "k4m";
	char _k128[] = "k128";

	if(strcmp(memtype, _k4m) == 0) mt = K4M;
	if(strcmp(memtype, _k128) == 0) mt = K128;
	
	errorlevel = IsConnect( cf, mt ); 
	if(errorlevel == TIME_OUT_ERR) 
	{
		fprintf(cf, "Ошибка. Время тайм-аута вышло. \n");		
		ClosePort(cf, errorlevel);
	}
	else
	if (errorlevel != NO_ERR) 
	{
		ClosePort(cf, errorlevel);
	}

	//////////////////////////////////////////////////////////////////	
	switch(Nop)
	{
	case (OP_READ) : { //чтение
								if(mt == K4M)
								{	
									int file;					
									if ((file = open(filename, O_CREAT | O_BINARY | O_WRONLY)) == -1)
									{
										printf_rus("Ошибка! Не удалось открыть файл.\n");
										fprintf(cf, "Ошибка. Не удалось открыть файл. \n");
										errorlevel = OPEN_CREAT_FILE_ERR;
                                        perror("");
										ClosePort(cf, errorlevel);
										return errorlevel;
									}

									errorlevel = k4m_read(addr, llong, wa, errorlevel, file, cf);	
									
									fprintf(cf, "Считано %x байт с начального адреса:%x. Записано в файл %s \n", llong, addr, filename);

									close( file );
								}

								if(mt == K128)
								{
									int file;					
									if ((file = open(filename, O_CREAT | O_BINARY | O_WRONLY)) == -1)
									{
										printf_rus("Ошибка! Не удалось открыть файл.\n");
										fprintf(cf, "Ошибка. Не удалось открыть файл. \n");
										errorlevel = OPEN_CREAT_FILE_ERR;
										ClosePort(cf, errorlevel);
										return errorlevel;
									}

									errorlevel = k128_read(addr, llong, wa, errorlevel, file, cf);											

									fprintf(cf, "Считано %x байт с начального адреса:%x. Записано в файл %s \n", llong, addr, filename);

									close( file );
								}						
		
								break;
				}
	case (OP_WRITE) : {//запись	
								if(mt == K4M)
								{
									int file;					
									if ((file = open(filename, O_CREAT | O_BINARY | O_RDONLY)) == -1)
									{
										printf_rus("Ошибка! Не удалось открыть файл.\n");
										fprintf(cf, "Ошибка. Не удалось открыть файл. \n");
										errorlevel = OPEN_CREAT_FILE_ERR;
										ClosePort(cf, errorlevel);
										return errorlevel;
									}

									llong = filelength(file);

									errorlevel = k4m_write(addr, llong, wa, errorlevel, file, cf);	
																		
									fprintf(cf, "Из файла %s записано %x байт по начальному адресу:%x. \n", filename, llong, addr);

									close( file );
								}

								if(mt == K128)
								{
									int file;					
									if ((file = open(filename, O_CREAT | O_BINARY | O_RDONLY)) == -1)
									{
										printf_rus("Ошибка! Не удалось открыть файл.\n");
										fprintf(cf, "Ошибка. Не удалось открыть файл. \n");
										errorlevel = OPEN_CREAT_FILE_ERR;
										ClosePort(cf, errorlevel);
										return errorlevel;
									}

									llong = filelength(file);

									errorlevel = k128_write(addr, llong, wa, errorlevel, file, cf);											
									
									fprintf(cf, "Из файла %s записано %x байт по начальному адресу:%x. \n", filename, llong, addr);

									close( file );
								}

								break;									
			    }
	case (OP_VERIFY) : {//сравнение
								if(mt == K4M)
								{
									int file;					
									if ((file = open(filename, O_CREAT | O_BINARY | O_RDONLY)) == -1)
									{
										printf_rus("Ошибка! Не удалось открыть файл.\n");
										fprintf(cf, "Ошибка. Не удалось открыть файл. \n");
										errorlevel = OPEN_CREAT_FILE_ERR;
										ClosePort(cf, errorlevel);
										return errorlevel;
									}

									errorlevel = k4m_verify(addr, llong, wa, errorlevel, file, cf);											
																		
									fprintf(cf, "Содержимое файла %s и КЗУ, с адреса %x + %x байт идентичны. \n", filename, addr, llong);

									close( file );
								}

								if(mt == K128)
								{
									int file;					
									if ((file = open(filename, O_CREAT | O_BINARY | O_RDONLY)) == -1)
									{
										printf_rus("Ошибка! Не удалось открыть файл.\n");
										fprintf(cf, "Ошибка. Не удалось открыть файл. \n");
										errorlevel = OPEN_CREAT_FILE_ERR;
										ClosePort(cf, errorlevel);
										return errorlevel;
									}

									errorlevel = k128_verify(addr, llong, wa, errorlevel, file, cf);	

									fprintf(cf, "Содержимое файла %s и КЗУ, с адреса %x + %x байт идентичны. \n", filename, addr, llong);

									close( file );
								}
								break;		
				 }	
	default : 
		{
			printf_rus("Не верная команда. \n");
			fprintf(cf, "Ошибка. Не верная команда. \n");
			errorlevel = COMMAND_ERR;
			ClosePort(cf, errorlevel);
		}
	}
/*************************************************************************************/
ClosePort(cf, errorlevel);
/*************************************************************************************/

return 0;
}