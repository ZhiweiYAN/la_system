/* *************************************************
 * File Name:
 * 		db_sync.cc
 * Brief:
 * 		The two database on two different machine will be synchronized.
 *		The synchronization rule is:
 *			The database that contains less records will be added records,
 *			which are from the database that contains more records.
 * Author:
 * 		Yan Zhiwei, jerod.yan@gmail.com (Drum Team)
 * Date:
 * 		2007-01-08
 * *************************************************/
#include "primary_db_sync.h"
/* *************************************************
 * Function Name:
 * 		int Sync_databases(PGconn *conn1, PGconn *conn2)
 * Input:
 * 		PGconn *conn1;
 * 		PGconn *conn2;
 * Output:
 * 		1 ---> success
 *			0 ---> failure
 * *************************************************/
int Sync_databases(PGconn *conn1, PGconn *conn2)
{

    int  success = -1;
    int  i = 0;

    /* Sync all tables in the database */
    for (i=0; i<global_par.company_num; i++) {
        success = Sync_database_table(conn1, conn2, global_par.company_par_array[i].company_name);
    }

    return success;
}

int Test_connection_db_server(char *user_name, char *password,char *db_name,char *ip_addr)
{
    int res = -1;
    PGconn  *conn = NULL;
    conn = Connect_db_server(user_name,password,db_name,ip_addr);
    if (NULL!=conn) {
        res = 1;
    } else {
        res = -1;
    }

    PQfinish(conn);
    conn = NULL;

    return res;

}

/* *************************************************
 * Function Name:
 * 		PGconn *Connect_db_server(char *user_name, char *password,char *db_name,char *ip_addr)
 * Input:
 * 		char *ip_addr;
 * 		char *user_name;
 * 		char *password;
 * 		char *db_name;
 * Output:
 * 		PGconn *conn ---> success
 *			NULL ---> failure
 * *************************************************/
PGconn *Connect_db_server(char *user_name, char *password,char *db_name,char *ip_addr)
{
    PGconn  *conn;
    char conn_string[COMM_LENGTH];

    /* Check input parameters */
    if (NULL==ip_addr||NULL==user_name||NULL==password||NULL==db_name) {
        OUTPUT_ERROR;
        return NULL;
    }

    bzero(conn_string,COMM_LENGTH);
    sprintf(conn_string,"user=%s password=%s dbname=%s hostaddr=%s",user_name,password,db_name,ip_addr);
    DBG("Connect to DB: |%s|\n",conn_string);
	LOG(INFO)<<"Connect to DB:" << conn_string;
	
    /* Connect the database */
    conn = PQconnectdb(conn_string);

    if (PQstatus(conn) != CONNECTION_OK) {
		LOG(ERROR)<<"Connect to DB, failed." << "detail: " <<conn_string;
        OUTPUT_ERROR;
        return NULL;
    }

    return conn;
}
/* *************************************************
 * Function Name:
 * 		int Sync_database_table(PGconn* conn_db_a, PGconn *conn_db_b, char *tb_name)
 * Input:
 *			PGconn* conn_db_a,
 *			PGconn *conn_db_b,
 * 		char *tb_name
 *
 * Output:
 * 		1 ---> success
 *		-1---> failure
 * *************************************************/

