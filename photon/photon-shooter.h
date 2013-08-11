// photon-shooter.h -- Photon-shooting infrastructure
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_PHOTON_SHOOTER_H
#define SNOGRAY_PHOTON_SHOOTER_H

#include <vector>

#include "photon.h"


namespace snogray {


// A class used to shoot photons, for building photon maps.  This is an
// abstract class, and must be subclassed.
//
class PhotonShooter
{
public:

  PhotonShooter (const std::string &_name) : name (_name) { }

  // A set of photons deposited during shooting.  Subclasses usually
  // have one or more PhotonSets which they are filling in.
  //
  class PhotonSet;

  // Shoot photons from the lights, depositing them in photon-sets
  // at appropriate points by calling PhotonSet::deposit.
  //
  void shoot (const GlobalRenderState &global_render_state);

  // Deposit (or ignore) the photon PHOTON in some photon-set.
  // ISEC is the intersection where the photon is being stored, and
  // BSDF_HISTORY is the bitwise-or of all BSDF past interactions
  // since this photon was emitted by the light (it will be zero for
  // the first intersection).
  //
  // This method must be defined by subclasses.
  //
  virtual void deposit (const Photon &photon,
			const Intersect &isec, unsigned bsdf_history)
    = 0;

  // Return true if all photon-sets are complete.
  //
  bool complete () const;

  unsigned target_count () const;
  unsigned cur_count () const;

  // Pointers to photon-sets being filled in by this shooter.  The
  // actual photon-sets are located elsewhere (probably as a field
  // in a subclass).
  //
  std::vector<PhotonSet *> photon_sets;

  // Name of this photon-shooter, used for progress/status messages.
  // Subclasses probably want to set this to something appropriate.
  //
  std::string name;
};


// A set of photons deposited during shooting.  Subclasses usually
// have one or more PhotonSets which they are filling in.
//
class PhotonShooter::PhotonSet
{
public:

  PhotonSet (unsigned _target_count, const std::string &_name,
	     PhotonShooter &shooter)
    : num_paths (0), target_count (_target_count), name (_name)
  {
    shooter.photon_sets.push_back (this);
  }

  // Return true if this set has reached its target photon count.
  //
  bool complete () const { return photons.size () == target_count; }

  // Deposited photons;
  //
  std::vector<Photon> photons;

  // Number of paths tried so far in generating this set.  This will
  // be incremented for each new path until this set is declare done
  // by setting the field PhotonSet::done to true.
  //
  unsigned num_paths;

  // Number of photons we'd like to generate for this set.
  //
  unsigned target_count;

  // Name of this set; this is used for generating messages after
  // shooting.
  //
  std::string name;
};


// Return true if all photon-sets are complete.
//
inline bool
PhotonShooter::complete () const
{
  for (std::vector<PhotonSet *>::const_iterator psi = photon_sets.begin();
       psi != photon_sets.end(); ++psi)
    if (! (*psi)->complete ())
      return false;
  return true;
}

inline unsigned
PhotonShooter::target_count () const
{
  unsigned count = 0;
  for (std::vector<PhotonSet *>::const_iterator psi = photon_sets.begin();
       psi != photon_sets.end(); ++psi)
    count += (*psi)->target_count;
  return count;
}

inline unsigned
PhotonShooter::cur_count () const
{
  unsigned count = 0;
  for (std::vector<PhotonSet *>::const_iterator psi = photon_sets.begin();
       psi != photon_sets.end(); ++psi)
    count += (*psi)->photons.size ();
  return count;
}


}

#endif // SNOGRAY_PHOTON_SHOOTER_H
