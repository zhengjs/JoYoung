//Synobj.h
#ifndef __SYNOBJ_H__
#define __SYNOBJ_H__

#include <Windows.h>

class Lock_
{
public:
	Lock_() 
	{ 
		InitializeCriticalSection(&m_cs); 
	}
	~Lock_() 
	{ 
		DeleteCriticalSection(&m_cs); 
	}
	
	void Lock() 
	{ 
		EnterCriticalSection(&m_cs); 
	}
	void UnLock() 
	{ 
		LeaveCriticalSection(&m_cs); 
	}
protected:
	CRITICAL_SECTION m_cs;
};

template <class T>
class AutoLock_
{
public:
	AutoLock_(T* pLock)
	{ 
		m_pLock = pLock; 
		m_pLock->Lock(); 
	}
	~AutoLock_()
	{ 
		m_pLock->UnLock(); 
	}
private:
	T* m_pLock;
};

typedef AutoLock_<Lock_>	CAutoLock;

#endif