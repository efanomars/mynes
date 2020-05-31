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
 * File:   setupstdconfig.h
 */

#ifndef STMG_MYNES_SETUP_STD_CONFIG_H
#define STMG_MYNES_SETUP_STD_CONFIG_H

#include <string>
#include <memory>

namespace stmg { class StdConfig; }
namespace stmi { class DeviceManager; }

namespace stmg
{

using std::shared_ptr;

/* The StdConfig setup.
 */
void mynesSetupStdConfig(shared_ptr<StdConfig>& refStdConfig, const shared_ptr<stmi::DeviceManager>& refDeviceManager
							, const std::string& sMynes, const std::string& sAppVersion
							, bool bNoSound, bool bTestMode, bool bTouchMode, bool bKeysMode) noexcept;

} // namespace stmg

#endif	/* STMG_MYNES_SETUP_STD_CONFIG_H */

