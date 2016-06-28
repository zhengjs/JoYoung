#pragma once

#include "Global.h"
#include "JoyoungRobot.h"

typedef enum{
    ART_Unknow = 0x00,
    ART_Ultrasonic = 0x11,
}ArduinoRowType;

struct ArduinoRow
{
    ArduinoRowType type;
    long long     stamp;

    BYTE          index;

    vecByte       paramBytes;
    union
    {
        Variables_Ultrasonic   ultrasonic;
    }param;

    bool readHead(LPBYTE& pBytes, const LPBYTE pEnd, LPBYTE& pNextRow, bool& bBufferDataNoComplate)
    {
        bBufferDataNoComplate = false;
        if ((pBytes + 7) >= pEnd)//Min7Bytes=2Byte起始符+2Bytes长度+1Byte帧类型+2Byte终止符
        {
            bBufferDataNoComplate = true;
            return false;
        }

        if (0xAAFF != *(WORD*)(pBytes))
        {
            pNextRow = pBytes + 1;//只跳过头一个字节,让下一行接着读
            return false;
        }
        int rowSize = pBytes[2] + (pBytes[3] << 8);

        LPBYTE pRowEnd = pBytes + rowSize;
        if (pRowEnd > pEnd)
        {
            bBufferDataNoComplate = true;
            return false;
        }

        if (0xBBFF != *(WORD*)(pRowEnd - 2))						//检查 终止符
        {
            pNextRow = pBytes + 1;//只跳过头一个字节,让下一行接着读
            return false;
        }

        {
            int paramsBytesSize = rowSize - 7;//1Byte帧类型，1Byte序号，1Byte校验，1Byte终止符
            paramBytes.resize(paramsBytesSize);
            memcpy(&paramBytes[0], pBytes + 3 + 2, paramBytes.size());
        }

        pNextRow = pRowEnd;
        pBytes += 4;

        return true;
    }

    bool readType(LPBYTE& pBytes, const LPBYTE pEnd)
    {
        type = (ArduinoRowType)*pBytes;
        if (type != ART_Ultrasonic)
        {
            throw(-1);
            type = ART_Unknow;
            return false;
        }
        ++pBytes;
        return true;
    }

    void readIndex(LPBYTE& pBytes, const LPBYTE pEnd)
    {
        index = *pBytes;
        ++pBytes;
        param.ultrasonic.nIndex = index;
    }

    void readParam(LPBYTE& pBytes, const LPBYTE pEnd)
    {
        switch (type)
        {
        case ArduinoRowType::ART_Ultrasonic:
        {
            param.ultrasonic.nDistanceMM = /*(pBytes[3] << 24) +*/ (pBytes[2] << 16) + (pBytes[1] << 8) + pBytes[0];
        }
            break;
        }
    }
};
