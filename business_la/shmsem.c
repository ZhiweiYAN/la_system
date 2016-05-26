/*********************************************************
 *project: Line communication charges supermarket
 *filename: shmsem.cc
 *version: 0.4
 *purpose: prototype of One semphore and one block memory
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#include "shmsem.h"

/*************************************************************************
 *  \brief
 *    malloc the share memory
 *
 *   create the shm at fisrt time
 *
 *  \par Input:
 *     mem_size: the size of the share memory.
 *     id: the offset of key_t, note only low 8 bit is used.
 *  \par Output:
 *  \Return:
 *    the start address of the share memory
************************************************************************/
void *InitialShm(size_t mem_size, int id)
{
   /*varibles for create share memory*/
   int oflag_shm;
   int shmid; 
   char *mem_ptr;
   char *path = NULL;
   
	/*check the parameter vaild*/
	if(mem_size <= 0)
	{
		perror("error@shmsem.c:InitialShm:memsize is not correct!!!\n");
		exit(1);
	}

   /*Get the share memery handle*/
   path = getenv("HOME");
   oflag_shm = IPC_CREAT|0666;
   
   if((shmid = shmget(ftok(path,(id%256)),mem_size,oflag_shm)) == -1)
   {
   		perror("error@shmsem.c:InitialShm:shmget\n");
			printf("shmid = %d, mem_size = %d, key_t = %d", shmid, mem_size,ftok(path,(id%256)));
   		return NULL;
   }
   
   /*map the share memery to main process space*/
   mem_ptr = (char *)shmat(shmid,0,0);
   if((void *)mem_ptr == (void *)-1)
   {
   		perror("error@shmsem.c:InitialShm:shmat\n");
   		return NULL;
   }
	
	/*clear the memory content at the first time*/
	bzero(mem_ptr, mem_size);

   /*chang memptr type to (void *)*/ 
   return  (void *)mem_ptr;
}

/*************************************************************************
 *  \brief
 *    malloc the share semphore
 *
 *   create the semphore at fisrt time
 *
 *  \par Input:
 *     id: the offset of key_t, note only low 8 bit is used.     
 *  \par Output:
 *      
 *  \Return:
 *    1: success
 *    0: fail
************************************************************************/
int InitialSem(int id)
{
   /*varibles for create semphore and share memory*/
   int oflag_sem;
   int semid;
   char *path = NULL;
   
   /*Get the semphore handle*/
   path = getenv("HOME");
   oflag_sem = IPC_CREAT|0666;
   if((semid = semget(ftok(path,(id%256)),1,oflag_sem)) == -1)
   {
   		perror("error@shmsem.c:InitialSem:semget\n");
   		return -1;
   }
   
	/* Set semphore default value 1 */
#ifdef  DEBUG_OBSERVE_SEMPHORE
   printf("PID:%d are setting init value of semphore, the sem val: %d\n", getpid(), semctl(semid,0,GETVAL,0));
#endif
   semctl(semid, 0, SETVAL, 1);
#ifdef  DEBUG_OBSERVE_SEMPHORE
   printf("PID:%d has set init value of semphore, the sem val: %d\n", getpid(), semctl(semid,0,GETVAL,0));
#endif

   return 1;
}

/* ************************************************* 
 * Function Name: DestroyShmSem
 * *************************************************/
int DestroyShmSem(void)
{
	return 1;
}

/*************************************************************************
 *  \brief
 *    acquire the handle of exsited semphore (new version)
 *
 *   use semget(semid,0,0); 
 *
 *  \par Input:
 *     id: the offset of key_t, note only low 8 bit is used.     
 *  \par Output:
 *      
 *  \Return:
 *    the id of the existed semphore
************************************************************************/
int GetExistedSemphoreExt(int id)
{
   /*varibles for get existed semphore*/
   int semid;
   char *path = NULL;
   path = getenv("HOME");
           
   /*Get the existed semphore handle*/
   if((semid = semget(ftok(path,(id%256)),0,0)) == -1)
   {
   		perror("error@shmsem.c:GetExistedSemphoreExt:semget\n");
   		exit(1);
   }
   
   return semid;
}

