//
//  AllocatedArray.hpp
//  Box2D
//
//  Created by Louis D. Langholtz on 6/15/16.
//
//

#ifndef Array_hpp
#define Array_hpp

#include <Box2D/Common/Settings.hpp>

#include <functional>

namespace box2d {

template <typename T, typename Deleter = std::function<void (void *)> >
class AllocatedArray
{
public:
	using size_type = size_t;
	using value_type = T;
	using const_value_type = const value_type;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using difference_type = std::ptrdiff_t;
	using deleter_type = Deleter;

	using iterator = pointer;
	using const_iterator = const_pointer;

	constexpr AllocatedArray(size_type max_size, pointer data, deleter_type deleter = noop_deleter):
		m_max_size{max_size}, m_data{data}, m_deleter{deleter}
	{
		assert(data);
	}
	
	~AllocatedArray() noexcept
	{
		m_deleter(m_data);
		m_data = nullptr;
	}

	AllocatedArray() = delete;
	AllocatedArray(const AllocatedArray& copy) = delete;

	AllocatedArray(AllocatedArray&& other) noexcept:
		m_max_size{other.m_max_size}, m_size{other.m_size}, m_data{other.m_data}, m_deleter{other.m_deleter}
	{
		other.m_size = 0;
		other.m_data = nullptr;
	}

	size_type size() const noexcept { return m_size; }
	size_type max_size() const noexcept { return m_max_size; }
	bool empty() const noexcept { return size() == 0; }

	pointer data() const noexcept { return m_data; }

	reference operator[](size_type i)
	{
		assert(i < m_size);
		return m_data[i];
	}

	const_reference operator[](size_type i) const
	{
		assert(i < m_size);
		return m_data[i];
	}

	iterator begin() { return iterator{m_data}; }
	iterator end() { return iterator{m_data + size()}; }
	const_iterator begin() const { return const_iterator{m_data}; }
	const_iterator end() const { return const_iterator{m_data + size()}; }
	
	const_iterator cbegin() const { return const_iterator{m_data}; }
	const_iterator cend() const { return const_iterator{m_data + size()}; }

	reference back() noexcept
	{
		assert(m_size > 0);
		return m_data[m_size - 1];		
	}

	const_reference back() const noexcept
	{
		assert(m_size > 0);
		return m_data[m_size - 1];
	}

	void clear() noexcept
	{
		m_size = 0;
	}

	void push_back(const_reference value)
	{
		assert(m_size < m_max_size);
		m_data[m_size] = value;
		++m_size;
	}
	
	void pop_back() noexcept
	{
		assert(m_size > 0);
		--m_size;
	}

private:
	static void noop_deleter(void*) {}

	size_type m_max_size = 0; ///< Max size. 8-bytes.
	size_type m_size = 0; ///< Current size. 8-bytes.
	pointer m_data = nullptr; ///< Pointer to allocated data space. 8-bytes.
	deleter_type m_deleter; ///< Deleter. 8-bytes (with default Deleter).
};

}; // namespace box2d

#endif /* AllocatedArray_hpp */
