// struct-light.h -- Abstract class for structured light sources
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
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
#include "uv.h"
#include "freelist.h"

#include "light.h"


namespace snogray {


// This is an a for 2d "structured" light sources, where the amount of
// radiation varies over the image surface.
//
class StructLight : public Light
{
private:

  class Region;

public:

  class Analyzer; // fwd decl

  // Basic constructor.
  //
  StructLight ()
    : root_region (0), num_sample_regions (0), inv_num_sample_regions (0)
  { }

  // Construct-and-analyze at the same time.
  //
  StructLight (const Analyzer &analyzer);

  // Analyze a light after construction.
  //
  void analyze (const Analyzer &analyzer);

  // Return the intensity at location U, V, and the pdf of this light's
  // intensity distribution at that point in PDF.
  //
  Color intensity (float u, float v, float &pdf) const
  {
    if (! root_region)
      {
	pdf = 0;
	return 0;
      }

    const Region *r = smallest_enclosing_region (u, v);

    pdf = inv_num_sample_regions * r->pdf_weight;

    return r->intensity;
  }

  // Return a location in this light sampled according to its intensity
  // distribution, remapped from the uniform distribution U, V.  Also
  // return the actual intensity and pdf at the returned location, in
  // INTENS and PDF.
  //
  UV intensity_sample (float u, float v, Color &intens, float &pdf) const
  {
    if (! root_region)
      {
	intens = 0;
	pdf = 0;
	return UV (0, 0);
      }

    // U is not supposed to ever be exactly 1, but in practice it can be
    // (likely due to precision problems in conversion), so tweak it in
    // that case.
    //
    if (u >= 1)
      u = 0.99999f;

    // Map each input UV to a region by treating the U coordinate as
    // an index into a vector of (pointers to) leaf regions.  This
    // weights each leaf region evenly, and since we make more
    // (smaller) regions where the intensity is high, this will
    // result in a rough distribution according to intensity.
    //
    float scaled_u = u * num_sample_regions;

    unsigned region_index = unsigned (scaled_u);
    Region *r = sample_regions[region_index];

    // Set the intensity and pdf (all points in a region have the
    // same values).
    //
    intens = r->intensity;
    pdf = inv_num_sample_regions * r->pdf_weight;

    // Use U, V to choose a specific point within R.  We discard
    // the information from U which was used to choose the region.
    //
    float u_offs = (scaled_u - floor (scaled_u)) * r->u_sz;
    float v_offs = v * r->v_sz;

    return UV (r->u_min + u_offs, r->v_min + v_offs);
  }

  // Dump a picture of the generated light regions to a file called
  // FILENAME.  ORIG_IMAGE should be the original image from which this
  // light was created.
  //
  void dump (const std::string &filename, const Image &orig_image) const;

  // Get some statistics about this light.
  //
  void get_stats (unsigned &num_sample_regions, unsigned &num_leaf_regions,
		  Color &mean_intensity)
    const;

private:

  // A region in a hierarchy of regions covering the struct.
  //
  struct Region
  {
    // Ways in which a region can be divided into sub-regions.
    //
    enum Kind { LEAF, U_SPLIT, V_SPLIT };

    // Make a leaf region.
    //
    Region (const Color &_intensity, float leaf_weight,
	    float _u_min, float _v_min, float _u_sz, float _v_sz,
	    const StructLight *_light)
      : intensity (_intensity),
	u_min (_u_min), v_min (_v_min), u_sz (_u_sz), v_sz (_v_sz),
	pdf_weight (leaf_weight / (u_sz * v_sz)),
	kind (LEAF), sub_region_0 (0), sub_region_1 (0), light (_light)
    { }

    // Make a split region.
    //
    Region (Kind _kind, Region *sub0, Region *sub1, const StructLight *_light)
      : intensity (0),
	u_min (sub0->u_min), v_min (sub0->v_min),
	u_sz (_kind == U_SPLIT ? sub0->u_sz + sub1->u_sz : sub0->u_sz),
	v_sz (_kind == V_SPLIT ? sub0->v_sz + sub1->v_sz : sub0->v_sz),
	pdf_weight (0), kind (_kind),
	sub_region_0 (sub0), sub_region_1 (sub1), light (_light)
    { }

    // Copy constructor
    //
    Region (const Region &src)
      : intensity (src.intensity),
	u_min (src.u_min), v_min (src.v_min), u_sz (src.u_sz), v_sz (src.v_sz),
	pdf_weight (src.pdf_weight), kind (src.kind),
	sub_region_0 (src.sub_region_0), sub_region_1 (src.sub_region_1)
    { }

    void dump (const Image &orig_image, Image &image) const;

    // The average intensity of light coming from this region.  Only
    // valid for leaf regions.
    //
    Color intensity;

    // The coordinates this region covers.
    //
    float u_min, v_min, u_sz, v_sz;

