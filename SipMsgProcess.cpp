#include "StdAfx.h"
#include "SipMsgProcess.h"
#include <time.h>
#include "string.h"
#include "md5.h"
#include <fstream>
#include "Common.h"
#include <afxinet.h>
SHELLEXECUTEINFO rtpsend;
SHELLEXECUTEINFO vlc;
using namespace std;
//extern queue<UA_Msg> uas_recvqueue;
extern queue<UA_Msg> uas_sendqueue;
//extern UA_Msg uas_curqueue;
extern UA_Msg uas_curSendMsg;
//extern HANDLE hMutex_uas;
extern CRITICAL_SECTION g_uas;

extern InfoNotify NotifyInfo;
extern BOOL bNetSet;
extern StatusCallID SCallId;
extern StatusCallID SAlarmCallID;
extern StatusCallID sInviteCallID;
extern CallID InviteKeepAliveID;
extern char *ByeVia;
extern char *ByeFrom;
extern char *ByeTo;
extern BOOL bACK;
extern BOOL bBYE;
extern BOOL bCANCEL;
extern BOOL bShowRealTime;
extern BOOL bNodeParent;
extern char strBye[MAXBUFSIZE];
//�ж��Ƿ���������Ϣ
extern char *contact;
extern BOOL bSipRegister;
extern BOOL bNodeType;
extern int nOverTime;
extern int nCurrentTime;
extern int nTimeCount;
extern time_t oldTime, currentTime;
extern BOOL bOverTime;
extern BOOL bFlag;
extern BOOL bVerify;
extern BOOL bUDPSipConnect;
extern char strPlayUrl[250];
extern char InviteCallID[100];
extern char ptztag[30];
//��Ա����
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern struct hisQuery tHisVidQuery;
extern ptzQuery tPTZQuery;
extern struct Authenticate g_authInfo;
extern struct Authenticate guac_authInfo;

osip_cseq_t *cseq;
CSipMsgProcess::CSipMsgProcess(void)
{
	parser_init();
	osip_message_init(&m_SipMsg.msg);
	osip_via_init(&m_SipMsg.via);
	osip_to_init(&m_SipMsg.to);
	osip_from_init(&m_SipMsg.from);
	osip_call_id_init(&m_SipMsg.callid);
	osip_contact_init(&m_SipMsg.contact);
	osip_cseq_init(&m_SipMsg.cseq);
	osip_content_type_init(&m_SipMsg.content_type);
	osip_content_length_init(&m_SipMsg.content_length);
	osip_uri_init(&m_SipMsg.uriServer);
	osip_uri_init(&m_SipMsg.uriClient);
	m_Type = 66;
	beginIndex = 0;
	endIndex = 0;
}

CSipMsgProcess::~CSipMsgProcess(void)
{
	//OISP��Դ�ͷ�
	osip_via_free(m_SipMsg.via);
	osip_to_free(m_SipMsg.to);
	osip_from_free(m_SipMsg.from);
	osip_call_id_free(m_SipMsg.callid);
	osip_contact_free(m_SipMsg.contact);
	osip_cseq_free(m_SipMsg.cseq);
	osip_content_type_free(m_SipMsg.content_type);
	osip_content_length_free(m_SipMsg.content_length);
	osip_uri_free(m_SipMsg.uriServer);
	osip_uri_free(m_SipMsg.uriClient);
	osip_message_free(m_SipMsg.msg);
}

