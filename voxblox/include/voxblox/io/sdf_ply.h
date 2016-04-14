#ifndef VOXBLOX_CORE_IO_SDF_PLY_H
#define VOXBLOX_CORE_IO_SDF_PLY_H

#include "voxblox/io/ply_writer.h"
#include "voxblox/core/map.h"

namespace voxblox {

namespace io {

enum PlyOutputTypes {
  // The full SDF colorized by the distance in each voxel.
  kSdfDistanceColor = 0,
  // Isosurface colorized by ???
  kSdfIsosurface
};

template <typename MapType>
bool outputMapAsPly(const MapType& map, const std::string& filename,
                    PlyOutputTypes type) {
  return false;
}

template <>
bool outputMapAsPly<TsdfMap>(const TsdfMap& map, const std::string& filename,
                             PlyOutputTypes type) {
  // Create a PlyWriter.
  PlyWriter writer(filename);

  if (type == kSdfDistanceColor) {
    // In this case, we get all the allocated voxels and color them based on
    // distance value.
    size_t num_blocks = map.getNumberOfAllocatedBlocks();
    // This function is block-specific:
    size_t num_voxels_per_block = map.getVoxelsPerBlock();

    // Maybe this isn't strictly true, since actually we may have stuff with 0
    // weight...
    size_t total_voxels = num_blocks * num_voxels_per_block;
    const bool has_color = true;
    writer.addVerticesWithProperties(total_voxels, has_color);
    if (!writer.writeHeader()) {
      return false;
    }

    BlockIndexList blocks;
    map.getAllAllocatedBlocks(&blocks);

    // Iterate over all blocks.
    const float max_distance = 0.5;
    for (const BlockIndex& index : blocks) {
      // Iterate over all voxels in said blocks.
      const TsdfBlock& block = map.getBlockByIndex(index);
      for (size_t i = 0; i < num_voxels_per_block; ++i) {
        float distance = block.getTsdfVoxelByLinearIndex(i).distance;

        // Get back the original coordinate of this voxel.
        Coordinates coord = block.getCoordinatesOfTsdfVoxelByLinearIndex(i);

        // Decide how to color this.
        // Distance > 0 = blue, distance < 0 = red.
        uint8_t color[3];
        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
        if (distance < 0) {
          color[0] = static_cast<uint8_t>(-distance / max_distance * 255);
        } else {
          color[2] = static_cast<uint8_t>(distance / max_distance * 255);
        }

        writer.writeVertex(coord, color);
      }
    }
    writer.closeFile();
    return true;
  }
  return false;
}

}  // namespace io

}  // namespace voxblox

#endif  // VOXBLOX_CORE_IO_SDF_PLY_H
