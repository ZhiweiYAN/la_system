/*********************************************************
 *project: Line communication charges supermarket
 *filename: read_config.c
 *version: 0.44
 *purpose: read configure from file
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
			 gexiaodan, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-22
 *********************************************************/
#ifndef READ_CONFIG_H
#define READ_CONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <unistd.h>

/*MAX COUNT FOR GLOBAL, COMPANIES, PACKETS, ITEMS PARAMETERS*/
#define MAX_COMPANY_NUMBER 64
#define MAX_ITEM_NUMBER 50
#define MAX_PACKET_NUMBER 20
#define MAX_PACKET_POSITION_NUMBER 15 /*the position number in erery packet*/

#define MAX_ITEMS_TO_PARSE 10000
#define ITEM_NAME_LENGTH 50
#define SYSTEM_NAME_LENGTH 30
#define MAX_VERIFY_NUMBER 10
#define MAX_PROXY_NUMBER 10
#define MAX_BUSINESS_NUMBER 10
#define MAX_DATABASE_USER_NUMBER 10
#define COMPANY_NAME_LENGTH 50
#define PACKET_NAME_LENGTH 50
#define ITEM_VALIDE_VALUE_LENGTH 100
#define FILE_NAME_LENGTH 200
//#define COMPANY_NAME_LENGTH 50
#define COMPANY_CONN_LENGTH 10
#define PACKET_POSITIONS 2
#define SUFFIX_FORWARD_FILENAME "_forward.cfg"
#define FORWARD_POSITION 0
#define SUFFIX_BACKWARD_FILENAME "_backward.cfg"
#define BACKWARD_POSITION 1
#define COMPANY_ITEM_NAME "COMPANY_CODE"
#define COMPANY_ITEM_INDEX 0
#define SERVICE_TYPE_ITEM_NAME "TRADE_CODE"
#define SERVICE_TYPE_ITEM_INDEX 1
#define CLIENT_ID_ITEM_NAME "CLIENT_ID"
#define CLIENT_ID_ITEM_INDEX 2
#define MONEY_ITEM_NAME "CHARGE_MONEY"
#define MONEY_ITEM_INDEX 3
#define SERIAL_NUMBER_ITEM_NAME "SERIAL_NUMBER"
#define SERIAL_NUMBER_ITEM_INDEX 4
#define RESPOND_CODE_ITEM_NAME "RESPONDE_CODE"
#define RESPOND_CODE_ITEM_INDEX 5
#define LA_SERIAL_NUMBER_ITEM_NAME "LA_SERIAL_NUMBER"
#define LA_SERIAL_NUMBER_ITEM_INDEX 6
#define LA_RECORD_DATE_ITEM_NAME "LA_RECORD_DATE"
#define LA_RECORD_DATE_ITEM_INDEX 7
#define LA_RECORD_TIME_ITEM_NAME "LA_RECORD_TIME"
#define LA_RECORD_TIME_ITEM_INDEX 8

#define LOCALHOST_ID_LENGTH 2
#define MAXINTERFACES 16

/*all input parameters*/
typedef struct {
    int start_pos;
    int len;
    int direction;
    char name[ITEM_NAME_LENGTH]; /* 20 */
    int type; /* 0:int, 1:string */
    char valid_value[ITEM_VALIDE_VALUE_LENGTH]; /* 100 */
    int db_record; /* 0:not record, 1:record */
    char db_alias_name[ITEM_NAME_LENGTH]; /* 20 */
    int db_alias_type; /* 0:int, 1:string, 2:time */
    int rev_0; /* reserve */
    int rec_1; /* reserve */
}item_parameters;

typedef struct {
    int item_count;
    item_parameters item_par_array[MAX_ITEM_NUMBER];
    int item_index[MAX_ITEM_NUMBER];
}packet_parameters;

typedef struct {
    char ip_address[16];
    int port;
    char real_ip_address[16];
    int real_port;
}network_parameters;

