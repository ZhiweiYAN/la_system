/*********************************************************
 *project: Line communication charges supermarket
 *filename: business.c
 *version: 0.4
 *purpose: main function of prototype of affair processing machine
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
//#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>

#include "business.h"

/*************************************************************************
 *  \brief
 *    The main fuction
 *
 *    1. fork a control process to initilize
 *    2. create a socket server for proxy
 *    3. fork a new process handle business of proxy
 *
 *  \par Input:
 *
 *  \par Output:
 *
 *  \Return:
 *    1: success
 *    0: fail
************************************************************************/
int main(int argc, char* argv[])
{
    //setup google logging.
    google::SetLogDestination(google::INFO,("./log/log_info_"));
	google::SetLogDestination(google::WARNING,("./log/log_warning_"));
	google::SetLogDestination(google::ERROR,("./log/log_error_"));
	google::SetLogDestination(google::FATAL,("./log/log_fatal_"));
	
    google::InitGoogleLogging(argv[0]);
	
    //Start log.      
    LOG(INFO)  <<  "================BEGIN LOG=================.";
	
    //google::SendEmail("jerod.yan@gmail.com",  "LOG BEGIN",  "LOG BEGIN");


	//Get path name and program name.
	strcpy(global_par.execute_path_name, program_invocation_name);
    strcpy(global_par.execute_path_short_name, program_invocation_short_name);
    LOG(INFO) <<"program name: "<< global_par.execute_path_short_name;
    LOG(INFO) <<"program name with its path name: "<<global_par.execute_path_name;

	/*varibles for configuration*/	
	char cfg_fname[FILE_NAME_LENGTH];
	int success = 0;

	/*varibles for check data channels*/
	struct Queue_business_process *shm_data_chan;
	int sem_data_chan;

	/*varibles for check data channels*/
	struct DB_status *shm_db_status;
	int sem_db_status;

	/*GetEvnName and Allname*/
	strcpy(cfg_fname, CONFIG_FILENAME);
	
	strcpy(global_par.execute_path_name, program_invocation_name);
	strcpy(global_par.execute_path_short_name, program_invocation_short_name);
	
	/*Get all config from "Business_X.cfg" file*/
	//WriteProcessInfoToFile("MAIN_PROCESS");
	success = ReadConfigAll(cfg_fname);

	/*malloc the share memory for check data channels at the first time*/
	sem_data_chan = InitialSem(SEM_DATA_CHAN_FTOK_ID);
	shm_data_chan = (struct Queue_business_process *)InitialShm(NUM_PROCESS_QUEUE * sizeof(struct Queue_business_process), SHM_DATA_CHAN_FTOK_ID);	
	bzero(shm_data_chan, NUM_PROCESS_QUEUE * sizeof(struct Queue_business_process));

	/*malloc the share memory for db status at the first time*/
	sem_db_status = InitialSem(SEM_DB_STATUS_FTOK_ID);
	shm_db_status = (struct DB_status *)InitialShm(sizeof(struct DB_status), SHM_DB_STATUS_FTOK_ID);	
	shm_db_status->status = PRIMARY;

  	printf("The procedure of configuration is ");
   if(success)
   {
   	OUTPUT_OK;
   }
   else
   {
   	OUTPUT_ERROR;
   }
   
  	/*child process to communication with proxy by Control Channel*/
	success = CreateCheckProcess();

	/*parent process to make up server for business of the Proxy*/
	int sock_server_proxy = CreateSocket();
  	success = BindServerPort(global_par.system_par.business_data_port, sock_server_proxy);
	printf("The procedure of setup server for affairs is ");
  	if(success)
  	{
  		OUTPUT_OK;
  	}
  	else
  	{
  		OUTPUT_ERROR;
  	}		
	
	success = CreateProcessAcceptClient(sock_server_proxy, MAXCONNECTIONNUM);
	/*Unmapping the share memory of data channels status*/
	success = UnmappingShareMem((void*)shm_data_chan);
	/*Unmapping the share memory of data channels status*/
	success = UnmappingShareMem((void*)shm_db_status);

      LOG(INFO)  <<  "==================END LOG=================.";
      google::ShutdownGoogleLogging();

	return 1;

}
