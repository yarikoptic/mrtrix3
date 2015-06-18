/*
   Copyright 2008 Brain Research Institute, Melbourne, Australia

   Written by J-Donald Tournier, 27/06/08.

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

#ifndef __interp_cubic_h__
#define __interp_cubic_h__

#include "transform.h"
#include "math/hermite.h"

namespace MR
{
  namespace Interp
  {

    //! \addtogroup interp
    // @{

    //! This class provides access to the voxel intensities of an image using cubic spline interpolation.
    /*! Interpolation is only performed along the first 3 (spatial) axes.
     * The (integer) position along the remaining axes should be set using the
     * template DataSet class.
     * The spatial coordinates can be set using the functions voxel(), image(),
     * and scanner().
     * For example:
     * \code
     * auto input = Image<float>::create (Argument[0]);
     *
     * // create an Interp::Cubic object using input as the parent data set:
     * Interp::Cubic<decltype(input) > interp (input);
     *
     * // set the scanner-space position to [ 10.2 3.59 54.1 ]:
     * interp.scanner (10.2, 3.59, 54.1);
     *
     * // get the value at this position:
     * float value = interp.value();
     * \endcode
     *
     * The template \a input class must be usable with this type of syntax:
     * \code
     * int xsize = input.size(0);    // return the dimension
     * int ysize = input.size(1);    // along the x, y & z dimensions
     * int zsize = input.size(2);
     * float v[] = { input.voxsize(0), input.voxsize(1), input.voxsize(2) };  // return voxel dimensions
     * input.index(0) = 0;               // these lines are used to
     * input.index(1)--;                 // set the current position
     * input.index(2)++;                 // within the data set
     * float f = input.value();
     * transform_type M = input.transform(); // a valid 4x4 transformation matrix
     * \endcode
     */

    template <class ImageType> class Cubic : public ImageType, public Transform
    {
      public:
        typedef typename ImageType::value_type value_type;

        using ImageType::size;
        using ImageType::index;
        using Transform::set_to_nearest;
        using Transform::voxelsize;
        using Transform::scanner2voxel;
        using Transform::operator!;
        using Transform::out_of_bounds;
        using Transform::bounds;

        //! construct a Nearest object to obtain interpolated values using the
        // parent DataSet class
        Cubic (const ImageType& parent, value_type value_when_out_of_bounds = Transform::default_out_of_bounds_value<value_type>()) :
          ImageType (parent),
          Transform (parent),
          out_of_bounds_value (value_when_out_of_bounds) { }

        //! Set the current position to <b>voxel space</b> position \a pos
        /*! This will set the position from which the image intensity values will
         * be interpolated, assuming that \a pos provides the position as a
         * (floating-point) voxel coordinate within the dataset. */
        template <class VectorType>
        bool voxel (const VectorType& pos) {
          Eigen::Vector3d f = Transform::set_to_nearest (pos);
          if (out_of_bounds)
            return true;
          P = pos;
          Hx.set (f[0]);
          Hy.set (f[1]);
          Hz.set (f[2]);
          return false;
        }
        //! Set the current position to <b>image space</b> position \a pos
        /*! This will set the position from which the image intensity values will
         * be interpolated, assuming that \a pos provides the position as a
         * coordinate relative to the axes of the dataset, in units of
         * millimeters. The origin is taken to be the centre of the voxel at [
         * 0 0 0 ]. */
        template <class VectorType>
        bool image (const VectorType& pos) {
          return voxel (voxelsize.inverse() * pos.template cast<double>());
        }
        //! Set the current position to the <b>scanner space</b> position \a pos
        /*! This will set the position from which the image intensity values will
         * be interpolated, assuming that \a pos provides the position as a
         * scanner space coordinate, in units of millimeters. */
        template <class VectorType>
        bool scanner (const VectorType& pos) {
          return voxel (Transform::scanner2voxel * pos.template cast<double>());
        }

        value_type value () {
          if (out_of_bounds)
            return out_of_bounds_value;

          ssize_t c[] = { ssize_t (std::floor (P[0])-1), ssize_t (std::floor (P[1])-1), ssize_t (std::floor (P[2])-1) };
          value_type r[4];
          for (ssize_t z = 0; z < 4; ++z) {
            index(2) = check (c[2] + z, size (2)-1);
            value_type q[4];
            for (ssize_t y = 0; y < 4; ++y) {
              index(1) = check (c[1] + y, size (1)-1);
              value_type p[4];
              for (ssize_t x = 0; x < 4; ++x) {
                index(0) = check (c[0] + x, size (0)-1);
                p[x] = ImageType::value();
              }
              q[y] = Hx.value (p);
            }
            r[z] = Hy.value (q);
          }
          return Hz.value (r);
        }

        const value_type out_of_bounds_value;

      protected:
        Math::Hermite<value_type> Hx, Hy, Hz;
        Eigen::Vector3d P;

        ssize_t check (ssize_t x, ssize_t dim) const {
          if (x < 0) return 0;
          if (x > dim) return dim;
          return x;
        }
    };





  template <class ImageType, typename... Args>
    inline Cubic<ImageType> make_cubic (const ImageType& parent, Args&&... args) {
      return Cubic<ImageType> (parent, std::forward<Args> (args)...);
    }



    //! @}

  }
}

#endif

