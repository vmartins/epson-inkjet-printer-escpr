#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<pthread.h>
#include	<string.h>
#include    <pthread.h>
#include 	"xfifo.h"
#include	"epson-typedefs.h"


//---------------------
// dependency of language or compiler.
//
#define	booleanRef			EPS_BOOL
#define	boolean_true		TRUE
#define	boolean_false		FALSE



//---------------------
#ifdef __cplusplus
extern "C" {
#endif


#define	kSleepShortWait					2
#define	kXFIFOWriteLimtWaitMilliSec		100


#define	kRingQueue1BlockSize			(1024*4)
#define	kRingQueueNumBlocks				(1024*1024*5/kRingQueue1BlockSize)




//---------------------
//	RingQueueContext Define
//---------------------

typedef enum {
	CanReadResult_OK,
	CanReadResult_Empty,
	CanReadResult_Busy
} rqCanReadResult;


typedef struct _rqElement {
	struct _rqElement*		next;
	char*					addr;
	int						pos;
} rqElement;


typedef struct {
	int					m_blocksize;
	rqElement*			m_wp;				// current  write element
	rqElement*			m_rp;
	booleanRef			m_setFlush;
	int					m_signalsize;
	int					m_total;
} RingQueueContext;


//---------------------
//	XThread Define
//---------------------
enum {
	kXThreadRunning_Default	= 0,
	kXThreadRunning_Start,
	kXThreadRunning_Stop
};

typedef struct
{
	pthread_mutex_t			m_threadMutex;
	pthread_cond_t 			m_threadCond;
    pthread_t 				m_thread;
	void*					m_refcon;

	volatile booleanRef		m_Error;
	volatile booleanRef		m_Opened;
	volatile booleanRef		m_Waiting;
    volatile booleanRef		m_Signal;
    volatile booleanRef		m_StopRequest;
	volatile booleanRef		m_Cancel;
    volatile int			m_Running;
} XThreadContext;



//---------------------
//	XFIFo define
//---------------------
typedef struct
{
	RingQueueContext*	queue;
	XThreadContext*		thread;
	FILE*				fp;
} XFIFOContext;



//---------------------
//	RingQueueContext Functions
//---------------------
RingQueueContext*	RingQueueOpen			(int blocksize, int numBlocks);
void				RingQueueClose			(RingQueueContext* queue);

int					RingQueueWrite			(RingQueueContext* queue, char* src, int requestSize);
int					RingQueueReadStart		(RingQueueContext* queue, char** buf, int *size);
void				RingQueueReadEnd		(RingQueueContext* queue);
int					RingQueueSetFlush		(RingQueueContext* queue);
booleanRef			RingQueueCanSignal		(RingQueueContext* queue);
booleanRef			RingQueueIsEmpty		(RingQueueContext* queue);
rqCanReadResult		RingQueueCanReadStart	(RingQueueContext* queue);
void				RingQueueBufferClear	(RingQueueContext* queue);
booleanRef			RingQueueCanSignal		(RingQueueContext* queue);


//---------------------
//	XThread Functions
//---------------------
typedef void* (*XThreadHandlerProcPtr)(void *arg);

XThreadContext*		XThreadInitialize(void);
void				XThreadTerminate	(XThreadContext* context);
int					XThreadOpen			(XThreadContext* context, XThreadHandlerProcPtr handler, void* refcon);
void				XThreadClose		(XThreadContext* context);
void				XThreadLock			(XThreadContext* context);
void				XThreadUnlock		(XThreadContext* context);
void				XThreadSignalAsync	(XThreadContext* context);
void				XThreadWaitAsync	(XThreadContext* context);
booleanRef			XThreadIsOpen		(XThreadContext* context);
booleanRef			XThreadIsClosing	(XThreadContext* context);
void				XThreadStart		(XThreadContext* context);
void				XThreadStop			(XThreadContext* context);
booleanRef			XThreadIsRunning	(XThreadContext* context);

//---------------------
//	XFIFo Functions
//---------------------
void XFIFOFlush(void* instance);
void* XThreadTask(void* refcon);


#ifdef __cplusplus
}
#endif




