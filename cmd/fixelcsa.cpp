/*
    Copyright 2013 Brain Research Institute, Melbourne, Australia

    Written by David Raffelt, 11/10/2013.

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
#include "command.h"
#include "progressbar.h"
#include "image/loop.h"
#include "image/voxel.h"
#include "image/buffer.h"
#include "image/transform.h"
#include "image/sparse/fixel_metric.h"
#include "image/sparse/voxel.h"
#include "math/vector.h"
#include "math/matrix.h"



using namespace MR;
using namespace App;
using Image::Sparse::FixelMetric;

typedef float value_type;

void usage ()
{
  AUTHOR = "David Raffelt (david.raffelt@florey.edu.au)";

  DESCRIPTION
  + "Compute the log of the change in fixel cross-sectional area during spatial normalisation. Used for fixel-based morphometry";


  ARGUMENTS

  + Argument ("template", "the fixel mask used to define fixels of interest. This can be generated by "
                          "thresholding the group average AFD fixel image.").type_image_in()

  + Argument ("jacobian", "the image of jacobian matrices computed from the transformation").type_image_in()

  + Argument ("output", "the modulated fixel image").type_text();

}


void run() {

  Image::Header input_header (argument[0]);
  Image::BufferSparse<FixelMetric> mask (input_header);
  Image::BufferSparse<FixelMetric>::voxel_type mask_vox (mask);

  Image::Buffer<float> jacobian_buf (argument[1]);
  Image::Buffer<float>::voxel_type  jacobian_vox(jacobian_buf);

  Image::check_dimensions (mask, jacobian_buf, 0, 3);

  Image::BufferSparse<FixelMetric> output (argument[2], input_header);
  Image::BufferSparse<FixelMetric>::voxel_type output_vox (output);
  Image::LoopInOrder loop (mask_vox, "modulating fixels...");


  for (loop.start (mask_vox, output_vox, jacobian_vox); loop.ok(); loop.next (mask_vox, output_vox, jacobian_vox)) {
    output_vox.value().set_size (mask_vox.value().size());
    for (size_t f = 0; f != mask_vox.value().size(); ++f) {
      output_vox.value()[f] = mask_vox.value()[f];

      Math::Matrix<double> jacobian_matrix (3,3);
      jacobian_vox[3] = 0;
      jacobian_matrix(0,0) = jacobian_vox.value();
      jacobian_vox[3] = 1;
      jacobian_matrix(0,1) = jacobian_vox.value();
      jacobian_vox[3] = 2;
      jacobian_matrix(0,2) = jacobian_vox.value();
      jacobian_vox[3] = 3;
      jacobian_matrix(1,0) = jacobian_vox.value();
      jacobian_vox[3] = 4;
      jacobian_matrix(1,1) = jacobian_vox.value();
      jacobian_vox[3] = 5;
      jacobian_matrix(1,2) = jacobian_vox.value();
      jacobian_vox[3] = 6;
      jacobian_matrix(2,0) = jacobian_vox.value();
      jacobian_vox[3] = 7;
      jacobian_matrix(2,1) = jacobian_vox.value();
      jacobian_vox[3] = 8;
      jacobian_matrix(2,2) = jacobian_vox.value();

      Math::Vector<double> v(3);
      v[0] = mask_vox.value()[f].dir[0];
      v[1] = mask_vox.value()[f].dir[1];
      v[2] = mask_vox.value()[f].dir[2];
      Math::normalise (v);

      Math::Vector<double> v_transformed(3);
      Math::mult (v_transformed, jacobian_matrix, v);
      output_vox.value()[f].value = Math::log (Math::determinant (jacobian_matrix) / Math::norm (v_transformed));
    }
  }
}