int CSipMsgProcess::SipParser(char *buffer, int Msglength)
{
	if (OSIP_SUCCESS != osip_message_init(&m_SipMsg.msg))
	{
		AfxMessageBox("OSIP������ʼ��ʧ��", MB_OK | MB_ICONERROR);
		return 1;
	}
	int i = osip_message_parse(m_SipMsg.msg, buffer, Msglength);
	if (i != OSIP_SUCCESS)
	{
		AfxMessageBox("SIP��Ϣ��������", MB_OK | MB_ICONERROR);
		return 1;
	}
	//m_SipMsg.msg->message = buffer;
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	if (m_SipMsg.msg->call_id->number == NULL)
	{
		m_SipMsg.msg->call_id->number = "";
		if (m_SipMsg.msg->call_id->host == NULL)
		{
			m_SipMsg.msg->call_id->host = "";
			AfxMessageBox("SIP��Ϣ��Call ID�ֶ�", MB_OK | MB_ICONERROR);
			return 0;
		}
	}
	//save XML message and parse the XML to the StrTemp
	char *XmlMessage = new char[XMLSIZE];
	memset(XmlMessage, 0, XMLSIZE);
	osip_body_t *XMLbody;
	osip_body_init(&XMLbody);
	osip_message_get_body(m_SipMsg.msg, 0, &XMLbody);
	if (XMLbody != NULL)
	{
		memcpy(XmlMessage, XMLbody->body, strlen(XMLbody->body));
		osip_body_free(XMLbody);
		XMLbody = NULL;
	}
	//parse the XML to the strXML
	string strXML(XmlMessage);
	if (m_SipMsg.msg->sip_method == NULL)//��ֹ�����strcmp����֤���ֿ�ָ��
		m_SipMsg.msg->sip_method = "";
	delete XmlMessage;//�Ѿ�����Ҫʹ����
	XmlMessage = NULL;
	//�ж��¼�����
//receive register message
	if (MSG_IS_REGISTER(m_SipMsg.msg))
	{
		m_Type = Register;
		//update timer
		nOverTime = atoi(Common::EXPIRES_VALUE);//��ʱʱ��
		bFlag = TRUE;
	}
//receive DO method		
	else if ((strcmp(m_SipMsg.msg->sip_method, "DO") == 0) && (m_SipMsg.msg->status_code == 0))
	{
		string temp;
		char CmdType[50];
		string::size_type CmdTypeStart;
		string::size_type CmdTypeEnd;
		if ((CmdTypeStart = strXML.find("<CmdType>", 0)) == string::npos)
		{
			AfxMessageBox("SIP��Ϣ��<CmdType>�ֶ�", MB_OK | MB_ICONERROR);
			return 1;
		}
		if ((CmdTypeEnd = strXML.find("</CmdType>", CmdTypeStart + 1)) == string::npos)
		{
			AfxMessageBox("SIP��Ϣ��</CmdType>�ֶ�", MB_OK | MB_ICONERROR);
			return 1;
		}
		temp = strXML.substr(CmdTypeStart + 10, CmdTypeEnd - CmdTypeStart - 10);
		strcpy(CmdType, temp.c_str());
		temp.erase(0, temp.length());
		if (strcmp(CmdType, "") == 0)
		{
			AfxMessageBox("CmdType is null", MB_OK | MB_ICONINFORMATION);
			return 1;
		}
		else if (strcmp(CmdType, "KeepAlive") == 0)//�ж��Ƿ�ʱע���������Ϣ
		{
			time(&currentTime);
			nCurrentTime = currentTime - oldTime;
			time(&oldTime);
			if (nCurrentTime < nOverTime)
			{
				string strXml = "<?xml version=\"1.0\"?>\r\n";
				strXml += "<Response>\r\n";
				strXml += "<CmdType>KeepAlive</CmdType>\r\n";
				strXml += "<Result>0</Result>\r\n";
				strXml += "</Response>\r\n";
				char *xml = new char[XMLSIZE];
				strcpy(xml, strXml.c_str());
				char *dst = new char[MAXBUFSIZE];
				Sip200Xml(&dst, m_SipMsg.msg, xml);
				UA_Msg uas_sendtemp;
				strcpy(uas_sendtemp.data, dst);
				EnterCriticalSection(&g_uas);
				uas_sendqueue.push(uas_sendtemp);
				LeaveCriticalSection(&g_uas);
				bFlag = TRUE;
				if (!bSipRegister)
					bSipRegister = TRUE;
				delete xml;
				delete dst;
				dst = NULL;
				xml = NULL;
			}
			else
			{
				bSipRegister = false;
				char *dst = new char[MAXBUFSIZE];
				Sip400(&dst, m_SipMsg.msg);
				UA_Msg uas_sendtemp;
				strcpy(uas_sendtemp.data, dst);
				EnterCriticalSection(&g_uas);
				uas_sendqueue.push(uas_sendtemp);
				LeaveCriticalSection(&g_uas);
				delete dst;
				dst = NULL;
			}
			return 0;
		}
		else if (strcmp(CmdType, "TimeGet") == 0)
		{
			m_Type = TimeGet;
			SCallId.nStatus = TimeGet;
			ShowTestData = " <---------  DO  \r\n";
			ShowTestTitle = "Ԥ��λ��ѯ";
		}
	}
	/************************************************************************/
	/*�����Ǹ��������UAC�˷��ص�XML���� writed by Bsp Lee   ���ֲ�ѯ�����ù���      for GB28181 */
	/***********************************************************************/
	else if ((strcmp(m_SipMsg.msg->sip_method, "Message") == 0) && (m_SipMsg.msg->status_code == 0))
	{
		string::size_type CmdTypeStart;
		string::size_type CmdTypeEnd;
		if ((CmdTypeStart = strXML.find("<CmdType>", 0)) == string::npos)
		{
			AfxMessageBox("SIP��Ϣ��<CmdType>�ֶ�", MB_OK | MB_ICONERROR);
			return 1;
		}
		if ((CmdTypeEnd = strXML.find("</CmdType>", CmdTypeStart + 1)) == string::npos)
		{
			AfxMessageBox("SIP��Ϣ��</CmdType>�ֶ�", MB_OK | MB_ICONERROR);
			return 1;
		}
		string CmdType = strXML.substr(CmdTypeStart + 9, CmdTypeEnd - CmdTypeStart - 9);
		if (CmdType.compare("") == 0)
		{
			AfxMessageBox("CmdType is null", MB_OK | MB_ICONINFORMATION);
			return 1;
		}
		else if (CmdType.compare("Alarm") == 0)
		{
			char *dst = new char[MAXBUFSIZE]; 
			Sip200OK(&dst, m_SipMsg.msg);
			UA_Msg uas_sendtemp;
			strcpy(uas_sendtemp.data, dst);
			EnterCriticalSection(&g_uas);
			uas_sendqueue.push(uas_sendtemp);
			LeaveCriticalSection(&g_uas);
			delete dst;
			dst = NULL;
			//update log				
			ShowTestData += "Alarm Notify  OK -------->\r\n";
			return 0;
		}
		else if ((CmdType.compare("Catalog") == 0)&& (strXML.find("DeviceList")!=string::npos))
		{
			char *dst = new char[MAXBUFSIZE];
			Sip200OK(&dst, m_SipMsg.msg);
			UA_Msg uas_sendtemp;
			strcpy(uas_sendtemp.data, dst);
			EnterCriticalSection(&g_uas);
			uas_sendqueue.push(uas_sendtemp);
			LeaveCriticalSection(&g_uas);
			delete dst;
			dst = NULL;
			//update log				
			ShowTestData += "Catalog Query  OK -------->\r\n";
			return 0;
		}
		else if ((CmdType.compare("Catalog") == 0) && (strXML.find("RecordList") != string::npos))
		{
			char *dst = new char[MAXBUFSIZE];
			Sip200OK(&dst, m_SipMsg.msg);
			UA_Msg uas_sendtemp;
			strcpy(uas_sendtemp.data, dst);
			EnterCriticalSection(&g_uas);
			uas_sendqueue.push(uas_sendtemp);
			LeaveCriticalSection(&g_uas);
			delete dst;
			dst = NULL;
			//update log				
			ShowTestData += "Catalog Query  OK -------->\r\n";

			//���ݵĲ��룬IDD_DLG_VIDEOQUERY�Ի����е�IDC_COMBO_LIST
			int start = strXML.find("<RecordListNum=");
			int end = strXML.find(">", start);
			string temp = strXML.substr(start + 15, end - start - 15);
			int recordListNum = stoi(temp);
			end = start  = 0;
			for (int i = 0; i < recordListNum; i++)
			{
				start = strXML.find("<FilePath>",end);
				end = strXML.find("</FilePath>", start);
				CString FilePath = strXML.substr(start+10, end-start-10).c_str();
				pWnd->m_VideoQuery.m_HistoryVideoList.InsertString(i, FilePath);
			}
			if (pWnd->m_VideoQuery.m_HistoryVideoList.GetCount())
				pWnd->m_VideoQuery.m_HistoryVideoList.SetCurSel(0);
			return 0;
		}
		else if (CmdType.compare("DeviceStatus") == 0)
		{
			char *dst = new char[MAXBUFSIZE];
			Sip200OK(&dst, m_SipMsg.msg);
			UA_Msg uas_sendtemp;
			strcpy(uas_sendtemp.data, dst);
			EnterCriticalSection(&g_uas);
			uas_sendqueue.push(uas_sendtemp);
			LeaveCriticalSection(&g_uas);
			delete dst;
			dst = NULL;
			//update log				
			ShowTestData += "DeviceStatus Query  OK -------->\r\n";
			return 0;
		}
		else if (CmdType.compare("DeviceInfo") == 0)
		{
			char *dst = new char[MAXBUFSIZE];
			Sip200OK(&dst, m_SipMsg.msg);
			UA_Msg uas_sendtemp;
			strcpy(uas_sendtemp.data, dst);
			EnterCriticalSection(&g_uas);
			uas_sendqueue.push(uas_sendtemp);
			LeaveCriticalSection(&g_uas);
			delete dst;
			dst = NULL;
			//update log				
			ShowTestData += "DeviceInfo Query  OK -------->\r\n";
			return 0;
		}
		else if (CmdType.compare("TimeGet") == 0)
		{
			m_Type = TimeGet;
			SCallId.nStatus = TimeGet;
			ShowTestData = " <---------  DO  \r\n";
			ShowTestTitle = "UAC��ʱ��������ȡ";
			return 0;
		}
	}
//receive notify message	
	else if (MSG_IS_NOTIFY(m_SipMsg.msg))
	{
		if (strcmp(m_SipMsg.msg->call_id->number, SAlarmCallID.CurID) == 0)
		{
			string str = buffer;
			//PTZCommand
			if (str.find("PTZCommand") != string::npos)
				m_Type = PTZ;
			//AlarmNotify
			else if(str.find("Alarm",0) != string::npos)
			{
				if ((str.find("<CmdType>", 0) != string::npos)
					&& (str.find("</CmdType>", 0) != string::npos)
					&& (str.find("<AlarmMethod>", 0) != string::npos)
					&& (str.find("</AlarmMethod>", 0) != string::npos)
					&& (str.find("<DeviceID>", 0) != string::npos)
					&& (str.find("</DeviceID>", 0) != string::npos)
					)
				{
					//update log
					ShowTestData = "<--------- NOTIFY\r\n";
					ShowTestTitle = "Alarm Send Test";
					int addressStart = str.find("<DeviceID>", 0);;
					int addressEnd = str.find("</DeviceID>", 0);
					CString DeviceID = str.substr(addressStart + 10, addressEnd - addressStart - 10).c_str();

					CString XmlAlarmSet;
					XmlAlarmSet = "<?xml version=\"1.0\"?>\r\n";
					XmlAlarmSet += "<Response>\r\n";
					XmlAlarmSet += "<CmdType>Alarm</CmdType>\r\n";
					XmlAlarmSet += "<SN>1</SN>\r\n";
					XmlAlarmSet += "<DeviceID>" + DeviceID + "</DeviceID>\r\n";
					XmlAlarmSet += "<Result>OK</Result>\r\n";
					XmlAlarmSet += "</Response>\r\n";
					char *dst = new char[MAXBUFSIZE];
					Sip200Xml(&dst, m_SipMsg.msg, XmlAlarmSet);
					UA_Msg uas_sendtemp;
					strcpy(uas_sendtemp.data, dst);
					EnterCriticalSection(&g_uas);
					uas_sendqueue.push(uas_sendtemp);
					LeaveCriticalSection(&g_uas);
					delete dst;
					ShowTestData += "200 OK ---------->\r\n";
				}
				else {
					char *dst = new char[MAXBUFSIZE];
					Sip400(&dst, m_SipMsg.msg);UA_Msg uas_sendtemp;
					strcpy(uas_sendtemp.data, dst);
					EnterCriticalSection(&g_uas);
					uas_sendqueue.push(uas_sendtemp);
					LeaveCriticalSection(&g_uas);
					delete dst;
				}
			}
			//������Ϣ
			else
			{
				//�������Ϣ
				ShowTestData += "<--------- NOTIFY\r\n";
				char *dst = new char[MAXBUFSIZE];
				Sip200OK(&dst, m_SipMsg.msg);
				UA_Msg uas_sendtemp;
				strcpy(uas_sendtemp.data, dst);
				EnterCriticalSection(&g_uas);
				uas_sendqueue.push(uas_sendtemp);
				LeaveCriticalSection(&g_uas);
				delete dst;
				ShowTestData += "200 OK ---------->\r\n";
			}

		}
		else//Ŀ¼����
		{
			m_Type = NodeType;
		}
	}
	else if (MSG_IS_BYE(m_SipMsg.msg))
	{
		char *dst = new char[MAXBUFSIZE];
		//Send sip 200 ok		
		char *dest = NULL;
		size_t len;
		CSipMsgProcess *Sip200 = new CSipMsgProcess;
		osip_message_set_version(Sip200->m_SipMsg.msg, "SIP/2.0");
		osip_message_set_status_code(Sip200->m_SipMsg.msg, 200);
		osip_message_set_reason_phrase(Sip200->m_SipMsg.msg, "OK");
		osip_call_id_clone(m_SipMsg.msg->call_id, &Sip200->m_SipMsg.msg->call_id);
		osip_from_clone(m_SipMsg.msg->from, &Sip200->m_SipMsg.msg->from);
		osip_to_clone(m_SipMsg.msg->to, &Sip200->m_SipMsg.msg->to);
		osip_cseq_clone(m_SipMsg.msg->cseq, &Sip200->m_SipMsg.msg->cseq);
		//copy via
		osip_message_get_via(m_SipMsg.msg, 0, &Sip200->m_SipMsg.via);
		osip_via_to_str(Sip200->m_SipMsg.via, &dest);
		osip_message_set_via(Sip200->m_SipMsg.msg, dest);
		osip_free(dest);
		osip_message_set_max_forwards(Sip200->m_SipMsg.msg, "70");
		osip_message_to_str(Sip200->m_SipMsg.msg, &dest, &len);
		memset(dst, 0, MAXBUFSIZE);
		memcpy(dst, dest, len);
		osip_free(dest);
		strcpy(uas_curSendMsg.data, dst);
		EnterCriticalSection(&g_uas);
		uas_sendqueue.push(uas_curSendMsg);
		LeaveCriticalSection(&g_uas);
		pWnd->m_Invite.GetDlgItem(IDC_BTN_BYE)->EnableWindow(FALSE);
		bBYE = TRUE;
		pWnd->bSelectKeepLive = TRUE;
		pWnd->m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(TRUE);
		pWnd->m_Invite.GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
		delete dst;
		//���������ӽ����طźͽ��������Ĵ���
		TerminateProcess(vlc.hProcess, 0);
		TerminateProcess(rtpsend.hProcess, 0);
	}
//SAlarmCallID �����¼�Ԥ�����մ洢
	else if (strcmp(m_SipMsg.msg->call_id->number, SAlarmCallID.CurID) == 0 /*&&strcmp(m_SipMsg.msg->call_id->host,SAlarmCallID.CurHost)==0*/)
	{
		// check from or to information
		if (!SipVerify(m_InfoServer, m_InfoClient, m_SipMsg.msg, 1))
		{
			AfxMessageBox(" Alarm Subscribe From or To is error", MB_OK | MB_ICONERROR);
			return 0;
		}
		if (m_SipMsg.msg->from->gen_params.nb_elt == 0)
		{
			AfxMessageBox("from have no tag");
			return 0;
		}
		osip_uri_param_t *h;
		osip_uri_param_init(&h);
		osip_from_get_tag(m_SipMsg.msg->from, &h);
		char Tag[10];
		strcpy(Tag, h->gvalue);
		osip_uri_param_free(h);
		if (strcmp(Tag, SAlarmCallID.CurTag) == 0)
		{
			m_Type = Alarm;
			//water:��¼�Է����ص�to tag
			osip_to_t *dest;
			osip_to_init(&dest);
			osip_uri_param_t *totag;
			dest = osip_message_get_to(m_SipMsg.msg);
			osip_to_get_tag(dest, &totag);
			strcpy(SAlarmCallID.CurToTag, totag->gvalue);
		}
		else
		{
			AfxMessageBox("from tag is error", MB_OK | MB_ICONERROR);
			return 0;
		}
	}
//����ƥ����ʵʱ����200OK
	else if (strcmp(m_SipMsg.msg->call_id->number, sInviteCallID.CurID) == 0 /*&& strcmp(m_SipMsg.msg->call_id->host, sInviteCallID.CurHost) == 0*/)
	{
		if (!InviteSipVerify(m_InfoServer, m_InfoClient, pWnd->inviteAddress, m_SipMsg.msg, 1))
		{
			AfxMessageBox("From or To is error");
			return 0;
		}
		if (m_SipMsg.msg->from->gen_params.nb_elt == 0)
		{
			AfxMessageBox("from CmdType have no tag", MB_OK | MB_ICONEXCLAMATION);
			return 0;
		}
		osip_uri_param_t *h;
		osip_uri_param_init(&h);
		osip_from_get_tag(m_SipMsg.msg->from, &h);
		char Tag[10];
		strcpy(Tag, h->gvalue);
		osip_uri_param_free(h);
		if (strcmp(Tag, sInviteCallID.CurTag) == 0)
		{
			string str = buffer;
			int i = str.find("PTZCommand");
			if (i == string::npos)
				m_Type = Invite;
			else
				m_Type = PTZ;
		}
		else if (strcmp(Tag, InviteKeepAliveID.Tag) == 0)
		{
			if (bShowRealTime)
				return 0;
			else
			{
				bShowRealTime = TRUE;
				return 0;
			}
		}
		else if (strcmp(Tag, ptztag) == 0)
			m_Type = PTZ;
		else
		{
			AfxMessageBox("From Tag is error", MB_OK | MB_ICONERROR);
			return 0;
		}
	}

	else if (strcmp(m_SipMsg.msg->call_id->number, SCallId.CurID) == 0 /*&& strcmp(m_SipMsg.msg->call_id->host, SCallId.CurHost) == 0*/)
	{
		if (m_SipMsg.msg->from->gen_params.nb_elt == 0)
		{
			AfxMessageBox("from CmdType have no tag", MB_OK | MB_ICONEXCLAMATION);
			return 0;
		}
		osip_uri_param_t *h;
		osip_uri_param_init(&h);
		osip_from_get_tag(m_SipMsg.msg->from, &h);
		char Tag[10];
		strcpy(Tag, h->gvalue);
		osip_uri_param_free(h);
		//��������m_Type
		if (strcmp(Tag, SCallId.CurTag) == 0)
			m_Type = SCallId.nStatus;
		else
		{
			AfxMessageBox("Tag is error", MB_OK | MB_ICONERROR);
			return 0;
		}
	}
//create response message
	switch (m_Type)
	{
	case Register:
		{
			bUDPSipConnect = TRUE;

			//update log
			ShowTestTitle = "Register Test";
			ShowTestData = "<--------- REGISTER \r\n";
			//update client information
			bNodeParent = FALSE;
			m_InfoClient.UserName = m_SipMsg.msg->from->displayname;
			m_InfoClient.UserAddress = m_SipMsg.msg->from->url->username;

			cseq = m_SipMsg.msg->cseq;

			osip_header * expires = NULL;
			osip_message_get_expires(m_SipMsg.msg, 0, &expires);
			string strTemp(buffer);
			string temp;
			string::size_type Start;
			string::size_type End;
			char *dst = new char[MAXBUFSIZE]; 
			if ( (Start=strTemp.find("Authorization",0))==string::npos)
			{
				Sip401(&dst,m_SipMsg.msg);
			}
			else
			{
				int index=strTemp.find("username");
				if (index==string::npos)
				{
					AfxMessageBox("Authorizationȱ��username��Ϣ��",MB_OK|MB_ICONERROR);
					return 0;
				}
				int index1=strTemp.find('"',index);
				int index2=strTemp.find('"',index1+1);
				guac_authInfo.username=strTemp.substr(index1+1,index2-index1-1);
				if (guac_authInfo.username.compare(g_authInfo.username)!=0)
				{
					AfxMessageBox("Authorization,username��һ�£�",MB_OK|MB_ICONERROR);
					return 0;
				}

				index=strTemp.find("realm");
				if (index==string::npos)
				{
					AfxMessageBox("Authorizationȱ��realm��Ϣ��",MB_OK|MB_ICONERROR);
					return 0;
					}
				index1=strTemp.find('"',index);
				index2=strTemp.find('"',index1+1);
				guac_authInfo.realm=strTemp.substr(index1+1,index2-index1-1);
				if (guac_authInfo.realm.compare(g_authInfo.realm)!=0)
				{
					AfxMessageBox("Authorization,realm��һ�£�",MB_OK|MB_ICONERROR);
					return 0;
				}

				index=strTemp.find("nonce");
				if (index==string::npos)
				{
					AfxMessageBox("Authorizationȱ��noce��Ϣ��",MB_OK|MB_ICONERROR);
					return 0;
				}
				index1=strTemp.find('"',index);
				index2=strTemp.find('"',index1+1);
				guac_authInfo.nonce=strTemp.substr(index1+1,index2-index1-1);
				if (guac_authInfo.nonce.compare(g_authInfo.nonce)!=0)
				{
					AfxMessageBox("Authorization,nonce��һ�£�",MB_OK|MB_ICONERROR);
					return 0;	
				}
				//opaque
				index=strTemp.find("opaque");
				if (index==string::npos)
				{
			 		AfxMessageBox("Authorizationȱ��opaque��Ϣ��",MB_OK|MB_ICONERROR);
			 		return 0;
				}
				index1=strTemp.find('"',index);
				index2=strTemp.find('"',index1+1);
				guac_authInfo.opaque=strTemp.substr(index1+1,index2-index1-1);
				//uri
				index=strTemp.find("uri");
				if (index==string::npos)
				{
			 		AfxMessageBox("Authorizationȱ��uri��Ϣ��",MB_OK|MB_ICONERROR);
			 		return 0;
				}
				//index1=strTemp.find('"',index);
				//index2=strTemp.find('"',index1+1);
				//guac_authInfo.uri=strTemp.substr(index1+1,index2-index1-1);
				//if (guac_authInfo.uri.compare(g_authInfo.uri)!=0)
				//{
			 //		AfxMessageBox("Authorization,uri��һ�£�",MB_OK|MB_ICONERROR);
			 //		return 0;
				//}
				//nc
				index=strTemp.find("nc=");
				if (index==string::npos)
				{
					AfxMessageBox("Authorizationȱ��nc��Ϣ��",MB_OK|MB_ICONERROR);
					return 0;
				}
				index1=strTemp.find('"',index);
				index2=strTemp.find('"',index1+1);
				guac_authInfo.nc=strTemp.substr(index1+1,index2-index1-1);
				//cnonce
				index=strTemp.find("cnonce");
				if (index==string::npos)
				{
					AfxMessageBox("Authorizationȱ��cnonce��Ϣ��",MB_OK|MB_ICONERROR);
					return 0;
				}
				index1=strTemp.find('"',index);
				index2=strTemp.find('"',index1+1);
				guac_authInfo.cnonce=strTemp.substr(index1+1,index2-index1-1);
				//response
				index=strTemp.find("response");
				if (index==string::npos)
				{
					AfxMessageBox("Authorizationȱ��response��Ϣ��",MB_OK|MB_ICONERROR);
					return 0;
				}
				index1=strTemp.find('"',index);
				index2=strTemp.find('"',index1+1);
				guac_authInfo.response=strTemp.substr(index1+1,index2-index1-1);

				char HA1[16 * 2 + 1];
				char HA2[16 * 2 + 1];
				char response[16 * 2 + 1];
				{//HA1=MD5(A1)=MD5(user:realm:password) ---->RFC2069
					md5_state_t state;
					md5_byte_t digest[16];
					int di;
					md5_init(&state);
					CString cstrGroup = g_authInfo.username.c_str();
					cstrGroup += ":";
					cstrGroup += g_authInfo.realm.c_str();
					cstrGroup += ":";
					cstrGroup += g_authInfo.password.c_str();
					md5_append(&state, (const md5_byte_t *)(cstrGroup.GetBuffer(cstrGroup.GetLength())), cstrGroup.GetLength());
					md5_finish(&state, digest);
					for (di = 0; di < 16; ++di)
						sprintf(HA1 + di * 2, "%02x", digest[di]);
				}
				{//HA2=MD5(A2)=MD5(method:digestURI) ---->RFC2069
					md5_state_t state;
					md5_byte_t digest[16];
					int di;
					md5_init(&state);
					CString cstrGroup = "REGISTER:";
					cstrGroup += g_authInfo.uri.c_str();
					md5_append(&state, (const md5_byte_t *)(cstrGroup.GetBuffer(cstrGroup.GetLength())), cstrGroup.GetLength());
					md5_finish(&state, digest);
					for (di = 0; di < 16; ++di)
						sprintf(HA2 + di * 2, "%02x", digest[di]);
				}
				{//response=MD5(HA1:nonce:HA2) ---->RFC2069
					md5_state_t state;
					md5_byte_t digest[16];
					int di;
					md5_init(&state);
					//CString cstrGroup=CString(HA1)+CString(g_authInfo.nonce.c_str())+CString(HA2);
					CString cstrGroup = CString(HA1) + ":" + CString(g_authInfo.nonce.c_str()) + ":" + CString(guac_authInfo.nc.c_str()) + ":" + CString(guac_authInfo.cnonce.c_str()) + ":" + CString(HA2);

					md5_append(&state, (const md5_byte_t *)(cstrGroup.GetBuffer(cstrGroup.GetLength())), cstrGroup.GetLength());
					md5_finish(&state, digest);
					for (di = 0; di < 16; ++di)
						sprintf(response + di * 2, "%02x", digest[di]);
				}
				g_authInfo.response = response;

				Sip200OKForRegOrQuit(&dst, m_SipMsg.msg);
				if (strcmp(expires->hvalue, "0") != 0) //ע��ɹ�
				{
					//�����ʷ��Ƶ��ѯ��Ϣ
					pWnd->m_VideoInfo.clear();
					pWnd->m_VideoQuery.m_HistoryVideoList.ResetContent();
					//update client information and show
					pWnd->m_CatalogQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText(m_InfoClient.UserAddress);
					pWnd->m_FlowQuery.m_CatQueAddress.SetWindowTextA(m_InfoClient.UserAddress);
					pWnd->GetDlgItem(IDC_STR_REMOTE_ADD)->SetWindowText(m_InfoClient.UserAddress);
					pWnd->GetDlgItem(IDC_STR_REMOTE_NAME)->SetWindowText(m_InfoClient.UserName);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(TRUE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_PLAY)->EnableWindow(TRUE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
					pWnd->m_PTZ.GetDlgItem(IDC_BTN_TEST)->EnableWindow(TRUE);
					pWnd->m_PTZ.GetDlgItem(IDC_BTN_PRE)->EnableWindow(TRUE);
					pWnd->m_VideoQuery.GetDlgItem(IDC_BTN_QUERY)->EnableWindow(TRUE);
					pWnd->m_VideoPlay.GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
					pWnd->m_Alarm.GetDlgItem(IDC_BTN_ALARM_SET)->EnableWindow(TRUE);
					pWnd->m_Alarm.GetDlgItem(IDC_BTN_ALARM_CANCEL)->EnableWindow(TRUE);
					pWnd->m_CatalogQuery.GetDlgItem(IDC_QUERY)->EnableWindow(TRUE);
					pWnd->m_DeviceInfQuery.GetDlgItem(IDC_DEVICEINFQUERY)->EnableWindow(TRUE);
					pWnd->m_CatalogQuery.GetDlgItem(IDC_DEVICEINFQUERY2)->EnableWindow(TRUE);
					pWnd->m_FlowQuery.GetDlgItem(IDC_FLOWQUERY)->EnableWindow(TRUE);
					pWnd->m_CoderSet.GetDlgItem(IDC_BTN_SET)->EnableWindow(TRUE);
					pWnd->m_PSTVSetTime.GetDlgItem(IDC_BUTTON_NTPTIME)->EnableWindow(TRUE);
					pWnd->m_PSTVSetTime.GetDlgItem(IDC_BUTTON_CurTIME)->EnableWindow(TRUE);
					pWnd->m_PSTVSetTime.GetDlgItem(IDC_BUTTON_SETTIME)->EnableWindow(TRUE);
					pWnd->m_CaptureImg.GetDlgItem(IDC_BUTTON_DOCAPTURE)->EnableWindow(TRUE);

					pWnd->m_CeventNotice.GetDlgItem(IDC_BUTTON_NOTICE_DISTRIBUTE)->EnableWindow(TRUE);
				}
				else //ע���ɹ�
				{
					//�����ʷ��Ƶ��ѯ��Ϣ
					pWnd->m_VideoInfo.clear();
					pWnd->m_VideoQuery.m_HistoryVideoList.ResetContent();
					//update client information and show
					pWnd->m_CatalogQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText(" ");
					pWnd->m_FlowQuery.m_CatQueAddress.SetWindowTextA(" ");
					pWnd->GetDlgItem(IDC_STR_REMOTE_ADD)->SetWindowText(" ");
					pWnd->GetDlgItem(IDC_STR_REMOTE_NAME)->SetWindowText(" ");
					pWnd->m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE);
					pWnd->m_PTZ.GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);
					pWnd->m_PTZ.GetDlgItem(IDC_BTN_PRE)->EnableWindow(FALSE);
					pWnd->m_VideoQuery.GetDlgItem(IDC_BTN_QUERY)->EnableWindow(FALSE);
					pWnd->m_VideoPlay.GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
					pWnd->m_Alarm.GetDlgItem(IDC_BTN_ALARM_SET)->EnableWindow(FALSE);
					pWnd->m_Alarm.GetDlgItem(IDC_BTN_ALARM_CANCEL)->EnableWindow(FALSE);
					pWnd->m_CatalogQuery.GetDlgItem(IDC_QUERY)->EnableWindow(FALSE);
					pWnd->m_DeviceInfQuery.GetDlgItem(IDC_DEVICEINFQUERY)->EnableWindow(FALSE);
					pWnd->m_CatalogQuery.GetDlgItem(IDC_DEVICEINFQUERY2)->EnableWindow(FALSE);
					pWnd->m_FlowQuery.GetDlgItem(IDC_FLOWQUERY)->EnableWindow(FALSE);
					pWnd->m_CoderSet.GetDlgItem(IDC_BTN_SET)->EnableWindow(FALSE);
					pWnd->m_PSTVSetTime.GetDlgItem(IDC_BUTTON_NTPTIME)->EnableWindow(FALSE);
					pWnd->m_PSTVSetTime.GetDlgItem(IDC_BUTTON_CurTIME)->EnableWindow(FALSE);
					pWnd->m_PSTVSetTime.GetDlgItem(IDC_BUTTON_SETTIME)->EnableWindow(FALSE);
					pWnd->m_CaptureImg.GetDlgItem(IDC_BUTTON_DOCAPTURE)->EnableWindow(FALSE);

					pWnd->m_CeventNotice.GetDlgItem(IDC_BUTTON_NOTICE_DISTRIBUTE)->EnableWindow(FALSE);
				}
			}

			UA_Msg uas_sendtemp;
			strcpy(uas_sendtemp.data, dst);
			EnterCriticalSection(&g_uas);
			uas_sendqueue.push(uas_sendtemp);
			LeaveCriticalSection(&g_uas);
			delete dst;
			dst = NULL;
			//update log				
			ShowTestData += "200  OK -------->\r\n";
			//���ò��Ա������־���̰�ť
			pWnd->EnableWindow(IDC_BTN_LOG, TRUE);
			pWnd->EnableWindow(IDC_BTN_RESULT, TRUE);
			bOverTime = TRUE;
			time(&oldTime);
		}
		break;
	case NodeType:
		{
			ShowTestData += "<----------- NOTIFY\r\n";
			//������������Ϣ�壬��ʾ��Ŀ¼����			string strTemp(buffer);
			string strTemp(buffer); 
			string temp;
			string::size_type VideoNumStart;
			string::size_type VideoNumEnd;
			if (strTemp.find("<CmdType>Catalog</CmdType>",0) != string::npos)
			{
				//analyse the CmdType "Parent"
				if (VideoNumStart=strTemp.find("<Parent>", 0) == string::npos)
				{
					AfxMessageBox("����ȱ��parent�ֶΣ�", MB_OK | MB_ICONEXCLAMATION);
					return 1;
				}
				if (VideoNumEnd=strTemp.find("</Parent>", 0) == string::npos)
				{
					AfxMessageBox("����ȱ��/parent�ֶΣ�", MB_OK | MB_ICONEXCLAMATION);
					return 1;
				}
				temp = strTemp.substr(VideoNumStart + 8, VideoNumEnd - VideoNumStart - 8);
				CString Parent;
				Parent.Format("%s", temp.c_str());
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("Parent is null", MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
				temp.erase(0, temp.length());
				if (!bNodeParent)
				{
					bNodeParent = TRUE;
				}
				else{}

				NotifyResponseAnylse(NotifyInfo, buffer);//�豸��Ϣ�Ĵ洢
				pWnd->m_Invite.m_selAddress.ResetContent();
				pWnd->m_PTZ.m_selAddress.ResetContent();
				pWnd->m_VideoQuery.m_selAddress.ResetContent();
				pWnd->m_Alarm.m_selAddress.ResetContent();
				pWnd->m_DeviceInfQuery.m_selAddress.ResetContent();
				pWnd->m_FlowQuery.m_selAddress.ResetContent();

				for (int i = 0; i<NotifyInfo.Devices.size(); i++)
				{
					pWnd->m_Invite.m_selAddress.InsertString(i, NotifyInfo.Devices[i].Name);
					pWnd->m_PTZ.m_selAddress.InsertString(i, NotifyInfo.Devices[i].Name);
					pWnd->m_VideoQuery.m_selAddress.InsertString(i, NotifyInfo.Devices[i].Name);
					pWnd->m_Alarm.m_selAddress.InsertString(i, NotifyInfo.Devices[i].Name);

					pWnd->m_CatalogQuery.m_selAddress.InsertString(i, NotifyInfo.Devices[i].Name);
					pWnd->m_DeviceInfQuery.m_selAddress.InsertString(i, NotifyInfo.Devices[i].Name);
					pWnd->m_FlowQuery.m_selAddress.InsertString(i, NotifyInfo.Devices[i].Name);

					pWnd->m_CeventNotice.m_selAddress.InsertString(i, NotifyInfo.Devices[i].Name);
				}
				if (NotifyInfo.Devices.size()>0)
				{
					pWnd->m_Invite.m_selAddress.SetCurSel(0);
					pWnd->m_Invite.GetDlgItem(IDC_EDT_PROTOCOL)->SetWindowTextA(NotifyInfo.Devices[0].Address);
					pWnd->m_PTZ.m_selAddress.SetCurSel(0);
					pWnd->m_PTZ.GetDlgItem(IDC_EDT_ADD)->SetWindowTextA(NotifyInfo.Devices[0].Address);
					pWnd->m_VideoQuery.m_selAddress.SetCurSel(0);
					pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(NotifyInfo.Devices[0].Address);
					pWnd->m_CatalogQuery.m_selAddress.SetCurSel(0);
					pWnd->m_CatalogQuery.GetDlgItem(IDC_EDIT_CHILDID)->SetWindowTextA(NotifyInfo.Devices[0].Address);
					pWnd->m_Alarm.m_selAddress.SetCurSel(0);
					pWnd->m_Alarm.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(NotifyInfo.Devices[0].Address);
					pWnd->m_DeviceInfQuery.m_selAddress.SetCurSel(0);
					pWnd->m_DeviceInfQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(NotifyInfo.Devices[0].Address);
					//pWnd->m_FlowQuery.m_selAddress.SetCurSel(0);
					//pWnd->m_FlowQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(NotifyInfo.Devices[0].Address);


					pWnd->m_CeventNotice.m_selAddress.SetCurSel(0);
					pWnd->m_CeventNotice.GetDlgItem(IDC_EDIT_EVENT_DEVICEID)->SetWindowTextA(NotifyInfo.Devices[0].Address);

				}
				string strXml = "<?xml version=\"1.0\"?>\r\n";
				strXml += "<Response>\r\n";
				strXml += "<CmdType>Catalog</CmdType>\r\n";
				strXml += "<Result>0</Result>\r\n";
				strXml += "</Response>\r\n";
				char *xml = new char[XMLSIZE];
				strcpy(xml, strXml.c_str());
				char *dst = new char[MAXBUFSIZE];
				Sip200Xml(&dst, m_SipMsg.msg, xml);
				UA_Msg uas_sendtemp;
				strcpy(uas_sendtemp.data, dst);
				EnterCriticalSection(&g_uas);
				uas_sendqueue.push(uas_sendtemp);
				LeaveCriticalSection(&g_uas);
				delete xml;
				xml = NULL;
				delete dst;
				dst = NULL;
				ShowTestData += "200 OK -------->\r\n";
			}
			//������������Ϣ�壬��ʾ�Ǳ���֪ͨ
			else if (strXML.find("<CmdType>AlarmNotify</CmdType>", 0) != string::npos)
			{
				string strXml = "<?xml version=\"1.0\"?>\r\n";
				strXml += "<Response>\r\n";
				strXml += "<CmdType>AlarmNotify</CmdType>\r\n";
				strXml += "<AlarmType>1</AlarmType>\r\n";
				strXml += "<Result>0</Result>\r\n";
				strXml += "</Response>\r\n";
				char *xml = new char[XMLSIZE];
				strcpy(xml, strXml.c_str());
				char *dst = new char[MAXBUFSIZE];
				Sip200Xml(&dst, m_SipMsg.msg, xml);
				UA_Msg uas_sendtemp;
				strcpy(uas_sendtemp.data, dst);
				EnterCriticalSection(&g_uas);
				uas_sendqueue.push(uas_sendtemp);
				LeaveCriticalSection(&g_uas);
				delete xml;
				xml = NULL;
				delete dst;
				dst = NULL;
				ShowTestData += "200 OK -------->\r\n";
			}
		}
		break;
	case Invite:
		{
			if (m_SipMsg.msg->status_code == 200 || m_SipMsg.msg->status_code == 500)
			{
				if (bACK && bBYE)
				{
					//update button status
					pWnd->m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(TRUE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_BYE)->EnableWindow(FALSE);
					bACK = FALSE;
				}
				else if (bCANCEL)
				{
					//update button status
					pWnd->m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(TRUE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_BYE)->EnableWindow(FALSE);
					bACK = FALSE;
					break;
				}
				else //��Ӧ��־��Ϊ�� bACK bBYE bCANCLE
				{
					ShowTestData += "200 OK <------------ \r\n";
					string strTemp = buffer;
					string temp;
					string::size_type VideoNumStart;
					string::size_type VideoNumEnd;

					//water:�ж��Ƿ�Ϊ�鲥����δ���о������ˣ���Ҫ���޸�
					/*if ((VideoNumStart = strTemp.find("<Multicast>", 0)) == string::npos)
						return 1;
					if ((VideoNumEnd = strTemp.find("</Multicast>", VideoNumStart + 1)) == string::npos)
						return 1;
					*/
					//temp = strTemp.substr(VideoNumStart + 11, VideoNumEnd - VideoNumStart - 11);
					//int m_multicast = atoi(temp.c_str());

					if ((VideoNumStart = strTemp.find("<SendSocket>", 0)) == string::npos)
						return 1;
					if ((VideoNumEnd = strTemp.find("</SendSocket>", VideoNumStart + 1)) == string::npos)
						return 1;
					//������socket�ֶ��еĵ�ַ�Ͷ˿�
					temp = strTemp.substr(VideoNumStart + 12, VideoNumEnd - VideoNumStart - 12);//224.0.0.1 UDP 2300
					int j = temp.find(' ', 0);
					string ip = temp.substr(0, j);//"224.0.0.1"
					string ip_first = ip;
					i = temp.find('P', j);
					temp.erase(0, i + 1);
					//�ж��Ƿ�Ϊ�鲥
					int m_multicast;
					int firstnum;
					i = ip_first.find('.', 0);
					ip_first = ip_first.substr(0, i);
					firstnum = atoi(ip_first.c_str());
					if ((firstnum > 223) && (firstnum < 240))
					{
						m_multicast = 1;
					}
					else
						m_multicast = 0;
					// Send ACK Message
					char *dst = new char[MAXBUFSIZE];
					SipACK(&dst, m_SipMsg.msg);
					//pWnd->SendData(dst);	
					UA_Msg uas_sendtemp;
					strcpy(uas_sendtemp.data, dst);
					EnterCriticalSection(&g_uas);
					uas_sendqueue.push(uas_sendtemp);
					LeaveCriticalSection(&g_uas);
					//pWnd->ShowSendData(dst);
					delete dst;
					bACK = TRUE;
					//update log
					ShowTestData += "ACK --------->\r\n";
					// create bye message
					//string port;
					//GetDlgItem(IDC_EDT_SOCKET)->GetWindowText(port);
					//����дVLC�����ļ�

					string m_vlc_sdp_str;
					if (!m_multicast)
					{
						m_vlc_sdp_str += "m=video ";
						m_vlc_sdp_str += "5540 ";
						m_vlc_sdp_str += "RTP/AVP 96\r";
						m_vlc_sdp_str += "a=rtpmap:96 H264/90000;\r";
						m_vlc_sdp_str += "a=framerate:25\r";
						m_vlc_sdp_str += "c=IN IP4 ";
						m_vlc_sdp_str += "127.0.0.1";
						m_vlc_sdp_str += "\r";
					}
					else
					{
						m_vlc_sdp_str += "m=video ";
						m_vlc_sdp_str += temp;
						m_vlc_sdp_str += "RTP/AVP 96\r";
						m_vlc_sdp_str += "a=rtpmap:96 H264/90000;\r";
						m_vlc_sdp_str += "a=framerate:25\r";
						m_vlc_sdp_str += "c=IN IP4 ";
						m_vlc_sdp_str += ip;
						m_vlc_sdp_str += "\r";
					}
					//д�����ļ�
					FILE *sdp;
					sdp = fopen(".//VLC//start.sdp", "wt+");
					if (sdp == NULL)
					{
						MessageBox(NULL, "Creat SDP Files Failed", "Error", MB_OK);
					}
					fprintf(sdp, "%s", m_vlc_sdp_str.c_str());
					fclose(sdp);
					//������������ʵʱ�����ֵĴ���
					ZeroMemory(&vlc, sizeof(vlc));
					vlc.cbSize = sizeof(vlc);
					vlc.fMask = SEE_MASK_NOCLOSEPROCESS;
					vlc.hwnd = NULL;
					vlc.lpVerb = NULL;
					vlc.lpFile = "vlc.exe";
					vlc.lpParameters = "start.sdp";
					vlc.lpDirectory = ".\\vlc\\";
					vlc.nShow = SW_SHOW;
					vlc.hInstApp = NULL;
					//ShellExecute(NULL, "open", "keppAliveSendTest.exe", NULL, ".\\vlc\\", SW_SHOWNORMAL);
					ShellExecuteEx(&vlc);

					////����RTP���ճ���
					if (!m_multicast)
					{
						//RTP���ճ���д�����ļ�
						CString str_localip;
						str_localip = m_InfoServer.IP;
						unsigned long localip = inet_addr(str_localip.GetBuffer());//��λ�ȡ����IP��
						localip = htonl(localip);
						unsigned long remoteip = inet_addr(ip.c_str());
						remoteip = htonl(remoteip);

						unsigned long localport = 1236;//�Լ��Ľ��ն˿ڱ�����дΪ1236
						unsigned long remoteport = atoi(temp.c_str());
						ofstream rtpcfg;
						rtpcfg.open(".\\RTPSend\\ipaddr.cfg");
						rtpcfg << localip << endl << remoteip << endl << localport << endl << remoteport << endl;
						rtpcfg.close();
						//�����հ�����
						ZeroMemory(&rtpsend, sizeof(rtpsend));
						rtpsend.cbSize = sizeof(rtpsend);
						rtpsend.fMask = SEE_MASK_NOCLOSEPROCESS;
						rtpsend.hwnd = NULL;
						rtpsend.lpVerb = NULL;
						rtpsend.lpFile = "keepAiliveTest.exe";
						rtpsend.lpParameters = NULL;
						rtpsend.lpDirectory = ".\\RTPsend\\";
						rtpsend.nShow = SW_SHOW;
						rtpsend.hInstApp = NULL;
						//ShellExecute(NULL, "open", "keppAliveSendTest.exe", NULL, ".\\rtpsend\\", SW_SHOWNORMAL);
						ShellExecuteEx(&rtpsend);

						//ShellExecute(NULL, "open", ".exe", NULL, ".\\RTPsend\\", SW_SHOWNORMAL);
					}

					char *datatemp = new char[MAXBUFSIZE];
					SipBYE(&datatemp, m_SipMsg.msg);
					strcpy(strBye, datatemp);
					delete datatemp;
					datatemp = NULL;
					pWnd->m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(TRUE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_BYE)->EnableWindow(TRUE);
					pWnd->m_Invite.GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
					//water:��¼�Է����ص�totag
					osip_to_t *dest;
					osip_to_init(&dest);
					osip_uri_param_t *totag;
					dest = osip_message_get_to(m_SipMsg.msg);
					osip_to_get_tag(dest, &totag);
					if(totag != nullptr)
						strcpy(sInviteCallID.CurToTag, totag->gvalue);
					else
						pWnd->MessageBox("toTagΪ��", "UAS ���Ľ�������", MB_OK | MB_ICONERROR);
					//water:end
				}
			}
			else if (m_SipMsg.msg->status_code == 100)
				ShowTestData += "trying 100 <------------\r\n";
			else if (m_SipMsg.msg->status_code == 101)
				ShowTestData += "101 <------------\r\n";
			else if (m_SipMsg.msg->status_code == 180)
				ShowTestData += "180 <------------\r\n";
			else if (m_SipMsg.msg->status_code == 400)
			{
				pWnd->ShowRecvData("\t-------ʵʱ������ʧ��--------\r\n");
				ShowTestData += "400 <------------\r\n";
			}
			else
				return 1;
		}
		break;
	case PTZ:
		{
			if (m_SipMsg.msg->status_code == 200)
			{
				//receive 200 ok message
				string temp;
				string::size_type VideoNumStart;
				string::size_type VideoNumEnd;
				if ((VideoNumStart = strXML.find("<Result>", 0)) == string::npos)
					return 1;
				if ((VideoNumEnd = strXML.find("</Result>", VideoNumStart + 1)) == string::npos)
					return 1;
				temp = strXML.substr(VideoNumStart + 8, VideoNumEnd - VideoNumStart - 8);
				CString Result;
				Result.Format("%s", temp.c_str());
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("Result is null", MB_OK | MB_ICONERROR);
					return 0;
				}
				temp.erase(0, temp.length());
				int nResult = atoi(Result);
				if (nResult == 0)
				{
					pWnd->ShowRecvData("\t----��̨�����ɹ�----\r\n");
				}
				else
				{
					pWnd->ShowRecvData("\t----��̨����ʧ��----\r\n");
				}
				//update log
				pWnd->ShowRecvData("\t----��̨���Գɹ�----\r\n");
				ShowTestData += " <---------  200 OK\r\n";
			}
			else if (m_SipMsg.msg->status_code == 400)
			{
				//receive 400 ok message
				pWnd->ShowRecvData("\t----��̨����ʧ��----\r\n");
				//update log				
				ShowTestData += " <---------  400\r\n";
			}
			else
			{
				//receive other message				
				return 1;
			}

		}
		break;
	case PreBitSet:
		{
			if (m_SipMsg.msg->status_code == 200)
			{
				//receive 200 ok message
				string temp;
				string::size_type VideoNumStart;
				string::size_type VideoNumEnd;
				if ((VideoNumStart = strXML.find("<Result>", 0)) == string::npos)
					return 1;
				if ((VideoNumEnd = strXML.find("</Result>", VideoNumStart + 1)) == string::npos)
					return 1;
				temp = strXML.substr(VideoNumStart + 8, VideoNumEnd - VideoNumStart - 8);
				CString Result;
				Result.Format("%s", temp.c_str());
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("Result is null", MB_OK | MB_ICONERROR);
					return 0;
				}
				temp.erase(0, temp.length());
				int nResult = atoi(Result);

				if ((VideoNumStart = strXML.find("<RealPresetNum>", 0)) == string::npos)
				{
					return 1;
				}
				if ((VideoNumEnd = strXML.find("</RealPresetNum>", VideoNumStart + 1)) == string::npos)
				{
					return 1;
				}
				temp = strXML.substr(VideoNumStart + 15, VideoNumEnd - VideoNumStart - 15);
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("RealPresetNum is null", MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
				CString destNum;
				destNum.Format("%s", temp.c_str());
				temp.erase(0, temp.length());
				RealNum = atoi(destNum);

				if ((VideoNumStart = strXML.find("<RemainPresetNum>", 0)) == string::npos)
				{
					return 1;
				}
				if ((VideoNumEnd = strXML.find("</RemainPresetNum>", VideoNumStart + 1)) == string::npos)
				{
					return 1;
				}
				temp = strXML.substr(VideoNumStart + 17, VideoNumEnd - VideoNumStart - 17);
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("RemainPresetNum is null", MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
				//CString destNum;
				destNum.Format("%s", temp.c_str());
				temp.erase(0, temp.length());
				RemainNum = atoi(destNum);

				if (nResult == 0)
				{

					if(RemainNum>0)
					{
						CString strTemp;
						strTemp = "<?xml version=\"1.0\"?>\r\n";
						strTemp += "<Action>\r\n";
						strTemp += "<Query>\r\n";
						strTemp += "<CmdType>PresetList</CmdType>\r\n";
						strTemp += "<Privilege>" + tPTZQuery.UserCode + "</Privilege>\r\n";
						//strTemp+="<CameraAddress>"+CameraAddress+"</CameraAddress>\r\n";						
						CString cst;
						cst.Format("%d", RealNum-RemainNum);
						strTemp += "<ReceivePresetNum>" + cst + "</ReceivePresetNum>\r\n";
					//	cst.Format("%d", endIndex + 1);
					//	strTemp += "<FromIndex>" + cst + "</FromIndex>\r\n";
					//	cst.Format("%d", endcur);
					//	strTemp += "<ToIndex>" + cst + "</ToIndex>\r\n";
				//		cst.Format("%d", endIndex + 1);
						strTemp += "</Query>\r\n";
						strTemp += "</Action>\r\n";
						char *xml = (LPSTR)(LPCTSTR)strTemp;
						char *buf = new char[MAXBUFSIZE];
						CSipMsgProcess *sipVideoQuery = new CSipMsgProcess;
						//sipVideoQuery->VideoSipXmlMsg(&buf,m_InfoServer,tPTZQuery.Address,m_InfoClient,xml);
						PreSetBitSipXmlMsg(&buf, m_InfoServer, m_InfoClient, tPTZQuery.Address, xml);
						UA_Msg uas_sendtemp;
						strcpy(uas_sendtemp.data, buf);
						EnterCriticalSection(&g_uas);
						uas_sendqueue.push(uas_sendtemp);
						LeaveCriticalSection(&g_uas);
						//pWnd->ShowSendData(buf);
						delete buf;
					}
				//	if (endIndex == endcur)pWnd->ShowRecvData("\t----Ԥ��λ��ѯ�ɹ�----\r\n");
					if (RemainNum==0)pWnd->ShowRecvData("\t----Ԥ��λ��ѯ�ɹ�----\r\n");
				}
				else
				{
					pWnd->ShowRecvData("\t----Ԥ��λ��ѯʧ��----\r\n");
				}
				//update log			
				ShowTestData += " <---------  200 OK\r\n";
			}
			else if (m_SipMsg.msg->status_code == 400)
			{
				//receive 400 ok message
				pWnd->ShowRecvData("\t----Ԥ��λ��ѯ����----\r\n");
				//update log				
				ShowTestData += " <---------  400\r\n";
			}
			else
			{
				//receive other message				
				return 1;
			}

		}
		break;
	//case HistoryQuery:
	//	{
	//		if (m_SipMsg.msg->status_code == 200)
	//		{
	//			string temp;
	//			string::size_type VideoNumStart;
	//			string::size_type VideoNumEnd;
	//			//result �ֶ�
	//			if ((VideoNumStart = strXML.find("<Result>", 0)) == string::npos)
	//				return 1;
	//			if ((VideoNumEnd = strXML.find("</Result>", VideoNumStart + 1)) == string::npos)
	//				return 1;
	//			temp = strXML.substr(VideoNumStart + 8, VideoNumEnd - VideoNumStart - 8);
	//			CString Result;
	//			Result.Format("%s", temp.c_str());
	//			if (strcmp(temp.c_str(), "") == 0)
	//			{
	//				AfxMessageBox("Result is null", MB_OK | MB_ICONEXCLAMATION);
	//				return 0;
	//			}
	//			temp.erase(0, temp.length());

	//			//RealFileNum
	//			if ((VideoNumStart = strXML.find("<RealFileNum>", 0)) == string::npos)
	//				return 1;
	//			if ((VideoNumEnd = strXML.find("</RealFileNum>", VideoNumStart + 1)) == string::npos)
	//				return 1;
	//			temp = strXML.substr(VideoNumStart + 13, VideoNumEnd - VideoNumStart - 13);
	//			if (strcmp(temp.c_str(), "") == 0)
	//			{
	//				AfxMessageBox("RealFileNum is null", MB_OK | MB_ICONEXCLAMATION);
	//				return 0;
	//			}
	//			CString realNum;
	//			realNum.Format("%s", temp.c_str());
	//			temp.erase(0, temp.length());
	//			int nRealNum = atoi(realNum);
	//			//SendFileNum
	//			if ((VideoNumStart = strXML.find("<SendFileNum>", 0)) == string::npos)
	//				return 1;
	//			if ((VideoNumEnd = strXML.find("</SendFileNum>", VideoNumStart + 1)) == string::npos)
	//				return 1;
	//			temp = strXML.substr(VideoNumStart + 13, VideoNumEnd - VideoNumStart - 13);
	//			if (strcmp(temp.c_str(), "") == 0)
	//			{
	//				AfxMessageBox("SendFileNum is null", MB_OK | MB_ICONEXCLAMATION);
	//				return 0;
	//			}
	//			CString sendNum;
	//			sendNum.Format("%s", temp.c_str());
	//			temp.erase(0, temp.length());
	//			int nSendNum = atoi(sendNum);

	//			if (nRealNum == 0 || nSendNum == 0)
	//			{
	//				pWnd->ShowRecvData("\t----������������ʷ��Ƶ��ѯ�����----\r\n");
	//				//update log				
	//				ShowTestData += "<----------- 200 OK\r\n";
	//				//update query information					
	//				pWnd->m_VideoQuery.m_HistoryVideoList.ResetContent();
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_BEGIN)->SetWindowText("null");
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_END)->SetWindowText("null");
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_FILESIZE)->SetWindowText("null");
	//				//pWnd->m_HistoryVideo.GetDlgItem(IDC_EDT_DECODERTAG)->SetWindowText("null");
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_REALNUM)->SetWindowText("null");
	//				//pWnd->m_VideoQuery.GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
	//				pWnd->m_VideoInfo.clear();
	//				return 0;
	//			}

	//		
	//			if (VideoQueryResponseAnylse(&pWnd->m_VideoInfo, buffer, nSendNum))
	//			{
	//				int nSend = pWnd->m_VideoInfo.size();

	//				pWnd->ShowRecvData("\t----��ʷ��Ƶ��ѯ�ɹ�----\r\n");
	//				//update query information
	//				for (int i = 0; i<nSend; i++)
	//				{
	//					pWnd->m_VideoQuery.m_HistoryVideoList.InsertString(i, pWnd->m_VideoInfo[i].Name);
	//				}
	//				i = 0;
	//				pWnd->m_VideoQuery.m_HistoryVideoList.SetCurSel(i);
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_BEGIN)->SetWindowText(pWnd->m_VideoInfo[i].BeginTime);
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_END)->SetWindowText(pWnd->m_VideoInfo[i].EndTime);
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_FILESIZE)->SetWindowText(pWnd->m_VideoInfo[i].FileSize);
	//				//pWnd->m_HistoryVideo.GetDlgItem(IDC_EDT_DECODERTAG)->SetWindowText(DecoderTag);
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_EDT_REALNUM)->SetWindowText(realNum);
	//				pWnd->m_VideoQuery.GetDlgItem(IDC_BTN_GETURL)->EnableWindow(TRUE);
	//				}
	//			else
	//			{
	//				pWnd->ShowRecvData("\t----��ʷ��Ƶ��ѯʧ��----\r\n");
	//			}
	//			//update log				
	//			ShowTestData += "<----------- 200 OK\r\n";
	//		}
	//		else if (m_SipMsg.msg->status_code == 400)
	//		{
	//			pWnd->ShowRecvData("\t-----��ʷ��Ƶ�ļ���ȡ����ʧ�ܣ�-----\r\n");
	//			//update log				
	//			ShowTestData += "<--------- 400\r\n";
	//		}
	//		else if (m_SipMsg.msg->status_code == 100)
	//		{
	//			pWnd->ShowRecvData("\t-----��ʷ��Ƶ�ļ���ȡtrying��-----\r\n");
	//			//update log				
	//			ShowTestData += "<--------- 100\r\n";
	//		}
	//		else
	//		{
	//			//receive other message
	//		}
	//	}
	//	break;
	case HistoryPlay:
		{
			if (m_SipMsg.msg->status_code == 200)
			{
				string temp;
				string::size_type VideoNumStart;
				string::size_type VideoNumEnd;
				if ((VideoNumStart = strXML.find("<Result>", 0)) == string::npos)
					return 1;
				if ((VideoNumEnd = strXML.find("</Result>", VideoNumStart + 1)) == string::npos)
					return 1;
				temp = strXML.substr(VideoNumStart + 8, VideoNumEnd - VideoNumStart - 8);
				CString Result;
				Result.Format("%s", temp.c_str());
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("Result is null", MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
				temp.erase(0, temp.length());
				int nResult = atoi(Result);
				if (nResult == 0)
				{
					pWnd->ShowRecvData("\t----��ʷ��ƵUrl��ȡ�ɹ�----\r\n");
					//get url's host and port
					if ((VideoNumStart = strXML.find("<Playurl>", 0)) == string::npos)
						return 1;
					if ((VideoNumEnd = strXML.find("</Playurl>", VideoNumStart + 1)) == string::npos)
						return 1;
					temp = strXML.substr(VideoNumStart + 9, VideoNumEnd - VideoNumStart - 9);
					char *playurl = new char[300];
					strcpy(playurl, temp.c_str());
					temp.erase(0, temp.length());
					if (strcmp(playurl, "") == 0)
					{
						AfxMessageBox("Play url is null", MB_OK | MB_ICONEXCLAMATION);
						return 0;
					}
					pWnd->rtspUrl.Format("%s", playurl);
					pWnd->m_VideoPlay.GetDlgItem(IDC_EDT_URL)->SetWindowText(pWnd->rtspUrl);
					int nlength = strlen(playurl);
					char *rtspIP = new char[30];
					char *rtspPort = new char[10];
					int nindex = 0;
					int i;
					int jIP = 0;
					int jPort = 0;
					// find the string "rtsp://"
					for (i = 0; i<nlength - 6; i++)
					{
						if (playurl[i] == 'r' || playurl[i] == 'R')
							if (playurl[i + 1] == 't' || playurl[i] == 'T')
								if (playurl[i + 2] == 's' || playurl[i] == 'S')
									if (playurl[i + 3] == 'p' || playurl[i] == 'P')
										if (playurl[i + 4] == ':')
											if (playurl[i + 5] == '/')
												if (playurl[i + 6] == '/')
												{
													nindex = i + 7;
													break;
												}
					}
					//find rtsp IP
					while (playurl[nindex] != '\0')
					{
						if (playurl[nindex] != ':')
						{
							if (playurl[nindex] == '/')
							{
								strcpy(rtspPort, "554");
								rtspIP[jIP] = '\0';
								break;
							}
							rtspIP[jIP] = playurl[nindex];
							jIP++;
							if (jIP>30)
							{
								AfxMessageBox("rtsp is error", MB_OK | MB_ICONERROR);
								delete rtspIP;
								delete rtspPort;
								delete playurl;
								return 0;
							}
							nindex++;
						}
						else
						{
							rtspIP[jIP] = '\0';
							nindex++;
							// find rtsp port;
							while (playurl[nindex] != '\0')
							{
								if (playurl[nindex] != '/')
								{
									rtspPort[jPort] = playurl[nindex];
									jPort++;
									if (jPort>10)
									{
										AfxMessageBox("rtsp url is error", MB_OK | MB_ICONERROR);
										delete rtspIP;
										delete rtspPort;
										delete playurl;
										return 0;
									}
									nindex++;
								}
								else
								{
									rtspPort[jPort] = '\0';
									break;
								}
							}
							break;
						}
					}
					pWnd->RTSPIP.Format("%s", rtspIP);
					pWnd->RTSPPort.Format("%s", rtspPort);
					pWnd->m_VideoPlay.GetDlgItem(IDC_EDT_IP)->SetWindowText(pWnd->RTSPIP);
					pWnd->m_VideoPlay.GetDlgItem(IDC_EDT_PORT)->SetWindowText(pWnd->RTSPPort);
					delete playurl;
					delete rtspIP;
					delete rtspPort;
				}
				else
				{
					pWnd->ShowRecvData("\t----��ʷ��ƵUrl��ȡʧ��----\r\n");
				}
				//update log				
				ShowTestData += "<--------- 200 OK\r\n";
			}
			else if (m_SipMsg.msg->status_code == 400)
			{
				pWnd->ShowRecvData("\t-----��ʷ��ƵUrl��ȡ����ʧ�ܣ�-----\r\n");
				//update log				
				ShowTestData += "<--------- 400\r\n";
			}
			else
			{
				//receive other message
				return 1;
			}
		}
		break;
	case DeviceInfQuery:
		{
			pWnd->m_DeviceInfQuery.GetDlgItem(IDC_EDIT_DEVICEINFO)->SetWindowTextA(buffer);
			pWnd->m_DeviceInfQuery.GetDlgItem(IDC_EDIT_DEVICEINFO)->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
		}
		break;
	case FlowQuery:
		{
			//����UAC�˷��ص�������ѯ��Ϣ
			ShowFlowQueryData(buffer);
		}
		break;
	case EncoderSet:
		{
			if (m_SipMsg.msg->status_code == 200)
			{
				//receive 200 ok message
				string temp;
				string::size_type EncoderStart;
				string::size_type EncoderEnd;
				if ((EncoderStart = strXML.find("<Result>", 0)) == string::npos)
					return 1;
				if ((EncoderEnd = strXML.find("</Result>", EncoderStart + 1)) == string::npos)
					return 1;
				temp = strXML.substr(EncoderStart + 8, EncoderEnd - EncoderStart - 8);
				CString Result;
				Result.Format("%s", temp.c_str());
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("Result is null", MB_OK | MB_ICONERROR);
					return 0;
				}
				temp.erase(0, temp.length());
				int nResult = atoi(Result);
				if (nResult == 0)
					pWnd->ShowRecvData("\t----���������óɹ�----\r\n");
				else
					pWnd->ShowRecvData("\t----��case Alarm��������ʧ��----\r\n");
				//update log
				pWnd->ShowRecvData("\t----���������Գɹ�----\r\n");
				ShowTestData += " <---------  200 OK \r\n";
			}
			else if (m_SipMsg.msg->status_code == 400)
			{
				//receive 400 ok message
				pWnd->ShowRecvData("\t----����������ʧ��----\r\n");
				//update log				
				ShowTestData += "<---------  400\r\n";
			}
			else
				//receive other message
				return 1;
		}
		break;
	case Alarm:
		{
			if (m_SipMsg.msg->status_code == 200)
			{
				//receive 200 ok message
				if (Common::FLAG_NowCancleReserving)//Ϊ���ʾ�ɹ�ȡ��Ԥ���¼�
				{
					pWnd->ShowRecvData("\t----ȡ������Ԥ���ɹ�----\r\n");
					ShowTestData += "<----------200  OK \r\n";
					Common::curAlreadyReserveEvent.erase(remove(Common::curAlreadyReserveEvent.begin(),
						Common::curAlreadyReserveEvent.end(), Common::nowOperatorEventMsg));
					Common::nowOperatorEventMsg = "";
					Common::FLAG_NowCancleReserving = false;
				}
				else//�����ʾ��������Ԥ���¼�
				{
					pWnd->ShowRecvData("\t----����Ԥ�����óɹ�----\r\n");
					ShowTestData += "<----------200  OK \r\n";
					Common::curAlreadyReserveEvent.push_back(Common::nowOperatorEventMsg);
					Common::nowOperatorEventMsg = "";
					pWnd->m_Alarm.GetDlgItem(IDC_BTN_ALARM_CANCEL)->EnableWindow(TRUE);//��ȡ������Ԥ����ť
				}	
			}
			else if (m_SipMsg.msg->status_code == 400)
			{
				//receive 400 ok message
				pWnd->ShowRecvData("\t----����Ԥ������ʧ��----\r\n");
				ShowTestData += " <---------  400\r\n";
			}
			else if (m_SipMsg.msg->status_code == 100)
			{
				ShowTestData += " <---------  100\r\n";
			}
			else if (m_SipMsg.msg->status_code == 101)
			{
				ShowTestData += " <---------  101\r\n";
			}
			else
			{
				//receive other message
				return 1;
			}
		}
		break;
	case TimeSet:
		{
			if (m_SipMsg.msg->status_code == 200)//receive 200 ok message
			{
				string temp;
				string::size_type timeSetStart;
				string::size_type timeSetEnd;
				if ((timeSetStart = strXML.find("<Result>", 0)) == string::npos)
					return 1;
				if ((timeSetEnd = strXML.find("</Result>", timeSetStart + 1)) == string::npos)
					return 1;

				temp = strXML.substr(timeSetStart + 8, timeSetEnd - timeSetStart - 8);
				CString Result;
				Result.Format("%s", temp.c_str());
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("Result is null", MB_OK | MB_ICONERROR);
					return 0;
				}
				temp.erase(0, temp.length());
				int nResult = atoi(Result);
				if (nResult == 0)
					pWnd->ShowRecvData("\t----ʱ������ɹ�----\r\n");
				else
					pWnd->ShowRecvData("\t----ʱ�����ʧ��----\r\n");
				ShowTestData += "<----------200  OK \r\n";
			}
			else if (m_SipMsg.msg->status_code == 400)
			{
				pWnd->ShowRecvData("\t----ʱ�����ʧ��----\r\n");
				ShowTestData += " <---------  400\r\n";
			}
			else
			{
				//receive other message
				return 1;
			}
		}
		break;
	case TimeGet:
		{
			//create time get xml
			time_t CurrentTime;
			CurrentTime = time(NULL);
			struct tm *pts;
			pts = localtime(&CurrentTime);
			CString strTime;
			strTime.Format("%d-%d-%dT%d:%d:%dZ", pts->tm_year + 1900, pts->tm_mon + 1, pts->tm_mday, pts->tm_hour, pts->tm_min, pts->tm_sec);
			CString strTemp;
			strTemp = "<?xml version=\"1.0\"?>\r\n";
			strTemp += "<Response>\r\n";
			strTemp += "<ControlResponse>\r\n";
			strTemp += "<CmdType>TimeGet</CmdType>\r\n";
			strTemp += "<Result>0</Result>\r\n";
			strTemp += "<Time>" + strTime + "</Time>\r\n";
			strTemp += "<Privilege>192016809088</Privilege>\r\n";
			strTemp += "</ControlResponse>\r\n";
			strTemp += "</Response>\r\n";
			char*xml = (LPSTR)(LPCTSTR)strTemp;
			char *dstMsg = new char[MAXBUFSIZE];
			Sip200Xml(&dstMsg, m_SipMsg.msg, xml);
			UA_Msg uas_sendtemp;
			strcpy(uas_sendtemp.data, dstMsg);
			EnterCriticalSection(&g_uas);
			uas_sendqueue.push(uas_sendtemp);
			LeaveCriticalSection(&g_uas);
			delete dstMsg;
			//update log
			ShowTestData += "200  OK ------->\r\n";
		}
		break;
	case CaptureImage:
		{
			if (m_SipMsg.msg->status_code == 200)
			{
				string::size_type ResultStart;
				string::size_type ResultEnd;
				if ((ResultStart = strXML.find("<Result>", 0)) == string::npos)
					return 1;
				if ((ResultEnd = strXML.find("</Result>", ResultStart + 1)) == string::npos)
					return 1;
				string temp = strXML.substr(ResultStart + 8, ResultEnd - ResultStart - 8);
				CString Result;
				Result.Format("%s", temp.c_str());
				if (strcmp(temp.c_str(), "") == 0)
				{
					AfxMessageBox("Result is null", MB_OK | MB_ICONERROR);
					return 0;
				}
				temp.erase(0, temp.length());
				int nResult = atoi(Result);
				if (nResult == 0)
					pWnd->ShowRecvData("\t----���ץͼ�ɹ�----\r\n");
				else
					pWnd->ShowRecvData("\t----���ץͼʧ��----\r\n");
				//update log
				pWnd->ShowRecvData("\t----�ɹ����Գɹ�----\r\n");
				ShowTestData += " <---------  200 OK \r\n";
				int URLStart = strXML.find("<URL>", 0);
				int URLEnd = strXML.find("</URL>", 0);
				string strURL = strXML.substr(URLStart + 5, URLEnd - URLStart - 5);
				CString CurrentURL = strURL.c_str();
				CString _URL;
				pWnd->m_CaptureImg.GetDlgItem(IDC_EDIT_URL)->GetWindowTextA(_URL);
				CString URL = CurrentURL +"\r\n"+ _URL;//����֮ǰ��ȡ����ͼƬURL
				pWnd->m_CaptureImg.GetDlgItem(IDC_EDIT_URL)->SetWindowTextA(URL);
				//��ȡ_URLЭ��
				int protocolEnd = strURL.find("://", 0);
				string protocol = strURL.substr(0, protocolEnd);
 				if (strcmp(protocol.c_str(), "http") == 0)
				{
					downloadImage(strURL, CCaptureImage::HTTP_FLAG);
				}
				else if (strcmp(protocol.c_str(), "ftp") == 0)
				{
					downloadImage(strURL, CCaptureImage::FTP_FLAG);
				}
				else
				{
					;
				}
			}
			else if (m_SipMsg.msg->status_code == 400)
			{
				//receive 400 ok message
				pWnd->ShowRecvData("\t----���ץͼ����ʧ��----\r\n");
				//update log				
				ShowTestData += "<---------  400\r\n";
			}
			else
				//receive other message
				return 1;
		}
		break;
	default:
		break;
	}
	return 0;
}

