// file-funs.cc -- Functions for operating on files
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

#include <fstream>
#include <cerrno>
#include <cstring>

#include "string-funs.h"
#include "excepts.h"

#include "file-funs.h"


using namespace snogray;
using namespace std;


// Return true if a file called FILE_NAME is readable.
//
bool 
snogray::file_exists (const string &file_name)
{
  ifstream test (file_name.c_str ());
  bool exists = !!test;
  return exists;
}

// Choose a "backup filename" (using the GNU convention of suffixes like
// ".~1~", ".~2~" etc), and rename FILE_NAME to it.  The backup filename
// is returned.  If this cannot be done an exception is thrown.
//
string
snogray::rename_to_backup_file (const string &file_name, unsigned backup_limit)
{
  string backup_name;

  unsigned backup_num;
  for (backup_num = 1; backup_num < backup_limit; backup_num++)
    {
      backup_name = file_name + ".~" + stringify (backup_num) + "~";
      if (! file_exists (backup_name))
	break;
    }
  if (backup_num == backup_limit)
    throw runtime_error (file_name + ": Too many backup files already exist");

  if (rename (file_name.c_str(), backup_name.c_str()) != 0)
    throw runtime_error (backup_name + ": " + strerror (errno));

  return backup_name;
}


// arch-tag: 3ebecb5b-999a-4574-ae71-08b47ccf14e3
