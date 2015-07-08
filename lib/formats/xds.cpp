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

#include "file/ofstream.h"
#include "file/utils.h"
#include "file/path.h"
#include "file/entry.h"
#include "header.h"
#include "formats/list.h"
#include "image_io/default.h"

namespace MR
{
    namespace Formats
    {

      std::unique_ptr<ImageIO::Base> XDS::read (Header& H) const
      {
        if (!Path::has_suffix (H.name(), ".bfloat") &&
            !Path::has_suffix (H.name(), ".bshort"))
          return std::unique_ptr<ImageIO::Base>();

        H.set_ndim (4);
        int BE;

        std::string name (H.name());
        name.replace (name.size()-6, 6, "hdr");

        std::ifstream in (name.c_str());
        if (!in)
          throw Exception ("error reading header file \"" + name + "\": " + strerror (errno));
        int dim[3];
        in >> dim[0] >> dim[1] >> dim[2] >> BE;
        H.size(0) = dim[1];
        H.size(1) = dim[0];
        H.size(2) = dim[2];
        in.close();

        H.datatype() = (Path::has_suffix (H.name(), ".bfloat") ? DataType::Float32 : DataType::UInt16);
        if (BE) 
          H.datatype().set_flag (DataType::LittleEndian);
        else 
          H.datatype().set_flag (DataType::BigEndian);

        H.size(2) = 1;

        H.voxsize(0) = 3.0;
        H.voxsize(1) = 3.0;
        H.voxsize(2) = 10.0;
        H.voxsize(3) = 1.0;

        H.stride(0) = -1;
        H.stride(1) = -2;
        H.stride(2) = 0;
        H.stride(3) = 3;

        std::unique_ptr<ImageIO::Default> io_handler (new ImageIO::Default (H));
        io_handler->files.push_back (File::Entry (H.name()));

        return std::move (io_handler);
      }







      bool XDS::check (Header& H, size_t num_axes) const
      {
        if (!Path::has_suffix (H.name(), ".bfloat") &&
            !Path::has_suffix (H.name(), ".bshort"))
          return false;

        if (num_axes > 4)
          throw Exception ("cannot create XDS image with more than 4 dimensions");

        if (num_axes == 4 && H.size (2) > 1)
          throw Exception ("cannot create multi-slice XDS image with a single file");

        if (num_axes < 2)
          throw Exception ("cannot create XDS image with less than 2 dimensions");

        H.set_ndim (4);

        H.size(2) = 1;
        for (size_t n = 0; n < 4; ++n)
          if (H.size (n) < 1)
            H.size(n) = 1;


        H.voxsize(0) = 3.0;
        H.voxsize(1) = 3.0;
        H.voxsize(2) = 10.0;
        H.voxsize(3) = 1.0;

        H.stride(0) = -1;
        H.stride(1) = -2;
        H.stride(2) = 0;
        H.stride(3) = 3;

        DataType dtype (Path::has_suffix (H.name(), ".bfloat") ? DataType::Float32 : DataType::UInt16);
        if (H.datatype().is_big_endian()) dtype.set_flag (DataType::LittleEndian);
        else dtype.set_flag (DataType::BigEndian);
        H.datatype() = dtype;

        return true;
      }




      std::unique_ptr<ImageIO::Base> XDS::create (Header& H) const
      {
        std::string header_name (H.name());
        header_name.replace (header_name.size()-6, 6, "hdr");

        File::OFStream out (header_name);
        out << H.size (1) << " " << H.size (0) << " " << H.size (3)
            << " " << (H.datatype().is_little_endian() ? 1 : 0) << "\n";
        out.close();

        std::unique_ptr<ImageIO::Default> io_handler (new ImageIO::Default (H));
        File::create (H.name(), footprint (H, "11 1"));
        io_handler->files.push_back (File::Entry (H.name()));

        return std::move (io_handler);
      }

    }
}
