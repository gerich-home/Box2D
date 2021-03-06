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

#ifndef PositionConstraint_hpp
#define PositionConstraint_hpp

#include <Box2D/Collision/Manifold.hpp>

namespace box2d {

	/// Contact Position Constraint.
	/// @note This structure is 108-bytes large on at least one 64-bit platform.
	struct PositionConstraint
	{
		using size_type = std::remove_const<decltype(MaxManifoldPoints)>::type;
		
		/// Position constraint body data.
		struct BodyData
		{
			using index_type = std::remove_const<decltype(MaxBodies)>::type;
			
			BodyData() noexcept = default;
			
			constexpr BodyData(index_type i, RealNum iM, RealNum iI, Vec2 lc) noexcept:
				index{i}, invMass{iM}, invI{iI}, localCenter{lc}
			{
				assert(iM >= 0);
				assert(iI >= 0);
			}
			
			index_type index; ///< Index within island of the associated body (2-bytes).
			RealNum invMass; ///< Inverse mass of associated body (a non-negative value, 4-bytes).
			RealNum invI; ///< Inverse rotational inertia about the center of mass of the associated body (a non-negative value, 4-bytes).
			Vec2 localCenter; ///< Local center of the associated body's sweep (8-bytes).
		};
		
		PositionConstraint() = default;
		
		PositionConstraint(const Manifold& m, const BodyData& bA, RealNum rA, const BodyData& bB, RealNum rB):
			manifold{m}, bodyA{bA}, radiusA{rA}, bodyB{bB}, radiusB{rB}
		{
			assert(m.GetPointCount() > 0);
			assert(bA.index != bB.index);
			assert(rA >= 0);
			assert(rB >= 0);
		}
		
		Manifold manifold; ///< Copy of contact's manifold with 1 or more contact points (60-bytes).
		
		BodyData bodyA; ///< Body A data (at least 18-bytes).
		
		RealNum radiusA; ///< "Radius" distance from the associated shape of fixture A (4-bytes). 0 or greater.
		
		BodyData bodyB; ///< Body A data (at least 18-bytes).
		
		RealNum radiusB; ///< "Radius" distance from the associated shape of fixture B (4-bytes). 0 or greater.
	};

} // namespace box2d

#endif /* PositionConstraint_hpp */
