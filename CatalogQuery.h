#pragma once
#include "afxwin.h"


// CCatalogQuery �Ի���

class CCatalogQuery : public CDialogEx
{
	DECLARE_DYNAMIC(CCatalogQuery)

public:
	CCatalogQuery(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCatalogQuery();

// �Ի�������
	enum { IDD = IDD_DLG_CATALOGUREQUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedQuery();
	CComboBox m_subNoteAddress;
//	afx_msg void OnBnClickedDeviceinfquery2();
//	afx_msg void OnBnClickedDeviceinfquery2();
	afx_msg void OnBnClickedDeviceinotequery();
};