typedef struct {
    int packet_count;
    char company_name[COMPANY_NAME_LENGTH];
    int company_conn;//Max connection num for each company
    network_parameters network_par;
    packet_parameters pkt_par_array[MAX_PACKET_NUMBER][MAX_PACKET_POSITION_NUMBER];
    int packet_important_level[MAX_PACKET_NUMBER];
    char packet_name[MAX_PACKET_NUMBER][PACKET_NAME_LENGTH];
    int use_middleware;
    char packet_encoding[COMPANY_NAME_LENGTH];
}company_parameters;

typedef struct {
    char localhost_ip_address[16];
    char localhost_id[LOCALHOST_ID_LENGTH+1];
    int verify_number;
    int verify_data_port;
    char verify_ip_addr_array[MAX_VERIFY_NUMBER][16];
    char verify_database_name[SYSTEM_NAME_LENGTH];
    int verify_database_user_number;
    char verify_database_user[MAX_DATABASE_USER_NUMBER][SYSTEM_NAME_LENGTH];
    char verify_database_password[MAX_DATABASE_USER_NUMBER][SYSTEM_NAME_LENGTH];
    int proxy_number;
    char proxy_ip_addr_array[MAX_PROXY_NUMBER][16];
    int proxy_control_port;
    int proxy_data_port;
    int proxy_max_sql_connection;
    int proxy_max_affair_connection;
    int proxy_max_temp_connection;
    int business_number;
    char business_ip_addr_array[MAX_BUSINESS_NUMBER][16];
    int business_data_port;
    char database_self_ip_address[16];
    char database_brother_ip_address[16];
    char database_invoice_ip_address[16];
    int database_watchdog_port;
    int database_heartbeat_port;
    int database_query_port;
    int database_data_port;
    char database_name[SYSTEM_NAME_LENGTH];
    int database_user_number;
    char database_user[MAX_DATABASE_USER_NUMBER][SYSTEM_NAME_LENGTH];
    char database_password[MAX_DATABASE_USER_NUMBER][SYSTEM_NAME_LENGTH];
    char bank_ip_address[16];
    int bank_port;
    int accountant_port;
    int business_virtual_money_port;
    char china_bank_ip_address[16];
    int china_bank_port;
}system_parameters;

typedef struct {
    int company_num;
    company_parameters company_par_array[MAX_COMPANY_NUMBER];
    system_parameters system_par;
    char execute_path_name[FILE_NAME_LENGTH];
    char execute_path_short_name[FILE_NAME_LENGTH];
}global_parameters;


/*filename*/
typedef struct {
    char company_cfg_filename[FILE_NAME_LENGTH];
    char company_name[COMPANY_NAME_LENGTH];
    int company_conn;
}company_config;

typedef struct {
    char system_cfg_filename[FILE_NAME_LENGTH];
    int company_number;
    company_config company_cfg_array[MAX_COMPANY_NUMBER];
}global_config;

typedef struct {
    char verify_ip_address[16];
    char verify_database_user[SYSTEM_NAME_LENGTH];
    char verify_database_password[SYSTEM_NAME_LENGTH];
    char proxy_ip_address[16];
    char business_ip_address[16];
    char database_user[SYSTEM_NAME_LENGTH];
    char database_password[SYSTEM_NAME_LENGTH];
}system_config;

typedef struct {
    char network_config_filename[FILE_NAME_LENGTH];
    char packet_config_filename[FILE_NAME_LENGTH];
    int use_middleware;
    char packet_encoding[COMPANY_NAME_LENGTH];
}individual_company_config;

typedef struct {
    char packet_config_filename[FILE_NAME_LENGTH];
    char packet_config_real_filename[PACKET_POSITIONS][FILE_NAME_LENGTH];
}packet_config;

typedef struct {
    int packet_number;
}packet_number_config;

typedef struct {
    int packet_important_level;
}packet_important_level_config;

typedef struct {
    int item_count;
}item_num_config;

/*map struct*/
typedef struct {
    char *TokenName;
    void *Place;
    int Type;
}Mapping;


#ifdef INCLUDED_BY_READCONFIG_C
/*global*/
global_parameters global_par;
company_config company_cfg;
global_config global_cfg;
Mapping Map_global_fix[] = {{(char *)("company_number"),			&(global_cfg.company_number),			0},
    {( char *)("system_cfg_filename"),		&(global_cfg.system_cfg_filename),		1},
    {NULL,						NULL,									-1},
};
Mapping Map_global_variable[] = {{(char *)"company_cfg_fname",		&(company_cfg.company_cfg_filename),		1},
    {(char *)"company_name",			&(company_cfg.company_name),				1},
    {(char *)"company_conn",				&(company_cfg.company_conn),				0},//which is a integer type
    {NULL,						NULL,										-1},
};

