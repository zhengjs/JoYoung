#include "StdAfx.h"
#include "SerialPort.h"
#include "Global.h"

#include <string>
using namespace std;

/************************************************************************/
/* SerialPort_I															*/
/************************************************************************/
class SerialPort_I:public SerialPort_
{
public:
	SerialPort_I();
	virtual ~SerialPort_I();

public:
    int	read(const BYTE *lpcBuf, int nRLen, int nWaitMilliseconds)override;
    int	write(const BYTE *lpcBuf, int nWLen, int nWaitMilliseconds)override;
    int	getPort()override{ return m_nPort; };

public:
	bool	open	(const UINT& SerialPort_, const UINT& serialRate);
	void    close	();

	bool	isOpened(){return (!!m_hSerial);}
	int		port	(){return m_nPort;}

public:
	int			m_nPort;
	HANDLE		m_hSerial;
	OVERLAPPED	m_OverlappedRead;
	string      m_lastError;
};

/************************************************************************/
/* SerialOpen &	SerialClose												*/
/************************************************************************/

SerialPort_*	SerialOpen	(const UINT& SerialPort_, const UINT& serialRate)
{
	SerialPort_I* pRet =new SerialPort_I();
	if(pRet)
	{
		if(!pRet->open(SerialPort_, serialRate))
			SafeDelete(pRet);
	}
	return pRet;
}

void SerialClose(LpSerialPort_& pSP)
{
	if(!pSP)
		return;
	SerialPort_I* pSPI =(SerialPort_I*)pSP;
	pSPI->close();
	SafeDelete(pSPI);
}
/************************************************************************/
/* SerialPort_I															*/
/************************************************************************/

SerialPort_I::SerialPort_I()
: m_hSerial(NULL)
, m_nPort(-1)
{
	memset(&m_OverlappedRead, 0, sizeof(OVERLAPPED));
}

SerialPort_I::~SerialPort_I()
{
	close();
}

