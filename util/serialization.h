/**************************************************************************/
/*  serialization.h                                                       */
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

#ifndef VOXEL_UTIL_SERIALIZATION_H
#define VOXEL_UTIL_SERIALIZATION_H

#include "span.h"

namespace VoxelUtility {
enum Endianess { ENDIANESS_BIG_ENDIAN,
	ENDIANESS_LITTLE_ENDIAN };

inline Endianess get_platform_endianess() {
	// https://stackoverflow.com/questions/4181951/how-to-check-whether-a-system-is-big-endian-or-little-endian#4181991
	const int n = 1;
	if (*(char *)&n == 1) {
		return ENDIANESS_LITTLE_ENDIAN;
	}
	return ENDIANESS_BIG_ENDIAN;
	// TODO In C++20 we'll be able to use std::endian
}

struct MemoryWriter {
	std::vector<uint8_t> &data;
	// Using network-order by default
	// TODO Apparently big-endian is dead
	// I chose it originally to match "network byte order",
	// but as I read comments about it there seem to be no reason to continue
	// using it. Needs a version increment.
	Endianess endianess = ENDIANESS_BIG_ENDIAN;

	MemoryWriter(std::vector<uint8_t> &p_data, Endianess p_endianess) :
			data(p_data), endianess(p_endianess) {}

	inline void store_8(uint8_t v) { data.push_back(v); }

	inline void store_16(uint16_t v) {
		if (endianess == ENDIANESS_BIG_ENDIAN) {
			data.push_back(v >> 8);
			data.push_back(v & 0xff);
		} else {
			data.push_back(v & 0xff);
			data.push_back(v >> 8);
		}
	}

	inline void store_32(uint32_t v) {
		if (endianess == ENDIANESS_BIG_ENDIAN) {
			data.push_back(v >> 24);
			data.push_back(v >> 16);
			data.push_back(v >> 8);
			data.push_back(v & 0xff);
		} else {
			data.push_back(v & 0xff);
			data.push_back(v >> 8);
			data.push_back(v >> 16);
			data.push_back(v >> 24);
		}
	}

	inline void store_float(float v) {
		union M {
			uint32_t i;
			float f;
		} m;
		m.f = v;
		store_32(m.i);
	}
};

struct MemoryReader {
	Span<const uint8_t> data;
	size_t pos = 0;
	// Using network-order by default
	Endianess endianess = ENDIANESS_BIG_ENDIAN;

	MemoryReader(Span<const uint8_t> p_data, Endianess p_endianess) :
			data(p_data), endianess(p_endianess) {}

	inline uint8_t get_8() {
		ERR_FAIL_COND_V(pos >= data.size(), 0);
		return data[pos++];
	}

	inline uint16_t get_16() {
		ERR_FAIL_COND_V(pos + 1 >= data.size(), 0);
		uint16_t v;
		if (endianess == ENDIANESS_BIG_ENDIAN) {
			v = (static_cast<uint16_t>(data[pos]) << 8) | data[pos + 1];
		} else {
			v = (static_cast<uint16_t>(data[pos]) |
					static_cast<uint16_t>(data[pos + 1]) << 8);
		}
		pos += sizeof(uint16_t);
		return v;
	}

	inline uint32_t get_32() {
		ERR_FAIL_COND_V(pos + 3 >= data.size(), 0);
		uint32_t v;
		if (endianess == ENDIANESS_BIG_ENDIAN) {
			v = (static_cast<uint32_t>(data[pos]) << 24) |
					(static_cast<uint32_t>(data[pos + 1]) << 16) |
					(static_cast<uint32_t>(data[pos + 2]) << 8) | data[pos + 3];
		} else {
			v = static_cast<uint32_t>(data[pos]) |
					(static_cast<uint32_t>(data[pos + 1]) << 8) |
					(static_cast<uint32_t>(data[pos + 2]) << 16) |
					(static_cast<uint32_t>(data[pos + 3]) << 24);
		}
		pos += sizeof(uint32_t);
		return v;
	}

	inline float get_float() {
		union M {
			uint32_t i;
			float f;
		} m;
		m.i = get_32();
		return m.f;
	}
};
} // namespace VoxelUtility

#endif // VOXEL_UTIL_SERIALIZATION_H