int Sync_database_table(PGconn* conn_db_a, PGconn *conn_db_b, char *tb_name)
{
    PGresult *res_db_from = NULL;
    PGresult *res_db_to = NULL;

    int success = 0;

    PGconn *conn_db_from = NULL;
    PGconn *conn_db_to = NULL;


    long long int max_id_db_a = 0;
    long long int min_id_db_a = 0;
    long long int record_sum_db_a = 0;

    long long int max_id_db_b = 0;
    long long int min_id_db_b = 0;
    long long int record_sum_db_b = 0;

    long long int min_id_db = 0;

    long long int m = 0;
    long long int i = 0;

    int field_sum = 0;
    long long int record_sum = 0;

    char query_string[COMM_LENGTH];
    char query_string_db_to[COMM_LENGTH];
    char query_string_db_from[COMM_LENGTH];
    char tb_tuples_fields[COMM_LENGTH];
    char tb_tuples_values[COMM_LENGTH];

    struct db_structure *db_str = NULL;

    bzero(query_string,COMM_LENGTH);
    bzero(query_string_db_to,COMM_LENGTH);
    bzero(query_string_db_from,COMM_LENGTH);
    bzero(tb_tuples_values,COMM_LENGTH);
    bzero(tb_tuples_values,COMM_LENGTH);


    /* Check the input parameters */
    if (NULL==conn_db_a||NULL==conn_db_b||NULL==tb_name) {
        perror("error@db_sync.cc:Sync_database():NULL==db_a");
        return -1;
    }

    /* Check the sync point */
    //	success = Calibrate_sync_point(conn_db_a, conn_db_b, tb_name);

    /* Check Database A  current serial values */
    if (-1 == Get_max_id(conn_db_a,tb_name, &max_id_db_a))
        return -1;
    if (-1 == Set_valid_id(conn_db_a,tb_name, max_id_db_a))
        return -1;

    if (-1 == Get_min_id(conn_db_a,tb_name, &min_id_db_a))
        return -1;
    if (-1 == Get_record_sum(conn_db_a,tb_name,&record_sum_db_a))
        return -1;
    /* Check Database B  current serial values */
    if (-1 == Get_max_id(conn_db_b,tb_name, &max_id_db_b))
        return -1;
    if (-1 == Set_valid_id(conn_db_b,tb_name, max_id_db_b))
        return -1;

    if (-1 == Get_min_id(conn_db_b,tb_name, &min_id_db_b))
        return -1;
    if (-1 == Get_record_sum(conn_db_b,tb_name,&record_sum_db_b))
        return -1;

    if (0 != record_sum_db_a) {
        if (record_sum_db_a != (max_id_db_a - min_id_db_a+1))
            return -1;
    }
    if (0 !=record_sum_db_b) {
        if (record_sum_db_b != (max_id_db_b - min_id_db_b+1))
            return -1;
    }


    /* Data are SAME, NO synchronization */
    if (max_id_db_a == max_id_db_b) {
        return 1;
    }

    /* Data are DIFFERENT, MUST synchronization */
    if (max_id_db_a > max_id_db_b) {
        conn_db_from = conn_db_a;
        conn_db_to = conn_db_b;
        min_id_db = max_id_db_b;
    } else {
        conn_db_from = conn_db_b;
        conn_db_to = conn_db_a;
        min_id_db = max_id_db_a;
    }


    /* Get the rest records which are not in db_to */
    bzero(query_string_db_from,COMM_LENGTH);
    sprintf(query_string_db_from,"SELECT * from %s where id > %llu ",tb_name,min_id_db);

    DBG("SYNC: SQL:|%s|\n",query_string_db_from);
    fflush(NULL);
    PQclear(res_db_from);
    res_db_from = NULL;
    res_db_from = PQexec(conn_db_from, query_string_db_from);
    if (PQresultStatus(res_db_from) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Sync_database():PGRES_TUPLES_OK");
        printf("%s\n",PQerrorMessage(conn_db_from));
        PQclear(res_db_from);
        res_db_from = NULL;
        return -1;
    }

    /* Get the information of resluts */
    field_sum = (long long int)PQnfields(res_db_from);
    record_sum = (long long int)PQntuples(res_db_from);

    db_str = (struct db_structure*)malloc(sizeof(struct db_structure)*field_sum);
    if (NULL==db_str) {
        perror("error@db_sync.cc:Sync_database():NULL==db_str");
        PQclear(res_db_from);
        PQclear(res_db_to);
        res_db_to = NULL;
        res_db_from = NULL;
        return -1;
    }

    for (i=0; i<field_sum; i++) {
        bzero((db_str+i)->field_name,SQL_LENGTH);
        bzero((db_str+i)->field_type,SQL_LENGTH);
        strcpy((db_str+i)->field_name,(char *)PQfname(res_db_from,i));
        strcpy((db_str+i)->field_type,Translate_ftype(PQftype(res_db_from,i)));
    }
    bzero(tb_tuples_fields,COMM_LENGTH);
    strcpy(tb_tuples_fields,(db_str+1)->field_name);
    for (i=2; i<field_sum; i++) {
        strcat(tb_tuples_fields,",");
        strcat(tb_tuples_fields,(db_str+i)->field_name);
    }

    for (m=0; m<record_sum; m++) {
        /* loop through all rows returned */
        bzero(tb_tuples_values,COMM_LENGTH);
        /* ATTENTION: set i value as 01 */
        for (i=1; i<field_sum; i++) {
            /* according to type , and generate values */
            if (0==strcmp("NUMERIC",(db_str+i)->field_type)) {
                strcat(tb_tuples_values,PQgetvalue(res_db_from,m,i));
            } else if (0==strcmp("VARCHAR",(db_str+i)->field_type)) {
                strcat(tb_tuples_values,"\'");
                strcat(tb_tuples_values,PQgetvalue(res_db_from,m,i));
                strcat(tb_tuples_values,"\'");
            } else if (0==strcmp("DATE",(db_str+i)->field_type)) {
                strcat(tb_tuples_values,"DATE \'");
                strcat(tb_tuples_values,PQgetvalue(res_db_from,m,i));
                strcat(tb_tuples_values,"\'");
            } else {
                strcat(tb_tuples_values,PQgetvalue(res_db_from,m,i));
            }
            if ((field_sum-1) != i)
                strcat(tb_tuples_values,",");
        }
        bzero(query_string_db_to,COMM_LENGTH);
        sprintf(query_string_db_to,"INSERT INTO %s (%s) VALUES (%s);", tb_name,tb_tuples_fields,tb_tuples_values);
        //printf("%s",query_string);
#ifdef DEBUG_SYNC_DB

        usleep(20000);
#endif
        /* Write one record/tuple data to database of destination */
        PQclear(res_db_to);
        res_db_to = NULL;
        res_db_to = PQexec(conn_db_to, query_string_db_to);
        if (PQresultStatus(res_db_to) != PGRES_COMMAND_OK) {
            perror("error@db_sync.cc:Sync_database()conn_db_to:PGRES_TUPLES_OK");
            printf("%s\n",PQerrorMessage(conn_db_to));
            PQclear(res_db_to);
            PQclear(res_db_from);
            res_db_to = NULL;
            res_db_from = NULL;
            free(db_str);
            db_str = NULL;
            return -1;
        }

        printf("\rSYNC_OK: %llu/%llu records.    ",m+1,record_sum);
        fflush(NULL);
    }

    PQclear(res_db_to);
    PQclear(res_db_from);
    res_db_to = NULL;
    res_db_from = NULL;

    /* Check Database A  current serial values */
    if (-1 == Get_max_id(conn_db_a,tb_name, &max_id_db_a))
        return -1;
    if (-1 == Get_min_id(conn_db_a,tb_name, &min_id_db_a))
        return -1;
    if (-1 == Get_record_sum(conn_db_a,tb_name,&record_sum_db_a))
        return -1;
    /* Check Database B  current serial values */
    if (-1 == Get_max_id(conn_db_b,tb_name, &max_id_db_b))
        return -1;
    if (-1 == Get_min_id(conn_db_b,tb_name, &min_id_db_b))
        return -1;
    if (-1 == Get_record_sum(conn_db_b,tb_name,&record_sum_db_b))
        return -1;
    if (0 != record_sum_db_a) {
        if (record_sum_db_a != (max_id_db_a - min_id_db_a+1))
            return -1;
    }
    if (0 !=record_sum_db_b) {
        if (record_sum_db_b != (max_id_db_b - min_id_db_b+1))
            return -1;
    }

    /* Data should be SAME, NO synchronization */
    if (record_sum_db_b == record_sum_db_a) {
        success = 1;
    }
    free(db_str);
    db_str = NULL;
    return success;
}
/* *************************************************
 * Function Name:
 * 		* Translate_ftype(Oid ftype_id)
 * Input:
 *
 * Output:
 * 		field_type;
 *
 * *************************************************/
