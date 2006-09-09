// cook-torrance.cc -- Cook-Torrance reflectance function
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <complex>
#include <list>

#include "snogmath.h"
#include "vec.h"
#include "excepts.h"
#include "trace.h"
#include "intersect.h"

#include "cook-torrance.h"

using namespace Snogray;
using namespace std;

// Source of "constant" (not-to-be-freed) CookTorrance BRDFs
//
const CookTorrance *
Snogray::cook_torrance (const Color &spec_col, float m, const Ior &ior)
{
  static std::list<CookTorrance> brdfs;

  for (std::list<CookTorrance>::iterator b = brdfs.begin ();
       b != brdfs.end (); b++)
    if (b->m == m && b->ior == ior && b->specular_color == spec_col)
      return &(*b);

  brdfs.push_front (CookTorrance (spec_col, m, ior));

  return &brdfs.front ();
}

// Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
// For best results, they should be distributed according to the BRDF's
// importance function.
//
void
CookTorrance::gen_samples (const Intersect &, SampleRayVec &)
  const
{
  throw std::runtime_error ("CookTorrance::gen_samples");
}

// Modify the value of each of the light-samples in SAMPLES according to
// the BRDF's reflectivity in the sample's direction.
//
void
CookTorrance::filter_samples (const Intersect &isec, SampleRayVec &,
			      SampleRayVec::iterator from,
			      SampleRayVec::iterator to)
  const
{
  // 1 / (N dot V)
  //
  // If NV == 0, then the eye-ray is perpendicular to the normal, which
  // basically means we can't see anything (but it's a rare case so don't
  // bother to optimize it, just protect against division by zero).
  //
  float nv_inv = isec.nv == 0 ? 0 : 1 / isec.nv;

  // Info for calculating the Fresnel term.
  //
  const Fresnel fres (isec.trace.medium ? isec.trace.medium->ior : 1, ior);

  for (SampleRayVec::iterator s = from; s != to; s++)
    if (s->val != 0)
      {
	// The Cook-Torrance specular term is:
	//
	//    f_s = (F / PI) * (D * G / (N dot V))
	//
	// We calculate each of these sub-terms below

	// Light-ray direction vector (normalized)
	//
	const Vec &l = s->dir;
	float nl = dot (isec.n, l);

	// Half-way vector between eye-ray and light-ray (normalized)
	//
	const Vec h = (isec.v + l).unit ();
	float nh = dot (isec.n, h);

	// Calculate D (microfacet distribution) term:
	//
	//    D = (1 / (4 * m^2 * (cos alpha)^2)) * e^(-((tan alpha) / m)^2)
	//
	// where alpha is the angle between N and H.
	//
	float cos_alpha = max (min (nh, 1.f), -1.f);
	float cos_4_alpha = cos_alpha * cos_alpha * cos_alpha * cos_alpha;

	float D;
	if (cos_4_alpha < Eps)
	  D = 0;			// cos_alpha is too small, avoid underflow
	else
	  {
	    float tan_alpha = tan (acos (cos_alpha));
	    float D_exp = exp (-tan_alpha * tan_alpha * m_2_inv);
	    D = m_2_inv / (4 * cos_4_alpha) * D_exp;
	  }

	// Calculate F (fresnel) term
	//
	float F = fres.reflectance (nl);

	// Calculate G (microfacet masking/shadowing) term
	//
	//    G = min (1,
	//             2 * (N dot H) * (N dot V) / (V dot H),
	//             2 * (N dot H) * (N dot L) / (V dot H))
	//
	float vh = dot (isec.v, h);
	float G = 2 * nh * ((isec.nv > nl) ? nl : isec.nv) / vh;
	G = G <= 1 ? G : 1;

	float specular = F * D * G * nv_inv * M_1_PI;
	float diffuse = nl * M_1_PI; // standard lambertian diffuse term

	// The final reflectance is:
	//
	//   f = k_d * f_d + k_s * f_s
	//
	s->set_refl (isec.color * diffuse + specular_color * specular);
      }
}

// arch-tag: a0a0049e-9af6-4438-ab58-081630151122
