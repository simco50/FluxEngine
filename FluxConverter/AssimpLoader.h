#pragma once
#include "BaseLoader.h"

class AssimpLoader : public BaseLoader
{
public:
	AssimpLoader();
	~AssimpLoader();

	void Load(const string& filePath) override;
};

