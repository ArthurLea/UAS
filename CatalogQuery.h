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
	CComboBox m_selAddress;
	afx_msg void OnBnClickedDeviceinotequery();
	afx_msg void OnBnClickedQuery();

	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeSubnote();
};
