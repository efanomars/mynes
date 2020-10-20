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
 * File:   setupxmlthemeloader.cc
 */

#include "setupxmlthemeloader.h"

#include <stmm-games-xml-gtk/xmlthemeloader.h>

#include <stmm-games-xml-gtk/xmlstdmodifierparsers.h>
#include <stmm-games-xml-gtk/xmlstdthanimationparsers.h>
#include <stmm-games-xml-gtk/xmlstdthwidgetparsers.h>

#include <stmm-games/stdconfig.h>

//#include <iostream>
#include <cassert>
#include <utility>
#include <vector>

namespace stmg { class GameDiskFiles; }


namespace stmg
{

void mynesSetupXmlThemeLoader(unique_ptr<XmlThemeLoader>& refXmlThemeLoader
								, const shared_ptr<StdConfig>& refStdConfig, const shared_ptr<GameDiskFiles>& refGameDiskFiles) noexcept
{
	assert(refXmlThemeLoader.get() == nullptr);
	//
	XmlThemeLoader::Init oInit;
	oInit.m_refAppConfig = refStdConfig;
	oInit.m_refGameDiskFiles = refGameDiskFiles;
	oInit.m_aModifierParsers = getXmlStdModifierParsers();
	oInit.m_aThAnimationParsers = getXmlStdThAnimationParsers();
	oInit.m_aThWidgetParsers = getXmlStdThWidgetParsers();
	//
	refXmlThemeLoader = std::make_unique<XmlThemeLoader>(std::move(oInit));
}

} //namespace stmg

