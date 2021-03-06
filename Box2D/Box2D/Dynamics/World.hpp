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

#ifndef B2_WORLD_H
#define B2_WORLD_H

#include <Box2D/Common/Math.hpp>
#include <Box2D/Common/BlockAllocator.hpp>
#include <Box2D/Common/StackAllocator.hpp>
#include <Box2D/Dynamics/ContactManager.hpp>
#include <Box2D/Dynamics/WorldCallbacks.hpp>
#include <Box2D/Dynamics/BodyList.hpp>
#include <Box2D/Dynamics/JointList.hpp>
#include <Box2D/Dynamics/ContactList.hpp>
#include <Box2D/Dynamics/Profile.hpp>

namespace box2d {

struct AABB;
struct BodyDef;
struct JointDef;
class Body;
class Fixture;
class Joint;
class Island;
class StepConf;

struct PreStepStats
{
	uint32 ignored = 0;
	uint32 destroyed = 0;
	uint32 updated = 0;
	uint32 added = 0;
};

struct RegStepStats
{
	uint32 islandsFound = 0;
	uint32 islandsSolved = 0;
	uint32 contactsAdded = 0;
	uint32 bodiesSlept = 0;
};

struct ToiStepStats
{
	uint32 islandsFound = 0;
	uint32 contactsChecked = 0;
	uint32 contactsAdded = 0;
};

struct StepStats
{
	PreStepStats pre;
	RegStepStats reg;
	ToiStepStats toi;
};

constexpr auto EarthlyGravity = Vec2{0, RealNum(-9.8)};

/// World.
/// @detail
/// The world class manages all physics entities, dynamic simulation,
/// and asynchronous queries. The world also contains efficient memory
/// management facilities.
/// @note This data structure is 392-bytes large (on at least one 64-bit platform).
class World
{
public:
	using size_type = size_t;
	using ts_iters_type = ts_iters_t;

	struct Def
	{
		constexpr Def& UseGravity(Vec2 value) noexcept;
		constexpr Def& UseLinearSlop(RealNum value) noexcept;
		constexpr Def& UseAngularSlop(RealNum value) noexcept;

		Vec2 gravity = EarthlyGravity;
		
		RealNum linearSlop = LinearSlop; // originally 0.005;
		
		RealNum angularSlop = Pi * 2 / 180;

		RealNum maxVertexRadius = 255.0f; // linearSlop * 2550000
	};
	
	static constexpr Def GetDefaultDef()
	{
		return Def{};	
	}

	static const BodyDef& GetDefaultBodyDef();

	/// Constructs a world object.
	World(const Def& def = GetDefaultDef());

	/// Destructor.
	/// @detail
	/// All physics entities are destroyed and all dynamically allocated memory is released.
	~World();

	/// Register a destruction listener. The listener is owned by you and must
	/// remain in scope.
	void SetDestructionListener(DestructionListener* listener) noexcept;

	/// Register a contact filter to provide specific control over collision.
	/// Otherwise the default filter is used. The listener is
	/// owned by you and must remain in scope. 
	void SetContactFilter(ContactFilter* filter) noexcept;

	/// Register a contact event listener. The listener is owned by you and must
	/// remain in scope.
	void SetContactListener(ContactListener* listener) noexcept;

	/// Creates a rigid body given a definition.
	/// @note No reference to the definition is retained.
	/// @warning This function is locked during callbacks.
	Body* CreateBody(const BodyDef& def = GetDefaultBodyDef());

	/// Destroys the given body.
	/// @note This function is locked during callbacks.
	/// @warning This automatically deletes all associated shapes and joints.
	/// @warning This function is locked during callbacks.
	void Destroy(Body* body);

	/// Creates a joint to constrain bodies together.
	/// @detail No reference to the definition
	/// is retained. This may cause the connected bodies to cease colliding.
	/// @warning This function is locked during callbacks.
	/// @return <code>nullptr</code> if world has <code>MaxJoints</code>,
	///   else pointer to newly created joint.
	Joint* CreateJoint(const JointDef& def);

	/// Destroys a joint.
	///
	/// @detail This may cause the connected bodies to begin colliding.
	///
	/// @warning This function is locked during callbacks.
	/// @warning Behavior is undefined if the passed joint was not created by this world.
	///
	/// @param joint Joint, created by this world, to destroy.
	void Destroy(Joint* joint);

