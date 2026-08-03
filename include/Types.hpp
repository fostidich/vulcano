#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

// Unsigned integer types
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
#if defined(__SIZEOF_INT128__)
using u128 = unsigned __int128;
#endif
using usize = std::size_t;

// Signed integer types
using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
#if defined(__SIZEOF_INT128__)
using i128 = __int128;
#endif
using isize = std::ptrdiff_t;

// Floating-point types
using f32 = float;
using f64 = double;

// String types
using cstr   = const char *;
using str    = std::string_view;
using string = std::string;
