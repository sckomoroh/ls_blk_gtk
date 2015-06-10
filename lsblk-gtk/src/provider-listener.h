#ifndef _IPROVIDER_LISTENER_
#define _IPROVIDER_LISTENER_

#include "partition-provider.h"

class IProviderListener
{
public:
	virtual void dataCompleted(std::list<PtrPartitionInfo> data) = 0;
};

#endif // _IPROVIDER_LISTENER_