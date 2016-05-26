/*********************************************************
 *project: Line communication charges supermarket
 *filename: common.c
 *version: 0.6
 *purpose: common data structure and operation about them
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-14
 *********************************************************/
 
 #include "common.h"
 
/*************************************************************************
 *  \brief
 *    save the process pid of data channel to the share memory
 *    the destination is determinated by semphore
 *
 *    1. get the semphore.
 *    2. find a empty slot at the array.
 *    3. put the own PID to this slot.
 *    4. release the semphore.
 * 	
 *  \par Input:
 *    sem_key: the semphore identified the array 
 *    shm_key: the share memory identified the array 
 *
 *  \par Output:
 *
 *  \Return:
 * 	1 : success
 * 	0 : error
************************************************************************/
int SavePidToArray()
{
	/*variable for semaphore and share memory*/
	int sem_id;
	struct Queue_business_process *process_info;
	
	int success = 0;
	int i = 0;
	pid_t pid = getpid();
		
	sem_id = GetExistedSemphoreExt(SEM_DATA_CHAN_FTOK_ID);
  	process_info = (struct Queue_business_process *)MappingShareMemOwnSpaceExt(SHM_DATA_CHAN_FTOK_ID);
	success = AcquireAccessRight(sem_id);
	//for (i=0; i<NUM_PROCESS_QUEUE; i++)
	for (i=0; i<800; i++)
	{
		if (0 == process_info[i].pid)
		{
			process_info[i].pid = pid;
			process_info[i].life_time = 0;
			//printf("save data process PID : %d to Queue_business_process[%d] which identified by key : %d!!!\n", pid, i, shm_key);
			break;
		}
	}			
	success = ReleaseAccessRight(sem_id);	
	success = UnmappingShareMem((void *)process_info);
	if (NUM_PROCESS_QUEUE == i)
	{
   		printf("\033[01;31mcommon.c:SavePidToArray:space not enough\033[0m\n");
	}

	return 1;
}


/*************************************************************************
 *  \brief
 *    �����ɵ���ˮ�ţ��ն˺ţ�����ʱ�����Ϣ��¼������ע�������
 *
 *    1. get the semphore.
 *    2. find a slot belonged to this pid.
 *    3. put the infos into this slot.
 *    4. release the semphore.
 * 	
 *  \par Output:
 *
 *  \Return:
 * 	1 : success
 * 	0 : error
************************************************************************/
int RegisterInfoToArray(int company_index, int service_type_index, unsigned long int charge_money, char *la_serial_number, char *terminal_id)
{
	/*variable for semaphore and share memory*/
	int sem_id;
	struct Queue_business_process *process_info;
	time_t time1 = 0;
	struct tm *time2 = NULL;
	
	int success = 0;
	int i = 0;
	pid_t pid = getpid();
		
	sem_id = GetExistedSemphoreExt(SEM_DATA_CHAN_FTOK_ID);
  	process_info = (struct Queue_business_process *)MappingShareMemOwnSpaceExt(SHM_DATA_CHAN_FTOK_ID);
	success = AcquireAccessRight(sem_id);
	for (i=0; i<NUM_PROCESS_QUEUE; i++)
	//for (i=0; i<800; i++)
	{
		if (pid == process_info[i].pid)
		{
			/*�ҵ������pid*/
			process_info[i].step_flag= 1; /*��Ҫ���͸����Ź�˾���н���*/
			process_info[i].company_index = company_index;
			process_info[i].service_type_index = service_type_index;
			process_info[i].charge_money = charge_money;
			bzero(process_info[i].la_serial_number, SERIAL_NUMBER_LENGTH + 1);
			strcpy(process_info[i].la_serial_number, la_serial_number);
			bzero(process_info[i].terminal_id, CLIENT_ID_INDEX_LENGTH_AT_HEADER+1);
			strcpy(process_info[i].terminal_id, terminal_id);
			/*�õ�ҵ������ʱ��*/
			time(&time1);
			time2=localtime(&time1);/*converts the calendar time to broken-time representation*/
			bzero(process_info[i].affair_generate_date, 9);
			sprintf(process_info[i].affair_generate_date, "%04d%02d%02d", (1900+time2->tm_year), 1+time2->tm_mon, time2->tm_mday);
			bzero(process_info[i].affair_generate_time, 9);
			sprintf(process_info[i].affair_generate_time, "%02d:%02d:%02d", time2->tm_hour, time2->tm_min, time2->tm_sec);
			//printf("save data process PID : %d to Queue_business_process[%d] which identified by key : %d!!!\n", pid, i, shm_key);
			break;
		}
	}			
	success = ReleaseAccessRight(sem_id);	
	success = UnmappingShareMem((void *)process_info);
	if (NUM_PROCESS_QUEUE == i)
	{
   		printf("\033[01;31mcommon.c:RegisterInfoToArray:no such pid info slot!\033[0m\n");
	}

	return 1;
}



