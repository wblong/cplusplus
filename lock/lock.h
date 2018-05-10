#ifndef BASE_CLOCK_H
#define BASE_CLOCK_H

#include <windows.h>

namespace base
{

class CLock
{
public:
	CLock();
	~CLock();

	void Acquire();
	void Release();
	bool Try();

private:
	CRITICAL_SECTION m_cs;

private:
	CLock(const CLock& right);
	CLock & operator =(const CLock & right);
};

class CAutoLock
{
public:
	explicit CAutoLock(CLock & Lock)
		: m_Lock(Lock)
	{
		m_Lock.Acquire();
	}

	~ CAutoLock()
	{
		m_Lock.Release();
	}

private:
	CLock & m_Lock;

private:
	CAutoLock(const CAutoLock& right);
	CAutoLock & operator == (const CAutoLock & right);
};

class CReadWriteLock
{
private:
	INT		m_nActive;		// 0 : no use, -1 : writing, >0 : number of reader
	CRITICAL_SECTION	m_cs;		

	INT		m_nWaitingReaders;
	INT		m_nWaitingWriters;
	HANDLE	m_hsemReaders;
	HANDLE	m_hsemWriters;

public:
	CReadWriteLock();
	~CReadWriteLock();

	void ReadAcquire();
	void WriteAcquire();

	void Release();
};

class CAutoRWLock
{
public:
	enum READ_WRITE_LOCK_TYPE
	{
		READ = 0,
		WRITE = 1,
	};

public:
	CAutoRWLock(CReadWriteLock& Locker, READ_WRITE_LOCK_TYPE eLockType)
		: m_Locker(Locker)
	{
		if (READ == eLockType)
		{
			m_Locker.ReadAcquire();
		}
		else
		{
			m_Locker.WriteAcquire();
		}
	}

	~CAutoRWLock()
	{
		m_Locker.Release();
	}

private:
	CReadWriteLock& m_Locker;
};

}
#endif