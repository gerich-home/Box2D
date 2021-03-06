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

#ifndef Angle_hpp
#define Angle_hpp

#include <Box2D/Common/Settings.hpp>

namespace box2d
{
	template <typename T>
	constexpr inline RealNum CvtDegreesToRadians(T value)
	{
		return static_cast<RealNum>(value * static_cast<float>(M_PI / 180.0));
	}
	
	class Angle
	{
	public:
		using data_type = RealNum;
		
		static constexpr Angle GetFromRadians(RealNum value) noexcept
		{
			return Angle{value};
		}

		Angle() = default;
		
		constexpr data_type ToRadians() const noexcept
		{
			return m_value;
		}
		
		constexpr auto operator- () const noexcept { return Angle{-m_value}; }
		constexpr auto operator+ () const noexcept { return Angle{+m_value}; }

		constexpr auto operator+= (Angle amount) noexcept
		{
			m_value += amount.m_value;
			return *this;
		}

		constexpr auto operator-= (Angle amount) noexcept
		{
			m_value -= amount.m_value;
			return *this;
		}

		constexpr auto operator*= (data_type amount) noexcept
		{
			m_value *= amount;
			return *this;
		}
		
		constexpr auto operator/= (data_type amount) noexcept
		{
			m_value /= amount;
			return *this;
		}

	private:
		
		constexpr Angle(data_type value) noexcept: m_value{value} {}
		
		data_type m_value;
	};
	
	constexpr Angle operator"" _rad(long double value) noexcept
	{
		return Angle::GetFromRadians(static_cast<RealNum>(value));
	}
	
	constexpr Angle operator"" _rad(unsigned long long int value) noexcept
	{
		return Angle::GetFromRadians(value);
	}
	
	constexpr Angle operator"" _deg(long double value) noexcept
	{
		return Angle::GetFromRadians(CvtDegreesToRadians(static_cast<Angle::data_type>(value)));
	}
	
	constexpr Angle operator"" _deg(unsigned long long int value) noexcept
	{
		return Angle::GetFromRadians(CvtDegreesToRadians(value));
	}
	
	constexpr inline bool operator== (Angle lhs, Angle rhs)
	{
		return lhs.ToRadians() == rhs.ToRadians();
	}
	
	constexpr inline bool operator!= (Angle lhs, Angle rhs)
	{
		return lhs.ToRadians() != rhs.ToRadians();
	}
	
	constexpr inline bool operator>= (Angle lhs, Angle rhs)
	{
		return lhs.ToRadians() >= rhs.ToRadians();
	}

	constexpr inline bool operator> (Angle lhs, Angle rhs)
	{
		return lhs.ToRadians() > rhs.ToRadians();
	}

	constexpr inline bool operator<= (Angle lhs, Angle rhs)
	{
		return lhs.ToRadians() <= rhs.ToRadians();
	}
	
	constexpr inline bool operator< (Angle lhs, Angle rhs)
	{
		return lhs.ToRadians() < rhs.ToRadians();
	}
	
	constexpr inline Angle operator+ (Angle lhs, Angle rhs)
	{
		return Angle::GetFromRadians(lhs.ToRadians() + rhs.ToRadians());
	}
	
	constexpr inline Angle operator- (Angle lhs, Angle rhs)
	{
		return Angle::GetFromRadians(lhs.ToRadians() - rhs.ToRadians());
	}

	constexpr inline Angle operator* (Angle::data_type scalar, Angle angle)
	{
		return Angle::GetFromRadians(angle.ToRadians() * scalar);
	}

	constexpr inline Angle operator* (Angle angle, Angle::data_type scalar)
	{
		return Angle::GetFromRadians(angle.ToRadians() * scalar);
	}
	
	constexpr inline Angle operator/ (Angle angle, Angle::data_type scalar)
	{
		return Angle::GetFromRadians(angle.ToRadians() / scalar);
	}

	constexpr inline Angle::data_type operator/ (Angle lhs, Angle rhs)
	{
		return lhs.ToRadians() / rhs.ToRadians();
	}

	/// Gets the reverse (counter) clockwise rotational angle to go from angle 1 to angle 2.
	/// @note The given angles must be normalized between -Pi to Pi radians.
	/// @return Angular rotation in the counter clockwise direction to go from angle 1 to angle 2.
	constexpr inline Angle GetRevRotationalAngle(Angle a1, Angle a2) noexcept
	{
		// If a1=90_deg and a2=45_deg then, 360_deg - (90_deg - 45) = 315_deg
		// If a1=90_deg and a2=-90_deg then, 360_deg - (90_deg - -90_deg) = 180_deg
		// If a1=45_deg and a2=90_deg then, 90_deg - 45_deg = 45_deg
		// If a1=90_deg and a2=45_deg then, 360_deg - 45_deg - 90_deg = 235_deg
		// If a1=-45_deg and a2=0_deg then, 45_deg
		// If a1=-90_deg and a2=-100_deg then, 360_deg - (-90_deg - -100_deg) = 350_deg
		// If a1=-100_deg and a2=-90_deg then, -90_deg - -100_deg = 10_deg
		return (a1 > a2)? 360_deg - (a1 - a2): a2 - a1;
	}

	inline Angle GetNormalized(Angle value) noexcept
	{
		const auto TwoPi = Pi * 2;
		const auto res = value.ToRadians() / TwoPi;
		const auto wholePart = static_cast<int>(res);
		const auto fractionPart = res - wholePart;
		return Angle::GetFromRadians(fractionPart * TwoPi);
	}
}

#endif /* Angle_hpp */
