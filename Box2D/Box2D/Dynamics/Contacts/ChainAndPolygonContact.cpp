/*
* Original work Copyright (c) 2006-2010 Erin Catto http://www.box2d.org
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

#include <Box2D/Dynamics/Contacts/ChainAndPolygonContact.hpp>
#include <Box2D/Common/BlockAllocator.hpp>
#include <Box2D/Dynamics/Fixture.hpp>
#include <Box2D/Dynamics/Body.hpp>
#include <Box2D/Collision/CollideShapes.hpp>
#include <Box2D/Collision/Shapes/ChainShape.hpp>
#include <Box2D/Collision/Shapes/EdgeShape.hpp>
#include <Box2D/Collision/Shapes/PolygonShape.hpp>

#include <new>

using namespace box2d;

Contact* ChainAndPolygonContact::Create(Fixture* fixtureA, child_count_t indexA,
										Fixture* fixtureB, child_count_t indexB,
										BlockAllocator& allocator)
{
	void* mem = allocator.Allocate(sizeof(ChainAndPolygonContact));
	return new (mem) ChainAndPolygonContact(fixtureA, indexA, fixtureB, indexB);
}

void ChainAndPolygonContact::Destroy(Contact* contact, BlockAllocator& allocator)
{
	Delete(static_cast<ChainAndPolygonContact*>(contact), allocator);
}

ChainAndPolygonContact::ChainAndPolygonContact(Fixture* fixtureA, child_count_t indexA,
											   Fixture* fixtureB, child_count_t indexB)
: Contact{fixtureA, indexA, fixtureB, indexB}
{
	assert(GetType(*fixtureA) == Shape::e_chain);
	assert(GetType(*fixtureB) == Shape::e_polygon);
}

Manifold ChainAndPolygonContact::Evaluate() const
{
	const auto fixtureA = GetFixtureA();
	const auto fixtureB = GetFixtureB();
	const auto xfA = fixtureA->GetBody()->GetTransformation();
	const auto xfB = fixtureB->GetBody()->GetTransformation();
	const auto edge = static_cast<const ChainShape*>(fixtureA->GetShape())->GetChildEdge(GetChildIndexA());
	return CollideShapes(edge, xfA, *static_cast<const PolygonShape*>(fixtureB->GetShape()), xfB);
}