void CSipMsgProcess::Sip200OK(char **dst, osip_message_t *srcmsg)
{
	char FromTag[10];
	int RandData;
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	//����200 OK����
	char *dest = NULL;
	size_t len = 0;
	CSipMsgProcess *Sip200 = new CSipMsgProcess;
	osip_message_set_version(Sip200->m_SipMsg.msg, "SIP/2.0");
	osip_message_set_status_code(Sip200->m_SipMsg.msg, 200);
	osip_message_set_reason_phrase(Sip200->m_SipMsg.msg, "OK");
	osip_call_id_clone(srcmsg->call_id, &Sip200->m_SipMsg.msg->call_id);
	CTime time;
	time = CTime::GetCurrentTime();
	CString date = time.Format("%Y-%m-%dT%H:%M:%S");
	osip_message_set_date(Sip200->m_SipMsg.msg, date);
	char *str2;
	osip_from_to_str(srcmsg->from, &str2);
	osip_message_set_from(Sip200->m_SipMsg.msg, str2);
	if (srcmsg->to->gen_params.nb_elt == 0)
	{
		osip_to_clone(srcmsg->to, &Sip200->m_SipMsg.msg->to);
		osip_to_set_tag(Sip200->m_SipMsg.msg->to, FromTag);
	}
	else
	{
		char *str1;
		osip_to_to_str(srcmsg->to, &str1);
		osip_message_set_to(Sip200->m_SipMsg.msg, str1);
		osip_free(str1);
	}

	osip_cseq_clone(srcmsg->cseq, &Sip200->m_SipMsg.msg->cseq);

	osip_message_get_via(srcmsg, 0, &Sip200->m_SipMsg.via);
	osip_via_to_str(Sip200->m_SipMsg.via, &dest);
	osip_message_set_via(Sip200->m_SipMsg.msg, dest);
	int i = osip_message_get_via(srcmsg, 1, &Sip200->m_SipMsg.via);

	CString st0;
	if (1 == i)
	{
		osip_via_to_str(Sip200->m_SipMsg.via, &dest);
		st0 = dest;
		st0 = "Via: " + st0 + "\r\n";
	}
	else
	{
		st0 = "";
	}
	osip_free(dest);
	osip_message_to_str(Sip200->m_SipMsg.msg, &dest, &len);
	memset(*dst, 0, MAXBUFSIZE);
	string sipTemp = dest;
	int index = sipTemp.find("From");
	sipTemp.insert(index, st0.GetBuffer(st0.GetLength()));
	//memcpy(*dst,dest,len);
	strcpy(*dst, sipTemp.c_str());
	osip_free(dest);
	osip_free(str2);
}

