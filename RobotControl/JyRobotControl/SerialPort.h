#pragma once

#include "Global.h"

class SerialPort_
{
public:
	SerialPort_(){;}
	virtual ~SerialPort_(){;}

	virtual int	read(const BYTE *lpcBuf, int nRLen, int nWaitMilliseconds = 2000) = 0;
	virtual int	write(const BYTE *lpcBuf, int nWLen, int nWaitMilliseconds = 2000) = 0;

	virtual int	getPort() = 0;
};

typedef SerialPort_* LpSerialPort_;

SerialPort_*	SerialOpen	(const UINT& SerialPort_, const UINT& serialRate);
void		SerialClose	(LpSerialPort_& pSP);
