#pragma once
#include "Rendering\Core\GraphicsDefines.h"

class Window
{
public:
	Window(
		const int width, 
		const int height, 
		const std::string& title = "Window", 
		const WindowType type = WindowType::WINDOWED, 
		const bool resizable = true, 
		const std::string& className = "windowClass");

	~Window();

	void SetIcon(const std::string& iconPath);
	void SetTitle(const string& title);
	void SetPosition(const int x, const int y);

	MulticastDelegate<int, int>& OnWindowSizeChanged() { return m_OnWindowSizeChangedEvent; }
	MulticastDelegate<bool>& OnWindowStateChanged() { return m_OnWindowStateChangedEvent; }

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	float GetAspect() const { return (float)m_Width / m_Height; }
	WindowType GetType() const { return m_Type; }
	bool IsMinimized() const { return m_Minimized; }
	bool IsMaximized() const { return m_Maximized; }
	bool IsActive() const { return m_Active; }

private:
	bool CreateClass(const std::string& className);
	bool CreateInstanceOfClass(const std::string& className, int windowWidth, int windowHeight);

	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	int m_Width;
	int m_Height;
	HWND m_Hwnd = nullptr;
	WindowType m_Type;
	bool m_Resizable = true;
	bool m_Resizing = false;

	bool m_Minimized = false;
	bool m_Maximized = false;
	bool m_Active = false;

	int m_StartWidth = -1;
	int m_StartHeight = -1;
	
	std::string m_Title;
	std::string m_ClassName;

	MulticastDelegate<int, int> m_OnWindowSizeChangedEvent;
	MulticastDelegate<bool> m_OnWindowStateChangedEvent;
};