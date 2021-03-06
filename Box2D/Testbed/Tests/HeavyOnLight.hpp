/*
 * Original work Copyright (c) 2008-2014 Erin Catto http://www.box2d.org
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

#ifndef HEAVY_ON_LIGHT_H
#define HEAVY_ON_LIGHT_H

namespace box2d {

class HeavyOnLight : public Test
{
public:
    
	HeavyOnLight()
	{
		{
			const auto ground = m_world->CreateBody();
			ground->CreateFixture(std::make_shared<EdgeShape>(Vec2(-40.0f, 0.0f), Vec2(40.0f, 0.0f)));
		}
        
		BodyDef bd;
		bd.type = BodyType::Dynamic;
		bd.position = Vec2(0.0f, 0.5f);
		const auto body1 = m_world->CreateBody(bd);
		body1->CreateFixture(std::make_shared<CircleShape>(0.5f), FixtureDef{}.UseDensity(10));
        
        bd.position = Vec2(0.0f, 6.0f);
        const auto body2 = m_world->CreateBody(bd);
		body2->CreateFixture(std::make_shared<CircleShape>(5.0f), FixtureDef{}.UseDensity(10));
	}
    
	static Test* Create()
	{
		return new HeavyOnLight;
	}
};

} // namespace box2d

#endif
