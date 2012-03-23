-- snogray.lua -- Lua scene interface for snogray
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
local snogray = {}


-- Imports
--
local table = require 'snogray.table'
local swig = require 'snogray.swig'
local raw = require "snogray.snograw"
local color = require 'snogray.color'
local texture = require 'snogray.texture'
local material = require 'snogray.material'
local surface = require 'snogray.surface'
local light = require 'snogray.light'
local transform = require 'snogray.transform'


-- Users typically have the snogray module as their default global
-- environment, so it needs to also export standard globals too.
--
setmetatable (snogray, { __index = _G })


----------------------------------------------------------------
-- Vector/position/bounding-box manipulation
--

local pos, vec = raw.Pos, raw.Vec

snogray.pos = pos
snogray.vec = vec
snogray.bbox = raw.BBox
snogray.ray = raw.Ray
snogray.uv = raw.UV
snogray.frame = raw.Frame

-- Handy scene origin position.
--
snogray.origin = pos (0, 0, 0)

snogray.midpoint = raw.midpoint
snogray.dot = raw.dot
snogray.cross = raw.cross


----------------------------------------------------------------
-- compat color support
--

snogray.color = color.std


----------------------------------------------------------------
-- compat texture support
--

snogray.image_tex = texture.image
snogray.mono_image_tex = texture.mono_image

snogray.grey_tex = texture.grey
snogray.intens_tex = texture.intens

snogray.check_tex = texture.check
snogray.check3d_tex = texture.check3d

snogray.perturb_pos_tex = texture.perturb_pos
snogray.perturb_uv_tex = texture.perturb_uv

snogray.linterp_tex = texture.linterp
snogray.sinterp_tex = texture.sinterp

snogray.rescale_tex = texture.rescale

snogray.plane_map_tex = texture.plane_map
snogray.cylinder_map_tex = texture.cylinder_map
snogray.lat_long_map_tex = texture.lat_long_map

snogray.perlin_tex = texture.perlin
snogray.perlin_abs_tex = texture.perlin_abs

snogray.x_tex = texture.x
snogray.y_tex = texture.y
snogray.z_tex = texture.z
snogray.u_tex = texture.u
snogray.v_tex = texture.v

snogray.worley_tex = texture.worley
snogray.worley_id_tex = texture.worley_id

snogray.xform_tex = texture.xform
snogray.scale_tex = texture.scale
snogray.rotate_tex = texture.rotate
snogray.rot_tex = texture.rot

snogray.arith_tex = texture.arith
snogray.mul_tex = texture.mul
snogray.add_tex = texture.add
snogray.sub_tex = texture.sub
snogray.div_tex = texture.div
snogray.mod_tex = texture.mod
snogray.pow_tex = texture.pow
snogray.floor_tex = texture.floor
snogray.ceil_tex = texture.ceil
snogray.trunc_tex = texture.trunc
snogray.min_tex = texture.min
snogray.max_tex = texture.max
snogray.avg_tex = texture.avg
snogray.mirror_tex = texture.mirror
snogray.abs_tex = texture.abs
snogray.neg_tex = texture.neg
snogray.sin_tex = texture.sin
snogray.cos_tex = texture.cos
snogray.tan_tex = texture.tan
snogray.atan2_tex = texture.atan2

snogray.cmp_tex = texture.cmp

snogray.eq_tex = texture.eq
snogray.ne_tex = texture.ne
snogray.lt_tex = texture.lt
snogray.le_tex = texture.le
snogray.gt_tex = texture.gt
snogray.ge_tex = texture.ge

snogray.fourier_series_tex = texture.fourier_series
snogray.perlin_series_tex = texture.perlin_series
snogray.perlin_abs_series_tex = texture.perlin_abs_series


----------------------------------------------------------------
-- compat material support
--

snogray.is_ior = material.is_ior
snogray.ior = material.ior
snogray.is_ior_spec = material.is_ior_spec
snogray.lambert = material.lambert
snogray.cook_torrance = material.cook_torrance
snogray.mirror = material.mirror
snogray.glass = material.glass
snogray.thin_glass = material.thin_glass
snogray.glow = material.glow
snogray.stencil = material.stencil


----------------------------------------------------------------
-- compat transform support
--

snogray.xform = transform.matrix
snogray.identity_xform = transform.identity
snogray.is_xform = transform.is_transform
snogray.scale = transform.scale
snogray.translate = transform.translate
snogray.rotate = transform.rotate
snogray.rotate_x = transform.rotate_x
snogray.rotate_y = transform.rotate_y
snogray.rotate_z = transform.rotate_z
snogray.basis_xform = transform.basis_xform

snogray.trans = snogray.translate
snogray.rot = snogray.rotate
snogray.rot_x = snogray.rotate_x
snogray.rot_y = snogray.rotate_y
snogray.rot_z = snogray.rotate_z

snogray.xform_z_to_y = transform.z_to_y
snogray.xform_y_to_z = transform.y_to_z
snogray.xform_x_to_y = transform.x_to_y
snogray.xform_y_to_x = transform.y_to_x

snogray.xform_flip_x = transform.flip_x
snogray.xform_flip_y = transform.flip_y
snogray.xform_flip_z = transform.flip_z


----------------------------------------------------------------
-- compat surface support
--

snogray.tessel_sphere = surface.tessel_sphere
snogray.tessel_sinc = surface.tessel_sinc
snogray.tessel_torus = surface.tessel_torus

snogray.mesh = surface.mesh
snogray.mesh_vertex_group = surface.mesh_vertex_group
snogray.mesh_vertex_normal_group = surface.mesh_vertex_normal_group

snogray.normalize_xform = surface.normalize_xform
snogray.y_base_normalize_xform = surface.y_base_normalize_xform
snogray.normalize = surface.normalize_mesh

snogray.sphere = surface.sphere
snogray.sphere2 = surface.sphere2

snogray.tripar = surface.tripar
snogray.triangle = surface.triangle
snogray.parallelogram = surface.parallelogram
snogray.rectangle = surface.rectangle

snogray.parallelepiped = surface.parallelepiped
snogray.cube = surface.cube

snogray.ellipse = surface.ellipse

snogray.cylinder = surface.cylinder
snogray.solid_cylinder = surface.solid_cylinder

snogray.model = surface.model
snogray.instance = surface.instance

snogray.surface_group = surface.group


----------------------------------------------------------------
-- compat light functions
--

snogray.point_light = light.point
snogray.sphere_light = light.sphere
snogray.triangle_light = light.triangle
snogray.far_light = light.far
snogray.envmap_light = light.envmap


----------------------------------------------------------------
-- Images
--

snogray.image = raw.image

snogray.envmap = raw.envmap


----------------------------------------------------------------
-- compat array functions

snogray.binary_search = table.binary_search
snogray.linear_interp_lookup = table.linear_interp_lookup


----------------------------------------------------------------
-- High-level rendering functions (mainly used by the top-level driver)
--

snogray.scene = raw.Scene
snogray.camera = raw.Camera

snogray.octree_builder_factory = raw.OctreeBuilderFactory


----------------------------------------------------------------

-- return module
--
return snogray
