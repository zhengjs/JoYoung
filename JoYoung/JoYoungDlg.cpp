// JoYoungDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "JoYoung.h"
#include "JoYoungDlg.h"
#include "Shlwapi.h"


#pragma comment(lib,"shlwapi.lib")

#include <string>
#include <vector>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CJoYoungDlg 对话框

CJoYoungDlg::CJoYoungDlg(CWnd* pParent /*=NULL*/)
: CDialog(CJoYoungDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_Kalmans_Ultrasonic[0] = new Kalman(0.125, 3200, 102300, 0);
    m_Kalmans_Ultrasonic[1] = new Kalman(0.125, 3200, 102300, 0);
    m_Kalmans_Ultrasonic[2] = new Kalman(0.125, 3200, 102300, 0);
    m_Kalmans_Ultrasonic[3] = new Kalman(0.125, 3200, 102300, 0);
}

CJoYoungDlg::~CJoYoungDlg()
{
}

void CJoYoungDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CJoYoungDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BtnL, &CJoYoungDlg::OnBnClickedBtnLeft)
	ON_BN_CLICKED(IDC_BtnU, &CJoYoungDlg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BtnR, &CJoYoungDlg::OnBnClickedBtnRight)
	ON_BN_CLICKED(IDC_BtnD, &CJoYoungDlg::OnBnClickedBtnDown)
	ON_BN_CLICKED(IDC_BtnStop, &CJoYoungDlg::OnBnClickedBtnStop)

	ON_EN_CHANGE(IDC_IntParam, &CJoYoungDlg::OnEnChangeIntparam)
	ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_Plan_Edge_Launch, &CJoYoungDlg::OnBnClickedPlanEdgeLaunch)
    ON_BN_CLICKED(IDC_Plan_Edge_Play, &CJoYoungDlg::OnBnClickedPlanEdgePlay)
END_MESSAGE_MAP()

wstring		GetAppPath()
{
	TCHAR szDllPath[MAX_PATH] = {0};
	GetModuleFileName(NULL,szDllPath,MAX_PATH);
	TCHAR drive[MAX_PATH];
	TCHAR dir[MAX_PATH];	
	_wsplitpath_s(szDllPath, drive, MAX_PATH, dir, MAX_PATH, NULL, 0, NULL, 0);

	TCHAR szAppPath[MAX_PATH] = {0};
	_stprintf_s(szAppPath,_T("%s%s"),drive, dir);

	return wstring(szAppPath);
}

LPCTSTR getConfigFilePath()
{
	static TCHAR szConfigFilePath[MAX_PATH] = {0};
	if(0 ==szConfigFilePath[0])
		_stprintf_s(szConfigFilePath,_T("%s\%s"),GetAppPath().c_str(),_T("AppConfig.ini"));
	return szConfigFilePath;

}

int GetRobotParamStringValueWithKeyName(LPCTSTR keyName, LPCTSTR defValue, LPTSTR bufValue, int bufSize)
{
	int nRet =::GetPrivateProfileString(_T("Robot"), keyName ,defValue, bufValue, bufSize, getConfigFilePath());
	return nRet;
}

int GetRobotParamIntValueWithKeyName(LPCTSTR keyName, int defValue)
{
	int nRet = ::GetPrivateProfileInt(_T("Robot"), keyName ,defValue, getConfigFilePath());
	return nRet;
}


// CJoYoungDlg 消息处理程序
static JoyoungRobot* m_pRobot;

std::string WChar2Ansi(std::wstring sWChar);

