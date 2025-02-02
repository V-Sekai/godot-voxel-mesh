/**************************************************************************/
/*  span.h                                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef SPAN_H
#define SPAN_H

#include "fixed_array.h"
#include <core/error/error_macros.h>
#include <vector>

// View into an array, referencing a pointer and a size.
// STL equivalent would be std::span<T> in C++20
template <typename T>
class Span {
public:
	inline Span() :
			_ptr(nullptr), _size(0) {}

	inline Span(T *p_ptr, size_t p_begin, size_t p_end) {
		CRASH_COND(p_end < p_begin);
		_ptr = p_ptr + p_begin;
		_size = p_end - p_begin;
	}

	inline Span(T *p_ptr, size_t p_size) :
			_ptr(p_ptr), _size(p_size) {}

	inline Span(Span<T> &p_other, size_t p_begin, size_t p_end) {
		CRASH_COND(p_end < p_begin);
		CRASH_COND(p_begin >= p_other.size());
		CRASH_COND(
				p_end >
				p_other.size()); // `>` because p_end is typically `p_begin + size`
		_ptr = p_other._ptr + p_begin;
		_size = p_end - p_begin;
	}

	// TODO Remove this one, prefer to_span() specializations
	inline Span(std::vector<T> &vec, size_t p_begin, size_t p_end) {
		CRASH_COND(p_end < p_begin);
		CRASH_COND(p_begin >= vec.size());
		CRASH_COND(p_end >
				vec.size()); // `>` because p_end is typically `p_begin + size`
		_ptr = &vec[p_begin];
		_size = p_end - p_begin;
	}

	// TODO Remove this one, prefer to_span() specializations
	template <unsigned int N>
	inline Span(FixedArray<T, N> &a) {
		_ptr = a.data();
		_size = a.size();
	}

	inline Span<T> sub(size_t from, size_t len) const {
		CRASH_COND(from + len > _size);
		return Span<T>(_ptr + from, len);
	}

	inline Span<T> sub(size_t from) const {
		CRASH_COND(from >= _size);
		return Span<T>(_ptr + from, _size - from);
	}

	template <typename U>
	Span<U> reinterpret_cast_to() const {
		const size_t size_in_bytes = _size * sizeof(T);
#ifdef DEBUG_ENABLED
		CRASH_COND(size_in_bytes % sizeof(U) != 0);
#endif
		return Span<U>(reinterpret_cast<U *>(_ptr), 0, size_in_bytes / sizeof(U));
	}

	inline T &operator[](size_t i) {
#ifdef DEBUG_ENABLED
		CRASH_COND(i >= _size);
#endif
		return _ptr[i];
	}

	inline const T &operator[](size_t i) const {
#ifdef DEBUG_ENABLED
		CRASH_COND(i >= _size);
#endif
		return _ptr[i];
	}

	inline size_t size() const {
		return _size;
	}

	inline T *data() {
		return _ptr;
	}

	inline const T *data() const {
		return _ptr;
	}

	inline void fill(const T v) {
		const T *end = _ptr + _size;
		for (T *p = _ptr; p != end; ++p) {
			*p = v;
		}
	}

private:
	T *_ptr;
	size_t _size;
};

template <typename T>
Span<T> to_span(std::vector<T> &vec) {
	return Span<T>(vec.data(), 0, vec.size());
}

template <typename T>
Span<const T> const_span_from_position_and_size(const std::vector<T> &vec,
		unsigned int pos,
		unsigned int size) {
	CRASH_COND(pos + size > vec.size());
	return Span<const T>(vec.data(), pos, pos + vec.size());
}

template <typename T>
Span<const T> to_span_const(const std::vector<T> &vec) {
	return Span<const T>(vec.data(), 0, vec.size());
}

template <typename T, unsigned int N>
Span<T> to_span(FixedArray<T, N> &a) {
	return Span<T>(a.data(), a.size());
}

template <typename T, unsigned int N>
Span<T> to_span(FixedArray<T, N> &a, unsigned int count) {
	CRASH_COND(count > a.size());
	return Span<T>(a.data(), count);
}

template <typename T, unsigned int N>
Span<const T> to_span_const(const FixedArray<T, N> &a, unsigned int count) {
	CRASH_COND(count > a.size());
	return Span<const T>(a.data(), count);
}

template <typename T, unsigned int N>
Span<const T> to_span_const(const FixedArray<T, N> &a) {
	return Span<const T>(a.data(), 0, a.size());
}

#endif // SPAN_H
