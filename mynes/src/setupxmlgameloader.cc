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
 * File:   setupxmlgameloader.cc
 */

#include "setupxmlgameloader.h"

#include <stmm-mynes-xml/xmlfillerevent.h>
#include <stmm-mynes-xml/xmlmynesevent.h>
#include <stmm-mynes-xml/xmlpressevent.h>
#include <stmm-mynes-xml/xmlsonarevent.h>
#include <stmm-mynes-xml/xmlsquarsorevent.h>

#include <stmm-games-xml-gtk/gamediskfiles.h>

#include <stmm-games-xml-game/xmlgameloader.h>
#include <stmm-games-xml-game/xmlstdeventparsers.h>
#include <stmm-games-xml-game/xmlstdgamewidgetparsers.h>

#include <stmm-games/stdconfig.h>

//#include <iostream>
#include <cassert>
#include <utility>


namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

void mynesSetupXmlGameLoader(unique_ptr<XmlGameLoader>& refXmlGameLoader
								, const shared_ptr<StdConfig>& refStdConfig, const shared_ptr<GameDiskFiles>& refGameDiskFiles) noexcept
{
	assert(refXmlGameLoader.get() == nullptr);
	//
	XmlGameLoader::Init oInit;
	oInit.m_refAppConfig = refStdConfig;
	oInit.m_refXmlGameFiles = refGameDiskFiles;
	//
	oInit.m_aEventParsers = getXmlStdEventParsers();
	oInit.m_aEventParsers.push_back(std::make_unique<XmlFillerEventParser>());
	oInit.m_aEventParsers.push_back(std::make_unique<XmlMynesEventParser>());
	oInit.m_aEventParsers.push_back(std::make_unique<XmlPressEventParser>());
	oInit.m_aEventParsers.push_back(std::make_unique<XmlSonarEventParser>());
	oInit.m_aEventParsers.push_back(std::make_unique<XmlSquarsorEventParser>());
	//
	oInit.m_aGameWidgetParsers = getXmlStdGameWidgetParsers();
	//
	//oInit.m_sDefaultGameName = "Classic-Small";
	//
	refXmlGameLoader = std::make_unique<XmlGameLoader>(std::move(oInit));
}

} //namespace stmg

