#ifndef __MSDOS__
#include "COMPORT.H"
#include "variable.h"
#include <io.h>
#include <string.h>
#endif

#ifdef __MSDOS__
#include <BA435\COMPORT.H>
#include <BA435\VARIABLE.H>
#endif

#define CRC_POLY 0x1021 
char buf[WORK_SIZE + 1];
char priem[0x90];

int sector[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int CRC16_bit(unsigned short int crc, char* msg, unsigned long msg_len)
  
{   
     unsigned short int j,data;
     
     while(msg_len-->0){
         data=*msg++ << 8;//данные в старший байт
	 for(j=0;j<8;++j){
	 
	     if((data^crc)& 0x8000)
	         crc=(crc<<1)^CRC_POLY;
	     else
	         crc=crc<<1;
		 
	     data<<=1;
	  }
      }
return (crc&0xffff);
}
//----------------------------------------------------------------
int IsConnect( FILE *file, int mtt )
{
	int mt;
	int errorlevel;

	char *priemTemp = &priem[0];
	int N;

		*(priemTemp + 0) = 0x3;
		*(priemTemp + 1) = 0x10;
		*(priemTemp + 2) = 0x62;
		*(priemTemp + 3) = 0x47;

		N = 4;

		    errorlevel = write_COM(priem, N);
			if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;

		N = 8;

			errorlevel = read_COM(priemTemp, N);
			if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;

			mt = *(priem+2);
			mt&=0xf;

			if( mt == K4M)
			{
				printf_rus("Подключено K-4M. \n");
				if( mtt == K4M )
				{
					fprintf(file, "Подключено устройство K-4M. \n");
					return NO_ERR;
				}
				else
				{
					printf_rus("Ошибка, введен не верный тип КЗУ. \n ");
					fprintf(file, "Ошибка, введен не верный тип КЗУ. \n");
					return KZU_CONECTED_ERR;
				}
			}
			else
				if(mt == K128)
				{
					printf_rus("Подключено K-128. \n");
					if( mtt == K128 )
					{
						fprintf(file, "Подключено устройство K-128. \n");
						return NO_ERR;
					}
					else
					{
						printf_rus("Ошибка, введен не верный тип КЗУ. \n");
						fprintf(file, "Ошибка, введен не верный тип КЗУ. \n");
						return KZU_CONECTED_ERR;
					}
				}
			else printf_rus("Ничего не подключено. \n");
return NO_ERR;
};
//----------------------------------------------------------------
int EraseSector(unsigned long addr)
{
	unsigned long N;
	char *zapros = &priem[0];
	unsigned short int crc;
	int errorlevel;

	*zapros = 0x06;
	*(zapros + 1) = 0x07;
	*(zapros + 2) = 00;
	*(zapros + 3) = 00;
	*(zapros + 4) = (char) ((addr>>12)&0xff)/0x4;

	sector[((addr>>12)&0xff)/0x4] = 1;

	crc = 0;
	crc = CRC16_bit(crc, zapros, 5 );
	*(zapros + 5) = crc&0xff;
	*(zapros + 6) = (crc>>8)&0xff;

	N = 7;
	errorlevel = write_COM(zapros, N);
	if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;

	N = 4;
    register int tm = -32000;
    while(priem[1] != 0x13)
    {
    	if(tm++ > 0)
        	return TIME_OUT;
		errorlevel = read_COM(zapros, N);
		if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;
    }

	if(priem[1] == 0x13)
			printf("бeЄвoа %x бвсав.\n", ((addr>>12)&0xff)/0x4);//сектор %x стерт

			else
			{
				printf_rus("Ошибка! Сектор ");
				printf("%x", ((addr>>12)&0xff)/0x4);	printf_rus(" не стерт. \n");

                printf(" %x  %x  %x  %x \n", priem[0], priem[1], priem[2], priem[3]);
			return ERASE_SECTOR_ERR;
			}

return NO_ERR;
}
//----------------------------------------------------------------
int readKZU(unsigned long addr, unsigned long size, int file)
{
	unsigned long N;
	char *zapros = &priem[0];
	unsigned short int crc;
	int errorlevel;

	*(zapros + 0) = 0x7;
	*(zapros + 1) = 0x1;
	*(zapros + 2) = (char) addr&0xff;
	*(zapros + 3) = (char) (addr>>8)&0xff;
	*(zapros + 4) = (char) (addr>>16)&0xff;
	*(zapros + 5) = (char) size;

	crc = 0;
	crc = CRC16_bit(crc, zapros, 6);

	*(zapros + 6) = crc&0xff;
	*(zapros + 7) = (crc>>8)&0xff;

	N = 8;
	errorlevel = write_COM(zapros, N);
	if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;


	N = size + 0x4;
	errorlevel = read_COM(zapros, N);
	if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;

	unsigned long k = 0;
    N = size + 0x4;
	for(k = 2; k < N - 2; k++) // без КК и КС
	buf[k - 2] = priem[k];

	N = 0;
	if((N = write(file, buf, size)) != size)
	{
		printf_rus("Ошибка! Запись в файл не удалась.\n");
		return WRITE_FILE_ERR;
	}
return NO_ERR;
}
//----------------------------------------------------------------
int writeKZU(unsigned long addr, unsigned long size, int file)
{
	unsigned long N;
	char *zapros = &priem[0];
	unsigned short int crc;
	int errorlevel;

	*zapros = (char) size + 0x6;//ИЧ + КК(1) + адрес(3) + КС(2)
	*(zapros + 1) = 0x04;
	*(zapros + 2) = (char) addr&0xff;
	*(zapros + 3) = (char) (addr>>8)&0xff;
	*(zapros + 4) = (char) (addr>>16)&0xff;

	N = 0;
	if((N = read(file, buf, size)) != size)
	{
		printf_rus("Ошибка! Не удалось читать из файла.\n");
		return 9;
	}

	for(unsigned long i = 0; i < size; priem[i + 5] = buf[i], i++);

	crc = 0;
	crc = CRC16_bit(crc, zapros, size + 0x5);

	*(zapros + (0x5 + size)) = crc&0xff;
	*(zapros + (0x6 + size)) = (crc>>8)&0xff;

	N = size + 7;
	errorlevel = write_COM(zapros, N);
	if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;

	N = 4;
	errorlevel = read_COM(zapros, N);
	if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;

	if(*(zapros + 1) == 0x13)
	{
		return NO_ERR;
	}
	else
	//if(*(zapros + 1) == 0x16)
	{
		printf_rus("Ошибка! Не удалось записать в КЗУ.\n");
		return WRITE_KZU_ERR;
	}
//return NO_ERR;
}
//----------------------------------------------------------------
int verifyKZU(unsigned long addr, unsigned long size, int file, unsigned long st)
{
	unsigned long N;
	char *zapros = &priem[0];
	unsigned short int crc;
	int errorlevel;

	*(zapros + 0) = 0x7;
	*(zapros + 1) = 0x1;
	*(zapros + 2) = (char) addr&0xff;
	*(zapros + 3) = (char) (addr>>8)&0xff;
	*(zapros + 4) = (char) (addr>>16)&0xff;
    *(zapros + 5) = (char) size;

	crc = 0;
	crc = CRC16_bit(crc, zapros, 6);

	*(zapros + 6) = crc&0xff;
	*(zapros + 7) = (crc>>8)&0xff;

	N = 8;
	errorlevel = write_COM(zapros, N);
	if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;

	N = size + 0x4;
	errorlevel = read_COM(zapros, N);
	if(errorlevel == TIME_OUT_ERR) return TIME_OUT_ERR;

	N = 0;
	if((N = read(file, buf, size)) != size)
	{
		printf_rus("Ошибка! Не удалось читать из файла.\n");
		return (-READ_FILE_ERR);
	}
    unsigned long k = 0;
	N = size + 0x4;
	for(k = 2; k < N - 2; k++) // без КК и КС
	{
		if(buf[k - 2] != priem[k])
		{
			printf_rus("Не сравнение: ");
			printf("%d \n", (k - 1) + st);
			return ((k - 1) + st);//12;
		}
	}
return NO_ERR;
}
//----------------------------------------------------------------
int end_of_sector(unsigned long addr, unsigned long llong, int wa, int file, FILE *cf, unsigned long verify_temp, int mt, int op)
{
	int errorlevel;

	switch (mt)
	{
	case (K4M) :
		{
			switch (op)
			{
				case (OP_READ) :
				{
					unsigned long delta = (K4M_MAX_SIZE - addr);

					errorlevel = readKZU(addr, delta, file);
					if(errorlevel == WRITE_FILE_ERR) 
					{
						fprintf(cf, "Ошибка. Не удалось записать в файл. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}	
					else
					if(errorlevel == TIME_OUT_ERR) 
					{
						fprintf(cf, "Ошибка. Время тайм-аута вышло. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}

					llong -= delta;
					addr += delta;

					if((addr == (K4M_MAX_SIZE + 1)) && wa) addr = 0x0;

					break;
				}
				case (OP_WRITE) :
				{
					unsigned long delta = (K4M_MAX_SIZE - addr);

					errorlevel = writeKZU(addr, delta, file);
					addr += delta;

					if(errorlevel == READ_FILE_ERR)
					{
						fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}
					if(errorlevel == WRITE_KZU_ERR) 
					{
						fprintf(cf, "Ошибка записи в КЗУ.\n");
						close(file);
						ClosePort(cf, errorlevel);
					}						
					else
					if(errorlevel == TIME_OUT_ERR) 
					{
						fprintf(cf, "Ошибка. Время тайм-аута вышло. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}

					llong -= delta;
					if((addr == (K4M_MAX_SIZE + 1)) && wa) addr = 0x0;

					break;
				}
				case (OP_VERIFY) :
				{
					unsigned long delta = (K4M_MAX_SIZE - addr);

					errorlevel = verifyKZU(addr, delta, file, verify_temp);

					if(errorlevel == -READ_FILE_ERR)
					{
						fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}
					if(errorlevel != NO_ERR) 
					{
						fprintf(cf, "Не сравнение в %d -м символе. \n", errorlevel);	
	 					close(file);
						errorlevel = VERIFY_ERR;
						ClosePort(cf, errorlevel);
					}
					else
					if(errorlevel == TIME_OUT_ERR) 
					{
						fprintf(cf, "Ошибка. Время тайм-аута вышло. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}

					verify_temp += delta;
					llong -= delta;
					addr += delta;
			
					if((addr == (K4M_MAX_SIZE + 1)) && wa) addr = 0x0;

					break;
				}
			}
			break;
		}
	case (K128) :
		{			
			switch (op)
			{
				case (OP_READ) :
				{					
					unsigned long Ns = 0;
					Ns = ((addr>>12)&0xff)/0x4;
					unsigned long delta = (((K128_SECTOR_SIZE - 1) + K128_SECTOR_SIZE * Ns) - addr);

					errorlevel = readKZU(addr, delta, file);

					if(errorlevel == WRITE_FILE_ERR) 
					{
						fprintf(cf, "Ошибка. Не удалось записать в файл. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}						
					else
					if(errorlevel == TIME_OUT_ERR) 
					{
						fprintf(cf, "Ошибка. Время тайм-аута вышло. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}

					llong -= delta;
					addr += delta;

					if(addr == ((K128_SECTOR_SIZE - 1) + K128_SECTOR_SIZE * Ns))  addr += 0x1;
					if((addr == (K128_MAX_SIZE + 1)) && wa) addr = 0x0;

					break;
				}
				case (OP_WRITE) :
				{
					unsigned long Ns = 0;
					Ns = ((addr>>12)&0xff)/0x4;
					unsigned long delta = (K128_MAX_SIZE - addr);
										
					if( sector[((addr>>12)&0xff)/0x4] == 0 ) 
					{
						errorlevel = EraseSector(addr);

						if(errorlevel == ERASE_SECTOR_ERR)
						{
							fprintf(cf, "Ошибка. Сектор %x не стерт.\n", ((addr>>12)&0xff)/0x4);
							return errorlevel;
						}
						else
						if(errorlevel == TIME_OUT_ERR) 
						{
							fprintf(cf, "Ошибка. Время тайм-аута вышло. \n");
							close(file);
							ClosePort(cf, errorlevel);
						}

						fprintf(cf, "Сектор %x успешно стерт. \n", ((addr>>12)&0xff)/0x4);
					}//end if

					errorlevel = writeKZU(addr, delta, file);
					addr += delta;

					if(errorlevel == READ_FILE_ERR)
					{
							fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
							close(file);
							ClosePort(cf, errorlevel);
					}
					if(errorlevel == WRITE_KZU_ERR) 
					{
							fprintf(cf, "Ошибка записи в КЗУ.\n");
							close(file);
							ClosePort(cf, errorlevel);
					}		
					if(errorlevel == TIME_OUT_ERR) 
					{
						fprintf(cf, "Ошибка. Время тайм-аута вышло. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}

					llong -= delta;
					if(addr == ((K128_SECTOR_SIZE - 1) + K128_SECTOR_SIZE * Ns))  addr += 0x1;
					if((addr == (K128_MAX_SIZE + 1)) && wa) addr = 0x0;

					break;
				}
				case (OP_VERIFY) :
				{
					unsigned long Ns = 0;
					Ns = ((addr>>12)&0xff)/0x4;
					unsigned long delta = (K128_MAX_SIZE - addr);

					errorlevel = verifyKZU(addr, delta, file, verify_temp);

					if(errorlevel == -READ_FILE_ERR)
					{
						fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}
					if(errorlevel != NO_ERR) 
					{
						fprintf(cf, "Не сравнение в %d -м символе. \n", errorlevel);	
	 					close(file);
						errorlevel = VERIFY_ERR;
						ClosePort(cf, errorlevel);
					}
					if(errorlevel == TIME_OUT_ERR) 
					{
						fprintf(cf, "Ошибка. Время тайм-аута вышло. \n");
						close(file);
						ClosePort(cf, errorlevel);
					}

					verify_temp += delta;
					llong -= delta;
					addr += delta;

					if(addr == ((K128_SECTOR_SIZE - 1) + K128_SECTOR_SIZE * Ns))  addr += 0x1;
					if((addr == K128_MAX_SIZE + 1) && wa) addr = 0x0;

					break;
				}
			}
			break;
		}
	}

return errorlevel;
}
//----------------------------------------------------------------
int check_size(unsigned long addr, unsigned long llong, unsigned long max_size, int wa)
{
	if(llong > max_size)
	{
		printf_rus("Ошибка. Объем информационной части больше объема КЗУ. \n");	
		return WORK_SIZE_ERR;
	}

	if((addr + llong > max_size) && !wa)
	{
		printf_rus("Ошибка. Объем информационной части больше объема КЗУ. \n");	
		return WORK_SIZE_ERR;
	}
return NO_ERR;
}
//----------------------------------------------------------------
int k128_read(unsigned long addr, unsigned long llong, int wa, int errorlevel, int file, FILE *cf)
{
	unsigned long Ns = 0; //N of sector
	
	errorlevel = check_size(addr, llong, K128_MAX_SIZE, wa);
	if(errorlevel == WORK_SIZE_ERR)
	{
		fprintf(cf, "Ошибка. Объем информационной части больше объема КЗУ.\n");
		close(file);
		ClosePort(cf, errorlevel);
	}

	while(llong > WORK_SIZE)
	{
/*****************************************************************/
		Ns = ((addr>>12)&0xff)/0x4;
		if( (addr > ((K128_SECTOR_SIZE - WORK_SIZE) + K128_SECTOR_SIZE * Ns)) && (addr < K128_SECTOR_SIZE + K128_SECTOR_SIZE * Ns))//if{}
		{
			end_of_sector(addr, llong, wa, file, cf, 0, K128, OP_READ);

		}//end if{}
	/*****************************************************************/
		errorlevel = readKZU(addr, WORK_SIZE, file);
		if(errorlevel == WRITE_FILE_ERR) 
		{
			fprintf(cf, "Ошибка. Не удалось записать в файл. \n");
			close(file);
			ClosePort(cf, errorlevel);
		}						

			llong -= WORK_SIZE;
			addr += WORK_SIZE;

			if((addr == K128_MAX_SIZE) && wa) addr = 0x0;
	}//end while

	/*****************************************************************/
	Ns = ((addr>>12)&0xff)/0x4;							
	if( (addr > ((K128_SECTOR_SIZE - WORK_SIZE) + K128_SECTOR_SIZE * Ns)) && (addr < K128_SECTOR_SIZE + K128_SECTOR_SIZE * Ns))//if{}
	{
		end_of_sector(addr, llong, wa, file, cf, 0, K128, OP_READ);

	}//end if{}
/*****************************************************************/

	errorlevel = readKZU(addr, llong, file);
	if(errorlevel == WRITE_FILE_ERR) 
	{
		fprintf(cf, "Ошибка. Не удалось записать в файл. \n");
		close(file);
		ClosePort(cf, errorlevel);
	}

return NO_ERR;
}//end k128_read
//----------------------------------------------------------------
int k128_write(unsigned long addr, unsigned long llong, int wa, int errorlevel, int file, FILE *cf)
{		
	unsigned long Ns = 0; //N of sector
					
	errorlevel = check_size(addr, llong, K128_MAX_SIZE, wa);
	if(errorlevel == WORK_SIZE_ERR)
	{
		fprintf(cf, "Ошибка. Объем информационной части больше объема КЗУ.\n");
		close(file);
		ClosePort(cf, errorlevel);
	}				
								
	while(llong > WORK_SIZE)
	{	
/*****************************************************************/
		Ns = ((addr>>12)&0xff)/0x4;
		if( (addr > ((K128_SECTOR_SIZE - WORK_SIZE) + K128_SECTOR_SIZE * Ns)) && (addr < K128_SECTOR_SIZE + K128_SECTOR_SIZE * Ns))//if{}
		{
			end_of_sector(addr, llong, wa, file, cf, 0, K128, OP_WRITE);
			
		}//end if{}
/*****************************************************************/

		if( sector[((addr>>12)&0xff)/0x4] == 0 ) 
		{
			errorlevel = EraseSector(addr);

			if(errorlevel == ERASE_SECTOR_ERR)
			{
				fprintf(cf, "Ошибка. Сектор %x не стерт.\n", ((addr>>12)&0xff)/0x4);
				close(file);
				ClosePort(cf, errorlevel);
			}				
			fprintf(cf, "Сектор %x успешно стерт. \n", ((addr>>12)&0xff)/0x4);
		}//end if

		errorlevel = writeKZU(addr, WORK_SIZE, file);

		if(errorlevel == READ_FILE_ERR)
		{
			fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
			close(file);
			ClosePort(cf, errorlevel);
		}
		if(errorlevel == WRITE_KZU_ERR) 
		{
			fprintf(cf, "Ошибка записи в КЗУ.\n");
			close(file);
			ClosePort(cf, errorlevel);
		}							

		llong -= WORK_SIZE;
		addr += WORK_SIZE;	

		if((addr == K128_MAX_SIZE) && wa) addr = 0x0;
	}//end while

/*****************************************************************/
	Ns = ((addr>>12)&0xff)/0x4;
	if( (addr > ((K128_SECTOR_SIZE - WORK_SIZE) + K128_SECTOR_SIZE * Ns)) && (addr < K128_SECTOR_SIZE + K128_SECTOR_SIZE * Ns))//if{}
	{
		end_of_sector(addr, llong, wa, file, cf, 0, K128, OP_WRITE);

	}//end if{}
/*****************************************************************/

	if( sector[((addr>>12)&0xff)/0x4] == 0 )
	{
		errorlevel = EraseSector(addr);

		if(errorlevel == ERASE_SECTOR_ERR)
		{
			fprintf(cf, "Ошибка. Сектор %x не стерт.\n", ((addr>>12)&0xff)/0x4);
			close(file);
			ClosePort(cf, errorlevel);
		}
		fprintf(cf, "Сектор %x успешно стерт. \n", ((addr>>12)&0xff)/0x4);
	}//end if

	errorlevel = writeKZU(addr, llong, file);

	if(errorlevel == READ_FILE_ERR)
	{
		fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
		close(file);
		ClosePort(cf, errorlevel);
	}
	if(errorlevel == WRITE_KZU_ERR) 
	{
		fprintf(cf, "Ошибка записи в КЗУ.\n");
		close(file);
		ClosePort(cf, errorlevel);
	}	
return NO_ERR;
}
//----------------------------------------------------------------
int k128_verify(unsigned long addr, unsigned long llong, int wa, int errorlevel, int file, FILE *cf)
{
	unsigned long Ns = 0; //N of sector

	llong = filelength(file);

    errorlevel = check_size(addr, llong, K128_MAX_SIZE, wa);
	if(errorlevel == WORK_SIZE_ERR)
	{
		fprintf(cf, "Ошибка. Объем информационной части больше объема КЗУ.\n");
		close(file);
		ClosePort(cf, errorlevel);
	}

	unsigned long temp = 0;

	while(llong > WORK_SIZE)
	{
/*****************************************************************/
		Ns = ((addr>>12)&0xff)/0x4;
		if( (addr > ((K128_SECTOR_SIZE - WORK_SIZE) + K128_SECTOR_SIZE * Ns)) && (addr < K128_SECTOR_SIZE + K128_SECTOR_SIZE * Ns))//if{}
		{
			end_of_sector(addr, llong, wa, file, cf, temp, K128, OP_VERIFY);

		}//end if{}
	/*****************************************************************/
		errorlevel = verifyKZU(addr, WORK_SIZE, file, temp);

		if(errorlevel == -READ_FILE_ERR)
		{
			fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
			close(file);
			ClosePort(cf, errorlevel);
		}
		if(errorlevel != NO_ERR) 
		{
			fprintf(cf, "Не сравнение в %d -м символе. \n", errorlevel);	
	 		close(file);
			errorlevel = VERIFY_ERR;
			ClosePort(cf, errorlevel);
		}

		temp += WORK_SIZE;
		llong -= WORK_SIZE;
		addr += WORK_SIZE;

		if((addr == K128_MAX_SIZE) && wa) addr = 0x0;
	}//end while

/*****************************************************************/
	Ns = ((addr>>12)&0xff)/0x4;
	if( (addr > ((K128_SECTOR_SIZE - WORK_SIZE) + K128_SECTOR_SIZE * Ns)) && (addr < K128_SECTOR_SIZE + K128_SECTOR_SIZE * Ns))//if{}
	{
		end_of_sector(addr, llong, wa, file, cf, temp, K128, OP_VERIFY);

	}//end if{}
/*****************************************************************/								
	errorlevel = verifyKZU(addr, llong, file, temp);
								
	if(errorlevel == -READ_FILE_ERR)
	{
		fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
		close(file);
		ClosePort(cf, errorlevel);
	}
	if(errorlevel != NO_ERR) 
	{
		fprintf(cf, "Не сравнение в %d -м символе. \n", errorlevel);	
	 	close(file);
		errorlevel = VERIFY_ERR;
		ClosePort(cf, errorlevel);
	}

return 0;
}
//----------------------------------------------------------------
int k4m_write(unsigned long addr, unsigned long llong, int wa, int errorlevel, int file, FILE *cf)
{
	errorlevel = check_size(addr, llong, K4M_MAX_SIZE, wa);
	if(errorlevel == WORK_SIZE_ERR)
	{
		fprintf(cf, "Ошибка. Объем информационной части больше объема КЗУ.\n");
		close(file);
		ClosePort(cf, errorlevel);
	} 
								
	while(llong > WORK_SIZE)
	{	
/*****************************************************************/
		if( (addr > (K4M_MAX_SIZE - WORK_SIZE)) && (addr < K4M_MAX_SIZE))//if{}
		{
			end_of_sector(addr, llong, wa, file, cf, 0, K4M, OP_WRITE);
			
		}//end if{}
/*****************************************************************/		

		errorlevel = writeKZU(addr, WORK_SIZE, file);

		if(errorlevel == READ_FILE_ERR)
		{
			fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
			close(file);
			ClosePort(cf, errorlevel);
		}
		if(errorlevel == WRITE_KZU_ERR) 
		{
			fprintf(cf, "Ошибка записи в КЗУ.\n");
			close(file);
			ClosePort(cf, errorlevel);
		}							

		llong -= WORK_SIZE;
		addr += WORK_SIZE;	

		if((addr == K4M_MAX_SIZE) && wa) addr = 0x0;
	}//end while

/*****************************************************************/
	if( (addr > (K4M_MAX_SIZE - WORK_SIZE)) && (addr < K4M_MAX_SIZE))//if{}
		{
			end_of_sector(addr, llong, wa, file, cf, 0, K4M, OP_WRITE);

		}//end if{}
/*****************************************************************/	

	errorlevel = writeKZU(addr, llong, file);

	if(errorlevel == READ_FILE_ERR)
	{
		fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
		close(file);
		ClosePort(cf, errorlevel);
	}
	if(errorlevel == WRITE_KZU_ERR) 
	{
		fprintf(cf, "Ошибка записи в КЗУ.\n");
		close(file);
		ClosePort(cf, errorlevel);
	}		

return 0;//end k128_write
}
//----------------------------------------------------------------
int k4m_read(unsigned long addr, unsigned long llong, int wa, int errorlevel, int file, FILE *cf)
{		
	errorlevel = check_size(addr, llong, K4M_MAX_SIZE, wa);
	if(errorlevel == WORK_SIZE_ERR)
	{
		fprintf(cf, "Ошибка. Объем информационной части больше объема КЗУ.\n");
		close(file);
		ClosePort(cf, errorlevel);
	} 

	while(llong > WORK_SIZE)
	{
/*****************************************************************/
		if( (addr > (K4M_MAX_SIZE - WORK_SIZE)) && (addr < K4M_MAX_SIZE))//if{}
		{
			end_of_sector(addr, llong, wa, file, cf, 0, K4M, OP_READ);
			
		}//end if{}
	/*****************************************************************/
		errorlevel = readKZU(addr, WORK_SIZE, file);
		if(errorlevel == WRITE_FILE_ERR) 
		{
			fprintf(cf, "Ошибка. Не удалось записать в файл. \n");
			close(file);
			ClosePort(cf, errorlevel);
		}	

			llong -= WORK_SIZE;
			addr += WORK_SIZE;								

			if((addr == K4M_MAX_SIZE) && wa) addr = 0x0;
	}//end while

	/*****************************************************************/			
	if( (addr > (K4M_MAX_SIZE - WORK_SIZE)) && (addr < K4M_MAX_SIZE))//if{}
	{
		end_of_sector(addr, llong, wa, file, cf, 0, K4M, OP_READ);

	}//end if{}
/*****************************************************************/

	errorlevel = readKZU(addr, llong, file);
	if(errorlevel == WRITE_FILE_ERR) 
	{
		fprintf(cf, "Ошибка. Не удалось записать в файл. \n");
		close(file);
		ClosePort(cf, errorlevel);
	}	
return NO_ERR;
}//end k4m_read
//----------------------------------------------------------------
int k4m_verify(unsigned long addr, unsigned long llong, int wa, int errorlevel, int file, FILE *cf)
{
	llong = filelength(file);

    errorlevel = check_size(addr, llong, K4M_MAX_SIZE, wa);
	if(errorlevel == WORK_SIZE_ERR)
	{
		fprintf(cf, "Ошибка. Объем информационной части больше объема КЗУ.\n");
		close(file);
		ClosePort(cf, errorlevel);
	} 

	unsigned long temp = 0;

	while(llong > WORK_SIZE)
	{
/*****************************************************************/
		if( (addr > (K4M_MAX_SIZE - WORK_SIZE)) && (addr < K4M_MAX_SIZE))//if{}
		{
			end_of_sector(addr, llong, wa, file, cf, temp, K4M, OP_VERIFY);
			
		}//end if{}
	/*****************************************************************/
		errorlevel = verifyKZU(addr, WORK_SIZE, file, temp);

		if(errorlevel == READ_FILE_ERR)
		{
			fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
			close(file);
			ClosePort(cf, errorlevel);
		}
		if(errorlevel != NO_ERR) 
		{
			fprintf(cf, "Не сравнение в %d -м символе. \n", errorlevel);	
	 		close(file);
			errorlevel = VERIFY_ERR;
			ClosePort(cf, errorlevel);
		}

		temp += WORK_SIZE;
		llong -= WORK_SIZE;
		addr += WORK_SIZE;
									
		if((addr == K4M_MAX_SIZE) && wa) addr = 0x0;
	}//end while

/*****************************************************************/
	if( (addr > (K4M_MAX_SIZE - WORK_SIZE)) && (addr < K4M_MAX_SIZE))//if{}
	{
		end_of_sector(addr, llong, wa, file, cf, temp, K4M, OP_VERIFY);

	}//end if{}
/*****************************************************************/								
	errorlevel = verifyKZU(addr, llong, file, temp);
								
	if(errorlevel == -READ_FILE_ERR)
	{
		fprintf(cf, "Ошибка. Не удалось читать из файла. \n");
		close(file);
		ClosePort(cf, errorlevel);
	}
	if(errorlevel != NO_ERR) 
	{
		fprintf(cf, "Не сравнение в %d -м символе. \n", errorlevel);	
	 	close(file);
		errorlevel = VERIFY_ERR;
		ClosePort(cf, errorlevel);
	}
return 0;
}//end k4m_verify
//----------------------------------------------------------------