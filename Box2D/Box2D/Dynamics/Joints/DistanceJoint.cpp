/*
* Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
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

#include <Box2D/Dynamics/Joints/DistanceJoint.h>
#include <Box2D/Dynamics/Body.h>
#include <Box2D/Dynamics/TimeStep.h>

using namespace box2d;

// 1-D constrained system
// m (v2 - v1) = lambda
// v2 + (beta/h) * x1 + gamma * lambda = 0, gamma has units of inverse mass.
// x2 = x1 + h * v2

// 1-D mass-damper-spring system
// m (v2 - v1) + h * d * v2 + h * k * 

// C = norm(p2 - p1) - L
// u = (p2 - p1) / norm(p2 - p1)
// Cdot = dot(u, v2 + cross(w2, r2) - v1 - cross(w1, r1))
// J = [-u -cross(r1, u) u cross(r2, u)]
// K = J * invM * JT
//   = invMass1 + invI1 * cross(r1, u)^2 + invMass2 + invI2 * cross(r2, u)^2

void DistanceJointDef::Initialize(Body* b1, Body* b2,
									const Vec2& anchor1, const Vec2& anchor2)
{
	bodyA = b1;
	bodyB = b2;
	localAnchorA = bodyA->GetLocalPoint(anchor1);
	localAnchorB = bodyB->GetLocalPoint(anchor2);
	const auto d = anchor2 - anchor1;
	length = d.Length();
}

DistanceJoint::DistanceJoint(const DistanceJointDef* def)
: Joint(def)
{
	m_localAnchorA = def->localAnchorA;
	m_localAnchorB = def->localAnchorB;
	m_length = def->length;
	m_frequencyHz = def->frequencyHz;
	m_dampingRatio = def->dampingRatio;
}

void DistanceJoint::InitVelocityConstraints(const SolverData& data)
{
	m_indexA = m_bodyA->m_islandIndex;
	m_indexB = m_bodyB->m_islandIndex;
	m_localCenterA = m_bodyA->m_sweep.localCenter;
	m_localCenterB = m_bodyB->m_sweep.localCenter;
	m_invMassA = m_bodyA->m_invMass;
	m_invMassB = m_bodyB->m_invMass;
	m_invIA = m_bodyA->m_invI;
	m_invIB = m_bodyB->m_invI;

	const auto cA = data.positions[m_indexA].c;
	const auto aA = data.positions[m_indexA].a;
	auto vA = data.velocities[m_indexA].v;
	auto wA = data.velocities[m_indexA].w;

	const auto cB = data.positions[m_indexB].c;
	const auto aB = data.positions[m_indexB].a;
	auto vB = data.velocities[m_indexB].v;
	auto wB = data.velocities[m_indexB].w;

	const Rot qA(aA), qB(aB);

	m_rA = Mul(qA, m_localAnchorA - m_localCenterA);
	m_rB = Mul(qB, m_localAnchorB - m_localCenterB);
	m_u = (cB + m_rB) - (cA + m_rA);

	// Handle singularity.
	const auto length = m_u.Length();
	if (length > LinearSlop)
	{
		m_u *= float_t(1) / length;
	}
	else
	{
		m_u = Vec2_zero;
	}

	const auto crAu = Cross(m_rA, m_u);
	const auto crBu = Cross(m_rB, m_u);
	auto invMass = m_invMassA + m_invIA * Square(crAu) + m_invMassB + m_invIB * Square(crBu);

	// Compute the effective mass matrix.
	m_mass = (invMass != float_t{0}) ? float_t(1) / invMass : float_t{0};

	if (m_frequencyHz > float_t{0})
	{
		const auto C = length - m_length;

		// Frequency
		const auto omega = float_t(2) * Pi * m_frequencyHz;

		// Damping coefficient
		const auto d = float_t(2) * m_mass * m_dampingRatio * omega;

		// Spring stiffness
		const auto k = m_mass * Square(omega);

		// magic formulas
		const auto h = data.step.get_dt();
		m_gamma = h * (d + h * k);
		m_gamma = (m_gamma != float_t{0}) ? float_t(1) / m_gamma : float_t{0};
		m_bias = C * h * k * m_gamma;

		invMass += m_gamma;
		m_mass = (invMass != float_t{0}) ? float_t(1) / invMass : float_t{0};
	}
	else
	{
		m_gamma = float_t{0};
		m_bias = float_t{0};
	}

	if (data.step.warmStarting)
	{
		// Scale the impulse to support a variable time step.
		m_impulse *= data.step.dtRatio;

		const auto P = m_impulse * m_u;
		vA -= m_invMassA * P;
		wA -= m_invIA * Cross(m_rA, P);
		vB += m_invMassB * P;
		wB += m_invIB * Cross(m_rB, P);
	}
	else
	{
		m_impulse = float_t{0};
	}

	data.velocities[m_indexA].v = vA;
	data.velocities[m_indexA].w = wA;
	data.velocities[m_indexB].v = vB;
	data.velocities[m_indexB].w = wB;
}

void DistanceJoint::SolveVelocityConstraints(const SolverData& data)
{
	auto vA = data.velocities[m_indexA].v;
	auto wA = data.velocities[m_indexA].w;
	auto vB = data.velocities[m_indexB].v;
	auto wB = data.velocities[m_indexB].w;

	// Cdot = dot(u, v + cross(w, r))
	const auto vpA = vA + Cross(wA, m_rA);
	const auto vpB = vB + Cross(wB, m_rB);
	const auto Cdot = Dot(m_u, vpB - vpA);

	const auto impulse = -m_mass * (Cdot + m_bias + m_gamma * m_impulse);
	m_impulse += impulse;

	const auto P = impulse * m_u;
	vA -= m_invMassA * P;
	wA -= m_invIA * Cross(m_rA, P);
	vB += m_invMassB * P;
	wB += m_invIB * Cross(m_rB, P);

	data.velocities[m_indexA].v = vA;
	data.velocities[m_indexA].w = wA;
	data.velocities[m_indexB].v = vB;
	data.velocities[m_indexB].w = wB;
}

bool DistanceJoint::SolvePositionConstraints(const SolverData& data)
{
	if (m_frequencyHz > float_t{0})
	{
		// There is no position correction for soft distance constraints.
		return true;
	}

	auto cA = data.positions[m_indexA].c;
	auto aA = data.positions[m_indexA].a;
	auto cB = data.positions[m_indexB].c;
	auto aB = data.positions[m_indexB].a;

	const auto qA = Rot(aA);
	const auto qB = Rot(aB);

	const auto rA = Mul(qA, m_localAnchorA - m_localCenterA);
	const auto rB = Mul(qB, m_localAnchorB - m_localCenterB);
	auto u = cB + rB - cA - rA;

	const auto length = u.Normalize();
	auto C = length - m_length;
	C = Clamp(C, -MaxLinearCorrection, MaxLinearCorrection);

	const auto impulse = -m_mass * C;
	const auto P = impulse * u;

	cA -= m_invMassA * P;
	aA -= m_invIA * Cross(rA, P);
	cB += m_invMassB * P;
	aB += m_invIB * Cross(rB, P);

	data.positions[m_indexA].c = cA;
	data.positions[m_indexA].a = aA;
	data.positions[m_indexB].c = cB;
	data.positions[m_indexB].a = aB;

	return Abs(C) < LinearSlop;
}

Vec2 DistanceJoint::GetAnchorA() const
{
	return m_bodyA->GetWorldPoint(m_localAnchorA);
}

Vec2 DistanceJoint::GetAnchorB() const
{
	return m_bodyB->GetWorldPoint(m_localAnchorB);
}

Vec2 DistanceJoint::GetReactionForce(float_t inv_dt) const
{
	return (inv_dt * m_impulse) * m_u;
}

float_t DistanceJoint::GetReactionTorque(float_t inv_dt) const
{
	BOX2D_NOT_USED(inv_dt);
	return float_t{0};
}

void DistanceJoint::Dump()
{
	const auto indexA = m_bodyA->m_islandIndex;
	const auto indexB = m_bodyB->m_islandIndex;

	log("  DistanceJointDef jd;\n");
	log("  jd.bodyA = bodies[%d];\n", indexA);
	log("  jd.bodyB = bodies[%d];\n", indexB);
	log("  jd.collideConnected = bool(%d);\n", m_collideConnected);
	log("  jd.localAnchorA = Vec2(%.15lef, %.15lef);\n", m_localAnchorA.x, m_localAnchorA.y);
	log("  jd.localAnchorB = Vec2(%.15lef, %.15lef);\n", m_localAnchorB.x, m_localAnchorB.y);
	log("  jd.length = %.15lef;\n", m_length);
	log("  jd.frequencyHz = %.15lef;\n", m_frequencyHz);
	log("  jd.dampingRatio = %.15lef;\n", m_dampingRatio);
	log("  joints[%d] = m_world->CreateJoint(&jd);\n", m_index);
}