/*************************************************************************
 *  \brief
 *    acquire the right to access the shared socket information
 *
 *   this is a blocking fuction, it wait until acquire the semphore
 *
 *  \par Input:
 *       semid: the handle of semphore
 *  \par Output:
 *      
 *  \Return:
 *    1: success
 *    0: fail
************************************************************************/
int AcquireAccessRight(int semid)
{
   struct sembuf ops; /*the operater data structure for semop*/
   
   /*make the acquire operator*/
   ops.sem_num = 0;
   ops.sem_op = -1;   /*substract 1 from semval*/
   ops.sem_flg = SEM_UNDO; 
        
   /*acquired the access right*/
   /*It will block the process and wait untill ...*/
#ifdef  DEBUG_OBSERVE_SEMPHORE
   printf("PID:%d are acquiring sem, the sem val: %d\n", getpid(), semctl(semid,0,GETVAL,0));
#endif
   semop(semid, &ops, 1);
#ifdef  DEBUG_OBSERVE_SEMPHORE
   printf("PID:%d has acquired sem, the sem val: %d\n", getpid(), semctl(semid,0,GETVAL,0));
#endif
   return 1;
}

/*************************************************************************
 *  \brief
 *    release the right to access the shared socket information
 *
 *   use semop();
 *
 *  \par Input:
 *     
 *  \par Output:
 *      
 *  \Return:
 *    1: success
 *    0: fail
************************************************************************/
int ReleaseAccessRight(int semid)
{
   struct sembuf ops; /*the operater data structure for semop*/
   
   /*make the release operator*/
   ops.sem_num = 0;
   ops.sem_op = 1;   /*add 1 to semval*/
   ops.sem_flg = SEM_UNDO;       
   
   /*released the access right*/
   /*It will excute over right now*/
#ifdef  DEBUG_OBSERVE_SEMPHORE
   printf("PID:%d are releasing sem, the sem val: %d\n", getpid(), semctl(semid,0,GETVAL,0));
#endif
   semop(semid, &ops, 1);
#ifdef  DEBUG_OBSERVE_SEMPHORE
   printf("PID:%d has released sem, the sem val: %d\n", getpid(), semctl(semid,0,GETVAL,0));
#endif     
  
   return 1;
}

/*************************************************************************
 *  \brief
 *    mapping the existed share memeoy in own space of process (New version)
 *
 *  \par Input:
 *     id: the offset of key_t, note only low 8 bit is used.     
 *  \par Output:
 *      
 *  \Return:
 *    the start address of the existed share memory
************************************************************************/
void* MappingShareMemOwnSpaceExt(int id)
{
   /*varibles for get existed share memory*/
   int shmid;
   void *mem_ptr = NULL;
   char *path = NULL;
   path = getenv("HOME");
      
   /*Get the existed share memory handle*/
   if((shmid = shmget(ftok(path,(id%256)),0,0)) == -1)
   {
   		perror("error@shmsem.c:MappingShareMemOwnSpaceExt:shmget\n");

   		exit(1);
   }
   
   /*mapping the share memery to own process space*/
   if((mem_ptr = shmat(shmid,NULL,0)) == (void *)-1)
   {
   		perror("error@shmsem.c:MappingShareMemOwnSpaceExt:shmat\n");
   		exit(1);
   }
        
   return mem_ptr;
}

/*************************************************************************
 *  \brief
 *    upmapping the existed share memeoy 
 *
 *   use semop();
 *
 *  \par Input:
 *     
 *  \par Output:
 *      
 *  \Return:
 *    1: success
 *    0: fail
************************************************************************/
int UnmappingShareMem(void *mem_ptr)
{
         
   /*unmapping the existed share memory*/
   if(shmdt(mem_ptr) == -1)
   {
   		perror("error@shmsem.c:UnmappingShareMem:shmdt\n");
   		return -1;
   }
   else
   		return 1;
}

/*************************************************************************
 *  \brief
 *    destruction the semphore
 *
 *   delete the semphore from the core
 *
 *  \par Input:
 *     id: the offset of key_t, note only low 8 bit is used.     
 *  \par Output:
 *      
 *  \Return:
 *    1: success
 *    0: fail
************************************************************************/
extern int errno;
int DestructionSem(int id)
{
   /*varibles for create semphore and share memory*/
   int oflag_sem;
   int semid;
   char *path = NULL;
   /*Get the semphore handle*/
   path = getenv("HOME");
   oflag_sem = IPC_CREAT|0666;
   printf("The ID : %d semphore will be destructed from core!!!!\n", id);
   if((semid = semget(ftok(path,(id%256)),1,oflag_sem)) == -1)
   {
		printf("The errno is %d, EACCES is %d\n", errno, EACCES);
   }	
	   
	if(semctl(semid, 0, IPC_RMID) == -1)
	{
		printf("The errno is %d, EINVAL is %d\n", errno, EINVAL);
	}
	else
	{
		printf("The ID : %d semphore is destructed from core successfully!!!!\n", id);
	}

   return 1;
}
