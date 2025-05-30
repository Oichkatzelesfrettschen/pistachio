/*     
 * $RCSfile: rwlock.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.6 $
 * $Date: 1993/02/01 22:40:21 $
 */

#ifndef rwlock_h
#define rwlock_h

/* 
 * Readers-writers lock interface
 */


typedef struct rwlock {
	int		activeWriter, activeReaders;
	int		queuedWriters, queuedReaders;
	Semaphore	readersSem, writersSem;
	int		flags;
	void		(* destroyFunc)(
			      struct rwlock *, void *
			      );
	VOID		*destFuncArg;
} ReadWriteLock;


typedef	void	(*RwlDestroyFunc)(
					   ReadWriteLock *, void *
					   );



xkern_return_t	readerLock( ReadWriteLock * );
void		readerUnlock( ReadWriteLock * );
void		rwLockDestroy( ReadWriteLock *, RwlDestroyFunc, VOID * );
void		rwLockDump( ReadWriteLock * );
void		rwLockInit( ReadWriteLock * );
xkern_return_t	writerLock( ReadWriteLock * );
void		writerUnlock( ReadWriteLock * );


#endif rwlock_h
