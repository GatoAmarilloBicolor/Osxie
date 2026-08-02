/**
 * This file is part of Osxie.
 *
 * Copyright (C) 2023 Osxie developers
 *
 * Osxie is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Osxie is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Osxie.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __XPC_ENDPOINT_H__
#define __XPC_ENDPOINT_H__

xpc_endpoint_t xpc_endpoint_create(xpc_connection_t connection);

#endif // __XPC_ENDPOINT_H__ 
