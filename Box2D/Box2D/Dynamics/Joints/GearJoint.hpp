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

#ifndef B2_GEAR_JOINT_H
#define B2_GEAR_JOINT_H

#include <Box2D/Dynamics/Joints/Joint.hpp>

namespace box2d {

/// Gear joint definition. This definition requires two existing
/// revolute or prismatic joints (any combination will work).
struct GearJointDef : public JointDef
{
	constexpr GearJointDef() noexcept: JointDef(JointType::Gear) {}

	/// The first revolute/prismatic joint attached to the gear joint.
	Joint* joint1 = nullptr;

	/// The second revolute/prismatic joint attached to the gear joint.
	Joint* joint2 = nullptr;

	/// The gear ratio.
	/// @see GearJoint for explanation.
	RealNum ratio = RealNum{1};
};

/// A gear joint is used to connect two joints together. Either joint
/// can be a revolute or prismatic joint. You specify a gear ratio
/// to bind the motions together:
/// coordinate1 + ratio * coordinate2 = constant
/// The ratio can be negative or positive. If one joint is a revolute joint
/// and the other joint is a prismatic joint, then the ratio will have units
/// of length or units of 1/length.
/// @warning You have to manually destroy the gear joint if joint1 or joint2
/// is destroyed.
class GearJoint : public Joint
{
public:
	GearJoint(const GearJointDef& data);
	
	Vec2 GetAnchorA() const override;
	Vec2 GetAnchorB() const override;

	Vec2 GetReactionForce(RealNum inv_dt) const override;
	RealNum GetReactionTorque(RealNum inv_dt) const override;

	/// The local anchor point relative to bodyA's origin.
	Vec2 GetLocalAnchorA() const { return m_localAnchorA; }
	
	/// The local anchor point relative to bodyB's origin.
	Vec2 GetLocalAnchorB() const  { return m_localAnchorB; }

	/// Get the first joint.
	Joint* GetJoint1() noexcept { return m_joint1; }

	/// Get the second joint.
	Joint* GetJoint2() noexcept { return m_joint2; }

	/// Get the first joint.
	const Joint* GetJoint1() const noexcept { return m_joint1; }
	
	/// Get the second joint.
	const Joint* GetJoint2() const noexcept { return m_joint2; }
	
	/// Set/Get the gear ratio.
	void SetRatio(RealNum ratio);
	RealNum GetRatio() const;

private:

	void InitVelocityConstraints(Span<Velocity> velocities, Span<const Position> positions, const StepConf& step, const ConstraintSolverConf& conf) override;
	void SolveVelocityConstraints(Span<Velocity> velocities, const StepConf& step) override;
	bool SolvePositionConstraints(Span<Position> positions, const ConstraintSolverConf& conf) const override;

	Joint* m_joint1;
	Joint* m_joint2;

	JointType m_typeA;
	JointType m_typeB;

	// Body A is connected to body C
	// Body B is connected to body D
	Body* m_bodyC;
	Body* m_bodyD;

	// Solver shared
	Vec2 m_localAnchorA;
	Vec2 m_localAnchorB;
	Vec2 m_localAnchorC;
	Vec2 m_localAnchorD;

	UnitVec2 m_localAxisC;
	UnitVec2 m_localAxisD;

	Angle m_referenceAngleA;
	Angle m_referenceAngleB;

	Angle m_constant;
	RealNum m_ratio;

	RealNum m_impulse = 0;

	// Solver temp
	index_t m_indexA, m_indexB, m_indexC, m_indexD;
	Vec2 m_lcA, m_lcB, m_lcC, m_lcD;
	RealNum m_mA, m_mB, m_mC, m_mD;
	RealNum m_iA, m_iB, m_iC, m_iD;
	Vec2 m_JvAC;
	Vec2 m_JvBD;
	RealNum m_JwA, m_JwB, m_JwC, m_JwD;
	RealNum m_mass;
};
	
} // namespace box2d

#endif
