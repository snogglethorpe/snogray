// mis-sample-weight.h -- Weighting function for multiple-importance-sampling
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

#ifndef SNOGRAY_MIS_SAMPLE_WEIGHT_H
#define SNOGRAY_MIS_SAMPLE_WEIGHT_H


namespace snogray {


// Return a weight for one sample in multiple importance sampling
// with two sample distributions.  PDF is the pdf for the sample
// being weighted, and NUM_SAMPLES is the number of samples taken
// from its distribution.  OTHER_PDF and NUM_OTHER_SAMPLES are the
// corresponding values from the other distribution being combined
// with this one via MIS.
//
// This function is also known as the "power heuristic".
//
// The weight is calculated by squaring the product of the PDF and
// number of samples for each distribution; the actual value
// returned is:
//
//   (TERM^2 / (TERM^2 + OTHER_TERM^2)) * (1 / NUM_SAMPLES)
//
// where TERM = PDF * NUM_SAMPLES,
// and OTHER_TERM = OTHER_PDF * NUM_OTHER_SAMPLES.
//
// Note that the return value includes an additional 1/NUM_SAMPLES
// factor compared to the usual definition of the power heuristic;
// this is done because we almost always want to divide by the
// number of samples taken (since we're basically averaging the
// samples), and it's cheaper to calculate here than separately
// dividing by the number of samples afterwards.
//
static float
mis_sample_weight (float pdf, float num_samples,
		   float other_pdf, float num_other_samples)
{
  // This weight is calculated using the "power heuristic", with an
  // exponent of 2.

  // TERM and OTHER_TERM are just the PDFs weighted by the number of
  // samples.
  //
  float term = num_samples * pdf;
  float other_term = num_other_samples * other_pdf;

  // Handle near-degenerate cases that cause problems because of
  // floating-point overflow.
  //
  if (term > 1e10)
    return 1;
  else if (other_term > 1e10)
    return 0;

  // Raise the terms to a power of 2.
  //
  float term_2 = term * term;
  float other_term_2 = other_term * other_term;

  // Return (TERM^2 / (TERM^2 + OTHER_TERM^2)) * (1 / NUM_SAMPLES).
  //
  // Since TERM = PDF * NUM_SAMPLES, we avoid a divide by actually
  // calculating (TERM * PDF) / (TERM^2 + OTHER_TERM^2) instead.
  //
  return (term * pdf) / (term_2 + other_term_2);
}


}

#endif // SNOGRAY_MIS_SAMPLE_WEIGHT_H
