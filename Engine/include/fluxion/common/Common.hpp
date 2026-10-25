#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>

#define FLUXION_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define FLUXION_FORCEINLINE __forceinline
#define FLUXION_NOINLINE    __declspec(noinline)
#define FLUXION_NOVTABLE    __declspec(novtable)

#if defined(_M_IX86) || defined(__i386__)
#define FLUXION_STDCALL __stdcall
#else
#define FLUXION_STDCALL
#endif

#if defined(_MSC_VER)
#define FLUXION_UNREACHABLE() __assume(false)
#else
#include <utility>
#define FLUXION_UNREACHABLE() std::unreachable() // C++23
#endif

#define FLUXION_ALIGNAS_CONSTANT_BUFFER alignas(256)

#define FLUXION_CHECK(expr) do { \
    HRESULT _hr = (expr); \
    if (FAILED(_hr)) { \
        char _buf[256]; \
        sprintf_s(_buf, "[Fluxion] " #expr " failed (0x%08X)\n", static_cast<uint32>(_hr)); \
        OutputDebugStringA(_buf); \
        return false; \
    } \
} while (0)

#define FLUXION_SAFE_INVOKE(ptr, method_call) \
    do { auto&& _p = (ptr); if (_p) { _p->method_call; } } while (0)

#define FLUXION_ENSURE(cond) \
    do { \
        if (!(cond)) { \
            char _buf[256]; \
            std::snprintf(_buf, sizeof(_buf), "[Fluxion] ENSURE failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
            OutputDebugStringA(_buf); \
            return; \
        } \
    } while (0)

#define FLUXION_ENSURE_RET(cond, val) \
    do { \
        if (!(cond)) { \
            char _buf[256]; \
            std::snprintf(_buf, sizeof(_buf), \
                          "[Fluxion] ENSURE failed: %s (%s:%d)\n", \
                          #cond, __FILE__, __LINE__); \
            OutputDebugStringA(_buf); \
            return (val); \
        } \
    } while (0)

#define FLUXION_INPUT_ELEMENT(StructType, SemanticName, SemanticIndex, Format, MemberName) \
    { \
        SemanticName, \
        SemanticIndex, \
        Format, \
        0, \
        static_cast<UINT>(offsetof(StructType, MemberName)), \
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, \
        0 \
    }

#define FLUXION_MESSAGEBOX_ERROR(window, msg) MessageBoxW(window, msg, L"Fluxion Engine Error", MB_OK | MB_ICONERROR);

#define FLUXION_WIDEN_IMPL(x) L ## x
#define FLUXION_WIDEN(x) FLUXION_WIDEN_IMPL(x)

namespace fluxion {
using int8     = std::int8_t;
using int16    = std::int16_t;
using int32    = std::int32_t;
using int64    = std::int64_t;
using uint8    = std::uint8_t;
using uint16   = std::uint16_t;
using uint32   = std::uint32_t;
using uint64   = std::uint64_t;
using iptr     = std::intptr_t;
using uptr     = std::uintptr_t;
using isize    = std::ptrdiff_t;
using usize    = std::size_t;
using float32  = float;
using float64  = double;
using wchar    = wchar_t;

static_assert(sizeof(float32) == 4, "float32 must be 32-bit");
static_assert(sizeof(float64) == 8, "float64 must be 64-bit");
static_assert(sizeof(wchar) == 2, "Windows wchar must be 16-bit UTF-16");

constexpr uint64 KiB(uint64 x) noexcept { return x << 10; }
constexpr uint64 MiB(uint64 x) noexcept { return x << 20; }
constexpr uint64 GiB(uint64 x) noexcept { return x << 30; }
}