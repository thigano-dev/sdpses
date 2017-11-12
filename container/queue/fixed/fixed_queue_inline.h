/**
 * @file	fixed_queue_inline.h
 * @brief	fixed-size queue inline
 * @author	Tsuguyoshi Higano
 * @date	Nov 13, 2017
 *
 * @par Project
 * Software Development Platform for Small-scale Embedded Systems (SDPSES)
 *
 * @copyright (c) Tsuguyoshi Higano, 2017
 *
 * @par License
 * Released under the MIT license@n
 * http://opensource.org/licenses/mit-license.php
 */

/*! @note The include guard is not required. */

#if defined(USE_ORIGINAL_ALLOCATOR_)
#include "allocator.h"
#endif /* USE_ORIGINAL_ALLOCATOR_ */

namespace sdpses {

namespace container {

/**
 * @brief	Constructor
 * @param	size_max		the maximum number of elements
 */
template <typename T>
inline FixedQueue<T>::FixedQueue(const std::size_t size_max)
	: kSIZE_MAX(size_max)
	, head_(0)
	, tail_(0)
	, size_(0)
#if defined(USE_ORIGINAL_ALLOCATOR_)
	, elements_(new(Allocator_allocate((sizeof(T) * size_max) + sizeof(std::size_t))) T[size_max])
#else
	, elements_(new T[size_max])
#endif
{
}

/**
 * @brief	Destructor
 */
template <typename T>
inline FixedQueue<T>::~FixedQueue()
{
#if defined(USE_ORIGINAL_ALLOCATOR_)
	for (std::size_t i = 0; i < kSIZE_MAX; i++) {
		elements_[i].~T();
	}
	Allocator_deallocate(elements_);
#else
	delete[] elements_;
#endif
}

/**
 * @brief	Removes all elements
 * @return	none
 */
template <typename T>
inline void FixedQueue<T>::clear()
{
	head_ = 0;
	tail_ = 0;
	size_ = 0;
}

/**
 * @brief	Inserts a element
 * @param	element			element
 * @return	none
 *
 * @pre not full
 */
template <typename T>
inline void FixedQueue<T>::push(const T& element)
{
	elements_[tail_] = element;
	if (++tail_ == kSIZE_MAX) { tail_ = 0; }
	size_++;
}

/**
 * @brief	Removes the next element
 * @return	none
 *
 * @pre not empty
 */
template <typename T>
inline void FixedQueue<T>::pop()
{
	if (++head_ == kSIZE_MAX) { head_ = 0; }
	size_--;
}

/**
 * @brief	Returns a reference to the next element
 * @return	a reference to the next element
 *
 * @pre not empty
 */
template <typename T>
inline T& FixedQueue<T>::peek()
{
	return elements_[head_];
}

template <typename T>
inline const T& FixedQueue<T>::peek() const
{
	return elements_[head_];
}

/**
 * @brief	Is empty
 * @retval	true			empty
 * @retval	false			not empty
 */
template <typename T>
inline bool FixedQueue<T>::empty() const
{
	return (size_) ? false : true;
}

/**
 * @brief	Is full
 * @retval	true			full
 * @retval	false			not full
 */
template <typename T>
inline bool FixedQueue<T>::full() const
{
	return (size_ < kSIZE_MAX) ? false : true;
}

/**
 * @brief	Returns the number of elements
 * @return	the number of elements
 */
template <typename T>
inline std::size_t FixedQueue<T>::size() const
{
	return size_;
}

/**
 * @brief	Returns the number of storable elements
 * @return	the number of storable elements
 */
template <typename T>
inline std::size_t FixedQueue<T>::availableSize() const
{
	return (kSIZE_MAX - size_);
}

/**
 * @brief	Returns the maximum number of elements
 * @return	the maximum number of elements
 */
template <typename T>
inline std::size_t FixedQueue<T>::maxSize() const
{
	return kSIZE_MAX;
}

} /* namespace container */

} /* namespace sdpses */
