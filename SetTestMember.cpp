// SetTestMember.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "SetTestMember.h"
#include "UASDlg.h"
#include "vector"
using namespace std;
struct Member 
{
	char mystr[40];
	int nIndex;	
};

// CSetTestMember dialog

IMPLEMENT_DYNAMIC(CSetTestMember, CDialog)

CSetTestMember::CSetTestMember(CWnd* pParent /*=NULL*/)
	: CDialog(CSetTestMember::IDD, pParent)
{

}

CSetTestMember::~CSetTestMember()
{
}

void CSetTestMember::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOX_LIST, m_QueryTestMember);	
}


BEGIN_MESSAGE_MAP(CSetTestMember, CDialog)
	ON_BN_CLICKED(IDC_BTN_ADD, &CSetTestMember::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_ALTER, &CSetTestMember::OnBnClickedBtnAlter)
	ON_BN_CLICKED(IDC_BTN_DELETE, &CSetTestMember::OnBnClickedBtnDelete)
	ON_CBN_SELCHANGE(IDC_BOX_LIST, &CSetTestMember::OnCbnSelchangeBoxList)
END_MESSAGE_MAP()


// CSetTestMember message handlers

BOOL CSetTestMember::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	FILE *in=NULL;
	in=fopen("测试厂商列表.txt","r");	
	char *temp=new char[40];
	char *tempIP=new char[40];
	char strProIP[40];
	int i=0;
	if (in!=NULL)
	{
		while (1)
		{
			if(feof(in))
				break;
			fscanf(in,"%s\n",temp);
			fscanf(in,"%s\n",tempIP);
			if (i==0)
			{
				strcpy(strProIP,tempIP);
			}
			m_QueryTestMember.InsertString(i,temp);
			
			i++;	
		}
		m_QueryTestMember.SetCurSel(0);
		CString strCBText;
		m_QueryTestMember.GetLBText( 0, strCBText);
		/*m_sTestAlter.SetWindowText(strCBText);*/
		GetDlgItem(IDC_EDT_MEMBERIP)->SetWindowText(strProIP);
		CString  m_sNumber;
		m_sNumber.Format("%d",i);		
		GetDlgItem(IDC_STR_COUNT)->SetWindowText(m_sNumber);
	}
	else
	{	
		MessageBox("测试厂商列表文件已被损坏或不存在","UAS 警告",MB_OK|MB_ICONEXCLAMATION);
	}
	if (in)fclose(in);	
	delete []temp;
	delete []tempIP;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSetTestMember::OnBnClickedBtnAdd()
{
	// TODO: Add your control notification handler code here
	FILE *in=NULL;
	in=fopen("测试厂商列表.txt","a+");	
	if (in!=NULL)
	{
		CString str;
		CString IP;
		GetDlgItem(IDC_EDT_ADDNAME)->GetWindowText(str);
		GetDlgItem(IDC_EDT_MEMBERIPADD)->GetWindowText(IP);
		if (str==""||IP=="")
		{			
			MessageBox("请输入厂商名称或厂商平台IP再添加","UAS 提示",MB_OK|MB_ICONINFORMATION);
			if(in)fclose(in);
			return;
		}
		else
		{			
			fprintf(in,"\n");
			fprintf(in,str);
			fprintf(in,"\n");
			fprintf(in,IP);
			GetDlgItem(IDC_EDT_ADDNAME)->SetWindowText("");
			GetDlgItem(IDC_EDT_MEMBERIPADD)->SetWindowText("");

			HWND   hnd=::FindWindow(NULL, _T("UAS"));
			CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
			pWnd->m_TestMember.InsertString(0,str);
			pWnd->m_TestMember.SetCurSel(0);
			m_QueryTestMember.InsertString(0,str);
			m_QueryTestMember.SetCurSel(0);
			CString strCBText;
			m_QueryTestMember.GetLBText( 0, strCBText);			
			///更新测试厂商列表总数
			CString  m_sNumber;
			GetDlgItem(IDC_STR_COUNT)->GetWindowText(m_sNumber);
			int i=atoi(m_sNumber);
			i++;
			m_sNumber.Format("%d",i);		
			GetDlgItem(IDC_STR_COUNT)->SetWindowText(m_sNumber);			
			MessageBox("添加厂商已成功","UAS 提示",MB_OK|MB_ICONINFORMATION);	
		}		
	}
	else
	{
		MessageBox("测试厂商列表文件已被损坏或不存在","UAS 警告",MB_OK|MB_ICONEXCLAMATION);
	}	
	if (in)fclose(in);
}

