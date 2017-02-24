#include "stdafx.h"
#include "Common.h"
Common::Common()
{
}


Common::~Common()
{
}
const char * Common::EXPIRES_VALUE = "60";
char *Common::CurrentBranch = new char[BRANCHNUM];
vector<string> Common::curAlreadyReserveEvent(20);
string Common::nowOperatorEventMsg = "";
BOOL Common::FLAG_NowCancleReserving = false;
BOOL Common::FLAG_NowEventReserve = false;

