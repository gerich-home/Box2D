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
#include <Box2D/Dynamics/Contacts/ContactSolver.hpp>
#include <Box2D/Dynamics/Contacts/PositionConstraint.hpp>
#include <Box2D/Collision/Shapes/PolygonShape.hpp>
#include <Box2D/Collision/CollideShapes.hpp>

using namespace box2d;

static constexpr auto Baumgarte = RealNum{2} / RealNum{10};

TEST(ContactSolver, SolvePosConstraintsForHorTouchingDoesntMove)
{
	const auto old_pA = Position{Vec2{-2, 0}, 0_deg};
	const auto old_pB = Position{Vec2{+2, 0}, 0_deg};

	const auto dim = RealNum(2);
	const auto shape = PolygonShape(dim, dim);
	const auto xfmA = Transformation(old_pA.linear, UnitVec2{old_pA.angular});
	const auto xfmB = Transformation(old_pB.linear, UnitVec2{old_pB.angular});
	const auto manifold = CollideShapes(shape, xfmA, shape, xfmB);
	ASSERT_EQ(manifold.GetType(), Manifold::e_faceA);
	ASSERT_EQ(manifold.GetPointCount(), 2);

	const auto indexA = PositionConstraint::BodyData::index_type{0};
	const auto indexB = PositionConstraint::BodyData::index_type{1};
	const auto lcA = Vec2{0, 0};
	const auto lcB = Vec2{0, 0};
	const auto bA = PositionConstraint::BodyData{indexA, RealNum(1), RealNum(1), lcA};
	const auto bB = PositionConstraint::BodyData{indexB, RealNum(1), RealNum(1), lcB};
	const auto pc = PositionConstraint{manifold, bA, 0, bB, 0};
	
	const auto conf = ConstraintSolverConf{};
	const auto solution = SolvePositionConstraint(pc, old_pA, true, old_pB, true, conf);
	
	EXPECT_EQ(solution.min_separation, 0);
	
	EXPECT_EQ(old_pA.linear.x, solution.pos_a.linear.x);
	EXPECT_EQ(old_pA.linear.y, solution.pos_a.linear.y);
	EXPECT_EQ(old_pA.angular, solution.pos_a.angular);

	EXPECT_EQ(old_pB.linear.x, solution.pos_b.linear.x);
	EXPECT_EQ(old_pB.linear.y, solution.pos_b.linear.y);
	EXPECT_EQ(old_pB.angular, solution.pos_b.angular);
}

TEST(ContactSolver, SolvePosConstraintsForVerTouchingDoesntMove)
{
	const auto old_pA = Position{Vec2{0, -2}, 0_deg};
	const auto old_pB = Position{Vec2{0, +2}, 0_deg};
	
	const auto dim = RealNum(2);
	const auto shape = PolygonShape(dim, dim);
	const auto xfmA = Transformation(old_pA.linear, UnitVec2{old_pA.angular});
	const auto xfmB = Transformation(old_pB.linear, UnitVec2{old_pB.angular});
	const auto manifold = CollideShapes(shape, xfmA, shape, xfmB);
	ASSERT_EQ(manifold.GetType(), Manifold::e_faceA);
	ASSERT_EQ(manifold.GetPointCount(), 2);
	
	const auto indexA = PositionConstraint::BodyData::index_type{0};
	const auto indexB = PositionConstraint::BodyData::index_type{1};
	const auto lcA = Vec2{0, 0};
	const auto lcB = Vec2{0, 0};
	const auto bA = PositionConstraint::BodyData{indexA, RealNum(1), RealNum(1), lcA};
	const auto bB = PositionConstraint::BodyData{indexB, RealNum(1), RealNum(1), lcB};
	const auto pc = PositionConstraint{manifold, bA, 0, bB, 0};
	
	const auto conf = ConstraintSolverConf{};
	const auto solution = SolvePositionConstraint(pc, old_pA, true, old_pB, true, conf);
	
	EXPECT_EQ(solution.min_separation, 0);
	
	EXPECT_EQ(old_pA.linear.x, solution.pos_a.linear.x);
	EXPECT_EQ(old_pA.linear.y, solution.pos_a.linear.y);
	EXPECT_EQ(old_pA.angular, solution.pos_a.angular);
	
	EXPECT_EQ(old_pB.linear.x, solution.pos_b.linear.x);
	EXPECT_EQ(old_pB.linear.y, solution.pos_b.linear.y);
	EXPECT_EQ(old_pB.angular, solution.pos_b.angular);
}

