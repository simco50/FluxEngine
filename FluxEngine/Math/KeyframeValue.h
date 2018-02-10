#pragma once

template <typename T>
struct KeyframeValue
{
	KeyframeValue() : ConstantValue(T()) {}

	KeyframeValue(T defaultValue) : ConstantValue(defaultValue) {}

	void Add(float key, T value)
	{
		Values[key] = value;
	}

	void Set(float key, T value)
	{
		auto v = Values.find(key);
		if (v != Values.end())
			v->second = value;
		else
			FLUX_LOG(LogType::Error, "Keyframe at key %f does not exist!", key);
	}

	void SetConstant(T value)
	{
		Values.clear();
		ConstantValue = value;
	}

	void Move(float source, float target)
	{
		auto s = Values.find(source);
		if (s != Values.end())
		{
			Values[target] = s->second;
			Values.erase(s);
		}
		else
			FLUX_LOG(LogType::Error, "Keyframe at key %f does not exist!", source);
	}

	T operator[](float interpValue)
	{
		//If the value is constant
		if(Values.size() == 0)
			return ConstantValue;
		//If there is only one keyframe
		if(Values.size() == 1)
		{
			auto it = Values.begin();
			if (interpValue >= it->first)
				return it->second;
			float blendA = (it->first - interpValue) / it->first;
			T result = (ConstantValue * blendA) + (it->second * (1 - blendA));
			return result;
		}

		//If there is more than one key
		if(interpValue < Values.begin()->first)
			return Values.begin()->second;

		for (auto value = Values.begin(); value != Values.end(); ++value)
		{
			if (value->first == interpValue)
				return value->second;
			if (value->first > interpValue)
			{
				auto v2 = value;
				auto v1 = --value;
				float length = abs(v2->first - v1->first);
				float blendA = (v2->first - interpValue) / length;
				T result = (v1->second * blendA) + (v2->second * (1 - blendA));
				return result;
			}
		}
		return Values.rbegin()->second;
	}

	const std::map<float, T>& GetData() const
	{
		return Values;
	}

	T ConstantValue;
private:
	std::map<float, T> Values;
};
