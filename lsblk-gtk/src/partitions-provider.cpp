#include "partitions-provider.h"

#include <stdio.h>
#include <mntent.h>
#include <libudev.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

using namespace std;

int sd_filter (const struct dirent * entry)
{
	if (strlen(entry->d_name) < 3)
	{
		return 0;
	}

	if (entry->d_name[0] != 's' || entry->d_name[1] != 'd')
	{
		return 0;
	}

	return 1;
}

PartitionsProvider::~PartitionsProvider()
{
	_clean();
}

void PartitionsProvider::init ()
{
	_clean();

	_enumerateDisksAndPartitions ();
	_enumerateDevices ();
	_enumeratePartitions ();
}

const std::list<PtrPartitionInfo>& PartitionsProvider::partitions()
{
	return m_partitions;
}

void PartitionsProvider::_enumerateDisksAndPartitions()
{
	struct dirent **fileListTemp;
	int num = scandir("/sys/block/", &fileListTemp, sd_filter, alphasort);

	for (int i=0; i<num; i++)
	{
		PtrPartitionInfo ptrInfo = new PartitionInfo;
		ptrInfo->deviceName = fileListTemp[i]->d_name;

		m_partitions.push_back(ptrInfo);

		char sdFolder[256] = { 0 };
		sprintf(sdFolder, "/sys/block/%s", fileListTemp[i]->d_name);

		_enumerateHolders(sdFolder, ptrInfo);
		
		struct dirent **fileListTemp2;
		printf("Scanning folder: '%s'\n", sdFolder);
		int num2 = scandir(sdFolder, &fileListTemp2, sd_filter, alphasort);
		for (int j=0; j<num2; j++)
		{
			PtrPartitionInfo ptrInfo2 = new PartitionInfo;
			ptrInfo2->deviceName = fileListTemp2[j]->d_name;
			ptrInfo2->ptrSlave = ptrInfo;
			ptrInfo->holders.push_back(ptrInfo2);
			
			char holdersFolder[256] = { 0 };
			sprintf(holdersFolder, "%s/%s", sdFolder, fileListTemp2[j]->d_name);

			_enumerateHolders(holdersFolder, ptrInfo2);
		}
	}
}

void PartitionsProvider::_enumerateHolders (const char* folderName, PtrPartitionInfo parent)
{
	char holdersFolder[256] = { 0 };
	sprintf(holdersFolder, "%s/holders", folderName);

	struct dirent **fileListTemp3;
	printf("Scanning holder folder: '%s' for '%s'\n", holdersFolder, folderName);
	int num3 = scandir(holdersFolder, &fileListTemp3, NULL, alphasort);
	for (int k=2; k<num3; k++)
	{
		PtrPartitionInfo ptrInfo3 = new PartitionInfo;
		ptrInfo3->deviceName = fileListTemp3[k]->d_name;
		ptrInfo3->ptrSlave = parent;

		parent->holders.push_back(ptrInfo3);

		char holderFolder[256] = { 0 };
		sprintf(holderFolder, "/sys/block/%s", fileListTemp3[k]->d_name);
		_enumerateHolders (holderFolder, ptrInfo3);
	}
}

void PartitionsProvider::_enumerateDevices()
{
	struct udev* udev;
	struct udev_enumerate* enumerate;
	struct udev_list_entry *devices;
	struct udev_list_entry *deviceEntry;
	struct udev_device* device;
	
	udev = udev_new();
	if (udev == NULL)
	{
		printf("Unable to create udev\n");
		return;
	}

	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "block");
	udev_enumerate_scan_devices(enumerate);

	devices = udev_enumerate_get_list_entry(enumerate);
	udev_list_entry_foreach(deviceEntry, devices)
	{
		string path = udev_list_entry_get_name(deviceEntry);
		device = udev_device_new_from_syspath(udev, path.c_str());

		string deviceNodePath = udev_device_get_devnode(device);
		const char* szFsType = udev_device_get_property_value(device, "ID_FS_TYPE");
		if (szFsType != NULL)
		{
			string fsType = szFsType;
			string sysNode = udev_device_get_sysname(device);
			
			list<PtrPartitionInfo> partitions = _findByDeviceName (m_partitions, sysNode);
			if (partitions.size() > 0)
			{
				const char* szDeviceMap = udev_device_get_property_value(device, "DM_NAME");
				
				list<PtrPartitionInfo>::iterator iter = partitions.begin();
				for (; iter != partitions.end(); iter++)
				{
					PtrPartitionInfo data = *iter;
					data->fullDeviceName = deviceNodePath;
					data->fileSystemType = fsType;

					if (szDeviceMap != NULL)
					{
						data->mappedTo = szDeviceMap;
					}
				}
			}
		}
		
		udev_device_unref(device);
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);
}

void PartitionsProvider::_enumeratePartitions()
{
	struct mntent* ptrMtab;
	FILE* stream = setmntent ("/etc/mtab", "r");

	if (stream == NULL) 
	{
		printf ("Unable to setmntent\n");
		return;
	}

	while ((ptrMtab = getmntent(stream)) != NULL) 
	{
		list<PtrPartitionInfo> partitionInfos = _findByFullDeviceName (m_partitions, ptrMtab->mnt_fsname);
		if (partitionInfos.size() > 0)
		{
			list<PtrPartitionInfo>::iterator iter = partitionInfos.begin();
			for (; iter != partitionInfos.end(); iter++)
			{
				PtrPartitionInfo data = *iter;
				data->mountPoint = ptrMtab->mnt_dir;
			}
		}
	}

	endmntent (stream);
}

std::list<PtrPartitionInfo> PartitionsProvider::_findByDeviceName(std::list<PtrPartitionInfo> partitions, std::string deviceName)
{
	list<PtrPartitionInfo> results;
	list<PtrPartitionInfo>::iterator iter = partitions.begin();
	for (; iter != partitions.end(); iter++)
	{
		PtrPartitionInfo data = *iter;
		if (data->holders.size() > 0)
		{
			list<PtrPartitionInfo> holders = _findByDeviceName (data->holders, deviceName);
			if (holders.size() > 0)
			{
				results.insert(results.end(), holders.begin(), holders.end());
			}
		}

		if (data->deviceName == deviceName || data->mappedTo == deviceName)
		{
			results.push_back(data);
		}
	}

	return results;
}

std::list<PtrPartitionInfo> PartitionsProvider::_findByFullDeviceName(std::list<PtrPartitionInfo> partitions, std::string fullDeviceName)
{
	list<PtrPartitionInfo> results;
	list<PtrPartitionInfo>::iterator iter = partitions.begin();
	for (; iter != partitions.end(); iter++)
	{
		PtrPartitionInfo data = *iter;
		if (data->holders.size() > 0)
		{
			list<PtrPartitionInfo> holders = _findByFullDeviceName (data->holders, fullDeviceName);
			if (holders.size() > 0)
			{
				results.insert(results.end(), holders.begin(), holders.end());
			}
		}

		if (data->fullDeviceName == fullDeviceName || data->mappedTo == fullDeviceName)
		{
			results.push_back(data);
		}
	}

	return results;
}

void PartitionsProvider::_clean()
{
	list<PtrPartitionInfo>::iterator iter = m_partitions.begin();
	for (; iter != m_partitions.end(); iter++)
	{
		PtrPartitionInfo data = *iter;
		delete data;
	}

	m_partitions.clear();
}
