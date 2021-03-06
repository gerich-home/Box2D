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

#include "gtest/gtest.h"
#include <cmath>
#include <limits>
#include <iostream>

TEST(float, BiggerValsIncreasinglyInaccurate)
{
	// This test is meant to demonstrate the increasing inaccuracy of the float type and help
	// recognize the problems that using this type can cause. Note that the double suffers the
	// same way except more slowly. This increasing inaccuracy is inherent to how floating point
	// types are designed.
	//
	// A way to avoid this problem, is to use fixed-point calculations (instead of floating-point
	// calculations).
	
	auto last_delta = float(0);
	auto val = float(1);
	for (auto i = 0; i < 24; ++i)
	{
		const auto next = std::nextafter(val, std::numeric_limits<float>::max());
		const auto delta = next - val;
		ASSERT_EQ(val + (delta / 2), val);
		
		// For  0x1p+0, delta of next value is 0x1p-23: ie. at      1, delta is 0.0000001192092895508
		// For  0x1p+1, delta of next value is 0x1p-22: ie. at      2, delta is 0.0000002384185791016
		// For  0x1p+2, delta of next value is 0x1p-21: ie. at      4, delta is 0.0000004768371582031
		// For  0x1p+3, delta of next value is 0x1p-20: ie. at      8, delta is 0.0000009536743164062
		// For  0x1p+4, delta of next value is 0x1p-19: ie. at     16, delta is 0.0000019073486328125
		// For  0x1p+5, delta of next value is 0x1p-18: ie. at     32, delta is 0.0000038146972656250
		// For  0x1p+6, delta of next value is 0x1p-17: ie. at     64, delta is 0.0000076293945312500
		// For  0x1p+7, delta of next value is 0x1p-16: ie. at    128, delta is 0.0000152587890625000
		// For  0x1p+8, delta of next value is 0x1p-15: ie. at    256, delta is 0.0000305175781250000
		// For  0x1p+9, delta of next value is 0x1p-14: ie. at    512, delta is 0.0000610351562500000
		// For 0x1p+10, delta of next value is 0x1p-13: ie. at   1024, delta is 0.0001220703125000000
		// For 0x1p+11, delta of next value is 0x1p-12: ie. at   2048, delta is 0.0002441406250000000
		// For 0x1p+12, delta of next value is 0x1p-11: ie. at   4096, delta is 0.0004882812500000000
		// For 0x1p+13, delta of next value is 0x1p-10: ie. at   8192, delta is 0.0009765625000000000
		// For 0x1p+14, delta of next value is  0x1p-9: ie. at  16384, delta is 0.0019531250000000000
		// For 0x1p+15, delta of next value is  0x1p-8: ie. at  32768, delta is 0.0039062500000000000
		// For 0x1p+16, delta of next value is  0x1p-7: ie. at  65536, delta is 0.0078125000000000000
		// For 0x1p+17, delta of next value is  0x1p-6: ie. at 131072, delta is 0.0156250000000000000
		// For 0x1p+18, delta of next value is  0x1p-5: ie. at 262144, delta is 0.0312500000000000000
		// For 0x1p+19, delta of next value is  0x1p-4: ie. at 524288, delta is 0.0625000000000000000
		//
		// If a floating-point type is used in the implementation of the simulation then, these
		// deltas mean that:
		// - The farther bodies get out from the origin (0, 0) the less accurately they can be moved.
		// - The larger shape vertex radiuses get, the less accurately time of impact can be
		//   calculated for those shapes.
#if 0
		std::cout << std::hexfloat;
		std::cout << "For " << std::setw(7) << val << ", delta of next value is " << std::setw(7) << delta;
		std::cout << std::defaultfloat;
		std::cout << ": ie. at " << std::setw(6) << val;
		std::cout << std::fixed;
		std::cout << ", delta is " << delta;
		std::cout << std::endl;
#endif
		val *= 2;
		EXPECT_GT(delta, last_delta);
		last_delta = delta;
	}
}

