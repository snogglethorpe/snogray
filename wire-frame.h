// wire-frame.h -- Wire-frame output support
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __WIRE_FRAME_H__
#define __WIRE_FRAME_H__

#include <cstring>
#include <cstdlib>

#include "surface.h"
#include "scene.h"
#include "camera.h"
#include "image-io.h"
#include "cmdlineparser.h"

namespace Snogray {

class WireFrameParams
{
public:

  WireFrameParams () : wire_color (1), tint (0.7), fill (0) { }

  Color wire_color;		// base color of wires
  float tint;			// amount of object color used
  float fill;			// intensity of inter-wire fill

  // Parse a wire-frame option argument
  //
  void parse (CmdLineParser clp)
  {
    const char *arg = clp.opt_arg ();
    const char *_color = strchr (arg, '/');
    const char *_fill = strchr (arg, ':');

    if (*arg == '.' || (*arg >= '0' && *arg <= '9'))
      tint = atof (arg);
    if (_fill)
      fill = atof (_fill + 1);
    if (_color)
      wire_color = atof (_color + 1);
  }
};

class WireFrameRendering
{
public:

  WireFrameRendering (const Scene &_scene, const Camera &_camera,
		      unsigned _width, unsigned _height,
		      unsigned _min_x, unsigned _min_y,
		      unsigned _lim_x, unsigned _lim_y,
		      const WireFrameParams &_params)
    : scene (_scene), camera (_camera), width (_width), height (_height),
      min_x (_min_x), min_y (_min_y), lim_x (_lim_x), lim_y (_lim_y),
      params (_params),
      y (_min_y),
      cur_row (new Color[_lim_x - _min_x]),
      prev_row (new Color[_lim_x - _min_x]),
      cur_surfaces (new const Surface *[_lim_x - _min_x]),
      prev_surfaces (new const Surface *[_lim_x - _min_x])
  {
    if (lim_x <= min_x)
      lim_x = width;
    if (lim_y <= min_y)
      lim_y = height;

    for (unsigned x_offs = 0; x_offs < lim_x - min_x; x_offs++)
      {
	cur_row[x_offs] = prev_row[x_offs] = 0;
	cur_surfaces[x_offs] = prev_surfaces[x_offs] = 0;
      }
  }

  ~WireFrameRendering ()
  {
    delete cur_row;
    delete prev_row;
    delete cur_surfaces;
    delete prev_surfaces;
  }

  // Do wire-frame rendering of the current row from SCENE and CAMERA;
  // this will also will update the previously rendered row.
  //
  void render_row (Trace &trace)
  {
    for (unsigned x = min_x; x < lim_x; x++)
      render_pixel (x, y, trace);
  }

  // Copy the contents of the previously rendered row to OUTPUT_ROW.
  //
  void get_prev_row (ImageRow &output_row) const
  {
    for (unsigned x_offs = 0; x_offs < output_row.width; x_offs++)
      output_row[x_offs] = prev_row[x_offs];
  }

  // Advance one row, making the current row become the previous, and
  // starting a new current row.
  //
  void advance_row ()
  {
    // Swap the "current" and "previous" arrays.  We don't bother to
    // re-initialize the new current arrays because that is done
    // implicitly by render_row.

    Color *tmp_row = cur_row;
    cur_row = prev_row;
    prev_row = tmp_row;

    const Surface **tmp_surfaces = cur_surfaces;
    cur_surfaces = prev_surfaces;
    prev_surfaces = tmp_surfaces;

    // Advance position.
    //
    y++;
  }

private:

