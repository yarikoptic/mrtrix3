/*
   Copyright 2009 Brain Research Institute, Melbourne, Australia

   Written by J-Donald Tournier, 26/08/09.

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

#include "file/utils.h"
#include "file/path.h"
#include "file/gz.h"
#include "file/nifti1_utils.h"
#include "header.h"
#include "image_io/gz.h"
#include "formats/list.h"

namespace MR
{
  namespace Formats
  {


    std::unique_ptr<ImageIO::Base> NIfTI_GZ::read (Header& H) const
    {
      if (!Path::has_suffix (H.name(), ".nii.gz")) 
        return std::unique_ptr<ImageIO::Base>();

      nifti_1_header NH;

      File::GZ zf (H.name(), "rb");
      zf.read (reinterpret_cast<char*> (&NH), sizeof (nifti_1_header));
      zf.close();

      size_t data_offset = File::NIfTI::read (H, NH);

      std::unique_ptr<ImageIO::GZ> io_handler (new ImageIO::GZ (H, sizeof(nifti_1_header)+sizeof(nifti1_extender)));
      memcpy (io_handler.get()->header(), &NH, sizeof(nifti_1_header));
      memset (io_handler.get()->header() + sizeof(nifti_1_header), 0, sizeof(nifti1_extender));
      io_handler->files.push_back (File::Entry (H.name(), data_offset));

      return std::move (io_handler);
    }





    bool NIfTI_GZ::check (Header& H, size_t num_axes) const
    {
      if (!Path::has_suffix (H.name(), ".nii.gz"))
        return false;

      if (num_axes < 3)
        throw Exception ("cannot create NIfTI-1.1 image with less than 3 dimensions");

      if (num_axes > 8)
        throw Exception ("cannot create NIfTI-1.1 image with more than 8 dimensions");

      H.set_ndim (num_axes);
      File::NIfTI::check (H, true);

      return true;
    }





    std::unique_ptr<ImageIO::Base> NIfTI_GZ::create (Header& H) const
    {
      if (H.ndim() > 7)
        throw Exception ("NIfTI-1.1 format cannot support more than 7 dimensions for image \"" + H.name() + "\"");

      std::unique_ptr<ImageIO::GZ> io_handler (new ImageIO::GZ (H, sizeof(nifti_1_header)+sizeof(nifti1_extender)));

      File::NIfTI::write (*reinterpret_cast<nifti_1_header*> (io_handler->header()), H, true);
      memset (io_handler->header()+sizeof(nifti_1_header), 0, sizeof(nifti1_extender));

      File::create (H.name());
      io_handler->files.push_back (File::Entry (H.name(), sizeof(nifti_1_header)+sizeof(nifti1_extender)));

      return std::move (io_handler);
    }

  }
}

