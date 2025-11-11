# Copyright (C) 2024, Princeton University.
# This source code is licensed under the BSD 3-Clause license found in the LICENSE file in the root directory of this source tree.

# Authors: Zeyu Ma

import gin
from numpy.random import uniform

from infinigen.core import surface
from infinigen.core.nodes.node_wrangler import Nodes, NodeWrangler
from infinigen.core.util.math import FixedSeed
from infinigen.core.util.organization import SurfaceTypes

type = SurfaceTypes.BlenderDisplacement
mod_name = "geo_sdftree"
name = "sdftree"

def shader_sdftree(nw: NodeWrangler):
    fac = nw.new_node(
        Nodes.VoronoiTexture,
        input_kwargs={
            "W": nw.new_node(
                Nodes.Attribute, attrs={"attribute_name": "tree_id"},
            ),
        },
        attrs={"voronoi_dimensions": "1D"},
    )
    color_ramp = nw.new_node(
        Nodes.ColorRamp, input_kwargs={"Fac": fac.outputs["Color"]}
    )
    color_ramp.color_ramp.elements[0].position = 0.376
    color_ramp.color_ramp.elements[0].color = [0.055, 0.125, 0.044, 1.0]
    color_ramp.color_ramp.elements[1].position = 1.0
    color_ramp.color_ramp.elements[1].color = [1.0, 0.759, 0.101, 1.0]

    radius = (0, 0, 0)
    principled_bsdf = nw.new_node(
        Nodes.PrincipledBSDF,
        input_kwargs={
            "Subsurface": 1.0000,
            "Subsurface Radius": radius,
            "Subsurface Color": color_ramp.outputs["Color"],
        },
    )
    material_output = nw.new_node(
        Nodes.MaterialOutput,
        input_kwargs={"Surface": principled_bsdf},
        attrs={"is_active_output": True},
    )
    return principled_bsdf

@gin.configurable
def geo_sdftree(nw: NodeWrangler, random_seed=0, selection=None):
    group_input = nw.new_node(
        Nodes.GroupInput, expose_input=[("NodeSocketGeometry", "Geometry", None)]
    )
    group_output = nw.new_node(
        Nodes.GroupOutput,
        input_kwargs={"Geometry": group_input.outputs["Geometry"]},
        attrs={"is_active_output": True},
    )


def apply(obj, selection=None, **kwargs):
    surface.add_geomod(obj, geo_sdftree, selection=selection)
    surface.add_material(obj, shader_sdftree, selection=selection)