bool SerialPort_I::open(const UINT& SerialPort_, const UINT& serialRate)
{
	if(NULL != m_hSerial)
	{
		if(SerialPort_ ==m_nPort)
			return true;
		else
			close();
	}

	do 
	{
		{
			/*ReadFile��WriteFile������ͬ�������첽��CreateFile��������������ڵ���CreateFile�������ʱָ����FILE_FLAG_OVERLAPPED��־��
			��ô����ReadFile��WriteFile�Ըþ�����еĲ�����Ӧ�����ص��ģ����δָ���ص���־�����д����Ӧ����ͬ���ġ��ص�������ʽ���ֳ�Ϊ�첽������ʽ����
			*/

			TCHAR szPort[50];
			wsprintf(szPort, _T("\\\\.\\COM%d"), SerialPort_);
			m_hSerial	=CreateFile(szPort,//COM��
				GENERIC_READ | GENERIC_WRITE, //�������д
				0, //��ռ��ʽ
				NULL,
				OPEN_EXISTING, //�򿪶����Ǵ���
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,//�ص�I/O�򿪴���-�첽������ʽ
				NULL);

			if(INVALID_HANDLE_VALUE  ==m_hSerial)
			{
				m_lastError	="�򿪴���(";
				m_lastError	+=SerialPort_;
				m_lastError	+=")ʧ��";
				break;
			}
		}
		  
		{
			/*��ʹ��ReadFile��WriteFile�ص�����ʱ���߳���Ҫ����OVERLAPPED�ṹ�Թ�����������ʹ�á��߳�ͨ��OVERLAPPED�ṹ��õ�ǰ�Ĳ���״̬���ýṹ����Ҫ�ĳ�Ա��hEvent��hEvent�Ƕ�д�¼���
			������ʹ���첽ͨѶʱ����������ʱ�������ܻ�û����ɣ��������ͨ�������¼���֪�Ƿ��д��ϡ�
			������ReadFile, WriteFile ������ʱ�򣬸ó�Ա���Զ�����Ϊ���ź�״̬�����ص�������ɺ󣬸ó�Ա�������Զ�����Ϊ���ź�״̬
			*/

			m_OverlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if(!m_OverlappedRead.hEvent)
				break;
			m_OverlappedRead.Offset = 0;
			m_OverlappedRead.OffsetHigh = 0;
		}

		{
			/*���� ReadFile �� WriteFile ��д����ʱ, ����Ҫ���ǳ�ʱ����, �����ָ����ʱ����û�ж�����д��ָ���������ַ�, ��ô ReadFile �� WriteFile ���������ء�
			���� GetCOmmTimeOuts ���Ի�ȡ��ǰ�ĳ�ʱ����, �ú��������һ��COMMTIMEOUTS �ṹ, ���� SetCommTimeouts �����ø�COMMTIMEOUTS �ṹ�����ó�ʱ

			�����ֳ�ʱ, �����ʱ���ܳ�ʱ, ����֮��������ǲ���ص�, д����ֻ֧���ܳ�ʱ, ���������ֳ�ʱ��֧��, �����ʱ��ָ����ʱ�����ַ�֮�������ӳ�ʱ��,
			�ܳ�ʱ��ָ��/ д�����ܹ����ѵ����ʱ��, COMMTIMEOUTS �ṹ�����г�Ա�����Ժ���Ϊ��λ, �ܳ�ʱ�ļ��㹫ʽΪ �ܳ�ʱ= ��/ дʱ��ϵ��> Ҫ���/ д���ַ���+ ��/ дʱ�䳣��
			
			�����ʱΪ O, ��ô�Ͳ��øó�ʱ, ����������ʱΪ O, ���ö������ʱ, �������д����������Ϊ O, ����д��ʱ��
			����������ʱΪ MAXDWORD, �Ҷ��ܳ�ʱΪ O, ��ô�ڶ�һ�λ������е����ݺ���������������, �������Ƿ������Ҫ����ַ�����
			*/

			COMMTIMEOUTS CommTimeOuts;
			CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;	//�������ʱ
			CommTimeOuts.ReadTotalTimeoutMultiplier = 0;	//��ʱ��ϵ��
			CommTimeOuts.ReadTotalTimeoutConstant = 0;		//��ʱ�䳣��
			CommTimeOuts.WriteTotalTimeoutMultiplier = 0;	//дʱ��ϵ��
			CommTimeOuts.WriteTotalTimeoutConstant = 5000;	//дʱ�䳣��
			SetCommTimeouts ( m_hSerial, &CommTimeOuts );
		}

		{
			/*
			DCB �ṹ�����˲����ʡ�����λ����żУ��λ��ֹͣλ���ı���ʽ������Ʒ�ʽ����Ϣ�� ���ڴ򿪺�, ���� GetCommState �������Ի�ȡ���ڵ�ǰ������, 
			�޸��� DCB �ṹ��, ���� SetCommState ����, ���µ� DCB �����������ô���, ����, ���ò�����λ 1152 OO, 8 λ����λ, ��У��, 1 λֹͣλ,�����Ʒ�ʽ��
			*/
			DCB dcb;
			dcb.DCBlength = sizeof ( DCB ) ;
			GetCommState ( m_hSerial, &dcb ) ;
			dcb.BaudRate	= serialRate;
			dcb.ByteSize	= DATABITS_8;
			dcb. Parity		= FALSE;
			dcb. StopBits	= ONESTOPBIT;
			dcb. fBinary	= TRUE;//�����Ʒ�ʽ

			if ( !SetCommState ( m_hSerial, &dcb ) )
				break;
		}
		{
			/*�趨 I/O �������Ĵ�С, Windows �� I/O ���������ݴ洮����������������, ���ͨ�����ʽϸ�, ��Ӧ�����ýϴ�Ļ�������С, ���� SetupComm �����������ô������������������Ĵ�С��*/
			if(!SetupComm ( m_hSerial, 10000, 10000 )) 
				break;
		}
		{
			/*ָ��һ�����ͨ���豸���¼�
				EV_RXCHAR�����뻺���������յ����ݣ������յ�һ���ֽڲ��������뻺������
			*/
			::SetCommMask(m_hSerial, EV_RXCHAR);
		}
		{
			/*�ڶ�д����֮ǰ����Ҫ��PurgeComm()������ջ�����
				PURGE_TXABORT �ж�����д�������������أ���ʹд������û����ɡ�
				PURGE_RXABORT �ж����ж��������������أ���ʹ��������û����ɡ�  
				PURGE_TXCLEAR ������������   
				PURGE_RXCLEAR ������뻺����
			*/
			::PurgeComm(m_hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

		}
		{
			//SETDTR ��DTR��Ϊ�ߵ�ƽ CLRDTR ��DTR��Ϊ�͵�ƽ	SETRTS ��RTS��Ϊ�ߵ�ƽ CLRRTS ��RTS��Ϊ�͵�ƽ
			//::EscapeCommFunction(m_hSerial, SETDTR);//��DTR��Ϊ�ߵ�ƽ
		}
		
		m_nPort	=SerialPort_;
		return TRUE;
	}
	while (false);
	
	m_lastError	="���ô���ʧ��!";
	close();
	return FALSE;
}


void SerialPort_I::close()//todo:�����������õ�ʱ��ע�⣬��û���̻߳��ڶ�д����
{
	if(m_hSerial)
	{
		//ͨ��SetCommMask(hFile,0)�������ͨѶ�豸�������¼�
		::SetCommMask(m_hSerial, 0);
	}

	if(m_OverlappedRead.hEvent != NULL)
	{
		::CloseHandle(m_OverlappedRead.hEvent);
		m_OverlappedRead.hEvent	=NULL;
	}
	memset(&m_OverlappedRead, 0, sizeof(OVERLAPPED));

	if(m_hSerial)
	{
		::CloseHandle(m_hSerial);
		m_hSerial	=NULL;
	}
	m_nPort	=-1;
}

int SerialPort_I::read(const BYTE *lpcBuf, int nRLen, int nWaitMilliseconds)
{
	if(!isOpened())
		return 0;

    BOOL bReadReslut;
	DWORD dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;
	::ClearCommError(m_hSerial, &dwErrorFlags, &ComStat);
	if(!ComStat.cbInQue)
		return 0;

	dwBytesRead = (DWORD) ComStat.cbInQue;
	dwBytesRead = min(dwBytesRead, (DWORD) nRLen);

    bReadReslut = ::ReadFile(m_hSerial, (LPVOID)lpcBuf, nRLen, &dwBytesRead, &m_OverlappedRead);
    if (!bReadReslut)
	{
		if(::GetLastError() == ERROR_IO_PENDING)//GetLastError()��������ERROR_IO_PENDING,�����������ڽ��ж����� 
		{
			int nReadReslut	=::WaitForSingleObject(m_OverlappedRead.hEvent, nWaitMilliseconds);//ʹ��WaitForSingleObject�����ȴ���ֱ����������ɻ���ʱ�ѴﵽnWaitMilliseconds (2���� nWaitMilliseconds =2000)
            bReadReslut = GetOverlappedResult(m_hSerial, &m_OverlappedRead, &dwBytesRead, FALSE);
            if (!bReadReslut)
            {
                DWORD dwError = GetLastError();
                bReadReslut = (dwError == ERROR_IO_INCOMPLETE);
            }
            //PurgeComm(hCom, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR); 
			return (int)dwBytesRead;
		}
		return 0;
	}
	return (int)dwBytesRead;
}

int SerialPort_I::write(const BYTE *lpcBuf, int nWLen, int nWaitMilliseconds)
{
	if(!isOpened())
		return 0;

	if(0 ==nWLen)
		return 0;

	OVERLAPPED osWrite = {0};
	DWORD dwWritten;
	DWORD dwRes;
	BOOL fRes;

	// Create this write operation's OVERLAPPED structure's hEvent.
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL)
		// error creating overlapped event handle
		return FALSE;

	// Issue write.
	if (!WriteFile(m_hSerial, lpcBuf, nWLen, &dwWritten, &osWrite)) {
		if (GetLastError() != ERROR_IO_PENDING) { 
			// WriteFile failed, but isn't delayed. Report error and abort.
			fRes = FALSE;
		}
		else
			// Write is pending.
			dwRes = WaitForSingleObject(osWrite.hEvent, nWaitMilliseconds);
		switch(dwRes)
		{
			// OVERLAPPED structure's event has been signaled. 
		case WAIT_OBJECT_0:
			if (!GetOverlappedResult(m_hSerial, &osWrite, &dwWritten, FALSE))
				fRes = FALSE;
			else
				// Write operation completed successfully.
				fRes = TRUE;
			break;

		default:
			// An error has occurred in WaitForSingleObject.
			// This usually indicates a problem with the
			// OVERLAPPED structure's event handle.
			fRes = FALSE;
			break;
		}
	}
   else
	   // WriteFile completed immediately.
	   fRes = TRUE;

	CloseHandle(osWrite.hEvent);

	return fRes ? nWLen : 0;
}
/*
VC++����ͨ�ű�����  
http://blog.csdn.net/kingepoch/article/details/8836244
http://www.cnblogs.com/Bonker/archive/2013/08/01/3229405.html
*/