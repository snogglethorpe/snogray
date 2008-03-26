// tuple-adaptor.h -- Converter between types and tuples
//
//  Copyright (C) 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TUPLE_ADAPTOR_H__
#define __TUPLE_ADAPTOR_H__


namespace snogray {


// An adaptor for converting values of type T to/from tuples of type DT*.
//
// This is the generic version which works for scalar values that are
// convertible to/from DT.  Other classes can define their own
// TupleAdaptors.
//
template<typename T, typename DT>
class TupleAdaptor
{
public:

  static const unsigned TUPLE_LEN = 1;

  TupleAdaptor (DT *_tuple) : tuple (_tuple) { }

  operator T () const { return T (*tuple); }
  TupleAdaptor &operator= (const T &val) { *tuple = DT (val); return *this; }

private:

  DT *tuple;
};


}

#endif // __TUPLE_ADAPTOR_H__
