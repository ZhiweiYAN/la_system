/*********************************************************
 *project: Line communication charges supermarket
 *filename: serial_number.c
 *version: 0.1
 *purpose: Reconstruct packet
 *developer: gexiaodan, Xi'an Jiaotong University(Drum Team)
 *data: 2007-1-27
 *********************************************************/
#include "serial_number.h"

/* *************************************************
 *  \brief
 *    Initialize a semaphore and a share memory to save serial number
 * 
 * Function Name: 
 * 		int Init_sem_shm_for_serial_number(void)
 *
 * Input:
 *		none
 *
 * Output:
 *		none
 *
 * Return:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int InitSemShmForSerialNumber(void)
{
	/*variables for semaphore and share memory*/
	int sem_id;
	int *serial_num_array = NULL;
	int i = 0;

	int success = 0;
	
	/*Create a share memory and a semaphore*/
  	sem_id = InitialSem(SEM_SERIAL_FTOK_ID);
   	serial_num_array = (int *)InitialShm(global_par.company_num * sizeof(int), SHM_SERIAL_FTOK_ID);
	
	memset(serial_num_array, 0, global_par.company_num * sizeof(int));
	for (i=0; i<global_par.company_num; i++)
	{
		serial_num_array[i] = 999;
	}
	success = UnmappingShareMem((void *)serial_num_array);/*unmap the share memory*/

	return 1;

}

/* *************************************************
 *  \brief
 *    generate serial number with 16 bit
 *	  format:company_index(2bit)&year(2bit)&month(2bit)&day(2bit)&time(5bit)&inc_num(3bit)
 * 
 * Function Name: 
 * 		int Generate_serial_number(int company_index, char *serial_number)
 *
 * Input: 
 *		int company_index ---> the company index
 *
 * Output:
 *		char *serial_number ---> serial number string 
 *		
 * return:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int GenerateSerialNumber(int company_index, char *serial_number)
{
	int success = 0;
	time_t time1 = 0;
	struct tm *time2 = NULL;
	unsigned int time_second = 0;
	int curr_value = 0;
	char curr_num_string[10] = "00";
	/*variables for semaphore and share memory*/
	int sem_id;
	int *serial_num_array = NULL;

	/*get the now time*/
	time(&time1);
	time2=localtime(&time1);/*converts the calendar time to broken-time representation*/
	time_second = time2->tm_hour * 3600 + time2->tm_min * 60 + time2->tm_sec;/*converts to seconds*/

	sprintf(serial_number, "%02d%02d%02d%02d%05d", company_index, (1900+time2->tm_year)%100, 1+time2->tm_mon, time2->tm_mday, time_second);
	
	sem_id = GetExistedSemphoreExt(SEM_SERIAL_FTOK_ID);
   	serial_num_array = (int *)MappingShareMemOwnSpaceExt(SHM_SERIAL_FTOK_ID);

	/*get the low(3bit) increase value from share memory*/
	success = AcquireAccessRight(sem_id);
	
	serial_num_array[company_index] += global_par.system_par.business_number;
	curr_value = serial_num_array[company_index];
	if (curr_value > 999)
	{
		serial_num_array[company_index] = strtol(global_par.system_par.localhost_id + 1, NULL, 10);
		curr_value = serial_num_array[company_index];
	}
	success = ReleaseAccessRight(sem_id);

	bzero(curr_num_string, 10);
	sprintf(curr_num_string, "%03d", curr_value);
	strcat(serial_number, curr_num_string);

	return 1;

}

