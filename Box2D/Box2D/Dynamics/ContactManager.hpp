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

#ifndef B2_CONTACT_MANAGER_H
#define B2_CONTACT_MANAGER_H

#include <Box2D/Collision/BroadPhase.hpp>
#include <Box2D/Dynamics/ContactList.hpp>

namespace box2d {

class Contact;
class ContactFilter;
class ContactListener;
class BlockAllocator;
struct FixtureProxy;

/// Contact Manager.
/// @detail
/// This is a delegate of World (every World instance has one of these).
/// Objects of this class manage the contacts for the world they are in.
/// @note This data structure is 112-bytes large (on at least one 64-bit platform).
class ContactManager
{
public:
	struct CollideStats
	{
		uint32 ignored = 0;
		uint32 destroyed = 0;
		uint32 updated = 0;
	};

	ContactManager(BlockAllocator& allocator, ContactFilter* filter, ContactListener* listener):
		m_allocator{allocator}, m_contactFilter{filter}, m_contactListener{listener} {}
	
	// Broad-phase callback.
	bool AddPair(void* proxyUserDataA, void* proxyUserDataB)
	{
		return Add(*static_cast<FixtureProxy*>(proxyUserDataA), *static_cast<FixtureProxy*>(proxyUserDataB));
	}

	contact_count_t FindNewContacts();

	/// Destroys the given contact and removes it from its list.
	/// @detail This updates the contact list, returns the memory to the allocator,
	///   and decrements the contact manager's contact count.
	/// @param c Contact to destroy.
	void Destroy(Contact* c);

	/// Processes the narrow phase collision for the contact list.
	/// @detail
	/// This finds and destroys the contacts that need filtering and no longer should collide or
	/// that no longer have AABB-based overlapping fixtures. Those contacts that persist and
	/// have active bodies (either or both) get their Update methods called with the current
	/// contact listener as its argument.
	/// Essentially this really just purges contacts that are no longer relevant.
	CollideStats Collide();
	
	/// Gets the contact list.
	/// @return Contact list or <code>nullptr</code> if empty.
	const ContactList& GetContacts() const noexcept { return m_contacts; }
	
	/// Gets the contact list.
	/// @return Contact list or <code>nullptr</code> if empty.
	ContactList& GetContacts() noexcept { return m_contacts; }

	BroadPhase m_broadPhase; ///< Broad phase data. 72-bytes.
	ContactFilter* m_contactFilter; ///< Contact filter. 8-bytes.
	ContactListener* m_contactListener; ///< Contact listener. 8-bytes.

private:

	/// Adds a contact for proxyA and proxyB if appropriate.
	/// @detail Adds a new contact object to represent a contact between proxy A and proxy B if
	/// all of the following are true:
	///   1. The bodies of the fixtures of the proxies are not the one and the same.
	///   2. No contact already exists for these two proxies.
	///   3. The bodies of the proxies should collide (according to Body::ShouldCollide).
	///   4. The contact filter says the fixtures of the proxies should collide.
	///   5. There exists a contact-create function for the pair of shapes of the proxies.
	/// @param proxyA Proxy A.
	/// @param proxyB Proxy B.
	/// @return <code>true</code> if a new contact was indeed added (and created), else <code>false</code>.
	/// @sa bool Body::ShouldCollide(const Body* other) const
	bool Add(const FixtureProxy& proxyA, const FixtureProxy& proxyB);
	
	void Add(Contact* contact);

	/// Removes contact from this manager.
	/// @warning Behavior is undefined if called with a contact that is not managed by this manager.
	/// @param contact Non-null pointer to a contact that is managed by this manager.
	void Remove(Contact* contact);
	
	ContactList m_contacts; ///< Container of contacts managed by this manager.
	BlockAllocator& m_allocator;
};

} // namespace box2d

#endif
