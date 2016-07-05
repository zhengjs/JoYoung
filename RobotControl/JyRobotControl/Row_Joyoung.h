#pragma once

#include "Global.h"
#include "JoyoungRobot.h"

typedef enum{
    RRT_Unknow = 0x00,
    RRT_MotorEncoder = 0x04,
    RRT_Bump = 0x02,
    RRT_Infrared = 0x0E,
    RRT_WheelDrop = 0x0F,
}JoyoungRowType;

struct JoyoungRow
{
    JoyoungRowType  type;
    long long       stamp;

    BYTE            index;

    vecByte         paramBytes;
    union
    {
        Variables_MotorEncoder motorEncoder;
        Variables_Bump			bump;
        Variables_Infrared		infrared;
        Variables_WheelDrop	drop;
    }param;

    bool readHead(LPBYTE& pBytes, const LPBYTE pEnd, LPBYTE& pNextRow, bool& bBufferDataNoComplate)
    {
        bBufferDataNoComplate = false;
        if ((pBytes + 7) >= pEnd)//Min7Bytes=1Byte起始符+2Bytes长度+1Byte帧类型+1Byte序号+1Byte校验+1Byte终止符
        {
            bBufferDataNoComplate = true;
            return false;
        }

        if (0xA5 != pBytes[0])
        {
            pNextRow = pBytes + 1;//只跳过头一个字节,让下一行接着读
            return false;
        }

        int paramsBytesSize = pBytes[2] + (pBytes[1] << 8) - 4;//1Byte帧类型，1Byte序号，1Byte校验，1Byte终止符

        LPBYTE pRowEnd = pBytes + 3 + 2 + paramsBytesSize + 2;
        if (pRowEnd > pEnd)
        {
            bBufferDataNoComplate = true;
            return false;
        }

        if (0x5A != *(pRowEnd - 1))						//检查 终止符
        {
            pNextRow = pBytes + 1;//只跳过头一个字节,让下一行接着读
            return false;
        }

        paramBytes.resize(paramsBytesSize);
        memcpy(&paramBytes[0], pBytes + 3 + 2, paramsBytesSize);

        const BYTE& xorByte = *(pRowEnd - 2);
        if (getBytes_Xor(pBytes + 1, pRowEnd - 2) != xorByte)//检查 奇偶校验
        {
            //throw(-1);
			printf_s("Head Xor verify failed!\n");
            pNextRow = pBytes + 1;//只跳过头一个字节
            return false;
        }

        pNextRow = pRowEnd;
        pBytes += 3;

        return true;
    }

    bool readType(LPBYTE& pBytes, const LPBYTE pEnd)
    {
        type = (JoyoungRowType)*pBytes;
        if (type != RRT_MotorEncoder &&type != RRT_Bump &&type != RRT_Infrared&&type != RRT_WheelDrop)
        {
            //throw(-1);
			printf_s("Unknown data type!\n");
            type = RRT_Unknow;
            return false;
        }
        ++pBytes;
        return true;
    }

    void readIndex(LPBYTE& pBytes, const LPBYTE pEnd)
    {
        index = *pBytes;
        ++pBytes;
    }

    void readParam(LPBYTE& pBytes, const LPBYTE pEnd)
    {
        switch (type)
        {
        case JoyoungRowType::RRT_MotorEncoder:
        {
                                                LPBYTE pRead = pBytes;
                                                param.motorEncoder.leftMotor = (pRead[3] << 0) + (pRead[2] << 8) + (pRead[1] << 16) + (pRead[0] << 24);
                                                pRead = pRead + 4;
                                                param.motorEncoder.rightMotor = (pRead[3] << 0) + (pRead[2] << 8) + (pRead[1] << 16) + (pRead[0] << 24);
                                                pRead = pRead + 4;
                                                param.motorEncoder.stamp = (pRead[3] << 0) + (pRead[2] << 8) + (pRead[1] << 16) + (pRead[0] << 24);

                                                char sBuf[200];
                                                _snprintf_s(sBuf, 200, 199, "MotorEncoder motorLeft:(%I64d) motorRight:(%I64d) stamp:(%I64d) \r\n",
                                                    (long long)param.motorEncoder.leftMotor, (long long)param.motorEncoder.rightMotor, (long long)param.motorEncoder.stamp);
                                                DebugLogLn_(sBuf);
        }
            break;
        case JoyoungRowType::RRT_Bump:
        {
                                        LPBYTE pRead = pBytes;
                                        param.bump.leftBump = pRead[0];
                                        param.bump.rightBump = pRead[1];

                                        char sBuf[200];
                                        _snprintf_s(sBuf, 200, 199, "Bump leftBump:(%I64d) rightBump:(%I64d) \r\n",
                                            (long long)param.bump.leftBump, (long long)param.bump.rightBump, (long long)param.motorEncoder.stamp);
                                        DebugLogLn_(sBuf);
        }
            break;
        case JoyoungRowType::RRT_Infrared:
        {
                                            LPBYTE pRead = pBytes;
                                            param.infrared.infraredL1 = pRead[0];
                                            param.infrared.infraredL2 = pRead[1];
                                            param.infrared.infraredC = pRead[2];
                                            param.infrared.infraredR2 = pRead[3];
                                            param.infrared.infraredR1 = pRead[4];

                                            char sBuf[200];
                                            _snprintf_s(sBuf, 200, 199, "Infrared left1:(%I64d) left2:(%I64d) center:(%I64d) right2:(%I64d) right1:(%I64d) \r\n",
                                                (long long)param.infrared.infraredL1, (long long)param.infrared.infraredL2,
                                                (long long)param.infrared.infraredC,
                                                (long long)param.infrared.infraredR2, (long long)param.infrared.infraredR1);
                                            DebugLogLn_(sBuf);
        }
            break;
        case JoyoungRowType::RRT_WheelDrop:
        {
                                             LPBYTE pRead = pBytes;
                                             param.drop.wheelDrop = pRead[0];

                                             char sBuf[200];
                                             _snprintf_s(sBuf, 200, 199, "WheelDrop :(%I64d) \r\n", (long long)param.drop.wheelDrop);
                                             DebugLogLn_(sBuf);
        }
            break;
        }
    }
};
