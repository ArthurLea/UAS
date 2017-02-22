#include "stdafx.h"
#include "Common.h"
Common::Common()
{
}


Common::~Common()
{
}
char *Common::CurrentBranch = new char[BRANCHNUM];
vector<string> Common::curAlreadyReserveEvent(20);
string Common::nowReservingEventMsg = "";
