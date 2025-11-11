// Copyright (C) 2024, Princeton University.
// This source code is licensed under the BSD 3-Clause license found in the LICENSE file in the root directory of this source tree.

// Authors: Zeyu Ma

// Inspired by https://www.shadertoy.com/view/4ttSWf by Inigo Quilez

DEVICE_FUNC void sdf_trees(
    float3_nonbuiltin position,
    float *sdf,
    float *auxs,
    int *i_params, float *f_params,
    int *landtiles_i_params, float *landtiles_f_params
) {
    /* params
        int:
        seed, n_lattice

        float:
    */
    int seed = i_params[0];
    int n_lattice = i_params[1];
    float density = f_params[0];
    float r_short = f_params[1];
    float ratio = f_params[2];
    float r_randomness = f_params[3];
    float ratio_randomness = f_params[4];
    float height_offset = f_params[5];
    float noise_octaves = f_params[6];
    float noise_scale = f_params[7];
    float noise_freq = f_params[8];
    float mask_octaves = f_params[9];
    float mask_freq = f_params[10];
    float mask_shift = f_params[11];
    noise_freq *= density;
    noise_scale /= density;
    r_short /= density;
    float cell_center_[3], distance, sdf_=1e9;
    float sdf_a;
    int tree_id = 0;
    for (int i = 0; i < n_lattice; i++) {
        int hash;
        Voronoi(position.x, position.y, 0, myhash(seed, i), 1, density, 1, &cell_center_[0], &distance, &hash);
        float mask = Perlin(cell_center_[0], cell_center_[1], cell_center_[2], myhash(1, seed), mask_octaves, mask_freq) + mask_shift < 0;
        if (mask) {
            float3_nonbuiltin cell_center = float3_nonbuiltin(cell_center_[0], cell_center_[1], cell_center_[2]);
            landtiles(cell_center, &sdf_a, NULL, 0, landtiles_i_params, landtiles_f_params, NULL, NULL);
            float scale = 1 - r_randomness + r_randomness*2*hash_to_float(myhash(0, myhash(hash, i)));
            float r_short_cell = r_short * scale;
            scale = 1 - ratio_randomness + ratio_randomness*2*hash_to_float(myhash(1, myhash(hash, i)));
            float ratio_cell = ratio * scale;
            cell_center.z = cell_center.z - sdf_a + height_offset * r_short_cell * ratio_cell;
            float3_nonbuiltin vec = position - cell_center;
            float tmp = sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z / (ratio_cell*ratio_cell)) - r_short_cell;
            if (tmp < sdf_) {
                sdf_ = tmp;
                tree_id = hash;
            }
        }
    }
    landtiles(position, &sdf_a, NULL, 0, landtiles_i_params, landtiles_f_params, NULL, NULL);
    *sdf = min(sdf_ + Perlin(position.x, position.y, position.z, seed, noise_octaves, noise_freq) * noise_scale, sdf_a + r_short * 0.1f);
    if (auxs != NULL) *auxs = (float)tree_id;
}