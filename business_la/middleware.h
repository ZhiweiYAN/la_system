/*********************************************************
 *project: Line communication charges supermarket
 *filename: middleware.h
 *version: 0.5
 *purpose: some function have relationship with middleware
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-3-8
 *********************************************************/
 
#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "common.h"

int TransmitPacketWithMiddleware(char *input_buf, int input_buf_len, char *output_buf, int *output_buf_len, int enterprise_code, int service_type);

#endif
