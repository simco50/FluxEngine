#include "stdafx.h"
#include "ParticleSystemLoader.h"
#include "Helpers/json.hpp"
#include "Rendering/ParticleSystem/Particle.h"

using json = nlohmann::json;

#define LOAD_VECTOR3_KEY(jsonData, name) \
{\
	json keys = jsonData[#name]["Keys"];\
	for (auto it = keys.begin(); it != keys.end(); ++it)\
	{\
		json j = it.value();\
		pSystem->name.Add(stof(it.key()), Vector3(j["X"].get<float>(), j["Y"].get<float>(), j["Z"].get<float>()));\
	}\
}\
{\
	json constantData = jsonData[#name]["Constant"];\
	pSystem->name.ConstantValue = Vector3(constantData["X"].get<float>(),constantData["Y"].get<float>(),constantData["Z"].get<float>());\
}

#define LOAD_FLOAT_KEY(jsonData, name) \
{\
	json keys = jsonData[#name]["Keys"];\
	for (auto it = keys.begin(); it != keys.end(); ++it)\
		pSystem->name.Add(stof(it.key()), it.value());\
}\
pSystem->name.ConstantValue = data[#name]["Constant"];

ParticleSystemLoader::ParticleSystemLoader()
{
}

ParticleSystemLoader::~ParticleSystemLoader()
{
}

ParticleSystem* ParticleSystemLoader::LoadContent(const string& assetFile)
{
	unique_ptr<IFile> pFile = FileSystem::GetFile(assetFile);
	if (pFile == nullptr)
		return nullptr;
	if (!pFile->Open(FileMode::Read, ContentType::Text))
		return nullptr;

	std::string buffer;
	buffer.resize(pFile->GetSize());
	pFile->Read((unsigned int)buffer.size(), (char*)&buffer[0]);
	pFile->Close();

	ParticleSystem* pSystem = new ParticleSystem();
	try
	{
		json data = json::parse(buffer.data());

		int version = data["Version"];
		if(version != VERSION)
		{
			stringstream error;
			error << "Particle version mismatch: Version is " << version << ". Expected " << VERSION << "!";
			throw exception(error.str().c_str());
		}
		//General
		pSystem->Duration = data["Duration"];
		pSystem->Loop = data["Loop"];
		pSystem->Lifetime = data["Lifetime"];
		pSystem->LifetimeVariance = data["LifetimeVariance"];
		pSystem->StartVelocity = data["StartVelocity"];
		pSystem->StartVelocityVariance = data["StartVelocityVariance"];
		pSystem->StartSize = data["StartSize"];
		pSystem->StartSizeVariance = data["StartSizeVariance"];
		pSystem->RandomStartRotation = data["RandomStartRotation"];
		pSystem->PlayOnAwake = data["PlayOnAwake"];
		pSystem->MaxParticles = data["MaxParticles"];

		//Emission
		pSystem->Emission = data["Emission"];
		json keyData = data["Bursts"];
		for(auto it = keyData.begin(); it != keyData.end(); ++it)
			pSystem->Bursts[stof(it.key())] = it.value();

		//Shape
		pSystem->Shape.ShapeType = (ParticleSystem::ShapeType)data["Shape"]["ShapeType"].get<int>();
		pSystem->Shape.Radius = data["Shape"]["Radius"];
		pSystem->Shape.EmitFromShell = data["Shape"]["EmitFromShell"];
		pSystem->Shape.EmitFromVolume = data["Shape"]["EmitFromVolume"];
		pSystem->Shape.Angle = data["Shape"]["Angle"];

		//Animation
		keyData = data["Size"]["Keys"];
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
			pSystem->Size.Add(stof(it.key()), it.value());
		pSystem->Size.ConstantValue = data["Size"]["Constant"];

		LOAD_FLOAT_KEY(data, Size);
		LOAD_VECTOR3_KEY(data, Velocity);
		LOAD_VECTOR3_KEY(data, LocalVelocity);
		LOAD_VECTOR3_KEY(data, Color);
		LOAD_FLOAT_KEY(data, Transparancy);
		LOAD_FLOAT_KEY(data, Rotation);

		//Rendering
		pSystem->SortingMode = (ParticleSortingMode)data["SortingMode"].get<int>();
		pSystem->BlendMode = (ParticleBlendMode)data["BlendMode"].get<int>();
		pSystem->ImagePath = data["ImagePath"].get<string>();
	}
	catch(exception exception)
	{
		string error = exception.what();
		FLUX_LOG(ERROR, "Particle loading failed!\nJson Parser: %s", wstring(error.begin(), error.end()).c_str());
	}
	return pSystem;
}

void ParticleSystemLoader::Destroy(ParticleSystem* objToDestroy)
{
	SafeDelete(objToDestroy);
}