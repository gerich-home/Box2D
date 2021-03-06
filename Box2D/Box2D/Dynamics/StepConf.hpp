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

#ifndef B2_STEP_CONF_HPP
#define B2_STEP_CONF_HPP

#include <Box2D/Common/Settings.hpp>
#include <Box2D/Common/Angle.hpp>

namespace box2d {

/// Step configuration.
/// @note This data structure is 56-bytes large (on at least one 64-bit platform).
class StepConf
{
public:
	using iteration_type = ts_iters_t;

	static constexpr auto InvalidIteration = static_cast<iteration_type>(-1);

	/// Gets the delta time (time amount for this time step).
	/// @sa set_dt(RealNum).
	/// @return Time step amount in seconds.
	RealNum get_dt() const noexcept { return dt; }

	/// Gets the inverse delta-t value.
	/// @return 1/dt or 0 if dt is 0.
	/// @sa get_dt().
	RealNum get_inv_dt() const noexcept { return inv_dt; }
	
	/// Sets the delta time value.
	/// @post Getting the delta time will return this set value.
	/// @post The inverse delta time value is the inverse of this set value or zero if the value is zero.
	/// @sa get_inv_dt().
	/// @param value Elapsed time amount (in seconds).
	void set_dt(RealNum value) noexcept
	{
		dt = value;
		inv_dt = (value != 0)? RealNum{1} / value: RealNum{0};
	}

	constexpr StepConf& use_dt(RealNum value) noexcept
	{
		set_dt(value);
		return *this;
	}

	/// Delta t ratio.
	/// @detail This is the delta-t times the inverse delta t from the previous world step.
	///   Value of 1 indicates that the time step has not varied.
	RealNum dtRatio = 1;

	/// Minimum still time to sleep.
	/// @detail The time that a body must be still before it will go to sleep.
	/// Set to an invalid value to disable sleeping.
	RealNum minStillTimeToSleep = RealNum{1} / 2; // aka 0.5

	/// This scale factor controls how fast overlap is resolved. Ideally this would be 1 so
	/// that overlap is removed in one time step. However using values close to 1 often lead
	/// to overshoot.
	RealNum regResolutionRate = RealNum{2} / 10; // aka 0.2.
	
	/// Time of impact resolution rate.
	RealNum toiResolutionRate = RealNum{75} / 100; // aka .75

	/// A velocity threshold for elastic collisions. Any collision with a relative linear
	/// velocity below this threshold will be treated as inelastic.
	RealNum velocityThreshold = RealNum{8} / 10; // RealNum{1};

	/// Maximum linear velocity of a body.
	/// This limit is very large and is used to prevent numerical problems.
	/// You shouldn't need to adjust this.
	RealNum maxTranslation = 4; // originally 2
	
	/// Maximum angular velocity of a body.
	/// This limit is very large and is used to prevent numerical problems.
	/// You shouldn't need to adjust this.
	Angle maxRotation = 1_rad * Pi / 2;

	/// Maximum linear correction.
	/// @detail This value should be greater than the linear slop value.
	RealNum maxLinearCorrection = LinearSlop * 40; // 40 * linearSlop. aka 0.004
	
	RealNum maxAngularCorrection = AngularSlop * 4;

	iteration_type regVelocityIterations = 8; ///< Velocity iterations.
	iteration_type regPositionIterations = 3; ///< Position iterations.
	iteration_type toiVelocityIterations = 8; ///< Velocity iterations.

	/// Maximum TOI stage position iterations.
	iteration_type toiPositionIterations = 20; ///< Position iterations.
	
	iteration_type maxTOIRootIterCount = MaxTOIRootIterCount;
	
	iteration_type maxTOIIterations = MaxTOIIterations;
	
	/// Maximum sub steps.
	/// @detail
	/// This is the maximum number of sub-steps per contact in continuous physics simulation.
	/// In other words, this is the maximum number of times in a world step that a contact will
	/// have continuous collision resolution done for it.
	iteration_type maxSubSteps = 48;
	
	bool doWarmStart = true; ///< Whether or not to perform warm starting (in the regular phase).
	bool doToi = true; ///< Whether or not to perform continuous collision detection.

private:
	RealNum dt; ///< Delta time. This is the time step in seconds.
	RealNum inv_dt; ///< Inverse time step (1/dt or 0 if dt == 0). @see dt.
};

} // namespace box2d

#endif
