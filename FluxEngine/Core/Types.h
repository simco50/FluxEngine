#pragma once

#if defined(_MSC_FULL_VER)
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;

using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;
#else
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
#endif

template<typename T>
using UniquePtr = std::unique_ptr<T>;
template<typename T>
using SharedPtr = std::shared_ptr<T>;
template<typename T>
using WeakPtr = std::weak_ptr<T>;

#define FORCEINLINE __forceinline
#define FORCENOINLINE __declspec(noinline)

#define PRAGMA_DISABLE_OPTIMIZATION __pragma(optimize("",off))
#define PRAGMA_ENABLE_OPTIMIZATION  __pragma(optimize("",on))

#define STD_CALL __stdcall
#define CDECL_CALL __cdecl