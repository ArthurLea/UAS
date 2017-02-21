#pragma once
#include "afxwin.h"


// CNetSet dialog

class CNetSet : public CDialog
{
	DECLARE_DYNAMIC(CNetSet)

public:
	CNetSet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNetSet();

// Dialog Data
	enum { IDD = IDD_DLG_NETSET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnAlter();
	// 修改按钮控件变量
	CButton m_kAlterBtn;
};
