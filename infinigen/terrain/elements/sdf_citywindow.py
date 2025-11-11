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
class SdfCityWindow(Element):
    name = ElementNames.SdfCityWindow

    def __init__(
        self,
        device,
        attachment,
        citybody,
        material=Materials.CityWindow,
        transparency=Transparency.IndividualTransparent,
    ):
        self.device = device
        self.aux_names = ["window_id", "window_id2"]
        self.int_params2 = attachment.int_params
        self.float_params2 = attachment.float_params

        self.int_params = AC(np.array([citybody.seed, citybody.row_range[0], citybody.row_range[1]], dtype=np.int32))
        self.float_params = AC(
            np.array(
                [
                    citybody.block_size,
                    citybody.road_width,
                    citybody.floor_height,
                    citybody.height_range[0],
                    citybody.height_range[1],
                    citybody.second_sec_prob,
                    citybody.cylinder_top_prob,
                    citybody.sphere_top_prob,
                    citybody.topbox_prob,
                ],
                dtype=np.float32,
            )
        )

        Element.__init__(self, "sdf_citywindow", material, transparency)
        self.tag = ElementTag.SdfCity
