/*********************************************************
 *project: Line communication charges supermarket
 *filename: read_config.c
 *version: 0.44
 *purpose: read configure from file
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
			 gexiaodan, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-22
 *********************************************************/
#define INCLUDED_BY_READCONFIG_C
#include "config.h"

/************************************************************************
 *  \brief
 *    Get the currernt wholename, and chang the local name to wholepath,
 *
 *  \par Input:
 *    filename: local filename
 *
 *  \par Output:
 *    filename: whole filename
 *
 *  \Return:
 *    0: failure.
 *    1: success.
************************************************************************/
int ChangLocalFilenameToGlobalFilename(char *filename)
{
    char cfg_fname[FILE_NAME_LENGTH];

    bzero(cfg_fname, FILE_NAME_LENGTH);
    strncpy(cfg_fname, global_par.execute_path_name, strlen(global_par.execute_path_name) - strlen(global_par.execute_path_short_name));
    strcat(cfg_fname, filename);
    strcpy(filename, cfg_fname);

    return 1;
}

/************************************************************************
 *  \brief
 *    open filename, and read contents into memory buffer,
 *
 *  \par Input:
 *    filename: config file name
 *
 *  \par Output:
 *    buf: point to the buffer that saves contents
************************************************************************/
char *GetConfigFileContent(char *filename)
{
    int FileSize;
	size_t ret = 0;
    FILE *fp = NULL;
    char *buf = NULL;

    ChangLocalFilenameToGlobalFilename(filename);

    if (NULL == (fp = fopen (filename, "r"))) {
        perror("error@configfile.c:GetConfigFileContent:fopen()\n");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    FileSize = ftell(fp);

    if (NULL == (buf = (char *)malloc(FileSize+1))) {
        perror("error@configfile.c:GetConfigFileContent:malloc()\n");
        exit(1);
    }

    fseek(fp, 0, SEEK_SET);
    ret = fread(buf, 1, FileSize, fp);
    buf[FileSize] = '\0';

    fclose(fp);
    fp = NULL;

    return buf;
}

/************************************************************************
 *  \brief
 *    parse buffer with Map[], and save the result to configinput
 *
 *  \par Input:
 *    char *buf: buffer to be parsed
 *
 *  \Return:
 *    1: success
 *	  0: error
************************************************************************/
int ParseContent(char *buf, int index, int global_flag, Mapping Map[])
{

    char *items[MAX_ITEMS_TO_PARSE]; /*array to save item*/
    int MapIdx;
    int item = 0; /*item number*/
    int InString = 0;/*variable to judge whether it's in a string*/
    int InItem = 0; /*variable to judge whether it's in a item*/
    char *p = buf;/*variable to point the begin of buf*/
    char *bufend = &buf[strlen(buf)];/*variable to point the end of buf*/
    int IntContent;/*variable to save content if it is int type*/
    double DoubleContent;/*variable to save content if it is double type*/
    int i;

    /*Generate a list in items[] with buf, without comments and whitespace*/
    while (p < bufend) {
        switch (*p) {
        case 13:	/*case enter key*/
            p++;
            break;
        case '#':	/*case comment*/
            *p = '\0';/*Replace '#' with '\0' in case of comment*/
            while (*p != '\n' && p < bufend)  /*Skip till EOL or EOF*/
                p++;
            InString = 0;
            InItem = 0;
            break;
        case '\n':
            InItem = 0;
            InString = 0;
            *p++='\0';
            break;
        case ' ':
        case '\t':	/*case whitespace*/
            if (InString)
                p++;
            else {                    /*Terminate non-strings once whitespace is found*/
                *p++ = '\0';
                InItem = 0;
            }
            break;

        case '"':	/*case begin/End of String*/
            *p++ = '\0';
            if (!InString) {
                items[item++] = p;
                InItem = ~InItem;
            } else
                InItem = 0;
            InString = ~InString;
            break;

        default:
            if (!InItem) {
                items[item++] = p;
                InItem = ~InItem;
            }
            p++;
        }
    }
    item--;

    for (i=0; i<item; i+= 3) {
        if (1 == global_flag) {
            if (0 > (MapIdx = GlobalParameterNameToMapIndex(items[i], Map))) {
                continue;
            }

        } else {
            if (0 > (MapIdx = ParameterNameToMapIndex(items[i], index, Map))) {
                continue;
            }
        }

        if (strcmp("=", items[i+1])) {
            perror("error@configfile.c:ParseContent:strcmp()\n");
            exit(1);
        }

        /*get the right content, and save to Map[]*/
        switch (Map[MapIdx].Type) {
        case 0:		/* Numerical*/
            if (1 != sscanf(items[i+2], "%d", &IntContent)) {
                perror("error@configfile.c:ParseContent:sscanf1()\n");
                exit(1);
            }
            * (int *) (Map[MapIdx].Place) = IntContent;
            break;
        case 1:		/*string*/
            strcpy((char *) Map[MapIdx].Place, items [i+2]);
            break;
        case 2:		/*Numerical double*/
            if (1 != sscanf(items[i+2], "%lf", &DoubleContent)) {
                perror("error@configfile.c:ParseContent:sscanf2()\n");
                exit(1);
            }
            * (double *)(Map[MapIdx].Place) = DoubleContent;
            break;
        default:
            perror("error@configfile.c:ParseContent:Unknown value type\n");
        }
    }
    return 1;
}

/************************************************************************
 *  \brief
 *     Returns the index number from Map[] for a given parameter name
 *
 *  \par Input:
 *	  char *s: parameter name string
 *	  int index: parameter index
 *
 *  \Return:
 *	  i: the index number if the string is a valid parameter name
 *    -1: error
************************************************************************/
int ParameterNameToMapIndex(char *s, int index, Mapping Map[])
{
    int i = 0;
    char suffix[20];
    char cmp_TokenName[50];
    memset(suffix, 0, 20);
    sprintf(suffix, "_%03d", index);

    while (Map[i].TokenName != NULL) {
        strcpy(cmp_TokenName, Map[i].TokenName);
        strcat(cmp_TokenName, suffix);
        if (0 == strcmp (cmp_TokenName, s))
            return i;
        else
            i++;
    }
    return -1;
};

/************************************************************************
 *  \brief
 *     Returns the index number from Map[] for a given parameter name(get the line number)
 *
 *  \par Input:
 *	  char *s: parameter name string
 *
 *  \Return:
 *	  i: the index number if the string is a valid parameter name
 *    -1: error
************************************************************************/
int GlobalParameterNameToMapIndex(char *s, Mapping Map[])
{
    int i = 0;

    while (Map[i].TokenName != NULL) {
        if (0 == strcmp (Map[i].TokenName, s))
            return i;
        else
            i++;
    }
    return -1;
};

/************************************************************************
 *  \brief
 *     Parse the command line parameters(global level) and read the config files.
 *
 *  \par Input:
 *    char *filename: config file name
 *
 *  \Return:
 *	  1: success
 *    0: error
************************************************************************/
int ReadConfigGlobalLevel(char *filename)
{
    char *content = NULL;
    char *original_content = NULL;
    int i = 0;

    memset(&global_cfg, 0, sizeof(global_config));
    printf("Parsing Configfile: %s\n", filename);

    /*Get the content from file*/
    content = GetConfigFileContent(filename);
    original_content = (char *)malloc(strlen(content)+1);
    strcpy(original_content, content);

    ParseContent(content, 0 , 1, Map_global_fix);

    global_par.company_num = global_cfg.company_number;
    //global_par.company_par_array = (company_parameters *)malloc(global_par.company_num * sizeof(company_parameters));
    memset(global_par.company_par_array, 0, global_par.company_num * sizeof(company_parameters));
    //global_cfg.company_cfg_array = (company_config *)malloc(global_cfg.company_number * sizeof(company_config));
    memset(global_cfg.company_cfg_array, 0, global_cfg.company_number * sizeof(company_config));

    /*parse the content*/
    for (i=0; i<global_cfg.company_number; i++) {
        memset(&company_cfg, 0, sizeof(company_config));
        strcpy(content, original_content);
        ParseContent(content, i, 0, Map_global_variable);
        memcpy(&global_cfg.company_cfg_array[i], &company_cfg, sizeof(company_config));
        memcpy(global_par.company_par_array[i].company_name, company_cfg.company_name, strlen(company_cfg.company_name));
        global_par.company_par_array[i].company_conn = company_cfg.company_conn;
    }
//for test global cfg
    /*for (i=0; i<global_cfg.company_number; i++)
    {
    	printf("COMPANY NAME = |%s|\n", global_par.company_par_array[i].company_name);
    	printf("COMPANY CONN NUM = |%d|\n", global_par.company_par_array[i].company_conn);
    }*/

    free(content);/*free memory that malloc in GetConfigFileContent()*/
    free(original_content);
    content = NULL;
    original_content = NULL;

    return 1;
}

/************************************************************************
 *  \brief
 *     Parse the command line parameters(system level) and read the config files.
 *
 *  \par Input:
 *    None
 *
 *  \Return:
 *	  1: success
 *    0: error
************************************************************************/
int ReadConfigSystemLevel(void)
{
    char *content = NULL;
    char *original_content = NULL;
    int i = 0;

    memset(&system_par, 0, sizeof(system_parameters));
    printf("Parsing Configfile: %s\n", global_cfg.system_cfg_filename);

    /*Get the content from file*/
    content = GetConfigFileContent(global_cfg.system_cfg_filename);
    original_content = (char *)malloc(strlen(content)+1);
    strcpy(original_content, content);

    ParseContent(content, 0 , 1, Map_system_fix);
    memcpy(&global_par.system_par, &system_par, sizeof(system_parameters));

    /*get the ip address of verify servers*/
    for (i=0; i<system_par.verify_number; i++) {
        memset(&system_cfg, 0, sizeof(system_config));
        strcpy(content, original_content);
        ParseContent(content, i, 0, Map_system_variable_0);
        memcpy(global_par.system_par.verify_ip_addr_array[i], system_cfg.verify_ip_address, strlen(system_cfg.verify_ip_address));
    }

    /*get the ip address of proxy servers*/
    for (i=0; i<system_par.proxy_number; i++) {
        memset(&system_cfg, 0, sizeof(system_config));
        strcpy(content, original_content);
        ParseContent(content, i, 0, Map_system_variable_1);
        memcpy(global_par.system_par.proxy_ip_addr_array[i], system_cfg.proxy_ip_address, strlen(system_cfg.proxy_ip_address));
    }
	
    /*get the ip address of business servers*/
    for (i=0; i<system_par.business_number; i++) {
        memset(&system_cfg, 0, sizeof(system_config));
        strcpy(content, original_content);
        ParseContent(content, i, 0, Map_system_variable_2);
        memcpy(global_par.system_par.business_ip_addr_array[i], system_cfg.business_ip_address, strlen(system_cfg.business_ip_address));
    }

    /*get the database user name and password for master and slave database*/
    for (i=0; i<system_par.database_user_number; i++) {
        memset(&system_cfg, 0, sizeof(system_config));
        strcpy(content, original_content);
        ParseContent(content, i, 0, Map_system_variable_3);
        memcpy(global_par.system_par.database_user[i], system_cfg.database_user, strlen(system_cfg.database_user));
        memcpy(global_par.system_par.database_password[i], system_cfg.database_password, strlen(system_cfg.database_password));
    }

    /*get the database user name and password for verify database*/
    for (i=0; i<system_par.verify_database_user_number; i++) {
        memset(&system_cfg, 0, sizeof(system_config));
        strcpy(content, original_content);
        ParseContent(content, i, 0, Map_system_variable_4);
        memcpy(global_par.system_par.verify_database_user[i], system_cfg.verify_database_user, strlen(system_cfg.verify_database_user));
        memcpy(global_par.system_par.verify_database_password[i], system_cfg.verify_database_password, strlen(system_cfg.verify_database_password));
    }

    free(content);/*free memory that malloc in GetConfigFileContent()*/
    free(original_content);
    content = NULL;
    original_content = NULL;

    return 1;
}

/************************************************************************
 *  \brief
 *     Parse the command line parameters(company level) and read the config files.
 *
 *  \par Input:
 *    None
 *
 *  \Return:
 *	  1: success
 *    0: error
************************************************************************/
int ReadConfigCompanyLevel(int company_index)
{
    char *content = NULL;

    memset(&individual_company_cfg, 0, sizeof(individual_company_config));
    printf("Parsing Configfile: %s\n", global_cfg.company_cfg_array[company_index].company_cfg_filename);

    /*Get the content from file*/
    content = GetConfigFileContent(global_cfg.company_cfg_array[company_index].company_cfg_filename);

    ParseContent(content, 0 , 1, Map_individual_company);

    global_par.company_par_array[company_index].use_middleware = individual_company_cfg.use_middleware;
    memcpy(global_par.company_par_array[company_index].packet_encoding, individual_company_cfg.packet_encoding, strlen(individual_company_cfg.packet_encoding));

    free(content);/*free memory that malloc in GetConfigFileContent()*/
    content = NULL;

    return 1;
}

/************************************************************************
 *  \brief
 *     Parse the command line parameters(network level) and read the config files.
 *
 *  \par Input:
 *    None
 *
 *  \Return:
 *	  1: success
 *    0: error
************************************************************************/
int ReadConfigNetworkLevel(int company_index)
{
    char *content = NULL;

    memset(&network_par, 0, sizeof(network_parameters));
    printf("Parsing Configfile: %s\n", individual_company_cfg.network_config_filename);

    /*Get the content from file*/
    content = GetConfigFileContent(individual_company_cfg.network_config_filename);

    ParseContent(content, 0 , 1, Map_network);

    memcpy(&global_par.company_par_array[company_index].network_par, &network_par, sizeof(network_parameters));

    free(content);/*free memory that malloc in GetConfigFileContent()*/
    content = NULL;

    return 1;
}

/************************************************************************
 *  \brief
 *     Parse the command line parameters(packet level) and read the config files.
 *
 *  \par Input:
 *    None
 *
 *  \Return:
 *	  1: success
 *    0: error
************************************************************************/
int ReadConfigPacketLevel(int company_index)
{
    char *content = NULL;
    char *original_content = NULL;
    int i = 0, j = 0;

    memset(&packet_number_cfg, 0, sizeof(packet_number_config));
    printf("Parsing Configfile: %s\n", individual_company_cfg.packet_config_filename);

    /*Get the content from file*/
    content = GetConfigFileContent(individual_company_cfg.packet_config_filename);
    original_content = (char *)malloc(strlen(content)+1);
    strcpy(original_content, content);

    ParseContent(content, 0 , 1, Map_packet_fix);

    global_par.company_par_array[company_index].packet_count = packet_number_cfg.packet_number;

    //global_par.company_par_array[company_index].pkt_par_array =
    //	(packet_parameters **)malloc(packet_number_cfg.packet_number * sizeof(packet_parameters *));
    for (i=0; i<packet_number_cfg.packet_number ; i++) {
        memset(&packet_cfg, 0, sizeof(packet_config));
        strcpy(content, original_content);
        ParseContent(content, i, 0, Map_packet_variable);
        strcpy(packet_cfg.packet_config_real_filename[FORWARD_POSITION], packet_cfg.packet_config_filename);
        strcat(packet_cfg.packet_config_real_filename[FORWARD_POSITION], SUFFIX_FORWARD_FILENAME);
        strcpy(packet_cfg.packet_config_real_filename[BACKWARD_POSITION], packet_cfg.packet_config_filename);
        strcat(packet_cfg.packet_config_real_filename[BACKWARD_POSITION], SUFFIX_BACKWARD_FILENAME);

        strcpy(global_par.company_par_array[company_index].packet_name[i], packet_cfg.packet_config_filename);
        global_par.company_par_array[company_index].packet_important_level[i] = packet_important_level_cfg.packet_important_level;

        //global_par.company_par_array[company_index].pkt_par_array[i]
        //	= (packet_parameters *)malloc(PACKET_POSITIONS * sizeof(packet_parameters));

        for (j=0; j<PACKET_POSITIONS; j++) {
            memset(&global_par.company_par_array[company_index].pkt_par_array[i][j], 0, sizeof(packet_parameters));
            ReadConfigItemLevel(company_index, j, i);
        }
    }

    free(content);/*free memory that malloc in GetConfigFileContent()*/
    free(original_content);
    content = NULL;
    original_content = NULL;

    return 1;
}

/************************************************************************
 *  \brief
 *     Parse the command line parameters(item level) and read the config files.
 *
 *  \par Input:
 *    None
 *
 *  \Return:
 *	  1: success
 *    0: error
************************************************************************/
int ReadConfigItemLevel(int company_index, int postion_index, int packet_index)
{
    char *content = NULL;
    char *original_content = NULL;
    int i = 0;

    memset(&item_num_cfg, 0, sizeof(item_num_config));
    printf("Parsing Configfile: %s\n",
           packet_cfg.packet_config_real_filename[postion_index]);

    /*Get the content from file*/
    content = GetConfigFileContent(packet_cfg.packet_config_real_filename[postion_index]);
    original_content = (char *)malloc(strlen(content)+1);
    strcpy(original_content, content);

    ParseContent(content, 0 , 1, Map_item_fix);

    global_par.company_par_array[company_index].pkt_par_array[packet_index][postion_index].item_count
    = item_num_cfg.item_count;

    //global_par.company_par_array[company_index].pkt_par_array[packet_index][postion_index].item_par_array
    //	= (item_parameters *)malloc(item_num_cfg.item_count * sizeof(item_parameters));
    memset(global_par.company_par_array[company_index].pkt_par_array[packet_index][postion_index].item_par_array,
           0, item_num_cfg.item_count * sizeof(item_parameters));

    //global_par.company_par_array[company_index].pkt_par_array[packet_index][postion_index].item_index
    //	= (int *)malloc(item_num_cfg.item_count * sizeof(int));
    //memset(global_par.company_par_array[company_index].pkt_par_array[packet_index][postion_index].item_index,
    //	0, item_num_cfg.item_count * sizeof(int));
    for (i=0; i<MAX_ITEM_NUMBER ; i++) {
        global_par.company_par_array[company_index].pkt_par_array[packet_index][postion_index].item_index[i] = -1;
    }

    for (i=0; i<item_num_cfg.item_count ; i++) {
        memset(&item_par, 0, sizeof(item_parameters));
        strcpy(content, original_content);
        ParseContent(content, i, 0, Map_item_variable);
        memcpy(&global_par.company_par_array[company_index].pkt_par_array[packet_index][postion_index].item_par_array[i],
               &item_par, sizeof(item_parameters));
    }

    free(content);/*free memory that malloc in GetConfigFileContent()*/
    free(original_content);
    content = NULL;
    original_content = NULL;

    return 1;
}

/************************************************************************
 *  \brief
 *     Read config from file
 *
 *  \par Input:
 *    char *filename : config file name
 *
 *  \Return:
 *	  1: success
 *    0: error
************************************************************************/
int ReadConfigAll(char *filename)
{
    int i = 0;

    ReadConfigGlobalLevel(filename);
    ReadConfigSystemLevel();

    for (i=0; i< global_cfg.company_number; i++) {
        ReadConfigCompanyLevel(i);
        ReadConfigNetworkLevel(i);
        ReadConfigPacketLevel(i);
    }

    /*Find the item index for each packet format*/
    FindItemIndexForEachPacketFormat(COMPANY_ITEM_NAME, strlen(COMPANY_ITEM_NAME), COMPANY_ITEM_INDEX);
    FindItemIndexForEachPacketFormat(SERVICE_TYPE_ITEM_NAME, strlen(SERVICE_TYPE_ITEM_NAME), SERVICE_TYPE_ITEM_INDEX);
    FindItemIndexForEachPacketFormat(MONEY_ITEM_NAME, strlen(MONEY_ITEM_NAME), MONEY_ITEM_INDEX);
    FindItemIndexForEachPacketFormat(SERIAL_NUMBER_ITEM_NAME, strlen(SERIAL_NUMBER_ITEM_NAME), SERIAL_NUMBER_ITEM_INDEX);
    FindItemIndexForEachPacketFormat(RESPOND_CODE_ITEM_NAME, strlen(RESPOND_CODE_ITEM_NAME), RESPOND_CODE_ITEM_INDEX);
    FindItemIndexForEachPacketFormat(LA_SERIAL_NUMBER_ITEM_NAME, strlen(LA_SERIAL_NUMBER_ITEM_NAME), LA_SERIAL_NUMBER_ITEM_INDEX);
    FindItemIndexForEachPacketFormat(LA_RECORD_DATE_ITEM_NAME, strlen(LA_RECORD_DATE_ITEM_NAME), LA_RECORD_DATE_ITEM_INDEX);

    /*Find localhost information*/
    GetLocalhostInformation();

    return 1;
}

/*************************************************************************
 *  \brief
 *    Find the item index for each packet format,
 *       the function is used after config at first in order to accelerate the parse procedure
 *
 *   find packet parameter match the item name,
 *
 *  \par Input:
 *     item_name: the string (must match the company item name).
 *	   item_name_len: the length of item_name.
 *     find_index: the fixed index by MARCO
 *  \par Output:
 *  \Return:
 *    1: success:find a suitable one.
 *    0: error:no such item
************************************************************************/
int FindItemIndexForEachPacketFormat(const char *item_name, int item_name_len, int find_index)
{
    int find_it = 0;
    int i = 0, j = 0, k = 0, l = 0;

    /*at first, find company index*/
    for (i=0;i<global_par.company_num;i++) {
        for (j=0;j<global_par.company_par_array[i].packet_count;j++) {
            for (l=0;l<PACKET_POSITIONS;l++) {
                find_it = 0;
                for (k=0;k<global_par.company_par_array[i].pkt_par_array[j][l].item_count;k++) {
                    if (0 == strcmp(item_name, global_par.company_par_array[i].pkt_par_array[j][l].item_par_array[k].name)) {
                        find_it = 1;
                        global_par.company_par_array[i].pkt_par_array[j][l].item_index[find_index] = k;
                    }
                }
            }
        }
    }

    return 1;
}

/*************************************************************************
 *  \brief
 *    Get the local machine information including IP address and localhost id
 *
 *   use ioctl to get system network configuration
 *
 *  \par Input:
 *  \par Output:
 *  \Return:
 *    1: success
 *    0: error
************************************************************************/
int GetLocalhostInformation(void)
{
    int fd, intrface;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    int i = 0;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)) {
            intrface = ifc.ifc_len / sizeof(struct ifreq);
	     for(int net_id=0;net_id<intrface;net_id++){
	     		/*Get     IP     of     the     net     card     */
            		if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[net_id]))) {
                		//puts(inet_ntoa(((struct sockaddr_in *)(&buf[intrface-1].ifr_addr))->sin_addr));
                		strcpy(global_par.system_par.localhost_ip_address, inet_ntoa(((struct sockaddr_in *)(&buf[net_id].ifr_addr))->sin_addr));
                		printf("LOCALHOST IP ADDRESS IS %s\n", global_par.system_par.localhost_ip_address);
                		if (0 == strcmp(global_par.system_par.localhost_ip_address, global_par.system_par.database_self_ip_address)) {
                    			strcpy(global_par.system_par.localhost_id, "DA");
                   		 	printf("LOCALHOST ID IS DA\n");
                    			goto end;
               		}

                		if (0 == strcmp(global_par.system_par.localhost_ip_address, global_par.system_par.database_brother_ip_address)) {
                    			strcpy(global_par.system_par.localhost_id, "DB");
                   			printf("LOCALHOST ID IS DB\n");
                    			goto end;
                		}

                		if (0 == strcmp(global_par.system_par.localhost_ip_address, global_par.system_par.database_invoice_ip_address)) {
                    			strcpy(global_par.system_par.localhost_id, "DI");
                    			printf("LOCALHOST ID IS DI\n");
                    			goto end;
                		}


                		for (i=0; i< global_par.system_par.proxy_number; i++) {
                    			if (0 == strcmp(global_par.system_par.localhost_ip_address, global_par.system_par.proxy_ip_addr_array[i])) {
                        			sprintf(global_par.system_par.localhost_id, "P%d", i);
                        			printf("LOCALHOST ID IS P%d\n", i);
                        			goto end;
                    			}
                		}

                		for (i=0; i< global_par.system_par.business_number; i++) {
                    			if (0 == strcmp(global_par.system_par.localhost_ip_address, global_par.system_par.business_ip_addr_array[i])) {
                        			sprintf(global_par.system_par.localhost_id, "B%d", i);
                        			printf("LOCALHOST ID IS B%d\n", i);
                        			goto end;
                    			}
                		}

		  		for (i=0; i< global_par.system_par.verify_number; i++) {
                    			if (0 == strcmp(global_par.system_par.localhost_ip_address, global_par.system_par.verify_ip_addr_array[i])) {
                        			sprintf(global_par.system_par.localhost_id, "V%d", i);
                       			printf("LOCALHOST ID IS V%d\n", i);
                        			goto end;
                    			}
                		}

                		if((intrface-1)==net_id){//last netcard ip adress
					strcpy(global_par.system_par.localhost_id, "NO");
                			printf("THE IP ADDRESS or SYSTEM CONFIGURE is ERROR!!!, PLEASE RECONFIG THEM!!!\n");
                			exit(1);
                		}

            		} else {
              		char str[256];
                		sprintf(str, "cpm:     ioctl     device  %s", buf[net_id].ifr_name);
                		perror(str);
           		}
		}
        } else
            perror("cpm:     ioctl");
    } else
     perror("cpm:     socket");

    close(fd);
end:
    return 1;
}