TEST(float, max)
{
	EXPECT_EQ(std::numeric_limits<float>::max() * 2, std::numeric_limits<float>::infinity());
	
	EXPECT_LT(std::sqrtf(std::numeric_limits<float>::max()), std::numeric_limits<float>::max());
	EXPECT_GT(std::numeric_limits<float>::max(), std::sqrtf(std::numeric_limits<float>::max()));
}

TEST(float, infinity)
{
	EXPECT_EQ(std::numeric_limits<float>::infinity() * 2, std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity() * 0.5f, std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity() * -1, -std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity() * -0.5f, -std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity() * std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity() * -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

	EXPECT_EQ(std::numeric_limits<float>::infinity() / 2, std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity() / 0.5f, std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity() / -1, -std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity() / -0.5f, -std::numeric_limits<float>::infinity());

	EXPECT_EQ(0.0f / std::numeric_limits<float>::infinity(), 0.0f);
	EXPECT_EQ(1.0f / std::numeric_limits<float>::infinity(), 0.0f);
	EXPECT_EQ(-1.0f / std::numeric_limits<float>::infinity(), 0.0f);

	EXPECT_TRUE(std::isnan(std::numeric_limits<float>::infinity() * 0));
	EXPECT_TRUE(std::isnan(std::numeric_limits<float>::infinity() / std::numeric_limits<float>::infinity()));
	EXPECT_TRUE(std::isnan(std::numeric_limits<float>::infinity() / -std::numeric_limits<float>::infinity()));
	
	EXPECT_GT(std::numeric_limits<float>::infinity(), 0.0f);
	EXPECT_LT(0.0f, std::numeric_limits<float>::infinity());
	EXPECT_EQ(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());

	EXPECT_EQ(static_cast<int>(std::numeric_limits<float>::infinity()), std::numeric_limits<int>::min());
	EXPECT_EQ(static_cast<int>(-std::numeric_limits<float>::infinity()), std::numeric_limits<int>::min());
	EXPECT_EQ(static_cast<double>(std::numeric_limits<float>::infinity()), std::numeric_limits<double>::infinity());
	EXPECT_EQ(static_cast<double>(-std::numeric_limits<float>::infinity()), -std::numeric_limits<double>::infinity());
}

TEST(float, nan)
{
	EXPECT_TRUE(std::isnan(std::numeric_limits<float>::quiet_NaN() * 0));
	EXPECT_TRUE(std::isnan(std::numeric_limits<float>::quiet_NaN() * 1));
	EXPECT_TRUE(std::isnan(std::numeric_limits<float>::quiet_NaN() * std::numeric_limits<float>::infinity()));
	EXPECT_TRUE(std::isnan(std::numeric_limits<float>::quiet_NaN() / 1));
	EXPECT_TRUE(std::isnan(0.0f / std::numeric_limits<float>::quiet_NaN()));
	EXPECT_TRUE(std::isnan(1.0f / std::numeric_limits<float>::quiet_NaN()));
	EXPECT_TRUE(std::isnan(std::numeric_limits<float>::quiet_NaN() / std::numeric_limits<float>::infinity()));
	EXPECT_FALSE(std::numeric_limits<float>::quiet_NaN() > 0.0f);
	EXPECT_FALSE(std::numeric_limits<float>::quiet_NaN() < 0.0f);
	EXPECT_NE(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN());
	EXPECT_NE(std::numeric_limits<float>::quiet_NaN(), 0.0f);
}

TEST(float, sqrt)
{
	EXPECT_EQ(std::sqrt(4.0f), 2.0f);
	EXPECT_EQ(std::sqrt(1.0f), 1.0f);
	EXPECT_EQ(std::sqrt(0.0f), 0.0f);
	EXPECT_EQ(std::sqrt(std::numeric_limits<float>::infinity()), std::numeric_limits<float>::infinity());
	EXPECT_TRUE(std::isnan(std::sqrt(-1.0f)));
	EXPECT_TRUE(std::isnan(std::sqrt(std::numeric_limits<float>::quiet_NaN())));
}
