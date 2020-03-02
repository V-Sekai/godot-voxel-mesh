#ifndef VOXEL_GENERATOR_GRAPH_H
#define VOXEL_GENERATOR_GRAPH_H

#include "../voxel_generator.h"
#include "program_graph.h"
#include "voxel_graph_runtime.h"

class VoxelGeneratorGraph : public VoxelGenerator {
	GDCLASS(VoxelGeneratorGraph, VoxelGenerator)
public:
	enum NodeTypeID {
		NODE_CONSTANT,
		NODE_INPUT_X,
		NODE_INPUT_Y,
		NODE_INPUT_Z,
		NODE_OUTPUT_SDF,
		NODE_ADD,
		NODE_SUBTRACT,
		NODE_MULTIPLY,
		NODE_SINE,
		NODE_FLOOR,
		NODE_ABS,
		NODE_SQRT,
		NODE_DISTANCE_2D,
		NODE_DISTANCE_3D,
		NODE_CLAMP,
		NODE_MIX,
		NODE_REMAP,
		NODE_CURVE,
		NODE_NOISE_2D,
		NODE_NOISE_3D,
		NODE_IMAGE_2D,
		NODE_TYPE_COUNT
	};

	VoxelGeneratorGraph();
	~VoxelGeneratorGraph();

	void clear();

	uint32_t create_node(NodeTypeID type_id, Vector2 position);
	void remove_node(uint32_t node_id);

	bool can_connect(uint32_t src_node_id, uint32_t src_port_index, uint32_t dst_node_id, uint32_t dst_port_index) const;
	void add_connection(uint32_t src_node_id, uint32_t src_port_index, uint32_t dst_node_id, uint32_t dst_port_index);
	void remove_connection(uint32_t src_node_id, uint32_t src_port_index, uint32_t dst_node_id, uint32_t dst_port_index);
	void get_connections(std::vector<ProgramGraph::Connection> &connections) const;
	//void get_connections_from_and_to(std::vector<ProgramGraph::Connection> &connections, uint32_t node_id) const;

	Variant get_node_param(uint32_t node_id, uint32_t param_index) const;
	void set_node_param(uint32_t node_id, uint32_t param_index, Variant value);

	Vector2 get_node_gui_position(uint32_t node_id) const;
	void set_node_gui_position(uint32_t node_id, Vector2 pos);

	NodeTypeID get_node_type_id(uint32_t node_id) const;
	PoolIntArray get_node_ids() const;

	int get_used_channels_mask() const override;

	void generate_block(VoxelBlockRequest &input) override;
	float generate_single(const Vector3i &position);

	enum BoundsType {
		BOUNDS_NONE = 0,
		BOUNDS_VERTICAL,
		BOUNDS_BOX,
		BOUNDS_TYPE_COUNT
	};

	void clear_bounds();
	void set_vertical_bounds(int min_y, int max_y, float bottom_sdf_value, float top_sdf_value, uint64_t bottom_type_value, uint64_t top_type_value);
	void set_box_bounds(Vector3i min, Vector3i max, float sdf_value, uint64_t type_value);

	Ref<Resource> duplicate(bool p_subresources) const override;

	// Debug

	float debug_measure_microseconds_per_voxel();
	void debug_load_waves_preset();

private:
	void compile();
	Interval analyze_range(Vector3i min_pos, Vector3i max_pos);

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	int _b_get_node_type_count() const;
	Dictionary _b_get_node_type_info(int type_id) const;
	PoolIntArray _b_get_node_ids() const;
	Array _b_get_connections() const;

	static void _bind_methods();

	struct Bounds {
		BoundsType type = BOUNDS_NONE;
		Vector3i min;
		Vector3i max;
		// Voxel values beyond bounds
		float sdf_value0 = 1.f;
		float sdf_value1 = 1.f;
		uint64_t type_value0 = 0;
		uint64_t type_value1 = 0;
	};

	ProgramGraph _graph;
	VoxelGraphRuntime _runtime;
	VoxelBuffer::ChannelId _channel = VoxelBuffer::CHANNEL_SDF;
	float _iso_scale = 0.1;
	Bounds _bounds;
};

VARIANT_ENUM_CAST(VoxelGeneratorGraph::NodeTypeID)
VARIANT_ENUM_CAST(VoxelGeneratorGraph::BoundsType)

#endif // VOXEL_GENERATOR_GRAPH_H
