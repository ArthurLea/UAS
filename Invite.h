#pragma once
#include "afxwin.h"


// CInvite dialog

class CInvite : public CDialog
{
	DECLARE_DYNAMIC(CInvite)

public:
	CInvite(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInvite();

// Dialog Data
	enum { IDD = IDD_DLG_INVITE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnBye();
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnCancel();
	CComboBox m_selAddress;
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeSeladress();
	int m_selName;
	afx_msg void OnBnClickedBtnTest();
};
