#pragma once

#include <Windows.h>
#include <exception>

/** 
* @brief Windows Semaphore
* @author ligd
* @date  2017/03/07
* @usage
	CSemaphore sp1;
	sp1.Create(1, 5, TEXT("name"));
	CSemaphore sp2;
	sp2.Open(TEXT("name"));
	sp1.Release();
	sp2.Aquire();
	sp2.TryAquire();
	sp2.Release();
	...
	sp2.Close();
	sp1.Close();
* @attention Close()���ֶ�����
* @Modified
**/
/**
 * @info
	DWORD dw = WaitForSingleObject(hProcess, 5000); //�ȴ�һ�����̽���
	switch (dw)
	{
		case WAIT_OBJECT_0:
		// hProcess������Ľ�����5���ڽ���
		break;

		case WAIT_TIMEOUT:
		// �ȴ�ʱ�䳬��5��
		break;

		case WAIT_FAILED:
		// ��������ʧ�ܣ����紫����һ����Ч�ľ��
		break;
	}
 */
class CSemaphore
{
public:
	/**
	 * ` create a Semaphore 0:unsignal >0:signal
	 * @param  lInitialCount �ź�����ʼ����ֵ
	 * @param  lMaximumCount �ź����������ֵ
	 * @param  pName         �ź�������
	 * @return               true:success false:fail
	 */
	bool Create(long lInitialCount,
		long lMaximumCount, const TCHAR* pName = NULL)
	{
		m_handle = CreateSemaphore(NULL, lInitialCount, lMaximumCount, pName);
		if (m_handle == NULL)
			return false;
		return true;
	}
	/**
	 * open a Semaphore 
	 * @param  pName �ź�������
	 * @return       true:success false:fail
	 */
	bool Open(const TCHAR* pName)
	{
		m_handle = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, pName);
		if (m_handle == NULL)
			return false;
		return true;
	}
	/**
	 * [Aquire description]
	 * @param  millisec [description]
	 * @return          [description]
	 */
	bool Aquire(unsigned long millisec = INFINITE)
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(m_handle, millisec);
	}

	bool TryAquire()
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(m_handle, 0);
	}
	/**
	 * �ź�������
	 * @param  lReleaseCount [description]
	 * @return               [description]
	 */
	long Release(long lReleaseCount = 1)
	{
		long preCnt = -1;
		ReleaseSemaphore(m_handle, lReleaseCount, &preCnt);
		return preCnt;
	}
   /**
    * �ͷ������ź���
    */
	void ReleaseAll()
	{
		do {} while (this->Release() != -1);
	}

	void Close()
	{
		CloseHandle(m_handle);
	}

private:
	HANDLE m_handle;
};

