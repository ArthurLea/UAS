#pragma once
#include "afxsock.h"

#define WM_SEND				WM_USER + 104
#define WM_RECEIVE			WM_USER + 105
#define WM_SOCKETCLOSE		WM_USER + 106
#define WM_CONNCET			WM_USER + 107

class CMySocket :
	public CSocket
{
public:
	CMySocket(void);
	~CMySocket(void);

public:
	void SendMsg(const char *Msg);

	void Initialize(CWnd *_pWnd);

protected:
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnConnect(int nErrorCode);

private:
	CWnd *m_pWnd;
};
