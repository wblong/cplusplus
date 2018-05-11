#pragma once

#include "sharedmeory.hpp"
#include <vector>
#include "semaphore.hpp"
#include "tstring.h"
#include <atomic>

/** 
* @brief Э�黯�����ڴ�
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
	* @brief ����Э���ڴ�
	* @author ligd
	* @date  2017/03/10
	* @param pName: �ڴ�����
			 blocksize�����ݿ��С
			 blockcnt: ���ݿ����
	* @return
	* @attention
	* @Modified
	**/
	bool Create(unsigned blocksize, unsigned blockcnt, const TCHAR* pName);

	/** 
	* @brief ����һ���Ѵ�����Э���ڴ�
	* @author ligd
	* @date  2017/03/10
	* @param pName: �ڴ�����
	* @return
	* @attention 
	* @Modified
	**/
	bool Open(const TCHAR* pName);

	/** 
	* @brief �ͷ���Դ
	* @author ligd
	* @date  2017/03/10
	* @param
	* @return
	* @attention 
	* @Modified 
	**/
	void Close();
	
	/**
	* @brief ��ʼд����
	* @author ligd
	* @date  2017/03/10
	* @param
	* @return	 д���ݵ�Ԫ���
	* @attention �����Ӧһ��EndWrite
	* @Modified
	**/
	int BeginWrite();

	/**
	* @brief ����д����
	* @author ligd
	* @date  2017/03/10
	* @param pos:д���ݵ�Ԫ���-BeginRead����ֵ
	* @return
	* @attention �����Ӧһ��BeginWrite
	* @Modified
	**/
	void EndWrite(int pos);

	/** 
	* @brief ��ʼ������
	* @author ligd
	* @date  2017/03/10
	* @param
	* @return	 �����ݵ�Ԫ���
	* @attention �����Ӧһ��EndRead
	* @Modified
	**/
	int BeginRead();

	/**
	* @brief ����������
	* @author ligd
	* @date  2017/03/10
	* @param pos:�����ݵ�Ԫ���-BeginRead����ֵ
	* @return	 
	* @attention �����Ӧһ��BeginRead
	* @Modified
	**/
	void EndRead(int pos);

	/** 
	* @brief ��ȡ���ݵ�Ԫ�׵�ַ
	* @author ligd
	* @date  2017/03/10
	* @param	pos:���ݿ���
	* @return	���ݵ�Ԫ�׵�ַ
	* @attention 
	* @Modified
	**/
	unsigned char* address(int pos) { return m_blockaddrs[pos]; }

	/**
	* @brief �ͷ����ж���Դ
	* @author ligd
	* @date  2017/03/14
	* @param
	* @return
	* @attention
	* @Modified
	**/
	void ReleaseAllRead();

	/**
	* @brief �ͷ�����д��Դ
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
	//�ڴ�ͷ����ָ��
	void pointerHeader();
	//�ڴ�ͷ����ֵ������ʱ��Ҫ��
	void assignHeader(unsigned bs, unsigned bc);
	//������������Ԫָ��
	void asignBlockAddrs();

private:
	CShMemory m_memory;	
	std::atomic_bool m_bValide{false};	//�˳�����
	long* m_pReadpos;			//readpos on shared memory
	long* m_pWritepos;			//writepos on shared memory
	unsigned* m_pBlocksize;		//���ݿ��С
	unsigned* m_pBlockcnt;		//���ݿ����
	std::vector<unsigned char*> m_blockaddrs;	//���ݵ�Ԫͷָ��
	unsigned char* m_useflags;	//���ݵ�Ԫʹ�ñ��
	CSemaphore m_emptys;		//ͬ���ź�-��
	CSemaphore m_fulls;			//ͬ���ź�-��
};

#define SP_EMPTY_NAME TEXT("sp_empty_")
#define SP_FULL_NAME TEXT("sp_full_")