/*************************************************************************
 *  \brief
 *    �����ɵ���ˮ�ţ��ն˺ţ�����ʱ�����Ϣ��¼������ע�������
 *
 *    1. get the semphore.
 *    2. find a slot belonged to this pid.
 *    3. put the infos into this slot.
 *    4. release the semphore.
 * 	
 *  \par Output:
 *
 *  \Return:
 * 	1 : success
 * 	0 : error
************************************************************************/
int RegisterInfoToArrayEx(int company_index, int service_type_index, unsigned long int charge_money, char *la_serial_number, char *terminal_id, char *phonenumber)
{
	/*variable for semaphore and share memory*/
	int sem_id;
	struct Queue_business_process *process_info;
	time_t time1 = 0;
	struct tm *time2 = NULL;
	
	int success = 0;
	int i = 0;
	pid_t pid = getpid();
		
	sem_id = GetExistedSemphoreExt(SEM_DATA_CHAN_FTOK_ID);
  	process_info = (struct Queue_business_process *)MappingShareMemOwnSpaceExt(SHM_DATA_CHAN_FTOK_ID);
	success = AcquireAccessRight(sem_id);
	for (i=0; i<NUM_PROCESS_QUEUE; i++)
	//for (i=0; i<800; i++)
	{
		if (pid == process_info[i].pid)
		{
			/*�ҵ������pid*/
			process_info[i].step_flag= 1; /*��Ҫ���͸����Ź�˾���н���*/
			process_info[i].company_index = company_index;
			process_info[i].service_type_index = service_type_index;
			process_info[i].charge_money = charge_money;
			bzero(process_info[i].la_serial_number, SERIAL_NUMBER_LENGTH + 1);
			strcpy(process_info[i].la_serial_number, la_serial_number);
			bzero(process_info[i].terminal_id, CLIENT_ID_INDEX_LENGTH_AT_HEADER+1);
			strcpy(process_info[i].terminal_id, terminal_id);
			/*�õ�ҵ������ʱ��*/
			time(&time1);
			time2=localtime(&time1);/*converts the calendar time to broken-time representation*/
			bzero(process_info[i].affair_generate_date, 9);
			sprintf(process_info[i].affair_generate_date, "%04d%02d%02d", (1900+time2->tm_year), 1+time2->tm_mon, time2->tm_mday);
			bzero(process_info[i].affair_generate_time, 9);
			sprintf(process_info[i].affair_generate_time, "%02d:%02d:%02d", time2->tm_hour, time2->tm_min, time2->tm_sec);
			strcpy(process_info[i].phonenumber, phonenumber);
			//printf("save data process PID : %d to Queue_business_process[%d] which identified by key : %d!!!\n", pid, i, shm_key);
			break;
		}
	}			
	success = ReleaseAccessRight(sem_id);	
	success = UnmappingShareMem((void *)process_info);
	if (NUM_PROCESS_QUEUE == i)
	{
   		printf("\033[01;31mcommon.c:RegisterInfoToArray:no such pid info slot!\033[0m\n");
	}

	return 1;
}

