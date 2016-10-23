#pragma once
#include "platform.h"

template<typename T, size_t Size>
struct CircularFifo{
	enum{
		c_capacity = Size + 1
	};

	CircularFifo():m_tail(0),m_head(0){}
	virtual ~CircularFifo(){}

	bool push(T& item);
	bool pop(T& item);
	bool was_empty();
	bool was_full();
	bool is_lock_free();
	size_t increment(size_t idx);
	std::atomic<size_t> m_tail;
	T m_array[c_capacity];
	std::atomic<size_t> m_head;
};

template<typename T, size_t Size>
bool CircularFifo<T, Size>::push(T& item){
	auto current_tail = m_tail.load(std::memory_order_relaxed);
	auto next_tail = increment(current_tail);
	if(next_tail != m_head.load(std::memory_order_acquire)){
		m_array[current_tail] = item;
		m_tail.store(next_tail, std::memory_order_release);
		return true;
	}else{
		// full queue
		return false;
	}
}

template<typename T, size_t Size>
bool CircularFifo<T,Size>::pop(T& item){
	auto current_head = m_head.load(std::memory_order_relaxed);
	if(current_head == m_tail.load(std::memory_order_acquire)){
		// empty queue
		return false;
	}else{
		item = m_array[current_head];
		m_head.store(increment(current_head), std::memory_order_release);
		return true;
	}
}

template<typename T, size_t Size>
bool CircularFifo<T,Size>::was_empty() {
	return (m_head.load() == m_tail.load());
}

template<typename T, size_t Size>
bool CircularFifo<T,Size>::was_full() {
	auto next_tail = increment(m_tail.load());
	return (next_tail == m_head.load());
}

template<typename T, size_t Size>
bool CircularFifo<T,Size>::is_lock_free() {
	return (m_tail.is_lock_free() && m_head.is_lock_free());
}

template<typename T, size_t Size>
size_t CircularFifo<T,Size>::increment(size_t idx) {
	return (idx+1)%c_capacity;
}