void CSetTestMember::OnBnClickedBtnAlter()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	CString Text;
	CString TextIP;
	GetDlgItem(IDC_EDT_MEMBERIP)->GetWindowText(TextIP);
	int nIndex=	m_QueryTestMember.GetCurSel();
	CString strCBText;
	m_QueryTestMember.GetLBText( nIndex, strCBText);
	FILE *in=NULL;
	in=fopen("测试厂商列表.txt","r");	
	int i=0;
	char *temp=new char[40];
	char *tempIP=new char[40];
	vector<Member>TestMember;
	vector<ProductMember>ProIP;
	ProductMember test;
	Member TestTemp;
	if (in!=NULL)
	{
		while(1)
		{
			if(feof(in))
				break;
			fscanf(in,"%s\n",temp);
			fscanf(in,"%s\n",tempIP);
			if (strCBText.Compare(temp)!=0)
			{
				strcpy(TestTemp.mystr,temp);
				TestTemp.nIndex=i;
				TestMember.push_back(TestTemp);
				strcpy(test.IP,tempIP);
				ProIP.push_back(test);
				i++;
			}
			else
			{
				strcpy(TestTemp.mystr,Text);
				strcpy(pWnd->ProductTestMember[i].IP,TextIP);
				TestTemp.nIndex=i;
				TestMember.push_back(TestTemp);
				strcpy(test.IP,TextIP);
				ProIP.push_back(test);
				i++;
			}
		}		
	} 
	else
	{
		MessageBox("测试厂商列表文件已被损坏或不存在","UAS 警告",MB_OK|MB_ICONEXCLAMATION);
		if(in)fclose(in);
		//释放内存
		delete []temp;
		delete []tempIP;
		vector<Member>().swap(TestMember);
		vector<ProductMember>().swap(ProIP);
		return;
	}
	if(in)fclose(in);		
	FILE *out=fopen("测试厂商列表.txt","w");
	for (int j=0;j<i;j++)
	{		
		if (j>0)
		{
			fprintf(out,"\n");
		}
		fprintf(out,TestMember[j].mystr);
		fprintf(out,"\n");
		fprintf(out,ProIP[i].IP);

	}
	//释放内存
	vector<Member>().swap(TestMember);
	vector<ProductMember>().swap(ProIP);
	delete []temp;	
	delete []tempIP;
	if (out)fclose(out);	
	m_QueryTestMember.DeleteString(nIndex);
	m_QueryTestMember.InsertString(nIndex,Text);
	m_QueryTestMember.SetCurSel(nIndex);
	//更新主对话框中的厂商列表	
	pWnd->m_TestMember.DeleteString(nIndex);	
	pWnd->m_TestMember.InsertString(nIndex,Text);
	pWnd->m_TestMember.SetCurSel(nIndex);	
	MessageBox("厂商名称修改已成功","UAS 提示",MB_OK|MB_ICONINFORMATION);
}

void CSetTestMember::OnBnClickedBtnDelete()
{
	// TODO: Add your control notification handler code here
	int nindex=	m_QueryTestMember.GetCurSel();
	CString strCBText;
	m_QueryTestMember.GetLBText( nindex, strCBText);
	FILE *in=NULL;
	in=fopen("测试厂商列表.txt","r");	
	int i=0;
	char *temp=new char[40];
	char *tempIP=new char[40];
	vector<Member>TestMember;
	vector<ProductMember>ProIP;
	Member TestTemp;
	ProductMember test;
	if (in!=NULL)
	{
		while(1)
		{
			if(feof(in))
				break;
			fscanf(in,"%s\n",temp);
			fscanf(in,"%s\n",tempIP);
			if (strCBText.Compare(temp)!=0)
			{
				strcpy(TestTemp.mystr,temp);
				TestTemp.nIndex=i;
				TestMember.push_back(TestTemp);
				strcpy(test.IP,tempIP);				
				ProIP.push_back(test);
				i++;
			}			
		}		
	} 
	else
	{		
		MessageBox("测试厂商列表文件已被损坏或不存在","UAS 警告",MB_OK|MB_ICONEXCLAMATION);
		if(in)fclose(in);
		//释放内存
		delete []temp;
		delete []tempIP;
		vector<Member>().swap(TestMember);
		vector<ProductMember>().swap(ProIP);
		return;
	}
	if(in)fclose(in);		
	FILE *out=fopen("测试厂商列表.txt","w");
	for (int j=0;j<=i-1;j++)
	{		
		if (j>0)
		{
			fprintf(out,"\n");
		}
		fprintf(out,TestMember[j].mystr);
		fprintf(out,"\n");
		fprintf(out,ProIP[i].IP);
	}
	
	//释放内存
	vector<Member>().swap(TestMember);
	vector<ProductMember>().swap(ProIP);
	delete []temp;	
	delete []tempIP;
	if (out)fclose(out);
	HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);	
	pWnd->m_TestMember.DeleteString(nindex);
	pWnd->m_TestMember.SetCurSel(0);
	m_QueryTestMember.DeleteString(nindex);
	m_QueryTestMember.SetCurSel(0);
	CString Text;
	m_QueryTestMember.GetLBText( 0, Text);
	///更新测试厂商列表总数
	CString  m_sNumber;
	GetDlgItem(IDC_STR_COUNT)->GetWindowText(m_sNumber);
	int count=atoi(m_sNumber);
	count--;
	m_sNumber.Format("%d",count);		
	GetDlgItem(IDC_STR_COUNT)->SetWindowText(m_sNumber);	
	MessageBox("测试厂商删除已成功","UAS 提示",MB_OK|MB_ICONINFORMATION);
}

void CSetTestMember::OnCbnSelchangeBoxList()
{
	// TODO: 在此添加控件通知处理程序代码
	HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	int nCurSel=m_QueryTestMember.GetCurSel();
	GetDlgItem(IDC_EDT_MEMBERIP)->SetWindowText(pWnd->ProductTestMember[nCurSel].IP);
}
