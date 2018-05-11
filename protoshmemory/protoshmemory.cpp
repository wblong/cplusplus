#include "protoshmemory.h"

bool CProtoShMemory::Create(unsigned blocksize, unsigned blockcnt, const TCHAR* pName)
{
	//内存大小为 固定长度+可变使用标记长度+数据缓冲区长度
	unsigned mem_size = fixed_header_size 
		+ blockcnt * sizeof(unsigned char) + blockcnt * blocksize;

	if (m_memory.Create(mem_size, pName))
	{
		pointerHeader();		//标定头部数据位置
		assignHeader(blocksize, blockcnt);	//头数据赋值
		asignBlockAddrs();		//计算并存储缓冲区地址
		//create semaphore
		if (m_emptys.Create(blockcnt, blockcnt, (std::tstring(SP_EMPTY_NAME) + pName).c_str())
			&& m_fulls.Create(0, blockcnt, (std::tstring(SP_FULL_NAME) + pName).c_str()))
		{
			m_bValide = true;
		}
	}
	return m_bValide;
}

bool CProtoShMemory::Open(const TCHAR* pName)
{
	if (m_memory.Open(pName))
	{
		pointerHeader();
		asignBlockAddrs();
		//open semaphore
		if (m_emptys.Open((std::tstring() + SP_EMPTY_NAME + pName).c_str())
			&& m_fulls.Open((std::tstring() + SP_FULL_NAME + pName).c_str()))
		{
			m_bValide = true;
		}
	}
	return m_bValide;
}

void CProtoShMemory::Close()
{
	m_bValide = false;
	m_emptys.Close();
	m_fulls.Close();
	m_memory.Close();
}

int CProtoShMemory::BeginWrite()
{
	long oldpos;
	long newpos;

	if (!m_emptys.Aquire(10))
		return -1;

	if (!m_bValide)	//on quit
	{
		m_fulls.Release();
		return -1;
	}

	do {	//attention ABA
		oldpos = *m_pWritepos;
		newpos = (oldpos + 1) % *m_pBlockcnt;
		//CAS
	} while (oldpos != ::InterlockedCompareExchange(m_pWritepos, newpos, oldpos));

	//spin lock
	do { } while (::InterlockedExchange8((char*)(m_useflags + oldpos), 1) == 1);
	return oldpos;
}

void CProtoShMemory::EndWrite(int pos)
{
	::InterlockedExchange8((char*)(m_useflags + pos), 0);
	m_fulls.Release();
}

int CProtoShMemory::BeginRead()
{
	long oldpos;
	long newpos;

	m_fulls.Aquire();
	if (!m_bValide)
	{
		m_emptys.Release();
		return -1;
	}
	
	do {	//attention ABA
		oldpos = *m_pReadpos;
		newpos = (oldpos + 1) % *m_pBlockcnt;
		//CAS
	} while (oldpos != ::InterlockedCompareExchange(m_pReadpos, newpos, oldpos));

	//spin lock
	do {} while (::InterlockedExchange8((char*)m_useflags + oldpos, 1) == 1);
	return oldpos;
}

void CProtoShMemory::EndRead(int pos)
{
	::InterlockedExchange8((char*)m_useflags + pos, 0);
	m_emptys.Release();
}

void CProtoShMemory::ReleaseAllRead()
{
	m_fulls.ReleaseAll();
}

void CProtoShMemory::ReleaseAllWrite()
{
	m_emptys.ReleaseAll();
}

void CProtoShMemory::pointerHeader()
{
	unsigned char* pBase = m_memory.address();
	m_pReadpos = (long*)pBase;
	m_pWritepos = (long*)((char*)m_pReadpos + sizeof(long));
	m_pBlocksize = (unsigned*)((char*)m_pWritepos + sizeof(long));
	m_pBlockcnt = (unsigned*)((char*)m_pBlocksize + sizeof(unsigned));
 	m_useflags = (unsigned char*)((char*)m_pBlockcnt + sizeof(unsigned));
}

void CProtoShMemory::assignHeader(unsigned bs, unsigned bc)
{
	*m_pReadpos = 0;
	*m_pWritepos = 0;
	*m_pBlocksize = bs;
	*m_pBlockcnt = bc;
	for (unsigned i = 0; i < bc; ++i)
		*(m_useflags + i) = 0;
}

void CProtoShMemory::asignBlockAddrs()
{
	const unsigned flag_size = *m_pBlockcnt * sizeof(unsigned char);
	unsigned char* p = m_memory.address() + fixed_header_size + flag_size;
	for (unsigned i = 0; i < *m_pBlockcnt; i++)
	{
		m_blockaddrs.push_back(p);
		p += *m_pBlocksize;
	}
}