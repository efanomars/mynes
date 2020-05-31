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
 * File:   setup.cc
 */

#include "setup.h"

#include "setupstdconfig.h"
#include "setupxmlgameloader.h"
#include "setupxmlthemeloader.h"

#include <stmm-games-xml-gtk/xmlthemeloader.h>
#include <stmm-games-xml-gtk/gamediskfiles.h>

#include <stmm-games-xml-game/xmlgameloader.h>
#include <stmm-games-xml-game/xmlhighscoresloader.h>
#include <stmm-games-xml-game/xmlpreferencesloader.h>

#include <stmm-games-gtk/mainwindow.h>

#include <stmm-games/stdconfig.h>

#include <stmm-input-gtk-dm/gtkdevicemanager.h>

#include <gtkmm.h>

//#include <iostream>
//#include <cassert>
#include <memory>

namespace stmg { class StdConfig; }


namespace stmg
{

static void printDirectories(const GameDiskFiles& oGameDiskFiles) noexcept
{
	std::cout << "Games and themes base paths" << '\n';
	auto aPaths = oGameDiskFiles.getGamesAndThemesBasePaths();
	for (const auto& sPath : aPaths) {
		std::cout << " - " << sPath << '\n';
	}
	std::cout << "Highscores and preferences base paths" << '\n';
	std::cout << " - " << oGameDiskFiles.getPrefsAndHighscoresBasePath() << '\n';
}

std::string mynesSetup(MainWindowData& oMainWindowData, const std::string& sMynes, const std::string& sAppVersion
						, bool bNoSound, bool bTestMode, bool bFullScreen, bool bTouchMode, bool bKeysMode) noexcept
{
	stmi::GtkDeviceManager::Init oInit;
	oInit.m_sAppName = sMynes;
	oInit.m_aGroups.push_back("gtk");
	if (! bNoSound) {
		oInit.m_aGroups.push_back("sound");
	}
	auto oPairDeviceManager = stmi::GtkDeviceManager::create(oInit);
	shared_ptr<stmi::DeviceManager> refDeviceManager = oPairDeviceManager.first;
	if (!refDeviceManager) {
		const std::string sError = "Error: Couldn't create device manager:\n" + oPairDeviceManager.second;
		return sError; //-------------------------------------------------------
	}

	shared_ptr<StdConfig>& refStdConfig = oMainWindowData.m_refStdConfig;
	mynesSetupStdConfig(refStdConfig, refDeviceManager, sMynes, sAppVersion
						, bNoSound, bTestMode, bTouchMode, bKeysMode);

	oMainWindowData.m_bFullscreen = bFullScreen;
	//
	#ifdef STMM_SNAP_PACKAGING
	const bool bIncludeHomeLocal = false;
	#else
	const bool bIncludeHomeLocal = true;
	#endif //STMM_SNAP_PACKAGING
	auto refGameDiskFiles = std::make_shared<GameDiskFiles>(sMynes, bIncludeHomeLocal);
	if (bTestMode) {
		printDirectories(*refGameDiskFiles);
	}

	unique_ptr<XmlGameLoader> refXmlGameLoader;
	mynesSetupXmlGameLoader(refXmlGameLoader, refStdConfig, refGameDiskFiles);
	unique_ptr<XmlThemeLoader> refXmlThemeLoader;
	mynesSetupXmlThemeLoader(refXmlThemeLoader, refStdConfig, refGameDiskFiles);

	oMainWindowData.m_refGameLoader = std::move(refXmlGameLoader);
	oMainWindowData.m_refThemeLoader = std::move(refXmlThemeLoader);
	oMainWindowData.m_refHighscoresLoader = std::make_unique<XmlHighscoresLoader>(refStdConfig, refGameDiskFiles);
	oMainWindowData.m_refAllPreferencesLoader = std::make_unique<XmlPreferencesLoader>(refStdConfig, refGameDiskFiles);

	oMainWindowData.m_oIconFile = refGameDiskFiles->getIconFile();
	oMainWindowData.m_sCopyright = "© 2019-2020 Stefano Marsili, Switzerland";
	oMainWindowData.m_sWebSite = "https://efanomars.com/games/" + sMynes;
	MainAuthorData oAuthor;
	oAuthor.m_sName = "Stefano Marsili";
	oAuthor.m_sEMail = "stemars@gmx.ch";
	oAuthor.m_sRole = "";
	oMainWindowData.m_aAuthors.push_back(std::move(oAuthor));

	return "";
}

} //namespace stmg

