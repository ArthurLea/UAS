#pragma once
#include "afxwin.h"


// CDeviceInfQuery �Ի���

class CDeviceInfQuery : public CDialogEx
{
	DECLARE_DYNAMIC(CDeviceInfQuery)

public:
	CDeviceInfQuery(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDeviceInfQuery();

// �Ի�������
	enum { IDD = IDD_DLG_DEVICEINFQUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDeviceinfquery();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
	virtual BOOL OnInitDialog();
};
