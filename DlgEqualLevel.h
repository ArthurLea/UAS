#pragma once


// CDlgEqualLevel �Ի���

class CDlgEqualLevel : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEqualLevel)

public:
	CDlgEqualLevel(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgEqualLevel();

// �Ի�������
	enum { IDD = IDD_EQUAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRegsitere();
	afx_msg void OnBnClickedNotifye();
};