/*system*/
system_parameters system_par;
system_config system_cfg;
Mapping Map_system_fix[] = {{(char *)"verify_number",					&(system_par.verify_number),						0},
    {(char *)"verify_data_port",					&(system_par.verify_data_port),					0},
    {(char *)"verify_database_name",				&(system_par.verify_database_name),						1},
    {(char *)"verify_database_user_number",		&(system_par.verify_database_user_number),					0},
    {(char *)"proxy_number",						&(system_par.proxy_number),							0},
    {(char *)"proxy_control_port",					&(system_par.proxy_control_port),					0},
    {(char *)"proxy_data_port",						&(system_par.proxy_data_port),						0},
    {(char *)"proxy_max_sql_connection",			&(system_par.proxy_max_sql_connection),				0},
    {(char *)"proxy_max_affair_connection",			&(system_par.proxy_max_affair_connection),			0},
    {(char *)"proxy_max_temp_connection",			&(system_par.proxy_max_temp_connection),			0},
    {(char *)"business_number",					&(system_par.business_number),						0},
    {(char *)"business_data_port",					&(system_par.business_data_port),					0},
    {(char *)"database_self_ip_address",			&(system_par.database_self_ip_address),				1},
    {(char *)"database_brother_ip_address",			&(system_par.database_brother_ip_address),			1},
    {(char *)"database_invoice_ip_address",			&(system_par.database_invoice_ip_address),			1},
    {(char *)"database_watchdog_port",				&(system_par.database_watchdog_port),				0},
    {(char *)"database_heartbeat_port",				&(system_par.database_heartbeat_port),				0},
    {(char *)"database_query_port",					&(system_par.database_query_port),					0},
    {(char *)"database_data_port",					&(system_par.database_data_port),					0},
    {(char *)"database_name",						&(system_par.database_name),						1},
    {(char *)"database_user_number",				&(system_par.database_user_number),					0},
    {(char *)"bank_ip_address",						&(system_par.bank_ip_address),						1},
    {(char *)"bank_port",							&(system_par.bank_port),							0},
    {(char *)"accountant_port",						&(system_par.accountant_port),						0},
    {(char *)"business_virtual_money_port",			&(system_par.business_virtual_money_port),			0},
    {(char *)"china_bank_ip_address",				&(system_par.china_bank_ip_address),				1},
    {(char *)"china_bank_port",						&(system_par.china_bank_port),						0},
    {NULL,									NULL,												-1},
};
Mapping Map_system_variable_0[] = {{(char *)"verify_ip_address",				&(system_cfg.verify_ip_address),			1},
    {NULL,									NULL,										-1},
};
Mapping Map_system_variable_1[] = {{(char *)"proxy_ip_address",					&(system_cfg.proxy_ip_address),				1},
    {NULL,									NULL,										-1},
};
Mapping Map_system_variable_2[] = {{(char *)"business_ip_address",				&(system_cfg.business_ip_address),			1},
    {NULL,									NULL,										-1},
};
Mapping Map_system_variable_3[] = {{(char *)"database_user",						&(system_cfg.database_user),				1},
    {(char *)"database_password",					&(system_cfg.database_password),			1},
    {NULL,									NULL,										-1},
};
Mapping Map_system_variable_4[] = {{(char *)"verify_database_user",						&(system_cfg.verify_database_user),				1},
    {(char *)"verify_database_password",					&(system_cfg.verify_database_password),			1},
    {NULL,									NULL,										-1},
};


/*company*/
individual_company_config individual_company_cfg;
Mapping Map_individual_company[] = {{(char *)"use_middleware",				&(individual_company_cfg.use_middleware),				0},
    {(char *)"network_cfg_fname",			&(individual_company_cfg.network_config_filename),		1},
    {(char *)"packet_cfg_fname",			&(individual_company_cfg.packet_config_filename),		1},
    {(char *)"packet_encoding",			&(individual_company_cfg.packet_encoding),		1},
    {NULL,							NULL,													-1},
};

