
#include "primary_db_start.h"

int main(int argc, char* argv[])
{
    int ret = 0;

    //setup google logging.
    google::SetLogDestination(google::INFO,     ("./log/log_info_"));
    google::SetLogDestination(google::WARNING,  ("./log/log_warn_"));
    google::SetLogDestination(google::ERROR,    ("./log/log_erro_"));
    google::SetLogDestination(google::FATAL,    ("./log/log_fata_"));

    google::InitGoogleLogging(argv[0]);

    //Start log.
    LOG(INFO)  <<  "================BEGIN LOG=================.";

    //google::SendEmail("jerod.yan@gmail.com",  "LOG BEGIN",  "LOG BEGIN");


    //Get path name and program name.
    strcpy(global_par.execute_path_name, program_invocation_name);
    strcpy(global_par.execute_path_short_name, program_invocation_short_name);
    LOG(INFO) <<"program name: "<< global_par.execute_path_short_name;


    if (readlink ("/proc/self/exe", global_par.execute_path_name, FILE_NAME_LENGTH-1) != -1) {
        LOG(INFO) <<"program name with its path name: "<<global_par.execute_path_name;
    }


    ret = Init_db_server();

    LOG(INFO)  <<  "==================END LOG=================.";
    google::ShutdownGoogleLogging();

    return ret;

}

