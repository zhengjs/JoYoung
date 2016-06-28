#include "StdAfx.h"
#include "Thread.h"

Thread_::Thread_(int nThreadID)
: m_nThreadID(nThreadID)
, m_pThreadRun(NULL)
, m_nThreadPriority(NULL)
, m_hThread(NULL)
, m_hThreadWantStop(NULL)
{
	m_hThreadWantStop	= CreateSemaphore(NULL, 0, 100, NULL);
}

Thread_::~Thread_(void)
{
	Stop();
	CloseHandle(m_hThreadWantStop);
}

bool Thread_::Start(ThreadProc_* pThreadRun, int nThreadPriority)
{
	if(m_pThreadRun)
		return false;

	m_pThreadRun		=pThreadRun;
	m_nThreadPriority	=nThreadPriority;

	DWORD dwThreadID = 0;
	m_hThread = ::CreateThread(NULL,0,ThreadFunc,LPVOID(this),0,&dwThreadID);
	if(m_hThread)
	{
		SetThreadPriority(m_hThread, m_nThreadPriority);
		return TRUE;
	}
	return FALSE;
}

void Thread_::Stop()
{
	if(m_hThread)
	{
		ReleaseSemaphore(m_hThreadWantStop, 1, NULL);

		DWORD dwResult = ::WaitForSingleObject(m_hThread, 10000);
		if(WAIT_OBJECT_0 != dwResult)
			TerminateThread(m_hThread, 0);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

DWORD WINAPI Thread_::ThreadFunc(LPVOID pParam)
{
	Thread_* pThis = (Thread_*)pParam;
	if(pThis == NULL)
		return 0;

	if(pThis->m_pThreadRun)
		pThis->m_pThreadRun->Run(pThis);
	
	return 0;
}

