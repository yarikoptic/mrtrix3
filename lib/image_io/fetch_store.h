/*
   Copyright 2008 Brain Research Institute, Melbourne, Australia

   Written by J-Donald Tournier, 23/05/09.

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

#ifndef __image_io_fetch_store_h__
#define __image_io_fetch_store_h__

#include "raw.h"
#include "datatype.h"

namespace MR
{


  template <typename ValueType>
    typename std::enable_if<!is_data_type<ValueType>::value, void>::type __set_fetch_store_functions (
        std::function<ValueType(const void*,size_t,default_type,default_type)>& fetch_func,
        std::function<void(ValueType,void*,size_t,default_type,default_type)>& store_func, 
        DataType datatype) { }



  template <typename ValueType>
    typename std::enable_if<is_data_type<ValueType>::value, void>::type __set_fetch_store_functions (
        std::function<ValueType(const void*,size_t,default_type,default_type)>& fetch_func,
        std::function<void(ValueType,void*,size_t,default_type,default_type)>& store_func, 
        DataType datatype);


  // define fetch/store methods for all types using C++11 extern templates, 
  // to avoid massive recompile times...
#define __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(ValueType) \
  MRTRIX_EXTERN template void __set_fetch_store_functions<ValueType> ( \
      std::function<ValueType(const void*,size_t,default_type,default_type)>& fetch_func, \
        std::function<void(ValueType,void*,size_t,default_type,default_type)>& store_func, \
        DataType datatype) 

#define __DEFINE_FETCH_STORE_FUNCTIONS \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(bool); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(uint8_t); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(int8_t); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(uint16_t); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(int16_t); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(uint32_t); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(int32_t); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(uint64_t); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(int64_t); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(float); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(double); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(cfloat); \
  __DEFINE_FETCH_STORE_FUNCTION_FOR_TYPE(cdouble)


#define MRTRIX_EXTERN extern
  __DEFINE_FETCH_STORE_FUNCTIONS;

}

#endif


