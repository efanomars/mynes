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
 * File:   testconfig.h
 */

#ifndef STMI_TEST_CONFIG_H
#define STMI_TEST_CONFIG_H

#include <vector>
#include <string>

namespace stmg
{

namespace testing
{

namespace config
{

const std::string& getTestBinaryDir();

/** The game file paths.
 * @return The game file full paths. Cannot be empty.
 */
std::vector<std::string> getTestGamesFilePaths();

/** The theme file paths.
 * @return The theme file full paths (theme.xml). Cannot be empty.
 */
std::vector<std::string> getTestThemesFilePaths();

} // namespace config

} // namespace testing

} // namespace stmg

#endif /* STMI_TEST_CONFIG_H */

