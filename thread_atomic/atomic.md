```C++
#include <atomic> 
#include <iostream>
#include <time.h>
#include <thread>
using namespace std;
// 全局的结果数据 
long total = 0;

// 点击函数
void click()
{
    for (int i = 0; i<1000000; ++i)
    {
        // 对全局数据进行无锁访问 
        total += 1;
    }
}


int main(int argc, char* argv[])
{
    // 计时开始
    clock_t start = clock();
    // 创建100个线程模拟点击统计
    std::thread mythread[100];
    for (int i = 0; i<100; ++i)
    {
        mythread[i] =thread(click);
    }
    for (int i = 0; i < 100; ++i) {
        mythread[i].join();
        //mythread[i].joinable();
    }
    // 计时结束
    clock_t finish = clock();
    // 输出结果
    cout << "result:" << total << endl;
    cout << "duration:" << finish - start << "ms" << endl;
    return 0;
}
```

```
result:44235129
duration:484ms
```

```
#include <atomic> 
#include <iostream>
#include <time.h>
#include <thread>
#include<mutex>
using namespace std;
// 全局的结果数据 
long total = 0;
std::mutex g_mutex;
// 点击函数
void click()
{
    for (int i = 0; i<1000000; ++i)
    {
        // 对全局数据进行无锁访问 
        g_mutex.lock();
        total += 1;
        g_mutex.unlock();
    }
}


int main(int argc, char* argv[])
{
    // 计时开始
    clock_t start = clock();
    // 创建100个线程模拟点击统计
    std::thread mythread[100];
    for (int i = 0; i<100; ++i)
    {
        mythread[i] =thread(click);
    }
    for (int i = 0; i < 100; ++i) {
        mythread[i].join();
        //mythread[i].joinable();
    }
    // 计时结束
    clock_t finish = clock();
    // 输出结果
    cout << "result:" << total << endl;
    cout << "duration:" << finish - start << "ms" << endl;
    return 0;
}
```

```
    result:100000000
    duration:23215ms
```

```
#include <atomic> 
#include <iostream>
#include <time.h>
#include <thread>
#include <mutex>
using namespace std;
// 全局的结果数据 
atomic_long total(0);
 
// 点击函数
void click()
{
    for (int i = 0; i<1000000; ++i)
    {
        // 对全局数据进行无锁访问 
     
        total += 1;
    }
}


int main(int argc, char* argv[])
{
    // 计时开始
    clock_t start = clock();
    // 创建100个线程模拟点击统计
    std::thread mythread[100];
    for (int i = 0; i<100; ++i)
    {
        mythread[i] =thread(click);
    }
    for (int i = 0; i < 100; ++i) {
        mythread[i].join();
        //mythread[i].joinable();
    }
    // 计时结束
    clock_t finish = clock();
    // 输出结果
    cout << "result:" << total << endl;
    cout << "duration:" << finish - start << "ms" << endl;
    return 0;
}
```

```
result:100000000
duration:4147ms
```