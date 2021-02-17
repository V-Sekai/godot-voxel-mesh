#include "instance_data.h"
#include "../util/array_slice.h"
#include "../util/math/funcs.h"
#include "../util/serialization.h"
#include "../voxel_constants.h"
#include <core/variant.h>

namespace {
const uint32_t TRAILING_MAGIC = 0x900df00d;
}

inline uint8_t norm_to_u8(float x) {
	return clamp(static_cast<int>(128.f * x + 128.f), 0, 0xff);
}

inline float u8_to_norm(uint8_t v) {
	return (static_cast<real_t>(v) - 0x7f) * VoxelConstants::INV_0x7f;
}

struct CompressedQuaternion4b {
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t w;

	static CompressedQuaternion4b from_quat(Quat q) {
		CompressedQuaternion4b c;
		c.x = norm_to_u8(q.x);
		c.y = norm_to_u8(q.y);
		c.z = norm_to_u8(q.z);
		c.w = norm_to_u8(q.w);
		return c;
	}

	Quat to_quat() const {
		Quat q;
		q.x = u8_to_norm(x);
		q.y = u8_to_norm(y);
		q.z = u8_to_norm(z);
		q.w = u8_to_norm(w);
		return q.normalized();
	}
};

void serialize_instance_block_data(const VoxelInstanceBlockData &src, std::vector<uint8_t> &dst) {
	const uint8_t version = 0;
	const uint8_t instance_format = 0;

	VoxelUtility::MemoryWriter w(dst, VoxelUtility::ENDIANESS_BIG_ENDIAN);

	w.store_8(version);
	w.store_8(src.layers.size());
	w.store_float(src.position_range);

	// TODO Introduce a margin to position coordinates, stuff can spawn offset from the ground.
	// Or just compute the ranges
	const float pos_norm_scale = 1.f / src.position_range;

	for (size_t i = 0; i < src.layers.size(); ++i) {
		const VoxelInstanceBlockData::LayerData &layer = src.layers[i];

		w.store_16(layer.id);
		w.store_16(layer.instances.size());
		w.store_float(layer.scale_min);
		w.store_float(layer.scale_max);
		w.store_8(instance_format);

		const float scale_norm_scale = 1.f / (layer.scale_max - layer.scale_min);

		for (size_t j = 0; j < layer.instances.size(); ++j) {
			const VoxelInstanceBlockData::InstanceData &instance = layer.instances[j];

			w.store_16(static_cast<uint16_t>(pos_norm_scale * instance.transform.origin.x * 0xffff));
			w.store_16(static_cast<uint16_t>(pos_norm_scale * instance.transform.origin.y * 0xffff));
			w.store_16(static_cast<uint16_t>(pos_norm_scale * instance.transform.origin.z * 0xffff));

			const float scale = instance.transform.get_basis().get_scale().y;
			w.store_8(static_cast<uint8_t>(scale_norm_scale * (scale - layer.scale_min) * 0xff));

			const Quat q = instance.transform.get_basis().get_rotation_quat();
			const CompressedQuaternion4b cq = CompressedQuaternion4b::from_quat(q);
			w.store_8(cq.x);
			w.store_8(cq.y);
			w.store_8(cq.z);
			w.store_8(cq.w);
		}
	}

	w.store_32(TRAILING_MAGIC);
}

bool deserialize_instance_block_data(VoxelInstanceBlockData &dst, ArraySlice<const uint8_t> src) {
	const uint8_t expected_version = 0;
	const uint8_t expected_instance_format = 0;

	VoxelUtility::MemoryReader r(src, VoxelUtility::ENDIANESS_BIG_ENDIAN);

	const uint8_t version = r.get_8();
	ERR_FAIL_COND_V(version != expected_version, false);

	const uint8_t layers_count = r.get_8();
	dst.layers.resize(layers_count);

	dst.position_range = r.get_float();

	for (size_t i = 0; i < dst.layers.size(); ++i) {
		VoxelInstanceBlockData::LayerData &layer = dst.layers[i];

		layer.id = r.get_16();

		const uint16_t instance_count = r.get_16();
		layer.instances.resize(instance_count);

		layer.scale_min = r.get_float();
		layer.scale_max = r.get_float();
		ERR_FAIL_COND_V(layer.scale_max < layer.scale_min, false);
		const float scale_range = layer.scale_max - layer.scale_min;

		const uint8_t instance_format = r.get_8();
		ERR_FAIL_COND_V(instance_format != expected_instance_format, false);

		for (size_t j = 0; j < layer.instances.size(); ++j) {
			const float x = (static_cast<float>(r.get_16()) / 0xffff) * dst.position_range;
			const float y = (static_cast<float>(r.get_16()) / 0xffff) * dst.position_range;
			const float z = (static_cast<float>(r.get_16()) / 0xffff) * dst.position_range;

			const float s = (static_cast<float>(r.get_8()) / 0xff) * scale_range + layer.scale_min;

			CompressedQuaternion4b cq;
			cq.x = r.get_8();
			cq.y = r.get_8();
			cq.z = r.get_8();
			cq.w = r.get_8();
			const Quat q = cq.to_quat();

			VoxelInstanceBlockData::InstanceData &instance = layer.instances[j];
			instance.transform = Transform(Basis(q).scaled(Vector3(s, s, s)), Vector3(x, y, z));
		}
	}

	const uint32_t control_end = r.get_32();
	ERR_FAIL_COND_V_MSG(control_end != TRAILING_MAGIC, false,
			String("Expected {0}, found {1}").format(varray(TRAILING_MAGIC, control_end)));

	return true;
}
