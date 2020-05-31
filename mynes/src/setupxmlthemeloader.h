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
 * File:   setupxmlthemeloader.h
 */

#ifndef STMG_MYNES_SETUP_XML_THEME_LOADER_H
#define STMG_MYNES_SETUP_XML_THEME_LOADER_H

#include <memory>

namespace stmg { class GameDiskFiles; }
namespace stmg { class StdConfig; }
namespace stmg { class XmlThemeLoader; }

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

/* The XmlThemeLoader setup.
 * @param refXmlThemeLoader Must be null.
 * @param refStdConfig Cannot be null.
 * @param refThemeDiskFiles Cannot be null.
 */
void mynesSetupXmlThemeLoader(unique_ptr<XmlThemeLoader>& refXmlThemeLoader
							, const shared_ptr<StdConfig>& refStdConfig, const shared_ptr<GameDiskFiles>& refGameDiskFiles) noexcept;

} // namespace stmg

#endif	/* STMG_MYNES_SETUP_XML_THEME_LOADER_H */

