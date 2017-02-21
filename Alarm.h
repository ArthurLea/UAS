#pragma once
#include "afxwin.h"
#include <vector>
using namespace std;

// CAlarm dialog

class CAlarm : public CDialog
{
	DECLARE_DYNAMIC(CAlarm)

public:
	CAlarm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarm();

// Dialog Data
	enum { IDD = IDD_DLG_ALARM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	vector<CString> arrAlarmType;
	afx_msg void OnBnClickedBtnAlarmSet();
	afx_msg void OnBnClickedBtnTimeset();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
	afx_msg void OnBnClickedBtnAlarmCancel();
	afx_msg void OnCbnSelchangeSelalarmtype();
	virtual BOOL OnInitDialog();
	CComboBox m_AlarmTypeSel;
};
