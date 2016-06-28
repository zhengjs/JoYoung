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
			/*ReadFile和WriteFile函数是同步还是异步由CreateFile函数决定，如果在调用CreateFile创建句柄时指定了FILE_FLAG_OVERLAPPED标志，
			那么调用ReadFile和WriteFile对该句柄进行的操作就应该是重叠的；如果未指定重叠标志，则读写操作应该是同步的。重叠操作方式（又称为异步操作方式）。
			*/

			TCHAR szPort[50];
			wsprintf(szPort, _T("\\\\.\\COM%d"), SerialPort_);
			m_hSerial	=CreateFile(szPort,//COM口
				GENERIC_READ | GENERIC_WRITE, //允许读和写
				0, //独占方式
				NULL,
				OPEN_EXISTING, //打开而不是创建
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,//重叠I/O打开串口-异步操作方式
				NULL);

			if(INVALID_HANDLE_VALUE  ==m_hSerial)
			{
				m_lastError	="打开串口(";
				m_lastError	+=SerialPort_;
				m_lastError	+=")失败";
				break;
			}
		}
		  
		{
			/*在使用ReadFile和WriteFile重叠操作时，线程需要创建OVERLAPPED结构以供这两个函数使用。线程通过OVERLAPPED结构获得当前的操作状态，该结构最重要的成员是hEvent。hEvent是读写事件。
			当串口使用异步通讯时，函数返回时操作可能还没有完成，程序可以通过检查该事件得知是否读写完毕。
			当调用ReadFile, WriteFile 函数的时候，该成员会自动被置为无信号状态；当重叠操作完成后，该成员变量会自动被置为有信号状态
			*/

			m_OverlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if(!m_OverlappedRead.hEvent)
				break;
			m_OverlappedRead.Offset = 0;
			m_OverlappedRead.OffsetHigh = 0;
		}

		{
			/*在用 ReadFile 和 WriteFile 读写串口时, 还需要考虑超时问题, 如果在指定的时间内没有读出或写入指定数量的字符, 那么 ReadFile 和 WriteFile 将立即返回。
			调用 GetCOmmTimeOuts 可以获取当前的超时设置, 该函数将填充一个COMMTIMEOUTS 结构, 调用 SetCommTimeouts 可以用个COMMTIMEOUTS 结构来设置超时

			有两种超时, 间隔超时和总超时, 他们之间的设置是不相关的, 写操作只支持总超时, 读操作两种超时都支持, 间隔超时是指接收时两个字符之间的最大延迟时间,
			总超时是指读/ 写操作总共花费的最大时间, COMMTIMEOUTS 结构的所有成员都是以毫秒为单位, 总超时的计算公式为 总超时= 读/ 写时间系数> 要求读/ 写的字符数+ 读/ 写时间常量
			
			如果超时为 O, 那么就不用该超时, 比如读间隔超时为 O, 则不用读间隔超时, 如果所有写操作参数均为 O, 则不用写超时。
			如果读间隔超时为 MAXDWORD, 且读总超时为 O, 那么在读一次缓冲区中的内容后读操作就立即完成, 而不管是否读入了要求的字符数。
			*/

			COMMTIMEOUTS CommTimeOuts;
			CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;	//读间隔超时
			CommTimeOuts.ReadTotalTimeoutMultiplier = 0;	//读时间系数
			CommTimeOuts.ReadTotalTimeoutConstant = 0;		//读时间常量
			CommTimeOuts.WriteTotalTimeoutMultiplier = 0;	//写时间系数
			CommTimeOuts.WriteTotalTimeoutConstant = 5000;	//写时间常量
			SetCommTimeouts ( m_hSerial, &CommTimeOuts );
		}

		{
			/*
			DCB 结构包含了波特率、数据位、奇偶校验位、停止位、文本方式或二进制方式等信息。 串口打开后, 调用 GetCommState 函数可以获取串口当前的配置, 
			修改完 DCB 结构后, 调用 SetCommState 函数, 用新的 DCB 设置重新配置串口, 比如, 设置波特率位 1152 OO, 8 位数据位, 无校验, 1 位停止位,二进制方式。
			*/
			DCB dcb;
			dcb.DCBlength = sizeof ( DCB ) ;
			GetCommState ( m_hSerial, &dcb ) ;
			dcb.BaudRate	= serialRate;
			dcb.ByteSize	= DATABITS_8;
			dcb. Parity		= FALSE;
			dcb. StopBits	= ONESTOPBIT;
			dcb. fBinary	= TRUE;//二进制方式

			if ( !SetCommState ( m_hSerial, &dcb ) )
				break;
		}
		{
			/*设定 I/O 缓冲区的大小, Windows 用 I/O 缓冲区来暂存串口输入和输出的数据, 如果通信速率较高, 则应该设置较大的缓冲区大小, 调用 SetupComm 函数可以设置串口输入和输出缓冲区的大小。*/
			if(!SetupComm ( m_hSerial, 10000, 10000 )) 
				break;
		}
		{
			/*指定一组监视通信设备的事件
				EV_RXCHAR：输入缓冲区中已收到数据，即接收到一个字节并放入输入缓冲区。
			*/
			::SetCommMask(m_hSerial, EV_RXCHAR);
		}
		{
			/*在读写串口之前，还要用PurgeComm()函数清空缓冲区
				PURGE_TXABORT 中断所有写操作并立即返回，即使写操作还没有完成。
				PURGE_RXABORT 中断所有读操作并立即返回，即使读操作还没有完成。  
				PURGE_TXCLEAR 清除输出缓冲区   
				PURGE_RXCLEAR 清除输入缓冲区
			*/
			::PurgeComm(m_hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

		}
		{
			//SETDTR 将DTR设为高电平 CLRDTR 将DTR设为低电平	SETRTS 将RTS设为高电平 CLRRTS 将RTS设为低电平
			//::EscapeCommFunction(m_hSerial, SETDTR);//将DTR设为高电平
		}
		
		m_nPort	=SerialPort_;
		return TRUE;
	}
	while (false);
	
	m_lastError	="设置串口失败!";
	close();
	return FALSE;
}


void SerialPort_I::close()//todo:不做保护，用的时候注意，有没有线程还在读写串口
{
	if(m_hSerial)
	{
		//通过SetCommMask(hFile,0)来清除该通讯设备的所有事件
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
		if(::GetLastError() == ERROR_IO_PENDING)//GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作 
		{
			int nReadReslut	=::WaitForSingleObject(m_OverlappedRead.hEvent, nWaitMilliseconds);//使用WaitForSingleObject函数等待，直到读操作完成或延时已达到nWaitMilliseconds (2秒钟 nWaitMilliseconds =2000)
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
VC++串口通信编程详解  
http://blog.csdn.net/kingepoch/article/details/8836244
http://www.cnblogs.com/Bonker/archive/2013/08/01/3229405.html
*/