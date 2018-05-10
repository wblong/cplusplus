#include "lock.h"

namespace base
{

CLock::CLock(void)
{
	::InitializeCriticalSectionAndSpinCount(&m_cs, 2000);
}

CLock::~CLock(void)
{
	::DeleteCriticalSection(&m_cs);
}

void CLock::Acquire()
{
	::EnterCriticalSection(&m_cs);
}

void CLock::Release()
{
	::LeaveCriticalSection(&m_cs);
}

bool CLock::Try()
{
	if (::TryEnterCriticalSection(&m_cs) != FALSE) 
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////
CReadWriteLock::CReadWriteLock()
{
	m_nActive = m_nWaitingReaders = m_nWaitingWriters = 0;

	m_hsemReaders = ::CreateSemaphore(NULL, 0, MAXLONG, NULL);
	m_hsemWriters = ::CreateSemaphore(NULL, 0, MAXLONG, NULL);

	::InitializeCriticalSection(&m_cs);
}

CReadWriteLock::~CReadWriteLock()
{
	::DeleteCriticalSection(&m_cs);

	::CloseHandle(m_hsemWriters);
	::CloseHandle(m_hsemReaders);

	m_nActive = m_nWaitingWriters = m_nWaitingReaders = 0;
}

void CReadWriteLock::ReadAcquire()
{
	::EnterCriticalSection(&m_cs);

	// if there is writing, or write op waiting, enter read waiting queue and wait for read semaphore
	// add read counter otherwise
	bool bWritePending = (m_nWaitingWriters > 0 || m_nActive < 0);

	if (bWritePending)
	{
		m_nWaitingReaders ++;
	}
	else
	{
		m_nActive ++;
	}

	::LeaveCriticalSection(&m_cs);

	if (bWritePending)
	{
		::WaitForSingleObject(m_hsemReaders, INFINITE);
	}
}

void CReadWriteLock::WriteAcquire()
{
	::EnterCriticalSection(&m_cs);

	// if there is writing or reading, enter write waiting queue and wait for write semaphore
	// start writing otherwise
	bool bOccupied = (0 != m_nActive);

	if (bOccupied)
	{
		m_nWaitingWriters ++;
	}
	else
	{
		m_nActive = -1;
	}

	::LeaveCriticalSection(&m_cs);

	if (bOccupied)
	{
		::WaitForSingleObject(m_hsemWriters, INFINITE);
	}
}

void CReadWriteLock::Release()
{
	::EnterCriticalSection(&m_cs);

	// update the read or write waiting counter
	if (m_nActive > 0)
	{
		m_nActive --;
	}
	else
	{
		m_nActive ++;
	}

	HANDLE hTriggerSem = NULL;
	LONG lCount = 1;

	// if no writing or reading, trigger waiting operation
	if (m_nActive == 0)
	{
		// write have higher priority, trigger one waiting write.
		if (m_nWaitingWriters > 0)
		{
			m_nActive = -1;
			m_nWaitingWriters --;
			hTriggerSem = m_hsemWriters;
		}
		// trigger all waiting read
		else if (m_nWaitingReaders > 0)
		{
			m_nActive = m_nWaitingReaders;
			m_nWaitingReaders = 0;
			hTriggerSem = m_hsemReaders;
			lCount = m_nActive;
		}
		else
		{
			// no waiting
		}
	}

	::LeaveCriticalSection(&m_cs);

	if (NULL != hTriggerSem)
	{
		::ReleaseSemaphore(hTriggerSem, lCount, NULL);
	}
}


}