	/// Steps the world ahead by the given configuration.
	///
	/// @detail
	/// Performs position and velocity updating, sleeping of non-moving bodies, updating
	/// of the contacts, and notifying the contact listener of begin-contact, end-contact,
	/// pre-solve, and post-solve events.
	/// If the given velocity and position iterations are more than zero,
	/// this method also respectively performs velocity and position resolution of the contacting bodies.
	///
	/// @note While body velocities are updated accordingly (per the sum of forces acting on them),
	/// body positions (barring any collisions) are updated as if they had moved the entire time step
	/// at those resulting velocities.
	/// In other words, a body initially at p0 going v0 fast with a sum acceleration of a,
	/// after time t and barring any collisions,
	/// will have a new velocity (v1) of v0 + (a * t) and a new position (p1) of p0 + v1 * t.
	///
	/// @warning Varying the time step may lead to non-physical behaviors.
	///
	/// @post Static bodies are unmoved.
	/// @post Kinetic bodies are moved based on their previous velocities.
	/// @post Dynamic bodies are moved based on their previous velocities, gravity,
	/// applied forces, applied impulses, masses, damping, and the restitution and friction values
	/// of their fixtures when they experience collisions.
	///
	/// @param conf Configuration for the simulation step.
	///
	/// @return Statistics for the step.
	///
	StepStats Step(const StepConf& conf);

	/// Queries the world for all fixtures that potentially overlap the provided AABB.
	/// @param callback a user implemented callback class.
	/// @param aabb the query box.
	void QueryAABB(QueryFixtureReporter* callback, const AABB& aabb) const;

	/// Ray-cast the world for all fixtures in the path of the ray. Your callback
	/// controls whether you get the closest point, any point, or n-points.
	/// The ray-cast ignores shapes that contain the starting point.
	/// @param callback a user implemented callback class.
	/// @param point1 the ray starting point
	/// @param point2 the ray ending point
	void RayCast(RayCastFixtureReporter* callback, const Vec2& point1, const Vec2& point2) const;

	/// Gets the world body list.
	/// @return Body list that can be iterated over using its begin and end methods or using ranged-based for-loops.
	BodyList& GetBodies() noexcept;

	/// Gets the world body list for this constant world.
	/// @return Body list that can be iterated over using its begin and end methods or using ranged-based for-loops.
	const BodyList& GetBodies() const noexcept;

	/// Gets the world joint list.
	/// @return World joint list.
	JointList& GetJoints() noexcept;

	/// Gets the world joint list.
	/// @return World joint list.
	const JointList& GetJoints() const noexcept;

	/// Gets the world contact list.
	/// @warning contacts are created and destroyed in the middle of a time step.
	/// Use ContactListener to avoid missing contacts.
	/// @return the head of the world contact list.
	ContactList& GetContacts() noexcept;

	/// Gets the world contact list.
	/// @warning contacts are created and destroyed in the middle of a time step.
	/// Use ContactListener to avoid missing contacts.
	/// @return the head of the world contact list.
	const ContactList& GetContacts() const noexcept;

	bool GetSubStepping() const noexcept;

	/// Enable/disable single stepped continuous physics. For testing.
	void SetSubStepping(bool flag) noexcept;

	/// Get the number of broad-phase proxies.
	size_type GetProxyCount() const noexcept;

	/// Get the height of the dynamic tree.
	size_type GetTreeHeight() const noexcept;

	/// Get the balance of the dynamic tree.
	size_type GetTreeBalance() const;

	/// Gets the quality metric of the dynamic tree.
	/// @detail The smaller the better.
	/// @return Value of zero or more.
	RealNum GetTreeQuality() const;

	/// Change the global gravity vector.
	void SetGravity(const Vec2 gravity) noexcept;
	
	/// Get the global gravity vector.
	Vec2 GetGravity() const noexcept;

	/// Is the world locked (in the middle of a time step).
	bool IsLocked() const noexcept;

	/// Shift the world origin. Useful for large worlds.
	/// The body shift formula is: position -= newOrigin
	/// @param newOrigin the new origin with respect to the old origin
	void ShiftOrigin(const Vec2 newOrigin);

	/// Get the contact manager for testing.
	const ContactManager& GetContactManager() const noexcept;

	RealNum GetLinearSlop() const noexcept;

	RealNum GetAngularSlop() const noexcept;

	/// Gets the minimum vertex radius that shapes in this world can be.
	RealNum GetMinVertexRadius() const noexcept;
	
