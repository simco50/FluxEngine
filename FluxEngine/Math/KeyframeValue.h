#pragma once

template<typename T>
struct KeyframeValue
{
	struct Key
	{
		float Time;
		T Value;
	};

	KeyframeValue()
	{}

	KeyframeValue(T constant)
	{
		SetConstant(constant);
	}

	void SetConstant(T constant)
	{
		m_Keys.resize(1);
		m_Keys[0].Time = 0.0f;
		m_Keys[0].Value = constant;
	}

	void Set(float time, T value)
	{
		for (Key& key : m_Keys)
		{
			if (key.Time == time)
			{
				key.Value = value;
				return;
			}
		}
		checkNoEntry();
	}

	void Add(float time, T value, bool sort = true)
	{
		check(std::find_if(m_Keys.begin(), m_Keys.end(), [time](const Key& a) {return a.Time == time; }) == m_Keys.end());
		Key k;
		k.Time = time;
		k.Value = value;
		m_Keys.push_back(k);
		if (sort)
		{
			std::sort(m_Keys.begin(), m_Keys.end(), [](const Key& a, const Key& b) {return a.Time < b.Time; });
		}
	}

	T GetValue(float time) const
	{
		if (m_Keys.size() == 0)
		{
			return T();
		}
		if (m_Keys.size() == 1)
		{
			return m_Keys[0].Value;
		}
		for (int i = 0; i < (int)m_Keys.size(); ++i)
		{
			const Key& a = m_Keys[i];
			if (a.Time > time)
			{
				int previousKey = (int)(i - 1 + m_Keys.size()) % m_Keys.size();
				const Key& b = m_Keys[previousKey];
				float t = (time - b.Time) / abs(a.Time - b.Time);
				return b.Value + t * (a.Value - b.Value);
			}
		}
		return T();
	}

	void Clear()
	{
		m_Keys.clear();
		m_PreviousKey = 0;
	}

	size_t ByteSize() const
	{
		return m_Keys.size() * sizeof(Key);
	}

	using ConstIterator = typename std::vector<Key>::const_iterator;

	ConstIterator begin()
	{
		return m_Keys.cbegin();
	}

	ConstIterator end()
	{
		return m_Keys.cend();
	}

private:
	mutable int m_PreviousKey = 0;
	std::vector<Key> m_Keys;
};