char * Translate_ftype(Oid ftype_id)
{
    if (ftype_id == 1700)
        return (char *)"NUMERIC";
    if (ftype_id == 1083)
        return (char *) "TIME";
    if (ftype_id == 1082)
        return (char *)"DATE";
    if (ftype_id == 1114)
        return (char *)"TIMESTAMP";
    if (ftype_id == 1043||ftype_id ==1042)
        return (char *)"VARCHAR";
    else
        return (char *)"VARCHAR";
}
/* *************************************************
 * Function Name:
 * 		int Calibrate_sync_point(PGconn* conn_db_a, PGconn *conn_db_b, char *tb_name)
 * Input:
 *
 * Output:
 * 		1 ---> sync_point is OK.
 *		1 ---> sync_point is different.
 *
 * *************************************************/
int Calibrate_sync_point(PGconn* conn_db_a, PGconn *conn_db_b, char *tb_name)
{
    PGresult *res_db_a = NULL;
    PGresult *res_db_b = NULL;
    PGconn* conn_db_delete = NULL;

    long long int cur_id_db_a = 0;
    long long int cur_id_db_b = 0;
    long long int min_id_db = 0;
    char *end_ptr = NULL;
    int j = 0;
    int success = 0;

    int field_sum = 0;
    long long int record_sum = 0;

    char query_string[COMM_LENGTH];

    bzero(query_string,COMM_LENGTH);

    /* Query command for current serial values of the table*/
    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT max(id) from %s;",tb_name);

    /* Check Database A  current serial values */
    PQclear(res_db_a);
    res_db_a = PQexec(conn_db_a, query_string);
    if (PQresultStatus(res_db_a) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Calibrate_sync_point():PGRES_TUPLES_OK01");
        printf("%s\n",PQerrorMessage(conn_db_a));
        PQclear(res_db_a);
        return -1;
    }
    //cur_id_db_a = strtoll(PQgetvalue(res_db_a, 0, 0),&end_ptr,10) ;
    if (0==PQgetisnull(res_db_a,0,0)) {
        cur_id_db_a = strtoll(PQgetvalue(res_db_a, 0, 0),&end_ptr,10) ;
    } else {
        cur_id_db_a = 0;
    }

    /* Check Database B  current serial values */
    PQclear(res_db_b);
    res_db_b = PQexec(conn_db_b, query_string);
    if (PQresultStatus(res_db_b) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Calibrate_sync_point():PGRES_TUPLES_OK02");
        printf("%s\n",PQerrorMessage(conn_db_b));
        PQclear(res_db_b);
        return -1;
    }
    //cur_id_db_b = strtoll(PQgetvalue(res_db_b, 0, 0),&end_ptr,10);
    if (0==PQgetisnull(res_db_b,0,0)) {
        cur_id_db_b = strtoll(PQgetvalue(res_db_b, 0, 0),&end_ptr,10);
    } else {
        cur_id_db_b = 0;
    }

    /* Compare the max id to get the minmum id */
    if (cur_id_db_b == cur_id_db_a)
        return 1;
    if (cur_id_db_b<cur_id_db_a) {
        min_id_db = cur_id_db_b;
        conn_db_delete = conn_db_b;
    } else {
        min_id_db = cur_id_db_a;
        conn_db_delete = conn_db_a;
    }

    /* Query command for current serial values of the table*/
    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT * from %s where id=%llu;",tb_name,min_id_db);

    /* Check Database A  current serial values */
    PQclear(res_db_a);
    res_db_a = PQexec(conn_db_a, query_string);
    if (PQresultStatus(res_db_a) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Calibrate_sync_point():PGRES_TUPLES_OK01");
        printf("%s\n",PQerrorMessage(conn_db_a));
        PQclear(res_db_a);
        return -1;
    }

    /* Check Database B  current serial values */
    PQclear(res_db_b);
    res_db_b = PQexec(conn_db_b, query_string);
    if (PQresultStatus(res_db_b) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Calibrate_sync_point():PGRES_TUPLES_OK02");
        printf("%s\n",PQerrorMessage(conn_db_b));
        PQclear(res_db_b);
        return -1;
    }

    /* Assume Table A and B have the same sturcture */
    field_sum = (long long int)PQnfields(res_db_a);
    //field_sum = (long long int)PQnfields(res_db_b);
    record_sum = (long long int)PQntuples(res_db_a);
    //record_sum = (long long int)PQntuples(res_db_b);
    for (j = 0; j < field_sum; j++) {
        if (0!=strcmp(PQgetvalue(res_db_b, 0, j),PQgetvalue(res_db_a, 0, j))) {
            perror("db_sync.cc:Calibrate_sync_point():diffent sync point");
            success = Delete_db_records(conn_db_delete,tb_name,min_id_db);
            perror("db_sync.cc:Calibrate task is over!");
            return 1;
        }
    }

    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Delete_db_records(PGconn *conn_db,char *tb_name, long long int min_db_id);
 * Input:
 *
 * Output:
 * 		1 ---> sync_point is OK.
 *		1 ---> sync_point is different.
 *
 * *************************************************/
int Delete_db_records(PGconn *conn_db,char *tb_name, long long int min_db_id)
{

    char query_string[COMM_LENGTH];
    PGresult *res_db = NULL;


    /* Query command for deleting records that satisfy the condition*/
    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"DELETE from %s where id=%llu;",tb_name,min_db_id);

    PQclear(res_db);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_COMMAND_OK) {
        perror("error@db_sync.cc:Delete_db_records():PGRES_COMMAND_OK01");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        return -1;
    }

    /* Query command for deleting records that satisfy the condition*/
    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT nextval(\'%s_id_seq\');",tb_name);

    PQclear(res_db);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Delete_db_records():PGRES_TUPLES_OK02");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        return -1;
    }

    /* Query command for deleting records that satisfy the condition*/
    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT setval(\'%s_id_seq\',%llu);",tb_name,min_db_id-2);

    PQclear(res_db);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Delete_db_records():PGRES_TUPLES_OK03");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        return -1;
    }

    /* Query command for deleting records that satisfy the condition*/
    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"VACUUM %s;",tb_name);

    PQclear(res_db);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_COMMAND_OK) {
        perror("error@db_sync.cc:Delete_db_records():PGRES_COMMAND_OK04");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        return -1;
    }
    return 1;
}