BOOL CJoYoungDlg::OnInitDialog()
{
    printf("CJoYoungDlg::OnInitDialog()");

	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	setSpeedLevel(IDC_BtnStop, 0);

	{
		int nSerialPort_ = GetRobotParamIntValueWithKeyName(_T("SerialPort_"), 5);
		int nSerialRate = GetRobotParamIntValueWithKeyName(_T("SerialRate"), 115200);
		m_pRobot = JoyoungRobot::connectRobot(6, 115200, 7, 115200);
        if (m_pRobot)
            m_pRobot->setSensorVariablesChangedCallbackProc(CJoYoungDlg::sensorVariablesChangedCallbackProc, this);
        

		string logFilePath;
		{
			TCHAR sDir[MAX_PATH];
			GetRobotParamStringValueWithKeyName(_T("LogPath"), _T(""), sDir, MAX_PATH);
			TCHAR sPath[MAX_PATH];
			TCHAR sName[MAX_PATH];
			SYSTEMTIME curTime;
			GetLocalTime(&curTime);
			int nFileIndex =0;
			do 
			{
				if(nFileIndex)
					swprintf(sName, _T("JyRobot_%d_%d_%d_%d_%d(%d).log"), curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond, nFileIndex);
				else
					swprintf(sName, _T("JyRobot_%d_%d_%d_%d_%d.log"), curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond);
				StrCpy(sPath, sDir);
				PathAppend(sPath, sName);
				if(!PathFileExists(sPath))
					break;
				++nFileIndex;
			} while (true);
			logFilePath	=WChar2Ansi(sPath);
		}

	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CJoYoungDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO:  在此处添加消息处理程序代码
    JoyoungRobot::disconnectRobot(m_pRobot);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CJoYoungDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CJoYoungDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#define StrID_Up	_T("Up")
#define StrID_Down	_T("Down")
#define StrID_Left	_T("Left")
#define StrID_Right	_T("Right")
#define StrID_Stop	_T("Stop")

#include <Shlwapi.h>

int string2BtnID(LPCTSTR strID)
{
	if(0 ==StrCmp(strID, StrID_Up))
		return IDC_BtnU;
	else if(0 ==StrCmp(strID, StrID_Down))
		return IDC_BtnD;
	else if(0 ==StrCmp(strID, StrID_Left))
		return IDC_BtnL;
	else if(0 ==StrCmp(strID, StrID_Right))
		return IDC_BtnR;
	else if(0 ==StrCmp(strID, StrID_Stop))
		return IDC_BtnStop;
	return IDC_BtnStop;
}

std::wstring Ansi2WChar(std::string sAnsi);

int string2BtnID(const string& strID)
{
	wstring wstrID =Ansi2WChar(strID);
	return string2BtnID(wstrID.c_str());
}

LPCTSTR btnID2String(int nBtnID)
{
	switch(nBtnID)
	{
	case IDC_BtnL:
		return StrID_Left;
	case IDC_BtnR:
		return StrID_Right;
	case IDC_BtnU:
		return StrID_Up;
	case IDC_BtnD:
		return StrID_Down;
	case IDC_BtnStop:
		return StrID_Stop;
	}
	return StrID_Stop;
}

void CJoYoungDlg::getSpeedLevel(int& nBtnID, int& nLevel)
{
	nBtnID	=IDC_BtnStop;
	nLevel	=0;

	CEdit* editCtrl		=(CEdit*)GetDlgItem(IDC_IntParam);
	CString cstrParam;
	editCtrl->GetWindowText(cstrParam);
	if(cstrParam.GetLength())
	{
		string strParam =WChar2Ansi((LPCWSTR)cstrParam);
		vector<string> dest;
		split_(strParam, dest, "|");
		nBtnID	=string2BtnID(dest[0]);
		if(dest.size()>1)
		{
			nLevel	=atoi(dest[1].c_str());
		}
	}
}

void CJoYoungDlg::setSpeedLevel(const int& nBtnID, const int nLevel)
{
	CEdit* editCtrl		=(CEdit*)GetDlgItem(IDC_IntParam);
	CString strParam;
	if(nLevel >0)
		strParam.Format(_T("%s|%d"), btnID2String(nBtnID), nLevel);
	else
		strParam.Format(_T("%s"), btnID2String(nBtnID));
	editCtrl->SetWindowText(strParam);
}

#define MAX_SPEED_LEVEL 5
void getSpeedWithBtnLevel(const int& nBtnID, const float& fLevel, int& nLeftMotorSpeed, int& nRightMotorSpeed);

bool CJoYoungDlg::processClickMoveButton(int& nBtnID, int& nLeftMotorSpeed, int& nRightMotorSpeed)
{
	int nParam	=0;
	int nShowBtnID, nShowSpeedLevel;
	getSpeedLevel(nShowBtnID, nShowSpeedLevel);
	if(nShowBtnID !=nBtnID)
	{
		if(nBtnID ==IDC_BtnStop)
			nShowSpeedLevel	=0;
		else if(nBtnID ==-nShowBtnID)
		{
			--nShowSpeedLevel;
			if(0 ==nShowSpeedLevel)
				nBtnID	=IDC_BtnStop;
			else
				nBtnID	=nShowBtnID;
		}
		else
			nShowSpeedLevel	=1;

		nShowBtnID	=nBtnID;
	} 
	else if(IDC_BtnStop !=nBtnID)
	{

		++nShowSpeedLevel;
		if(nShowSpeedLevel >MAX_SPEED_LEVEL)
			return false;
	}
	
	setSpeedLevel(nShowBtnID, nShowSpeedLevel);
	getSpeedWithBtnLevel(nShowBtnID, (float)nShowSpeedLevel, nLeftMotorSpeed, nRightMotorSpeed);

	return true;
}

void CJoYoungDlg::OnBnClickedBtnLeft()
{
	if(m_pRobot)
	{
		int nBtnID =IDC_BtnL, nLS =0, nRS =0;
		if(processClickMoveButton(nBtnID, nLS, nRS))
			m_pRobot->setMoveType(MT_Speed, nLS, nRS);
	}
}

void CJoYoungDlg::OnBnClickedBtnRight()
{
	if(m_pRobot)
	{
		int nBtnID =IDC_BtnR, nLS =0, nRS =0;
		if(processClickMoveButton(nBtnID, nLS, nRS))
		{
			m_pRobot->setMoveType(MT_Speed, nLS, nRS);
		}
	}
}

void CJoYoungDlg::OnBnClickedBtnUp()
{
	if(m_pRobot)
	{
		int nBtnID =IDC_BtnU, nLS =0, nRS =0;
		if(processClickMoveButton(nBtnID, nLS, nRS))
			m_pRobot->setMoveType(MT_Speed, nLS, nRS);
	}
}

void CJoYoungDlg::OnBnClickedBtnDown()
{
	if(m_pRobot)
	{
		int nBtnID =IDC_BtnD, nLS =0, nRS =0;
		if(processClickMoveButton(nBtnID, nLS, nRS))
			m_pRobot->setMoveType(MT_Speed, nLS, nRS);
	}
}

void CJoYoungDlg::OnBnClickedBtnStop()
{
    if (m_pRobot)
    {
        int nBtnID = IDC_BtnStop, nLS = 0, nRS = 0;
        processClickMoveButton(nBtnID, nLS, nRS);

        m_pRobot->setMoveType(MT_Stop, 0, 0);
        //todo:在这个里面自动关闭掉curretPlan?
        do{
            MovingPlanManager* curPlanManager = m_pRobot->movingPlanManager();
            if (!curPlanManager)
                break;
            MovingPlan* curPlan = curPlanManager->planCurrent();
            if (!curPlan)
                break;
            curPlan->planStop();
        } while (false);
    }
}

void CJoYoungDlg::OnEnChangeIntparam()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

#define  TestTimer 101

void CJoYoungDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(TestTimer ==nIDEvent)
	{
/*
		JY_CMD randCmd	=(JY_CMD)max(JY_CMD_Unknow +1, rand() %JY_CMD_Max);
		int randAng	=rand() %90;
		m_pRobot->asyncSendCommnad(randCmd, randAng);
*/
	}
	else
		CDialog::OnTimer(nIDEvent);
}

void CJoYoungDlg::sensorVariablesChangedCallbackProc(LPVOID pProcParam,
    const SensorType sensorType, const int sensorIndex,
    const LPVOID sensorVariablesData, const int sensorVariablesDataSize)
{
    if (!pProcParam)
        return;
    CJoYoungDlg* pThis = (CJoYoungDlg*)pProcParam;
    
    pThis->sensorVariablesChangedCallbackProc_(sensorType, sensorIndex, sensorVariablesData, sensorVariablesDataSize);
}

void CJoYoungDlg::sensorVariablesChangedCallbackProc_(const SensorType sensorType, const int sensorIndex, const LPVOID sensorVariablesData, const int sensorVariablesDataSize)
{
    switch (sensorType)
    {
    case SensorType::ST_MotorEncoder:
    {
        if (sensorVariablesDataSize != sizeof(Variables_MotorEncoder))
            break;
        Variables_MotorEncoder* pME = (Variables_MotorEncoder*)sensorVariablesData;
    }
        break;
    case SensorType::ST_Bump:
    {
        if (sensorVariablesDataSize != sizeof(Variables_Bump))
            break;
        Variables_Bump* pBump = (Variables_Bump*)sensorVariablesData;
    }
        break;
    case SensorType::ST_Infrared:
    {
        if (sensorVariablesDataSize != sizeof(Variables_Infrared))
            break;
        Variables_Infrared* pInf = (Variables_Infrared*)sensorVariablesData;
    }
        break;
    case SensorType::ST_WheelDrop:
    {
        if (sensorVariablesDataSize != sizeof(Variables_WheelDrop))
            break;
        Variables_WheelDrop* pWD = (Variables_WheelDrop*)sensorVariablesData;
    }
        break;
    case SensorType::ST_Ultrasonic:
    {
        if (sensorVariablesDataSize != sizeof(Variables_Ultrasonic))
            break;
        Variables_Ultrasonic* pData = (Variables_Ultrasonic*)sensorVariablesData;
        int nEditID = IDC_Ultrasonic0 + pData->nIndex;
        CEdit* editCtrl = (CEdit*)GetDlgItem(nEditID);
        CString sValue;
//        float fDistanceFiltered = m_Kalmans_Ultrasonic[pData->nIndex]->getFilteredValue(pData->nDistanceMM);
//        sValue.Format(_T("%d / %d"), pData->nDistanceMM, fDistanceFiltered);
        sValue.Format(_T("%d mm"), min(1500, pData->nDistanceMM));
        editCtrl->SetWindowText(sValue);

        do{
            CEdit* edit_taskCur = (CEdit*)GetDlgItem(IDC_Edge_Task_Current);
            if (edit_taskCur)
                edit_taskCur->SetWindowText(_T(""));

            MovingPlanManager* curPlanManager = m_pRobot->movingPlanManager();
            if (!curPlanManager)
                break;
            MovingTask* curTask = curPlanManager->taskCurrent();
            if (!curTask)
                break;

            if (edit_taskCur)
                edit_taskCur->SetWindowText(Ansi2WChar(curTask->taskName()).c_str());
        }
        while (false);
    }
        break;
    default:
        break;
    }
}

#include <limits> 
#include <cmath>

void getSpeedWithBtnLevel(const int& nBtnID, const float& fLevel, int& nLeftMotorSpeed, int& nRightMotorSpeed)
{
	float fLevel2 =min(MAX_SPEED_LEVEL, max(0, fLevel));
	if(fabs(fLevel2) < std::numeric_limits<float>::epsilon()  ||IDC_BtnStop == nBtnID)
	{
		nLeftMotorSpeed	=0;nRightMotorSpeed =0;return;
	}
	else
	{
		switch(nBtnID)
		{//85 - 340 255 / 51 /85 / 136 / 187 / 238 / 289 / 340
		case IDC_BtnL:
			{
				float fMin =85, fMax =135;
				nLeftMotorSpeed	=85;nRightMotorSpeed =(int)(((fMax -fMin) /MAX_SPEED_LEVEL) *fLevel2 +fMin);return;
			}
			break;
		case IDC_BtnR:
			{
				float fMin =85, fMax =135;
				nRightMotorSpeed =85;nLeftMotorSpeed =(int)(((fMax -fMin) /MAX_SPEED_LEVEL) *fLevel2 +fMin);return;
			}
			break;	
		case IDC_BtnU:
			{
				float fMin =85, fMax =340;
				nLeftMotorSpeed =nRightMotorSpeed =(int)(((fMax -fMin) /MAX_SPEED_LEVEL) *fLevel2 +fMin);return; 
			}
			break;	
		case IDC_BtnD:
			{
				float fMin =-85, fMax =-340;
				nLeftMotorSpeed =nRightMotorSpeed =(int)(((fMax -fMin) /MAX_SPEED_LEVEL) *fLevel2 +fMin);return; 
			}
			break;	
		}
	}
}

std::wstring Ansi2WChar(std::string sAnsi)
{
	int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)sAnsi.c_str(), sAnsi.length(), 0, 0);
	if(nSize <= 0) return NULL;
	WCHAR *pwszDst = new WCHAR[nSize+1];
	if( NULL == pwszDst) return NULL;
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)sAnsi.c_str(), sAnsi.length(), pwszDst, nSize);
	pwszDst[nSize] = 0;
	if( pwszDst[0] == 0xFEFF) // skip Oxfeff
		for(int i = 0; i < nSize; i ++) 
			pwszDst[i] = pwszDst[i+1]; 
	wstring wcharString(pwszDst);
	delete pwszDst;
	return wcharString;
}

std::string WChar2Ansi(std::wstring sWChar)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, sWChar.c_str(), -1, NULL, 0, NULL, NULL);
	if (nLen<= 0) return std::string("");
	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");
    WideCharToMultiByte(CP_ACP, 0, sWChar.c_str(), -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen -1] = 0;
	std::string strTemp(pszDst);
	delete [] pszDst;
	return strTemp;
}

void CJoYoungDlg::OnBnClickedPlanEdgeLaunch()
{
    if (!m_pRobot)
        return;
    MovingPlanManager* curPlanManager = m_pRobot->movingPlanManager();
    if (!curPlanManager)
        return;
    curPlanManager->planLaunch(MovingPlanManager::Plan_Edge);
}

void CJoYoungDlg::OnBnClickedPlanEdgePlay()
{
    MovingPlanManager* curPlanManager = m_pRobot->movingPlanManager();
    if (!curPlanManager)
        return;
    MovingPlan* pPlan = curPlanManager->planCurrent();
    if (!pPlan || pPlan->planName().compare(MovingPlanManager::Plan_Edge))
        return;
    pPlan->planPlay();
}
