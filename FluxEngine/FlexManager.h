#pragma once
class FlexSystem;

class FlexManager
{
public:
	FlexManager();
	~FlexManager();

	void Initialize();
	void Shutdown();
	void Update();

	FlexSystem* CreateSystem();

private:

	NvFlexLibrary* m_pFlexLibrary = nullptr;
	vector<FlexSystem*> m_pSystems;
};

