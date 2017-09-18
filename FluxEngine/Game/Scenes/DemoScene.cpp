#include "stdafx.h"
#include "DemoScene.h"
#include "Rendering/MeshRenderer.h"
#include "Scenegraph/GameObject.h"
#include "Materials/Forward/DefaultMaterial.h"
#include "Core/Components/Transform.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/ParticleSystem/ParticleEmitter.h"
#include "Rendering/Shader.h"

DemoScene::DemoScene()
{}

DemoScene::~DemoScene()
{
}

void DemoScene::Initialize()
{
	/*Shader pShader;
	pShader.Load("Resources/test.hlsl");
	ShaderVariation* pVertexShader = pShader.GetVariation(ShaderType::VertexShader, { "HAHA" });
	ShaderVariation* pPixelShader = pShader.GetVariation(ShaderType::PixelShader, { "HAHA" });*/
}

void DemoScene::Update()
{
	/*for (int i = 0; i < objects.size(); ++i)
	{
		objects[i]->GetTransform()->Rotate(0, 0, pow(-1, i) * 2);
	}*/
}

void DemoScene::LateUpdate()
{
}