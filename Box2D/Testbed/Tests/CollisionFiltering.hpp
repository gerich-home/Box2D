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

#ifndef COLLISION_FILTERING_H
#define COLLISION_FILTERING_H

namespace box2d {

// This is a test of collision filtering.
// There is a triangle, a box, and a circle.
// There are 6 shapes. 3 large and 3 small.
// The 3 small ones always collide.
// The 3 large ones never collide.
// The boxes don't collide with triangles (except if both are small).
const int16	k_smallGroup = 1;
const int16 k_largeGroup = -1;

const uint16 k_defaultCategory = 0x0001;
const uint16 k_triangleCategory = 0x0002;
const uint16 k_boxCategory = 0x0004;
const uint16 k_circleCategory = 0x0008;

const uint16 k_triangleMask = 0xFFFF;
const uint16 k_boxMask = 0xFFFF ^ k_triangleCategory;
const uint16 k_circleMask = 0xFFFF;

class CollisionFiltering : public Test
{
public:
	CollisionFiltering()
	{
		// Ground body
		{
			FixtureDef sd;
			sd.friction = 0.3f;

			BodyDef bd;
			const auto ground = m_world->CreateBody(bd);
			ground->CreateFixture(std::make_shared<EdgeShape>(Vec2(-40.0f, 0.0f), Vec2(40.0f, 0.0f)), sd);
		}

		// Small triangle
		Vec2 vertices[3];
		vertices[0] = Vec2(-1.0f, 0.0f);
		vertices[1] = Vec2(1.0f, 0.0f);
		vertices[2] = Vec2(0.0f, 2.0f);
		PolygonShape polygon;
		polygon.Set(Span<const Vec2>{vertices, 3});

		FixtureDef triangleShapeDef;
		triangleShapeDef.density = 1.0f;

		triangleShapeDef.filter.groupIndex = k_smallGroup;
		triangleShapeDef.filter.categoryBits = k_triangleCategory;
		triangleShapeDef.filter.maskBits = k_triangleMask;

		BodyDef triangleBodyDef;
		triangleBodyDef.type = BodyType::Dynamic;
		triangleBodyDef.position = Vec2(-5.0f, 2.0f);

		const auto body1 = m_world->CreateBody(triangleBodyDef);
		body1->CreateFixture(std::make_shared<PolygonShape>(polygon), triangleShapeDef);

		// Large triangle (recycle definitions)
		vertices[0] *= 2.0f;
		vertices[1] *= 2.0f;
		vertices[2] *= 2.0f;
		polygon.Set(Span<const Vec2>{vertices, 3});
		triangleShapeDef.filter.groupIndex = k_largeGroup;
		triangleBodyDef.position = Vec2(-5.0f, 6.0f);
		triangleBodyDef.fixedRotation = true; // look at me!

		const auto body2 = m_world->CreateBody(triangleBodyDef);
		body2->CreateFixture(std::make_shared<PolygonShape>(polygon), triangleShapeDef);

		{
			BodyDef bd;
			bd.type = BodyType::Dynamic;
			bd.position = Vec2(-5.0f, 10.0f);
			const auto body = m_world->CreateBody(bd);
			body->CreateFixture(std::make_shared<PolygonShape>(0.5f, 1.0f), FixtureDef().UseDensity(1));

			PrismaticJointDef jd;
			jd.bodyA = body2;
			jd.bodyB = body;
			jd.enableLimit = true;
			jd.localAnchorA = Vec2(0.0f, 4.0f);
			jd.localAnchorB = Vec2_zero;
			jd.localAxisA = Vec2(0.0f, 1.0f);
			jd.lowerTranslation = -1.0f;
			jd.upperTranslation = 1.0f;

			m_world->CreateJoint(jd);
		}

		// Small box
		polygon.SetAsBox(1.0f, 0.5f);
		FixtureDef boxShapeDef;
		boxShapeDef.density = 1.0f;
		boxShapeDef.restitution = 0.1f;

		boxShapeDef.filter.groupIndex = k_smallGroup;
		boxShapeDef.filter.categoryBits = k_boxCategory;
		boxShapeDef.filter.maskBits = k_boxMask;

		BodyDef boxBodyDef;
		boxBodyDef.type = BodyType::Dynamic;
		boxBodyDef.position = Vec2(0.0f, 2.0f);

		const auto body3 = m_world->CreateBody(boxBodyDef);
		body3->CreateFixture(std::make_shared<PolygonShape>(polygon), boxShapeDef);

		// Large box (recycle definitions)
		polygon.SetAsBox(2.0f, 1.0f);
		boxShapeDef.filter.groupIndex = k_largeGroup;
		boxBodyDef.position = Vec2(0.0f, 6.0f);

		const auto body4 = m_world->CreateBody(boxBodyDef);
		body4->CreateFixture(std::make_shared<PolygonShape>(polygon), boxShapeDef);

		// Small circle
		auto circle = CircleShape(1);

		FixtureDef circleShapeDef;
		circleShapeDef.density = 1.0f;

		circleShapeDef.filter.groupIndex = k_smallGroup;
		circleShapeDef.filter.categoryBits = k_circleCategory;
		circleShapeDef.filter.maskBits = k_circleMask;

		BodyDef circleBodyDef;
		circleBodyDef.type = BodyType::Dynamic;
		circleBodyDef.position = Vec2(5.0f, 2.0f);
		
		const auto body5 = m_world->CreateBody(circleBodyDef);
		body5->CreateFixture(std::make_shared<CircleShape>(circle), circleShapeDef);

		// Large circle
		circle.SetRadius(circle.GetRadius() * 2);
		circleShapeDef.filter.groupIndex = k_largeGroup;
		circleBodyDef.position = Vec2(5.0f, 6.0f);

		const auto body6 = m_world->CreateBody(circleBodyDef);
		body6->CreateFixture(std::make_shared<CircleShape>(circle), circleShapeDef);
	}

	static Test* Create()
	{
		return new CollisionFiltering;
	}
};
	
} // namespace box2d

#endif
