-- all-in-one.lua -- Old "all-in-one" Lua scene interface for snogray
--
--  Copyright (C) 2007-2012  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--

-- module
--
local all_in_one = {}


-- Imports
--
local table = require 'snogray.table'
local swig = require 'snogray.swig'
local raw = require "snogray.snograw"
local coord = require 'snogray.coord'
local color = require 'snogray.color'
local texture = require 'snogray.texture'
local material = require 'snogray.material'
local surface = require 'snogray.surface'
local light = require 'snogray.light'
local transform = require 'snogray.transform'


-- Users typically have the snogray module as their default global
-- environment, so it needs to also export standard globals too.
--
setmetatable (all_in_one, { __index = _G })


----------------------------------------------------------------
-- compat coord suport
--

all_in_one.pos = coord.pos
all_in_one.vec = coord.vec
all_in_one.bbox = coord.bbox
all_in_one.ray = coord.ray
all_in_one.uv = coord.uv
all_in_one.frame = coord.frame

all_in_one.origin = coord.origin

all_in_one.midpoint = coord.midpoint
all_in_one.dot = coord.dot
all_in_one.cross = coord.cross


----------------------------------------------------------------
-- compat color support
--

all_in_one.color = color.std


----------------------------------------------------------------
-- compat texture support
--

all_in_one.image_tex = texture.image
all_in_one.mono_image_tex = texture.mono_image

all_in_one.grey_tex = texture.grey
all_in_one.intens_tex = texture.intens

all_in_one.check_tex = texture.check
all_in_one.check3d_tex = texture.check3d

all_in_one.perturb_pos_tex = texture.perturb_pos
all_in_one.perturb_uv_tex = texture.perturb_uv

all_in_one.linterp_tex = texture.linterp
all_in_one.sinterp_tex = texture.sinterp

all_in_one.rescale_tex = texture.rescale

all_in_one.plane_map_tex = texture.plane_map
all_in_one.cylinder_map_tex = texture.cylinder_map
all_in_one.lat_long_map_tex = texture.lat_long_map

all_in_one.perlin_tex = texture.perlin
all_in_one.perlin_abs_tex = texture.perlin_abs

all_in_one.x_tex = texture.x
all_in_one.y_tex = texture.y
all_in_one.z_tex = texture.z
all_in_one.u_tex = texture.u
all_in_one.v_tex = texture.v

all_in_one.worley_tex = texture.worley
all_in_one.worley_id_tex = texture.worley_id

all_in_one.xform_tex = texture.xform
all_in_one.scale_tex = texture.scale
all_in_one.rotate_tex = texture.rotate
all_in_one.rot_tex = texture.rot

all_in_one.arith_tex = texture.arith
all_in_one.mul_tex = texture.mul
all_in_one.add_tex = texture.add
all_in_one.sub_tex = texture.sub
all_in_one.div_tex = texture.div
all_in_one.mod_tex = texture.mod
all_in_one.pow_tex = texture.pow
all_in_one.floor_tex = texture.floor
all_in_one.ceil_tex = texture.ceil
all_in_one.trunc_tex = texture.trunc
all_in_one.min_tex = texture.min
all_in_one.max_tex = texture.max
all_in_one.avg_tex = texture.avg
all_in_one.mirror_tex = texture.mirror
all_in_one.abs_tex = texture.abs
all_in_one.neg_tex = texture.neg
all_in_one.sin_tex = texture.sin
all_in_one.cos_tex = texture.cos
all_in_one.tan_tex = texture.tan
all_in_one.atan2_tex = texture.atan2

all_in_one.cmp_tex = texture.cmp

all_in_one.eq_tex = texture.eq
all_in_one.ne_tex = texture.ne
all_in_one.lt_tex = texture.lt
all_in_one.le_tex = texture.le
all_in_one.gt_tex = texture.gt
all_in_one.ge_tex = texture.ge

all_in_one.fourier_series_tex = texture.fourier_series
all_in_one.perlin_series_tex = texture.perlin_series
all_in_one.perlin_abs_series_tex = texture.perlin_abs_series


----------------------------------------------------------------
-- compat material support
--

all_in_one.is_ior = material.is_ior
all_in_one.ior = material.ior
all_in_one.is_ior_spec = material.is_ior_spec
all_in_one.lambert = material.lambert
all_in_one.cook_torrance = material.cook_torrance
all_in_one.mirror = material.mirror
all_in_one.glass = material.glass
all_in_one.thin_glass = material.thin_glass
all_in_one.glow = material.glow
all_in_one.stencil = material.stencil


----------------------------------------------------------------
-- compat transform support
--

all_in_one.xform = transform.matrix
all_in_one.identity_xform = transform.identity
all_in_one.is_xform = transform.is_transform
all_in_one.scale = transform.scale
all_in_one.translate = transform.translate
all_in_one.rotate = transform.rotate
all_in_one.rotate_x = transform.rotate_x
all_in_one.rotate_y = transform.rotate_y
all_in_one.rotate_z = transform.rotate_z
all_in_one.basis_xform = transform.basis_xform

all_in_one.trans = all_in_one.translate
all_in_one.rot = all_in_one.rotate
all_in_one.rot_x = all_in_one.rotate_x
all_in_one.rot_y = all_in_one.rotate_y
all_in_one.rot_z = all_in_one.rotate_z

all_in_one.xform_z_to_y = transform.z_to_y
all_in_one.xform_y_to_z = transform.y_to_z
all_in_one.xform_x_to_y = transform.x_to_y
all_in_one.xform_y_to_x = transform.y_to_x

all_in_one.xform_flip_x = transform.flip_x
all_in_one.xform_flip_y = transform.flip_y
all_in_one.xform_flip_z = transform.flip_z


----------------------------------------------------------------
-- compat surface support
--

all_in_one.tessel_sphere = surface.tessel_sphere
all_in_one.tessel_sinc = surface.tessel_sinc
all_in_one.tessel_torus = surface.tessel_torus

all_in_one.mesh = surface.mesh
all_in_one.mesh_vertex_group = surface.mesh_vertex_group
all_in_one.mesh_vertex_normal_group = surface.mesh_vertex_normal_group

all_in_one.normalize_xform = surface.normalize_xform
all_in_one.y_base_normalize_xform = surface.y_base_normalize_xform
all_in_one.normalize = surface.normalize_mesh

all_in_one.sphere = surface.sphere
all_in_one.sphere2 = surface.sphere2

all_in_one.tripar = surface.tripar
all_in_one.triangle = surface.triangle
all_in_one.parallelogram = surface.parallelogram
all_in_one.rectangle = surface.rectangle

all_in_one.parallelepiped = surface.parallelepiped
all_in_one.cube = surface.cube

all_in_one.ellipse = surface.ellipse

all_in_one.cylinder = surface.cylinder
all_in_one.solid_cylinder = surface.solid_cylinder

all_in_one.model = surface.model
all_in_one.instance = surface.instance

all_in_one.surface_group = surface.group


----------------------------------------------------------------
-- compat light functions
--

all_in_one.point_light = light.point
all_in_one.sphere_light = light.sphere
all_in_one.triangle_light = light.triangle
all_in_one.far_light = light.far
all_in_one.envmap_light = light.envmap


----------------------------------------------------------------

-- return module
--
return all_in_one
