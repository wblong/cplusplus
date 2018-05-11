#pragma once

#include "tstring.h"
#include <Windows.h>

/**
* @brief Windows Shared Memory
* @author Ligd
* @date  2016/8/30
* @usage
    const unsigned max_size = 7680 * 1080 * 3 / 2 * 100;
    const TCHAR* sh_name = TEXT("shm");

    CShMemory shm;
    if (!shm.Create(max_size, sh_name))
    {
        cout << "shmemory create failt!" << endl;
        return -1;
    }
    memset(shm.address(), 9, max_size);

    CShMemory shm2;
    if (!shm2.Open(sh_name))
    {
        cout << "shmemory open failt!" << endl;
        return -1;
    }

    cout << (int)shm2.address()[max_size - 1] << endl;
    shm.Close();
    shm2.Close();
* @attention Close()需手动调用
* @Modified
**/
#include <iostream>
class CShMemory
{
public:
    bool Create(unsigned long size, const TCHAR* pName)
    {
        m_name = pName;
        // HANDLE CreateFileMapping(
        //   HANDLE hFile,                       //物理文件句柄 INVALID_HANDLE_VALUE
        //   LPSECURITY_ATTRIBUTES lpAttributes, //安全设置 NULL
        //   DWORD flProtect,                    //保护设置 PAGE_READWRITE
        //   DWORD dwMaximumSizeHigh,            //高位文件大小
        //   DWORD dwMaximumSizeLow,             //低位文件大小
        //   LPCTSTR lpName                      //共享内存名称
        // );
        m_handle = CreateFileMapping(INVALID_HANDLE_VALUE,
            NULL, PAGE_READWRITE, 0, size, pName);
        if (m_handle == NULL)
        {
            return false;
        }
        return mapView();
    }

    bool Open(const TCHAR* pName)
    {
        m_name = pName;
        // HANDLE OpenFileMapping(
        //   DWORD dwDesiredAccess,  // 访问模式
        //   BOOL bInheritHandle,    // 继承标志
        //   LPCTSTR lpName          // 文件映射对象名指针
        // );
        // OpenFileMapping 打开一个已命名的文件映射对象
        m_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pName);
        if (m_handle == NULL)
        {
            return false;
        }
        return mapView();
    }

    void Close()
    {
        UnmapViewOfFile(m_address);
        CloseHandle(m_handle);
        m_size = 0;
    }

    unsigned size() 
    {
        if(!m_size)
        {
            MEMORY_BASIC_INFORMATION mbi;
            ::VirtualQuery(m_address, &mbi, sizeof(mbi));
            m_size = (unsigned)mbi.RegionSize;
        }
        return m_size;
    }

    unsigned char* address() { return m_address; }

    const std::tstring& name() { return m_name; }

private:
    bool mapView()
    {
        // MapViewOfFile 在调用进程的地址空间映射一个文件视图
        // LPVOID MapViewOfFile(
        //   HANDLE hFileMappingObject,  // 已创建的文件映射对象句柄
        //   DWORD dwDesiredAccess,      // 访问模式
        //   DWORD dwFileOffsetHigh,     // 文件偏移的高32位
        //   DWORD dwFileOffsetLow,      // 文件偏移的低32位
        //   DWORD dwNumberOfBytesToMap  // 映射视图的大小
        // );
        m_address = (unsigned char*)MapViewOfFile(m_handle,
            FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (m_address == NULL)
        {
            CloseHandle(m_handle);
            return false;
        }
        return true;
    }

private:
    std::tstring m_name;
    unsigned char* m_address { nullptr };
    HANDLE m_handle {NULL};
    unsigned m_size { 0 };
};