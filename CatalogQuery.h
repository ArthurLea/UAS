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
	afx_msg void OnBnClickedQuery();
	CComboBox m_subNoteAddress;
//	afx_msg void OnBnClickedDeviceinfquery2();
//	afx_msg void OnBnClickedDeviceinfquery2();
	afx_msg void OnBnClickedDeviceinotequery();
};