    // A multiplier to get a leaf region's pdf.  A leaf's pdf is equal
    // to PDF_WEIGHT / NUM_SAMPLE_REGIONS.
    //
    float pdf_weight;

    // What kind of region this is.
    //
    Kind kind;

    // If KIND is not LEAF, the two sub-regions this region is
    // split into; otherwise 0.
    //
    Region *sub_region_0, *sub_region_1;

    // The light this region is in.
    //
    const StructLight *light;
  };

  // Add a new leaf region.
  //
  Region *add_region (const Color &intensity, float leaf_weight,
		      float u_min, float v_min, float u_sz, float v_sz)
  {
    // To correctly sample this region, choosing it from all leaf regions
    // with approximate probability (LEAF_WEIGHT / TOTAL_LEAF_WEIGHTS), we
    // make duplicate entries into the SAMPLE_REGIONS vector.  This only
    // works with integer values of LEAF_WEIGHT, so we round it here.
    //
    leaf_weight = floor (leaf_weight + 0.5f);

    // Always add to SAMPLE_REGIONS at least once.
    //
    if (leaf_weight == 0)
      leaf_weight = 1;

    Region *r
      = new (regions) Region (intensity, leaf_weight,
			      u_min, v_min, u_sz, v_sz, this);

    // Make LEAF_WEIGHT entries in SAMPLE_REGIONS pointing to R.  Since we
    // choose regions randomly from SAMPLE_REGIONS, this means R will be
    // chosen with probability LEAF_WEIGHT / NUM_SAMPLE_REGIONS.
    //
    while (leaf_weight > 0)
      {
	sample_regions.push_back (r);
	leaf_weight -= 1.f;
      }

    return r;
  }

  // Add a new non-leaf region.
  //
  Region *add_region (Region::Kind kind, Region *sub0, Region *sub1)
  {
    return new (regions) Region (kind, sub0, sub1, this);
  }

  // Return the smallest region which contains the coordinates U, V.
  //
  const Region *smallest_enclosing_region (float u, float v) const
  {
    const Region *r = root_region;

    if (! r)
      return 0;

    while (r->kind != Region::LEAF)
      if (r->kind == Region::U_SPLIT)
	r = (u < r->sub_region_1->u_min) ? r->sub_region_0 : r->sub_region_1;
      else
	r = (v < r->sub_region_1->v_min) ? r->sub_region_0 : r->sub_region_1;

    return r;
  }

  // All regions are allocated from here.
  //
  Freelist<Region> regions;

  // The root of the region tree, covering the entirety of this light's area.
  //
  Region *root_region;

  // A vector of (pointers to) all leaf regions.  There may be
  // duplicates, as some leaf-regions can be counted more than once.
  //
  std::vector<Region *> sample_regions;

  // Floating point count of leaf regions (to avoid the need for
  // conversion), and its inverse, 1 / NUM_SAMPLE_REGIONS.  Used in
  // calculating pdf etc.  These correspond to the number of entries in the
  // SAMPLE_REGIONS vector, including duplicates, not the number of unique
  // leaf regions.
  //
  float num_sample_regions, inv_num_sample_regions;
};


// "Analyzer" class for constructing StructLights

class StructLight::Analyzer
{
public:

  enum SplitDim { U_DIM, V_DIM };

  virtual ~Analyzer () { }

  // Analyzes the region (0,0) - (1,1) and adds a region-tree
  // to SLIGHT covering it.  Returns the root region.
  //
  Region *analyze (StructLight &slight) const
  {
    return analyze (0, 0, 1, 1, slight);
  }

  // Analyzes the region (U,V) - (U+U_SZ, V+V_SZ) and adds a region-tree
  // to SLIGHT covering it.  Returns the root region.
  //
  Region *analyze (float u, float v, float u_sz, float v_sz,
		   StructLight &slight)
    const;

  //
  // The following methods should be provided by subclasses.
  //

  // Return the intensity of the entire region (U, V) - (U+U_SZ, V+V_SZ)
  //
  virtual Color intensity (float u, float v, float u_sz, float v_sz)
    const = 0;

  // Return true if the region (U, V) - (U+U_SZ, V+V_SZ) should be
  // split.  If true is returned, then the axis and size on which to
  // split are returned in SPLIT_DIM and SPLIT_POINT respectively.  If
  // false is returned, then LEAF_WEIGHT is the "weight" of the
  // resulting region, indicating that it's LEAF_WEIGHT times as bright
  // as the region size would indicate.
  //
  virtual bool find_split_point (float u, float v, float u_sz, float v_sz,
				 SplitDim &split_dim, float &split_point,
				 float &leaf_weight)
    const = 0;
};



inline
StructLight::StructLight (const Analyzer &analyzer)
  : root_region (0)
{
  analyze (analyzer);
}


}

#endif /* __STRUCT_LIGHT_V_SZ__ */


// arch-tag: b3dd6b27-2ac3-40b1-b146-7f1883aa01bf
