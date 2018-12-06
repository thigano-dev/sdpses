/**
 * @file	fixed_queue.h
 * @brief	fixed-size queue
 * @author	Tsuguyoshi Higano
 * @date	Dec 06, 2018
 *
 * @par Project
 * Software Development Platform for Small-scale Embedded Systems (SDPSES)
 *
 * @copyright (c) Tsuguyoshi Higano, 2017-2018
 *
 * @par License
 * Released under the MIT license@n
 * http://opensource.org/licenses/mit-license.php
 */

#ifndef SDPSES_CONTAINER_FIXED_QUEUE_H_INCLUDED_
#define SDPSES_CONTAINER_FIXED_QUEUE_H_INCLUDED_

#include <cstddef>

namespace sdpses {

namespace container {

/**
 * @class	FixedQueue
 * @brief	Fixed-size Queue class
 * @note	Don't inherit from this class.
 */
template <typename T>
class FixedQueue {

public:
	explicit FixedQueue(std::size_t size_max);
	~FixedQueue();

	void clear();
	void push(const T& element);
	void pop();
	T& front();
	const T& front() const;

	bool empty() const;
	bool full() const;

	std::size_t size() const;
	std::size_t availableSize() const;
	std::size_t maxSize() const;

private:
	FixedQueue();
	FixedQueue(const FixedQueue&);
	FixedQueue& operator=(const FixedQueue&);

	const std::size_t kSIZE_MAX;

	std::size_t head_;
	std::size_t tail_;
	std::size_t size_;
	T* const elements_;
};

} /* namespace container */

} /* namespace sdpses */

#include "fixed_queue_inline.h"

#endif /* SDPSES_CONTAINER_FIXED_QUEUE_H_INCLUDED_ */
