#pragma once

namespace BitOperations
{
	template<typename T>
	bool LeastSignificantBit(T mask, unsigned int* pIndex)
	{
		if (mask == 0)
		{
			return false;
		}
		*pIndex = 0;
		while (!(mask & 1))
		{
			mask >>= 1;
			++(*pIndex);
		}
		return true;
	}

	template<typename T>
	bool MostSignificantBit(T mask, unsigned int* pIndex)
	{
		if (mask == 0)
		{
			return false;
		}
		*pIndex = 0;
		while (mask)
		{
			mask >>= 1;
			++(*pIndex);
		}
		--(*pIndex);
		return true;
	}
}

template<typename size_t Bits, typename Storage = unsigned int>
class BitField;

template<typename size_t Bits, typename Storage>
class BitField
{
public:
	BitField()
	{
		ClearAll();
	}

	BitField(const bool set)
	{
		if (set)
		{
			SetAll();
		}
		else
		{
			ClearAll();
		}
	}

	inline void SetBit(const size_t bit)
	{
		Data[StorageIndexOfBit(bit)] |= MakeBitmaskForStorage(bit);
	}

	void SetBitAndUp(const size_t bit)
	{
		size_t storageIndex = StorageIndexOfBit(bit);
		for (size_t i = storageIndex + 1; i < Elements(); ++i)
		{
			Data[i] = ((Storage)~0);
		}
		Data[storageIndex] |= ((Storage)~0) << IndexOfBitInStorage(bit);
	}

	void SetBitAndDown(const size_t bit)
	{
		size_t storageIndex = StorageIndexOfBit(bit + 1);
		for (int i = storageIndex - 1; i >= 0; --i)
		{
			Data[i] = ((Storage)~0);
		}
		Data[storageIndex] |= ((Storage)~0) >> (BitsPerStorage() - IndexOfBitInStorage(bit + 1));
	}

	inline void ClearBit(const size_t bit)
	{
		Data[StorageIndexOfBit(bit)] &= ~MakeBitmaskForStorage(bit);
	}

	inline bool GetBit(const size_t bit) const
	{
		return (Data[StorageIndexOfBit(bit)] & MakeBitmaskForStorage(bit)) != 0;
	}

	void AssignBit(const size_t bit, const bool set)
	{
		set ? SetBit(bit) : ClearBit(bit);
	}

	void ClearAll()
	{
		for (size_t i = 0; i < Elements(); i++)
		{
			Data[i] = Storage();
		}
	}

	void SetAll()
	{
		for (size_t i = 0; i < Elements(); i++)
		{
			Data[i] = ((Storage)~0);
		}
	}

	bool IsEqual(const BitField& other) const
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			if (Data[i] != other.Data[i])
			{
				return false;
			}
		}
		return true;
	}

	bool AnyBitSet() const
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			if (Data[i] > 0)
			{
				return true;
			}
		}
		return false;
	}

	bool NoBitSet() const
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			if (Data[i] > 0)
			{
				return false;
			}
		}
		return true;
	}

	bool MostSignificantBit(size_t* pIndex)
	{
		for (size_t i = Elements() - 1; i >= 0; --i)
		{
			if (BitOperations::MostSignificantBit(Data[i], pIndex) == true)
			{
				*pIndex += i * BitsPerStorage();
				return true;
			}
		}
		return false;
	}

	bool LeastSignificantBit(size_t* pIndex)
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			if (BitOperations::LeastSignificantBit(Data[i], pIndex) == true)
			{
				*pIndex += i * BitsPerStorage();
				return true;
			}
		}
		return false;
	}

	bool operator==(const BitField& other) const
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			if (Data[i] != other.Data[i])
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=(const BitField& other) const
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			if (Data[i] != other.Data[i])
			{
				return true;
			}
		}
		return false;
	}

	BitField& operator&=(const BitField& other)
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			Data[i] &= other.Data[i];
		}
		return *this;
	}

	BitField& operator|=(const BitField& other)
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			Data[i] |= other.Data[i];
		}
		return *this;
	}

	BitField& operator^=(const BitField& other)
	{
		for (size_t i = 0; i < Elements(); ++i)
		{
			Data[i] ^= other.Data[i];
		}
		return *this;
	}

	BitField operator&(const BitField& other) const
	{
		BitField out;
		for (size_t i = 0; i < Elements(); ++i)
		{
			out.Data[i] = Data[i] & other.Data[i];
		}
		return out;
	}

	BitField operator|(const BitField& other) const
	{
		BitField out;
		for (size_t i = 0; i < Elements(); ++i)
		{
			out.Data[i] = Data[i] | other.Data[i];
		}
		return out;
	}

	BitField operator^(const BitField& other) const
	{
		BitField out;
		for (size_t i = 0; i < Elements(); ++i)
		{
			out.Data[i] = Data[i] ^ other.Data[i];
		}
		return out;
	}

	BitField operator~() const
	{
		BitField out;
		for (size_t i = 0; i < Elements(); ++i)
		{
			out.Data[i] = ~Data[i];
		}
		return out;
	}

private:
	static constexpr size_t StorageIndexOfBit(size_t bit)
	{
		return bit / BitsPerStorage();
	}

	static constexpr size_t IndexOfBitInStorage(size_t bit)
	{
		return bit % BitsPerStorage();
	}

	static constexpr size_t BitsPerStorage()
	{
		return sizeof(Storage) * 8;
	}

	static constexpr size_t MakeBitmaskForStorage(size_t bit)
	{
		return 1 << IndexOfBitInStorage(bit);
	}

	static constexpr size_t Elements()
	{
		return (Bits + BitsPerStorage() - 1) / BitsPerStorage();
	}

	Storage Data[Elements()];
};