/*
 This file is part of Osxie.

 Copyright (C) 2020 Lubos Dolezel

 Osxie is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Osxie is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Osxie.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VECTOR_MAKE_H
#define VECTOR_MAKE_H

#include <simd/vector_types.h>

inline simd_float3 simd_make_float3(float x, float y, float z) {
    return (simd_float3){x, y, z};
}

inline simd_uint2 simd_make_uint2(unsigned int x, unsigned int y) {
    return (simd_uint2){x, y};
}

#endif
