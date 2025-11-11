# Copyright (C) 2023, Princeton University.
# This source code is licensed under the BSD 3-Clause license found in the LICENSE file in the root directory of this source tree.

# Authors: Alexander Raistrick, Zeyu Ma


import gin
import bpy
from infinigen.core import surface
from infinigen.core.nodes.node_wrangler import Nodes
from infinigen.core.util import color
from infinigen.core.util.random import random_general as rg

type = None


@gin.configurable
def shader_atmosphere(
    nw, enable_scatter=True, density=("uniform", 0, 0.006), anisotropy=0.5, dynamic=False, **kwargs
):
    nw.force_input_consistency()
    density = rg(density)

    principled_volume = nw.new_node(
        Nodes.PrincipledVolume,
        input_kwargs={
            "Color": color.color_category("fog"),
            "Density": density,
            "Anisotropy": rg(anisotropy),
        },
    )

    if dynamic:
        fc = principled_volume.inputs['Density'].driver_add('default_value')
        drv = fc.driver
        drv.expression = "0.002*(frame - 160)/320 * (frame > 160)"
        var = drv.variables.new()
        var.name = "frame"
        var.targets[0].id_type = 'SCENE'
        scene = bpy.context.scene
        var.targets[0].id = scene
        var.targets[0].data_path = "frame_current"

    return (None, principled_volume)


def apply(obj, selection=None, **kwargs):
    surface.add_material(obj, shader_atmosphere, selection=selection)