void CSipMsgProcess::Sip200OKForRegOrQuit(char **dst, osip_message_t *srcmsg)
{
	char FromTag[10];
	int RandData;
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	//����200 OK����
	char *dest = NULL;
	size_t len = 0;
	CSipMsgProcess *Sip200 = new CSipMsgProcess;
	osip_message_set_version(Sip200->m_SipMsg.msg, "SIP/2.0");
	osip_message_set_status_code(Sip200->m_SipMsg.msg, 200);
	osip_message_set_reason_phrase(Sip200->m_SipMsg.msg, "OK");
	osip_call_id_clone(srcmsg->call_id, &Sip200->m_SipMsg.msg->call_id);
	CTime time;
	time = CTime::GetCurrentTime();
	CString date = time.Format("%Y-%m-%dT%H:%M:%S");
	osip_message_set_date(Sip200->m_SipMsg.msg, date);
	char *str2;
	osip_from_to_str(srcmsg->from, &str2);
	osip_message_set_from(Sip200->m_SipMsg.msg, str2);
	if (srcmsg->to->gen_params.nb_elt == 0)
	{
		osip_to_clone(srcmsg->to, &Sip200->m_SipMsg.msg->to);
		osip_to_set_tag(Sip200->m_SipMsg.msg->to, FromTag);
	}
	else
	{
		char *str1;
		osip_to_to_str(srcmsg->to, &str1);
		osip_message_set_to(Sip200->m_SipMsg.msg, str1);
		osip_free(str1);
	}

	osip_cseq_clone(srcmsg->cseq, &Sip200->m_SipMsg.msg->cseq);
	//copy contact
	osip_message_get_contact(srcmsg, 0, &Sip200->m_SipMsg.contact);
	osip_contact_to_str(Sip200->m_SipMsg.contact, &dest);
	osip_message_set_contact(Sip200->m_SipMsg.msg, dest);
	osip_free(dest);
	//copy Expires
	osip_header *expires = NULL;
	osip_message_get_expires(srcmsg, 0, &expires);
	osip_message_set_expires(Sip200->m_SipMsg.msg, expires->hvalue);
	//copy via
	osip_message_get_via(srcmsg, 0, &Sip200->m_SipMsg.via);
	osip_via_to_str(Sip200->m_SipMsg.via, &dest);
	osip_message_set_via(Sip200->m_SipMsg.msg, dest);
	int i = osip_message_get_via(srcmsg, 1, &Sip200->m_SipMsg.via);

	CString st0;
	if (1 == i)
	{
		osip_via_to_str(Sip200->m_SipMsg.via, &dest);
		st0 = dest;
		st0 = "Via: " + st0 + "\r\n";
	}
	else
	{
		st0 = "";
	}
	osip_free(dest);
	osip_message_to_str(Sip200->m_SipMsg.msg, &dest, &len);
	memset(*dst, 0, MAXBUFSIZE);
	string sipTemp = dest;
	int index = sipTemp.find("From");
	sipTemp.insert(index, st0.GetBuffer(st0.GetLength()));
	//memcpy(*dst,dest,len);
	strcpy(*dst, sipTemp.c_str());
	osip_free(dest);
	osip_free(str2);
}
void CSipMsgProcess::SipACK(char **dst, osip_message_t *srcmsg)
{
	char FromTag[10];
	int RandData;
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	//����ACK����
	char *dest = NULL;
	size_t len;
	CSipMsgProcess *Sip = new CSipMsgProcess;
	osip_message_set_method(Sip->m_SipMsg.msg, "ACK");
	osip_message_set_uri(Sip->m_SipMsg.msg, srcmsg->to->url);
	osip_message_set_version(Sip->m_SipMsg.msg, "SIP/2.0");
	osip_call_id_clone(srcmsg->call_id, &Sip->m_SipMsg.msg->call_id);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);	*/
	osip_from_set_url(Sip->m_SipMsg.from, srcmsg->from->url);
	osip_from_set_displayname(Sip->m_SipMsg.from, srcmsg->from->displayname);
	osip_from_set_tag(Sip->m_SipMsg.from, sInviteCallID.CurTag);
	osip_from_to_str(Sip->m_SipMsg.from, &ByeFrom);
	osip_message_set_from(Sip->m_SipMsg.msg, ByeFrom);
	osip_to_clone(srcmsg->to, &Sip->m_SipMsg.msg->to);
	if (srcmsg->to->gen_params.nb_elt == 0)
	{
		osip_to_set_tag(Sip->m_SipMsg.msg->to, FromTag);
	}
	osip_to_to_str(Sip->m_SipMsg.msg->to, &ByeTo);

	osip_cseq_set_method(Sip->m_SipMsg.cseq, "ACK");
	osip_cseq_set_number(Sip->m_SipMsg.cseq, "1");

	osip_message_set_contact(Sip->m_SipMsg.msg, contact);
	osip_message_get_via(srcmsg, 0, &Sip->m_SipMsg.via);
	osip_via_to_str(Sip->m_SipMsg.via, &ByeVia);
	osip_message_set_via(Sip->m_SipMsg.msg, ByeVia);
	osip_cseq_to_str(Sip->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(Sip->m_SipMsg.msg, dest);
	osip_free(dest);
	//osip_message_set_expires(Sip->m_SipMsg.msg,"30");
	osip_message_set_max_forwards(Sip->m_SipMsg.msg, "70");
	osip_message_to_str(Sip->m_SipMsg.msg, &dest, &len);
	memset(*dst, 0, MAXBUFSIZE);
	memcpy(*dst, dest, len);
	osip_free(dest);
}

