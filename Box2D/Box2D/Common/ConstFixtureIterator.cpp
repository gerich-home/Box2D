//
//  ConstFixtureIterator.cpp
//  Box2D
//
//  Created by Louis D. Langholtz on 3/7/16.
//
//

#include <Box2D/Common/ConstFixtureIterator.hpp>
#include <Box2D/Dynamics/Fixture.h>

using namespace box2d;

ConstFixtureIterator::pointer const * ConstFixtureIterator::next(pointer const * q) const noexcept
{
	return &((*q)->m_next);
}