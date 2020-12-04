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
 * File:   testFillerEvent.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "fillerevent.h"

#include <stmm-games-fake/fixtureGame.h>
#include <stmm-games-fake/fakelevelview.h>
#include <stmm-games-fake/mockevent.h>

#include <stmm-games/events/logevent.h>
#include <stmm-games/traitsets/tiletraitsets.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

class FillerEventGameFixture : public GameFixture
							//default , public FixtureVariantDevicesKeys_Two, public FixtureVariantDevicesJoystick_Two
							, public FixtureVariantPrefsTeams<1>
							, public FixtureVariantPrefsMates<0,1>
							//default , public FixtureVariantMatesPerTeamMax_Three, public FixtureVariantAIMatesPerTeamMax_Zero
							//default , public FixtureVariantAllowMixedAIHumanTeam_False, public FixtureVariantPlayersMax_Six
							//default , public FixtureVariantTeamsMin_One, public FixtureVariantTeamsMax_Two
							//default , public FixtureVariantLayoutTeamDistribution_AllTeamsInOneLevel
							//default , public FixtureVariantLayoutShowMode_Show
							//default , public FixtureVariantLayoutCreateVarWidgetsFromVariables_False
							//default , public FixtureVariantLayoutCreateActionWidgetsFromKeyActions_False
							, public FixtureVariantVariablesGame_Time
							//, public FixtureVariantVariablesTeam
							, public FixtureVariantVariablesPlayer_Lives<3>
							//default , public FixtureVariantLevelInitBoardWidth<10>
							//default , public FixtureVariantLevelInitBoardHeight<6>
							//default , public FixtureVariantLevelInitShowWidth<10>
							//default , public FixtureVariantLevelInitShowHeight<6>
{
protected:
	void setup() override
	{
		GameFixture::setup();
	}
	void teardown() override
	{
		GameFixture::teardown();
	}

	void fillBoard(int32_t /*nBoardW*/, int32_t /*nBoardH*/, std::vector<Tile>& /*aBoard*/) override
	{
	}
	virtual int32_t getTotTiles() const
	{
		return 1;
	}
	virtual double getFactorOfAvailablePositions() const
	{
		return 0.0;
	}
	virtual FillerEvent::FILL_MODE getFillMode() const
	{
		return FillerEvent::FILL_MODE_REPLACE;
	}
	virtual Coords getCoords() const
	{
		return Coords{};
	}
	virtual unique_ptr<TileSelector> getTargetTiles() const
	{
		return unique_ptr<TileSelector>{};
	}
	virtual RandomTiles::ProbTileGen getProbTileGen() const
	{
		return RandomTiles::ProbTileGen{};
	}
};

////////////////////////////////////////////////////////////////////////////////
class FillerEventCase1GameFixture : public FillerEventGameFixture
{
protected:

	int32_t getTotTiles() const override
	{
		return 4;
	}
	RandomTiles::ProbTileGen getProbTileGen() const override
	{
		RandomTiles::ProbTileGen oPTG;
		{
			std::vector< std::unique_ptr<TraitSet> > aTraitSets;
			std::unique_ptr<CharUcs4TraitSet> refITS = std::make_unique<CharUcs4TraitSet>(65,65);
			auto refCharTraitSet = std::make_unique<CharTraitSet>(std::move(refITS));
			aTraitSets.push_back(std::move(refCharTraitSet));
			RandomTiles::ProbTraitSets oPTS;
			oPTS.m_nProb = 10;
			oPTS.m_aTraitSets = std::move(aTraitSets);
			//
			oPTG.m_aProbTraitSets.push_back(std::move(oPTS));
		}
		return oPTG;
	}
	void setup() override
	{
		FillerEventGameFixture::setup();

		const int32_t nLevel = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		FillerEvent::Init oFInit;
		oFInit.m_p0Level = p0Level;
		oFInit.m_oFillGen.m_eFillMode = getFillMode();
		oFInit.m_oFillGen.m_nTotTiles = getTotTiles();
		oFInit.m_oFillGen.m_fFactorOfAvailablePositions = getFactorOfAvailablePositions();
		oFInit.m_oFillGen.m_oProbTileGen = getProbTileGen();
		oFInit.m_oCoords = getCoords();
		oFInit.m_refTargetTiles = getTargetTiles();

		auto refFillerEvent = make_unique<FillerEvent>(std::move(oFInit));
		m_p0FillerEvent = refFillerEvent.get();
		p0Level->addEvent(std::move(refFillerEvent));
		p0Level->deactivateEvent(m_p0FillerEvent);
	}
	void teardown() override
	{
		FillerEventGameFixture::teardown();
	}
protected:
	shared_ptr<Level> m_refLevel;
	FillerEvent* m_p0FillerEvent;
};


