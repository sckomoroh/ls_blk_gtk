#ifndef _PARTITIONS_PROVIDER_H_
#define _PARTITIONS_PROVIDER_H_

#include <list>
#include <string>

typedef struct _partitionInfo
{
	std::string mountPoint;
	std::string mappedTo;
	std::string fileSystemType;
	std::string deviceName;
	std::string fullDeviceName;
	_partitionInfo* ptrSlave;
	std::list<_partitionInfo*> holders;
} PartitionInfo, *PtrPartitionInfo;

class PartitionsProvider
{
private:
	std::list<PtrPartitionInfo> m_partitions;
	
public:
	void init();
	const std::list<PtrPartitionInfo>& partitions();

	virtual ~PartitionsProvider();
	
private:
	void _enumerateDisksAndPartitions();
	void _enumerateDevices();
	void _enumeratePartitions();

	void _enumerateHolders(const char* folderName, PtrPartitionInfo parent);
	
	std::list<PtrPartitionInfo> _findByDeviceName(std::list<PtrPartitionInfo> partitions, std::string deviceName);
	std::list<PtrPartitionInfo> _findByFullDeviceName(std::list<PtrPartitionInfo> partitions, std::string fullDeviceName);

	void _clean();
};

#endif // _PARTITIONS_PROVIDER_H_