/*network*/
network_parameters network_par;
Mapping Map_network[] = {{(char *)"ip_address",			&(network_par.ip_address),										1},
    {(char *)"port",				&(network_par.port),											0},
    {(char *)"real_ip_address",		&(network_par.real_ip_address),									1},
    {(char *)"real_port",			&(network_par.real_port),										0},
    {NULL,					NULL,															-1},
};

/*packet*/
packet_config packet_cfg;
packet_number_config packet_number_cfg;
packet_important_level_config packet_important_level_cfg;
Mapping Map_packet_fix[] = {{(char *)"packet_count",			&(packet_number_cfg.packet_number),						0},
    {NULL,						NULL,													-1},
};
Mapping Map_packet_variable[] = {{(char *)"packet_name",			&(packet_cfg.packet_config_filename),					1},
    {(char *)"packet_important_level",	&(packet_important_level_cfg.packet_important_level),	0},
    {NULL,						NULL,													-1},
};

/*item*/
item_parameters item_par;
item_num_config item_num_cfg;
Mapping Map_item_fix[] = {{(char *)"item_count",			&(item_num_cfg.item_count),		0},
    {NULL,					NULL,                           -1},
};
Mapping Map_item_variable[] = {{(char *)"start_pos",			&(item_par.start_pos),			0},
    {(char *)"len",					&(item_par.len ),				0},
    {(char *)"direction",			&(item_par.direction ),		    0},
    {(char *)"name",				&(item_par.name),				1},
    {(char *)"type",				&(item_par.type),				0},
    {(char *)"valid_value",         &(item_par.valid_value),        1},
    {(char *)"db_record",           &(item_par.db_record),          0},
    {(char *)"db_alias_name",       &(item_par.db_alias_name),      1},
    {(char *)"db_alias_type",       &(item_par.db_alias_type),      0},
    {(char *)"rev_0",               &(item_par.rev_0),              0},
    {(char *)"rev_1",               &(item_par.rec_1),              0},
    {NULL,					NULL,                           -1},
};

#endif

#ifndef INCLUDED_BY_READCONFIG_C
/*global*/
extern global_parameters global_par;
extern company_config company_cfg;
extern global_config global_cfg;
extern Mapping Map_global_fix[];
extern Mapping Map_global_variable[];

/*system*/
extern system_parameters system_par;
extern system_config system_cfg;
extern Mapping Map_system_fix[];
extern Mapping Map_system_variable_0[];
extern Mapping Map_system_variable_1[];
extern Mapping Map_system_variable_2[];
extern Mapping Map_system_variable_3[];
extern Mapping Map_system_variable_4[];

/*company*/
extern individual_company_config individual_company_cfg;
extern Mapping Map_individual_company[];

/*network*/
extern Mapping Map_network[];
extern network_parameters network_par;

/*packet*/
extern packet_config packet_cfg;
extern packet_number_config packet_number_cfg;
extern packet_important_level_config packet_important_level_cfg;
extern Mapping Map_packet_fix[];
extern Mapping Map_packet_variable[];

/*item*/
extern item_parameters item_par;
extern item_num_config item_num_cfg;
extern Mapping Map_item_fix[];
extern Mapping Map_item_variable[];

#endif

int ChangLocalFilenameToGlobalFilename(char *filename);
char *GetConfigFileContent (char *filename);
int ParseContent(char *buf, int index, int global_flag, Mapping Map[]);
int ParameterNameToMapIndex (char *s, int index, Mapping Map[]);
int GlobalParameterNameToMapIndex(char *s, Mapping Map[]);
int ReadConfigGlobalLevel(char *filename);
int ReadConfigSystemLevel(void);
int ReadConfigCompanyLevel(int company_index);
int ReadConfigNetworkLevel(int company_index);
int ReadConfigPacketLevel(int company_index);
int ReadConfigItemLevel(int company_index, int postion_index, int packet_index);
int ReadConfigAll(char *filename);
int FindItemIndexForEachPacketFormat(const char *item_name, int item_name_len, int find_index);
int GetLocalhostInformation(void);

#endif
