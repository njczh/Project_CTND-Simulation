#include "pch.h"
#include "wsn.h"


wsn::wsn()
{
}


wsn::~wsn()
{
}

int wsn::SetRandomAddr()
{
	//srand((unsigned)time(NULL));
	return rand();
}
