#include "stdafx.h"
#include "AssimpLoader.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

BaseLoader* CreateLoader(const string& extenstion)
{
	if (extenstion == "dae" || extenstion == "obj" || extenstion == "3ds")
		return new AssimpLoader();
	return nullptr;
}


void Convert()
{
	string inputPath;
	cout << "Input: ";
	getline(cin, inputPath);

	int dotPos = inputPath.rfind('.') + 1;
	string extension = inputPath.substr(dotPos, inputPath.length() - dotPos);
		
	BaseLoader* pLoader = CreateLoader(extension);
	if(pLoader == nullptr)
		throw string("Extension not supported");

	pLoader->Load(inputPath);

	stringstream outputPath;
	string file;
	cout << "Output: ";
	getline(cin, file);
	outputPath << file;
	if (outputPath.str().find(".flux") == string::npos)
		outputPath << ".flux";

	pLoader->Write(outputPath.str());
	delete pLoader;
}

void TryConvert()
{
	try
	{
		Convert();
	}
	catch (string msg)
	{
		cout << msg << endl;
		TryConvert();
	}
}

void main()
{
	TryConvert();	
	_CrtDumpMemoryLeaks();
	system("pause");
}
