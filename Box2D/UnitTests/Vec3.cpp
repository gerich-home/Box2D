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
#include <Box2D/Common/Math.hpp>
#include <sstream>

using namespace box2d;

TEST(Vec3, ByteSizeIs_12_24_or_48)
{
	switch (sizeof(RealNum))
	{
		case  4: EXPECT_EQ(sizeof(Vec3), size_t(12)); break;
		case  8: EXPECT_EQ(sizeof(Vec3), size_t(24)); break;
		case 16: EXPECT_EQ(sizeof(Vec3), size_t(48)); break;
		default: FAIL(); break;
	}
}
