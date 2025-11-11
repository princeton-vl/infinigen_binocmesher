// Copyright (C) 2025, Princeton University.
// This source code is licensed under the BSD 3-Clause license found in the LICENSE file in the root directory of this source tree.

// Authors: Zeyu Ma

// Inspired by https://www.shadertoy.com/view/XtsSWs by otaviogood

DEVICE_FUNC void sdf_citywindow(
    float3_nonbuiltin position,
    float *sdf,
    float *auxs,
    int *i_params, float *f_params,
    int *landtiles_i_params, float *landtiles_f_params
) {
    /* params
        int:
        seed,
        row_range_min,
        row_range_max,

        float:
        block_size,
        road_width,
        floor_height,
        height_range_min,
        height_range_max,
        second_sec_prob,
        cylinder_top_prob,
        sphere_top_prob,
        topbox_prob,
    */

    
    int seed = i_params[0];
    int row_range_min = i_params[1];
    int row_range_max = i_params[2];
    float block_size = f_params[0];
    float road_width = f_params[1];
    float floor_height = f_params[2];
    float height_range_min = f_params[3];
    float height_range_max = f_params[4];
    float second_sec_prob = f_params[5];
    float cylinder_top_prob = f_params[6];
    float sphere_top_prob = f_params[7];
    float topbox_prob = f_params[8];

    float block_x0 = floor(position.x / block_size) * block_size;
    float block_y0 = floor(position.y / block_size) * block_size;
    
    int block_seed = myhash(seed, myhash(int(floor(position.x / block_size)), int(floor(position.y / block_size))));
    int n_building_x = round(log_uniform(row_range_min, row_range_max, myhash(block_seed, 0)));
    int n_building_y = round(log_uniform(row_range_min, row_range_max, myhash(block_seed, 1)));
    float normalized_x = (position.x - block_x0) / block_size * n_building_x;
    float normalized_y = (position.y - block_y0) / block_size * n_building_y;
    
    int building_seed = myhash(block_seed, int(normalized_x) + int(normalized_y) * n_building_x);
    float building_center_x = (int(normalized_x) + 0.5f) / n_building_x * block_size;
    float building_center_y = (int(normalized_y) + 0.5f) / n_building_y * block_size;

    float sdf_a;
    float3_nonbuiltin cell_center = float3_nonbuiltin(building_center_x + block_x0, building_center_y + block_y0, 0);
    landtiles(cell_center, &sdf_a, NULL, 0, landtiles_i_params, landtiles_f_params, NULL, NULL);

    *sdf = building(
        NULL,
        position.x - block_x0, position.y - block_y0, position.z,
        building_center_x, building_center_y, building_seed,
        block_size, floor_height, -sdf_a,
        block_size / n_building_x - road_width,
        block_size / n_building_y - road_width,
        height_range_min, height_range_max,
        second_sec_prob, cylinder_top_prob, sphere_top_prob, topbox_prob
    ) + 0.05;
    if (auxs != NULL) {
        auxs[1] = float(building_seed);
        float frac_height = position.z / floor_height - floor(position.z / floor_height);
        float dx = (position.x - block_x0 - building_center_x) / ((block_size / n_building_x - road_width) / 10);
        float dy = (position.y - block_y0 - building_center_y) / ((block_size / n_building_y - road_width) / 10);
        float dz = position.z / floor_height;
        if (myhash(building_seed, 1) % 5 != 0) {
            float window_id = float(myhash(myhash(myhash(building_seed, int(floor(dz))), int(floor(dx))), int(floor(dy))));
            auxs[0] = window_id;
        }
        else {
            float window_id = float(myhash(myhash(myhash(building_seed, int(floor(dz))), 0), 0));
            auxs[0] = window_id;
        }

    }
}