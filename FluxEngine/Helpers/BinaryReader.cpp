#include "stdafx.h"
#include "BinaryReader.h"


BinaryReader::BinaryReader()
{
}

BinaryReader::~BinaryReader()
{
	Close();
	m_pStream.reset();
}

void BinaryReader::Open(const string& filePath)
{
	m_pStream = make_unique<ifstream>();
	m_pStream->open(filePath, ios::binary);
	if(m_pStream->fail())
	{
		FLUX_LOG(ERROR, "[BinaryReader::Open()] > File %s not found!", filePath.c_str());
		m_pStream.reset();
		return;
	}
}

void BinaryReader::Close()
{
	IsOpen();
	m_pStream->close();
}

unsigned int BinaryReader::GetPosition() const
{
	IsOpen();
	return static_cast<unsigned int>(m_pStream->tellg());
}

void BinaryReader::SetPosition(const unsigned position)
{
	IsOpen();
	m_pStream->seekg(position);
}

void BinaryReader::MovePosition(const unsigned places)
{
	IsOpen();
	m_pStream->seekg(GetPosition() + places);
}

bool BinaryReader::IsOpen() const
{
	if(!(m_pStream != nullptr && m_pStream->is_open()))
	{
		return false;
	}
	return true;
}

string BinaryReader::ReadString()
{
	int stringLength = (char)Read<char>();
	stringstream ss;
	for (int i = 0; i < stringLength; ++i)
	{
		ss << Read<char>();
	}
	return ss.str();
}

string BinaryReader::ReadLongString()
{
	if (m_pStream == nullptr)
		FLUX_LOG(ERROR, "[BinaryReader::ReadLongString()] > BinaryReader doesn't exist!\nUnable to read binary data...");

	auto stringLength = Read<UINT>();

	stringstream ss;
	for (UINT i = 0; i<stringLength; ++i)
	{
		ss << Read<wchar_t>();
	}

	return ss.str();
}

string BinaryReader::ReadNullString()
{
	if (m_pStream == nullptr)
	{
		FLUX_LOG(ERROR, "[BinaryReader::ReadNullString()] > BinaryReader doesn't exist!\nUnable to read binary data...");
		return "";
	}

	string buff;
	getline(*m_pStream, buff, '\0');

	return buff;
}
