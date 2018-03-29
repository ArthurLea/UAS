#pragma once
#include "afxwin.h"


// CDeviceInfQuery 对话框

class CDeviceInfQuery : public CDialogEx
{
	DECLARE_DYNAMIC(CDeviceInfQuery)

public:
	CDeviceInfQuery(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDeviceInfQuery();

// 对话框数据
	enum { IDD = IDD_DLG_DEVICEINFQUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDeviceinfquery();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
	virtual BOOL OnInitDialog();
};
