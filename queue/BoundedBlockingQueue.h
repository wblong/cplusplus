#pragma once
#ifndef BLOCKINGQUEUE_H_
#define BLOCKINGQUEUE_H_

#include <queue>
#include <windows.h>
using namespace std;

template <typename T>
class BoundedBlockingQueue 
{ 
public: 
static const unsigned int cnMaximumSize = 1000; // MAX_FRM_CNT;

    BoundedBlockingQueue() : maxSize(cnMaximumSize) 
    {
        _lock = CreateMutex(NULL,false,NULL);
        _rsem = CreateSemaphore(NULL,0,maxSize,NULL);
        _wsem = CreateSemaphore(NULL,maxSize,maxSize,NULL);
    } 
    ~BoundedBlockingQueue() 
    { 
        CloseHandle(_lock);
        CloseHandle(_rsem);
        CloseHandle(_wsem);
    } 
    void push(const T& data);
    T front();
    T pop();
    bool empty()
    {
        WaitForSingleObject(_lock,INFINITE);
        bool is_empty = _array.empty();
        ReleaseMutex(_lock);
        return is_empty;
    }
    int size()
    {
//         WaitForSingleObject(_lock,INFINITE);
//         int size = (int)_array.size();
//         ReleaseMutex(_lock);
//         return size;

		return (int)_array.size();
    }
private: 
    deque<T> _array;
    int maxSize;
    HANDLE _lock;
    HANDLE _rsem, _wsem;
};

template <typename T>
void BoundedBlockingQueue <T>::push(const T& value ) 
{ 
    WaitForSingleObject(_wsem,INFINITE);
    WaitForSingleObject(_lock,INFINITE);
    _array.push_back(value);
    ReleaseMutex(_lock);
    ReleaseSemaphore(_rsem,1,NULL);
}

template <typename T>
T BoundedBlockingQueue<T>::pop() 
{ 
    WaitForSingleObject(_rsem,INFINITE);
    WaitForSingleObject(_lock,INFINITE);
    T _temp = _array.front();
    _array.pop_front();
    ReleaseMutex(_lock);
    ReleaseSemaphore(_wsem,1,NULL);
    return _temp;
}
template <typename T>
T BoundedBlockingQueue<T>::front() 
{ 
    WaitForSingleObject(_rsem,INFINITE);
    WaitForSingleObject(_lock,INFINITE);
    T _temp = _array.front();
    ReleaseMutex(_lock);
    ReleaseSemaphore(_wsem,1,NULL);
    return _temp;
}

#endif