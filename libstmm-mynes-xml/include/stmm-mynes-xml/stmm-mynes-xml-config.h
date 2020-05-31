/*
 * Copyright © 2019  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   stmm-mynes-xml-config.h
 */

#ifndef STMG_STMM_MYNES_XML_LIB_CONFIG_H
#define STMG_STMM_MYNES_XML_LIB_CONFIG_H

#include <string>

namespace stmg
{

namespace libconfig
{

namespace mynes_xml
{

/** The stmm-mynes-xml library version.
 * @return The version string. Cannot be empty.
 */
const std::string& getVersion() noexcept;

} // namespace mynes_xml

} // namespace libconfig

} // namespace stmg

#endif	/* STMG_STMM_MYNES_XML_LIB_CONFIG_H */

