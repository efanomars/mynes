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
 * File:   setupxmlgameloader.h
 */

#ifndef STMG_MYNES_SETUP_XML_GAME_LOADER_H
#define STMG_MYNES_SETUP_XML_GAME_LOADER_H

#include <memory>

namespace stmg { class XmlGameLoader; }
namespace stmg { class StdConfig; }
namespace stmg { class GameDiskFiles; }

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

/* The XmlGameLoader setup.
 * @param refXmlGameLoader Must be null.
 * @param refStdConfig Cannot be null.
 * @param refGameDiskFiles Cannot be null.
 */
void mynesSetupXmlGameLoader(unique_ptr<XmlGameLoader>& refXmlGameLoader
							, const shared_ptr<StdConfig>& refStdConfig, const shared_ptr<GameDiskFiles>& refGameDiskFiles) noexcept;

} // namespace stmg

#endif	/* STMG_MYNES_SETUP_XML_GAME_LOADER_H */

