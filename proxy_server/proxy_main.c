#include "proxy_init.h"
int main(int argc, char* argv[])
{
	int success = 0;
	
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
	
	success = Init_proxy_server();

    	LOG(INFO)  <<  "==================END LOG=================.";
    	google::ShutdownGoogleLogging();

	return success;

}

