/**
 * @desc �ź����������߳������̵߳�ͬ�����ùؼ�������������̼߳�Ļ���
 */
#include <stdio.h>  
#include <process.h>  
#include <windows.h>

long g_nNum;
unsigned int __stdcall Fun(void *pPM);
const int THREAD_NUM = 1000;
//�ź�����ؼ���  
HANDLE            g_hThreadParameter;
CRITICAL_SECTION  g_csThreadCode;

int main()
{
	printf("�����߳�ͬ�� �ź���Semaphore\n");
	printf(" -- by MoreWindows( http://blog.csdn.net/MoreWindows ) --\n\n");

	//��ʼ���ź����͹ؼ���  
	g_hThreadParameter = CreateSemaphore(NULL, 0, 1, NULL);//��ǰ0����Դ���������1��ͬʱ����  
	InitializeCriticalSection(&g_csThreadCode);
	
	HANDLE  handle[THREAD_NUM];
	g_nNum = 0;
	int i = 0;
	while (i < THREAD_NUM)
	{
		//��1����������ȫ���ԣ�NULLΪĬ�ϰ�ȫ����
		//��2��������ָ���̶߳�ջ�Ĵ�С�����Ϊ0�����̶߳�ջ��С�ʹ��������̵߳���ͬ��һ����0
		//��3��������ָ���̺߳����ĵ�ַ��Ҳ�����̵߳���ִ�еĺ�����ַ(�ú������Ƽ��ɣ��������ƾͱ�ʾ��ַ)
		//��4�����������ݸ��̵߳Ĳ�����ָ�룬����ͨ����������ָ�룬���̺߳�������ת��Ϊ��Ӧ���ָ��
		//��5���������̳߳�ʼ״̬��0:�������У�CREATE_SUSPEND��suspended�����ң�
		//��6�����������ڼ�¼�߳�ID�ĵ�ַ

		handle[i] = (HANDLE)_beginthreadex(NULL, 0, Fun, &i, 0, NULL);

		WaitForSingleObject(g_hThreadParameter, INFINITE);//�ȴ��ź���>0  ����һ

		++i;
	}
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);//����һ
	
	
	//�����ź����͹ؼ���  
	DeleteCriticalSection(&g_csThreadCode);
	CloseHandle(g_hThreadParameter);
	for (i = 0; i < THREAD_NUM; i++)
		CloseHandle(handle[i]);
	return 0;
}
/**
 * �����̺߳���
 * @param  pPM ����
 * @return     0
 */
unsigned int __stdcall Fun(void *pPM)
{
	int nThreadNum = *(int *)pPM;
	ReleaseSemaphore(g_hThreadParameter, 1, NULL);//�ź���++  

	Sleep(50);//some work should to do  

	EnterCriticalSection(&g_csThreadCode);
		++g_nNum;
		Sleep(0);//some work should to do  
		printf("�̱߳��Ϊ%d  ȫ����ԴֵΪ%d\n", nThreadNum, g_nNum);
		/*FILE*gFile = fopen("test.txt", 'w+');
		fprintf(g_File, "�̱߳��Ϊ%d  ȫ����ԴֵΪ%d\n", nThreadNum, g_nNum);
		fclose(gFile);*/
	LeaveCriticalSection(&g_csThreadCode);
	return 0;
}