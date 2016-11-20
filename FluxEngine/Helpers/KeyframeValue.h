#pragma once
template <typename T>
struct KeyframeValue
{
	KeyframeValue(T defaultValue)
	{
		Values[0.0f] = defaultValue;
		Values[1.0f] = defaultValue;
	}

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
			DebugLog::LogFormat(LogType::ERROR, L"Keyframe at key %f does not exist!", key);
	}

	void SetConstant(T value)
	{
		Values.clear();
		Values[0] = value;
		Values[1] = value;
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
			DebugLog::LogFormat(LogType::ERROR, L"Keyframe at key %f does not exist!", source);
	}

	T operator[](float interpValue)
	{
		for (auto value = Values.begin(); value != Values.end(); ++value)
		{
			if (value->first > interpValue)
			{
				auto v2 = value;
				auto v1 = --value;
				if (v1 == --Values.begin())
				{
					v1 = --Values.end();
				}

				float length = abs(v2->first - v1->first);
				float blendA = (v2->first - interpValue) / length;
				T result = (v1->second * blendA) + (v2->second * (1 - blendA));
				return result;
			}
		}
		return T();
	}
private:
	map<float, T> Values;
};