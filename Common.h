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

	static vector<string> curAlreadyReserveEvent;//当前已经预定了的事件消息存储，用于判断是否重复预定
	static string nowReservingEventMsg;//正在想预定的报警事件
};