	/// Gets the maximum vertex radius that shapes in this world can be.
	RealNum GetMaxVertexRadius() const noexcept;

	RealNum GetInvDeltaTime() const noexcept;

private:

	using flags_type = uint32;

	// m_flags
	enum Flag: flags_type
	{
		e_newFixture	= 0x0001,
		e_locked		= 0x0002,

		e_substepping   = 0x0020,
		
		/// Step complete. @detail Used for sub-stepping. @sa m_subStepping.
		e_stepComplete  = 0x0040,
	};

	friend class Body;
	friend class Fixture;
	friend class ContactManager;

	/// Sloves the step.
	/// @detail Finds islands, integrates and solves constraints, solves position constraints.
	/// @note This may miss collisions involving fast moving bodies and allow them to tunnel through each other.
	RegStepStats Solve(const StepConf& step);

	/// Solves the given island.
	///
	/// @detail This:
	///   1. Updates every island-body's sweep.pos0 to its sweep.pos1.
	///   2. Updates every island-body's sweep.pos1 to the new "solved" position for it.
	///   3. Updates every island-body's velocity to the new accelerated, dampened, and "solved" velocity for it.
	///   4. Synchronizes every island-body's transform (by updating it to transform one of the body's sweep).
	///   5. Reports to the listener (if non-null).
	///
	/// @param step Time step information.
	/// @param island Island of bodies, contacts, and joints to solve for.
	///
	/// @return <code>true</code> if the contact and joint position constraints were solved, <code>false</code> otherwise.
	bool Solve(const StepConf& step, Island& island);

	static body_count_t AddToIsland(Island& island, Body& body);

	/// Builds island based off of a given "seed" body.
	/// @post Contacts are listed in the island in the order that bodies list those contacts.
	/// @post Joints are listed the island in the order that bodies list those joints.
	Island BuildIsland(Body& seed,
						 BodyList::size_type& remNumBodies,
						 contact_count_t& remNumContacts,
						 JointList::size_type& remNumJoints);

	/// Solves the step using successive time of impact (TOI) events.
	/// @detail Used for continuous physics.
	/// @note This is intended to detect and prevent the tunneling that the faster Solve method may miss.
	/// @param step Time step value to use.
	ToiStepStats SolveTOI(const StepConf& step);

	/// "Solves" collisions for the given time of impact.
	///
	/// @param step Time step to solve for.
	/// @param contact Contact.
	///
	/// @note Precondition 1: there is no contact having a lower TOI in this time step that has not already been solved for.
	/// @note Precondition 2: there is not a lower TOI in the time step for which collisions have not already been processed.
	///
	bool SolveTOI(const StepConf& step, Contact& contact);

	/// Solves the time of impact for bodies 0 and 1 of the given island.
	///
	/// @detail This:
	///   1. Updates pos0 of the sweeps of bodies 0 and 1.
	///   2. Updates pos1 of the sweeps, the transforms, and the velocities of the other bodies in this island.
	///
	/// @pre <code>island.m_bodies</code> contains at least two bodies, the first two of which are bodies 0 and 1.
	/// @pre <code>island.m_bodies</code> contains appropriate other bodies of the contacts of the two bodies.
	/// @pre <code>island.m_contacts</code> contains the contact that specified the two identified bodies.
	/// @pre <code>island.m_contacts</code> contains appropriate other contacts of the two bodies.
	///
	/// @param step Time step information.
	/// @param island Island to do time of impact solving for.
	///
	/// @return <code>true</code> if successful, <code>false</code> otherwise.
	///
	bool SolveTOI(const StepConf& step, Island& island);

	/// Updates bodies.
	/// @detail
	/// Updates bodies[i]->m_sweep.pos1 to position[i]
	/// Copies velocity and position array data back out to the bodies
	static void UpdateBodies(Span<Body*> bodies,
							 Span<const Position> positions, Span<const Velocity> velocities);

	void ResetBodiesForSolveTOI();
	void ResetContactsForSolveTOI();
	void ResetContactsForSolveTOI(Body& body);