TEST(ContactSolver, SolvePosConstraintsForOverlappingZeroRateDoesntMove)
{
	const auto dim = RealNum(2);
	const auto shape = PolygonShape(dim, dim);
	const auto xfmA = Transformation(Vec2_zero, UnitVec2{0_deg});
	const auto xfmB = Transformation(Vec2_zero, UnitVec2{0_deg});
	const auto manifold = CollideShapes(shape, xfmA, shape, xfmB);
	ASSERT_EQ(manifold.GetType(), Manifold::e_faceA);
	ASSERT_EQ(manifold.GetPointCount(), 2);
	
	const auto indexA = PositionConstraint::BodyData::index_type{0};
	const auto indexB = PositionConstraint::BodyData::index_type{1};
	const auto lcA = Vec2{};
	const auto lcB = Vec2{};
	const auto bA = PositionConstraint::BodyData{indexA, RealNum(1), RealNum(1), lcA};
	const auto bB = PositionConstraint::BodyData{indexB, RealNum(1), RealNum(1), lcB};
	const auto pc = PositionConstraint{manifold, bA, 0, bB, 0};

	const auto old_pA = Position{Vec2{0, 0}, 0_deg};
	const auto old_pB = Position{Vec2{0, 0}, 0_deg};
	
	const auto maxLinearCorrection = std::numeric_limits<RealNum>::infinity();
	const auto conf = ConstraintSolverConf{}.UseResolutionRate(0).UseMaxLinearCorrection(maxLinearCorrection);
	const auto solution = SolvePositionConstraint(pc, old_pA, true, old_pB, true, conf);

	EXPECT_EQ(solution.min_separation, -2 * dim);
	
	EXPECT_EQ(old_pA.linear.x, solution.pos_a.linear.x);
	EXPECT_EQ(old_pA.linear.y, solution.pos_a.linear.y);
	EXPECT_EQ(old_pA.angular, solution.pos_a.angular);
	
	EXPECT_EQ(old_pB.linear.x, solution.pos_b.linear.x);
	EXPECT_EQ(old_pB.linear.y, solution.pos_b.linear.y);
	EXPECT_EQ(old_pB.angular, solution.pos_b.angular);
}

TEST(ContactSolver, SolvePosConstraintsForHorOverlappingMovesHorOnly1)
{
	const auto ctr_x = RealNum(100);
	
	// square A is left of square B
	const auto old_pA = Position{{ctr_x - 1, 0}, 0_deg};
	const auto old_pB = Position{{ctr_x + 1, 0}, 0_deg};
	
	const auto dim = RealNum(2);
	const auto shape = PolygonShape(dim, dim);
	const auto xfmA = Transformation(old_pA.linear, UnitVec2{old_pA.angular});
	const auto xfmB = Transformation(old_pB.linear, UnitVec2{old_pB.angular});
	const auto manifold = CollideShapes(shape, xfmA, shape, xfmB);
	ASSERT_EQ(manifold.GetType(), Manifold::e_faceA);
	ASSERT_EQ(Vec2{manifold.GetLocalNormal()}, Vec2(+1, 0));
	ASSERT_EQ(manifold.GetLocalPoint(), Vec2(+2, 0));
	ASSERT_EQ(manifold.GetPointCount(), 2);
	ASSERT_EQ(manifold.GetPoint(0).localPoint, Vec2(-2, +2));
	ASSERT_EQ(manifold.GetPoint(1).localPoint, Vec2(-2, -2));
	
	const auto indexA = PositionConstraint::BodyData::index_type{0};
	const auto indexB = PositionConstraint::BodyData::index_type{1};
	const auto lcA = Vec2{0, 0};
	const auto lcB = Vec2{0, 0};
	const auto bA = PositionConstraint::BodyData{indexA, RealNum(1), RealNum(1), lcA};
	const auto bB = PositionConstraint::BodyData{indexB, RealNum(1), RealNum(1), lcB};
	const auto pc = PositionConstraint{manifold, bA, 0, bB, 0};
	
	const auto maxLinearCorrection = std::numeric_limits<RealNum>::infinity();
	const auto conf = ConstraintSolverConf{}.UseResolutionRate(Baumgarte).UseMaxLinearCorrection(maxLinearCorrection);
	const auto solution = SolvePositionConstraint(pc, old_pA, true, old_pB, true, conf);
	
	EXPECT_TRUE(almost_equal(solution.min_separation, RealNum(-2))); // -2.002398
		
	// object a just moves left
	EXPECT_LT(solution.pos_a.linear.x, old_pA.linear.x);
	EXPECT_EQ(solution.pos_a.linear.y, old_pA.linear.y);
	EXPECT_EQ(solution.pos_a.angular, old_pA.angular);
	
	// object b just moves right
	EXPECT_GT(solution.pos_b.linear.x, old_pB.linear.x);
	EXPECT_EQ(solution.pos_b.linear.y, old_pB.linear.y);
	EXPECT_EQ(solution.pos_b.angular, old_pB.angular);
}

