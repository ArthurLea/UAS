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
	static char *CurrentBranch;

	static vector<string> curAlreadyReserveEvent;//��ǰ�Ѿ�Ԥ���˵��¼���Ϣ�洢�������ж��Ƿ��ظ�Ԥ��
	static string nowReservingEventMsg;//������Ԥ���ı����¼�
};

