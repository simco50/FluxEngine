#pragma once
#include "GameTimer.h"

class InputManager;
class CameraComponent;
class RenderTarget;
class ShadowMapper;
class MaterialManager;

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
	std::wstring Title = L"SirenEngine";
	XMFLOAT4 ClearColor = (XMFLOAT4)Colors::Green;
	bool MSAA;
	UINT MsaaQuality = 0;
	bool VerticalSync = true;
	bool UseDeferredRendering = true;

	float AspectRatio() const { return (float)Width / (float)Height; }
};

struct SceneContext
{
	CameraComponent* CurrentCamera = nullptr;
	vector<CameraComponent*> Cameras;
	InputManager* Input = nullptr;
	ShadowMapper* ShadowMapper = nullptr;
	MaterialManager* MaterialManager = nullptr;
};

struct EngineContext
{
	ID3D11Device* D3Device = nullptr;
	ID3D11DeviceContext* D3DeviceContext = nullptr;
	HWND Hwnd = nullptr;
	GameSettings GameSettings;
	RenderTarget* DefaultRenderTarget = nullptr;
};

struct GameContext
{
	EngineContext* Engine;
	SceneContext* Scene;
};