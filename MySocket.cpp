#include "StdAfx.h"
#include "MySocket.h"
#include "DXP.h"

CMySocket::CMySocket(void)
{
}

CMySocket::~CMySocket(void)
{
}


void CMySocket::Initialize(CWnd *_pWnd)
{
	m_pWnd = _pWnd;
}

void CMySocket::OnReceive(int nErrorCode)
{
	if(nErrorCode==0)
	{
		m_pWnd->SendMessage(WM_RECEIVE);
	}

	CAsyncSocket::OnReceive(nErrorCode);
}

void CMySocket::OnClose(int nErrorCode)
{
	if(nErrorCode==0)
	{
		m_pWnd->SendMessage(WM_SOCKETCLOSE);
	}

	CAsyncSocket::OnClose(nErrorCode);
}

void CMySocket::OnConnect(int nErrorCode)
{
	if(nErrorCode==0)
	{
		m_pWnd->SendMessage(WM_CONNCET);
	}

	CAsyncSocket::OnConnect(nErrorCode);
}

void CMySocket::SendMsg(const char *Msg)
{
	int nlength=strlen(Msg);
	Send(Msg, nlength);
}