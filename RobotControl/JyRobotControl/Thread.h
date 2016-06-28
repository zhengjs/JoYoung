#pragma once

#include "Global.h"

class Thread_;

struct ThreadProc_
{
	virtual void Run(Thread_* pThread) = 0;
};

class Thread_
{
public:
	Thread_(int nThreadID = 0);
	virtual ~Thread_();

	bool	Start(ThreadProc_* pThreadRun, int nThreadPriority=THREAD_PRIORITY_NORMAL);
	void	Stop();

	bool	wantStop(int nWaitTime =0)
	{
		int waitStopState = WaitForSingleObject(m_hThreadWantStop, nWaitTime);
		if(WAIT_OBJECT_0 ==waitStopState)
		{
			ReleaseSemaphore(m_hThreadWantStop, 1, NULL);
			return true;
		}
		return false;
	}

	int		threadID(){return m_nThreadID;}

private:
	static DWORD WINAPI ThreadFunc(LPVOID pParam);

private:
	ThreadProc_*	m_pThreadRun;
	int				m_nThreadID;
	HANDLE			m_hThread;
	int				m_nThreadPriority;
	HANDLE			m_hThreadWantStop;//Semaphore
};

