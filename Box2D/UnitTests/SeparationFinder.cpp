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
#include <Box2D/Collision/SeparationFinder.hpp>
#include <Box2D/Collision/Simplex.hpp>
#include <Box2D/Collision/TimeOfImpact.hpp>
#include <Box2D/Collision/DistanceProxy.hpp>
#include <Box2D/Collision/Distance.hpp>
#include <Box2D/Collision/Shapes/PolygonShape.hpp>

using namespace box2d;

TEST(SeparationFinder, ByteSizeIs_40_56_or_96)
{
	switch (sizeof(RealNum))
	{
		case  4: EXPECT_EQ(sizeof(SeparationFinder), size_t(40)); break;
		case  8: EXPECT_EQ(sizeof(SeparationFinder), size_t(56)); break;
		case 16: EXPECT_EQ(sizeof(SeparationFinder), size_t(96)); break;
		default: FAIL(); break;
	}
}

TEST(SeparationFinder, BehavesAsExpected)
{
	const auto shape = PolygonShape{0.5f, 0.5f};
	const auto distproxy = GetDistanceProxy(shape, 0);

	const auto x = RealNum(100);
	const auto sweepA = Sweep{Position{Vec2{-x, 0}, 0_deg}, Position{Vec2{+x, 0}, 0_deg}};
	const auto sweepB = Sweep{Position{Vec2{+x, 0}, 0_deg}, Position{Vec2{-x, 0}, 0_deg}};
	
	auto t = RealNum{0}; // Will be set to value of t2
	auto last_s = MaxFloat;
	auto last_distance = MaxFloat;
	auto xfA = GetTransformation(sweepA, t);
	auto xfB = GetTransformation(sweepB, t);
	Simplex::Cache cache;
	auto distanceInfo = Distance(distproxy, xfA, distproxy, xfB, cache);
	cache = Simplex::GetCache(distanceInfo.simplex.GetEdges());
	const auto fcn = SeparationFinder::Get(cache.GetIndices(), distproxy, xfA, distproxy, xfB);
	EXPECT_EQ(fcn.GetType(), SeparationFinder::e_faceA);
	EXPECT_EQ(Vec2(fcn.GetAxis()), Vec2(1, 0));
	EXPECT_EQ(fcn.GetLocalPoint(), Vec2(0.5, 0));

	auto last_min_sep = MaxFloat;
	for (auto i = 0u; i < 500; ++i)
	{
		// Prepare input for distance query.
		const auto witnessPoints = GetWitnessPoints(distanceInfo.simplex);
		const auto distance = Sqrt(GetLengthSquared(witnessPoints.a - witnessPoints.b));

		const auto minSeparation = fcn.FindMinSeparation(xfA, xfB);

		EXPECT_EQ(minSeparation.indexPair, (IndexPair{IndexPair::InvalidIndex, 2}));
		EXPECT_LT(minSeparation.distance, last_s);
		if (minSeparation.distance > 0)
		{
			EXPECT_LT(distance, last_distance);
			EXPECT_NEAR(double(minSeparation.distance), double(distance), 0.00001);
		}
		else if (minSeparation.distance < 0)
		{
			if (last_min_sep < 0 && distance != 0)
			{
				EXPECT_GT(distance, last_distance);
			}
		}
		last_min_sep = minSeparation.distance;
		
		const auto s = fcn.Evaluate(minSeparation.indexPair, xfA, xfB);
		EXPECT_EQ(s, minSeparation.distance);
		if (s >= 0)
		{
			EXPECT_NEAR(double(s), double(distance), 0.0001);
		}
		else
		{
			EXPECT_LE(double(s), double(distance));
		}
		EXPECT_LT(s, last_s);
		
		//t = std::nextafter(t, 1.0f);
		t += RealNum(.001);
		last_distance = distance;
		last_s = s;
		xfA = GetTransformation(sweepA, t);
		xfB = GetTransformation(sweepB, t);
		distanceInfo = Distance(distproxy, xfA, distproxy, xfB, cache);
		cache = Simplex::GetCache(distanceInfo.simplex.GetEdges());
	}
}