TEST(ContactSolver, SolvePosConstraintsForHorOverlappingMovesHorOnly2)
{
	const auto ctr_x = RealNum(100);
	
	// square A is right of square B
	const auto old_pA =  Position{{ctr_x + 1, 0}, 0_deg};
	const auto old_pB = Position{{ctr_x - 1, 0}, 0_deg};
	
	const auto dim = RealNum(2);
	const auto shape = PolygonShape(dim, dim);
	const auto xfmA = Transformation(old_pA.linear, UnitVec2{old_pA.angular});
	const auto xfmB = Transformation(old_pB.linear, UnitVec2{old_pB.angular});
	const auto manifold = CollideShapes(shape, xfmA, shape, xfmB);
	ASSERT_EQ(manifold.GetType(), Manifold::e_faceA);
	ASSERT_EQ(Vec2{manifold.GetLocalNormal()}, Vec2(-1, 0));
	ASSERT_EQ(manifold.GetLocalPoint(), Vec2(-2, 0));
	ASSERT_EQ(manifold.GetPointCount(), 2);
	ASSERT_EQ(manifold.GetPoint(0).localPoint, Vec2(+2, -2));
	ASSERT_EQ(manifold.GetPoint(1).localPoint, Vec2(+2, +2));
	
	const auto indexA = PositionConstraint::BodyData::index_type{0};
	const auto indexB = PositionConstraint::BodyData::index_type{1};
	const auto lcA = Vec2{0, 0};
	const auto lcB = Vec2{0, 0};
	const auto bA = PositionConstraint::BodyData{indexA, RealNum(1), RealNum(1), lcA};
	const auto bB = PositionConstraint::BodyData{indexB, RealNum(1), RealNum(1), lcB};
	const auto pc = PositionConstraint{manifold, bA, 0, bB, 0};
	
	const auto maxLinearCorrection = std::numeric_limits<RealNum>::infinity();
	const auto conf = ConstraintSolverConf{}.UseResolutionRate(Baumgarte).UseMaxLinearCorrection(maxLinearCorrection);
	const auto solution = SolvePositionConstraint(pc, old_pA, true, old_pB, true, conf);
	
	EXPECT_TRUE(almost_equal(solution.min_separation, RealNum(-2))); // -2.002398
	
	// square A just moves right
	EXPECT_GT(solution.pos_a.linear.x, old_pA.linear.x);
	EXPECT_EQ(solution.pos_a.linear.y, old_pA.linear.y);
	EXPECT_EQ(solution.pos_a.angular, old_pA.angular);
	
	// square B just moves left
	EXPECT_LT(solution.pos_b.linear.x, old_pB.linear.x);
	EXPECT_EQ(solution.pos_b.linear.y, old_pB.linear.y);
	EXPECT_EQ(solution.pos_b.angular, old_pB.angular);
}