/* *************************************************
 * Function Name:
 * 		int Get_max_id(PGconn *conn_db,char *tb_name, long long int *max_id);
 * Input:
 *
 * Output:
 * 		long long int *max_id --> id
 *		   -1 ---> failure
 * *************************************************/
int Get_max_id(PGconn *conn_db,char *tb_name, long long int *max_id)
{
    PGresult *res_db = NULL;
    char *end_ptr = NULL;
    char query_string[COMM_LENGTH];

    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT max(id) from %s;",tb_name);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Get_max_id():PGRES_TUPLES_OK");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        res_db = NULL;
        return -1;
    }

    /* If the database is empty */
    if (0==PQgetisnull(res_db,0,0)) {
        *max_id = strtoll(PQgetvalue(res_db, 0, 0),&end_ptr,10) ;
    } else {
        *max_id = 0;
    }
    PQclear(res_db);
    res_db = NULL;
    return 1;
}


/* *************************************************
 * Function Name:
 * 		int Get_min_id(PGconn *conn_db,char *tb_name, long long int *min_id);
 * Input:
 *
 * Output:
 * 		long long int *min_id --> id
 *		   -1 ---> failure
 * *************************************************/
int Get_min_id(PGconn *conn_db,char *tb_name, long long int *min_id)
{
    PGresult *res_db = NULL;
    char *end_ptr = NULL;
    char query_string[COMM_LENGTH];

    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT min(id) from %s;",tb_name);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Get_min_id():PGRES_TUPLES_OK");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        res_db = NULL;
        return -1;
    }

    /* If the database is empty */
    if (0==PQgetisnull(res_db,0,0)) {
        *min_id = strtoll(PQgetvalue(res_db, 0, 0),&end_ptr,10) ;
    } else {
        *min_id = 0;
    }
    PQclear(res_db);
    res_db = NULL;
    return 1;
}


