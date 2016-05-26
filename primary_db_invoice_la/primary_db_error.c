/*
 * =====================================================================================
 *
 *       Filename:  primary_db_error.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  6/19/2010 10:33:28 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */
#include "primary_db_error.h"

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Log_error
 *  Description:  Record the errors into log file
 * =====================================================================================
 */
inline void Log_error(const char *file_name, const char *func_name, const int line_num)
{
    FILE* fp_log = NULL;
    time_t t;

    fp_log = fopen("log_error.txt","a+");
    if (NULL==fp_log) {
        fclose(fp_log);
        fp_log = NULL;
        return;
    }

    t = time(NULL);
    fprintf(fp_log,"%24.24s ERROR:FUNCTION_NAME:%s(),FILENAME:%s:(%d)\n",ctime(&t),func_name,file_name,line_num);

    fflush(fp_log);
    fclose(fp_log);
    fp_log = NULL;

    return;

}

inline void Log_string(const char *string)
{
    FILE* fp_log = NULL;
    time_t t;

    fp_log = fopen("log_error.txt","a+");
    if (NULL==fp_log) {
        fclose(fp_log);
        fp_log = NULL;
        return;
    }

    t = time(NULL);
    fprintf(fp_log,"%24.24s ",ctime(&t));
    fprintf(fp_log,"|%s|\n",string);

    fflush(fp_log);
    fclose(fp_log);
    fp_log = NULL;

    return;

}