	/// Processes the contacts of a given body for TOI handling.
	/// @detail This does the following:
	///   1. Advances the appropriate associated other bodies to the given TOI (advancing
	///      their sweeps and synchronizing their transforms to their new sweeps).
	///   2. Updates the contact manifolds and touching statuses and notifies listener (if one given) of
	///      the appropriate contacts of the body.
	///   3. Adds those contacts that are still enabled and still touching to the given island
	///      (or resets the other bodies advancement).
 	///   4. Adds to the island, those other bodies that haven't already been added of the contacts that got added.
	/// @note Precondition: there should be no lower TOI for which contacts have not already been processed.
	/// @param[in,out] island Island. On return this may contain additional contacts or bodies.
	/// @param[in,out] body A dynamic/accelerable body.
	/// @param[in] toi Time of impact (TOI). Value between 0 and 1.
	/// @param listener Pointer to listener that will be called, or nullptr.
	static void ProcessContactsForTOI(Island& island, Body& body, RealNum toi, ContactListener* listener = nullptr);
	
	bool Add(Body& b);
	bool Add(Joint& j);

	bool Remove(Body& b);
	bool Remove(Joint& j);

	/// Whether or not "step" is complete.
	/// @detail The "step" is completed when there are no more TOI events for the current time step.
	/// @sa <code>SetStepComplete</code>.
	bool IsStepComplete() const noexcept;

	void SetStepComplete(bool value) noexcept;

	void SetAllowSleeping() noexcept;
	void UnsetAllowSleeping() noexcept;

	struct ContactToiData
	{
		contact_count_t count = 0;
		Contact* contact = nullptr; ///< Contact for which the time of impact is relavant.
		RealNum toi = MaxFloat; ///< Time of impact (TOI) as a fractional value between 0 and 1.
	};

	/// Updates the contact times of impact.
	/// @detail While checking contacts and setting their time of impact values this also
	///   finds the contact with the lowest (soonest) time of impact.
	/// @return Contact with the least time of impact and its time of impact, or null contact.
	ContactToiData UpdateContactTOIs(const StepConf& step);

	bool HasNewFixtures() const noexcept;

	void SetNewFixtures() noexcept;
	
	void UnsetNewFixtures() noexcept;
	
	BlockAllocator m_blockAllocator; ///< Block allocator. 136-bytes.

	StackAllocator m_stackAllocator; ///< Stack allocator. 64-bytes.
	
	ContactFilter m_defaultFilter; ///< Default contact filter. 8-bytes.
	
	ContactManager m_contactMgr{
		m_blockAllocator, &m_defaultFilter, nullptr
	}; ///< Contact manager. 112-bytes.

	BodyList m_bodies; ///< Body collection.
	JointList m_joints; ///< Joint collection.

	Vec2 m_gravity; ///< Gravity setting. 8-bytes.

	DestructionListener* m_destructionListener = nullptr; ///< Destruction listener. 8-bytes.

	flags_type m_flags = e_stepComplete;

	/// Inverse delta-t from previous step.
	/// @detail Used to compute time step ratio to support a variable time step.
	/// @note 4-bytes large.
	/// @sa Step.
	RealNum m_inv_dt0 = 0;

	const RealNum m_linearSlop;
	const RealNum m_angularSlop;

