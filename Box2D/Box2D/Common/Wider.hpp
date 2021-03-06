/*
 * Copyright (c) 2016 Louis Langholtz https://github.com/louis-langholtz/Box2D
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef Wider_hpp
#define Wider_hpp

#include <cstdint>

namespace box2d
{
	template <typename T> struct Wider {};
	
	template<> struct Wider<std::int8_t> { using type = std::int16_t; };
	template<> struct Wider<std::int16_t> { using type = std::int32_t; };
	template<> struct Wider<std::int32_t> { using type = std::int64_t; };
	template<> struct Wider<std::int64_t> { using type = __int128_t; };

	template<> struct Wider<std::uint8_t> { using type = std::uint16_t; };
	template<> struct Wider<std::uint16_t> { using type = std::uint32_t; };
	template<> struct Wider<std::uint32_t> { using type = std::uint64_t; };
	template<> struct Wider<std::uint64_t> { using type = __uint128_t; };

	template<> struct Wider<float> { using type = double; };
	template<> struct Wider<double> { using type = long double; };
	
} // namespace box2d

#endif /* Wider_hpp */
