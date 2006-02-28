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

#include <cmath>
#include <complex>
#include <list>

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
Snogray::cook_torrance (const Color &spec_col, float m,
			float ior, float ior_imag)
{
  static std::list<CookTorrance> brdfs;

  for (std::list<CookTorrance>::iterator b = brdfs.begin ();
       b != brdfs.end (); b++)
    if (b->m == m && b->ior == ior && b->ior_imag == ior_imag
	&& b->specular_color == spec_col)
      return &(*b);

  brdfs.push_front (CookTorrance (spec_col, m, ior, ior_imag));

  return &brdfs.front ();
}

// Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
// For best results, they should be distributed according to the BRDF's
// importance function.
//
void
CookTorrance::gen_samples (const Intersect &isec, SampleRayVec &samples)
  const
{
  throw std::runtime_error ("CookTorrance::gen_samples");
}

// Modify the value of each of the light-samples in SAMPLES according to
// the BRDF's reflectivity in the sample's direction.
//
void
CookTorrance::filter_samples (const Intersect &isec, SampleRayVec &samples,
			      SampleRayVec::iterator from,
			      SampleRayVec::iterator to)
  const
{
  // Surface normal
  //
  const Vec &N = isec.normal;

  // Eye-ray direction vector (normalized)
  //
  const Vec V = -isec.ray.dir;

  // 1 / (N dot V)
  //
  float NV = dot (N, V);
  float NV_inv = 1 / NV;

  // Index of refraction of the medium this surface is adjacent to, and
  // this material.
  //
  float nI = isec.trace.medium ? isec.trace.medium->ior : 1;
  float nTr = ior;
  float nTi = ior_imag;
  float n = nTr / nI;
  float k = nTi / nI;
  float n2k2 = n * n * k * k;
  float n2_m_k2 = n * n - k * k;

  for (SampleRayVec::iterator s = from; s != to; s++)
    {
      // The Cook-Torrance specular term is:
      //
      //    p_s = (F / PI) * (D * G / (N dot V))
      //
      // We calculate each of these sub-terms below

      // Light-ray direction vector (normalized)
      //
      const Vec &L = s->dir;
      float NL = dot (N, L);

      // Half-way vector between eye-ray and light-ray (normalized)
      //
      const Vec H = (V + L).unit ();
      float NH = dot (N, H);

      // Calculate D (microfacet distribution) term:
      //
      //    D = (1 / (4 * m^2 * (cos alpha)^2)) * e^(-((tan alpha) / m)^2)
      //
      // where alpha is the angle between N and H.

      float cos_alpha = NH;
      float cos_4_alpha = cos_alpha * cos_alpha * cos_alpha * cos_alpha;
      float tan_alpha = tan (acos (cos_alpha));
      float D_exp = exp (-tan_alpha * tan_alpha * m_2_inv);
      float D = m_2_inv / (4 * cos_4_alpha) * D_exp;

      // Calculate F (fresnel) term
      //
      //    F = (abs(Fs)^2 + abs(Fp)^2) / 2
      //

      float cos_th = NL;
      float th = acos (cos_th);

      float Fs, Fp;

      if (ior_imag == 0)
	{
	  // No complex term
	  //
	  //    Fp = (n cos th - cos thT)
	  //         / (n cos th + cos thT)
	  //
	  //    Fs = (cos th - n cos thT)
	  //         / (cos th + n cos thT)
	  //
	  // where nI and n are the indices of refraction, and th and
	  // thT are the reflection and refraction angles of the light
	  // ray.

	  float sin_thT = sin (th) / n;
	  float thT = asin (sin_thT);
	  float cos_thT = cos (thT);

	  float nc1 = n * cos_th;
	  float nc2 = n * cos_thT;

	  Fs = (nc1 - cos_thT) / (nc1 + cos_thT);
	  Fp = (cos_th - nc2) / (cos_th + nc2);
	}
      else
	{
	  // Complex term (k is imaginary part -- ior == n + i * k)
	  //
	  //       a^2 + b^2 - 2 a cos th + cos^2 th
	  //  Fs = ---------------------------------------
	  //       a^2 + b^2 + 2 a cos th + cos^2 th
	  //
	  //          a^2 + b^2 - 2 a sin th tan th + sin^2 th tan^2 th
	  //  Fp = Fs -------------------------------------------------
	  //          a^2 + b^2 - 2 a sin th tan th + sin^2 th tan^2 th
	  //
	  // Where:
	  //
	  //   2 a^2 = sqrt ((n^2 - k^2 - sin^2 th)^2 + 4 n^2 k^2)
	  //            + (n^2 - k^2 - sin^2 th)
	  //
	  //   2 b^2 = sqrt ((n^2 - k^2 - sin^2 th)^2 + 4 n^2 k^2)
	  //            - (n^2 - k^2 - sin^2 th)
	  //

	  float sin_th = sin (th);
	  float n2_m_k2_m_sin2_th = n2_m_k2 - sin_th * sin_th;
	  float sin_th_tan_th = sin_th * tan (th);

	  float a2_b2_common
	    = sqrt (n2_m_k2_m_sin2_th * n2_m_k2_m_sin2_th + 4 * n2k2);
	  float a2 = (a2_b2_common + n2_m_k2_m_sin2_th) * 0.5;
	  float b2 = (a2_b2_common - n2_m_k2_m_sin2_th) * 0.5;

	  float a2_p_b2 = a2 + b2;
	  float a = sqrt (a2);

	  float Fs_term1 = a2_p_b2 + cos_th * cos_th;
	  float Fs_term2 = 2 * a * cos_th;
	  Fs = (Fs_term1 - Fs_term2) / (Fs_term1 + Fs_term2);

	  float Fp_term1 = a2_p_b2 + sin_th_tan_th * sin_th_tan_th;
	  float Fp_term2 = 2 * a * sin_th_tan_th;
	  Fp = Fs * ((Fp_term1 - Fp_term2) / (Fp_term1 + Fp_term2));
	}

      float F = (Fs * Fs + Fp * Fp) * 0.5;

      // Calculate G (microfacet masking/shadowing) term
      //
      //    G = min (1,
      //             2 * (N dot H) * (N dot V) / (V dot H),
      //             2 * (N dot H) * (N dot L) / (V dot H))
      //
      float VH = dot (V, H);
      float G = 2 * NH * ((NV > NL) ? NL : NV) / VH;
      G = G <= 1 ? G : 1;

      float specular = F * D * G * NV_inv * M_1_PI;
      float diffuse = NL * M_1_PI; // standard lambertian diffuse term

      // The final reflectance is:
      //
      //   p = k_d * p_d + k_s * p_s
      //
      s->set_refl (isec.color * diffuse + specular_color * specular);
    }
}

// arch-tag: a0a0049e-9af6-4438-ab58-081630151122