	/// Maximum vertex radius.
	/// @detail
	/// This is the maximum shape vertex radius that any bodies' of this world should create
	/// fixtures for. Requests to create fixtures for shapes with vertex radiuses bigger than
	/// this must be rejected. As an upper bound, this value prevents shapes from getting
	/// associated with this world that would otherwise not be able to be simulated due to
	/// numerical issues. It can also be set below this upper bound to constrain the differences
	/// between shape vertex radiuses to possibly more limited visual ranges.
	const RealNum m_maxVertexRadius;
};

constexpr inline World::Def& World::Def::UseGravity(Vec2 value) noexcept
{
	gravity = value;
	return *this;
}

constexpr inline World::Def& World::Def::UseLinearSlop(RealNum value) noexcept
{
	linearSlop = value;
	return *this;
}

constexpr inline World::Def& World::Def::UseAngularSlop(RealNum value) noexcept
{
	angularSlop = value;
	return *this;
}

inline BodyList& World::GetBodies() noexcept
{
	return m_bodies;
}

inline const BodyList& World::GetBodies() const noexcept
{
	return m_bodies;
}

inline JointList& World::GetJoints() noexcept
{
	return m_joints;
}

inline const JointList& World::GetJoints() const noexcept
{
	return m_joints;
}

inline ContactList& World::GetContacts() noexcept
{
	return m_contactMgr.GetContacts();
}

inline const ContactList& World::GetContacts() const noexcept
{
	return m_contactMgr.GetContacts();
}

inline Vec2 World::GetGravity() const noexcept
{
	return m_gravity;
}

inline bool World::IsLocked() const noexcept
{
	return (m_flags & e_locked) == e_locked;
}

inline const ContactManager& World::GetContactManager() const noexcept
{
	return m_contactMgr;
}

inline bool World::IsStepComplete() const noexcept
{
	return m_flags & e_stepComplete;
}

inline void World::SetStepComplete(bool value) noexcept
{
	if (value)
	{
		m_flags |= e_stepComplete;
	}
	else
	{
		m_flags &= ~e_stepComplete;		
	}
}

inline bool World::GetSubStepping() const noexcept
{
	return m_flags & e_substepping;
}

inline void World::SetSubStepping(bool flag) noexcept
{
	if (flag)
	{
		m_flags |= e_substepping;
	}
	else
	{
		m_flags &= ~e_substepping;
	}
}

inline bool World::HasNewFixtures() const noexcept
{
	return m_flags & e_newFixture;
}

inline void World::SetNewFixtures() noexcept
{
	m_flags |= World::e_newFixture;
}

inline void World::UnsetNewFixtures() noexcept
{
	m_flags &= ~e_newFixture;
}

inline RealNum World::GetLinearSlop() const noexcept
{
	return m_linearSlop;
}

inline RealNum World::GetAngularSlop() const noexcept
{
	return m_angularSlop;
}

inline RealNum World::GetMinVertexRadius() const noexcept
{
	// This scaling factor should not be modified.
	// Making it smaller means some shapes could have insufficient buffer for continuous collision.
	// Making it larger may create artifacts for vertex collision.
	return GetLinearSlop() * 2;
}

inline RealNum World::GetMaxVertexRadius() const noexcept
{
	return m_maxVertexRadius;
}

inline RealNum World::GetInvDeltaTime() const noexcept
{
	return m_inv_dt0;
}

/// Gets the AABB extension for the given world.
/// @detail
/// Fattens AABBs in the dynamic tree. This allows proxies
/// to move by a small amount without triggering a tree adjustment.
/// This is in meters.
inline RealNum GetAabbExtension(const World& world) noexcept
{
	return world.GetLinearSlop() * 20;
}

inline body_count_t GetBodyCount(const World& world) noexcept
{
	return world.GetBodies().size();
}

inline World::size_type GetJointCount(const World& world) noexcept
{
	return world.GetJoints().size();
}

inline contact_count_t GetContactCount(const World& world) noexcept
{
	return world.GetContacts().size();
}

/// Steps the world ahead by a given time amount.
///
/// @detail
/// Performs position and velocity updating, sleeping of non-moving bodies, updating
/// of the contacts, and notifying the contact listener of begin-contact, end-contact,
/// pre-solve, and post-solve events.
/// If the given velocity and position iterations are more than zero,
/// this method also respectively performs velocity and position resolution of the contacting bodies.
///
/// @note While body velocities are updated accordingly (per the sum of forces acting on them),
/// body positions (barring any collisions) are updated as if they had moved the entire time step
/// at those resulting velocities.
/// In other words, a body initially at p0 going v0 fast with a sum acceleration of a,
/// after time t and barring any collisions,
/// will have a new velocity (v1) of v0 + (a * t) and a new position (p1) of p0 + v1 * t.
///
/// @warning Varying the time step may lead to non-physical behaviors.
///
/// @post Static bodies are unmoved.
/// @post Kinetic bodies are moved based on their previous velocities.
/// @post Dynamic bodies are moved based on their previous velocities, gravity,
/// applied forces, applied impulses, masses, damping, and the restitution and friction values
/// of their fixtures when they experience collisions.
///
/// @param timeStep Amount of time to simulate (in seconds). This should not vary.
/// @param velocityIterations Number of iterations for the velocity constraint solver.
/// @param positionIterations Number of iterations for the position constraint solver.
///   The position constraint solver resolves the positions of bodies that overlap.
StepStats Step(World& world, RealNum timeStep,
			   World::ts_iters_type velocityIterations = 8, World::ts_iters_type positionIterations = 3);

size_t GetFixtureCount(const World& world) noexcept;

size_t GetShapeCount(const World& world) noexcept;

size_t GetAwakeCount(const World& world) noexcept;

size_t Awaken(World& world);

/// Clears forces.
/// @detail
/// Manually clear the force buffer on all bodies.
void ClearForces(World& world) noexcept;

} // namespace box2d

#endif
