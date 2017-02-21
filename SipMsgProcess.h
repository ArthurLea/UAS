#pragma once

#include "DXP.h"
#include <string.h>
#include <osipparser2/headers/osip_via.h>
#include <osipparser2/headers/osip_call_id.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <osipparser2/osip_uri.h>
#include <osipparser2/osip_port.h>
#include <osipparser2/osip_list.h>
#include <osipparser2/osip_headers.h>
#include <osipparser2/osip_const.h>
#include "UAS.h"
#include "UASDlg.h"

class CSipMsgProcess
{
public:
	CSipMsgProcess(void);
	~CSipMsgProcess(void);

public:
	struct SipMsg 
	{
		osip_message_t *msg;
		osip_via_t *via;
		osip_from_t *from;
		osip_to_t *to;
		osip_cseq_t *cseq;
		osip_contact_t *contact;
		osip_call_id_t *callid;
		osip_content_type *content_type;
		osip_content_length *content_length;
		osip_uri_t *uriServer;
		osip_uri_t *uriClient;
	} m_SipMsg;
	//事件类型
	int m_Type;
	int beginIndex;
	int endIndex;
	int RealNum;
public:
	int SipParser(char *buffer,int Msglength);
	BOOL SipVerify(InfoServer m_InfoServer,InfoClient m_InfoClient,osip_message_t *srcMsg,int nto);
	BOOL InviteSipVerify(InfoServer m_InfoServer,InfoClient m_InfoClient,CString address,osip_message_t *srcMsg,int nto);
	//clone from and to
	void Sip200Xml(char **dstBuf,osip_message_t *srcmsg,CString Xml);
	void Sip200OK(char **dst,osip_message_t *srcmsg);
	void Sip400(char **dst,osip_message_t *srcmsg);	
	void Sip401(char **dst,osip_message_t *srcmsg);	
	//void Sip600(char **dst,osip_message_t *srcmsg);
	//alarm
	int SipRegisterCreate(char **strRegister,InfoServer m_InfoServer,InfoClient m_InfoClient);
	int SipRegisterWithAuthCreate(char **strRegister,InfoServer m_InfoServer,InfoClient m_InfoClient);
	void SipSubscribeMsg(char **dst,InfoServer m_InfoServer,InfoClient m_InfoClient,char *Xml);
	void SipSubscribeMsgCancel(char **dst,InfoServer m_InfoServer,InfoClient m_InfoClient,char *Xml);
	// sip do method
	void SipXmlMsg(char **dst,InfoServer m_InfoServer,InfoClient m_InfoClient,char *Xml);
	void SipCancelMsg(char **dstCancel,InfoServer m_InfoServer,InfoClient m_InfoClient,CString address);
	void PreSetBitSipXmlMsg(char **dst,InfoServer m_InfoServer,InfoClient m_InfoClient,CString address,char *Xml);
	void CatalogQuerySipXmlMsg(char **dst,InfoServer m_InfoServer,CString address,InfoClient m_InfoClient,char *Xml);
	void DeviceInfQuerySipXmlMsg(char **dst,InfoServer m_InfoServer,CString address,InfoClient m_InfoClient,char *Xml);
	void FlowQuerySipXmlMsg(char **dst,InfoServer m_InfoServer,CString address,InfoClient m_InfoClient,char *Xml);
	void VideoSipXmlMsg(char **dst,InfoServer m_InfoServer,CString address,InfoClient m_InfoClient,char *Xml);
	//analyse video query information
	BOOL VideoQueryResponseAnylse(vector <InfoVideo> *VideoInfo,char *buf ,int nSendNum);
	BOOL NotifyResponseAnylse(InfoNotify& NotifyInfo,char *buf);
	// get url history video do method
	void GetUrlDOSipXmlMsg(char **dst,InfoServer m_InfoServer,InfoClient m_InfoClient,char *Xml);
	// invite message
	void SipInviteMsg(char **dstInvite,InfoServer m_InfoServer,InfoClient m_InfoClient,char *InviteXml,CString strAddress);
	// ptz message
	void SipPtzMsg(char **dstInvite,InfoServer m_InfoServer,CString address,InfoClient m_InfoClient,char *PtzXml);
	void SipEncoderSetMsg(char **dstInvite,InfoServer m_InfoServer,CString address,InfoClient m_InfoClient,char *EncoderSetXml);
	void DORealTimeKeepLiveMsg(char **dst,InfoServer m_InfoServer,InfoClient m_InfoClient,CString address,char *Xml);
	void SipACK(char **dst,osip_message_t *srcmsg);
	void SipBYE(char **dst,osip_message_t *srcmsg);
	void SipCancel(char **dst,osip_message_t *srcmsg);
	void CopyContact(char **dst,InfoServer m_InfoServer);
};