void CSipMsgProcess::SipBYE(char **dst, osip_message_t *srcmsg)
{
	char *dest = NULL;
	size_t len;
	CSipMsgProcess *Sip = new CSipMsgProcess;
	osip_message_set_method(Sip->m_SipMsg.msg, "BYE");
	osip_message_set_uri(Sip->m_SipMsg.msg, srcmsg->to->url);
	osip_message_set_version(Sip->m_SipMsg.msg, "SIP/2.0");
	osip_call_id_clone(srcmsg->call_id, &Sip->m_SipMsg.msg->call_id);

	osip_from_set_tag(Sip->m_SipMsg.from, sInviteCallID.CurTag);
	osip_message_set_from(Sip->m_SipMsg.msg, ByeFrom);

	osip_message_set_to(Sip->m_SipMsg.msg, ByeTo);

	osip_cseq_set_method(Sip->m_SipMsg.cseq, "BYE");
	osip_cseq_set_number(Sip->m_SipMsg.cseq, "2");
	osip_message_set_contact(Sip->m_SipMsg.msg, contact);

	///////////////////////////////////////////////////
	/*
		modified by lee
	*/
	osip_message_get_via(srcmsg, 0, &Sip->m_SipMsg.via);
	osip_via_to_str(Sip->m_SipMsg.via, &dest);
	osip_message_set_via(Sip->m_SipMsg.msg, dest);
	osip_free(dest);
	///////////////////////////////////////////////////	
	osip_cseq_to_str(Sip->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(Sip->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_max_forwards(Sip->m_SipMsg.msg, "70");
	osip_message_to_str(Sip->m_SipMsg.msg, &dest, &len);

	memset(*dst, 0, MAXBUFSIZE);
	memcpy(*dst, dest, len);
	osip_free(dest);
}

void CSipMsgProcess::SipCancel(char **dst, osip_message_t *srcmsg)
{
	char *dest = NULL;
	size_t len;
	CSipMsgProcess *Sip = new CSipMsgProcess;
	//char *dstCode=(LPSTR)(LPCTSTR)strAddress;
	osip_message_set_method(Sip->m_SipMsg.msg, "CANCEL");
	osip_message_set_uri(Sip->m_SipMsg.msg, srcmsg->from->url);
	osip_message_set_version(Sip->m_SipMsg.msg, "SIP/2.0");
	osip_call_id_clone(srcmsg->call_id, &Sip->m_SipMsg.msg->call_id);
	
	osip_from_set_tag(Sip->m_SipMsg.from, sInviteCallID.CurTag);
	//osip_from_to_str(Sip->m_SipMsg.from,&dest);
	osip_message_set_from(Sip->m_SipMsg.msg, ByeFrom);

	osip_message_set_to(Sip->m_SipMsg.msg, ByeTo);

	osip_cseq_set_method(Sip->m_SipMsg.cseq, "CANCEL");
	osip_cseq_set_number(Sip->m_SipMsg.cseq, "2");
	osip_message_set_contact(Sip->m_SipMsg.msg, contact);
	/*osip_message_get_via(srcmsg,0,&Sip->m_SipMsg.via);
	osip_via_to_str(Sip->m_SipMsg.via,&dest);*/
	osip_message_set_via(Sip->m_SipMsg.msg, ByeVia);

	osip_cseq_to_str(Sip->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(Sip->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_max_forwards(Sip->m_SipMsg.msg, "70");
	osip_message_to_str(Sip->m_SipMsg.msg, &dest, &len);
	memset(*dst, 0, MAXBUFSIZE);
	memcpy(*dst, dest, len);
	osip_free(dest);
}

void CSipMsgProcess::Sip400(char **dst, osip_message_t *srcmsg)
{
	char FromTag[10];
	int RandData;
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	//����400����
	char *dest = NULL;
	size_t len;
	CSipMsgProcess *Sip200 = new CSipMsgProcess;
	osip_message_set_version(Sip200->m_SipMsg.msg, "SIP/2.0");
	osip_message_set_status_code(Sip200->m_SipMsg.msg, 400);
	osip_message_set_reason_phrase(Sip200->m_SipMsg.msg, "");
	osip_call_id_clone(srcmsg->call_id, &Sip200->m_SipMsg.msg->call_id);
	osip_from_clone(srcmsg->from, &Sip200->m_SipMsg.msg->from);
	osip_to_clone(srcmsg->to, &Sip200->m_SipMsg.msg->to);
	if (srcmsg->to->gen_params.nb_elt == 0)
	{
		osip_to_set_tag(Sip200->m_SipMsg.msg->to, FromTag);
	}
	osip_cseq_clone(srcmsg->cseq, &Sip200->m_SipMsg.msg->cseq);

	osip_message_set_contact(Sip200->m_SipMsg.msg, contact);
	//copy via
	osip_message_get_via(srcmsg, 0, &Sip200->m_SipMsg.via);
	osip_via_to_str(Sip200->m_SipMsg.via, &dest);
	osip_message_set_via(Sip200->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_to_str(Sip200->m_SipMsg.msg, &dest, &len);
	memset(*dst, 0, MAXBUFSIZE);
	memcpy(*dst, dest, len);
	osip_free(dest);
}

void CSipMsgProcess::Sip401(char **dst, osip_message_t *srcmsg)
{
	char FromTag[10];
	int RandData;
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	//����400����
	char *dest = NULL;
	size_t len;
	CSipMsgProcess *Sip200 = new CSipMsgProcess;
	osip_message_set_version(Sip200->m_SipMsg.msg, "SIP/2.0");
	osip_message_set_status_code(Sip200->m_SipMsg.msg, 401);
	osip_message_set_reason_phrase(Sip200->m_SipMsg.msg, "Unauthorized");
	osip_call_id_clone(srcmsg->call_id, &Sip200->m_SipMsg.msg->call_id);
	osip_from_clone(srcmsg->from, &Sip200->m_SipMsg.msg->from);
	osip_to_clone(srcmsg->to, &Sip200->m_SipMsg.msg->to);
	if (srcmsg->to->gen_params.nb_elt == 0)
	{
		osip_to_set_tag(Sip200->m_SipMsg.msg->to, FromTag);
	}
	osip_cseq_clone(srcmsg->cseq, &Sip200->m_SipMsg.msg->cseq);

	osip_message_set_contact(Sip200->m_SipMsg.msg, contact);

	osip_header * expires = NULL;
	osip_message_get_expires(m_SipMsg.msg, 0, &expires);
	osip_message_set_expires(Sip200->m_SipMsg.msg, expires->hvalue);
	//copy via
	int i = osip_message_get_via(srcmsg, 0, &Sip200->m_SipMsg.via);
	osip_via_to_str(Sip200->m_SipMsg.via, &dest);
	i = osip_message_get_via(srcmsg, 1, &Sip200->m_SipMsg.via);
	osip_message_set_via(Sip200->m_SipMsg.msg, dest);
	CString st0;
	if (1 == i)
	{
		osip_via_to_str(Sip200->m_SipMsg.via, &dest);
		//osip_message_set_via(Sip200->m_SipMsg.msg,dest);
		st0 = dest;
		st0 = "Via: " + st0 + "\r\n";
	}
	else
	{
		st0 = "";
	}
	osip_free(dest);
	osip_message_to_str(Sip200->m_SipMsg.msg, &dest, &len);

	int RandD;
	srand((unsigned int)time(0));
	RandD = rand();
	char strR[20];
	itoa(RandD, strR, 16);
	CString strrand = strR;
	CTime time;
	time = CTime::GetCurrentTime();
	CString csttime = time.Format("%M%m%H");
	CString csttime1 = time.Format("%S%y%d");
	int ict = atoi(csttime.GetBuffer(csttime.GetLength()));
	int ict1 = atoi(csttime1.GetBuffer(csttime.GetLength()));
	char chict[20];
	itoa(ict, strR, 16);
	strrand += strR;
	itoa(ict1, strR, 16);
	strrand += strR;
	g_authInfo.realm = "user";
	g_authInfo.nonce = strrand + csttime;
	string st = "WWW-Authenticate:Digest realm=\"user\",nonce=\"" + strrand + csttime + "\", opaque=\"34523\",algorithm=MD5,qop=\"auth\" \r\n";
	string strtemp = dest;
	int index = strtemp.find("Content-Length");
	strtemp.insert(index, st);
	index = strtemp.find("From");
	strtemp.insert(index, st0.GetBuffer(st0.GetLength()));
	memset(*dst, 0, MAXBUFSIZE);
	//memcpy(*dst,dest,len);
	strcpy(*dst, strtemp.c_str());
	osip_free(dest);
}
//void CSipMsgProcess::Sip600(char **dst,osip_message_t *srcmsg)
//{
//	char FromTag[10];
//	int RandData;
//	RandData=rand();	
//	char str[8];		
//	itoa(RandData,str,10);
//	strcpy(FromTag,str);
//	//����600����
//	char *dest=NULL;
//	size_t len;
//	CSipMsgProcess *Sip200=new CSipMsgProcess;
//	osip_message_set_version(Sip200->m_SipMsg.msg,"SIP/2.0");
//	osip_message_set_status_code(Sip200->m_SipMsg.msg,600);	
//	osip_message_set_reason_phrase(Sip200->m_SipMsg.msg,"");
//	osip_call_id_clone(srcmsg->call_id,&Sip200->m_SipMsg.msg->call_id);
//	osip_from_clone(srcmsg->from,&Sip200->m_SipMsg.msg->from);
//	osip_to_clone(srcmsg->to,&Sip200->m_SipMsg.msg->to);
//	if (srcmsg->to->gen_params.nb_elt==0 )
//	{
//		osip_to_set_tag(Sip200->m_SipMsg.msg->to,FromTag);
//	}	
//	osip_cseq_clone(srcmsg->cseq,&Sip200->m_SipMsg.msg->cseq);
//	osip_message_get_contact(srcmsg,0,&Sip200->m_SipMsg.contact);
//	osip_contact_to_str(Sip200->m_SipMsg.contact,&dest);
//	osip_message_set_contact(Sip200->m_SipMsg.msg,dest);
//	osip_free(dest);
//	osip_message_get_via(srcmsg,0,&Sip200->m_SipMsg.via);
//	osip_via_to_str(Sip200->m_SipMsg.via,&dest);
//	osip_message_set_via(Sip200->m_SipMsg.msg,dest);
//	osip_free(dest);	
//	osip_message_to_str(Sip200->m_SipMsg.msg,&dest,&len);
//	memset(*dst,0,MAXBUFSIZE);
//	memcpy(*dst,dest,len);
//	osip_free(dest);
//}
//��SIPͷ��XML�ĵ���Ϣ���������Ϣ
void CSipMsgProcess::Sip200Xml(char **dstBuf, osip_message_t *srcmsg, CString Xml)
{
	char FromTag[10];
	int RandData;
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	//����200 OK����
	char *dest = NULL;
	size_t len;
	CSipMsgProcess *Sip200 = new CSipMsgProcess;
	osip_message_set_version(Sip200->m_SipMsg.msg, "SIP/2.0");
	osip_message_set_status_code(Sip200->m_SipMsg.msg, 200);
	osip_message_set_reason_phrase(Sip200->m_SipMsg.msg, "OK");
	osip_call_id_clone(srcmsg->call_id, &Sip200->m_SipMsg.msg->call_id);
	osip_from_clone(srcmsg->from, &Sip200->m_SipMsg.msg->from);
	osip_to_clone(srcmsg->to, &Sip200->m_SipMsg.msg->to);
	if (srcmsg->to->gen_params.nb_elt == 0)
	{
		osip_to_set_tag(Sip200->m_SipMsg.msg->to, FromTag);
	}
	osip_cseq_clone(srcmsg->cseq, &Sip200->m_SipMsg.msg->cseq);
	////copy contact
	/*osip_message_get_contact(srcmsg,0,&Sip200->m_SipMsg.contact);
	osip_contact_to_str(Sip200->m_SipMsg.contact,&dest);*/
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	osip_message_set_contact(Sip200->m_SipMsg.msg, contact);
	//osip_free(dest);
	//copy via
	osip_message_get_via(srcmsg, 0, &Sip200->m_SipMsg.via);
	osip_via_to_str(Sip200->m_SipMsg.via, &dest);
	osip_message_set_via(Sip200->m_SipMsg.msg, dest);
	int i = osip_message_get_via(srcmsg, 1, &Sip200->m_SipMsg.via);
	//osip_message_set_via(Sip200->m_SipMsg.msg,dest);
	CString st0;
	if (1 == i)
	{
		osip_via_to_str(Sip200->m_SipMsg.via, &dest);
		//osip_message_set_via(Sip200->m_SipMsg.msg,dest);
		st0 = dest;
		st0 = "Via: " + st0 + "\r\n";
	}

	osip_free(dest);
	//add XML message in the sip message
	osip_message_set_content_type(Sip200->m_SipMsg.msg, "Application/MANSCDP+XML");
	// 	char *xml=(LPSTR)(LPCTSTR)Xml;
	osip_message_set_body(Sip200->m_SipMsg.msg, Xml.GetBuffer(), Xml.GetLength());
	osip_message_to_str(Sip200->m_SipMsg.msg, &dest, &len);
	memset(*dstBuf, 0, MAXBUFSIZE);
	string strtemp = dest;
	int index = strtemp.find("From");
	strtemp.insert(index, st0.GetBuffer(st0.GetLength()));
	//memcpy(*dst,dest,len);
	strcpy(*dstBuf, strtemp.c_str());
	//memcpy(*dstBuf,dest,len);
	osip_free(dest);
}

void CSipMsgProcess::SipInviteMsg(char **dstInvite, InfoServer m_InfoServer, InfoClient m_InfoClient, char *InviteXml, CString strAddress)
{
	char FromTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)strAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);
	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����
																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	memset(sInviteCallID.CurID, 0, NUMSIZE);
	memset(sInviteCallID.CurHost, 0, IPSIZE);
	memset(sInviteCallID.CurTag, 0, NUMSIZE);
	strcpy(sInviteCallID.CurHost, srcIP);
	strcpy(sInviteCallID.CurID, CallID);
	strcpy(sInviteCallID.CurTag, FromTag);
	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);
	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "INVITE");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");
	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "INVITE");
	
	//char branch[20] = "z9hG4bK";
	//for (int i = 0; i < 8; i++)
	//{
	//	RandData = rand() % 10;
	//	branch[i + 7] = RandData + '0';
	//}
	//osip_via_set_branch(SipHeader->m_SipMsg.via, branch);//�����
	//strcpy(Common::CurrentBranch, branch);
	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");

	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	//save invite call id
	strcpy(InviteCallID, dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);

	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, InviteXml, strlen(InviteXml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dstInvite, dest);
	osip_free(dest);
}

