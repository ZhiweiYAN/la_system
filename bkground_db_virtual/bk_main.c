/* *************************************************
 * File name:
 * 		bk_main.cc
 * Description:
 * 		The program is run at the database server.
 * 		It initilizes all child process.
 * 		After that, the bk_server begins to work for
 *		charge terminals and fininace terminals.
 * Author:
 * 		Zhiwei Yan, jerod.yan@gmail.com
 * Date:
 * 		2007-07-07
 * *************************************************/
//#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>

#include "bk_init.h"

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
	
	char cfg_fname[FILE_NAME_LENGTH] = CONFIG_FILENAME;
	int success = 0;
	
	int welcome_sd_finance_control = 0;
	struct sockaddr_in sa_finance_control;

	/* for the process management */
	pid_t pid_daemon_finance_control = 0;

	/* for share memory: databases handle, server mode, and process table */
	int semid_pr = 0;
	struct ShareMemProcess *mem_process_ptr = NULL;

	strcpy(global_par.execute_path_name, program_invocation_name);
	strcpy(global_par.execute_path_short_name, program_invocation_short_name);
	success = ReadConfigAll(cfg_fname);
	
	/* Read parameters from the config file */
	success = Init_parameters();
	printf("Initiating the parameters from the configfile : ");
	if(-1 == success) {
		OUTPUT_ERROR;
		perror("error@bk_main.c:main():Read_parameters");

		return -1;
	} else {
		OUTPUT_OK;
	}

	/* initialize socket variable */
	bzero(&sa_finance_control,sizeof(struct sockaddr_in));
	success =Init_finance_control_process(global_par.system_par.accountant_port,&welcome_sd_finance_control, &sa_finance_control);
	printf("Initiating the view communication with financial terminal :");
	if(-1 == success) {
		OUTPUT_ERROR;
		perror("error@bk_main.c:main():Init_comm_businessview_record");
		return -1;
	} else {
		OUTPUT_OK;
	}


	/* Initialize PROCESS semaphore and One block memory for process*/
	semid_pr = InitialSem(SEM_PROCESS_FTOK_ID);
   	mem_process_ptr = (struct ShareMemProcess *)InitialShm(sizeof(struct ShareMemProcess), SHM_PROCESS_FTOK_ID);

	printf("Initiating the process share memory and semaphore:");
	if(-1 == success) {
		OUTPUT_ERROR;
		perror("error@bk_main.c:main():InitialShmSemDB_PROCESS_ID");
		return -1;
	} else {
		OUTPUT_OK;
	}

	/* Set default values for the ShareMemProcess structure */
	bzero(mem_process_ptr->process_table,sizeof(struct ChildProcessStatus)*MAX_PROCESS_NUMBRER);
	success = UnmappingShareMem((void*)mem_process_ptr);

	/* Initialize the monitor process*/
	fflush(NULL);
	success = Init_monitor_process();
	printf("Initiating the server status monitor:");
	if(-1 == success) {
		OUTPUT_ERROR;
		perror("error@bk_main.c:main():Init_monitor");
		return -1;
	} else {
		OUTPUT_OK;
	}

	/* Initialize the time counter process - clock*/
	fflush(NULL);
	success = Init_counter_process();
	printf("Initiating the server time counter:");
	if(-1 == success) {
		OUTPUT_ERROR;
		perror("error@bk_main.c:main():Init_counter");
		return -1;
	} else {
		OUTPUT_OK;
	}

	/* Two auto refresh tasks processes */
	fflush(NULL);
	success = Init_auto_renew_deposit_process();
	printf("Initiating auto refresh bank deposit:");
	if(-1 == success) {
		OUTPUT_ERROR;
		perror("error@bk_main.c:main():Init_counter");
		return -1;
	} else {
		OUTPUT_OK;
	}

	fflush(NULL);
	success = Init_auto_renew_balance_process();
	printf("Initiating auto refresh virtual balance:");
	if(-1 == success) {
		OUTPUT_ERROR;
		perror("error@bk_main.c:main():Init_counter");
		return -1;
	} else {
		OUTPUT_OK;
	}
	fflush(NULL);
	
	if((pid_daemon_finance_control = fork()) < 0) {
		perror("error:bk_main.c:Daemon_db_server():fork");
		return -1;
	} else if (0 == pid_daemon_finance_control) {
		success = Daemon_finance_control_server(welcome_sd_finance_control,&sa_finance_control);
		if(-1==success)
			exit(0);
	}
	/* wait daemon process pid */
	if(waitpid(pid_daemon_finance_control,NULL,0)!=pid_daemon_finance_control)
		perror("bk_main.c:Init_db_server():waitpid");
	else
		printf("\r\033[32mTHE DAEMON FINANCE_CONTROL HAS EXITED NOW.\033[0m\n");

      LOG(INFO)  <<  "==================END LOG=================.";
      google::ShutdownGoogleLogging();


	return 1;
}
