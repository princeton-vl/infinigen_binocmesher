# Copyright (C) 2023, Princeton University.
# This source code is licensed under the BSD 3-Clause license found in the LICENSE file in the root directory of this source tree.

# Authors: Lingjie Mei

from numpy.random import uniform

from infinigen.assets.materials import common
from infinigen.core.nodes.node_info import Nodes
from infinigen.core.nodes.node_wrangler import NodeWrangler
from infinigen.core.util.color import hsv2rgba
from infinigen.core import surface
from infinigen.core.util.organization import SurfaceTypes

import gin


type = SurfaceTypes.BlenderDisplacement
mod_name = "geo_citywindow"
name = "citywindow"

def shader_citywindow(nw: NodeWrangler, **kwargs):
    attribute = nw.new_node(
        Nodes.Attribute,
        attrs={"attribute_name": "window_id"},
    )
    mod_res = nw.new_node(
        Nodes.Math,
        [attribute, 3],
        attrs={"operation": "MODULO"},
    )
    g1 = nw.new_node(
        Nodes.Math,
        [mod_res, 0],
        attrs={"operation": "COMPARE"},
    )
    frame = nw.new_node(Nodes.Value)
    fcurve  = frame.outputs[0].driver_add("default_value")
    driver  = fcurve.driver
    driver.type = 'SCRIPTED'
    driver.expression = "frame"

    frame_threshold = nw.scalar_add(
        240,
        nw.new_node(
            Nodes.Math,
            [attribute, 240],
            attrs={"operation": "MODULO"},
        )
    )
    g2 = nw.new_node(Nodes.Math, [frame, frame_threshold], attrs={"operation": "GREATER_THAN"})
    factor = nw.scalar_multiply(g1, g2)
    mod_res2 = nw.new_node(
        Nodes.Math,
        [attribute, 5],
        attrs={"operation": "MODULO"},
    )
    factor0 = nw.new_node(
        Nodes.Math,
        [mod_res2, 0],
        attrs={"operation": "COMPARE"},
    )
    light_color = nw.new_node(Nodes.MixRGB, [factor0, (1, 1, 1, 1), (240/255, 237/255, 105/255, 1)])
    emission = nw.new_node(Nodes.MixRGB, [factor, (0, 0, 0, 1), light_color])
    principled_bsdf = nw.new_node(
        Nodes.PrincipledBSDF,
        input_kwargs={
            "Metallic": 1.0,
            "Roughness": 0.1,
            "Emission": emission,
        },
    )
    material_output = nw.new_node(
        Nodes.MaterialOutput,
        input_kwargs={"Surface": principled_bsdf},
        attrs={"is_active_output": True},
    )

    return principled_bsdf

@gin.configurable
def geo_citywindow(nw: NodeWrangler, random_seed=0, selection=None):
    group_input = nw.new_node(
        Nodes.GroupInput, expose_input=[("NodeSocketGeometry", "Geometry", None)]
    )
    group_output = nw.new_node(
        Nodes.GroupOutput,
        input_kwargs={"Geometry": group_input.outputs["Geometry"]},
        attrs={"is_active_output": True},
    )

def apply(obj, selection=None, clear=False, **kwargs):
    surface.add_geomod(obj, geo_citywindow, selection=selection)
    surface.add_material(obj, shader_citywindow, selection=selection)
