// string-funs.cc -- Random string helper functions
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "string-funs.h"

using namespace Snogray;
using namespace std;

// Return a string version of NUM
//
string 
Snogray::stringify (unsigned num)
{
  string str ((num > 9) ? stringify (num / 10) : "");
  char ch = (num % 10) + '0';
  str += ch;
  return str;
}

// Return a string version of NUM, with commas added every 3rd place
//
string 
Snogray::commify (unsigned long long num, unsigned sep_count)
{
  string str ((num > 9) ? commify (num / 10, sep_count % 3 + 1) : "");
  char ch = (num % 10) + '0';
  if (sep_count == 3 && num > 9)
    str += ',';
  str += ch;
  return str;
}

// arch-tag: c8a36b93-7176-431a-b46b-6cf51c7eff55
