# Copyright (C) 2023, Princeton University.
# This source code is licensed under the BSD 3-Clause license found in the LICENSE file in the root directory of this source tree.

# Authors: Zeyu Ma

# Inspired by https://www.shadertoy.com/view/XtsSWs by otaviogood

import gin
import numpy as np
from numpy import ascontiguousarray as AC

from infinigen.core.util.organization import (
    ElementNames,
    ElementTag,
    Materials,
    Tags,
    Transparency,
)
from infinigen.terrain.utils import random_int

from .core import Element


@gin.configurable
class SdfCity(Element):
    name = ElementNames.SdfCity

    def __init__(
        self,
        device,
        attachment,
        material=Materials.CityCollection,
        transparency=Transparency.Opaque,
        block_size=100,
        row_range=[10, 20],
        road_width=2,
        floor_height=1,
        height_range=[10, 20],
        second_sec_prob=0.5,
        cylinder_top_prob=0.3,
        sphere_top_prob=0.1,
        topbox_prob=0.3,
    ):
        self.device = device
        self.seed = seed = random_int()
        self.block_size = block_size
        self.row_range = row_range
        self.road_width = road_width
        self.floor_height = floor_height
        self.height_range = height_range
        self.second_sec_prob = second_sec_prob
        self.cylinder_top_prob = cylinder_top_prob
        self.sphere_top_prob = sphere_top_prob
        self.topbox_prob = topbox_prob
        
        self.aux_names = [Materials.CityWindow, "instance_id"]
        self.int_params2 = attachment.int_params
        self.float_params2 = attachment.float_params

        self.int_params = AC(np.array([seed, row_range[0], row_range[1]], dtype=np.int32))
        self.float_params = AC(
            np.array(
                [
                    block_size,
                    road_width,
                    floor_height,
                    height_range[0],
                    height_range[1],
                    second_sec_prob,
                    cylinder_top_prob,
                    sphere_top_prob,
                    topbox_prob,
                ],
                dtype=np.float32,
            )
        )

        Element.__init__(self, "sdf_city", material, transparency)
        self.tag = ElementTag.SdfCity
