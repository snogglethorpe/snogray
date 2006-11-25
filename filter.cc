// filter.cc -- Filter datatype
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "excepts.h"
#include "val-table.h"
#include "mitchell-filt.h"
#include "gauss-filt.h"
#include "box-filt.h"

#include "filter.h"


using namespace Snogray;


// Return a new a filter depending on the parameters in PARAMS.
//
Filter *
Filter::make (const ValTable &params)
{
  std::string filter_type = params.get_string ("filter");

  if (filter_type.empty ())
    return new MitchellFilt ();
  else
    {
      ValTable filter_params;

      unsigned type_end = filter_type.find_first_of ("/");
      if (type_end < filter_type.length ())
	{
	  unsigned params_start
	    = filter_type.find_first_not_of ("/ \t", type_end + 1);

	  filter_params.parse (filter_type.substr (params_start));

	  filter_type = filter_type.substr (0, type_end);
	}

      if (filter_type == "none")
	return 0;
      else if (filter_type == "mitchell")
	return new MitchellFilt (filter_params);
      else if (filter_type == "gauss")
	return new GaussFilt (filter_params);
      else if (filter_type == "box")
	return new BoxFilt (filter_params);
      else
	throw std::runtime_error (filter_type + ": unknown output filter type");
    }
}

// arch-tag: b777ab5a-d4d2-44af-a23b-e8012cab289c
