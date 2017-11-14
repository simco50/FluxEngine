#include "stdafx.h"
#include "MountPoint.h"
#include "FileSystem/FileSystemHelpers.h"

IMountPoint::IMountPoint(const std::string& physicalPath) :
	m_PhysicalPath(ToLower(physicalPath))
{

}
