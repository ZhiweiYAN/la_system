/*********************************************************
 *project: Line communication charges supermarket
 *filename: shmsem.h
 *version: 0.3
 *purpose: prototype of One semphore and one block memory
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2006-12-26
 *********************************************************/
#ifndef SHMSEM_H
#define SHMSEM_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "primary_db_common.h"

void *InitialShm(size_t mem_size, int id);
int InitialSem(int id);
int InitialShmSem(size_t mem_size, int sh_id);
int GetExistedSemphore(int sh_id);
int GetExistedSemphoreExt(int id);
int AcquireAccessRight(int semid);
int ReleaseAccessRight(int semid);
void* MappingShareMemOwnSpace(int sh_id);
void *MappingShareMemOwnSpaceExt(int id);
int UnmappingShareMem(void *mem_ptr);
int DestroyShmSem(int sh_id);

#endif
