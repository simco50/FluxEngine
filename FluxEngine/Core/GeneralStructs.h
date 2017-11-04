#pragma once
class Camera;


struct SceneContext
{
	Camera* Camera = nullptr;
};

struct EngineContext
{
	HWND Hwnd = nullptr;
};

struct GameContext
{
	EngineContext* Engine;
	SceneContext* Scene;
};