  // Calculate "wires" resulting from the transition from PREV_SURF, at
  // pixel-position PREV_X, PREV_Y, to SURF, which is pointed to by
  // CAMERA_RAY (the inconsistency in argument types merely reflects what
  // we know our caller to have handy).  If a "wire pixel" should be draw
  // as part of PREV_SURF, it is draw directly into PREV_COLOR.  A boolean
  // is returned saying whether a "wire pixel" should be drawn as part of
  // SURF.
  //
  bool do_transition (const Surface *prev_surf,
		      unsigned prev_x, unsigned prev_y,
		      const Surface *surf, const Ray &camera_ray,
		      Trace &trace,
		      Color &prev_color)
  {
    bool draw = false;

    if (surf != prev_surf)
      {
	// Different from preceeding pixel

	Ray ray (camera_ray, Scene::DEFAULT_HORIZON);

	if (surf && (!prev_surf || prev_surf->intersects (ray)))
	  //
	  // SURF is "on top" of PREV_SURF, or there is no PREV_SURF;
	  // Make the "wire" part of SURF.
	  //
	  draw = true;
	else
	  //
	  // Either SURF and PREV_SURF abut, or PREV_SURF ends here and was
	  // previously on top of SURF.  Make a wire part of PREV_SURF.
	  //
	  {
	    Ray prev_camera_ray = camera.get_ray (prev_x, prev_y, width, height);

	    prev_color = wire_color (prev_surf, prev_camera_ray, trace);

	    // If SURF abuts PREV_SURF -- that is, SURF was not previously
	    // underneath PREV_SURF -- then make a wire part of SURF too,
	    // unless they are the same color, or part of the same mesh
	    // (these latter two cases avoid lots of "double-width" wires
	    // being drawn inside meshes).
	    //
	    if (surf && surf->material() != prev_surf->material())
	      {
		Ray prev_ray (prev_camera_ray, Scene::DEFAULT_HORIZON);
		if (! surf->intersects (prev_ray))
		  draw = true;
	      }
	  }
      }

    return draw;
  }

  // Calculate the "wire frame color" of the scene at location X, Y
  //
  void render_pixel (unsigned x, unsigned y, Trace &trace)
  {
    // Wire-frame rendering: find surf object intersecting
    // camera ray, and see if it's different from the previous
    // pixel (in vertical or horizontal direction); if so, we
    // want to draw a "wire" to illustrate the transition.

    // Translate the image position X, Y into a ray radiating from
    // the camera.
    //
    Ray camera_ray = camera.get_ray (x, y, width, height);

    Ray intersected_ray (camera_ray, Scene::DEFAULT_HORIZON);

    unsigned x_offs = x - min_x;

    // Surface we found
    //
    IsecParams isec_params;
    const Surface *surf = scene.intersect (intersected_ray, isec_params, trace);

    // Set to true if we need to draw this point as part of a wire.
    //
    bool draw = false;

    // Do transition from vertically previous pixel
    //
    if (y > min_y
	&& do_transition (prev_surfaces[x_offs], x, y - 1, surf, camera_ray,
			  trace,
			  prev_row[x_offs]))
      draw = true;

    // Do transition from horizontally previous pixel
    //
    if (x_offs > 0
	&& do_transition (cur_surfaces[x_offs - 1], x - 1, y, surf, camera_ray,
			  trace, cur_row[x_offs - 1]))
      draw = true;

    if (draw)
      cur_row[x_offs] = wire_color (surf, camera_ray, trace);
    else if (params.fill > 0 && surf)
      cur_row[x_offs] = trace.render (camera_ray) * params.fill;
    else if (surf)
      cur_row[x_offs] = 0;
    else
      cur_row[x_offs] = scene.background (camera_ray);

    cur_surfaces[x_offs] = surf;
  }

  Color wire_color (const Surface *surf, const Ray &camera_ray, Trace &trace)
  {
    if (params.tint < 0.001)
      return params.wire_color;
    else
      {
	Color surf_color;
	if (params.fill > 0)
	  surf_color = trace.render (camera_ray);
	else
	  surf_color = surf->material()->color;

	return surf_color * params.tint + params.wire_color * (1 - params.tint);
      }
  }

  // Scene/camera we're rendering.
  //
  const Scene &scene;
  const Camera &camera;

  // Size of complete image (used to calculate camera angle correctly).
  //
  unsigned width, height;

  // Portion of image which will be actually rendered.
  //
  unsigned min_x, min_y, lim_x, lim_y;

  WireFrameParams params;

  // Current Y location, in range [MIN_Y, LIM_Y).
  //
  unsigned y;

  // Rendered colors of current and previous row.
  //
  Color *cur_row, *prev_row;

  // The surfaces found at each pixel location in the current and previous row.
  //
  const Surface **cur_surfaces, **prev_surfaces;
};

}

#endif /* __WIRE_FRAME_H__ */

// arch-tag: 227dfc57-de9b-4ef5-bc3d-d54d3ebd8ae0