TEST(ContactSolver, SolvePosConstraintsForVerOverlappingMovesVerOnly1)
{
	const auto ctr_y = RealNum(100);
	
	// square A is below square B
	const auto old_pA = Position{{0, ctr_y - 1}, 0_deg};
	const auto old_pB = Position{{0, ctr_y + 1}, 0_deg};
	
	const auto dim = RealNum(2);
	const auto shape = PolygonShape(dim, dim);
	const auto xfmA = Transformation(old_pA.linear, UnitVec2{old_pA.angular});
	const auto xfmB = Transformation(old_pB.linear, UnitVec2{old_pB.angular});
	const auto manifold = CollideShapes(shape, xfmA, shape, xfmB);
	ASSERT_EQ(manifold.GetType(), Manifold::e_faceA);
	ASSERT_EQ(Vec2{manifold.GetLocalNormal()}, Vec2(0, 1));
	ASSERT_EQ(manifold.GetLocalPoint(), Vec2(0, 2));
	ASSERT_EQ(manifold.GetPointCount(), 2);
	ASSERT_EQ(manifold.GetPoint(0).localPoint, Vec2(-2, -2));
	ASSERT_EQ(manifold.GetPoint(1).localPoint, Vec2(+2, -2));
	
	const auto indexA = PositionConstraint::BodyData::index_type{0};
	const auto indexB = PositionConstraint::BodyData::index_type{1};
	const auto lcA = Vec2{0, 0};
	const auto lcB = Vec2{0, 0};
	const auto bA = PositionConstraint::BodyData{indexA, RealNum(1), RealNum(1), lcA};
	const auto bB = PositionConstraint::BodyData{indexB, RealNum(1), RealNum(1), lcB};
	const auto pc = PositionConstraint{manifold, bA, 0, bB, 0};
	
	const auto maxLinearCorrection = std::numeric_limits<RealNum>::infinity();
	const auto conf = ConstraintSolverConf{}.UseResolutionRate(Baumgarte).UseMaxLinearCorrection(maxLinearCorrection);
	const auto solution = SolvePositionConstraint(pc, old_pA, true, old_pB, true, conf);
	
	EXPECT_TRUE(almost_equal(solution.min_separation, RealNum(-2))); // -2.002398
	
	// object a just moves down only
	EXPECT_EQ(solution.pos_a.linear.x, old_pA.linear.x);
	EXPECT_LT(solution.pos_a.linear.y, old_pA.linear.y);
	EXPECT_EQ(solution.pos_a.angular, old_pA.angular);
	
	{
		// confirm object a moves more in x direction than in y direction.
		const auto mov_a = solution.pos_a - old_pA;	
		EXPECT_LT(Abs(mov_a.linear.x), Abs(mov_a.linear.y));
	}
	
	// object b just moves up only
	EXPECT_EQ(solution.pos_b.linear.x, old_pB.linear.x);
	EXPECT_GT(solution.pos_b.linear.y, old_pB.linear.y);
	EXPECT_EQ(solution.pos_b.angular, old_pB.angular);
	
	{
		// confirm object a moves more in x direction than in y direction.
		const auto mov_b = solution.pos_b - old_pB;
		EXPECT_LT(Abs(mov_b.linear.x), Abs(mov_b.linear.y));
	}
}

