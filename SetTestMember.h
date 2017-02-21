#pragma once
#include "afxwin.h"


// CSetTestMember dialog

class CSetTestMember : public CDialog
{
	DECLARE_DYNAMIC(CSetTestMember)

public:
	CSetTestMember(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetTestMember();

// Dialog Data
	enum { IDD = IDD_DLG_TESTMEMBER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// 厂商列表
	CComboBox m_QueryTestMember;
	// 显示修改厂商的名称	
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnAlter();
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void OnCbnSelchangeBoxList();
};
