/**
 * @desc 信号量处理主线程与子线程的同步，用关键段来处理各子线程间的互斥
 */
#include <stdio.h>  
#include <process.h>  
#include <windows.h>

long g_nNum;
unsigned int __stdcall Fun(void *pPM);
const int THREAD_NUM = 1000;
//信号量与关键段  
HANDLE            g_hThreadParameter;
CRITICAL_SECTION  g_csThreadCode;

int main()
{
	printf("经典线程同步 信号量Semaphore\n");
	printf(" -- by MoreWindows( http://blog.csdn.net/MoreWindows ) --\n\n");

	//初始化信号量和关键段  
	g_hThreadParameter = CreateSemaphore(NULL, 0, 1, NULL);//当前0个资源，最大允许1个同时访问  
	InitializeCriticalSection(&g_csThreadCode);
	
	HANDLE  handle[THREAD_NUM];
	g_nNum = 0;
	int i = 0;
	while (i < THREAD_NUM)
	{
		//第1个参数：安全属性，NULL为默认安全属性
		//第2个参数：指定线程堆栈的大小。如果为0，则线程堆栈大小和创建它的线程的相同。一般用0
		//第3个参数：指定线程函数的地址，也就是线程调用执行的函数地址(用函数名称即可，函数名称就表示地址)
		//第4个参数：传递给线程的参数的指针，可以通过传入对象的指针，在线程函数中再转化为对应类的指针
		//第5个参数：线程初始状态，0:立即运行；CREATE_SUSPEND：suspended（悬挂）
		//第6个参数：用于记录线程ID的地址

		handle[i] = (HANDLE)_beginthreadex(NULL, 0, Fun, &i, 0, NULL);

		WaitForSingleObject(g_hThreadParameter, INFINITE);//等待信号量>0  并减一

		++i;
	}
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);//并减一
	
	
	//销毁信号量和关键段  
	DeleteCriticalSection(&g_csThreadCode);
	CloseHandle(g_hThreadParameter);
	for (i = 0; i < THREAD_NUM; i++)
		CloseHandle(handle[i]);
	return 0;
}
/**
 * 处理线程函数
 * @param  pPM 参数
 * @return     0
 */
unsigned int __stdcall Fun(void *pPM)
{
	int nThreadNum = *(int *)pPM;
	ReleaseSemaphore(g_hThreadParameter, 1, NULL);//信号量++  

	Sleep(50);//some work should to do  

	EnterCriticalSection(&g_csThreadCode);
		++g_nNum;
		Sleep(0);//some work should to do  
		printf("线程编号为%d  全局资源值为%d\n", nThreadNum, g_nNum);
		/*FILE*gFile = fopen("test.txt", 'w+');
		fprintf(g_File, "线程编号为%d  全局资源值为%d\n", nThreadNum, g_nNum);
		fclose(gFile);*/
	LeaveCriticalSection(&g_csThreadCode);
	return 0;
}