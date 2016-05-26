/*
 * =====================================================================================
 *
 *       Filename:  primary_db_error.h
 *
 *    Description:  :
 *
 *        Version:  1.0
 *        Created:  6/19/2010 10:34:01 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <time.h>

#ifdef DEBUG
# define DBG(format, args...) printf(format, ##args)
#else
# define DBG(format, args...)
#endif

//inline void Log_error(const char *file_name, const char *func_name, const int line_num);
//inline void Log_string(const char *string);

#define OUTPUT_OK do{printf("[\033[32mOK\033[0m]\n");fflush(NULL);}while(0);
#define OUTPUT_ERROR do{ printf("[\033[31mERROR\033[0m] %s:%d,%s()\n",__FILE__, __LINE__, __FUNCTION__);LOG(ERROR)<<__FUNCTION__;fflush(NULL);}while(0);
#define MALLOC_ERROR do{printf("Malloc Memory Error");LOG(ERROR)<<"Malloc Memory Error"<<__FUNCTION__;fflush(NULL);}while(0);


