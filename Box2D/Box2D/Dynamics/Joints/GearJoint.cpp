/*
* Original work Copyright (c) 2007-2011 Erin Catto http://www.box2d.org
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

#include <Box2D/Dynamics/Joints/GearJoint.hpp>
#include <Box2D/Dynamics/Joints/RevoluteJoint.hpp>
#include <Box2D/Dynamics/Joints/PrismaticJoint.hpp>
#include <Box2D/Dynamics/Body.hpp>
#include <Box2D/Dynamics/StepConf.hpp>
#include <Box2D/Dynamics/Contacts/ContactSolver.hpp>

using namespace box2d;

// Gear Joint:
// C0 = (coordinate1 + ratio * coordinate2)_initial
// C = (coordinate1 + ratio * coordinate2) - C0 = 0
// J = [J1 ratio * J2]
// K = J * invM * JT
//   = J1 * invM1 * J1T + ratio * ratio * J2 * invM2 * J2T
//
// Revolute:
// coordinate = rotation
// Cdot = angularVelocity
// J = [0 0 1]
// K = J * invM * JT = invI
//
// Prismatic:
// coordinate = dot(p - pg, ug)
// Cdot = dot(v + cross(w, r), ug)
// J = [ug cross(r, ug)]
// K = J * invM * JT = invMass + invI * cross(r, ug)^2

GearJoint::GearJoint(const GearJointDef& def)
: Joint(def)
{
	m_joint1 = def.joint1;
	m_joint2 = def.joint2;

	m_typeA = m_joint1->GetType();
	m_typeB = m_joint2->GetType();

	assert(m_typeA == JointType::Revolute || m_typeA == JointType::Prismatic);
	assert(m_typeB == JointType::Revolute || m_typeB == JointType::Prismatic);

	Angle coordinateA, coordinateB;

	// TODO_ERIN there might be some problem with the joint edges in Joint.

	m_bodyC = m_joint1->GetBodyA();
	SetBodyA(m_joint1->GetBodyB());

	// Get geometry of joint1
	const auto xfA = GetBodyA()->GetTransformation();
	const auto aA = GetBodyA()->GetAngle();
	const auto xfC = m_bodyC->GetTransformation();
	const auto aC = m_bodyC->GetAngle();

	if (m_typeA == JointType::Revolute)
	{
		const auto revolute = static_cast<RevoluteJoint*>(def.joint1);
		m_localAnchorC = revolute->GetLocalAnchorA();
		m_localAnchorA = revolute->GetLocalAnchorB();
		m_referenceAngleA = revolute->GetReferenceAngle();
		m_localAxisC = UnitVec2::GetZero();

		coordinateA = aA - aC - m_referenceAngleA;
	}
	else
	{
		const auto prismatic = static_cast<PrismaticJoint*>(def.joint1);
		m_localAnchorC = prismatic->GetLocalAnchorA();
		m_localAnchorA = prismatic->GetLocalAnchorB();
		m_referenceAngleA = prismatic->GetReferenceAngle();
		m_localAxisC = prismatic->GetLocalAxisA();

		const auto pC = m_localAnchorC;
		const auto pA = InverseRotate(Rotate(m_localAnchorA, xfA.q) + (xfA.p - xfC.p), xfC.q);
		coordinateA = Dot(pA - pC, m_localAxisC) * 1_rad;
	}

	m_bodyD = m_joint2->GetBodyA();
	SetBodyB(m_joint2->GetBodyB());

	// Get geometry of joint2
	const auto xfB = GetBodyB()->GetTransformation();
	const auto aB = GetBodyB()->GetAngle();
	const auto xfD = m_bodyD->GetTransformation();
	const auto aD = m_bodyD->GetAngle();

	if (m_typeB == JointType::Revolute)
	{
		const auto revolute = static_cast<RevoluteJoint*>(def.joint2);
		m_localAnchorD = revolute->GetLocalAnchorA();
		m_localAnchorB = revolute->GetLocalAnchorB();
		m_referenceAngleB = revolute->GetReferenceAngle();
		m_localAxisD = UnitVec2::GetZero();

		coordinateB = aB - aD - m_referenceAngleB;
	}
	else
	{
		const auto prismatic = static_cast<PrismaticJoint*>(def.joint2);
		m_localAnchorD = prismatic->GetLocalAnchorA();
		m_localAnchorB = prismatic->GetLocalAnchorB();
		m_referenceAngleB = prismatic->GetReferenceAngle();
		m_localAxisD = prismatic->GetLocalAxisA();

		const auto pD = m_localAnchorD;
		const auto pB = InverseRotate(Rotate(m_localAnchorB, xfB.q) + (xfB.p - xfD.p), xfD.q);
		coordinateB = Dot(pB - pD, m_localAxisD) * 1_rad;
	}

	m_ratio = def.ratio;

	m_constant = coordinateA + m_ratio * coordinateB;
}

void GearJoint::InitVelocityConstraints(Span<Velocity> velocities, Span<const Position> positions, const StepConf& step, const ConstraintSolverConf& conf)
{
	m_indexA = GetBodyA()->GetIslandIndex();
	m_indexB = GetBodyB()->GetIslandIndex();
	m_indexC = m_bodyC->GetIslandIndex();
	m_indexD = m_bodyD->GetIslandIndex();
	m_lcA = GetBodyA()->GetLocalCenter();
	m_lcB = GetBodyB()->GetLocalCenter();
	m_lcC = m_bodyC->GetLocalCenter();
	m_lcD = m_bodyD->GetLocalCenter();
	m_mA = GetBodyA()->GetInverseMass();
	m_mB = GetBodyB()->GetInverseMass();
	m_mC = m_bodyC->GetInverseMass();
	m_mD = m_bodyD->GetInverseMass();
	m_iA = GetBodyA()->GetInverseInertia();
	m_iB = GetBodyB()->GetInverseInertia();
	m_iC = m_bodyC->GetInverseInertia();
	m_iD = m_bodyD->GetInverseInertia();

	const auto aA = positions[m_indexA].angular;
	auto vA = velocities[m_indexA].linear;
	auto wA = velocities[m_indexA].angular;

	const auto aB = positions[m_indexB].angular;
	auto vB = velocities[m_indexB].linear;
	auto wB = velocities[m_indexB].angular;

	const auto aC = positions[m_indexC].angular;
	auto vC = velocities[m_indexC].linear;
	auto wC = velocities[m_indexC].angular;

	const auto aD = positions[m_indexD].angular;
	auto vD = velocities[m_indexD].linear;
	auto wD = velocities[m_indexD].angular;

	const auto qA = UnitVec2(aA);
	const auto qB = UnitVec2(aB);
	const auto qC = UnitVec2(aC);
	const auto qD = UnitVec2(aD);

	m_mass = RealNum{0};

	if (m_typeA == JointType::Revolute)
	{
		m_JvAC = Vec2_zero;
		m_JwA = RealNum{1};
		m_JwC = RealNum{1};
		m_mass += m_iA + m_iC;
	}
	else
	{
		const auto u = Rotate(m_localAxisC, qC);
		const auto rC = Rotate(m_localAnchorC - m_lcC, qC);
		const auto rA = Rotate(m_localAnchorA - m_lcA, qA);
		m_JvAC = u * 1;
		m_JwC = Cross(rC, u);
		m_JwA = Cross(rA, u);
		m_mass += m_mC + m_mA + m_iC * Square(m_JwC) + m_iA * Square(m_JwA);
	}

	if (m_typeB == JointType::Revolute)
	{
		m_JvBD = Vec2_zero;
		m_JwB = m_ratio;
		m_JwD = m_ratio;
		m_mass += Square(m_ratio) * (m_iB + m_iD);
	}
	else
	{
		const auto u = Rotate(m_localAxisD, qD);
		const auto rD = Rotate(m_localAnchorD - m_lcD, qD);
		const auto rB = Rotate(m_localAnchorB - m_lcB, qB);
		m_JvBD = m_ratio * u;
		m_JwD = m_ratio * Cross(rD, u);
		m_JwB = m_ratio * Cross(rB, u);
		m_mass += Square(m_ratio) * (m_mD + m_mB) + m_iD * Square(m_JwD) + m_iB * Square(m_JwB);
	}

	// Compute effective mass.
	m_mass = (m_mass > RealNum{0}) ? RealNum{1} / m_mass : RealNum{0};

	if (step.doWarmStart)
	{
		vA += (m_mA * m_impulse) * m_JvAC;
		wA += 1_rad * m_iA * m_impulse * m_JwA;
		vB += (m_mB * m_impulse) * m_JvBD;
		wB += 1_rad * m_iB * m_impulse * m_JwB;
		vC -= (m_mC * m_impulse) * m_JvAC;
		wC -= 1_rad * m_iC * m_impulse * m_JwC;
		vD -= (m_mD * m_impulse) * m_JvBD;
		wD -= 1_rad * m_iD * m_impulse * m_JwD;
	}
	else
	{
		m_impulse = RealNum{0};
	}

	velocities[m_indexA].linear = vA;
	velocities[m_indexA].angular = wA;
	velocities[m_indexB].linear = vB;
	velocities[m_indexB].angular = wB;
	velocities[m_indexC].linear = vC;
	velocities[m_indexC].angular = wC;
	velocities[m_indexD].linear = vD;
	velocities[m_indexD].angular = wD;
}

void GearJoint::SolveVelocityConstraints(Span<Velocity> velocities, const StepConf& step)
{
	auto vA = velocities[m_indexA].linear;
	auto wA = velocities[m_indexA].angular;
	auto vB = velocities[m_indexB].linear;
	auto wB = velocities[m_indexB].angular;
	auto vC = velocities[m_indexC].linear;
	auto wC = velocities[m_indexC].angular;
	auto vD = velocities[m_indexD].linear;
	auto wD = velocities[m_indexD].angular;

	auto Cdot = Dot(m_JvAC, vA - vC) + Dot(m_JvBD, vB - vD);
	Cdot += (m_JwA * wA.ToRadians() - m_JwC * wC.ToRadians()) + (m_JwB * wB.ToRadians() - m_JwD * wD.ToRadians());

	const auto impulse = -m_mass * Cdot;
	m_impulse += impulse;

	vA += (m_mA * impulse) * m_JvAC;
	wA += 1_rad * m_iA * impulse * m_JwA;
	vB += (m_mB * impulse) * m_JvBD;
	wB += 1_rad * m_iB * impulse * m_JwB;
	vC -= (m_mC * impulse) * m_JvAC;
	wC -= 1_rad * m_iC * impulse * m_JwC;
	vD -= (m_mD * impulse) * m_JvBD;
	wD -= 1_rad * m_iD * impulse * m_JwD;

	velocities[m_indexA].linear = vA;
	velocities[m_indexA].angular = wA;
	velocities[m_indexB].linear = vB;
	velocities[m_indexB].angular = wB;
	velocities[m_indexC].linear = vC;
	velocities[m_indexC].angular = wC;
	velocities[m_indexD].linear = vD;
	velocities[m_indexD].angular = wD;
}

bool GearJoint::SolvePositionConstraints(Span<Position> positions, const ConstraintSolverConf& conf) const
{
	auto cA = positions[m_indexA].linear;
	auto aA = positions[m_indexA].angular;
	auto cB = positions[m_indexB].linear;
	auto aB = positions[m_indexB].angular;
	auto cC = positions[m_indexC].linear;
	auto aC = positions[m_indexC].angular;
	auto cD = positions[m_indexD].linear;
	auto aD = positions[m_indexD].angular;

	const UnitVec2 qA(aA), qB(aB), qC(aC), qD(aD);

	const auto linearError = RealNum{0};

	Angle coordinateA, coordinateB;

	Vec2 JvAC, JvBD;
	RealNum JwA, JwB, JwC, JwD;
	auto mass = RealNum{0};

	if (m_typeA == JointType::Revolute)
	{
		JvAC = Vec2_zero;
		JwA = RealNum{1};
		JwC = RealNum{1};
		mass += m_iA + m_iC;

		coordinateA = aA - aC - m_referenceAngleA;
	}
	else
	{
		const auto u = Rotate(m_localAxisC, qC);
		const auto rC = Rotate(m_localAnchorC - m_lcC, qC);
		const auto rA = Rotate(m_localAnchorA - m_lcA, qA);
		JvAC = u * 1;
		JwC = Cross(rC, u);
		JwA = Cross(rA, u);
		mass += m_mC + m_mA + m_iC * Square(JwC) + m_iA * Square(JwA);

		const auto pC = m_localAnchorC - m_lcC;
		const auto pA = InverseRotate(rA + (cA - cC), qC);
		coordinateA = 1_rad * Dot(pA - pC, m_localAxisC);
	}

	if (m_typeB == JointType::Revolute)
	{
		JvBD = Vec2_zero;
		JwB = m_ratio;
		JwD = m_ratio;
		mass += Square(m_ratio) * (m_iB + m_iD);

		coordinateB = aB - aD - m_referenceAngleB;
	}
	else
	{
		const auto u = Rotate(m_localAxisD, qD);
		const auto rD = Rotate(m_localAnchorD - m_lcD, qD);
		const auto rB = Rotate(m_localAnchorB - m_lcB, qB);
		JvBD = m_ratio * u;
		JwD = m_ratio * Cross(rD, u);
		JwB = m_ratio * Cross(rB, u);
		mass += Square(m_ratio) * (m_mD + m_mB) + m_iD * Square(JwD) + m_iB * Square(JwB);

		const auto pD = m_localAnchorD - m_lcD;
		const auto pB = InverseRotate(rB + (cB - cD), qD);
		coordinateB = 1_rad * Dot(pB - pD, m_localAxisD);
	}

	const auto C = (coordinateA + m_ratio * coordinateB) - m_constant;

	auto impulse = RealNum{0};
	if (mass > RealNum{0})
	{
		impulse = -C.ToRadians() / mass;
	}

	cA += m_mA * impulse * JvAC;
	aA += 1_rad * m_iA * impulse * JwA;
	cB += m_mB * impulse * JvBD;
	aB += 1_rad * m_iB * impulse * JwB;
	cC -= m_mC * impulse * JvAC;
	aC -= 1_rad * m_iC * impulse * JwC;
	cD -= m_mD * impulse * JvBD;
	aD -= 1_rad * m_iD * impulse * JwD;

	positions[m_indexA].linear = cA;
	positions[m_indexA].angular = aA;
	positions[m_indexB].linear = cB;
	positions[m_indexB].angular = aB;
	positions[m_indexC].linear = cC;
	positions[m_indexC].angular = aC;
	positions[m_indexD].linear = cD;
	positions[m_indexD].angular = aD;

	// TODO_ERIN not implemented
	return linearError < conf.linearSlop;
}

Vec2 GearJoint::GetAnchorA() const
{
	return GetWorldPoint(*GetBodyA(), GetLocalAnchorA());
}

Vec2 GearJoint::GetAnchorB() const
{
	return GetWorldPoint(*GetBodyB(), GetLocalAnchorB());
}

Vec2 GearJoint::GetReactionForce(RealNum inv_dt) const
{
	return inv_dt * m_impulse * m_JvAC;
}

RealNum GearJoint::GetReactionTorque(RealNum inv_dt) const
{
	return inv_dt * m_impulse * m_JwA;
}

void GearJoint::SetRatio(RealNum ratio)
{
	assert(IsValid(ratio));
	m_ratio = ratio;
}

RealNum GearJoint::GetRatio() const
{
	return m_ratio;
}
