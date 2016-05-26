/*********************************************************
 *project: Line communication charges supermarket
 *filename: parse_pkt_common.c
 *version: 0.1
 *purpose: some common function have relationship with parse packet
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-22
 *********************************************************/

#include "parse_pkt_common.h"

/*************************************************************************
 *  \brief
 *    Find the company which the packet is belonged to,
 *       return their index
 *
 *   use global parameter (all pkt type) match the packet, find suitable one
 *
 *  \par Input:
 *     pkt: the pointer of buffer contained the packet to be parsed.
 *     pkt_len: the length of the packet.
 *     pkt_postion: the direction of packet transmited (0:forward, 1:backward)
 *  \par Output:
 *     company_index: the index of company.
 *  \Return:
 *    1: success:find a suitable one.
 *    0: error:no such type packet
************************************************************************/
int FindCompanyIndexFromPacket(char *pkt, int pkt_len, int pkt_postion, int *company_index)
{
    int success = 0, find_it = 0;
    int i = 0, j = 0;
    int company_item_index = 0;
    /*	int service_type_item_index = 0; */
    char real_valid_value[ITEM_VALIDE_VALUE_LENGTH];
    char ideal_valid_value[ITEM_VALIDE_VALUE_LENGTH];
    int direction = 0;
    int start_pos = 0;
    int len = 0;

    /*at first, find company index*/
    find_it = 0;
    for (i=0; i<global_par.company_num; i++) {
        for (j=0; j<global_par.company_par_array[i].packet_count; j++) {
            company_item_index = global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_index[COMPANY_ITEM_INDEX];
            direction = global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_par_array[company_item_index].direction;
            start_pos = global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_par_array[company_item_index].start_pos;
            len = global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_par_array[company_item_index].len;
            memset(ideal_valid_value, 0, ITEM_VALIDE_VALUE_LENGTH);
            memset(real_valid_value, 0, ITEM_VALIDE_VALUE_LENGTH);
            memcpy(ideal_valid_value, global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_par_array[company_item_index].valid_value, len);
            if (0 == direction) {
                memcpy(real_valid_value, pkt+start_pos, len);
            } else {
                memcpy(real_valid_value, pkt+pkt_len-start_pos, len);
            }
            if (0 == memcmp(ideal_valid_value, real_valid_value, len)) {
                find_it = 1;
                goto FIND_COMPANY;
            }
        }
    }
    if (0 == find_it) {
        perror("No suitable company match this packet!!!\n");
        success = 0;
        return success;
    }

FIND_COMPANY:
    *company_index = i;
    success = 1;
    return success;

}

/*************************************************************************
 *  \brief
 *    Find the service type which the packet is belonged to,
 *       return their index
 *
 *   use global parameter (all pkt type) match the packet, find suitable one
 *
 *  \par Input:
 *     pkt: the pointer of buffer contained the packet to be parsed.
 *     pkt_len: the length of the packet.
 *     pkt_postion: the direction of packet transmited (0:forward, 1:backward)
 *     company_index: the index of company.
 *  \par Output:
 *     service_type_index: the index of service type.
 *  \Return:
 *    1: success:find a suitable one.
 *    0: error:no such type packet
************************************************************************/
int FindServiceTypeIndexFromPacket(char *pkt, int pkt_len, int pkt_postion, int company_index, int *service_type_index)
{
    int success = 0, find_it = 0;
    int i = 0, j = 0;
    int service_type_item_index = 0;
    char real_valid_value[ITEM_VALIDE_VALUE_LENGTH];
    char ideal_valid_value[ITEM_VALIDE_VALUE_LENGTH];
    int direction = 0;
    int start_pos = 0;
    int len = 0;

    i = company_index;

    find_it = 0;
    /*then find service type index*/
    for (j=0; j<global_par.company_par_array[i].packet_count; j++) {
        service_type_item_index = global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_index[SERVICE_TYPE_ITEM_INDEX];
        direction = global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_par_array[service_type_item_index].direction;
        start_pos = global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_par_array[service_type_item_index].start_pos;
        len = global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_par_array[service_type_item_index].len;
        memset(ideal_valid_value, 0, ITEM_VALIDE_VALUE_LENGTH);
        memset(real_valid_value, 0, ITEM_VALIDE_VALUE_LENGTH);
        memcpy(ideal_valid_value, global_par.company_par_array[i].pkt_par_array[j][pkt_postion].item_par_array[service_type_item_index].valid_value, len);
        if (0 == direction) { //items directions, 0: positive; 1:negtive.
            memcpy(real_valid_value, pkt+start_pos, len);
        } else {
            memcpy(real_valid_value, pkt+pkt_len-start_pos, len);
        }
        if (0 == memcmp(ideal_valid_value, real_valid_value, len)) {
            find_it = 1;
            goto FIND_SERVICE_TYPE;
        }
    }
    if (0 == find_it) {
        perror("No suitable service type match this packet!!!\n");
        success = 0;
        return success;
    }
FIND_SERVICE_TYPE:
    *service_type_index = j;

    success = 1;
    return success;
}


int Fill_serial_number_to_packet(char *pkt, int pkt_postion, int company_index, int service_type_index, char *serial_number)
{
    int serial_number_item_index = 0;
    int direction = 0;
    int start_pos = 0;
    /*	int len = 0;*/
    int pkt_len = strlen(pkt);

    serial_number_item_index = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_index[SERIAL_NUMBER_ITEM_INDEX];
    direction = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[serial_number_item_index].direction;
    start_pos = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[serial_number_item_index].start_pos;
    //len = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[service_type_item_index].len;
    if (0 == direction) { //items directions, 0: positive; 1:negtive.
        memcpy(pkt+start_pos, serial_number, strlen(serial_number));
    } else {
        memcpy(pkt+pkt_len-start_pos, serial_number, strlen(serial_number));
    }

    return 1;
}

int	Get_money_from_packet(char *pkt, int pkt_postion, int company_index, int service_type_index, char *money)
{
    int charge_money_item_index = 0;
    int direction = 0;
    int start_pos = 0;
    int len = 0;
    int pkt_len = strlen(pkt);

    charge_money_item_index = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_index[MONEY_ITEM_INDEX];
    direction = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[charge_money_item_index].direction;
    start_pos = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[charge_money_item_index].start_pos;
    len = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[charge_money_item_index].len;
    if (0 == direction) { //items directions, 0: positive; 1:negtive.
        memcpy(money, pkt+start_pos, len);
    } else {
        memcpy(money, pkt+pkt_len-start_pos, len);
    }

    return 1;
}
