#pragma once


// CEventNoticeDistribute �Ի���

class CEventNoticeDistribute : public CDialogEx
{
	DECLARE_DYNAMIC(CEventNoticeDistribute)

public:
	CEventNoticeDistribute(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CEventNoticeDistribute();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIG_ALARMEVENT_NOTICE_DISTRIBUTE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonNoticeDistribute();
	virtual BOOL OnInitDialog();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
};
