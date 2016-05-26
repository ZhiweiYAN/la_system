/*********************************************************
 *project: Line communication charges supermarket
 *filename: shmsem.h
 *version: 0.4
 *purpose: prototype of One semphore and one block memory
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#ifndef SHMSEM_H
#define SHMSEM_H

#include "bk_common.h"

void *InitialShm(size_t mem_size, int id);
int InitialSem(int id);
int GetExistedSemphoreExt(int id);
int AcquireAccessRight(int semid);
int ReleaseAccessRight(int semid);
void *MappingShareMemOwnSpaceExt(int id);
int UnmappingShareMem(void *mem_ptr);
int DestroyShmSem(void);
int DestructionSem(int id);

#endif
