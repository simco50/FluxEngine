#include "FluxEngine.h"
#include "ParticleSystem.h"

#include "External/NlohmannJson/json.hpp"
#include "IO/InputStream.h"

using json = nlohmann::json;

#define LOAD_VECTOR3_KEY(jsonData, name) \
{\
	(name).Clear(); \
	json keys = (jsonData)[#name]["Keys"];\
	for (auto it = keys.begin(); it != keys.end(); ++it)\
	{\
		json j = it.value();\
		(name).Add(stof(it.key()), Vector3(j["X"].get<float>(), j["Y"].get<float>(), j["Z"].get<float>()));\
	}\
}\
{\
	json constantData = (jsonData)[#name]["Constant"];\
	(name).ConstantValue = Vector3(constantData["X"].get<float>(),constantData["Y"].get<float>(),constantData["Z"].get<float>());\
} \

#define LOAD_FLOAT_KEY(jsonData, name) \
{\
	(name).Clear(); \
	json keys = (jsonData)[#name]["Keys"];\
	for (auto it = keys.begin(); it != keys.end(); ++it)\
		(name).Add(stof(it.key()), it.value());\
}\
(name).ConstantValue = data[#name]["Constant"]

bool ParticleSystem::Load(InputStream& inputStream)
{
	std::string fileName = inputStream.GetSource();
	AUTOPROFILE_DESC(ParticleSystem_Load, Paths::GetFileName(fileName));

	std::string buffer;
	buffer.resize(inputStream.GetSize());
	inputStream.Read((char*)&buffer[0], buffer.size());

	try
	{
		json data = json::parse(buffer);

		int version = data["Version"];
		if (version != VERSION)
		{
			std::stringstream error;
			error << "Particle version mismatch: Version is " << version << ". Expected " << VERSION << "!";
			throw std::exception(error.str().c_str());
		}
		//General
		Duration = data["Duration"];
		Loop = data["Loop"];
		Lifetime = data["Lifetime"];
		LifetimeVariance = data["LifetimeVariance"];
		StartVelocity = data["StartVelocity"];
		StartVelocityVariance = data["StartVelocityVariance"];
		StartSize = data["StartSize"];
		StartSizeVariance = data["StartSizeVariance"];
		RandomStartRotation = data["RandomStartRotation"];
		PlayOnAwake = data["PlayOnAwake"];
		MaxParticles = data["MaxParticles"];

		//Emission
		Emission = data["Emission"];
		json keyData = data["Bursts"];
		Bursts.clear();
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
			Bursts[stof(it.key())] = it.value();

		//Shape
		Shape.ShapeType = (ParticleSystem::ShapeType)data["Shape"]["ShapeType"].get<int>();
		Shape.Radius = data["Shape"]["Radius"];
		Shape.EmitFromShell = data["Shape"]["EmitFromShell"];
		Shape.EmitFromVolume = data["Shape"]["EmitFromVolume"];
		Shape.Angle = data["Shape"]["Angle"];

		//Animation
		keyData = data["Size"]["Keys"];
		Size.Clear();
		for (auto it = keyData.begin(); it != keyData.end(); ++it)
			Size.Add(stof(it.key()), it.value());
		Size.ConstantValue = data["Size"]["Constant"];

		LOAD_FLOAT_KEY(data, Size);
		LOAD_VECTOR3_KEY(data, Velocity);
		LOAD_VECTOR3_KEY(data, LocalVelocity);
		LOAD_VECTOR3_KEY(data, Color);
		LOAD_FLOAT_KEY(data, Transparancy);
		LOAD_FLOAT_KEY(data, Rotation);

		//Rendering
		SortingMode = (ParticleSortingMode)data["SortingMode"].get<int>();

		enum ParticleBlendMode
		{
			Alpha = 0,
			Add,
		};
		ParticleBlendMode blendMode = (ParticleBlendMode)data["BlendMode"].get<int>();
		switch (blendMode)
		{
		case Alpha:
			BlendingMode = BlendMode::ALPHA;
			break;
		case Add:
			BlendingMode = BlendMode::ADD;
			break;
		default:
			break;
		}
		ImagePath = data["ImagePath"].get<std::string>();
	}
	catch (std::exception& exception)
	{
		std::string error = exception.what();
		FLUX_LOG(Warning, "Particle loading failed!\nJson Parser: %s", std::wstring(error.begin(), error.end()).c_str());
		return false;
	}

	RefreshMemoryUsage();

	return true;
}

void ParticleSystem::RefreshMemoryUsage()
{
	unsigned int memoryUsage = sizeof(ParticleSystem);
	memoryUsage += (unsigned int)Size.ByteSize();
	memoryUsage += (unsigned int)Velocity.ByteSize();
	memoryUsage += (unsigned int)LocalVelocity.ByteSize();
	memoryUsage += (unsigned int)Color.ByteSize();
	memoryUsage += (unsigned int)Transparancy.ByteSize();
	memoryUsage += (unsigned int)Rotation.ByteSize();
	SetMemoryUsage(memoryUsage);
}
