
// Main driver

int main (int argc, char *const *argv)
{
  unsigned aa_factor = 1, aa_overlap = 0;
  float (*aa_filter) (int offs, unsigned size) = ImageOutput::aa_gauss_filter;
  float target_gamma = 2.2;	// Only applies to formats that understand it
  const char *input_fmt = 0, *output_fmt = 0;	// 0 means auto-detect

  // Command-line option specs
  static struct option long_options[] = {
    { "aa-factor",	required_argument, 0, 'a' },
    { "aa-overlap",	required_argument, 0, 'A' },
    { "aa-filter",	required_argument, 0, 'F' },
    { "gamma",		required_argument, 0, 'g' },
    { "input-format",	required_argument, 0, 'I' },
    { "output-format",	required_argument, 0, 'O' },
    { 0, 0, 0, 0 }
  };
  CmdLineParser clp (argc, argv, "a:A:F:g:O:", long_options);

  // Parse command-line options
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
	// Anti-aliasing options
      case 'a':
	aa_factor = clp.unsigned_opt_arg ();
	break;
      case 'A':
	aa_overlap = clp.unsigned_opt_arg ();
	break;
      case 'F':
	{
	  const char *filt_name = clp.opt_arg ();
	  if (strcmp (filt_name, "box") == 0)
	    aa_filter = ImageOutput::aa_box_filter;
	  else if (strcmp (filt_name, "triang") == 0)
	    aa_filter = ImageOutput::aa_triang_filter;
	  else if (strcmp (filt_name, "gauss") == 0)
	    aa_filter = ImageOutput::aa_gauss_filter;
	  else
	    clp.opt_err ("requires an anti-aliasing filter name"
			 " (box, triang, gauss)");
	}
	break;

	// Output image options
      case 'g':
	target_gamma = clp.float_opt_arg ();
	break;
      case 'O':
	output_fmt = clp.opt_arg ();
	break;

	// Input image options
      case 'I':
	input_fmt = clp.opt_arg ();
	break;
      }

  if (clp.num_remaining_args() > 2)
    {
      cerr << "Usage: " << prog_name
	   << " [-a AA_FACTOR] [-g GAMMA]"
	   << " [INPUT_IMAGE_FILE [OUTPUT_IMAGE_FILE]]" << endl;
      exit (10);
    }
  const char *input_file = clp.get_arg ();  // 0 if none specified
  const char *output_file = clp.get_arg (); // 0 if none specified

  // Set output format automatically if necessary
  if  (! output_fmt)
    {
      const char *output_ext = rindex (output_file, '.');

      if (! output_ext)
	{
	  cerr << clp.err_pfx()
	       << output_file
	       << ": No filename extension to determine output type"
	       << endl;
	  exit (25);
	}

      output_fmt = ++output_ext;
    }

  // Make the output-format-specific parameter block
  ImageSinkParams *image_params = 0;
  bool target_gamma_used = false;
  if (strcmp (output_fmt, "png") == 0)
    {
      image_params
	= new PngImageSinkParams (output_file, limit_width, limit_height,
				  target_gamma);
      target_gamma_used = true;
    }
  else if (strcmp (output_fmt, "exr") == 0)
    image_params
      = new ExrImageSinkParams (output_file, limit_width, limit_height);
  else
    {
      cerr << clp.err_pfx() << output_file << ": Unknown output type" << endl;
      exit (26);
    }

  // Print image info
  if (! quiet)
    {
      cout << "image.size:	    "
	   << setw (11) << (stringify (final_width)
			    + " x " + stringify (final_height))
	   << endl;
      if (limit_x != 0 || limit_y != 0
	  || limit_width != final_width || limit_height != final_height)
	cout << "image.limit:"
	     << setw (19) << (stringify (limit_x) + "," + stringify (limit_y)
			      + " - " + stringify (limit_x + limit_width)
			      + "," + stringify (limit_y + limit_height))
	     << " (" << limit_width << " x "  << limit_height << ")"
	     << endl;

      if (target_gamma_used)
        cout << "image.target_gamma:        "
	     << setw (4) << target_gamma << endl;

      // Anti-aliasing info
      if ((aa_factor + aa_overlap) > 1)
	{
	  if (aa_factor > 1)
	    cout << "image.aa_factor:           "
		 << setw (4) << aa_factor << endl;

	  if (aa_overlap > 0)
	    cout << "image.aa_kernel_size:      "
		 << setw (4) << (aa_factor + aa_overlap*2)
		 << " (overlap = " << aa_overlap << ")" << endl;
	  else
	    cout << "image.aa_kernel_size:      "
		 << setw (4) << aa_factor << endl;

	  cout << "image.aa_filter:       " << setw (8);
	  if (aa_filter == ImageOutput::aa_box_filter)
	    cout << "box";
	  else if (aa_filter == ImageOutput::aa_triang_filter)
	    cout << "triang";
	  else if (aa_filter == ImageOutput::aa_gauss_filter)
	    cout << "gauss";
	  else
	    cout << "???";
	  cout << endl;
	}
    }

  // Set camera aspect ratio to give pixels a 1:1 aspect ratio
  camera.set_aspect_ratio ((float)width / (float)height);

  // Define our scene!
  define_scene (scene, camera);

  // Print scene info
  if (! quiet)
    {
      cout << "scene.num_objects:   "
	   << setw (10) << commify (scene.objs.size ()) << endl;
      cout << "scene.num_lights:    "
	   << setw (10) << commify (scene.lights.size ()) << endl;
      cout << "scene.num_materials:  "
	   << setw (9) << commify (scene.materials.size ()) << endl;
      cout << "scene.voxtree_num_nodes:"
	   << setw (7) << commify (scene.obj_voxtree.num_nodes ()) << endl;
      cout << "scene.voxtree_max_depth:"
	   << setw (7) << commify (scene.obj_voxtree.max_depth ()) << endl;
    }

  // The image we're creating
  ImageOutput image (*image_params, aa_factor, aa_overlap, aa_filter);

  // Limits in terms of higher-resolution pre-AA image
  unsigned hr_limit_x = limit_x * aa_factor;
  unsigned hr_limit_y = limit_y * aa_factor;
  unsigned hr_limit_max_x = hr_limit_x + limit_width * aa_factor;
  unsigned hr_limit_max_y = hr_limit_y + limit_height * aa_factor;

  if (! quiet)
    cout << endl;

  // Main ray-tracing loop
  for (unsigned y = hr_limit_y; y < hr_limit_max_y; y++)
    {
      ImageRow &output_row = image.next_row ();

      // Progress indicator
      if (progress)
	{
	  if (aa_factor > 1)
	    cout << "\rrendering: line "
		 << setw (5) << y / aa_factor
		 << "_" << (y - (y / aa_factor) * aa_factor);
	  else
	    cout << "\rrendering: line "
		 << setw (5) << y;
	  cout << " (" << (y - hr_limit_y) * 100 / (hr_limit_max_y - hr_limit_y)
	       << "%)";
	  cout.flush ();
	}

      for (unsigned x = hr_limit_x; x < hr_limit_max_x; x++)
	{
	  float u = (float)x / (float)width;
	  float v = (float)(height - y) / (float)height;
	  Ray camera_ray = camera.get_ray (u, v);

	  output_row[x - hr_limit_x] = scene.render (camera_ray);
	}
    }

  if (progress)
    {
      cout << "\rrendering: done              " << endl;
      if (! quiet)
	cout << endl;
    }

  // Print render stats
  if (! quiet)
    {
      Scene::Stats &sstats = scene.stats;
      Voxtree::Stats &vstats1 = sstats.voxtree_closest_intersect;
      Voxtree::Stats &vstats2 = sstats.voxtree_intersects;

      cout << "Stats:" << endl;

      cout << "  closest_intersect:" << endl;
      cout << "     scene calls:       "
	   << setw (14) << commify (sstats.scene_closest_intersect_calls)
	   << endl;
      cout << "     voxtree calls:     "
	   << setw (14) << commify (vstats1.tree_intersect_calls) << endl;
      cout << "     voxtree node calls:"
	   << setw (14) << commify (vstats1.node_intersect_calls) << endl;
      cout << "     obj calls:         "
	   << setw (14) << commify (sstats.obj_closest_intersect_calls) << endl;

      cout << "  intersects:" << endl;
      cout << "     scene calls:       "
	   << setw (14) << commify (sstats.scene_intersects_calls) << endl;
      cout << "     voxtree calls:     "
	   << setw (14) << commify (vstats2.tree_intersect_calls) << endl;
      cout << "     voxtree node calls:"
	   << setw (14) << commify (vstats2.node_intersect_calls) << endl;
      cout << "     obj calls:         "
	   << setw (14) << commify (sstats.obj_intersects_calls) << endl;
    }
}

// arch-tag: 9852837a-ecf5-4400-9b79-f0cca96a6736
