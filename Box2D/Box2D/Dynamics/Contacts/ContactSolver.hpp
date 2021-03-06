/*
 * Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2016 Louis Langholtz https://github.com/louis-langholtz/Box2D
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

#ifndef B2_CONTACT_SOLVER_H
#define B2_CONTACT_SOLVER_H

#include <Box2D/Common/Math.hpp>
#include <Box2D/Common/Span.hpp>

namespace box2d {

	class VelocityConstraint;
	class PositionConstraint;
	
	struct PositionSolution
	{
		Position pos_a;
		Position pos_b;
		RealNum min_separation;
	};

	inline PositionSolution operator+ (PositionSolution lhs, PositionSolution rhs)
	{
		return PositionSolution{lhs.pos_a + rhs.pos_a, lhs.pos_b + rhs.pos_b, lhs.min_separation + rhs.min_separation};
	}

	inline PositionSolution operator- (PositionSolution lhs, PositionSolution rhs)
	{
		return PositionSolution{lhs.pos_a - rhs.pos_a, lhs.pos_b - rhs.pos_b, lhs.min_separation - rhs.min_separation};
	}

	/// Constraint solver configuration data.
	/// @detail
	/// Defines how a constraint solver should resolve a given constraint.
	/// @sa SolvePositionConstraint.
	struct ConstraintSolverConf
	{
		ConstraintSolverConf& UseResolutionRate(RealNum value) noexcept;

		ConstraintSolverConf& UseLinearSlop(RealNum value) noexcept;
		
		ConstraintSolverConf& UseAngularSlop(RealNum value) noexcept;

		ConstraintSolverConf& UseMaxLinearCorrection(RealNum value) noexcept;

		ConstraintSolverConf& UseMaxAngularCorrection(RealNum value) noexcept;

		/// Resolution rate.
		/// @detail
		/// Defines the percentage of the overlap that should get resolved in a single solver call.
		/// Value greater than zero and less than or equal to one.
		/// Ideally this would be 1 so that overlap is removed in one time step.
		/// However using values close to 1 often leads to overshoot.
		/// @note Recommended values are: <code>0.2</code> for solving regular constraints
		///   or <code>0.75</code> for solving TOI constraints.
		RealNum resolutionRate = RealNum(0.2);

		/// Linear slop.
		/// @note The negative of this amount is the maximum amount of separation to create.
		/// @note Recommended value: <code>LinearSlop</code>.
		RealNum linearSlop = LinearSlop;

		/// Angular slop.
		/// @note Recommended value: <code>AngularSlop</code>.
		RealNum angularSlop = AngularSlop;

		/// Maximum linear correction.
		/// @detail
		/// Maximum amount of overlap to resolve in a single solver call. Helps prevent overshoot.
		/// @note Recommended value: <code>linearSlop * 40</code>.
		RealNum maxLinearCorrection = linearSlop * 20;
		
		/// Maximum angular correction.
		/// @detail Maximum angular position correction used when solving constraints.
		/// Helps to prevent overshoot.
		/// @note Recommended value: <code>angularSlop * 4</code>.
		RealNum maxAngularCorrection = angularSlop * 4;
	};

	inline ConstraintSolverConf& ConstraintSolverConf::UseResolutionRate(RealNum value) noexcept
	{
		resolutionRate = value;
		return *this;
	}

	inline ConstraintSolverConf& ConstraintSolverConf::UseLinearSlop(RealNum value) noexcept
	{
		linearSlop = value;
		return *this;
	}

	inline ConstraintSolverConf& ConstraintSolverConf::UseAngularSlop(RealNum value) noexcept
	{
		angularSlop = value;
		return *this;
	}

	inline ConstraintSolverConf& ConstraintSolverConf::UseMaxLinearCorrection(RealNum value) noexcept
	{
		maxLinearCorrection = value;
		return *this;
	}

	inline ConstraintSolverConf& ConstraintSolverConf::UseMaxAngularCorrection(RealNum value) noexcept
	{
		maxAngularCorrection = value;
		return *this;
	}

	/// Solves the given position constraint.
	/// @detail
	/// This pushes apart the two given positions for every point in the contact position constraint
	/// and returns the minimum separation value from the position solver manifold for each point.
	/// @sa http://allenchou.net/2013/12/game-physics-resolution-contact-constraints/
	/// @return Minimum separation distance of the position constraint's manifold points
	///   (prior to "solving").
	PositionSolution SolvePositionConstraint(const PositionConstraint& pc,
											 Position positionA, bool moveA,
											 Position positionB, bool moveB,
											 ConstraintSolverConf conf);
	
	inline ConstraintSolverConf GetDefaultPositionSolverConf()
	{
		return ConstraintSolverConf{}.UseResolutionRate(RealNum(0.2));
	}
	
	/// Solves the given position constraints.
	/// @detail This updates positions (and nothing else) by calling the position constraint solving function.
	/// @note Can't expect the returned minimum separation to be greater than or equal to
	///  <code>-conf.linearSlop</code> because code won't push the separation above this
	///   amount to begin with.
	/// @return Minimum separation.
	/// @sa MinSeparationThreshold.
	/// @sa Solve.
	RealNum SolvePositionConstraints(Span<const PositionConstraint> positionConstraints,
									 Span<Position> positions,
									 ConstraintSolverConf conf = GetDefaultPositionSolverConf());
	
	inline ConstraintSolverConf GetDefaultToiPositionSolverConf()
	{
		// For solving TOI events, use a faster/higher resolution rate than normally used.
		return ConstraintSolverConf{}.UseResolutionRate(RealNum(0.75));
	}

	/// Solves the given position constraints.
	/// @detail This updates positions (and nothing else) for the two bodies identified by the
	///   given indexes by calling the position constraint solving function.
	/// @param indexA Index within the island of body A.
	/// @param indexB Index within the island of body B.
	/// @note Can't expect the returned minimum separation to be greater than or equal to
	///  ConstraintSolverConf.max_separation because code won't push the separation above this
	///   amount to begin with.
	/// @return Minimum separation.
	RealNum SolvePositionConstraints(Span<const PositionConstraint> positionConstraints,
									 Span<Position> positions,
									 island_count_t indexA, island_count_t indexB,
									 ConstraintSolverConf conf = GetDefaultToiPositionSolverConf());

	/// Solves the velocity constraint.
	/// @detail This updates the tangent and normal impulses of the velocity constraint points of the given velocity
	///   constraint and updates the given velocities.
	/// @pre The velocity constraint must have a valid normal, a valid tangent,
	///   valid point relative positions, and valid velocity biases.
	void SolveVelocityConstraint(VelocityConstraint& vc, Velocity& velA, Velocity& velB);
	
} // namespace box2d

#endif

