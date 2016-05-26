/*********************************************************
 *project: Line communication charges supermarket
 *filename: middleware.c
 *version: 0.5
 *purpose: some function have relationship with middleware
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-3-8
 *********************************************************/

#include "middleware.h"

/*************************************************************************
 *  \brief
 *    retransmit the recieved packet to enterprise server with middleware
 *    
 *   1. here is not implemented.
 *
 *  \par Input:
 *     input_buf: the buffer contain the input packet.
 *     input_buf_len: the length of the input_buf.
 *     enterprise_code: the company server type (ChinaUnicom or ChinaMobile).
 *  \par Output:
 *     output_buf: the buffer contain the output packet.
 *     output_buf_len: the length of the output_buf.
 *  \Return:
 *    1: success
 *    0: error
************************************************************************/
int TransmitPacketWithMiddleware(char *input_buf, int input_buf_len, char *output_buf, int *output_buf_len, int enterprise_code, int service_type)
{
	int success;
	/*accoding to company_index handle the transaction */
	switch(enterprise_code)
	{
		default:
			break;
	}	
	success = 1;
	return success;
}