void CSipMsgProcess::SipPtzMsg(char **dstInvite, InfoServer m_InfoServer, CString address, InfoClient m_InfoClient, char *PtzXml)
{
	char FromTag[50];
	char ToTag[50];
	char CallID[50];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;//
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	strcpy(FromTag, sInviteCallID.CurTag);
	strcpy(ToTag, sInviteCallID.CurToTag);
	strcpy(CallID, sInviteCallID.CurID);
	if (strcmp(CallID, "") == 0)
	{
		int RandData;
		srand((unsigned int)time(0));
		RandData = rand();
		char str[8];
		itoa(RandData, str, 10);
		strcpy(FromTag, str);
		RandData = rand();
		itoa(RandData, str, 10);
		strcpy(CallID, str);
	}
	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����	
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	// 	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	//osip_via_set_branch(SipHeader->m_SipMsg.via,"z9hG4bK--22bd7321");//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);
	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����
																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	osip_to_set_tag(SipHeader->m_SipMsg.to, ToTag);
	osip_to_set_url(SipHeader->m_SipMsg.to, m_SipMsg.uriServer);
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	strcpy(ptztag, FromTag);
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	//osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "DO");
	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "Message");
	pWnd->nPtz++;
	char str1[20];
	itoa(pWnd->nPtz, str1, 10);

	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, str1);

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	//osip_message_set_method(SipHeader->m_SipMsg.msg, "DO");
	osip_message_set_method(SipHeader->m_SipMsg.msg, "Message");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");

	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	//save invite call id
	//strcpy(InviteCallID,dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, PtzXml, strlen(PtzXml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dstInvite, dest);
	osip_free(dest);
}

