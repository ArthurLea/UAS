#pragma once
#define BRANCHNUM 20
#include <queue>
#include <vector>
#include <string>
using namespace std;
class Common
{
public:
	Common();
	~Common();

public:
	static const char * EXPIRES_VALUE;

	static char *CurrentBranch;

	static vector<string> curAlreadyReserveEvent;//��ǰ�Ѿ�Ԥ���˵��¼���Ϣ�洢�������ж��Ƿ��ظ�Ԥ��
	static string nowOperatorEventMsg;//������Ԥ���ı����¼�

	static BOOL FLAG_NowEventReserve;//��ʾ�Ƿ����ڽ����¼�Ԥ��
	static BOOL FLAG_NowCancleReserving;//����ȡ��Ԥ���¼��ı�־
};

