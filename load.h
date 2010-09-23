// load.h -- Snogray scene (etc) loader
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LOAD_H__
#define __LOAD_H__


namespace snogray {

class Scene;
class Camera;
class ValTable;


// Load from FILENAME into SCENE and CAMERA.  PARAMS contains various
// global configuration parameters, which can be examined or modified
// by loaders.
//
// FMT is the type of loader; if it is empty, then the file-extension
// of FILENAME is used instead.
//
extern void load_file (const std::string &filename, const std::string &fmt,
		       Scene &scene, Camera &camera, ValTable &params);


}

#endif // __LOAD_H__