void CSipMsgProcess::SipEncoderSetMsg(char **dstInvite, InfoServer m_InfoServer, CString address, InfoClient m_InfoClient, char *EncoderSetXml)
{
	char FromTag[50];
	char CallID[50];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;//
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//RandData=rand();	
	//char sdtr[8];	
	//char branch[20];
	//itoa(RandData,sdtr,16);
	//strcpy(branch,"z9hG4bK-");
	//strcat(branch,sdtr);

	char branch[20] = "z9hG4bK";
	for (int i = 0; i < 8; i++)
	{
		RandData = rand() % 10;
		branch[i + 7] = RandData + '0';
	}
	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);
	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����
																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	//strcpy(ptztag,FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "DO");
	/*pWnd->nPtz++;
	char str1[20];
	itoa(pWnd->nPtz,str1,10);*/

	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "DO");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");

	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	//save invite call id
	//strcpy(InviteCallID,dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, EncoderSetXml, strlen(EncoderSetXml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dstInvite, dest);
	osip_free(dest);
}

void CSipMsgProcess::SipSubscribeMsg(char **dst, InfoServer m_InfoServer, InfoClient m_InfoClient, char *Xml)
{
	char FromTag[50];
	char CallID[50];
	//Զ��������Ϣ
	char *dstCode = (LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//RandData=rand();	
	//char sdtr[8];	
	//char branch[20];
	//itoa(RandData,sdtr,16);
	//strcpy(branch,"z9hG4bK-");
	//strcat(branch,sdtr);
	char branch[20] = "z9hG4bK";
	for (int i = 0; i < 8; i++)
	{
		RandData = rand() % 10;
		branch[i + 7] = RandData + '0';
	}
	osip_via_set_branch(SipHeader->m_SipMsg.via, branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	
	//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);

	//�洢�¼�Ԥ����Call_ID�ֶΣ��Ե�UAC����Ԥ��֪ͨ����Call_ID�ıȽ�
	memset(SAlarmCallID.CurID, 0, NUMSIZE);
	memset(SAlarmCallID.CurHost, 0, IPSIZE);
	memset(SAlarmCallID.CurTag, 0, NUMSIZE);
	strcpy(SAlarmCallID.CurHost, srcIP);
	strcpy(SAlarmCallID.CurID, CallID);
	strcpy(SAlarmCallID.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "SUBSCRIBE");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "SUBSCRIBE");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	//osip_message_set_max_forwards(SipHeader->m_SipMsg.msg,"70");

	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_expires(SipHeader->m_SipMsg.msg, "90");
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "90");
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);

	string st = "Event:presence\r\n";
	string strtemp = dest;
	int index = strtemp.find("Content-Type");
	strtemp.insert(index, st);

	strcpy(*dst, strtemp.c_str());
	osip_free(dest);
}

void CSipMsgProcess::SipSubscribeMsgCancel(char **dst, InfoServer m_InfoServer, InfoClient m_InfoClient, char *Xml)
{
	char FromTag[50];
	char CallID[50];
	char ToTag[50];
	//Զ��������Ϣ
	char *dstCode = (LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	strcpy(FromTag, SAlarmCallID.CurTag);
	strcpy(CallID, SAlarmCallID.CurID);
	strcpy(ToTag, SAlarmCallID.CurToTag);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	int RandData;
	char branch[20] = "z9hG4bK";
	for (int i = 0; i < 8; i++)
	{
		RandData = rand() % 10;
		branch[i + 7] = RandData + '0';
	}
	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����CAllID���������������Ҫ֮ǰ�Ѿ�Ԥ���˵�

	//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	osip_to_set_tag(SipHeader->m_SipMsg.to, ToTag);
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);
	//HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	//CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	memset(SAlarmCallID.CurID, 0, NUMSIZE);
	memset(SAlarmCallID.CurHost, 0, IPSIZE);
	memset(SAlarmCallID.CurTag, 0, NUMSIZE);
	strcpy(SAlarmCallID.CurHost, srcIP);
	strcpy(SAlarmCallID.CurID, CallID);
	strcpy(SAlarmCallID.CurTag, FromTag);
	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "SUBSCRIBE");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "SUBSCRIBE");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	//osip_message_set_max_forwards(SipHeader->m_SipMsg.msg,"70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_expires(SipHeader->m_SipMsg.msg, "0");
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "90");
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);

	string st = "Event:presence\r\n";
	string strtemp = dest;
	int index = strtemp.find("Content-Type");
	strtemp.insert(index, st);

	strcpy(*dst, strtemp.c_str());
	osip_free(dest);
}

void CSipMsgProcess::SipAlarmEventNotifyMsg(char ** dst, InfoServer m_InfoServer, InfoClient m_InfoClient, char * Xml)
{
	char FromTag[50];
	char CallID[50];
	char ToTag[50];
	//Զ��������Ϣ
	char *dstCode = (LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	//int RandData;
	//char branch[20] = "z9hG4bK";
	//for (int i = 0; i < 8; i++)
	//{
	//	RandData = rand() % 10;
	//	branch[i + 7] = RandData + '0';
	//}
	//osip_via_set_branch(SipHeader->m_SipMsg.via, branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(ToTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����CAllID���������������Ҫ֮ǰ�Ѿ�Ԥ���˵�

																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	osip_to_set_tag(SipHeader->m_SipMsg.to, ToTag);
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);
	//HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	//CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "Message");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "Message");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg,"70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_expires(SipHeader->m_SipMsg.msg, "0");
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	//osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "90");
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);

	string strtemp = dest;
	//string st = "Event:presence\r\n";
	//int index = strtemp.find("Content-Type");
	//strtemp.insert(index, st);

	strcpy(*dst, strtemp.c_str());
	osip_free(dest);
}

void CSipMsgProcess::CatalogQuerySipXmlMsg(char **dst, InfoServer m_InfoServer, CString address, InfoClient m_InfoClient, char *Xml)
{
	char FromTag[10];
	char ToTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;

	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(ToTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str); 

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);	
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	//char branch[20] = "z9hG4bK";
	//for (int i = 0; i < 8; i++)
	//{
	//	RandData = rand() % 10;
	//	branch[i + 7] = RandData + '0';
	//}
	//osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);
	osip_to_set_tag(SipHeader->m_SipMsg.to, ToTag);
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "Message");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "Message");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);


	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	//TRACE(dest);
	osip_free(dest);
}

void CSipMsgProcess::DeviceStatusQueryXmlMsg(char ** dst, InfoServer m_InfoServer, CString address, InfoClient m_InfoClient, char * Xml)
{
	char FromTag[10];
	char ToTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;

	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(ToTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);	
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	//char branch[20] = "z9hG4bK";
	//for (int i = 0; i < 8; i++)
	//{
	//	RandData = rand() % 10;
	//	branch[i + 7] = RandData + '0';
	//}
	//osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);
	osip_to_set_tag(SipHeader->m_SipMsg.to, ToTag);
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "Message");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "Message");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);


	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	//TRACE(dest);
	osip_free(dest);
}

void CSipMsgProcess::DeviceInfQuerySipXmlMsg(char **dst, InfoServer m_InfoServer, CString address, InfoClient m_InfoClient, char *Xml)
{
	char FromTag[10];
	char ToTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;

	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(ToTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);

	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

	//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_to_set_tag(SipHeader->m_SipMsg.to, ToTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "Message");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "Message");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	//osip_contact_set_displayname(SipHeader->m_SipMsg.contact,srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	//TRACE(dest);
	osip_free(dest);
}

void CSipMsgProcess::FlowQuerySipXmlMsg(char **dst, InfoServer m_InfoServer, CString address, InfoClient m_InfoClient, char *Xml)
{
	char FromTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;

	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);	
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);	
	char branch[20] = "z9hG4bK";
	for (int i = 0; i < 8; i++)
	{
		RandData = rand() % 10;
		branch[i + 7] = RandData + '0';
	}
	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "DO");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "DO");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	//osip_contact_set_displayname(SipHeader->m_SipMsg.contact,srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	// 	osip_contact_to_str(SipHeader->m_SipMsg.contact,&dest);
	// 	osip_message_set_contact(SipHeader->m_SipMsg.msg,dest);
	// 	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	//TRACE(dest);
	osip_free(dest);
}

void CSipMsgProcess::VideoSipXmlMsg(char **dst, InfoServer m_InfoServer, CString address, InfoClient m_InfoClient, char *Xml)
{
	char FromTag[10];
	char ToTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;

	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);

	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(ToTag, str);

	RandData = rand(); 
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[50];
	// 	itoa(RandData,sdtr,16);	
	// 	strcpy(branch,"z9hG4bK-524287-1---");
	// 	strcat(branch,sdtr);
	// 	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_to_set_tag(SipHeader->m_SipMsg.to, ToTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "Message");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "Message");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	//osip_contact_set_displayname(SipHeader->m_SipMsg.contact,srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	osip_free(dest);
}

void CSipMsgProcess::PreSetBitSipXmlMsg(char **dst, InfoServer m_InfoServer, InfoClient m_InfoClient, CString address, char *Xml)
{
	char FromTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);	
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	// 	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "DO");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "DO");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	//osip_contact_set_displayname(SipHeader->m_SipMsg.contact,srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	osip_free(dest);
}

void CSipMsgProcess::SipTimeSetXmlMsg(char **dst, InfoServer m_InfoServer, InfoClient m_InfoClient, char *Xml)
{
	char FromTag[10];
	char CallID[10];
	//Զ��������Ϣ
	char *dstCode = (LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);	
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	char branch[20] = "z9hG4bK";
	for (int i = 0; i < 8; i++)
	{
		RandData = rand() % 10;
		branch[i + 7] = RandData + '0';
	}
	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "DO");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "DO");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	//osip_contact_set_displayname(SipHeader->m_SipMsg.contact,srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	osip_free(dest);
}

void CSipMsgProcess::SipCaptureImageMsg(char **dst, InfoServer m_InfoServer, InfoClient m_InfoClient, char *cameraAddress, char *Xml)
{
	char FromTag[10];
	char CallID[10];
	//Զ��������Ϣ
	char *dstCode = (LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, cameraAddress);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);	
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	char branch[20] = "z9hG4bK";
	for (int i = 0; i < 8; i++)
	{
		RandData = rand() % 10;
		branch[i + 7] = RandData + '0';
	}
	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "DO");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "DO");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	//osip_contact_set_displayname(SipHeader->m_SipMsg.contact,srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");


	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/DDCP");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	osip_free(dest);
}

void CSipMsgProcess::SipCancelMsg(char **dstCancel, InfoServer m_InfoServer, InfoClient m_InfoClient, CString address)
{
	char FromTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	// 	int RandData;
	// 	srand((unsigned int)time(0));
	// 	RandData=rand();	
	// 	char str[8];		
	// 	itoa(RandData,str,10);
	// 	strcpy(FromTag,str);
	// 	RandData=rand();
	// 	itoa(RandData,str,10);
	// 	strcpy(CallID,str);
	strcpy(FromTag, sInviteCallID.CurTag);
	strcpy(CallID, sInviteCallID.CurID);
	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����	
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	//osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	//osip_via_set_branch(SipHeader->m_SipMsg.via,"z9hG4bK--22bd7321");//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);
	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����
																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);*/
	memset(sInviteCallID.CurID, 0, NUMSIZE);
	memset(sInviteCallID.CurHost, 0, IPSIZE);
	memset(sInviteCallID.CurTag, 0, NUMSIZE);
	strcpy(sInviteCallID.CurHost, srcIP);
	strcpy(sInviteCallID.CurID, CallID);
	strcpy(sInviteCallID.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);
	//osip_from_set_tag();
	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "CANCEL");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "1");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "CANCEL");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");

	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	//save invite call id
	strcpy(InviteCallID, dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	//osip_message_set_body(SipHeader->m_SipMsg.msg,CancelXml,strlen(CancelXml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dstCancel, dest);
	osip_free(dest);
}

BOOL CSipMsgProcess::NotifyResponseAnylse(InfoNotify& NotifyInfo, char *buf)
{
	string strTemp(buf);
	string temp;
	string::size_type VideoStart = 0;
	string::size_type CmdTypeStart;
	string::size_type CmdTypeEnd;
	InfoVideo Video;
	int varCount = 0;
	int nVideo = 0;
	CmdTypeStart = strTemp.find("<Parent>", VideoStart + 1);
	CmdTypeEnd = strTemp.find("</Parent>", CmdTypeStart + 1);
	if (CmdTypeStart != string::npos && CmdTypeEnd != string::npos)
		NotifyInfo.Parent = strTemp.substr(CmdTypeStart + 8, CmdTypeEnd - CmdTypeStart - 8).c_str();

	while ((VideoStart = strTemp.find("<Item>", VideoStart)) != string::npos)
	{
		InfoDvice infoDviceT;
		//Name�ֶ�
		if ((CmdTypeStart = strTemp.find("<Name>", VideoStart + 1)) == string::npos)
			break;
		if ((CmdTypeEnd = strTemp.find("</Name>", CmdTypeStart + 1)) == string::npos)
			break;
		infoDviceT.Name = strTemp.substr(CmdTypeStart + 6, CmdTypeEnd - CmdTypeStart - 6).c_str();

		//Address�ֶ�
		if ((CmdTypeStart = strTemp.find("<Address>", VideoStart + 1)) == string::npos)
			break;
		if ((CmdTypeEnd = strTemp.find("</Address>", CmdTypeStart + 1)) == string::npos)
			break;
		infoDviceT.Address = strTemp.substr(CmdTypeStart + 9, CmdTypeEnd - CmdTypeStart - 9).c_str();

		NotifyInfo.Devices.push_back(infoDviceT);
		VideoStart = CmdTypeEnd + 1;
	}
	return TRUE;
}

//��ʷ��Ƶ��ѯ��Ӧ��Ϣ����
BOOL CSipMsgProcess::VideoQueryResponseAnylse(vector <InfoVideo> *VideoInfo, char *buf, int nSendNum)
{
	string strTemp(buf);
	string temp;
	string::size_type VideoStart = 0;
	string::size_type CmdTypeStart;
	string::size_type CmdTypeEnd;
	InfoVideo Video;
	int varCount = 0;
	int nVideo = 0;
	while ((VideoStart = strTemp.find("<Item>", VideoStart)) != string::npos)
	{
		if ((CmdTypeStart = strTemp.find("<Name>", VideoStart + 1)) == string::npos)
			break;
		if ((CmdTypeEnd = strTemp.find("</Name>", CmdTypeStart + 1)) == string::npos)
			break;
		temp = strTemp.substr(CmdTypeStart + 6, CmdTypeEnd - CmdTypeStart - 6);
		strcpy(Video.Name, temp.c_str());
		varCount++;
		temp.erase(0, temp.length());

		if ((CmdTypeStart = strTemp.find("<CreationTime>", VideoStart + 1)) == string::npos)
			break;
		if ((CmdTypeEnd = strTemp.find("</CreationTime>", CmdTypeStart + 1)) == string::npos)
			break;
		temp = strTemp.substr(CmdTypeStart + 14, CmdTypeEnd - CmdTypeStart - 14);
		strcpy(Video.BeginTime, temp.c_str());
		varCount++;
		temp.erase(0, temp.length());

		if ((CmdTypeStart = strTemp.find("<LastWriteTime>", VideoStart + 1)) == string::npos)
			break;
		if ((CmdTypeEnd = strTemp.find("</LastWriteTime>", CmdTypeStart + 1)) == string::npos)
			break;
		temp = strTemp.substr(CmdTypeStart + 15, CmdTypeEnd - CmdTypeStart - 15);

		strcpy(Video.EndTime, temp.c_str());
		varCount++;
		temp.erase(0, temp.length());

		if ((CmdTypeStart = strTemp.find("<FileSize>", VideoStart + 1)) == string::npos)
			break;
		if ((CmdTypeEnd = strTemp.find("</FileSize>", CmdTypeStart + 1)) == string::npos)
			break;
		temp = strTemp.substr(CmdTypeStart + 10, CmdTypeEnd - CmdTypeStart - 10);
		strcpy(Video.FileSize, temp.c_str());
		varCount++;
		temp.erase(0, temp.length());
		VideoStart = CmdTypeEnd + 1;
		if (varCount == 4)
		{
			varCount = 0;
			(*VideoInfo).push_back(Video);
			nVideo++;
		}
	}
	if (nSendNum != nVideo)
	{
		return FALSE;
	}
	return TRUE;
}

