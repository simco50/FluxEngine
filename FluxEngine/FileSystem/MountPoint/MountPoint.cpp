#include "stdafx.h"
#include "MountPoint.h"
#include "FileSystem/FileSystemHelpers.h"

IMountPoint::IMountPoint(const std::string& physicalPath, const int order /*= -1*/) :
	m_PhysicalPath(ToLower(physicalPath)),
	m_Order(order)
{

}