/* *************************************************
 *  \brief
 *    reconstruct forward packet which received from client,
 *	we add company_index(2bit), service_type_index(2 bit), 
 *	client_id(8 bit), and charge_money(10 bit)  
 * 
 * Function Name: 
 * 		int Reconstruct_forward_packet(char *packet)
 *
 * Input: 
 *		char *packet ---> the packet needs reconstructing
 *
 * Output:
 *      none
 *		
 * return:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int FillSerialNumberWithinForwardPacket(char *packet, int company_index, int service_type_index, char *la_serial_number)
{
	char serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";

	if(-1 != global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_index[LA_SERIAL_NUMBER_ITEM_INDEX])
	{
		GenerateSerialNumber(company_index ,serial_number);
		FillSerialNumberToPacket(packet, FORWARD_POSITION, company_index, service_type_index, serial_number, la_serial_number);		
	}
	return 1;

}

/* *************************************************
 *  \brief
 *    fill the serial number to packet
 * 
 * Function Name: 
 * 		int Fill_serial_number_to_packet(char *pkt, int pkt_postion, int company_index, int service_type_index, char *serial_number)
 *
 * Input: 
 *		char *pkt ---> the pointer of buffer contained the packet to be filled in
 *      int pkt_postion ---> the direction of packet transmitted (0:forward, 1:backward)
 *      int company_index ---> the index of company
 *		int service_type_index ---> the index of serial type
 *		char *serial_number ---> the serial number to be filled in
 *
 * Output:
 *
 *		
 * return:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int FillSerialNumberToPacket(char *pkt, int pkt_postion, int company_index, int service_type_index, char *serial_number, char *la_serial_number)
{
	int serial_number_item_index = 0;
	int direction = 0;
	int start_pos = 0;
	int len = 0;
	int pkt_len = strlen(pkt);
	int offset = 0;
	
	serial_number_item_index = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_index[LA_SERIAL_NUMBER_ITEM_INDEX];
	direction = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[serial_number_item_index].direction;
	start_pos = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[serial_number_item_index].start_pos;
	len = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[serial_number_item_index].len;

	if((unsigned int)len < strlen(serial_number))
	{	
		offset= strlen(serial_number)-len;		
	}
	if(0 == direction) //items directions, 0: positive; 1:negtive.
	{
		memset(pkt+start_pos, ' ', len);
		memcpy(pkt+start_pos, serial_number+offset, strlen(serial_number)-offset);	
		memcpy(la_serial_number, serial_number+offset, strlen(serial_number)-offset);			
	}
	else
	{
		memset(pkt+pkt_len-start_pos, ' ', len);
		memcpy(pkt+pkt_len-start_pos-1, serial_number+offset, strlen(serial_number)-offset);
		memcpy(la_serial_number, serial_number+offset, strlen(serial_number)-offset);			
	}

	return 1;
}


/* *************************************************
 *  \brief
 *    reconstruct forward packet which received from client,
 *	we add company_index(2bit), service_type_index(2 bit), 
 *	client_id(8 bit), and charge_money(10 bit)  
 * 
 * Function Name: 
 * 		int Reconstruct_forward_packet(char *packet)
 *
 * Input: 
 *		char *packet ---> the packet needs reconstructing
 *
 * Output:
 *      none
 *		
 * return:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int FillRecordDateWithinForwardPacket(char *packet, int company_index, int service_type_index)
{
	int record_date_item_index = 0;
	int direction = 0;
	int start_pos = 0;
	int len = 0;
	int pkt_len = strlen(packet);
	time_t time1 = 0;
	struct tm *time2 = NULL;
	char record_date[9];
	char record_time[7];
	char record_whole_date_time[15];
	
	if(-1 != global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_index[LA_RECORD_DATE_ITEM_INDEX])
	{
		time(&time1);
		time2=localtime(&time1);/*converts the calendar time to broken-time representation*/

		bzero(record_date, 9);
		sprintf(record_date, "%04d%02d%02d", 1900+time2->tm_year, 1+time2->tm_mon, time2->tm_mday);
		bzero(record_time, 7);
		sprintf(record_time, "%02d%02d%02d", time2->tm_hour, time2->tm_min, time2->tm_sec);
		bzero(record_whole_date_time, 15);
		sprintf(record_whole_date_time, "%04d%02d%02d%02d%02d%02d", 1900+time2->tm_year, 1+time2->tm_mon, time2->tm_mday, time2->tm_hour, time2->tm_min, time2->tm_sec);
	
		record_date_item_index = global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_index[LA_RECORD_DATE_ITEM_INDEX];
		direction = global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_par_array[record_date_item_index].direction;
		start_pos = global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_par_array[record_date_item_index].start_pos;
		len = global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_par_array[record_date_item_index].len;

		if((8 != len)&&(6 != len)&&(14 != len))
		{
			printf("The Form of record date is not YYYYMMDD, HHMMSS or YYYYMMDDHHMMSS, you need reconstruction the item of RECORD_DATE\n");
		}
		else
		{
			if(8 == len)
			{
				if(0 == direction) //items directions, 0: positive; 1:negtive.
				{
					memcpy(packet+start_pos, record_date, 8);
				}
				else
				{
					memcpy(packet+pkt_len-start_pos-1, record_date, 8);
				}
			}
			if(6 == len)
			{
				if(0 == direction) //items directions, 0: positive; 1:negtive.
				{
					memcpy(packet+start_pos, record_time, 6);
				}
				else
				{
					memcpy(packet+pkt_len-start_pos-1, record_date, 6);
				}
			}
			if(14 == len)
			{
				if(0 == direction) //items directions, 0: positive; 1:negtive.
				{
					memcpy(packet+start_pos, record_whole_date_time, 14);
				}
				else
				{
					memcpy(packet+pkt_len-start_pos-1, record_whole_date_time, 14);
				}
			}
		}
	}

	if(-1 != global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_index[LA_RECORD_TIME_ITEM_INDEX])
	{
		time(&time1);
		time2=localtime(&time1);/*converts the calendar time to broken-time representation*/

		bzero(record_date, 9);
		sprintf(record_date, "%04d%02d%02d", 1900+time2->tm_year, 1+time2->tm_mon, time2->tm_mday);
		bzero(record_time, 7);
		sprintf(record_time, "%02d%02d%02d", time2->tm_hour, time2->tm_min, time2->tm_sec);
		bzero(record_whole_date_time, 15);
		sprintf(record_whole_date_time, "%04d%02d%02d%02d%02d%02d", 1900+time2->tm_year, 1+time2->tm_mon, time2->tm_mday, time2->tm_hour, time2->tm_min, time2->tm_sec);
	
		record_date_item_index = global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_index[LA_RECORD_TIME_ITEM_INDEX];
		direction = global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_par_array[record_date_item_index].direction;
		start_pos = global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_par_array[record_date_item_index].start_pos;
		len = global_par.company_par_array[company_index].pkt_par_array[service_type_index][FORWARD_POSITION].item_par_array[record_date_item_index].len;

		if((8 != len)&&(6 != len)&&(14 != len))
		{
			printf("The Form of record time is not YYYYMMDD, HHMMSS or YYYYMMDDHHMMSS, you need reconstruction the item of RECORD_TIME\n");
		}
		else
		{
			if(8 == len)
			{
				if(0 == direction) //items directions, 0: positive; 1:negtive.
				{
					memcpy(packet+start_pos, record_date, 8);
				}
				else
				{
					memcpy(packet+pkt_len-start_pos-1, record_date, 8);
				}
			}
			if(6 == len)
			{
				if(0 == direction) //items directions, 0: positive; 1:negtive.
				{
					memcpy(packet+start_pos, record_time, 6);
				}
				else
				{
					memcpy(packet+pkt_len-start_pos-1, record_date, 6);
				}
			}
			if(14 == len)
			{
				if(0 == direction) //items directions, 0: positive; 1:negtive.
				{
					memcpy(packet+start_pos, record_whole_date_time, 14);
				}
				else
				{
					memcpy(packet+pkt_len-start_pos-1, record_whole_date_time, 14);
				}
			}
		}
	}
	
	return 1;

}

