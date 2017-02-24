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

	static vector<string> curAlreadyReserveEvent;//当前已经预定了的事件消息存储，用于判断是否重复预定
	static string nowOperatorEventMsg;//正在想预定的报警事件

	static BOOL FLAG_NowEventReserve;//表示是否正在进行事件预定
	static BOOL FLAG_NowCancleReserving;//正在取消预警事件的标志
};

