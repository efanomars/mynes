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
 * File:   testLoadGames.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "setupstdconfig.h"
#include "setupxmlgameloader.h"
#include "setupxmlthemeloader.h"

#include "testconfig.h"

#include <stmm-games-xml-gtk/gamediskfiles.h>
#include <stmm-games-xml-gtk/xmlthemeloader.h>
#include <stmm-games-xml-game/xmlgameloader.h>

#include <stmm-games-gtk/theme.h>

#include <stmm-games-fake/fixtureTestBase.h>

#include <stmm-games/stdpreferences.h>
#include <stmm-games/gameowner.h>

#include <stmm-input-fake/fakedevicemanager.h>
#include <stmm-input-fake/fakekeydevice.h>

#include <vector>
#include <memory>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

template<int32_t nTeams, int32_t nPlayers, bool bNoSound, bool bTouch, bool bKeys>
class ReleaseGamesFixture : public TestBaseFixture, public GameOwner
{
public:
	void gameEnded() noexcept override
	{
	}
	void gameInterrupt(GameProxy::INTERRUPT_TYPE /*eInterruptType*/) noexcept override
	{
	}

protected:
	void setup() override
	{
		TestBaseFixture::setup();

		m_refFakeDM = std::make_shared<stmi::testing::FakeDeviceManager>();
		//const int32_t nKeyDevId =
		m_refFakeDM->simulateNewDevice<stmi::testing::FakeKeyDevice>();

		const std::string sMynes = "mynes";
		const std::string sAppVersion = "323.232";
		const bool bNoSoundMode = bNoSound;
		const bool bTestMode = true;
		const bool bTouchMode = bTouch;
		const bool bKeysMode = bKeys;
		mynesSetupStdConfig(m_refStdConfig, m_refFakeDM, sMynes, sAppVersion, bNoSoundMode, bTestMode, bTouchMode, bKeysMode);

		std::vector<File> aGameFiles;
		{
		auto aGameFilesPaths = config::getTestGamesFilePaths();
		for (auto& sFilePath : aGameFilesPaths) {
			aGameFiles.emplace_back(std::move(sFilePath));
		}
		}
		std::vector<File> aThemeFiles;
		{
		auto aThemeFilesPaths = config::getTestThemesFilePaths();
		for (auto& sFilePath : aThemeFilesPaths) {
			aThemeFiles.emplace_back(std::move(sFilePath));
		}
		}
		std::string sIconFile;
		std::string sHighscoresDir;
		std::string sPreferencesFile;
		m_refGameDiskFiles = std::make_shared<GameDiskFiles>(sMynes, false, std::move(aGameFiles), false, std::move(aThemeFiles), false
															, sIconFile, sHighscoresDir, sPreferencesFile);

		mynesSetupXmlThemeLoader(m_refXmlThemeLoader, m_refStdConfig, m_refGameDiskFiles);
		mynesSetupXmlGameLoader(m_refXmlGameLoader, m_refStdConfig, m_refGameDiskFiles);

		m_refPrefs = std::make_shared<StdPreferences>(m_refStdConfig);

		m_refPrefs->setTotTeams(nTeams);
		for (int32_t nTeam = 0; nTeam < nTeams; ++nTeam) {
			m_refPrefs->getTeamFull(nTeam)->setTotMates(nPlayers);
		}
	}
	void teardown() override
	{
		TestBaseFixture::teardown();
	}
	void onePlayer()
	{
		auto refTheme = m_refXmlThemeLoader->getTheme("");
		REQUIRE(refTheme);
		auto& oNamed = refTheme->getNamed();
		shared_ptr<Game> refGame;

		auto oPairGame = m_refXmlGameLoader->getNewGame("Rolling", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Classic-Small", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Classic-Medium", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Classic-Big", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);
	}
	void twoPlayers(bool bNot)
	{
		auto refTheme = m_refXmlThemeLoader->getTheme("");
		REQUIRE(refTheme);
		auto& oNamed = refTheme->getNamed();
		shared_ptr<Game> refGame;

		auto oPairGame = m_refXmlGameLoader->getNewGame("Rolling2", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		if (bNot) {
			REQUIRE(! refGame);
		} else {
			REQUIRE(refGame);
		}

		oPairGame = m_refXmlGameLoader->getNewGame("Classic2-Huge", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		if (bNot) {
			REQUIRE(! refGame);
		} else {
			REQUIRE(refGame);
		}
	}
public:
	shared_ptr<stmi::testing::FakeDeviceManager> m_refFakeDM;
	shared_ptr<StdConfig> m_refStdConfig;
	shared_ptr<GameDiskFiles> m_refGameDiskFiles;
	unique_ptr<XmlGameLoader> m_refXmlGameLoader;
	unique_ptr<XmlThemeLoader> m_refXmlThemeLoader;

	shared_ptr<StdPreferences> m_refPrefs;
};

class ReleaseOnePlayerSoundFFGamesFixture : public ReleaseGamesFixture<1, 1, false, false, false>
{
};

TEST_CASE_METHOD(STFX<ReleaseOnePlayerSoundFFGamesFixture>, "CheckSinglePlayerSound")
{
	onePlayer();
}

class ReleaseOnePlayerSoundFTGamesFixture : public ReleaseGamesFixture<1, 1, false, false, true>
{
};

TEST_CASE_METHOD(STFX<ReleaseOnePlayerSoundFTGamesFixture>, "CheckSinglePlayerSoundKeys")
{
	onePlayer();
}

class ReleaseOnePlayerNoSoundFFGamesFixture : public ReleaseGamesFixture<1, 1, true, false, false>
{
};

TEST_CASE_METHOD(STFX<ReleaseOnePlayerNoSoundFFGamesFixture>, "CheckSinglePlayerNoSound")
{
	onePlayer();
}

class ReleaseTwoPlayersSoundFTGamesFixture : public ReleaseGamesFixture<1, 2, false, false, true>
{
};

TEST_CASE_METHOD(STFX<ReleaseTwoPlayersSoundFTGamesFixture>, "CheckTwoPlayerSound")
{
	twoPlayers(false);
}

class ReleaseTwoPlayersSoundTFGamesFixture : public ReleaseGamesFixture<1, 2, false, true, false>
{
};

TEST_CASE_METHOD(STFX<ReleaseTwoPlayersSoundTFGamesFixture>, "CheckTwoPlayerSoundTouch")
{
	twoPlayers(true);
}

class ReleaseTwoPlayersNoSoundFTGamesFixture : public ReleaseGamesFixture<1, 2, true, false, true>
{
};

TEST_CASE_METHOD(STFX<ReleaseTwoPlayersNoSoundFTGamesFixture>, "CheckTwoPlayerNoSound")
{
	twoPlayers(false);
}

} // namespace testing

} // namespace stmg
