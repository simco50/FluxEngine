#pragma once

class RenderTarget;
struct GameSettings;
class ImgUIDrawer;
class PhysicsCore;

class FluxCore
{
public:
	FluxCore();

	virtual ~FluxCore();

	int Run(HINSTANCE hInstance);
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	EngineContext& GetEngineContext() { return m_EngineContext; }

protected:
	EngineContext m_EngineContext;

	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HRESULT RegisterWindowClass();
	HRESULT MakeWindow();
	void InitializeHighDefinitionMouse();
	void OnPause(const bool paused);
	void CalculateFrameStats() const;

	void GameLoop();
	virtual void PrepareGame() = 0;
	virtual void Initialize(EngineContext* pEngineContext) = 0;
	virtual void Update() = 0;

	//Window variables
	HINSTANCE m_hInstance = nullptr;

	std::string m_WindowClassName = "WindowClass1";

	//Window states
	bool m_Minimized = false;
	bool m_Maximized = false;
	bool m_Resizing = false;
};
