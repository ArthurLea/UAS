#pragma once
#include "afxwin.h"


// CCatalogQuery 对话框

class CCatalogQuery : public CDialogEx
{
	DECLARE_DYNAMIC(CCatalogQuery)

public:
	CCatalogQuery(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCatalogQuery();

// 对话框数据
	enum { IDD = IDD_DLG_CATALOGUREQUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_selAddress;
	afx_msg void OnBnClickedDeviceinotequery();
	afx_msg void OnBnClickedQuery();

	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeSubnote();
};
