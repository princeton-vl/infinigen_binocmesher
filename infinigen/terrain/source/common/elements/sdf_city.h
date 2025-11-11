// Copyright (C) 2025, Princeton University.
// This source code is licensed under the BSD 3-Clause license found in the LICENSE file in the root directory of this source tree.

// Authors: Zeyu Ma

// Inspired by https://www.shadertoy.com/view/XtsSWs by otaviogood

DEVICE_FUNC float building(
    float *max_h,
    float x, float y, float z,
    float center_x, float center_y, int seed,
    float block_size, float floor_height, float terrain_height,
    float size_x, float size_y,
    float height_range_min, float height_range_max,
    float second_sec_prob,
    float cylinder_top_prob,
    float sphere_top_prob,
    float topbox_prob
) {
    // Randomly choose height and shape
    float h = log_uniform(height_range_min, height_range_max, myhash(seed, 0));
    h /= fmaxf(1.f, 4.f * sqrt((center_x - 0.5) * (center_x - 0.5) + (center_y - 0.5) * (center_y - 0.5)) / block_size);
    h = floor(h / floor_height) * floor_height;
    if (h < terrain_height + floor_height) {
        if (max_h != NULL) *max_h = 0;
        return 1;
    }

    float dx = x - center_x;
    float dy = y - center_y;
    float sdf = fmaxf(fmaxf(abs(dx) - size_x / 2, abs(dy) - size_y / 2), z - h);
    float top_size_x = size_x;
    float top_size_y = size_y;
    float last_h = h;

    if (hash_to_float(myhash(seed, 2)) < second_sec_prob) {
        float h2 = last_h * 0.5 * hash_to_float(myhash(seed, 7));
        h2 = ceil(h2 / floor_height) * floor_height;
        last_h = h2;
        h += h2;
        sdf = fminf(sdf, fmaxf(fmaxf(abs(dx) - size_x * 0.8 / 2, abs(dy) - size_y * 0.8 / 2), z - h));
        top_size_x = size_x * 0.8;
        top_size_y = size_y * 0.8;
    }

    float cut_height = floor_height * 0.1;
    float cut_sdf = fmaxf(fmaxf(abs(dx) - top_size_x / 2 * 0.9, abs(dy) - top_size_y / 2 * 0.9), h - cut_height - z);
    sdf = fmaxf(sdf, -cut_sdf);

    float topbox_h = 0;
    int has_box = 0;
    if (hash_to_float(myhash(seed, 3)) < topbox_prob) {
        has_box = 1;
        int n_box = int(hash_to_float(myhash(seed, 11)) * 3) + 1;
        for (int _ = 0; _ < n_box; _++) {
            float box_size_x = top_size_x * (0.2 + 0.2 * hash_to_float(myhash(seed, myhash(12, _))));
            float box_size_y = top_size_y * (0.2 + 0.2 * hash_to_float(myhash(seed, myhash(13, _))));
            float box_size_z = floor_height;
            topbox_h = max(topbox_h, box_size_z);
            float box_center_x = center_x - top_size_x / 2 + top_size_x * hash_to_float(myhash(seed, myhash(14, _)));
            float box_center_y = center_y - top_size_y / 2 + top_size_y * hash_to_float(myhash(seed, myhash(15, _)));
            sdf = fminf(sdf, fmaxf(fmaxf(abs(x - box_center_x) - box_size_x / 2, abs(y - box_center_y) - box_size_y / 2), z - h - box_size_z));
        }
    }
    float radius = 1e9;
    if (hash_to_float(myhash(seed, 5)) < cylinder_top_prob) {
        float h3 = last_h * 0.2 * hash_to_float(myhash(seed, 8));
        h3 = ceil(h3 / floor_height) * floor_height;
        topbox_h = max(topbox_h, h3);
        radius = min(top_size_x, top_size_y) / 2 * (0.5 + 0.5 * hash_to_float(myhash(seed, 6)));
        sdf = fminf(sdf, fmaxf(sqrt(dx*dx + dy*dy) - radius, z - h - h3));
    }
    if (!has_box && hash_to_float(myhash(seed, 9)) < sphere_top_prob) {
        float dz = z - h - topbox_h;
        radius = min(min(top_size_x, top_size_y) / 2, radius) * (0.5 + 0.5 * hash_to_float(myhash(seed, 10)));
        sdf = fminf(sdf, sqrt(dx*dx + dy*dy + dz*dz) - radius);
    }
    if (max_h != NULL) *max_h = h + topbox_h;
    return sdf;
}

DEVICE_FUNC void sdf_city(
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
    float max_h;
    *sdf = building(
        &max_h,
        position.x - block_x0, position.y - block_y0, position.z,
        building_center_x, building_center_y, building_seed,
        block_size, floor_height, -sdf_a,
        block_size / n_building_x - road_width,
        block_size / n_building_y - road_width,
        height_range_min, height_range_max,
        second_sec_prob, cylinder_top_prob, sphere_top_prob, topbox_prob
    );
    if (auxs != NULL) {
        auxs[0] = 0;
        auxs[1] = float(building_seed);
    }
    float frac_height = position.z / floor_height - floor(position.z / floor_height);

    float window_portion_v = 0.5 + 0.5 * hash_to_float(myhash(building_seed, 3));
    float window_portion_h = 0.5 + 0.3 * hash_to_float(myhash(building_seed, 4));

    if (frac_height > 0.5-window_portion_v/2 && frac_height < 0.5+window_portion_v/2 && position.z < max_h - floor_height * 0.15) {
        float dx = (position.x - block_x0 - building_center_x) / ((block_size / n_building_x - road_width) / 10);
        float dy = (position.y - block_y0 - building_center_y) / ((block_size / n_building_y - road_width) / 10);
        float dz = position.z / floor_height;
        float window_id = float(myhash(myhash(myhash(building_seed, int(floor(dz))), int(floor(dx))), int(floor(dy))));
        if (myhash(building_seed, 1) % 5 != 0) {
            if (dx - floor(dx) >= 0.5-window_portion_h/2 && dx - floor(dx) <= 0.5+window_portion_h/2) {
                // auxs[0] = 1;
                // auxs[1] = window_id;
                *sdf += 0.1;
            }
            if (dy - floor(dy) >= 0.5-window_portion_h/2 && dy - floor(dy) <= 0.5+window_portion_h/2) {
                // auxs[0] = 1;
                // auxs[1] = window_id;
                *sdf += 0.1;
            }
        }
        else {
            // auxs[0] = 1;
            // auxs[1] = window_id;
            *sdf += 0.1;
        }
    }

}