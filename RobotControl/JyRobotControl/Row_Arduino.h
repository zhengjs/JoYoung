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
        if ((pBytes + 7) >= pEnd)//Min7Bytes=2Byte��ʼ��+2Bytes����+1Byte֡����+2Byte��ֹ��
        {
            bBufferDataNoComplate = true;
            return false;
        }

        if (0xAAFF != *(WORD*)(pBytes))
        {
            pNextRow = pBytes + 1;//ֻ����ͷһ���ֽ�,����һ�н��Ŷ�
            return false;
        }
        int rowSize = pBytes[2] + (pBytes[3] << 8);

        LPBYTE pRowEnd = pBytes + rowSize;
        if (pRowEnd > pEnd)
        {
            bBufferDataNoComplate = true;
            return false;
        }

        if (0xBBFF != *(WORD*)(pRowEnd - 2))						//��� ��ֹ��
        {
            pNextRow = pBytes + 1;//ֻ����ͷһ���ֽ�,����һ�н��Ŷ�
            return false;
        }

        {
            int paramsBytesSize = rowSize - 7;//1Byte֡���ͣ�1Byte��ţ�1ByteУ�飬1Byte��ֹ��
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
