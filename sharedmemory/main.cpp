#include<iostream>
#include"sharedmeory.hpp"
using namespace std;
int main()
{ 
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
	cout << (int)shm2.address()[0] << endl;
	shm.Close();
	shm2.Close();
	return 0;
}
 