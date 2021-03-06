/*
* Original work Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
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

#include <Box2D/Dynamics/Joints/PrismaticJoint.hpp>
#include <Box2D/Dynamics/Body.hpp>
#include <Box2D/Dynamics/StepConf.hpp>
#include <Box2D/Dynamics/Contacts/ContactSolver.hpp>

using namespace box2d;

// Linear constraint (point-to-line)
// d = p2 - p1 = x2 + r2 - x1 - r1
// C = dot(perp, d)
// Cdot = dot(d, cross(w1, perp)) + dot(perp, v2 + cross(w2, r2) - v1 - cross(w1, r1))
//      = -dot(perp, v1) - dot(cross(d + r1, perp), w1) + dot(perp, v2) + dot(cross(r2, perp), v2)
// J = [-perp, -cross(d + r1, perp), perp, cross(r2,perp)]
//
// Angular constraint
// C = a2 - a1 + a_initial
// Cdot = w2 - w1
// J = [0 0 -1 0 0 1]
//
// K = J * invM * JT
//
// J = [-a -s1 a s2]
//     [0  -1  0  1]
// a = perp
// s1 = cross(d + r1, a) = cross(p2 - x1, a)
// s2 = cross(r2, a) = cross(p2 - x2, a)


// Motor/Limit linear constraint
// C = dot(ax1, d)
// Cdot = = -dot(ax1, v1) - dot(cross(d + r1, ax1), w1) + dot(ax1, v2) + dot(cross(r2, ax1), v2)
// J = [-ax1 -cross(d+r1,ax1) ax1 cross(r2,ax1)]

// Block Solver
// We develop a block solver that includes the joint limit. This makes the limit stiff (inelastic) even
// when the mass has poor distribution (leading to large torques about the joint anchor points).
//
// The Jacobian has 3 rows:
// J = [-uT -s1 uT s2] // linear
//     [0   -1   0  1] // angular
//     [-vT -a1 vT a2] // limit
//
// u = perp
// v = axis
// s1 = cross(d + r1, u), s2 = cross(r2, u)
// a1 = cross(d + r1, v), a2 = cross(r2, v)

// M * (v2 - v1) = JT * df
// J * v2 = bias
//
// v2 = v1 + invM * JT * df
// J * (v1 + invM * JT * df) = bias
// K * df = bias - J * v1 = -Cdot
// K = J * invM * JT
// Cdot = J * v1 - bias
//
// Now solve for f2.
// df = f2 - f1
// K * (f2 - f1) = -Cdot
// f2 = invK * (-Cdot) + f1
//
// Clamp accumulated limit impulse.
// lower: f2(3) = max(f2(3), 0)
// upper: f2(3) = min(f2(3), 0)
//
// Solve for correct f2(1:2)
// K(1:2, 1:2) * f2(1:2) = -Cdot(1:2) - K(1:2,3) * f2(3) + K(1:2,1:3) * f1
//                       = -Cdot(1:2) - K(1:2,3) * f2(3) + K(1:2,1:2) * f1(1:2) + K(1:2,3) * f1(3)
// K(1:2, 1:2) * f2(1:2) = -Cdot(1:2) - K(1:2,3) * (f2(3) - f1(3)) + K(1:2,1:2) * f1(1:2)
// f2(1:2) = invK(1:2,1:2) * (-Cdot(1:2) - K(1:2,3) * (f2(3) - f1(3))) + f1(1:2)
//
// Now compute impulse to be applied:
// df = f2 - f1

PrismaticJointDef::PrismaticJointDef(Body* bA, Body* bB, const Vec2 anchor, const Vec2 axis) noexcept:
	JointDef{JointType::Prismatic, bA, bB},
	localAnchorA{GetLocalPoint(*bA, anchor)},
	localAnchorB{GetLocalPoint(*bB, anchor)},
	localAxisA{GetLocalVector(*bA, axis)},
	referenceAngle{bB->GetAngle() - bA->GetAngle()}	
{
	// Intentionally empty.
}

PrismaticJoint::PrismaticJoint(const PrismaticJointDef& def)
: Joint(def)
{
	m_localAnchorA = def.localAnchorA;
	m_localAnchorB = def.localAnchorB;
	m_localXAxisA = GetUnitVector(def.localAxisA, UnitVec2::GetZero());
	m_localYAxisA = GetRevPerpendicular(m_localXAxisA);
	m_referenceAngle = def.referenceAngle;

	m_lowerTranslation = def.lowerTranslation;
	m_upperTranslation = def.upperTranslation;
	m_maxMotorForce = def.maxMotorForce;
	m_motorSpeed = def.motorSpeed;
	m_enableLimit = def.enableLimit;
	m_enableMotor = def.enableMotor;
}

void PrismaticJoint::InitVelocityConstraints(Span<Velocity> velocities,
											 Span<const Position> positions,
											 const StepConf& step,
											 const ConstraintSolverConf& conf)
{
	m_indexA = GetBodyA()->GetIslandIndex();
	m_indexB = GetBodyB()->GetIslandIndex();
	m_localCenterA = GetBodyA()->GetLocalCenter();
	m_localCenterB = GetBodyB()->GetLocalCenter();
	m_invMassA = GetBodyA()->GetInverseMass();
	m_invMassB = GetBodyB()->GetInverseMass();
	m_invIA = GetBodyA()->GetInverseInertia();
	m_invIB = GetBodyB()->GetInverseInertia();

	const auto cA = positions[m_indexA].linear;
	const auto aA = positions[m_indexA].angular;
	auto vA = velocities[m_indexA].linear;
	auto wA = velocities[m_indexA].angular;

	const auto cB = positions[m_indexB].linear;
	const auto aB = positions[m_indexB].angular;
	auto vB = velocities[m_indexB].linear;
	auto wB = velocities[m_indexB].angular;

	const auto qA = UnitVec2(aA);
	const auto qB = UnitVec2(aB);

	// Compute the effective masses.
	const auto rA = Rotate(m_localAnchorA - m_localCenterA, qA);
	const auto rB = Rotate(m_localAnchorB - m_localCenterB, qB);
	const auto d = (cB - cA) + rB - rA;

	const auto mA = m_invMassA, mB = m_invMassB;
	const auto iA = m_invIA, iB = m_invIB;

	// Compute motor Jacobian and effective mass.
	{
		m_axis = Rotate(m_localXAxisA, qA);
		m_a1 = Cross(d + rA, m_axis);
		m_a2 = Cross(rB, m_axis);

		m_motorMass = mA + mB + iA * m_a1 * m_a1 + iB * m_a2 * m_a2;
		if (m_motorMass > RealNum{0})
		{
			m_motorMass = RealNum{1} / m_motorMass;
		}
	}

	// Prismatic constraint.
	{
		m_perp = Rotate(m_localYAxisA, qA);

		m_s1 = Cross(d + rA, m_perp);
		m_s2 = Cross(rB, m_perp);

		const auto k11 = mA + mB + iA * m_s1 * m_s1 + iB * m_s2 * m_s2;
		const auto k12 = iA * m_s1 + iB * m_s2;
		const auto k13 = iA * m_s1 * m_a1 + iB * m_s2 * m_a2;
		auto k22 = iA + iB;
		if (k22 == RealNum{0})
		{
			// For bodies with fixed rotation.
			k22 = RealNum{1};
		}
		const auto k23 = iA * m_a1 + iB * m_a2;
		const auto k33 = mA + mB + iA * m_a1 * m_a1 + iB * m_a2 * m_a2;

		m_K.ex = Vec3(k11, k12, k13);
		m_K.ey = Vec3(k12, k22, k23);
		m_K.ez = Vec3(k13, k23, k33);
	}

	// Compute motor and limit terms.
	if (m_enableLimit)
	{
		const auto jointTranslation = Dot(m_axis, d);
		if (Abs(m_upperTranslation - m_lowerTranslation) < (conf.linearSlop * 2))
		{
			m_limitState = e_equalLimits;
		}
		else if (jointTranslation <= m_lowerTranslation)
		{
			if (m_limitState != e_atLowerLimit)
			{
				m_limitState = e_atLowerLimit;
				m_impulse.z = RealNum{0};
			}
		}
		else if (jointTranslation >= m_upperTranslation)
		{
			if (m_limitState != e_atUpperLimit)
			{
				m_limitState = e_atUpperLimit;
				m_impulse.z = RealNum{0};
			}
		}
		else
		{
			m_limitState = e_inactiveLimit;
			m_impulse.z = RealNum{0};
		}
	}
	else
	{
		m_limitState = e_inactiveLimit;
		m_impulse.z = RealNum{0};
	}

	if (!m_enableMotor)
	{
		m_motorImpulse = RealNum{0};
	}

	if (step.doWarmStart)
	{
		// Account for variable time step.
		m_impulse *= step.dtRatio;
		m_motorImpulse *= step.dtRatio;

		const auto P = m_impulse.x * m_perp + (m_motorImpulse + m_impulse.z) * m_axis;
		const auto LA = m_impulse.x * m_s1 + m_impulse.y + (m_motorImpulse + m_impulse.z) * m_a1;
		const auto LB = m_impulse.x * m_s2 + m_impulse.y + (m_motorImpulse + m_impulse.z) * m_a2;

		vA -= mA * P;
		wA -= 1_rad * iA * LA;

		vB += mB * P;
		wB += 1_rad * iB * LB;
	}
	else
	{
		m_impulse = Vec3_zero;
		m_motorImpulse = RealNum{0};
	}

	velocities[m_indexA].linear = vA;
	velocities[m_indexA].angular = wA;
	velocities[m_indexB].linear = vB;
	velocities[m_indexB].angular = wB;
}

void PrismaticJoint::SolveVelocityConstraints(Span<Velocity> velocities, const StepConf& step)
{
	auto vA = velocities[m_indexA].linear;
	auto wA = velocities[m_indexA].angular;
	auto vB = velocities[m_indexB].linear;
	auto wB = velocities[m_indexB].angular;

	const auto mA = m_invMassA, mB = m_invMassB;
	const auto iA = m_invIA, iB = m_invIB;

	// Solve linear motor constraint.
	if (m_enableMotor && m_limitState != e_equalLimits)
	{
		const auto Cdot = Dot(m_axis, vB - vA) + m_a2 * wB.ToRadians() - m_a1 * wA.ToRadians();
		auto impulse = m_motorMass * (m_motorSpeed - Cdot);
		const auto oldImpulse = m_motorImpulse;
		const auto maxImpulse = step.get_dt() * m_maxMotorForce;
		m_motorImpulse = Clamp(m_motorImpulse + impulse, -maxImpulse, maxImpulse);
		impulse = m_motorImpulse - oldImpulse;

		const auto P = impulse * m_axis;
		const auto LA = impulse * m_a1;
		const auto LB = impulse * m_a2;

		vA -= mA * P;
		wA -= 1_rad * iA * LA;

		vB += mB * P;
		wB += 1_rad * iB * LB;
	}

	const auto Cdot1 = Vec2{Dot(m_perp, vB - vA) + m_s2 * wB.ToRadians() - m_s1 * wA.ToRadians(), (wB - wA).ToRadians()};

	if (m_enableLimit && (m_limitState != e_inactiveLimit))
	{
		// Solve prismatic and limit constraint in block form.
		const auto Cdot2 = Dot(m_axis, vB - vA) + m_a2 * wB.ToRadians() - m_a1 * wA.ToRadians();
		const auto Cdot = Vec3{Cdot1.x, Cdot1.y, Cdot2};

		const auto f1 = m_impulse;
		m_impulse += Solve33(m_K, -Cdot);

		if (m_limitState == e_atLowerLimit)
		{
			m_impulse.z = Max(m_impulse.z, RealNum{0});
		}
		else if (m_limitState == e_atUpperLimit)
		{
			m_impulse.z = Min(m_impulse.z, RealNum{0});
		}

		// f2(1:2) = invK(1:2,1:2) * (-Cdot(1:2) - K(1:2,3) * (f2(3) - f1(3))) + f1(1:2)
		const auto b = -Cdot1 - (m_impulse.z - f1.z) * Vec2{m_K.ez.x, m_K.ez.y};
		const auto f2r = Solve22(m_K, b) + Vec2{f1.x, f1.y};
		m_impulse.x = f2r.x;
		m_impulse.y = f2r.y;

		const auto df = m_impulse - f1;

		const auto P = df.x * m_perp + df.z * m_axis;
		const auto LA = df.x * m_s1 + df.y + df.z * m_a1;
		const auto LB = df.x * m_s2 + df.y + df.z * m_a2;

		vA -= mA * P;
		wA -= 1_rad * iA * LA;

		vB += mB * P;
		wB += 1_rad * iB * LB;
	}
	else
	{
		// Limit is inactive, just solve the prismatic constraint in block form.
		const auto df = Solve22(m_K, -Cdot1);
		m_impulse.x += df.x;
		m_impulse.y += df.y;

		const auto P = df.x * m_perp;
		const auto LA = df.x * m_s1 + df.y;
		const auto LB = df.x * m_s2 + df.y;

		vA -= mA * P;
		wA -= 1_rad * iA * LA;

		vB += mB * P;
		wB += 1_rad * iB * LB;
	}

	velocities[m_indexA].linear = vA;
	velocities[m_indexA].angular = wA;
	velocities[m_indexB].linear = vB;
	velocities[m_indexB].angular = wB;
}

// A velocity based solver computes reaction forces(impulses) using the velocity constraint solver.Under this context,
// the position solver is not there to resolve forces.It is only there to cope with integration error.
//
// Therefore, the pseudo impulses in the position solver do not have any physical meaning.Thus it is okay if they suck.
//
// We could take the active state from the velocity solver.However, the joint might push past the limit when the velocity
// solver indicates the limit is inactive.
bool PrismaticJoint::SolvePositionConstraints(Span<Position> positions, const ConstraintSolverConf& conf) const
{
	auto cA = positions[m_indexA].linear;
	auto aA = positions[m_indexA].angular;
	auto cB = positions[m_indexB].linear;
	auto aB = positions[m_indexB].angular;

	const auto qA = UnitVec2{aA};
	const auto qB = UnitVec2{aB};

	const auto mA = m_invMassA, mB = m_invMassB;
	const auto iA = m_invIA, iB = m_invIB;

	// Compute fresh Jacobians
	const auto rA = Rotate(m_localAnchorA - m_localCenterA, qA);
	const auto rB = Rotate(m_localAnchorB - m_localCenterB, qB);
	const auto d = cB + rB - cA - rA;

	const auto axis = Rotate(m_localXAxisA, qA);
	const auto a1 = Cross(d + rA, axis);
	const auto a2 = Cross(rB, axis);
	const auto perp = Rotate(m_localYAxisA, qA);

	const auto s1 = Cross(d + rA, perp);
	const auto s2 = Cross(rB, perp);

	const auto C1 = Vec2{Dot(perp, d), (aB - aA - m_referenceAngle).ToRadians()};

	auto linearError = Abs(C1.x);
	const auto angularError = Abs(C1.y);

	auto active = false;
	auto C2 = RealNum{0};
	if (m_enableLimit)
	{
		const auto translation = Dot(axis, d);
		if (Abs(m_upperTranslation - m_lowerTranslation) < (RealNum{2} * conf.linearSlop))
		{
			// Prevent large angular corrections
			C2 = Clamp(translation, -conf.maxLinearCorrection, conf.maxLinearCorrection);
			linearError = Max(linearError, Abs(translation));
			active = true;
		}
		else if (translation <= m_lowerTranslation)
		{
			// Prevent large linear corrections and allow some slop.
			C2 = Clamp(translation - m_lowerTranslation + conf.linearSlop, -conf.maxLinearCorrection, RealNum{0});
			linearError = Max(linearError, m_lowerTranslation - translation);
			active = true;
		}
		else if (translation >= m_upperTranslation)
		{
			// Prevent large linear corrections and allow some slop.
			C2 = Clamp(translation - m_upperTranslation - conf.linearSlop, RealNum{0}, conf.maxLinearCorrection);
			linearError = Max(linearError, translation - m_upperTranslation);
			active = true;
		}
	}

	Vec3 impulse;
	if (active)
	{
		const auto k11 = mA + mB + iA * s1 * s1 + iB * s2 * s2;
		const auto k12 = iA * s1 + iB * s2;
		const auto k13 = iA * s1 * a1 + iB * s2 * a2;
		auto k22 = iA + iB;
		if (k22 == RealNum{0})
		{
			// For fixed rotation
			k22 = RealNum{1};
		}
		const auto k23 = iA * a1 + iB * a2;
		const auto k33 = mA + mB + iA * a1 * a1 + iB * a2 * a2;

		const auto K = Mat33{Vec3{k11, k12, k13}, Vec3{k12, k22, k23}, Vec3{k13, k23, k33}};

		const auto C = Vec3{C1.x, C1.y, C2};

		impulse = Solve33(K, -C);
	}
	else
	{
		const auto k11 = mA + mB + iA * s1 * s1 + iB * s2 * s2;
		const auto k12 = iA * s1 + iB * s2;
		auto k22 = iA + iB;
		if (k22 == RealNum{0})
		{
			k22 = RealNum{1};
		}

		const auto K = Mat22{Vec2{k11, k12}, Vec2{k12, k22}};

		const auto impulse1 = Solve(K, -C1);
		impulse.x = impulse1.x;
		impulse.y = impulse1.y;
		impulse.z = RealNum{0};
	}

	const auto P = impulse.x * perp + impulse.z * axis;
	const auto LA = impulse.x * s1 + impulse.y + impulse.z * a1;
	const auto LB = impulse.x * s2 + impulse.y + impulse.z * a2;

	cA -= mA * P;
	aA -= 1_rad * iA * LA;
	cB += mB * P;
	aB += 1_rad * iB * LB;

	positions[m_indexA].linear = cA;
	positions[m_indexA].angular = aA;
	positions[m_indexB].linear = cB;
	positions[m_indexB].angular = aB;

	return (linearError <= conf.linearSlop) && (angularError <= conf.angularSlop);
}

Vec2 PrismaticJoint::GetAnchorA() const
{
	return GetWorldPoint(*GetBodyA(), GetLocalAnchorA());
}

Vec2 PrismaticJoint::GetAnchorB() const
{
	return GetWorldPoint(*GetBodyB(), GetLocalAnchorB());
}

Vec2 PrismaticJoint::GetReactionForce(RealNum inv_dt) const
{
	return inv_dt * (m_impulse.x * m_perp + (m_motorImpulse + m_impulse.z) * m_axis);
}

RealNum PrismaticJoint::GetReactionTorque(RealNum inv_dt) const
{
	return inv_dt * m_impulse.y;
}

RealNum PrismaticJoint::GetJointTranslation() const
{
	const auto pA = GetWorldPoint(*GetBodyA(), m_localAnchorA);
	const auto pB = GetWorldPoint(*GetBodyB(), m_localAnchorB);
	return Dot(pB - pA, GetWorldVector(*GetBodyA(), Vec2{m_localXAxisA}));
}

RealNum PrismaticJoint::GetJointSpeed() const
{
	const auto bA = GetBodyA();
	const auto bB = GetBodyB();

	const auto rA = Rotate(m_localAnchorA - bA->GetLocalCenter(), bA->GetTransformation().q);
	const auto rB = Rotate(m_localAnchorB - bB->GetLocalCenter(), bB->GetTransformation().q);
	const auto p1 = bA->GetWorldCenter() + rA;
	const auto p2 = bB->GetWorldCenter() + rB;
	const auto d = p2 - p1;
	const auto axis = Rotate(m_localXAxisA, bA->GetTransformation().q);

	const auto vA = bA->GetVelocity().linear;
	const auto vB = bB->GetVelocity().linear;
	const auto wA = bA->GetVelocity().angular;
	const auto wB = bB->GetVelocity().angular;

	return Dot(d, (GetRevPerpendicular(axis) * wA.ToRadians())) + Dot(axis, vB + (GetRevPerpendicular(rB) * wB.ToRadians()) - vA - (GetRevPerpendicular(rA) * wA.ToRadians()));
}

bool PrismaticJoint::IsLimitEnabled() const noexcept
{
	return m_enableLimit;
}

void PrismaticJoint::EnableLimit(bool flag) noexcept
{
	if (m_enableLimit != flag)
	{
		GetBodyA()->SetAwake();
		GetBodyB()->SetAwake();
		m_enableLimit = flag;
		m_impulse.z = RealNum{0};
	}
}

RealNum PrismaticJoint::GetLowerLimit() const noexcept
{
	return m_lowerTranslation;
}

RealNum PrismaticJoint::GetUpperLimit() const noexcept
{
	return m_upperTranslation;
}

void PrismaticJoint::SetLimits(RealNum lower, RealNum upper)
{
	assert(lower <= upper);
	if ((lower != m_lowerTranslation) || (upper != m_upperTranslation))
	{
		GetBodyA()->SetAwake();
		GetBodyB()->SetAwake();
		m_lowerTranslation = lower;
		m_upperTranslation = upper;
		m_impulse.z = RealNum{0};
	}
}

bool PrismaticJoint::IsMotorEnabled() const noexcept
{
	return m_enableMotor;
}

void PrismaticJoint::EnableMotor(bool flag) noexcept
{
	GetBodyA()->SetAwake();
	GetBodyB()->SetAwake();
	m_enableMotor = flag;
}

void PrismaticJoint::SetMotorSpeed(RealNum speed) noexcept
{
	GetBodyA()->SetAwake();
	GetBodyB()->SetAwake();
	m_motorSpeed = speed;
}

void PrismaticJoint::SetMaxMotorForce(RealNum force) noexcept
{
	GetBodyA()->SetAwake();
	GetBodyB()->SetAwake();
	m_maxMotorForce = force;
}

RealNum PrismaticJoint::GetMotorForce(RealNum inv_dt) const noexcept
{
	return inv_dt * m_motorImpulse;
}
