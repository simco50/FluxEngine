#include "stdafx.h"
#include "ParticleSystemLoader.h"
#include "../Helpers/json.hpp"
#include "../Graphics/Particle.h"

using json = nlohmann::json;

ParticleSystemLoader::ParticleSystemLoader()
{
}


ParticleSystemLoader::~ParticleSystemLoader()
{
}

ParticleSystem* ParticleSystemLoader::LoadContent(const wstring& assetFile)
{
	std::string s(assetFile.begin(), assetFile.end());
	ifstream file(s);

	ParticleSystem* pSettings = new ParticleSystem();
	try
	{
		json data = json::parse(file);

		pSettings->Lifetime = data["Lifetime"];
		pSettings->LifetimeVariance = data["LifetimeVariance"];
		pSettings->Emission = data["Emission"];
		pSettings->MaxParticles = data["MaxParticles"];
		pSettings->StartVelocity = data["StartVelocity"];
		pSettings->StartVelocityVariance = data["StartVelocityVariance"];

		json keyData = data["Size"]["Keys"];
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
			pSettings->Size.Add(stof(it.key()), it.value().get<float>());

		keyData = data["Velocity"]["Keys"];
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
		{
			json j = it.value();
			vector<float> v;
			for (auto i = j.begin(); i != j.end(); ++i)
				v.push_back(i.value().get<float>());
			pSettings->Velocity.Add(stof(it.key()), (Vector3)*v.data());
		}

		keyData = data["LocalVelocity"]["Keys"];
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
		{
			json j = it.value();
			vector<float> v;
			for (auto i = j.begin(); i != j.end(); ++i)
				v.push_back(i.value().get<float>());
			pSettings->LocalVelocity.Add(stof(it.key()), (Vector3)*v.data());
		}

		keyData = data["Color"]["Keys"];
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
		{
			json j = it.value();
			vector<float> v;
			for (auto i = j.begin(); i != j.end(); ++i)
				v.push_back(i.value().get<float>());
			pSettings->Color.Add(stof(it.key()), (Vector3)*v.data());
		}

		keyData = data["Transparancy"]["Keys"];
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
			pSettings->Transparany.Add(stof(it.key()), it.value().get<float>());

		keyData = data["Rotation"]["Keys"];
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
			pSettings->Rotation.Add(stof(it.key()), it.value().get<float>());

		pSettings->RandomStartRotation = data["RandomStartRotation"].get<bool>();

		json shapeData = data["Shape"];
		pSettings->Shape.ShapeType = (ParticleSystem::ShapeType)shapeData["ShapeType"].get<int>();
		pSettings->Shape.Radius = shapeData["Radius"].get<float>();
		pSettings->Shape.EmitFromShell = shapeData["EmitFromShell"].get<bool>();
		pSettings->Shape.EmitFromVolume = shapeData["EmitFromVolume"].get<bool>();
		pSettings->Shape.Angle = shapeData["Angle"].get<float>();


		pSettings->SortingMode = (ParticleSortingMode)data["SortingMode"].get<int>();
		string filePath = data["ImagePath"].get<string>();
		pSettings->ImagePath = wstring(filePath.begin(), filePath.end());
	}
	catch(...)
	{
		DebugLog::Log(L"Particle loading failed!", LogType::ERROR);
	}

	return pSettings;
}

void ParticleSystemLoader::Destroy(ParticleSystem* objToDestroy)
{
	SafeDelete(objToDestroy);
}