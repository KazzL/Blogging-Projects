/*
 * slosi.h - CC31xx/CC32xx Host Driver Implementation
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#ifndef __SL_OSI_H__
#define	__SL_OSI_H__


//#include "datatypes.h"
#define WIN32_LEAN_AND_MEAN //prevents windows including winsock1.h

#include <windows.h>
#include "stdlib.h"



#ifdef	__cplusplus
extern "C" {
#endif


#define OSI_WAIT_FOREVER   			0xFFFFFFFF

#define OSI_NO_WAIT        			0



#define CLOCK_RATE						80000000
#define NUM_OF_CLOCK_CYCLES_IN_uSEC		(CLOCK_RATE / 1000000)

/*******************************************************************************

	Types and Constants
  
********************************************************************************/

typedef enum
{
  OSI_OK,
  OSI_OPERATION_FAILED,
  OSI_ABORTED,
  OSI_INVALID_PARAMS,
  OSI_MEMORY_ALLOCATION_FAILURE,
  OSI_TIMEOUT
}OsiReturnVal_e;

// TODO: Verify and fix defines
typedef unsigned int GpInputEvent_t;

typedef unsigned int GpPort_e;

typedef unsigned int GpOutput_t;

#define ENTER_CRITICAL_SECTION			
#define EXIT_CRITICAL_SECTION	



/*!
	\brief type definition for a time value
	
	\note	On each porting or platform the type could be whatever is needed - integer, pointer to structure etc.
*/
typedef unsigned int OsiTime_t;


int osi_Sleep(UINT32 Duration);

/*!
	\brief 	type definition for a sync object container
	
	Sync object is object used to synchronize between two threads or thread and interrupt handler.
	One thread is waiting on the object and the other thread send a signal, which then
	release the waiting thread.
	The signal must be able to be sent from interrupt context.
	This object is generally implemented by binary semaphore or events.
		
	\note	On each porting or platform the type could be whatever is needed - integer, structure etc.
*/
typedef HANDLE OsiSyncObj_t;

/*!
	\brief 	type definition for a locking object container
	
	Locking object are used to protect a resource from mutual accesses of two or more threads. 
	The locking object should suppurt reentrant locks by a signal thread.
	This object is generally implemented by mutex semaphore
	
	\note	On each porting or platform the type could be whatever is needed - integer, structure etc.
*/
typedef HANDLE OsiLockObj_t;

/*!
	\brief 	type definition for a spawn entry callback

	the spawn mechanism enable to run a function on different context. 
	This mechanism allow to transfer the execution context from interrupt context to thread context
	or changing teh context from an unknown user thread to general context.
	The implementation of the spawn mechanism depends on the user's system reqeuirements and could varies 
	from implementation of serialized execution using signle thread to creating thread per call
	
	\note	The stack size of the execution thread must be at least of TBD bytes!
*/
typedef void (*_SlSpawnEntryFunc_t)(void *);
typedef void (*P_OSI_SPAWN)(void *);

/*!
	\brief 	This function creates a sync object

	The sync object is used for synchronization between diffrent thread or ISR and 
	a thread.

	\param	pSyncObj	-	pointer to the sync object control block
	
	\return upon successful creation the function should return 0
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/
int osi_SyncObjCreate(OsiSyncObj_t* pSyncObj);

/*!
	\brief 	This function deletes a sync object

	\param	pSyncObj	-	pointer to the sync object control block
	
	\return upon successful deletion the function should return 0
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/
int osi_SyncObjDelete(OsiSyncObj_t* pSyncObj);

/*!
	\brief 		This function generates a sync signal for the object. 
	
	All suspended threads waiting on this sync object are resumed

	\param		pSyncObj	-	pointer to the sync object control block
	
	\return 	upon successful signaling the function should return 0
				Otherwise, a negative value indicating the error code shall be returned
	\note		the function could be called from ISR context
	\warning
*/
int osi_SyncObjSignal(OsiSyncObj_t* pSyncObj);

/*!
	\brief 	This function waits for a sync signal of the specific sync object

	\param	pSyncObj	-	pointer to the sync object control block
	\param	Timeout		-	numeric value specifies the maximum number of mSec to 
							stay suspended while waiting for the sync signal
							Currently, the simple link driver uses only two values:
								- OSI_WAIT_FOREVER
								- OSI_NO_WAIT
	
	\return upon successful reception of the signal within the timeout window return 0
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/
int osi_SyncObjWait(OsiSyncObj_t* pSyncObj , OsiTime_t Timeout);

/*!
	\brief 	This function clears a sync object

	\param	pSyncObj	-	pointer to the sync object control block
	
	\return upon successful clearing the function should return 0
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/
int osi_SyncObjClear(OsiSyncObj_t* pSyncObj);

/*!
	\brief 	This function creates a locking object.
	
	The locking object is used for protecting a shared resources between different 
	threads.

	\param	pLockObj	-	pointer to the locking object control block
	
	\return upon successful creation the function should return 0
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/
int osi_LockObjCreate(OsiLockObj_t* pLockObj);

/*!
	\brief 	This function deletes a locking object.
	
	\param	pLockObj	-	pointer to the locking object control block
	
	\return upon successful deletion the function should return 0
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/
int osi_LockObjDelete(OsiLockObj_t* pLockObj);

/*!
	\brief 	This function locks a locking object. 
	
	All other threads that call this function before this thread calls 
	the osi_LockObjUnlock would be suspended	
	
	\param	pLockObj	-	pointer to the locking object control block
	\param	Timeout		-	numeric value specifies the maximum number of mSec to 
							stay suspended while waiting for the locking object
							Currently, the simple link driver uses only two values:
								- OSI_WAIT_FOREVER
								- OSI_NO_WAIT
	
	
	\return upon successful reception of the locking object the function should return 0
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/
int osi_LockObjLock(OsiLockObj_t* pLockObj , OsiTime_t Timeout);

/*!
	\brief 	This function unlock a locking object.
	
	\param	pLockObj	-	pointer to the locking object control block
	
	\return upon successful unlocking the function should return 0
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/
int osi_LockObjUnlock(OsiLockObj_t* pLockObj);


/*!
	\brief 	This function call the pEntry callback from a different context
	
	\param	pEntry		-	pointer to the entry callback function 
	
	\param	pValue		- 	pointer to any type of memory structure that would be
							passed to pEntry callback from the execution thread.
							
	\param	flags		- 	execution flags - reserved for future usage
	
	\return upon successful registration of the spawn the function should return 0
			(the function is not blocked till the end of the execution of the function
			and could be returned before the execution is actually completed)
			Otherwise, a negative value indicating the error code shall be returned
	\note
	\warning
*/


extern int osi_Spawn(_SlSpawnEntryFunc_t pEntry , void* pValue , unsigned long flags);
extern int osi_Execute(_SlSpawnEntryFunc_t pEntry , void* pValue , unsigned long flags);


/*!
	\brief 	This function returns the values of the system clock in mSec.
                    The system clock is set to 0 during initialization.

		\return system clock in mSec
	\note
	\warning
*/

unsigned long osi_GetTime();


#ifdef  __cplusplus
}
#endif // __cplusplus

#endif
