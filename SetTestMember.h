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
	// �����б�
	CComboBox m_QueryTestMember;
	// ��ʾ�޸ĳ��̵�����	
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnAlter();
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void OnCbnSelchangeBoxList();
};