void CSipMsgProcess::GetUrlDOSipXmlMsg(char **dst, InfoServer m_InfoServer, InfoClient m_InfoClient, char *Xml)
{
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	char FromTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)pWnd->videoAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����
	// 	RandData=rand();	
	// 	char sdtr[10];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);
	// 	strcpy(branch,"z9hG4bK--");
	// 	strcat(branch,sdtr);
	// 	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	//osip_via_set_branch(SipHeader->m_SipMsg.via,"z9hG4bK--22bd7122");//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);

	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����	

																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	memset(SCallId.CurID, 0, NUMSIZE);
	memset(SCallId.CurHost, 0, IPSIZE);
	memset(SCallId.CurTag, 0, NUMSIZE);
	strcpy(SCallId.CurHost, srcIP);
	strcpy(SCallId.CurID, CallID);
	strcpy(SCallId.CurTag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "DO");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, "2");

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "DO");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");

	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	osip_free(dest);
}

BOOL CSipMsgProcess::SipVerify(InfoServer m_InfoServer, InfoClient m_InfoClient, osip_message_t *srcMsg, int nto)
{
	//Զ��������Ϣ
	char *dstCode = (LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;
	if (srcMsg->from->url->username == NULL)
	{
		srcMsg->from->url->username = "";
	}
	if (srcMsg->from->displayname == NULL)
	{
		srcMsg->from->displayname = "";
	}
	if (srcMsg->from->url->host == NULL)
	{
		srcMsg->from->url->host = "";
	}
	if (srcMsg->from->url->port == NULL)
	{
		srcMsg->from->url->port = "";
	}
	if (srcMsg->to->url->username == NULL)
	{
		srcMsg->to->url->username = "";
	}
	if (srcMsg->to->displayname == NULL)
	{
		srcMsg->to->displayname = "";
	}
	if (srcMsg->to->url->host == NULL)
	{
		srcMsg->to->url->host = "";
	}
	if (srcMsg->to->url->port == NULL)
	{
		srcMsg->to->url->port = "";
	}
	if (nto == 1)
	{
		if (strcmp(srcCode, srcMsg->from->url->username) == 0 &&
			//strcmp(srcUserName,srcMsg->from->displayname)==0 &&
			strcmp(srcIP, srcMsg->from->url->host) == 0 &&
			strcmp(srcPort, srcMsg->from->url->port) == 0 &&
			strcmp(dstCode, srcMsg->to->url->username) == 0 &&
			//strcmp(dstUserName,srcMsg->to->displayname)==0 &&
			strcmp(dstIP, srcMsg->to->url->host) == 0 &&
			strcmp(dstPort, srcMsg->to->url->port) == 0)
		{
			return TRUE;
		}
	}
	else if (nto == 0)
	{
		if (strcmp(dstCode, srcMsg->from->url->username) == 0 &&
			//strcmp(dstUserName,srcMsg->from->displayname)==0 &&
			strcmp(dstIP, srcMsg->from->url->host) == 0 &&
			strcmp(dstPort, srcMsg->from->url->port) == 0 &&
			strcmp(srcCode, srcMsg->to->url->username) == 0 &&
			//strcmp(srcUserName,srcMsg->to->displayname)==0 &&
			strcmp(srcIP, srcMsg->to->url->host) == 0 &&
			strcmp(srcPort, srcMsg->to->url->port) == 0)
		{
			return TRUE;
		}
	}
	else
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CSipMsgProcess::InviteSipVerify(InfoServer m_InfoServer, InfoClient m_InfoClient, CString address, osip_message_t *srcMsg, int nto)
{
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;	
	char *dstCode = (LPSTR)(LPCTSTR)address;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;
	if (srcMsg->from->url->username == NULL)
	{
		srcMsg->from->url->username = "";
	}
	if (srcMsg->from->displayname == NULL)
	{
		srcMsg->from->displayname = "";
	}
	if (srcMsg->from->url->host == NULL)
	{
		srcMsg->from->url->host = "";
	}
	if (srcMsg->from->url->port == NULL)
	{
		srcMsg->from->url->port = "";
	}
	if (srcMsg->to->url->username == NULL)
	{
		srcMsg->to->url->username = "";
	}
	if (srcMsg->to->displayname == NULL)
	{
		srcMsg->to->displayname = "";
	}
	if (srcMsg->to->url->host == NULL)
	{
		srcMsg->to->url->host = "";
	}
	if (srcMsg->to->url->port == NULL)
	{
		srcMsg->to->url->port = "";
	}
	if (nto == 1)
	{
		if (strcmp(srcCode, srcMsg->from->url->username) == 0 &&
			//strcmp(srcUserName,srcMsg->from->displayname)==0 &&
			strcmp(srcIP, srcMsg->from->url->host) == 0 &&
			strcmp(srcPort, srcMsg->from->url->port) == 0 &&
			strcmp(dstCode, srcMsg->to->url->username) == 0 &&
			//strcmp(dstUserName,srcMsg->to->displayname)==0 &&
			strcmp(dstIP, srcMsg->to->url->host) == 0 &&
			strcmp(dstPort, srcMsg->to->url->port) == 0)
		{
			return TRUE;
		}
	}
	else if (nto == 0)
	{
		if (strcmp(dstCode, srcMsg->from->url->username) == 0 &&
			//strcmp(dstUserName,srcMsg->from->displayname)==0 &&
			strcmp(dstIP, srcMsg->from->url->host) == 0 &&
			strcmp(dstPort, srcMsg->from->url->port) == 0 &&
			strcmp(srcCode, srcMsg->to->url->username) == 0 &&
			//strcmp(srcUserName,srcMsg->to->displayname)==0 &&
			strcmp(srcIP, srcMsg->to->url->host) == 0 &&
			strcmp(srcPort, srcMsg->to->url->port) == 0)
		{
			return TRUE;
		}
	}
	else
	{
		return TRUE;
	}
	return FALSE;
}
//real time keep live

void CSipMsgProcess::DORealTimeKeepLiveMsg(char **dst, InfoServer m_InfoServer, InfoClient m_InfoClient, CString address, char *Xml)
{
	char FromTag[10];
	char CallID[10];
	//Զ��������Ϣ
	//char *dstCode=(LPSTR)(LPCTSTR)m_InfoClient.UserAddress;
	char *dstCode = (LPSTR)(LPCTSTR)address;
	char *dstUserName = (LPSTR)(LPCTSTR)m_InfoClient.UserName;
	char *dstIP = (LPSTR)(LPCTSTR)m_InfoClient.IP;
	char *dstPort = (LPSTR)(LPCTSTR)m_InfoClient.Port;
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;

	int RandData;
	srand((unsigned int)time(0));
	RandData = rand();
	char str[8];
	itoa(RandData, str, 10);
	strcpy(FromTag, str);
	RandData = rand();
	itoa(RandData, str, 10);
	strcpy(CallID, str);

	char *dest;
	CSipMsgProcess *SipHeader = new CSipMsgProcess;
	////////////////////////
	osip_uri_set_host(SipHeader->m_SipMsg.uriServer, dstIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriServer, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriServer, dstCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriServer, dstPort);

	osip_uri_set_host(SipHeader->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(SipHeader->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(SipHeader->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(SipHeader->m_SipMsg.uriClient, srcPort);

	osip_via_set_version(SipHeader->m_SipMsg.via, "2.0");
	osip_via_set_protocol(SipHeader->m_SipMsg.via, "UDP");
	osip_via_set_port(SipHeader->m_SipMsg.via, srcPort);
	//osip_via_set_branch(via,"123456789");//�����
	// 	RandData=rand();	
	// 	char sdtr[8];	
	// 	char branch[20];
	// 	itoa(RandData,sdtr,16);
	// 	strcpy(branch,"z9hG4bK-");
	// 	strcat(branch,sdtr);
	// 	osip_via_set_branch(SipHeader->m_SipMsg.via,branch);//�����
	//osip_via_set_branch(SipHeader->m_SipMsg.via,"z9hG4bK--22bd7332");//�����
	osip_via_set_host(SipHeader->m_SipMsg.via, srcIP);
	//osip_call_id_set_host(SipHeader->m_SipMsg.callid, srcIP);
	osip_call_id_set_number(SipHeader->m_SipMsg.callid, CallID);//�����
																//osip_from_set_displayname(SipHeader->m_SipMsg.from,srcUserName);
	osip_from_set_tag(SipHeader->m_SipMsg.from, FromTag);//�����
	osip_from_set_url(SipHeader->m_SipMsg.from, SipHeader->m_SipMsg.uriClient);
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	memset(InviteKeepAliveID.Num, 0, NUMSIZE);
	memset(InviteKeepAliveID.Host, 0, IPSIZE);
	memset(InviteKeepAliveID.Tag, 0, NUMSIZE);
	strcpy(InviteKeepAliveID.Host, srcIP);
	strcpy(InviteKeepAliveID.Num, CallID);
	strcpy(InviteKeepAliveID.Tag, FromTag);

	//osip_to_set_displayname(SipHeader->m_SipMsg.to,dstUserName);	
	osip_to_set_url(SipHeader->m_SipMsg.to, SipHeader->m_SipMsg.uriServer);

	pWnd->nRealtime++;
	char str1[20];
	itoa(pWnd->nRealtime, str1, 10);
	osip_cseq_set_method(SipHeader->m_SipMsg.cseq, "DO");
	osip_cseq_set_number(SipHeader->m_SipMsg.cseq, str1);

	osip_message_set_uri(SipHeader->m_SipMsg.msg, SipHeader->m_SipMsg.uriServer);
	osip_message_set_method(SipHeader->m_SipMsg.msg, "DO");

	osip_contact_set_url(SipHeader->m_SipMsg.contact, SipHeader->m_SipMsg.uriClient);
	osip_contact_set_displayname(SipHeader->m_SipMsg.contact, srcUserName);
	osip_message_set_max_forwards(SipHeader->m_SipMsg.msg, "70");
	osip_message_set_expires(SipHeader->m_SipMsg.msg, "10");

	osip_to_to_str(SipHeader->m_SipMsg.to, &dest);
	osip_message_set_to(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_call_id_to_str(SipHeader->m_SipMsg.callid, &dest);
	//
	osip_message_set_call_id(SipHeader->m_SipMsg.msg, InviteCallID);
	//osip_message_set_call_id(SipHeader->m_SipMsg.msg,dest);	
	osip_free(dest);

	osip_from_to_str(SipHeader->m_SipMsg.from, &dest);
	osip_message_set_from(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_contact_to_str(SipHeader->m_SipMsg.contact, &dest);
	osip_message_set_contact(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_cseq_to_str(SipHeader->m_SipMsg.cseq, &dest);
	osip_message_set_cseq(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);

	osip_via_to_str(SipHeader->m_SipMsg.via, &dest);
	osip_message_set_via(SipHeader->m_SipMsg.msg, dest);
	osip_free(dest);
	osip_message_set_content_type(SipHeader->m_SipMsg.msg, "Application/MANSCDP+XML");
	osip_message_set_body(SipHeader->m_SipMsg.msg, Xml, strlen(Xml));
	size_t length;
	osip_message_to_str(SipHeader->m_SipMsg.msg, &dest, &length);
	strcpy(*dst, dest);
	osip_free(dest);
}

void CSipMsgProcess::CopyContact(char **dst, InfoServer m_InfoServer)
{
	//����������Ϣ
	char *srcCode = (LPSTR)(LPCTSTR)m_InfoServer.UserAddress;
	char *srcUserName = (LPSTR)(LPCTSTR)m_InfoServer.UserName;
	char *srcIP = (LPSTR)(LPCTSTR)m_InfoServer.IP;
	char *srcPort = (LPSTR)(LPCTSTR)m_InfoServer.Port;
	char *dest = NULL;
	CSipMsgProcess *Sip = new CSipMsgProcess;
	osip_uri_set_host(Sip->m_SipMsg.uriClient, srcIP);
	osip_uri_set_scheme(Sip->m_SipMsg.uriClient, "sip");
	osip_uri_set_username(Sip->m_SipMsg.uriClient, srcCode);
	osip_uri_set_port(Sip->m_SipMsg.uriClient, srcPort);
	osip_contact_set_url(Sip->m_SipMsg.contact, Sip->m_SipMsg.uriClient);
	//osip_contact_set_displayname(Sip->m_SipMsg.contact,srcUserName);
	osip_contact_to_str(Sip->m_SipMsg.contact, &dest);
	osip_message_set_contact(Sip->m_SipMsg.msg, dest);
	strcpy(*dst, dest);
	osip_free(dest);
}

void CSipMsgProcess::ShowFlowQueryData(char *buffer)
{
	string str = buffer;
	int msgBodyStart = str.find("<?xml version=\"1.0\"?>", 0);
	int msgBodyEnd = str.find("</Response>", 0);
	string strEnd = "</Response>\r\n";

	CString msgBody = str.substr(msgBodyStart, msgBodyEnd - msgBodyStart + strEnd.length()).c_str();
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	pWnd->m_FlowQuery.m_FlowQueryInfo.SetWindowTextA(msgBody);
	pWnd->m_FlowQuery.GetDlgItem(IDC_EDIT_FLOWQUERY)->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

void CSipMsgProcess::downloadImage(string url, int protocol_flag)
{
	//��ȡ��ǰ����·��
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos);

	//��ȡip��ַ���ļ���
	int ipStart = url.find("/", 0) + 2;
	int ipEnd = url.substr(ipStart).find("/") + ipStart;
	string URL = url.substr(ipStart, ipEnd - ipStart);

	int nameStart = url.find_last_of('/') + 1;
	CString fileName = url.substr(nameStart).c_str();

	if (protocol_flag == CCaptureImage::HTTP_FLAG)
	{
		CInternetSession session;
		session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000 * 20);
		session.SetOption(INTERNET_OPTION_CONNECT_BACKOFF, 1000);
		session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);

		CHttpConnection* pConnection = session.GetHttpConnection(URL.c_str(), (INTERNET_PORT)8080);
		CHttpFile* pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,
			fileName);

		CString szHeaders = "Accept: audio/x-aiff, audio/basic, audio/midi,\
							audio/mpeg, audio/wav, image/jpeg, image/gif, image/jpg, image/png,\
							image/mng, image/bmp, text/plain, textml, textm\r\n";

		pFile->AddRequestHeaders(szHeaders);
		pFile->SendRequest();

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);

		if (dwRet != HTTP_STATUS_OK)
		{
			CString errText;
			errText.Format("POST���������룺%d", dwRet);
			AfxMessageBox(errText);
		}
		else
		{ 
			int len = pFile->GetLength();
			char buf[2000];
			int numread;
			CString filepath;
			//CString strFile = "response.jpg";
			CString strFile = pFile->GetObject();

			filepath.Format("%s\\%s", path, strFile);
			CFile myfile(filepath,
				CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
			while ((numread = pFile->Read(buf, sizeof(buf) - 1)) > 0)
			{
				buf[numread] = '\0';
				strFile += buf;
				myfile.Write(buf, numread);
			}
			myfile.Close();
		}
		session.Close();
		pFile->Close();
		delete pFile;
	}
	else if (protocol_flag == CCaptureImage::FTP_FLAG)
	{
		CInternetSession* pInternetSession = NULL;
		CFtpConnection* pFtpConnection = NULL;
		//��������  
		pInternetSession = new CInternetSession(AfxGetAppName());
		//��������ip��ַ  
		//��Ҫ����Ϊ�������ĸ�Ŀ¼����ʹ��"\\"�Ϳ�����  
		//������һ��CFtpConnection����֮��Ϳ���ͨ�������������ϴ��ļ��������ļ���  
		pFtpConnection = pInternetSession->GetFtpConnection(URL.c_str(), TEXT("anonymous"), TEXT(""));

		//���÷�������Ŀ¼  
		//��ȡ�ļ��ڷ�������Ŀ¼

		int fileStartIndex = ipEnd + 1;// url.substr(ipStart).find("/") + ipStart;
		int fileEndIndex = nameStart - 1; // url.find_last_of('/') + 1;

		string fileIndex = url.substr(fileStartIndex, fileEndIndex - fileStartIndex);

		CString index;
		CString temp = fileIndex.c_str();
		index.Format("\\%s", temp);
		bool bRetVal = pFtpConnection->SetCurrentDirectory(index);
		if (bRetVal == false)
		{
			//	AfxMessageBox("Ŀ¼����ʧ��");
			return;
		}
		else
		{
			//�洢λ��
			index.Format("%s\\%s", path, fileName);
			pFtpConnection->GetFile(fileName, index);
		}
		//�ͷ�Դ  
		if (NULL != pFtpConnection)
		{
			pFtpConnection->Close();
			delete pFtpConnection;
			pFtpConnection = NULL;
		}
		if (NULL != pInternetSession)
		{
			delete pInternetSession;
			pInternetSession = NULL;
		}
	}

}
