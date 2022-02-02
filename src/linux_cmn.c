/*__________________________________   linux-cmn.c   ___________________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
 *   Copyright (c) 2007  Seiko Epson Corporation                 All rights reserved.
 *
 *   Copyright protection claimed includes all forms and matters of
 *   copyrightable material and information now allowed by statutory or judicial
 *   law or hereinafter granted, including without limitation, material generated
 *   from the software programs which are displayed on the screen such as icons,
 *   screen display looks, etc.
 *
 */
/*******************************************|********************************************/
/*                                                                                      */
/*                      Linux platform depend External Definitions                      */
/*                                                                                      */
/*                                    Function Calls                                    */
/*                              --------------------------                              */
/*                  void*		epsmpMemAlloc	(size								);  */
/*                  void		epsmpMemFree	(memblock							);  */
/*                  EPS_INT32   epsmpSleep		(time								);  */
/*                  EPS_UINT32  epsmpGetTime	(void								);  */
/*                  EPS_INT32   epsmpLockSync	(void								);  */
/*                  EPS_INT32   epsmpUnlockSync (void								);  */
/*                                                                                      */
/*******************************************|********************************************/

#if defined (unix) || (__unix)
/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-def.h"
#include "epson-escpr-err.h"
#include "linux_cmn.h"
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h> /* struct timeval, select() */
/* ICANON, ECHO, TCSANOW, struct termios */
#include <termios.h> /* tcgetattr(), tcsetattr() */
#include <stdlib.h> /* atexit(), exit() */
#include <unistd.h> /* read() */
#include <stdio.h> /* printf() */
#include <pthread.h>

/*------------------------------  Local Grobal Variables   -----------------------------*/
/*******************************************|********************************************/
static struct termios g_old_kbd_mode;
static pthread_mutex_t g_mutex;

/*----------------------------------  Local Functions  ---------------------------------*/
/*******************************************|********************************************/
static void cooked(void);
static void raw(void);

/*--------------------------------  Function Declarations   ----------------------------*/
/*******************************************|********************************************/

/*******************************************|********************************************/
/* Function name:   epsmpMemAlloc()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* s            size_t              I: Number of bytes to allocate, in bytes.           */
/*                                                                                      */
/* Return value:                                                                        */
/*      pointer to new buffer                                                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Dynamically allocates a memory buffer.                                          */
/*                                                                                      */
/*******************************************|********************************************/
void* epsmpMemAlloc(
					
			size_t s
			
){
	return malloc(s);
}


/*******************************************|********************************************/
/* Function name:   epsmpMemFree()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* p            void*               I: Block of memory to free.                         */
/*                                                                                      */
/* Return value:                                                                        */
/*  N/A                                                                                 */
/*                                                                                      */
/* Description:                                                                         */
/*      Frees an allocated block of memory allocated in the xxxMemAlloc () function.    */
/*                                                                                      */
/*******************************************|********************************************/
void epsmpMemFree(
				  
			void* p
			
){
	free(p);
}


/*******************************************|********************************************/
/* Function name:   epsmpSleep()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* t            void*               I: Interval (specified in microseconds)             */
/*                                                                                      */
/* Return value:                                                                        */
/*      0               - Success                                                       */
/*      -1              - Failure                                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      Pauses execution in the current thread for a specified number of microseconds.  */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32 epsmpSleep(
					 
			EPS_UINT32 t
			
){
	usleep( t);
	return 0;
}


/*******************************************|********************************************/
/* Function name:   epsmpGetTime()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      System time                                                                     */
/*                                                                                      */
/* Description:                                                                         */
/*      The time of the system is acquired.                                             */
/*                                                                                      */
/*******************************************|********************************************/
EPS_UINT32 epsmpGetTime(void)
{
	struct tms tm;
	return (times(&tm)*10);
}


/*******************************************|********************************************/
/* Function name:   epsmpGetLocalTime()                                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* epsTime      EPS_LOCAL_TIME*     O: time structure.                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      0                                                                               */
/*                                                                                      */
/* Description:                                                                         */
/*      get the system localtime.                                                       */
/*                                                                                      */
/*******************************************|********************************************/
EPS_UINT32 epsmpGetLocalTime(EPS_LOCAL_TIME *epsTime)
{
	epsTime->year =  1900;
	epsTime->mon =  1;
	epsTime->day = 1;
	epsTime->hour = 1;
	epsTime->min = 1;
	epsTime->sec = 1;

	return 0;
}


/*******************************************|********************************************/
/* Function name:   epsmpLockSync()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      0               - Success                                                       */
/*      -1              - Failure                                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      Lock the synchronization object.                                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32 epsmpLockSync(void)
{
	return pthread_mutex_lock(&g_mutex);
}


/*******************************************|********************************************/
/* Function name:   epsmpUnlockSync()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      0               - Success                                                       */
/*      -1              - Failure                                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      UnLock the synchronization object.                                              */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32 epsmpUnlockSync(void)
{
	return pthread_mutex_unlock(&g_mutex);
}



static void cooked(void)
{
	tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}

static void raw(void)
{
	static char init;
	struct termios new_kbd_mode;

	if(init)
		return;
	tcgetattr(0, &g_old_kbd_mode);
	memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
	new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
	new_kbd_mode.c_cc[VTIME] = 0;
	new_kbd_mode.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &new_kbd_mode);
	atexit(cooked);

	init = 1;
}

EPS_INT32 _kbhit(void)
{
	struct timeval timeout;
	fd_set read_handles;
	int status;

	raw();
	FD_ZERO(&read_handles);
	FD_SET(0, &read_handles);
	timeout.tv_sec = timeout.tv_usec = 0;
	status = select(0 + 1, &read_handles, NULL, NULL, &timeout);
	tcsetattr(0, TCSANOW, &g_old_kbd_mode);
	if(status < 0)
	{
		printf("select() failed in kbhit()\n");
		exit(1);
	}
	return status;
}

EPS_INT32 _getch(void)
{
	unsigned char temp;

	raw();
	if(read(0, &temp, 1) != 1){
		tcsetattr(0, TCSANOW, &g_old_kbd_mode);
		return 0;
	}
	tcsetattr(0, TCSANOW, &g_old_kbd_mode);
	return temp;
}


void _endthread(void)
{
	pthread_exit(NULL);
}


void InitMutex(void)
{
	pthread_mutex_init(&g_mutex, NULL); /*default (fast) mutex*/
}

void TermMutex(void)
{
	pthread_mutex_destroy(&g_mutex);
}


#endif

/*__________________________________   linux-cmn.c   ___________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
