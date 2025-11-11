# Copyright (C) 2023, Princeton University.
# This source code is licensed under the BSD 3-Clause license found in the LICENSE file in the root directory of this source tree.

# Authors: Zeyu Ma

# Inspired by https://www.shadertoy.com/view/4ttSWf by Inigo Quilez

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
class SdfTrees(Element):
    name = ElementNames.SdfTrees

    def __init__(
        self,
        device,
        attachment,
        material=Materials.TreeCollection,
        transparency=Transparency.Opaque,
        n_lattice=60,
        density=0.5,
        r_short=0.1,
        ratio=1.5,
        r_randomness=0.5,
        ratio_randomness=0.5,
        height_offset=0.5,
        noise_octaves=9,
        noise_scale=0.1,
        noise_freq=20,
        mask_octaves=11,
        mask_freq=0.05,
        mask_shift=-0.2,
    ):
        self.device = device
        seed = random_int()
        self.aux_names = ["tree_id"]
        self.int_params2 = attachment.int_params
        self.float_params2 = attachment.float_params

        self.int_params = AC(np.array([seed, n_lattice], dtype=np.int32))
        self.float_params = AC(
            np.array(
                [
                    density, r_short, ratio, r_randomness, ratio_randomness, height_offset,
                    noise_octaves, noise_scale, noise_freq, mask_octaves, mask_freq, mask_shift,
                ],
                dtype=np.float32,
            )
        )

        Element.__init__(self, "sdf_trees", material, transparency)
        self.tag = ElementTag.SdfTrees
