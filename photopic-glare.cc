
// Glare point-spread-function (PSF) based on the human visual system
// in non-dark-adapted ("photopic") state.
//
// From [SSZG95]:
// Spencer, G., Shirley, P., Zimmerman, K., Greenberg, D.P.
// "Physically based glare effect for digital images"
// Proceedings of SIGGRAPH, pp. 325–334. ACM, New York (1995)

static float
f0 (float theta)
{
  // The original formula is:
  //
  //   f0 (theta) = 2.61 * 10^6 * exp (-(theta_deg / 0.02) ^ 2)
  //
  theta = theta * 180 / PIf; // convert to degrees
  return 2.61e6f * exp (-2500 * theta * theta);
}

static float
f1 (float theta)
{
  // The original formula is:
  //
  //   f1 (theta) = 20.91 / (theta_deg + 0.02)^3
  //
  theta = theta * 180 / PIf; // convert to degrees
  float theta_plus = theta + 0.02f;
  return 20.91f / (theta_plus * theta_plus * theta_plus);
}

static float
f2 (float theta)
{
  // The original formula is:
  //
  //   f1 (theta) = 72.37 / (theta_deg + 0.02)^2
  //
  theta = theta * 180 / PIf; // convert to degrees
  float theta_plus = theta + 0.02f;
  return 72.37f / (theta_plus * theta_plus);
}

PhotopicGlare::operator () (float theta)
{
  return 0.384f * f0 (theta) + 0.478f * f1 (theta) + 0.138f * f2 (theta);
}
