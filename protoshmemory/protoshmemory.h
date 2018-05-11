#pragma once

#include "sharedmeory.hpp"
#include <vector>
#include "semaphore.hpp"
#include "tstring.h"
#include <atomic>

/** 
* @brief 协议化共享内存
	 0 1 2 3   4 5 6 7   8 9 10 11   12 13 14 15  
	| readpos | writepos| blocksize | blockcnt |  useflags(0/1) | ...data area...|
* @author ligd
* @date  2017/03/08
* @attention 
* @Modified
**/
class CProtoShMemory
{
public:
	/**
	* @brief 创建协议内存
	* @author ligd
	* @date  2017/03/10
	* @param pName: 内存名称
			 blocksize：数据块大小
			 blockcnt: 数据块个数
	* @return
	* @attention
	* @Modified
	**/
	bool Create(unsigned blocksize, unsigned blockcnt, const TCHAR* pName);

	/** 
	* @brief 连接一个已创建的协议内存
	* @author ligd
	* @date  2017/03/10
	* @param pName: 内存名称
	* @return
	* @attention 
	* @Modified
	**/
	bool Open(const TCHAR* pName);

	/** 
	* @brief 释放资源
	* @author ligd
	* @date  2017/03/10
	* @param
	* @return
	* @attention 
	* @Modified 
	**/
	void Close();
	
	/**
	* @brief 开始写过程
	* @author ligd
	* @date  2017/03/10
	* @param
	* @return	 写数据单元编号
	* @attention 必须对应一个EndWrite
	* @Modified
	**/
	int BeginWrite();

	/**
	* @brief 结束写过程
	* @author ligd
	* @date  2017/03/10
	* @param pos:写数据单元编号-BeginRead返回值
	* @return
	* @attention 必须对应一个BeginWrite
	* @Modified
	**/
	void EndWrite(int pos);

	/** 
	* @brief 开始读过程
	* @author ligd
	* @date  2017/03/10
	* @param
	* @return	 读数据单元编号
	* @attention 必须对应一个EndRead
	* @Modified
	**/
	int BeginRead();

	/**
	* @brief 结束读过程
	* @author ligd
	* @date  2017/03/10
	* @param pos:读数据单元编号-BeginRead返回值
	* @return	 
	* @attention 必须对应一个BeginRead
	* @Modified
	**/
	void EndRead(int pos);

	/** 
	* @brief 获取数据单元首地址
	* @author ligd
	* @date  2017/03/10
	* @param	pos:数据块编号
	* @return	数据单元首地址
	* @attention 
	* @Modified
	**/
	unsigned char* address(int pos) { return m_blockaddrs[pos]; }

	/**
	* @brief 释放所有读资源
	* @author ligd
	* @date  2017/03/14
	* @param
	* @return
	* @attention
	* @Modified
	**/
	void ReleaseAllRead();

	/**
	* @brief 释放所有写资源
	* @author ligd
	* @date  2017/03/14
	* @param
	* @return
	* @attention
	* @Modified
	**/
	void ReleaseAllWrite();

	enum { fixed_header_size = sizeof(long)*2 + sizeof(unsigned)*2 };

private:
	//内存头区域指向
	void pointerHeader();
	//内存头区域赋值（创建时需要）
	void assignHeader(unsigned bs, unsigned bc);
	//设置数据区单元指针
	void asignBlockAddrs();

private:
	CShMemory m_memory;	
	std::atomic_bool m_bValide{false};	//退出控制
	long* m_pReadpos;			//readpos on shared memory
	long* m_pWritepos;			//writepos on shared memory
	unsigned* m_pBlocksize;		//数据块大小
	unsigned* m_pBlockcnt;		//数据块个数
	std::vector<unsigned char*> m_blockaddrs;	//数据单元头指针
	unsigned char* m_useflags;	//数据单元使用标记
	CSemaphore m_emptys;		//同步信号-空
	CSemaphore m_fulls;			//同步信号-满
};

#define SP_EMPTY_NAME TEXT("sp_empty_")
#define SP_FULL_NAME TEXT("sp_full_")