#pragma once
#include "afxwin.h"


// CShowLog dialog

class CShowLog : public CDialog
{
	DECLARE_DYNAMIC(CShowLog)

public:
	CShowLog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowLog();

// Dialog Data
	enum { IDD = IDD_DLG_LOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnRefresh();
	virtual BOOL OnInitDialog();
	// …Ë÷√Ωπµ„
	CButton m_focus;
};