/*************************************************************************
 *  \brief
 *    �޸Ľ���ע�����Ĵ���״̬step_flag��ͬʱ�޸�һ��ʱ��
 *
 *    step_flag = 1         ��Ҫ���͸����Ź�˾���н���
 *    step_flag = 2         ����Ź�˾���׳ɹ�����Ҫ����ҵ�����ݿ��¼
 *    step_flag = 3         ���ݿ��¼�ɹ�����Ҫ���͸����ʴ������
 *    step_flag = 4         ���ʴ������ɹ�����Ҫ���͸��ͻ���
 *    ����ɹ����͸��ͻ��ˣ�ע������е���һ������� 
 * 	
 *  \par Output:
 *
 *  \Return:
 * 	1 : success
 * 	0 : error
************************************************************************/
int ModifyInfoStateInArray(int step_flag)
{
	/*variable for semaphore and share memory*/
	int sem_id;
	struct Queue_business_process *process_info;
	time_t time1 = 0;
	struct tm *time2 = NULL;
	
	int success = 0;
	int i = 0;
	pid_t pid = getpid();
		
	sem_id = GetExistedSemphoreExt(SEM_DATA_CHAN_FTOK_ID);
  	process_info = (struct Queue_business_process *)MappingShareMemOwnSpaceExt(SHM_DATA_CHAN_FTOK_ID);
	success = AcquireAccessRight(sem_id);
	for (i=0; i<NUM_PROCESS_QUEUE; i++)
	//for (i=0; i<800; i++)
	{
		if (pid == process_info[i].pid)
		{
			/*�ҵ������pid*/
			process_info[i].step_flag= step_flag;
			/*�õ�ҵ������ʱ��*/
			time(&time1);
			time2=localtime(&time1);/*converts the calendar time to broken-time representation*/
			bzero(process_info[i].affair_generate_time, 9);
			sprintf(process_info[i].affair_generate_time, "%02d:%02d:%02d", time2->tm_hour, time2->tm_min, time2->tm_sec);
			//printf("save data process PID : %d to Queue_business_process[%d] which identified by key : %d!!!\n", pid, i, shm_key);
			break;
		}
	}			
	success = ReleaseAccessRight(sem_id);	
	success = UnmappingShareMem((void *)process_info);
	if (NUM_PROCESS_QUEUE == i)
	{
   		printf("\033[01;31mcommon.c:ModifyInfoStateInArray:no such pid info slot!\033[0m\n");
	}

	return 1;
}

/*************************************************************************
 *  \brief
 *    �޸Ľ���ע�����Ĵ���״̬step_flag��ͬʱ�޸�һ��ʱ��
 *
 *    step_flag = 1         ��Ҫ���͸����Ź�˾���н���
 *    step_flag = 2         ����Ź�˾���׳ɹ�����Ҫ����ҵ�����ݿ��¼
 *    step_flag = 3         ���ݿ��¼�ɹ�����Ҫ���͸����ʴ������
 *    step_flag = 4         ���ʴ������ɹ�����Ҫ���͸��ͻ���
 *    ����ɹ����͸��ͻ��ˣ�ע������е���һ������� 
 * 	
 *  \par Output:
 *
 *  \Return:
 * 	1 : success
 * 	0 : error
************************************************************************/
int RecordAbnormalSerialToLog(struct Queue_business_process *process_info)
{
	/*variable for semaphore and share memory*/
	FILE *fp;
	char log_line_str[TEMP_LENGTH*5];
	char fname[TEMP_LENGTH*5];

	bzero(log_line_str, TEMP_LENGTH*5);
	switch(process_info->step_flag)
	{
		case 1:	/*���Ź�˾����ʧ��*/
			sprintf(fname, "%s_telecom_service_falure.log", global_par.system_par.localhost_id);
			if((fp = fopen(fname, "a+")) == NULL)
			{
				printf("Fail to open %s\n", fname);
 				return -1;
			}	
			sprintf(log_line_str, "|%02d|%02d|%s|%s|%010lu|%s|%s|%s|", 
			//sprintf(log_line_str, "|%02d|%02d|%s|%s|%s|%s|", 
				process_info->company_index,
				process_info->service_type_index,
				process_info->terminal_id,
				process_info->la_serial_number,
				process_info->charge_money,
				process_info->affair_generate_date,
				process_info->affair_generate_time,
				process_info->phonenumber
				);
			fprintf(fp, "%s\n", log_line_str);
			fclose(fp);
			break;
		case 2:	/*���ݿ��¼ʧ��*/
			sprintf(fname, "%s_database_record_falure.log", global_par.system_par.localhost_id);
			if((fp = fopen(fname, "a+")) == NULL)
			{
				printf("Fail to open %s\n", fname);
 				return -1;
			}	
			//sprintf(log_line_str, "|%d|%d|%s|%s|%llu|%s|%s|", 
			sprintf(log_line_str, "|%02d|%02d|%s|%s|%010lu|%s|%s|%s|", 
				process_info->company_index,
				process_info->service_type_index,
				process_info->terminal_id,
				process_info->la_serial_number,
				process_info->charge_money,
				process_info->affair_generate_date,
				process_info->affair_generate_time,
				process_info->phonenumber
				);
			fprintf(fp, "%s\n", log_line_str);
			fclose(fp);
			break;
		case 3:	/*���ʴ���ʧ��*/
			sprintf(fname, "%s_virtual_money_falure.log", global_par.system_par.localhost_id);
			if((fp = fopen(fname, "a+")) == NULL)
			{
				printf("Fail to open %s\n", fname);
 				return -1;
			}	
			//sprintf(log_line_str, "|%d|%d|%s|%s|%llu|%s|%s|", 
			sprintf(log_line_str, "|%02d|%02d|%s|%s|%010lu|%s|%s|%s|", 
				process_info->company_index,
				process_info->service_type_index,
				process_info->terminal_id,
				process_info->la_serial_number,
				process_info->charge_money,
				process_info->affair_generate_date,
				process_info->affair_generate_time,
				process_info->phonenumber
				);
			fprintf(fp, "%s\n", log_line_str);
			fclose(fp);
			break;
		case 4:	/*���ؿͻ���ʧ��*/
			sprintf(fname, "%s_send_proxy_falure.log", global_par.system_par.localhost_id);
			if((fp = fopen(fname, "a+")) == NULL)
			{
				printf("Fail to open %s\n", fname);
 				return -1;
			}	
			//sprintf(log_line_str, "|%d|%d|%s|%s|%llu|%s|%s|", 
			sprintf(log_line_str, "|%02d|%02d|%s|%s|%010lu|%s|%s|%s|", 
				process_info->company_index,
				process_info->service_type_index,
				process_info->terminal_id,
				process_info->la_serial_number,
				process_info->charge_money,
				process_info->affair_generate_date,
				process_info->affair_generate_time,
				process_info->phonenumber
				);
			fprintf(fp, "%s\n", log_line_str);
			fclose(fp);
			break;
		default: /*������*/
			break;
	}

	/*�����ע�����*/
	process_info->step_flag= 0; 
	process_info->company_index = 0;
	process_info->service_type_index = 0;
	process_info->charge_money = 0;
	bzero(process_info->la_serial_number, SERIAL_NUMBER_LENGTH + 1);
	bzero(process_info->terminal_id, CLIENT_ID_INDEX_LENGTH_AT_HEADER+1);
	bzero(process_info->affair_generate_date, 9);
	bzero(process_info->affair_generate_time, 9);
	bzero(process_info->phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);

	return 1;
}