TEST_CASE_METHOD(STFX<FillerEventCase1GameFixture>, "Case1_Fill_n")
{
	Level* p0Level = m_refLevel.get();
	const int32_t nBoardW = p0Level->boardWidth();
	const int32_t nBoardH = p0Level->boardHeight();

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 64738;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );
	
	LogEvent::msgLog().reset();
	
	m_p0FillerEvent->addListener(FillerEvent::LISTENER_GROUP_FILLED, p0LogEvent, 55);

	MockEvent::Init oMockInit;
	oMockInit.m_p0Level = p0Level;
	auto refMockEvent = make_unique<MockEvent>(std::move(oMockInit));
	MockEvent* p0MockEvent = refMockEvent.get();
	p0Level->addEvent(std::move(refMockEvent));

	p0MockEvent->addListener(8889, m_p0FillerEvent, FillerEvent::MESSAGE_FILL);

	auto oCheckTiles = [&](int32_t nExpectedTiles) {
		int32_t nTotTiles = 0;
		for (int32_t nY = 0; nY < nBoardH; ++nY) {
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				const Tile& oTile = p0Level->boardGetTile(nX, nY);
				if (! oTile.getTileChar().isCharIndex()) {
					if (oTile.getTileChar().getChar() == 65) {
						++nTotTiles;
					}
				}
			}
		}
		REQUIRE( nTotTiles == nExpectedTiles );
	};
	
	p0Level->setFallEachTicks(4);
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 1 );

	p0MockEvent->setTriggerValue(8889, 0, 1); // group 8889, value, trigger after one tick

	oCheckTiles(0);

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );

	oCheckTiles(0);

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 3 );

	oCheckTiles(4);
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE( oEntry.m_nGameTick == 2 );
		REQUIRE( oEntry.m_nValue == 4 );
	}

	p0MockEvent->setTriggerValue(8889, 50, 0); // group 8889, value, trigger after one tick

	oCheckTiles(4);

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 4 );

//std::cout << "--------" << '\n'; p0Level->dump(true, false, false, false, false, false);
	oCheckTiles(54);
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE( oEntry.m_nGameTick == 3 );
		REQUIRE( oEntry.m_nValue == 50 );
	}

	p0MockEvent->setTriggerValue(8889, 10, 0); // group 8889, value, trigger after one tick

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 5 );

//std::cout << "--------" << '\n'; p0Level->dump(true, false, false, false, false, false);
	oCheckTiles(60);

	const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
	REQUIRE_FALSE( oEntry.isEmpty() );
	REQUIRE( oEntry.m_nTag == 64738 );
	REQUIRE( oEntry.m_nGameTick == 4 );
	REQUIRE( oEntry.m_nLevel == 0 );
	REQUIRE( oEntry.m_nMsg == 55 );
	REQUIRE( oEntry.m_nValue == 6 );
	REQUIRE( oEntry.m_nTriggeringEventAddr == reinterpret_cast<int64_t>(m_p0FillerEvent) );
}


