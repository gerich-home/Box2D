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

#ifndef B2_STACK_ALLOCATOR_H
#define B2_STACK_ALLOCATOR_H

#include <Box2D/Common/Settings.hpp>

namespace box2d {

/// Stack allocator.
/// @detail
/// This is a stack allocator used for fast per step allocations.
/// You must nest allocate/free pairs. The code will assert
/// if you try to interleave multiple allocate/free pairs.
/// @note This class satisfies the C++11 std::unique_ptr() Deleter concept.
/// @note This data structure is 64-bytes large (on at least one 64-bit platform).
class StackAllocator
{
public:
	using size_type = size_t;

	struct Configuration
	{
		size_type preallocation_size = 100 * 1024;
		size_type allocation_records = 32;
	};

	static constexpr Configuration GetDefaultConfiguration()
	{
		return Configuration{};
	}

	StackAllocator(Configuration config = GetDefaultConfiguration()) noexcept;

	~StackAllocator() noexcept;

	StackAllocator(const StackAllocator& copy) = delete;

	/// Allocates an aligned block of memory of the given size.
	/// @return Pointer to memory if the allocator has allocation records left,
	/// <code>nullptr</code> otherwise.
	/// @sa GetEntryCount.
	void* Allocate(size_type size) noexcept;

	void Free(void* p) noexcept;

	template <typename T>
	T* AllocateArray(size_type size) noexcept
	{
		return static_cast<T*>(Allocate(size * sizeof(T)));
	}

	/// Functional operator for freeing memory allocated by this object.
	/// @detail This method frees memory (like called Free) and allows this object
	///   to be used as deleter to std::unique_ptr.
	void operator()(void *p) noexcept
	{
		Free(p);
	}

	auto GetMaxAllocation() const noexcept
	{
		return m_maxAllocation;
	}

	/// Gets the current allocation record entry usage count.
	/// @return Value between 0 and the maximum number of entries possible for this allocator.
	/// @sa GetMaxEntries.
	auto GetEntryCount() const noexcept
	{
		return m_entryCount;
	}

	/// Gets the current index location.
	/// @detail This represents the number of bytes used (of the storage allocated at construction
    ///    time by this object). Storage remaining is calculated by subtracting this value from
	///    <code>StackSize</code>.
	/// @return Value between 0 and <code>StackSize</code>.
	auto GetIndex() const noexcept
	{
		return m_index;
	}

	/// Gets the total number of bytes that this object has currently allocated.
	auto GetAllocation() const noexcept
	{
		return m_allocation;
	}
	
	auto GetPreallocatedSize() const noexcept
	{
		return m_size;
	}
	
	auto GetMaxEntries() const noexcept
	{
		return m_max_entries;
	}
	
private:

	struct AllocationRecord
	{
		void* data;
		size_type size;
		bool usedMalloc;
	};
	
	char* const m_data;
	AllocationRecord* const m_entries;
	size_type const m_size;
	size_type const m_max_entries;
	
	size_type m_index = 0;
	size_type m_allocation = 0;
	size_type m_maxAllocation = 0;
	size_type m_entryCount = 0;
};
	
} // namespace box2d

#endif
