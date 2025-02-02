/**************************************************************************/
/*  voxel_mesher.cpp                                                      */
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

#include "voxel_mesher.h"
#include "../storage/voxel_buffer.h"
#include "../util/godot/funcs.h"

Ref<Mesh> VoxelMesher::build_mesh(Ref<VoxelBuffer> voxels, Array materials) {
	ERR_FAIL_COND_V(voxels.is_null(), Ref<ArrayMesh>());

	Output output;
	Input input = { **voxels, 0 };
	build(output, input);

	if (output.surfaces.is_empty()) {
		return Ref<ArrayMesh>();
	}

	Ref<ArrayMesh> mesh;
	mesh.instantiate();

	int surface_index = 0;
	for (int i = 0; i < output.surfaces.size(); ++i) {
		Array surface = output.surfaces[i];
		if (surface.is_empty()) {
			continue;
		}

		CRASH_COND(surface.size() != Mesh::ARRAY_MAX);
		if (!is_surface_triangulated(surface)) {
			continue;
		}

		mesh->add_surface_from_arrays(output.primitive_type, surface, Array());
		if (i < materials.size()) {
			mesh->surface_set_material(surface_index, materials[i]);
		}
		++surface_index;
	}

	return mesh;
}

void VoxelMesher::build(Output &output, const Input &input) {
	ERR_PRINT("Not implemented");
}

unsigned int VoxelMesher::get_minimum_padding() const {
	return _minimum_padding;
}

unsigned int VoxelMesher::get_maximum_padding() const {
	return _maximum_padding;
}

void VoxelMesher::set_padding(int minimum, int maximum) {
	CRASH_COND(minimum < 0);
	CRASH_COND(maximum < 0);
	_minimum_padding = minimum;
	_maximum_padding = maximum;
}

void VoxelMesher::_bind_methods() {
	// Shortcut if you want to generate a mesh directly from a fixed grid of
	// voxels. Useful for testing the different meshers.
	ClassDB::bind_method(D_METHOD("build_mesh", "voxel_buffer", "materials"),
			&VoxelMesher::build_mesh);
	ClassDB::bind_method(D_METHOD("get_minimum_padding"),
			&VoxelMesher::get_minimum_padding);
	ClassDB::bind_method(D_METHOD("get_maximum_padding"),
			&VoxelMesher::get_maximum_padding);
}
