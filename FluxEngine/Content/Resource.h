#pragma once

class Resource
{
public:
	Resource();
	~Resource();

	virtual bool Load(const std::string& filePath) = 0;
};