// JoYoungDlg.h : 头文件
//

#pragma once

#include "JoyoungRobot.h"
#include "Global.h"
#include "Kalman.h"

#include "MovingPlan.h"
#include "MovingTask_Base.h"

// CJoYoungDlg 对话框
class CJoYoungDlg : public CDialog
{
// 构造
public:
	CJoYoungDlg(CWnd* pParent = NULL);	// 标准构造函数
    virtual ~CJoYoungDlg();

// 对话框数据
	enum { IDD = IDD_JOYOUNG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLeft();
	afx_msg void OnBnClickedBtnUp();
	afx_msg void OnBnClickedBtnRight();
	afx_msg void OnBnClickedBtnDown();
	afx_msg void OnBnClickedBtnStop();
protected:
	void	getSpeedLevel(int& nBtnID, int& nLevel);
	void	setSpeedLevel(const int& nBtnID, const int nLevel);

	bool	processClickMoveButton(int& nBtnID, int& nLeftMotorSpeed, int& nRightMotorSpeed);
public:
	afx_msg void OnEnChangeIntparam();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

protected:
    static void sensorVariablesChangedCallbackProc(LPVOID pProcParam, 
                                         const SensorType sensorType, const int sensorIndex, 
                                         const LPVOID sensorVariablesData, const int sensorVariablesDataSize);

    void sensorVariablesChangedCallbackProc_(const SensorType sensorType, const int sensorIndex,
                                   const LPVOID sensorVariablesData, const int sensorVariablesDataSize);

protected:
    Kalman* m_Kalmans_Ultrasonic[4];

public:
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedPlanEdgeLaunch();
    afx_msg void OnBnClickedPlanEdgePlay();
};