////////////////////////////////////////////////////////////////////////////////
class FillerEventCase2GameFixture : public FillerEventGameFixture
{
protected:
	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		FillerEventGameFixture::fillBoard(nBoardW, nBoardH, aBoard);
		Tile oTile;
		oTile.getTileChar().setChar(65);
		aBoard[Level::Init::getBoardIndex(NPoint{3,2}, NSize{nBoardW, nBoardH})] = oTile;
		aBoard[Level::Init::getBoardIndex(NPoint{5,0}, NSize{nBoardW, nBoardH})] = oTile;
		aBoard[Level::Init::getBoardIndex(NPoint{5,1}, NSize{nBoardW, nBoardH})] = oTile;
		aBoard[Level::Init::getBoardIndex(NPoint{0,5}, NSize{nBoardW, nBoardH})] = oTile;
		aBoard[Level::Init::getBoardIndex(NPoint{8,4}, NSize{nBoardW, nBoardH})] = oTile;
	}

	int32_t getTotTiles() const override
	{
		return 4;
	}
	FillerEvent::FILL_MODE getFillMode() const override
	{
		return FillerEvent::FILL_MODE_IGNORE;
	}
	unique_ptr<TileSelector> getTargetTiles() const override
	{
		auto refCTS = make_unique<CharTraitSet>(make_unique<CharUcs4TraitSet>(65));
		//TileSelector oTS{make_unique<TileSelector::Trait>(false, std::move(refCTS))};
		auto refSelect = make_unique<TileSelector>(make_unique<TileSelector::Trait>(false, std::move(refCTS)));
		return refSelect;
	}
	void setup() override
	{
		FillerEventGameFixture::setup();

		const int32_t nLevel = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		FillerEvent::Init oFInit;
		oFInit.m_p0Level = p0Level;
		oFInit.m_oFillGen.m_eFillMode = getFillMode();
		oFInit.m_oFillGen.m_nTotTiles = getTotTiles();
		oFInit.m_oFillGen.m_fFactorOfAvailablePositions = getFactorOfAvailablePositions();
		oFInit.m_oFillGen.m_oProbTileGen = getProbTileGen();
		oFInit.m_oCoords = getCoords();
		oFInit.m_refTargetTiles = getTargetTiles();

		auto refFillerEvent = make_unique<FillerEvent>(std::move(oFInit));
		m_p0FillerEvent = refFillerEvent.get();
		p0Level->addEvent(std::move(refFillerEvent));
		p0Level->deactivateEvent(m_p0FillerEvent);
	}
	void teardown() override
	{
		FillerEventGameFixture::teardown();
	}
protected:
	shared_ptr<Level> m_refLevel;
	FillerEvent* m_p0FillerEvent;
};

TEST_CASE_METHOD(STFX<FillerEventCase2GameFixture>, "Case2_Count")
{
	Level* p0Level = m_refLevel.get();
	const int32_t nBoardW = p0Level->boardWidth();
	const int32_t nBoardH = p0Level->boardHeight();

//std::cout << "--------" << '\n'; p0Level->dump(true, false, false, false, false, false);

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 64738;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );
	
	LogEvent::msgLog().reset();
	
	m_p0FillerEvent->addListener(FillerEvent::LISTENER_GROUP_FILLED, p0LogEvent, 55);

	MockEvent::Init oMockInit;
	oMockInit.m_p0Level = p0Level;
	auto refMockEvent = make_unique<MockEvent>(std::move(oMockInit));
	MockEvent* p0MockEvent = refMockEvent.get();
	p0Level->addEvent(std::move(refMockEvent));

	p0MockEvent->addListener(8889, m_p0FillerEvent, FillerEvent::MESSAGE_FILL);

	auto oCheckTiles = [&](int32_t nExpectedTiles) {
		int32_t nTotTiles = 0;
		for (int32_t nY = 0; nY < nBoardH; ++nY) {
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				const Tile& oTile = p0Level->boardGetTile(nX, nY);
				if (! oTile.getTileChar().isCharIndex()) {
					if (oTile.getTileChar().getChar() == 65) {
						++nTotTiles;
					}
				}
			}
		}
		REQUIRE( nTotTiles == nExpectedTiles );
	};
	
	p0Level->setFallEachTicks(4);
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 1 );

	p0MockEvent->setTriggerValue(8889, 0, 1); // group 8889, value, trigger after one tick

	oCheckTiles(5);

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );

	oCheckTiles(5);

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 3 );

	oCheckTiles(5); // nothing is really modified
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE( oEntry.m_nGameTick == 2 );
		REQUIRE( oEntry.m_nValue == 4 );
	}

	p0MockEvent->setTriggerValue(8889, 50, 0); // group 8889, value, trigger after one tick

	oCheckTiles(5);

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 4 );

//std::cout << "--------" << '\n'; p0Level->dump(true, false, false, false, false, false);
	oCheckTiles(5);

	const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
	REQUIRE_FALSE( oEntry.isEmpty() );
	REQUIRE( oEntry.m_nTag == 64738 );
	REQUIRE( oEntry.m_nGameTick == 3 );
	REQUIRE( oEntry.m_nLevel == 0 );
	REQUIRE( oEntry.m_nMsg == 55 );
	REQUIRE( oEntry.m_nValue == 5 );
	REQUIRE( oEntry.m_nTriggeringEventAddr == reinterpret_cast<int64_t>(m_p0FillerEvent) );
}


} // namespace testing

} // namespace stmg
