// VirtualAnalysis.cpp  homework 3
//

#include "stdafx.h"
#include "windows.h"
#include <fstream>
#include <queue>
#include <vector>
#include <iostream>
#define CUSNUM 8
#define SERVNUM 3
#define INTERVAL 1
#define PAGELIMIT 1

using namespace std;

//
//int service[SERVNUM];
//vector<customer> customers;
//queue<customer *> cusQueue;
//queue<int> servQueue;
HANDLE allo;// = CreateSemaphore(NULL,0,1,NULL);
HANDLE trac;// = CreateSemaphore(NULL,1,1,NULL);
HANDLE thread[2];
DWORD threadID[2];
DWORD dwPageSize ;
int currentTime;
LPVOID lpvResult;
LONG alloState = 1;
LONG tracState = 1;
int cnt = 0;
VOID ErrorExit(LPTSTR lpMsg)
{
    _tprintf(TEXT("Error! %s with error code of %ld.\n"),
             lpMsg, GetLastError ());
    exit (0);
}
//void servFunc(customer * cus)
//{
//	Sleep(cus->serveTime * INTERVAL);
//	if (WaitForSingleObject(Mutex,INFINITE) == WAIT_OBJECT_0)
//	{
//		servQueue.push(cus->servNum);
//		ReleaseSemaphore(Simul,1,NULL);
//		FILE * fpw;	
//		fpw = fopen("output.txt","a+");
//		fprintf(fpw,"%d ",cus->enterTime);
//		fprintf(fpw,"%d ",cus->startTime);
//		fprintf(fpw,"%d ",cus->serveTime+cus->startTime);
//		fprintf(fpw,"%d \n",cus->servNum);
//
//		fclose(fpw);
//		ReleaseSemaphore(Mutex,1,NULL);
//	}
//
//}

void Allocator()
{
	/*while(1)
	{
		if(cusQueue.size()==0)
			continue;
		if (WaitForSingleObject(Simul,INFINITE) == WAIT_OBJECT_0)
		{
			cusQueue.front()->servNum = servQueue.front();
			cusQueue.front()->startTime = currentTime;
			printf("customer %d gets service from time %d to time %d\n",cusQueue.front()->seqNum,cusQueue.front()->startTime
				,cusQueue.front()->startTime+cusQueue.front()->serveTime);
			thread[servQueue.front()+1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)(servFunc),cusQueue.front(),0,&threadID[servQueue.front()+1]);
			cusQueue.pop();
			servQueue.pop();
			ReleaseSemaphore(Mutex,1,NULL);
		}
	}*/
	BOOL decommitResult = 0, lockResult = 0, unlockResult = 0, releaseResult = 0;
	while(1)
	{
		if(WaitForSingleObject(allo,INFINITE) == WAIT_OBJECT_0)
		{
			//if(alloState == 0x01)
			//{
			 //_tprintf (TEXT("please print your operation here.\n"));
			 //LONG operation;
			 //cin >> operation;
			 switch(cnt)
			 {
			case 0:
				lpvResult = VirtualAlloc(NULL,PAGELIMIT*dwPageSize,MEM_RESERVE,PAGE_NOACCESS);
				if (lpvResult == NULL )
					ErrorExit(TEXT("VirtualAlloc reserve failed."));
				_tprintf (TEXT("Page has been reserved\n"));
				ReleaseSemaphore(trac,1,NULL);   // &tracState
				cnt ++;
				break;
			case 1:
				lpvResult = VirtualAlloc(lpvResult,PAGELIMIT*dwPageSize,MEM_COMMIT,PAGE_READWRITE);   // PAGELIMIT*  ? ?
				if (lpvResult == NULL )
					ErrorExit(TEXT("VirtualAlloc commit failed."));
				_tprintf (TEXT("Page has been commited\n"));
				ReleaseSemaphore(trac,1,NULL);
				cnt++;
				break;

			case 2:
				lockResult = VirtualLock(lpvResult,PAGELIMIT*dwPageSize);
				if (lockResult == 0 )
					ErrorExit(TEXT("VirtualLock failed."));
				_tprintf (TEXT("Page has been locked\n"));
				ReleaseSemaphore(trac,1,NULL);
				cnt++;
				break;
			case 3:
				unlockResult = VirtualUnlock(lpvResult,PAGELIMIT*dwPageSize);
				if (lockResult == 0 )
					ErrorExit(TEXT("VirtualUnLock failed."));
				_tprintf (TEXT("Page has been unlocked\n"));
				ReleaseSemaphore(trac,1,NULL);
				cnt++;
				break;

			case 4:
				 decommitResult = VirtualFree(lpvResult,PAGELIMIT*dwPageSize,MEM_DECOMMIT);
				if (decommitResult == 0 )
					ErrorExit(TEXT("VirtualFree decommit failed."));
				_tprintf (TEXT("Page has been decommited\n"));
				ReleaseSemaphore(trac,1,NULL);
				cnt++;
				break;
			case 5:
				releaseResult = VirtualFree(lpvResult,PAGELIMIT*dwPageSize,MEM_RELEASE);
				if (lockResult == 0 )
					ErrorExit(TEXT("VirtualRelease failed."));
				_tprintf (TEXT("Page has been released\n"));
				ReleaseSemaphore(trac,1,NULL);
				cnt++;
				break;
			 //}
			//exit(0);
			}
		}
	}
}
void Tracker()
{
	MEMORY_BASIC_INFORMATION memoInfo ;

	while(1)
	{
		if(WaitForSingleObject(trac,INFINITE) == WAIT_OBJECT_0)
		{
			SIZE_T queryResult = VirtualQuery(lpvResult, &memoInfo, sizeof(MEMORY_BASIC_INFORMATION));
			
			_tprintf (TEXT("current memory state is 0x%x.\n"), memoInfo.State);
			_tprintf (TEXT("current base address is 0x%x.\n"), memoInfo.BaseAddress);
			_tprintf (TEXT("current Region Size is 0x%x.\n"), memoInfo.RegionSize);
			_tprintf (TEXT("*******************\n")); 
			 MEMORYSTATUS memoStatus;
			 GlobalMemoryStatus(&memoStatus);
			// _tprintf (TEXT("memory load is 0x%x.\n"), memoStatus.dwMemoryLoad);

			ReleaseSemaphore(allo,1,NULL);   // &alloState
		}
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	
	allo = CreateSemaphore(NULL,0,1,NULL);
	trac = CreateSemaphore(NULL,1,1,NULL);
	SYSTEM_INFO sSysInfo;         // Useful information about the system

    GetSystemInfo(&sSysInfo);     // Initialize the structure.

    _tprintf (TEXT("This computer has page size %d.\n"), sSysInfo.dwPageSize);
	
	dwPageSize = sSysInfo.dwPageSize;
	
	thread[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)(Allocator),NULL,0,&threadID[0]);
	thread[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)(Tracker),NULL,0,&threadID[1]);
	currentTime = 0;

	while(1)
	{
		Sleep(INTERVAL);
		currentTime++;
	}
	
	return 0;
}


