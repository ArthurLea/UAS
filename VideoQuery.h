#pragma once
#include "afxwin.h"


// CVideoQuery dialog

class CVideoQuery : public CDialog
{
	DECLARE_DYNAMIC(CVideoQuery)

public:
	CVideoQuery(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoQuery();

// Dialog Data
	enum { IDD = IDD_DLG_VIDEOQUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnQuery();
	afx_msg void OnBnClickedBtnGeturl();
public:
	CComboBox m_HistoryVideoList;
	afx_msg void OnCbnSelchangeComboList();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
	CEdit m_BeginTime;
	CEdit m_EndTime;

public:
	virtual BOOL OnInitDialog();
};
