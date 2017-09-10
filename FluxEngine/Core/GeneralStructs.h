#pragma once
class Camera;

enum class WindowStyle
{
	WINDOWED,
	FULLSCREEN,
	BORDERLESS
};

struct GameSettings
{
	int Width = 1240;
	int Height = 720;
	WindowStyle WindowStyle = WindowStyle::WINDOWED;
	std::string Title = "";
	XMFLOAT4 ClearColor = (XMFLOAT4)Colors::Green;
	bool MSAA;
	UINT MsaaQuality = 0;
	bool VerticalSync = true;

	float AspectRatio() const { return (float)Width / (float)Height; }
};

struct SceneContext
{
	Camera* Camera = nullptr;
};

struct EngineContext
{
	HWND Hwnd = nullptr;
	GameSettings GameSettings;
};

struct GameContext
{
	EngineContext* Engine;
	SceneContext* Scene;
};