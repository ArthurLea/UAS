#pragma once
#include "afxwin.h"


// CPSTVSetTime 对话框

class CPSTVSetTime : public CDialogEx
{
	DECLARE_DYNAMIC(CPSTVSetTime)

public:
	CPSTVSetTime(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPSTVSetTime();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIG_PSTVTTIME };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonNtptime();
	afx_msg void OnBnClickedButtonCurtime();
	afx_msg void OnBnClickedButtonSettime();
	CEdit m_Time;
private:
	struct   NTP_Packet
	{
		int      Control_Word;
		int      root_delay;
		int      root_dispersion;
		int      reference_identifier;
		__int64 reference_timestamp;
		__int64 originate_timestamp;
		__int64 receive_timestamp;
		int      transmit_timestamp_seconds;
		int      transmit_timestamp_fractions;
	};
};
