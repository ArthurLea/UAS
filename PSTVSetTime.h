#pragma once
#include "afxwin.h"


// CPSTVSetTime �Ի���

class CPSTVSetTime : public CDialogEx
{
	DECLARE_DYNAMIC(CPSTVSetTime)

public:
	CPSTVSetTime(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CPSTVSetTime();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIG_PSTVTTIME };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonNtptime();
	afx_msg void OnBnClickedButtonCurtime();
	afx_msg void OnBnClickedButtonSettime();
	CEdit m_Time;
};