/*************************************************************************
 *  \brief
 *    delete the process pid from Queue_business_process
 *    the destination is determinated by semphore
 *
 *    1. get the semphore.
 *    2. find the slot occupied by own pid.
 *    3. clear the items.
 *    4. release the semphore.
 * 	
 *  \par Input:
 *    sem_key: the semphore identified the array 
 *    shm_key: the share memory identified the array 
 *
 *  \par Output:
 *
 *  \Return:
 * 	1 : success
 * 	0 : error
************************************************************************/
int DeletePidFromArray()
{
	/*variable for semaphore and share memory*/
	int sem_id;
	struct Queue_business_process *process_info;
	
	int success = 0;
	int i = 0;
	pid_t pid = getpid();
		
	sem_id = GetExistedSemphoreExt(SEM_DATA_CHAN_FTOK_ID);
  	process_info = (struct Queue_business_process *)MappingShareMemOwnSpaceExt(SHM_DATA_CHAN_FTOK_ID);
	success = AcquireAccessRight(sem_id);

	for (i=0; i<NUM_PROCESS_QUEUE; i++)
	{
		if (pid == process_info[i].pid)
		{
			process_info[i].pid = 0;
			process_info[i].life_time = 0;
			process_info[i].step_flag= 0; 
			process_info[i].company_index = 0;
			process_info[i].service_type_index = 0;
			process_info[i].charge_money = 0;
			bzero(process_info[i].la_serial_number, SERIAL_NUMBER_LENGTH + 1);
			bzero(process_info[i].terminal_id, CLIENT_ID_INDEX_LENGTH_AT_HEADER+1);
			bzero(process_info[i].affair_generate_date, 9);
			bzero(process_info[i].affair_generate_time, 9);
			//printf("delete data process PID : %d from Queue_business_process[%d] which identified by key : %d!!!\n", pid, i, shm_key);
			break;
		}
	}			
	success = ReleaseAccessRight(sem_id);	
	success = UnmappingShareMem((void *)process_info);
	if (NUM_PROCESS_QUEUE == i)
	{
   		printf("\033[01;31mcommon.c:DeletePidFromArray:pid not found!!!\033[0m\n");
	}

	return 1;
}
