/*
 * Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   binconfig.h
 */

#ifndef STMG_STMM_BIN_CONFIG_H
#define STMG_STMM_BIN_CONFIG_H

#include <string>

namespace stmg
{

namespace binconfig
{

/** The mynes executable version.
 * @return The version string. Cannot be empty.
 */
const std::string& getExecutableVersion() noexcept;

} // namespace binconfig

} // namespace stmg

#endif	/* STMG_STMM_BIN_CONFIG_H */