/* *************************************************
 * Function Name:
 * 		int Get_record_sum(PGconn *conn_db,char *tb_name, long long int *record_sum);
 * Input:
 *
 * Output:
 * 		long long int *record_sum --> id
 *		   -1 ---> failure
 *			1 ---> success
 * *************************************************/
int Get_record_sum(PGconn *conn_db,char *tb_name, long long int *record_sum)
{
    PGresult *res_db = NULL;
    char *end_ptr = NULL;
    char query_string[COMM_LENGTH];

    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT count(id) from %s;",tb_name);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Get_record_sum():PGRES_TUPLES_OK");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        res_db = NULL;
        return -1;
    }

    /* If the database is empty */
    if (0==PQgetisnull(res_db,0,0)) {
        *record_sum = strtoll(PQgetvalue(res_db, 0, 0),&end_ptr,10);
    } else {
        *record_sum = 0;
    }
    PQclear(res_db);
    res_db = NULL;
    return 1;
}


int Set_valid_id(PGconn *conn_db,char *tb_name, long long int cur_id)
{
    PGresult *res_db = NULL;
    char query_string[COMM_LENGTH];

    if (0==cur_id)
        return 1;

    /* Query command for deleting records that satisfy the condition*/
    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT nextval(\'%s_id_seq\');",tb_name);

    PQclear(res_db);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Set_valid_id():PGRES_TUPLES_OK02");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        return -1;
    }
    /* Query command for deleting records that satisfy the condition*/
    bzero(query_string,COMM_LENGTH);
    sprintf(query_string,"SELECT setval(\'%s_id_seq\',%llu);",tb_name,cur_id);

    PQclear(res_db);
    res_db = PQexec(conn_db, query_string);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@db_sync.cc:Set_valid_id():PGRES_TUPLES_OK03");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res_db);
        return -1;
    }

    PQclear(res_db);
    res_db = NULL;

    return 1;
}