//--------------------------------------------------
#pragma mark -
//--------------------------------------------------
void*	XFIFOOpen(void)
{
	int				err		= -1;
	XFIFOContext*	context	= (XFIFOContext*)malloc(sizeof(XFIFOContext));

	if( context ){
		context->queue	= NULL;
		context->thread	= NULL;
		context->fp		= stdout;

		context->queue = RingQueueOpen(kRingQueue1BlockSize, kRingQueueNumBlocks);
		if( context->queue ){
			context->thread = XThreadInitialize();
			if( context->thread ){
				err = XThreadOpen(context->thread, XThreadTask, context);
				if( context->thread ){
					err = 0;
				}
			}
		}
	}
	if( err != 0 ){
		XFIFOClose((void**)&context);
	}
	return context;
}
//--------------------------------------------------
void	XFIFOClose(void** instancePtr)
{
	if( instancePtr  &&  *instancePtr ){
		XFIFOContext*	context = (XFIFOContext*)(*instancePtr);

		if( XThreadIsOpen(context->thread) == boolean_true ){
			XFIFOFlush(context);
		}

		XThreadClose(context->thread);
		RingQueueClose(context->queue);

		context->thread	= NULL;
		context->queue	= NULL;

		free(context);
		
		*instancePtr = NULL;
	}
}
//--------------------------------------------------
int	XFIFOWrite(void* instance, char* buffer, int requestSize)
{
	int				err					= -1;
	int				sleepTimeMilliSec	= kSleepShortWait;
	XFIFOContext*	context				= (XFIFOContext*)instance;

	if( context  &&  buffer  &&  context->queue  &&  context->thread  &&  context->thread->m_Error == boolean_false){
		if( requestSize <= 0 ){
			err = 0;
		}
		else{
			int		restSize = requestSize;

			while( restSize > 0 ){
				int		actSize = RingQueueWrite(context->queue, buffer, restSize);

				buffer		+= actSize;
				restSize	-= actSize;

				if( actSize > 0  &&  RingQueueCanSignal(context->queue) == boolean_true ){
					XThreadSignalAsync(context->thread);
				}

				if( restSize > 0 ){
					usleep(sleepTimeMilliSec*1000);
					sleepTimeMilliSec += sleepTimeMilliSec;

					if( sleepTimeMilliSec > kXFIFOWriteLimtWaitMilliSec ){
						sleepTimeMilliSec = kXFIFOWriteLimtWaitMilliSec;
					}
				}
			}
			if( context->thread->m_Error == boolean_false ){
				err = 0;
			}
		}
	}
	return err;
}
//--------------------------------------------------
void	XFIFOCancel(void* instance)
{
	if( instance ){
		XFIFOContext*	context = (XFIFOContext*)instance;
		if( context->thread ){
			context->thread->m_Cancel = boolean_true;
		}
	}
}
//--------------------------------------------------
#pragma mark -
//--------------------------------------------------
void	XFIFOFlush(void* instance)
{
	XFIFOContext*	context				= (XFIFOContext*)instance;
	int				sleepTimeMilliSec	= kSleepShortWait;
	booleanRef		flush				= boolean_false;

	if( context ){
		while(		XThreadIsClosing(context->thread) == boolean_false
				&&  RingQueueIsEmpty(context->queue) == boolean_false
				&&  context->thread->m_Cancel == boolean_false
		){
			if (flush == boolean_false) {
				flush = RingQueueSetFlush(context->queue);
				XThreadSignalAsync(context->thread);
			}

			usleep(sleepTimeMilliSec*1000);
			sleepTimeMilliSec += sleepTimeMilliSec;

			if( sleepTimeMilliSec > kXFIFOWriteLimtWaitMilliSec ){
				sleepTimeMilliSec = kXFIFOWriteLimtWaitMilliSec;
			}
		}
	}
}
//--------------------------------------------------
void* XThreadTask(void* refcon)
{
	XThreadContext*		thread	= (XThreadContext*)refcon;
	XFIFOContext*		context = (XFIFOContext*)thread->m_refcon;
	RingQueueContext*	queue	= context->queue;
	char*				buffer	= NULL;
	int					size	= 0;

	XThreadStart(thread);
	
	while( XThreadIsClosing(thread) == boolean_false  &&  thread->m_Cancel == boolean_false){
		XThreadWaitAsync(thread);

		if( XThreadIsClosing(thread) == boolean_true  ||  thread->m_Cancel == boolean_true ){
			break;
		}

		while( RingQueueCanReadStart(queue) == CanReadResult_OK  &&  thread->m_Cancel == boolean_false ){
			if( RingQueueReadStart(queue, &buffer, &size) > 0 ){
				ssize_t	actSize = fwrite(buffer, 1, size, context->fp);

				if( actSize != size ){
					RingQueueReadEnd(queue);
					thread->m_Error = boolean_true;
					break;
				}
			}
			RingQueueReadEnd(queue);
		}
	}
	XThreadStop(thread);
	return NULL;
}
//--------------------------------------------------
#pragma mark -
#pragma mark XThread
//--------------------------------------------------
XThreadContext*	XThreadInitialize(void)
{
	XThreadContext*	context = (XThreadContext*)malloc(sizeof(XThreadContext));
	if( context )
	{
		context->m_StopRequest 	= boolean_false;
		context->m_Running 		= kXThreadRunning_Default;
		context->m_Opened		= boolean_false;
		context->m_Waiting 		= boolean_false;
		context->m_Signal		= boolean_false;
		context->m_Cancel		= boolean_false;
		context->m_Error		= boolean_false;
		context->m_refcon		= NULL;
	}
	return context;
}
//--------------------------------------------------
void	XThreadTerminate(XThreadContext* context)
{
	XThreadClose(context);
}
//--------------------------------------------------
int	XThreadOpen(XThreadContext* context, XThreadHandlerProcPtr handler, void* refcon)
{
    int				result;
	pthread_attr_t	attr;

	if( context )
	{
		context->m_StopRequest 	= boolean_false;
		context->m_Running 		= kXThreadRunning_Default;
		context->m_Opened		= boolean_false;
		context->m_Waiting 		= boolean_false;
		context->m_Signal		= boolean_false;
		context->m_Cancel		= boolean_false;
		context->m_Error		= boolean_false;
		context->m_refcon		= refcon;

		pthread_mutex_init(&context->m_threadMutex, NULL);

		if ( pthread_cond_init(&context->m_threadCond, NULL) == 0) {
			if ( pthread_attr_init(&attr) == 0) {
				if ( pthread_attr_setschedpolicy(&attr, SCHED_FIFO) == 0) {

					result = pthread_create(&context->m_thread, &attr, handler, context);
					if (result == 0) {
						pthread_attr_destroy(&attr);
						while (context->m_Running == kXThreadRunning_Default) {
							usleep(1000L);
						}
						context->m_Opened	= boolean_true;
						return(0);
					}
				}
				pthread_attr_destroy(&attr);
			}
			pthread_cond_destroy(&context->m_threadCond);
		}
		pthread_mutex_destroy(&context->m_threadMutex);
	}
    return(-1);		// error
}
//--------------------------------------------------
void XThreadClose(XThreadContext* context)
{
	if( context ){
		void*	result;

		if (context->m_Opened	== boolean_true) {
			XThreadLock(context);
			context->m_StopRequest = boolean_true;
			pthread_cond_signal(&context->m_threadCond);
			XThreadUnlock(context);
		}
		pthread_join(context->m_thread, &result);
        pthread_cond_destroy(&context->m_threadCond);
		pthread_mutex_destroy(&context->m_threadMutex);

		free(context);
    }
}
//--------------------------------------------------
void	XThreadLock(XThreadContext* context)
{
	if( context )
		pthread_mutex_lock(&context->m_threadMutex);
}
//--------------------------------------------------
void	XThreadUnlock(XThreadContext* context)
{
	if( context )
		pthread_mutex_unlock(&context->m_threadMutex);
}
//--------------------------------------------------
void XThreadSignalAsync(XThreadContext* context)
{
	XThreadLock(context);
	if (context->m_Waiting == boolean_true){
		pthread_cond_signal(&context->m_threadCond);
	}
	XThreadUnlock(context);
}
//--------------------------------------------------
void XThreadWaitAsync(XThreadContext* context)
{
	XThreadLock(context);
	if (context->m_StopRequest == boolean_false) {
		context->m_Waiting = boolean_true;
		pthread_cond_wait(&context->m_threadCond, &context->m_threadMutex);
		context->m_Waiting = boolean_false;
	}
	XThreadUnlock(context);
}
//--------------------------------------------------
booleanRef	XThreadIsOpen(XThreadContext* context)
{
	return (context  &&  context->m_Opened == boolean_true) ? boolean_true : boolean_false;
};
//--------------------------------------------------
booleanRef	XThreadIsClosing(XThreadContext* context)
{
	return (context == NULL  ||  context->m_StopRequest == boolean_true) ? boolean_true : boolean_false;
};
//--------------------------------------------------
void	XThreadStart(XThreadContext* context)
{
	if( context )
		context->m_Running = kXThreadRunning_Start;
};
//--------------------------------------------------
void	XThreadStop(XThreadContext* context)
{
	if( context )
		context->m_Running = kXThreadRunning_Stop;
};
//--------------------------------------------------
booleanRef	XThreadIsRunning(XThreadContext* context)
{
	return (context  &&  context->m_Running == kXThreadRunning_Start) ? boolean_true : boolean_false;
};
//--------------------------------------------------
#pragma mark -
#pragma mark RingQueueOpen
//--------------------------------------------------
RingQueueContext*	RingQueueOpen(int blocksize, int numBlocks)
{
	RingQueueContext*	queue = NULL;

	if( blocksize > 1  &&  numBlocks > 2 ){
		int	totalsize = sizeof(RingQueueContext) + (sizeof(rqElement) + blocksize) * numBlocks;

		queue = (RingQueueContext*)malloc(totalsize);
		if( queue ){
			queue->m_blocksize		= blocksize;
			queue->m_wp				= NULL;
			queue->m_rp				= NULL;
			queue->m_setFlush		= boolean_false;
			queue->m_signalsize		= blocksize >> 1;
			queue->m_total			= 0;

			rqElement*	anElmTop	= (rqElement*)(queue + 1);
			rqElement*	anElm		= anElmTop;
			char*		addr		= (char*)(anElmTop + numBlocks);

			queue->m_wp				= anElmTop;
			queue->m_rp				= anElmTop;

			int xx;
			for (xx = 0;  xx < numBlocks - 1;  xx++) {
				anElm->addr		= addr;
				anElm->pos		= 0;
				anElm->next		= anElm + 1;
				addr  			+= blocksize;
				anElm			= anElm->next;
			}
			// last queue
			anElm->addr		= addr;	
			anElm->pos		= 0;
			anElm->next		= anElmTop;	// !!!!! endless queue link !!!!!. 
		}
	}
	return queue;
}
//---------------------------------------------------------------
void RingQueueClose(RingQueueContext* queue)
{
	if( queue ){
		free(queue);
	}
}
//---------------------------------------------------------------
int RingQueueWrite(RingQueueContext* queue, char* src, int requestSize)
{
	int		rest, space, actSize = 0;

    rqElement*	rp = queue->m_rp;

	if (queue == NULL  ||  src == NULL  ||  requestSize <= 0)
		return(0);

	queue->m_setFlush	= boolean_false;

    rest = requestSize;
    while (rest > 0  &&  queue->m_wp->next != rp) {

		space = queue->m_blocksize - queue->m_wp->pos;

        if (rest >= space) {
			if (space > 0)
				memmove(queue->m_wp->addr + queue->m_wp->pos, src, space);	// can not use memcpy().

	//			memmove((void*)(queue->m_wp->addr + queue->m_wp->pos), (const void*)src, (size_t)space);	// can not use memcpy().

            src					+= space;		// set new pointer
            queue->m_wp->pos	+= space;		// increment write position
            rest				-= space;		// decrement writed size

            queue->m_wp 		= queue->m_wp->next;	// next wp !!!
			queue->m_wp->pos	= 0;
        }
        else if (rest > 0) {
			memmove(queue->m_wp->addr + queue->m_wp->pos, src, rest);	// can not use memcpy().
			queue->m_wp->pos 	+= rest;
			rest 		 = 0;
        }
    }
    if (rest > 0) {	// (rest > 0) means that queue->m_wp->next == rp, then can not flush.

		space = queue->m_blocksize - queue->m_wp->pos - 1;	// "-1".....Important

		if (space > 0) {
			if (rest >= space) {
				memmove(queue->m_wp->addr + queue->m_wp->pos, src, space);	// can not use memcpy().
				queue->m_wp->pos	+= space;		// increment write position
				rest		-= space;		// decrement writed size
			}
			else {
				if (rest > 0)
					memmove(queue->m_wp->addr + queue->m_wp->pos, src, rest);	// can not use memcpy().
				queue->m_wp->pos 	+= rest;
				rest 		 = 0;
			}
		}
    }

	actSize			= requestSize - rest;
	queue->m_total += actSize;

	return(actSize);		// Actual writed size (byte).
}
//---------------------------------------------------------------
int RingQueueSetFlush(RingQueueContext* queue)
{
    rqElement*	rp = queue->m_rp;

	if (queue != NULL) {
        if (queue->m_wp->next != rp) {
            if (queue->m_wp->pos > 0) {
                queue->m_wp = queue->m_wp->next;
            }
			if (queue->m_setFlush == boolean_false) {
				queue->m_setFlush = boolean_true;
				return(1);
			}
            return(0);
        }
    }
    return(-1);
}
//---------------------------------------------------------------
booleanRef RingQueueCanSignal(RingQueueContext* queue)
{
    if (queue != NULL  &&  queue->m_total >= queue->m_signalsize) {
        queue->m_total = 0;
        return(boolean_true);
    }
    return(boolean_false);
}
//---------------------------------------------------------------
booleanRef	RingQueueIsEmpty(RingQueueContext* queue)
{
    rqElement*	rp = queue->m_rp;

    if (queue != NULL) {
        if (rp != queue->m_wp  ||  rp->pos != 0) {
            return(boolean_false);
        }
     }
    return(boolean_true);
}
//---------------------------------------------------------------
void RingQueueBufferClear(RingQueueContext* queue)
{
	if (queue != NULL) {
		queue->m_rp = queue->m_wp;
		do {
            queue->m_wp->pos	= 0;					// reset size
			queue->m_wp = queue->m_wp->next;
		} while (queue->m_wp != queue->m_rp);
	}
}
//---------------------------------------------------------------
int RingQueueCount(RingQueueContext* queue)		// for debug
{
    rqElement		*wp, *rp;
	int				xx = 1;

	if (queue != NULL) {
		wp = queue->m_wp;
		rp = queue->m_rp;
	
		while (rp != wp) {
			rp = rp->next;
			xx++;
		};
		if (rp->pos == 0)
			xx--;
	}
	return(xx);
}
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//		thread task function
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
rqCanReadResult RingQueueCanReadStart(RingQueueContext* queue)
{
	if (queue == NULL)					return(CanReadResult_Empty);
	if (queue->m_rp != queue->m_wp)		return(CanReadResult_OK);
	if (queue->m_rp->pos == 0)			return(CanReadResult_Empty);
	return(CanReadResult_Busy);
}
//--------------------------------------------------------------
int RingQueueReadStart(RingQueueContext* queue, char** buffer, int *size)
{
	if (queue  &&  queue->m_rp != queue->m_wp) {
		*buffer		= queue->m_rp->addr;
		*size		= queue->m_rp->pos;
		return(*size);
	}
	return(0);
}
//---------------------------------------------------------------
void RingQueueReadEnd(RingQueueContext* queue)
{
	if (queue  &&  queue->m_rp != queue->m_wp) {
 		queue->m_rp->pos	= 0;						// reset size
        queue->m_rp			= queue->m_rp->next;		// next rp.
	}
}
//---------------------------------------------------------------
