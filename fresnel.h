// fresnel.h -- Calculation of fresnel reflection
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FRESNEL_H__
#define __FRESNEL_H__

#include "snogmath.h"
#include "intersect.h"

namespace snogray {

// Complex index of refraction
//
class Ior
{
public:

  Ior (float _n, float _k = 0) : n (_n), k (_k) { }

  bool operator== (const Ior &ior2) const { return n == ior2.n && k == ior2.k; }

  // N and K are the real and imaginary components of the index of
  // refraction (only metals have an imaginary components; dielectrics such
  // as glass have K = 0).
  //
  float n, k;
};

// A Fresnel object holds various quantities derived from the final index
// of refraction (that being the ratio of the indices of refraction of the
// two materials on either side of an interface).
//
class Fresnel
{
public:

  // Calculate a "simple" index of refraction as the ratio of the two
  // real IORs given, with no imaginary component.
  //
  Fresnel (float n1, float n2)
    : ior (n2 / n1), n2k2 (0), n2_m_k2 (ior.n * ior.n)
  { }

  // Calculate the final IOR for a medium->material interface (where the
  // Ior object represents the material).
  //
  Fresnel (float n1, const Ior &ior2)
    : ior (ior2.n / n1, ior2.k / n1),
      n2k2 (ior.n * ior.n * ior.k * ior.k),
      n2_m_k2 (ior.n * ior.n - ior.k * ior.k)
  { }

  // Calculate the final IOR for a material->medium interface (where the
  // Ior object represents the material).  In this case, there must
  // be no imaginary component to the index of refaction, as such
  // materials are opaque!
  //
  Fresnel (const Ior &ior1, float n2)
    : ior (n2 / ior1.n), n2k2 (0), n2_m_k2 (ior.n * ior.n)
  { }

  // Calculate Ior reflectance from light at a given angle to the normal;
  // COS_ANGLE is the cosine of the angle (this is usually available,
  // rather than the actual angle).
  //
  float reflectance (float cos_refl_angle) const
  {
    // Clamp COS_REFL_ANGLE between -1 and 1, as values even just slightly
    // outside that range (not uncommon, due to floating-point precision
    // errors) can cause a floating-point exception.
    //
    cos_refl_angle = max (min (cos_refl_angle, 1.f), -1.f);

    float refl_angle = acos (cos_refl_angle);

    // Reflectance of parallel and perpendicular polarized light.
    //
    float Rp, Rs;

    if (ior.k == 0)
      {
	// No complex term
	//
	//    F = (abs(Fs)^2 + abs(Fp)^2) / 2
	//
	//    Fp = (n cos refl_angle - cos trans_angle)
	//         / (n cos refl_angle + cos trans_angle)
	//
	//    Fs = (cos refl_angle - n cos trans_angle)
	//         / (cos refl_angle + n cos trans_angle)
	//
	// where n is the final index of refraction, and refl_angle and
	// trans_angle are the reflection and refraction refl_angles of the
	// light ray.

	float sin_trans_angle = max (min (sin (refl_angle) / ior.n, 1.f), -1.f);

	float trans_angle = asin (sin_trans_angle);
	float cos_trans_angle = cos (trans_angle);

	float nc1 = ior.n * cos_refl_angle;
	float nc2 = ior.n * cos_trans_angle;

	// XXX The following conditionals protect against zero-divide by
	// making the result zero in that case; but they seem wrong -- as
	// the denominator _approaches_ zero, we'll return a very large
	// value, which seems bizarre ... shouldn't we always return 0-1?

	float Fs_den = nc1 + cos_trans_angle;
	float Fs = (Fs_den == 0) ? 0 : (nc1 - cos_trans_angle) / Fs_den;

	float Fp_den = cos_refl_angle + nc2;
	float Fp = (Fp_den == 0) ? 0 : (cos_refl_angle - nc2) / Fp_den;

	// Square Fs and Fp to get reflectance
	//
	Rs = Fs * Fs;
	Rp = Fp * Fp;
      }
    else
      {
	// Complex term (k is imaginary part -- ior == n + i * k)
	//
	//       a^2 + b^2 - 2 a cos refl_angle + cos^2 refl_angle
	//  Rs = -------------------------------------------------
	//       a^2 + b^2 + 2 a cos refl_angle + cos^2 refl_angle
	//
	//            a^2 + b^2 - 2 a sin refl_angle tan refl_angle
	//              + sin^2 refl_angle tan^2 refl_angle
	//  Rp = Rs -------------------------------------------------
	//            a^2 + b^2 - 2 a sin refl_angle tan refl_angle
	//		+ sin^2 refl_angle tan^2 refl_angle
	//
	// Where:
	//
	//   2 a^2 = sqrt ((n^2 - k^2 - sin^2 refl_angle)^2 + 4 n^2 k^2)
	//            + (n^2 - k^2 - sin^2 refl_angle)
	//
	//   2 b^2 = sqrt ((n^2 - k^2 - sin^2 refl_angle)^2 + 4 n^2 k^2)
	//            - (n^2 - k^2 - sin^2 refl_angle)
	//

	float sin_refl_angle = sin (refl_angle);
	float n2_m_k2_m_sin2_refl_angle
	  = n2_m_k2 - sin_refl_angle * sin_refl_angle;
	float sin_tan_refl_angle = sin_refl_angle * tan (refl_angle);

	float a2_b2_common
	  = sqrt (n2_m_k2_m_sin2_refl_angle * n2_m_k2_m_sin2_refl_angle
		  + 4 * n2k2);
	float a2 = (a2_b2_common + n2_m_k2_m_sin2_refl_angle) / 2;
	float b2 = (a2_b2_common - n2_m_k2_m_sin2_refl_angle) / 2;

	float a2_p_b2 = a2 + b2;
	float a = sqrt (a2);

	float Rs_term1 = a2_p_b2 + cos_refl_angle * cos_refl_angle;
	float Rs_term2 = 2 * a * cos_refl_angle;
	Rs = (Rs_term1 - Rs_term2) / (Rs_term1 + Rs_term2);

	float Rp_term1 = a2_p_b2 + sin_tan_refl_angle * sin_tan_refl_angle;
	float Rp_term2 = 2 * a * sin_tan_refl_angle;
	Rp = Rs * ((Rp_term1 - Rp_term2) / (Rp_term1 + Rp_term2));
      }

    return (Rs + Rp) / 2;
  }

  // Final index of refracetion (the ratio of the indices of refraction on
  // either side of the interface).
  //
  Ior ior;

  // Some derived quantities.
  //
  float n2k2;			// N^2 * K^2
  float n2_m_k2;		// N^2 - K^2
};

}

#endif /* __FRESNEL_H__ */

// arch-tag: c0eb92bc-2471-485e-bbde-0fde045a2f86
