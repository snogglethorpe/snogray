// struct-light.v_sz -- Abstract class for structured light sources
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __STRUCT_LIGHT_V_SZ__
#define __STRUCT_LIGHT_V_SZ__

#include <string>
#include "image.h"

#include "freelist.h"
#include "light.h"


namespace Snogray {


// This is an a for 2d "structured" light sources, where the amount of
// radiation varies over the image surface.
//
class StructLight : public Light
{
public:

  class Analyzer; // fwd decl

  // Basic constructor.  (U, V) - (U+U_SZ, V+V_SZ) is the cordinate the entire
  // 2d surface being structured.
  //
  StructLight (const Analyzer &analyzer);

  StructLight ()
    : u_min (0), v_min (0), u_sz (0), v_sz (0), root_region (0),
      num_leaf_regions (0)
  { }

  // Make a light after construction.
  //
  void analyze (const Analyzer &analyzer);

  // Generate some samples of this light and add them to SAMPLES; N is
  // the surface normal of the surface being illuminated.
  //
  void gen_region_samples (SampleRayVec &samples)
    const
  {
    if (root_region)
      root_region->gen_samples (u_min, v_min, u_sz, v_sz, samples, this);
  }

  // Dump a picture of the generated light regions to a file called
  // FILENAME.  ORIG_IMAGE should be the original image from which this
  // light was created.
  //
  void dump (const std::string &filename, const Image &orig_image) const;

  // A region in a hierarchy of regions covering the struct.
  //
  struct Region
  {
    // Ways in which a region can be divided into sub-regions.
    //
    enum SplitType { NO_SPLIT, U_SPLIT, V_SPLIT };

    // Make a leaf region
    //
    Region (const Color &_radiance)
      : radiance (_radiance),
	split_type (NO_SPLIT), split_point (0),
	sub_region_0 (0), sub_region_1 (0)
    { }

    // Make a split region
    //
    Region (SplitType _split_type, float _split_point,
	    Region *sub0, Region *sub1)
      : radiance (sub0->radiance + sub1->radiance),
	split_type (_split_type), split_point (_split_point),
	sub_region_0 (sub0), sub_region_1 (sub1)
    { }

    // Copy constructor
    //
    Region (const Region &src)
      : radiance (src.radiance),
	split_type (src.split_type), split_point (src.split_point),
	sub_region_0 (src.sub_region_0), sub_region_1 (src.sub_region_1)
    { }

    // Generate some samples from this region and its sub-regions, and
    // add them to SAMPLES; N is the surface normal of the surface
    // being illuminated.  The boundaries of the region are
    // (U_MIN,V_MIN) - (U_MIN+U_SZ, V_MIN+V_SZ).
    //
    void gen_samples (float u_min, float v_min, float u_sz, float v_sz,
		      SampleRayVec &samples, const StructLight *light)
      const;

    void dump (float u_min, float v_min, float u_sz, float v_sz,
	       const Image &orig_image, float scale, Image &image)
      const;

    // The amount of light coming from this region.
    // It is the sum (not average) of all pixels in the region.
    //
    Color radiance;

    // How this region is split into sub-regions;
    //
    SplitType split_type;

    // If SPLIT_TYPE is not NO_SPLIT, the size of sub_region_0
    //
    float split_point;

    // If SPLIT_TYPE is not NO_SPLIT, the two sub-regions this region is
    // split into; otherwise 0.
    //
    Region *sub_region_0, *sub_region_1;
  };

  Region *add_region (const Color &radiance)
  {
    num_leaf_regions++;
    return new (regions) Region (radiance);
  }
  Region *add_region (Region::SplitType split_type, float split_point,
		      Region *sub0, Region *sub1)
  {
    return new (regions) Region (split_type, split_point, sub0, sub1);
  }

  class Analyzer
  {
  public:

    enum SplitDim { U_DIM, V_DIM };

    virtual ~Analyzer () { }

    // Analyzes the region (U,V) - (U+U_SZ, V+V_SZ) and adds a region-tree
    // to SLIGHT covering it.  Returns the root region.
    //
    Region *analyze (float u, float v, float u_sz, float v_sz,
		     StructLight &slight)
      const;

    //
    // The following methods should be provided by subclasses.
    //

    // Return the radiance of the entire region (U, V) - (U+U_SZ, V+V_SZ)
    //
    virtual Color radiance (float u, float v, float u_sz, float v_sz) const = 0;

    // Get the coordinates which bound the root.
    //
    virtual void get_root_bounds (float &u_min, float &v_min,
				  float &u_sz, float &v_sz)
      const = 0;

    // Return true if the region (U, V) - (U+U_SZ, V+V_SZ) should be
    // split.  If true is returned, then the size and axis on which to
    // split are returned in SPLIT_POINT and SPLIT_DIM respectively.
    //
    virtual bool find_split_point (float u, float v, float u_sz, float v_sz,
				   float &split_point, SplitDim &split_dim)
      const = 0;
  };

  // The lower bound of the root region (in some abstract unit).
  //
  float u_min, v_min;

  // Size of this light (in some abstract unit).
  //
  float u_sz, v_sz;

  // All regions are allocated from here.
  //
  Freelist<Region> regions;

  // The root of the region tree, covering the entirety of this light's area.
  //
  Region *root_region;

  unsigned num_leaf_regions;
};

inline void
StructLight::analyze (const Analyzer &analyzer)
{
  // should free any non-null root_region  XXX

  analyzer.get_root_bounds (u_min, v_min, u_sz, v_sz);

  root_region = analyzer.analyze (u_min, v_min, u_sz, v_sz, *this);
}

inline
StructLight::StructLight (const Analyzer &analyzer)
  : u_min (0), v_min (0), u_sz (0), v_sz (0), root_region (0)
{
  analyze (analyzer);
}

}

#endif /* __STRUCT_LIGHT_V_SZ__ */


// arch-tag: b3dd6b27-2ac3-40b1-b146-7f1883aa01bf
