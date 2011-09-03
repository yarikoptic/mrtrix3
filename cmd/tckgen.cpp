/*
    Copyright 2009 Brain Research Institute, Melbourne, Australia

    Written by J-Donald Tournier, 25/10/09.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "app.h"
#include "image/voxel.h"
#include "dataset/interp/linear.h"
#include "dwi/tractography/exec.h"
#include "dwi/tractography/iFOD1.h"
#include "dwi/tractography/iFOD2.h"
#include "dwi/tractography/fact.h"
#include "dwi/tractography/wbfact.h"
#include "dwi/tractography/vecstream.h"

MRTRIX_APPLICATION

using namespace MR;
using namespace App;

void usage ()
{
  DESCRIPTION
  + "perform streamlines tracking.";

  ARGUMENTS
  + Argument ("source",
              "the image containing the source data. "
              "For iFOD1/2, this should be the FOD file, expressed in spherical harmonics. "
              "For VecStream, this should be the directions file."
             ).type_image_in()

  + Argument ("tracks",
              "the output file containing the tracks generated."
             ).type_file();


  const char* algorithms[] = { "ifod1", "ifod2", "fact", "wbfact", "vecstream", NULL };

  OPTIONS
  + Option ("algorithm",
            "specify the tractography algorithm to use. Valid choices are: iFOD1, "
            "iFOD2, FACT, WBFACT, VecStream (default: iFOD2).")
  + Argument ("name").type_choice (algorithms)

  + Option ("seed",
            "specify the seed region of interest. This should be either the path "
            "to a binary mask image, or a comma-separated list of 4 floating-point "
            "values, specifying the [x,y,z] coordinates of the centre and radius "
            "of a spherical ROI.")
  .allow_multiple()
  + Argument ("spec")

  + Option ("include",
            "specify an inclusion region of interest, in the same format as the "
            "seed region. Only tracks that enter all such inclusion ROI will be "
            "produced.")
  .allow_multiple()
  + Argument ("spec")

  + Option ("exclude",
            "specify an exclusion region of interest, in the same format as the "
            "seed region. Only tracks that enter any such exclusion ROI will be "
            "discarded.")
  .allow_multiple()
  + Argument ("spec")

  + Option ("mask",
            "specify a mask region of interest, in the same format as the seed "
            "region. Tracks will be terminated when they leave any such ROI.")
  .allow_multiple()
  + Argument ("spec")

  + Option ("grad",
            "specify the diffusion encoding scheme (may be required for FACT "
            "and RSFACT, ignored otherwise)")
  + Argument ("file")

  + Option ("step",
            "set the step size of the algorithm in mm (default for iFOD1: 0.1 x voxelsize; for iFOD2: 0.5 x voxelsize).")
  + Argument ("size").type_float (0.0, 0.0, INFINITY)

  + Option ("angle",
            "set the maximum angle between successive steps (default is 90° x stepsize / voxelsize).")
  + Argument ("theta").type_float (0.0, 90.0, 90.0)

  + Option ("number",
            "set the desired number of tracks. The program will continue to "
            "generate tracks until this number of tracks have been selected "
            "and written to the output file (default is 100 for *_STREAM methods, "
            "1000 for *_PROB methods).")
  + Argument ("tracks").type_integer (1, 1, std::numeric_limits<int>::max())

  + Option ("maxnum",
            "set the maximum number of tracks to generate. The program will "
            "not generate more tracks than this number, even if the desired "
            "number of tracks hasn't yet been reached (default is 100 x number).")
  + Argument ("tracks").type_integer (1, 1, INT_MAX)

  + Option ("maxlength",
            "set the maximum length of any track in mm (default is 100 x voxelsize).")
  + Argument ("value").type_float (0.0, 0.0, INFINITY)

  + Option ("minlength",
            "set the minimum length of any track in mm (default is 5 x voxelsize).")
  + Argument ("value").type_float (0.0, 0.0, INFINITY)

  + Option ("cutoff",
            "set the FA or FOD amplitude cutoff for terminating tracks "
            "(default is 0.1).")
  + Argument ("value").type_float (0.0, 0.1, INFINITY)

  + Option ("initcutoff",
            "set the minimum FA or FOD amplitude for initiating tracks (default "
            "is twice the normal cutoff).")
  + Argument ("value").type_float (0.0, 0.1, INFINITY)

  + Option ("trials",
            "set the maximum number of sampling trials at each point (only "
            "used for probabilistic tracking).")
  + Argument ("number").type_integer (1, MAX_TRIALS, std::numeric_limits<int>::max())

  + Option ("unidirectional",
            "track from the seed point in one direction only (default is to "
            "track in both directions).")

  + Option ("initdirection",
            "specify an initial direction for the tracking (this should be "
            "supplied as a vector of 3 comma-separated values.")
  + Argument ("dir").type_sequence_float()

  + Option ("noprecomputed",
            "do NOT pre-compute legendre polynomial values. Warning: "
            "this will slow down the algorithm by a factor of approximately 4.")

  + Option ("power",
            "raise the FOD to the power specified (default is 1/nsamples).")
  + Argument ("value").type_float (1e-6, 1.0, 1e6)

  + Option ("samples",
            "set the number of FOD samples to take per step for the 2nd order "
            "(iFOD2) method (Default: 4).")
  + Argument ("number").type_integer (2, 4, 100);
}




void run ()
{
  using namespace DWI::Tractography;

  Properties properties;

  int algorithm = 1;
  Options opt = get_options ("algorithm");
  if (opt.size()) algorithm = opt[0][0];

  opt = get_options ("seed");
  for (size_t i = 0; i < opt.size(); ++i)
    properties.seed.add (ROI (opt[i][0]));

  opt = get_options ("include");
  for (size_t i = 0; i < opt.size(); ++i)
    properties.include.add (ROI (opt[i][0]));

  opt = get_options ("exclude");
  for (size_t i = 0; i < opt.size(); ++i)
    properties.exclude.add (ROI (opt[i][0]));

  opt = get_options ("mask");
  for (size_t i = 0; i < opt.size(); ++i)
    properties.mask.add (ROI (opt[i][0]));

  opt = get_options ("grad");
  if (opt.size()) properties["DW_scheme"] = std::string (opt[0][0]);

  opt = get_options ("step");
  if (opt.size()) properties["step_size"] = std::string (opt[0][0]);

  opt = get_options ("angle");
  if (opt.size()) properties["max_angle"] = std::string (opt[0][0]);

  opt = get_options ("number");
  if (opt.size()) properties["max_num_tracks"] = std::string (opt[0][0]);

  opt = get_options ("maxnum");
  if (opt.size()) properties["max_num_attempts"] = std::string (opt[0][0]);

  opt = get_options ("maxlength");
  if (opt.size()) properties["max_dist"] = std::string (opt[0][0]);

  opt = get_options ("minlength");
  if (opt.size()) properties["min_dist"] = std::string (opt[0][0]);

  opt = get_options ("cutoff");
  if (opt.size()) properties["threshold"] = std::string (opt[0][0]);

  opt = get_options ("initcutoff");
  if (opt.size()) properties["init_threshold"] = std::string (opt[0][0]);

  opt = get_options ("trials");
  if (opt.size()) properties["max_trials"] = std::string (opt[0][0]);

  opt = get_options ("unidirectional");
  if (opt.size()) properties["unidirectional"] = "1";

  opt = get_options ("initdirection");
  if (opt.size()) properties["init_direction"] = std::string (opt[0][0]);

  opt = get_options ("noprecomputed");
  if (opt.size()) properties["sh_precomputed"] = "0";

  opt = get_options ("power");
  if (opt.size()) properties["fod_power"] = std::string (opt[0][0]);

  opt = get_options ("samples");
  if (opt.size()) properties["samples_per_step"] = std::string (opt[0][0]);

  Image::Header source (argument[0]);

  switch (algorithm) {
    case 0:
      Exec<iFOD1>::run (source, argument[1], properties);
      break;
    case 1:
      Exec<iFOD2>::run (source, argument[1], properties);
      break;
    case 2:
      Exec<FACT>::run (source, argument[1], properties);
      break;
    case 3:
      Exec<WBFACT>::run (source, argument[1], properties);
      break;
    case 4:
      Exec<VecStream>::run (source, argument[1], properties);
      break;
    default:
      assert (0);
  }
}
