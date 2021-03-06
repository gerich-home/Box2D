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

#include <Box2D/Dynamics/WorldCallbacks.hpp>
#include <Box2D/Dynamics/Fixture.hpp>

using namespace box2d;

// Return true if contact calculations should be performed between these two shapes.
// If you implement your own collision filter you may want to build from this implementation.
bool ContactFilter::ShouldCollide(Fixture* fixtureA, Fixture* fixtureB)
{
	const auto& filterA = fixtureA->GetFilterData();
	const auto& filterB = fixtureB->GetFilterData();

	if ((filterA.groupIndex == filterB.groupIndex) && (filterA.groupIndex != 0))
	{
		return filterA.groupIndex > 0;
	}

	return ((filterA.maskBits & filterB.categoryBits) != 0) && ((filterA.categoryBits & filterB.maskBits) != 0);
}
