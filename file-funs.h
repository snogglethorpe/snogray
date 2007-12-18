// file-funs.h -- Functions for operating on files
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FILE_FUNS_H__
#define __FILE_FUNS_H__


#include <string>


namespace snogray {


// Return true if a file called FILE_NAME is readable.
//
extern bool file_exists (const std::string &file_name);

// Choose a "backup filename" (using the GNU convention of suffixes like
// ".~1~", ".~2~" etc), and rename FILE_NAME to it.  The backup filename
// is returned.  If this cannot be done an exception is thrown.
//
extern std::string rename_to_backup_file (const std::string &file_name,
					  unsigned backup_limit = 100);


}

#endif // __FILE_FUNS_H__


// arch-tag: 7fbb5346-812e-461b-9308-5de6b02d32ab
