// JoYoungDlg.h : ͷ�ļ�
//

#pragma once

#include "JoyoungRobot.h"
#include "Global.h"
#include "Kalman.h"

#include "MovingPlan.h"
#include "MovingTask_Base.h"

// CJoYoungDlg �Ի���
class CJoYoungDlg : public CDialog
{
// ����
public:
	CJoYoungDlg(CWnd* pParent = NULL);	// ��׼���캯��
    virtual ~CJoYoungDlg();

// �Ի�������
	enum { IDD = IDD_JOYOUNG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