TEST(ContactSolver, SolvePosConstraintsForVerOverlappingMovesVerOnly2)
{
	const auto ctr_y = RealNum(100);

	// square A is above square B
	const auto old_pA = Position{{0, ctr_y + 1}, 0_deg};
	const auto old_pB = Position{{0, ctr_y - 1}, 0_deg};
	
	const auto dim = RealNum(2);
	const auto shape = PolygonShape(dim, dim);
	const auto xfmA = Transformation(old_pA.linear, UnitVec2{old_pA.angular});
	const auto xfmB = Transformation(old_pB.linear, UnitVec2{old_pB.angular});
	const auto manifold = CollideShapes(shape, xfmA, shape, xfmB);
	
	ASSERT_EQ(manifold.GetType(), Manifold::e_faceA);
	ASSERT_EQ(Vec2{manifold.GetLocalNormal()}, Vec2(0, -1));
	ASSERT_EQ(manifold.GetLocalPoint(), Vec2(0, -2));
	ASSERT_EQ(manifold.GetPointCount(), 2);
	ASSERT_EQ(manifold.GetPoint(0).localPoint, Vec2(+2, +2));
	ASSERT_EQ(manifold.GetPoint(1).localPoint, Vec2(-2, +2));
	
	const auto indexA = PositionConstraint::BodyData::index_type{0};
	const auto indexB = PositionConstraint::BodyData::index_type{1};
	const auto lcA = Vec2{0, 0};
	const auto lcB = Vec2{0, 0};
	const auto bA = PositionConstraint::BodyData{indexA, RealNum(1), RealNum(1), lcA};
	const auto bB = PositionConstraint::BodyData{indexB, RealNum(1), RealNum(1), lcB};
	const auto pc = PositionConstraint{manifold, bA, 0, bB, 0};

	const auto maxLinearCorrection = std::numeric_limits<RealNum>::infinity();
	const auto conf = ConstraintSolverConf{}.UseResolutionRate(Baumgarte).UseMaxLinearCorrection(maxLinearCorrection);
	const auto solution = SolvePositionConstraint(pc, old_pA, true, old_pB, true, conf);
	
	EXPECT_TRUE(almost_equal(solution.min_separation, RealNum(-2))); // -2.002398
	
	// square A just moves up only
	EXPECT_EQ(solution.pos_a.linear.x, old_pA.linear.x);
	EXPECT_GT(solution.pos_a.linear.y, old_pA.linear.y);
	EXPECT_EQ(solution.pos_a.angular, old_pA.angular);
	
	{
		// confirm object a moves more in x direction than in y direction.
		const auto mov_a = solution.pos_a - old_pA;	
		EXPECT_LT(Abs(mov_a.linear.x), Abs(mov_a.linear.y));
	}
	
	// square B just moves down only
	EXPECT_EQ(solution.pos_b.linear.x, old_pB.linear.x);
	EXPECT_LT(solution.pos_b.linear.y, old_pB.linear.y);
	EXPECT_EQ(solution.pos_b.angular, old_pB.angular);
	
	{
		// confirm object a moves more in x direction than in y direction.
		const auto mov_b = solution.pos_b - old_pB;
		EXPECT_LT(Abs(mov_b.linear.x), Abs(mov_b.linear.y));
	}
}

TEST(ContactSolver, SolvePosConstraintsForPerfectlyOverlappingSquares)
{
	const auto dim = RealNum(2);
	const auto shape = PolygonShape(dim, dim);
	const auto xfmA = Transformation(Vec2_zero, UnitVec2{0_deg});
	const auto xfmB = Transformation(Vec2_zero, UnitVec2{0_deg});
	const auto manifold = CollideShapes(shape, xfmA, shape, xfmB);
	ASSERT_EQ(manifold.GetType(), Manifold::e_faceA);
	ASSERT_EQ(manifold.GetPointCount(), 2);

	const auto indexA = PositionConstraint::BodyData::index_type{0};
	const auto indexB = PositionConstraint::BodyData::index_type{1};
	const auto lcA = Vec2{};
	const auto lcB = Vec2{};
	const auto bA = PositionConstraint::BodyData{indexA, RealNum(1), RealNum(1), lcA};
	const auto bB = PositionConstraint::BodyData{indexB, RealNum(1), RealNum(1), lcB};
	const auto pc = PositionConstraint{manifold, bA, GetVertexRadius(shape), bB, GetVertexRadius(shape)};

	const auto old_pA = Position{Vec2{0, 0}, 0_deg};
	const auto old_pB = Position{Vec2{0, 0}, 0_deg};

	const auto conf = ConstraintSolverConf{};
	const auto solution = SolvePositionConstraint(pc, old_pA, true, old_pB, true, conf);
	
	EXPECT_LT(solution.min_separation, -conf.linearSlop);
	
	// object a moves left only
	EXPECT_LT(solution.pos_a.linear.x, old_pA.linear.x);
	EXPECT_EQ(solution.pos_a.linear.y, old_pA.linear.y);
	EXPECT_EQ(solution.pos_a.angular, old_pA.angular);
	
	// object b moves right only.
	EXPECT_GT(solution.pos_b.linear.x, old_pB.linear.x);
	EXPECT_EQ(solution.pos_b.linear.y, old_pB.linear.y);
	EXPECT_EQ(solution.pos_b.angular, old_pB.angular);
}
