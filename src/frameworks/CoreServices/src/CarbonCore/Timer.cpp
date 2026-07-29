/*
This file is part of Osxie.

Copyright (C) 2017 Lubos Dolezel

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

#include <CarbonCore/Timer.h>
#include <mach/mach_time.h>
#include <stdint.h>

void Microseconds(UnsignedWide* tickCount)
{
	uint64_t time;

	time = mach_absolute_time();

	*reinterpret_cast<uint64_t*>(tickCount) = time / 1000000000ll;
	time -= *reinterpret_cast<uint64_t*>(tickCount) * 1000000000ll;

	*reinterpret_cast<uint64_t*>(tickCount) += time / 1000;
}

