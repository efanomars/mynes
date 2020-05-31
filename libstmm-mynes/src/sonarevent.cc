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
 * File:   sonarevent.cc
 */

#include "sonarevent.h"

#include "sonarblock.h"

#include <stmm-games/named.h>
#include <stmm-games/gameproxy.h>
#include <stmm-games/level.h>
#include <stmm-games/levelblock.h>
#include <stmm-games/tile.h>
#include <stmm-games/utile/tilecoords.h>
#include <stmm-games/util/helpers.h>
#include <stmm-games/util/namedindex.h>
#include <stmm-games/util/coords.h>
#include <stmm-games/util/util.h>

#include <cassert>
#include <iostream>
#include <utility>
#include <array>

namespace stmg
{

constexpr const std::array<NPoint, 8> s_aDirDelta{NPoint{1,0}, NPoint{1,-1}, NPoint{0,-1}, NPoint{-1,-1}, NPoint{-1,0}, NPoint{-1,1}, NPoint{0,1}, NPoint{1,1}};

static const std::string s_sTileAniSonarName = "TILEANI:SONAR";

static const std::string s_sCharMineName = "MINE";

static const constexpr int32_t s_nSonarNumberDiff = 1000000;

SonarEvent::SonarEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_oData(std::move(oInit))
{
	commonInit();
}

void SonarEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}
int32_t SonarEvent::getNamedIndex(NamedIndex& oNamedIndex, const std::string& sName, const std::string&
									#ifndef NDEBUG
									sDescType
									#endif //NDEBUG
									) noexcept
{
	#ifndef NDEBUG
	int32_t nIndex = oNamedIndex.getIndex(sName);
	if (nIndex < 0) {
		std::cout << "Warning! SonarEvent: " << sDescType << " '" << sName << "' not defined!" << '\n';
		nIndex = oNamedIndex.addName(sName);
	}
	#else //NDEBUG
	const int32_t nIndex = oNamedIndex.addName(sName);
	#endif //NDEBUG
	return nIndex;
}
void SonarEvent::commonInit() noexcept
{
	// check no gaps in shape ids.
	#ifndef NDEBUG
	int32_t nShapeIdx = 0;
	int32_t nCurShapeId = m_oData.m_oBlock.shapeFirst();
	while (nCurShapeId >= 0) {
		assert(nShapeIdx == nCurShapeId);
		assert(m_oData.m_oBlock.shapeTotVisibleBricks(nCurShapeId) >= 2);
		nCurShapeId = m_oData.m_oBlock.shapeNext(nCurShapeId);
		++nShapeIdx;
	};
	assert((m_oData.m_nInitialShape >= -1) && (m_oData.m_nInitialShape < nShapeIdx));
	#endif //NDEBUG

	assert((m_oData.m_oBlockRotateInterval.m_oTicks.m_nFrom >= 0)
			&& (m_oData.m_oBlockRotateInterval.m_oTicks.m_nFrom <= m_oData.m_oBlockRotateInterval.m_oTicks.m_nTo));
	assert((m_oData.m_oBlockRotateInterval.m_oMillisec.m_nFrom >= 0)
			&& (m_oData.m_oBlockRotateInterval.m_oMillisec.m_nFrom <= m_oData.m_oBlockRotateInterval.m_oMillisec.m_nTo));
	if (m_oData.m_bLaunches) {
		assert((m_oData.m_oDirRotateInterval.m_oTicks.m_nFrom >= 0)
				&& (m_oData.m_oDirRotateInterval.m_oTicks.m_nFrom <= m_oData.m_oDirRotateInterval.m_oTicks.m_nTo));
		assert((m_oData.m_oDirRotateInterval.m_oMillisec.m_nFrom >= 0)
				&& (m_oData.m_oDirRotateInterval.m_oMillisec.m_nFrom <= m_oData.m_oDirRotateInterval.m_oMillisec.m_nTo));
	}

	Level& oLevel = level();
	NamedIndex& oCharsIndex = oLevel.getNamed().chars();
	m_nCharMineIndex = getNamedIndex(oCharsIndex, s_sCharMineName, "char");

	NamedIndex& oTileAnisIndex = oLevel.getNamed().tileAnis();
	m_nTileAniSonarIndex = getNamedIndex(oTileAnisIndex, s_sTileAniSonarName, "tile ani");

	m_nBoardW = oLevel.boardWidth();
	m_nBoardH = oLevel.boardHeight();

	assert((m_oData.m_oArea.m_nX >= 0) && (m_oData.m_oArea.m_nW >= 1) && (m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW <= m_nBoardW));
	assert((m_oData.m_oArea.m_nY >= 0) && (m_oData.m_oArea.m_nH >= 1) && (m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH <= m_nBoardH));

	assert(m_oData.m_refSonarTile);

	//
	runtimeInit();
}
void SonarEvent::runtimeInit() noexcept
{
	// run-time
	m_eState = SONAR_STATE_ACTIVATE;
	m_aSonarCells.clear();
	m_aNextBlockRotationTicks.clear();
	m_aNextDirRotationTicks.clear();
	m_aBlockData.clear();
	m_aUpdateMinesNumber.clear();
	m_nActiveSonars = 0;
	m_nLockCount = 0;
	m_nLastBlockRotationTick = -1;
}

void SonarEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
	Level& oLevel = level();
	auto& oGame = oLevel.game();
	const int32_t nTimer = oGame.gameElapsed();
//std::cout << "SonarEvent::trigger nTimer=" << nTimer << '\n';
	switch (m_eState)
	{
		case SONAR_STATE_ACTIVATE:
		{
			m_eState = SONAR_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nTimer);
				break;
			}
		} //fallthrough
		case SONAR_STATE_INIT:
		{
			m_eState = SONAR_STATE_RUN;
			
			oLevel.boardAddListener(this);
			checkForSonarCells(m_oData.m_oArea);
		} //fallthrough
		case SONAR_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				if (nMsg == SonarEvent::MESSAGE_ACTIVATE_SONAR_AT) {
					const NPoint oXY = Util::unpackPointFromInt32(nValue);
//std::cout << "SonarEvent::trigger  ACTIVATE_SONAR_AT  nX=" << oXY.m_nX << " nY=" << oXY.m_nY << '\n';
					checkForSonarCell(oXY.m_nX, oXY.m_nY);
				}
				reactivateEvent();
				break; //switch
			}
			m_nLastBlockRotationTick = nTimer;
			checkForNumberUpdates();
			checkForBlockRotations(oGame, nTimer);
			if (m_oData.m_bLaunches) {
				checkForDirRotations(oGame, nTimer);
				checkBlockMoved();
			}
			reactivateEvent();
		}
		break;
	}
}
void SonarEvent::setSonarCellValue(int32_t nSonarIdx) noexcept
{
	Level& oLevel = level();

	const NPoint& oXY = m_aSonarCells[nSonarIdx];
	const int32_t nX = oXY.m_nX;
	const int32_t nY = oXY.m_nY;
	SonarData& oSonarData = m_aBlockData[nSonarIdx];
	const int32_t nCurShape = oSonarData.m_nCurShape;

	auto& refSonarBlock = oSonarData.m_refBlock;
	assert(refSonarBlock);

	const NPoint oPos = refSonarBlock->blockPos();
	const int32_t nBlockX = oPos.m_nX;
	const int32_t nBlockY = oPos.m_nY;
	const std::vector<int32_t>& aBrickIds = m_oData.m_oBlock.brickIds();
	int32_t nTotMines = 0;
	for (const int32_t& nBrickId : aBrickIds) {
		if (m_oData.m_oBlock.shapeBrickVisible(nCurShape, nBrickId)) {
			const int32_t nBrickX = m_oData.m_oBlock.shapeBrickPosX(nCurShape, nBrickId);
			const int32_t nBrickY = m_oData.m_oBlock.shapeBrickPosY(nCurShape, nBrickId);
			const int32_t nBoardX = nBlockX + nBrickX;
			const int32_t nBoardY = nBlockY + nBrickY;
			if ((nBoardX >= 0) && (nBoardX < m_nBoardW) && (nBoardY >= 0) && (nBoardY < m_nBoardH)) {
				const Tile& oTile = oLevel.boardGetTile(nBoardX, nBoardY);
				const TileChar& oChar = oTile.getTileChar();
				if (oChar.isCharIndex() && (static_cast<int32_t>(oChar.getCharIndex()) == m_nCharMineIndex)) {
					++nTotMines;
				}
			}
		}
	}
	Tile oSonarTile = oLevel.boardGetTile(nX, nY);
//std::cout << "SonarEvent::setSonarCellValue nSonarIdx=" << nSonarIdx << "  nTotMines=" << nTotMines << '\n';
	nTotMines = nTotMines % 10;
	const uint32_t nNumberChar = static_cast<uint32_t>('0') + static_cast<uint32_t>(nTotMines + s_nSonarNumberDiff);
//std::cout << "          ::setSonarCellValue nNumberChar=" << nNumberChar << "  nX=" << nX << "  nY=" << nY << '\n';
	oSonarTile.getTileChar().setChar(nNumberChar);
	lockedSetTile(nX, nY, oSonarTile);
}
void SonarEvent::checkForNumberUpdates() noexcept
{
	// Takes for granted that no sonar is removed while updating the sonar cells
	// and that there are nested modifications that would need to check again the
	// already checked sonars
	const int32_t nTotSonars = static_cast<int32_t>(m_aUpdateMinesNumber.size());
//std::cout << "SonarEvent::checkForNumberUpdates nTotSonars=" << nTotSonars << '\n';
	for (int32_t nSonarIdx = 0; nSonarIdx < nTotSonars; ++nSonarIdx) {
		const bool bUpdate = m_aUpdateMinesNumber[nSonarIdx];
		if (bUpdate) {
			setSonarCellValue(nSonarIdx);
			m_aUpdateMinesNumber[nSonarIdx] = false;
		}
	}
}
void SonarEvent::checkForBlockRotations(GameProxy& oGame, int32_t nTimer) noexcept
{
	const int32_t nTotSonars = static_cast<int32_t>(m_aUpdateMinesNumber.size());
	for (int32_t nSonarIdx = 0; nSonarIdx < nTotSonars; ++nSonarIdx) {
		int32_t& nNextRotationTicks = m_aNextBlockRotationTicks[nSonarIdx];
		if (nNextRotationTicks >= 0) {
			// active
			if (nNextRotationTicks <= nTimer) {
				rotateBlock(nSonarIdx);
				m_aUpdateMinesNumber[nSonarIdx] = true;
				nNextRotationTicks = nTimer + geTicksToBlockRotation(oGame);
			}
		}
	}
}
void SonarEvent::checkForDirRotations(GameProxy& oGame, int32_t nTimer) noexcept
{
//std::cout << "SonarEvent::checkForDirRotations 1" << '\n';
	const int32_t nTotSonars = static_cast<int32_t>(m_aNextDirRotationTicks.size());
	for (int32_t nSonarIdx = 0; nSonarIdx < nTotSonars; ++nSonarIdx) {
		int32_t& nNextRotationTicks = m_aNextDirRotationTicks[nSonarIdx];
		if (nNextRotationTicks >= 0) {
			// ready to launch
			if (nNextRotationTicks <= nTimer) {
				rotateDir(nSonarIdx);
				nNextRotationTicks = nTimer + geTicksToDirRotation(oGame);
			}
		}
	}
}
void SonarEvent::checkBlockMoved() noexcept
{
	const int32_t nTotSonars = static_cast<int32_t>(m_aBlockData.size());
	for (int32_t nSonarIdx = 0; nSonarIdx < nTotSonars; ++nSonarIdx) {
		SonarData& oSonarData = m_aBlockData[nSonarIdx];
		if (oSonarData.m_bLaunched && oSonarData.m_refBlock->changedPosition()) {
			m_aUpdateMinesNumber[nSonarIdx] = true;
		}
	}
}
void SonarEvent::rotateBlock(int32_t nSonarIdx) noexcept
{
	SonarData& oSonarData = m_aBlockData[nSonarIdx];
	auto& refSonarBlock = oSonarData.m_refBlock;
	assert(refSonarBlock);

	int32_t& nCurShapeId = oSonarData.m_nCurShape;
	assert(nCurShapeId >= 0);
	nCurShapeId = m_oData.m_oBlock.shapeNext(nCurShapeId);
	if (nCurShapeId < 0) {
		nCurShapeId = m_oData.m_oBlock.shapeFirst();
	}
	refSonarBlock->blockMoveRotate(nCurShapeId, 0,0);
}
void SonarEvent::rotateDir(int32_t nSonarIdx) noexcept
{
	SonarData& oSonarData = m_aBlockData[nSonarIdx];
	++oSonarData.m_nCurLaunchDirIdx;
	if (oSonarData.m_nCurLaunchDirIdx >= static_cast<int32_t>(s_aDirDelta.size())) {
		oSonarData.m_nCurLaunchDirIdx = 0;
	}
	level().boardAnimateTile(m_aSonarCells[nSonarIdx]);
//std::cout << "SonarEvent::rotateDir oSonarData.m_nCurLaunchDirIdx=" << oSonarData.m_nCurLaunchDirIdx << '\n';
}
void SonarEvent::reactivateEvent() noexcept
{
	if (m_nActiveSonars > 0) {
		const int32_t nTimer = level().game().gameElapsed();
		const bool bNotRotatedYetThisTick = (nTimer > m_nLastBlockRotationTick);
		level().activateEvent(this, nTimer + (bNotRotatedYetThisTick ? 0 : 1));
	}
}

int32_t SonarEvent::getSonarIdxFromPos(int32_t nX, int32_t nY) const noexcept
{
	const NPoint oFindXY = NPoint{nX, nY};
	const auto itFind = std::find_if(m_aSonarCells.cbegin(), m_aSonarCells.cend(), [oFindXY](const NPoint& oXY)
	{
		return (oXY == oFindXY);
	});
	if (itFind == m_aSonarCells.cend()) {
		return -1;
	}
	return static_cast<int32_t>(std::distance(m_aSonarCells.cbegin(), itFind));
}
void SonarEvent::maybeCreateSonarCell(int32_t nX, int32_t nY) noexcept
{
//std::cout << "SonarEvent::maybeCreateSonarCell 0" << '\n';
	Level& oLevel = level();
	const TileAnimator* p0TA = oLevel.boardGetTileAnimator(nX, nY, m_nTileAniSonarIndex);
	if (p0TA != nullptr) {
		if (p0TA == this) {
			const int32_t nSonarIdx = getSonarIdxFromPos(nX, nY);
			assert(nSonarIdx >= 0);
			SonarData& oSonarData = m_aBlockData[nSonarIdx];
			auto& refSonarBlock = oSonarData.m_refBlock;
			if (! refSonarBlock) {
				activateSonar(nX, nY, nSonarIdx);
			} else if (m_oData.m_bLaunches && ! oSonarData.m_bLaunched) {
				launchSonar(nSonarIdx);
			}
		}
		return; //--------------------------------------------------------------
	}
//std::cout << "SonarEvent::maybeCreateSonarCell 1" << '\n';
	const Tile& oTile = oLevel.boardGetTile(nX, nY);
	const bool bSelected = m_oData.m_refSonarTile->select(oTile);
	if (! bSelected) {
		return; //--------------------------------------------------------------
	}
//std::cout << "SonarEvent::maybeCreateSonarCell 2" << '\n';
	createSonar(nX, nY);
}
void SonarEvent::checkActiveSonarsCovering(int32_t nX, int32_t nY) noexcept
{
	const int32_t nTotSonars = static_cast<int32_t>(m_aSonarCells.size());
	for (int32_t nSonarIdx = 0; nSonarIdx < nTotSonars; ++nSonarIdx) {
		const SonarData& oSonarData = m_aBlockData[nSonarIdx];
		if (oSonarData.m_nCurShape >= 0) {
			NRect oShapeRect{};
			const NPoint oPos = oSonarData.m_refBlock->blockPos();
			oShapeRect.m_nX = oPos.m_nX;
			oShapeRect.m_nY = oPos.m_nY;
			oShapeRect.m_nX += m_oData.m_oBlock.shapeMinX(oSonarData.m_nCurShape);
			oShapeRect.m_nY += m_oData.m_oBlock.shapeMinY(oSonarData.m_nCurShape);
			oShapeRect.m_nW = m_oData.m_oBlock.shapeWidth(oSonarData.m_nCurShape);
			oShapeRect.m_nH = m_oData.m_oBlock.shapeHeight(oSonarData.m_nCurShape);
			if (oShapeRect.containsPoint(NPoint{nX, nY})) {
				m_aUpdateMinesNumber[nSonarIdx] = true;
			}
		}
	}
}
void SonarEvent::checkActiveSonarsCovering(const NRect& oRect) noexcept
{
	const int32_t nTotSonars = static_cast<int32_t>(m_aSonarCells.size());
	for (int32_t nSonarIdx = 0; nSonarIdx < nTotSonars; ++nSonarIdx) {
		const SonarData& oSonarData = m_aBlockData[nSonarIdx];
		if (oSonarData.m_nCurShape >= 0) {
			NRect oShapeRect{};
			const NPoint oPos = oSonarData.m_refBlock->blockPos();
			oShapeRect.m_nX = oPos.m_nX;
			oShapeRect.m_nY = oPos.m_nY;
			oShapeRect.m_nX += m_oData.m_oBlock.shapeMinX(oSonarData.m_nCurShape);
			oShapeRect.m_nY += m_oData.m_oBlock.shapeMinY(oSonarData.m_nCurShape);
			oShapeRect.m_nW = m_oData.m_oBlock.shapeWidth(oSonarData.m_nCurShape);
			oShapeRect.m_nH = m_oData.m_oBlock.shapeHeight(oSonarData.m_nCurShape);
			if (NRect::doIntersect(oShapeRect, oRect)) {
				m_aUpdateMinesNumber[nSonarIdx] = true;
			}
		}
	}
}
void SonarEvent::checkForSonarCell(int32_t nX, int32_t nY) noexcept
{
	checkActiveSonarsCovering(nX, nY);
	maybeCreateSonarCell(nX, nY);
}
void SonarEvent::checkForSonarCells(const NRect& oRect) noexcept
{
//std::cout << "SonarEvent::checkForHoles" << '\n';
	checkActiveSonarsCovering(oRect);
	for (int32_t nX = oRect.m_nX; nX < oRect.m_nX + oRect.m_nW; ++nX) {
		for (int32_t nY = oRect.m_nY; nY < oRect.m_nY + oRect.m_nH; ++nY) {
			maybeCreateSonarCell(nX, nY);
		}
	}
}
void SonarEvent::checkForSonarCells(const Coords& oCoords) noexcept
{
	for (auto it = oCoords.begin(); it != oCoords.end(); it.next()) {
		const int32_t nX = it.x();
		const int32_t nY = it.y();
		checkActiveSonarsCovering(nX, nY);
		maybeCreateSonarCell(nX, nY);
	}
}
void SonarEvent::checkForSonarCells(const LevelBlock& oBlock) noexcept
{
	NRect oRect;
	const NPoint oMinPos = oBlock.blockBricksMinPos();
	oRect.m_nX = oMinPos.m_nX;
	oRect.m_nY = oMinPos.m_nY;
	const NSize oSize = oBlock.blockSize();
	oRect.m_nW = oSize.m_nW;
	oRect.m_nH = oSize.m_nH;
	checkForSonarCells(oRect);
}
void SonarEvent::createSonar(int32_t nX, int32_t nY) noexcept
{
//std::cout << "SonarEvent::createSonar nX=" << nX << "  nY=" << nY << '\n';
	Level& oLevel = level();

	const int32_t nNewIdx = static_cast<int32_t>(m_aSonarCells.size());
	m_aSonarCells.push_back(NPoint{nX, nY});
	m_aNextBlockRotationTicks.push_back(-1);
	m_aNextDirRotationTicks.push_back(-1);
	m_aBlockData.push_back(SonarData{});
	m_aUpdateMinesNumber.push_back(false);
	oLevel.boardSetTileAnimator(nX, nY, m_nTileAniSonarIndex, this, nNewIdx);
	oLevel.boardAnimateTile(NPoint{nX, nY});
	if (m_oData.m_bAutoCreated) {
		// create sonar block
		activateSonar(nX, nY, nNewIdx);
	}
}
int32_t SonarEvent::geTicksToBlockRotation(GameProxy& oGame) noexcept
{
	return geTicksFromTimeRange(m_oData.m_oBlockRotateInterval, oGame);
}
int32_t SonarEvent::geTicksToDirRotation(GameProxy& oGame) noexcept
{
	return geTicksFromTimeRange(m_oData.m_oDirRotateInterval, oGame);
}
int32_t SonarEvent::geTicksFromTimeRange(const NTimeRange& oTimeRange, GameProxy& oGame) noexcept
{
	const NRange oRotate = oTimeRange.getCumulatedTicksRange(oGame.gameInterval());
	assert(oRotate.m_nFrom <= oRotate.m_nTo);
	const int32_t nTicksToRotation = oGame.random(oRotate.m_nFrom, oRotate.m_nTo);
	return std::max(1, nTicksToRotation);
}
void SonarEvent::activateSonar(int32_t nX, int32_t nY, int32_t nSonarIdx) noexcept
{
//std::cout << "SonarEvent::activateSonar nX=" << nX << "  nY=" << nY << " nSonarIdx=" << nSonarIdx << '\n';
	SonarData& oSonarData = m_aBlockData[nSonarIdx];
	auto& refSonarBlock = oSonarData.m_refBlock;
	assert(! refSonarBlock);
	Level& oLevel = level();
	auto& oGame = oLevel.game();

	const int32_t nTotShapes = m_oData.m_oBlock.totShapes();
	int32_t nInitialShape = m_oData.m_nInitialShape;
	if (nInitialShape < 0) {
		nInitialShape = oGame.random(0, nTotShapes - 1);
	}
	oSonarData.m_nCurShape = nInitialShape;

	NPoint oPos{nX, nY};
	oPos.m_nX += m_oData.m_oRelPos.m_nX;
	oPos.m_nY += m_oData.m_oRelPos.m_nY;
	m_oSonarBlockRecycler.create(refSonarBlock, m_oData.m_oBlock, nInitialShape, oPos);
	oLevel.blockAdd(refSonarBlock.get(), LevelBlock::MGMT_TYPE_AUTO_SCROLL);

	++m_nActiveSonars;

	const int32_t nTimer = oGame.gameElapsed();
	m_aNextBlockRotationTicks[nSonarIdx] = nTimer + geTicksToBlockRotation(oGame);
	if (m_oData.m_bLaunches) {
		oSonarData.m_nCurLaunchDirIdx = 0;
		oLevel.boardAnimateTile(NPoint{nX, nY});
		m_aNextDirRotationTicks[nSonarIdx] = nTimer + geTicksToDirRotation(oGame);
	}

	m_aUpdateMinesNumber[nSonarIdx] = true;

	reactivateEvent();
	//NOT HERE! setSonarCellValue(nSonarIdx);

	const int32_t nValue = Util::packPointToInt32(NPoint{nX, nY});
	informListeners(LISTENER_GROUP_SONAR_CREATED, nValue);
}
void SonarEvent::launchSonar(int32_t nSonarIdx) noexcept
{
//std::cout << "SonarEvent::launchSonar nSonarIdx=" << nSonarIdx << '\n';
	SonarData& oSonarData = m_aBlockData[nSonarIdx];
	oSonarData.m_bLaunched = true;
	m_aNextBlockRotationTicks[nSonarIdx] = -1;
	m_aNextDirRotationTicks[nSonarIdx] = -1;

	const NPoint& oXY = m_aSonarCells[nSonarIdx];
	const int32_t nValue = Util::packPointToInt32(oXY);

	assert(oSonarData.m_nCurLaunchDirIdx >= 0);
	const NPoint& oDeltaXY = s_aDirDelta[oSonarData.m_nCurLaunchDirIdx];
	NRect oInRect{- 2 * m_nBoardW, - 2 * m_nBoardH, m_nBoardW * 5, m_nBoardH * 5};
	oSonarData.m_refBlock->launch(oDeltaXY, oInRect);

	reactivateEvent();

	informListeners(LISTENER_GROUP_SONAR_LAUNCHED, nValue);
}
void SonarEvent::removeSonar(int32_t nSonarIdx) noexcept
{
	SonarData& oSonarData = m_aBlockData[nSonarIdx];
	// save block
	auto refSonarBlock = oSonarData.m_refBlock;

	Level& oLevel = level();

	const NPoint& oXY = m_aSonarCells[nSonarIdx];
	const int32_t nX = oXY.m_nX;
	const int32_t nY = oXY.m_nY;
//std::cout << "SonarEvent::removeSonar nX=" << nX << "  nY=" << nY << " nSonarIdx=" << nSonarIdx << '\n';
	std::swap(m_aSonarCells[nSonarIdx], m_aSonarCells.back());
	std::swap(m_aBlockData[nSonarIdx], m_aBlockData.back());
	std::swap(m_aUpdateMinesNumber[nSonarIdx], m_aUpdateMinesNumber.back());
	std::swap(m_aNextBlockRotationTicks[nSonarIdx], m_aNextBlockRotationTicks.back());
	std::swap(m_aNextDirRotationTicks[nSonarIdx], m_aNextDirRotationTicks.back());
	m_aSonarCells.pop_back();
	m_aBlockData.pop_back();
	m_aUpdateMinesNumber.pop_back();
	m_aNextBlockRotationTicks.pop_back();
	m_aNextDirRotationTicks.pop_back();
	if (refSonarBlock) {
		// was active
		--m_nActiveSonars;
		refSonarBlock->remove();
	}
	//
	oLevel.boardSetTileAnimator(nX, nY, m_nTileAniSonarIndex, nullptr, 0);
	oLevel.boardAnimateTile(NPoint{nX, nY});

	reactivateEvent();

	const int32_t nValue = Util::packPointToInt32(NPoint{nX, nY});
	informListeners(LISTENER_GROUP_SONAR_REMOVED, nValue);
}
void SonarEvent::lockedSetTile(int32_t nX, int32_t nY, const Tile& oTile) noexcept
{
	assert(m_nLockCount == 0);
	++m_nLockCount;
	level().boardSetTile(nX, nY, oTile);
	--m_nLockCount;
}

void SonarEvent::boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& /*refTiles*/) noexcept
{
	// remove all sonars that will be scrolled outside the area
	const int32_t nDeltaX = Direction::deltaX(eDir);
	const int32_t nDeltaY = Direction::deltaY(eDir);
	for (int32_t nSonarIdx = 0; nSonarIdx < static_cast<int32_t>(m_aSonarCells.size()); ++nSonarIdx) {
		const NPoint& oXY = m_aSonarCells[nSonarIdx];
		const int32_t nNewX = oXY.m_nX + nDeltaX;
		const int32_t nNewY = oXY.m_nY + nDeltaY;
		const bool bKeep = m_oData.m_oArea.containsPoint(NPoint{nNewX, nNewY});
		if (! bKeep) {
			removeSonar(nSonarIdx);
			// Since the last m_aSonarCells has moved to nSonarIdx redo it
			--nSonarIdx;
		}
	}
}
void SonarEvent::boardPostScroll(Direction::VALUE eDir) noexcept
{
	// move all sonars that will not be scrolled outside the area
	const int32_t nDeltaX = Direction::deltaX(eDir);
	const int32_t nDeltaY = Direction::deltaY(eDir);
	for (int32_t nSonarIdx = 0; nSonarIdx < static_cast<int32_t>(m_aSonarCells.size()); ++nSonarIdx) {
		NPoint& oXY = m_aSonarCells[nSonarIdx];
		oXY.m_nX += nDeltaX;
		oXY.m_nY += nDeltaY;
		#ifndef NDEBUG
		const bool bInArea = m_oData.m_oArea.containsPoint(oXY);
		assert(bInArea);
		#endif //NDEBUG
	}
	const bool bDirIsUp = (eDir == Direction::UP);
	if (bDirIsUp || (eDir == Direction::DOWN)) {
		if (bDirIsUp) {
			// new tiles have been added in the bottom row of the board
			checkForSonarCells(NRect{0, m_nBoardH - 1, m_nBoardW, 1});
		} else {
			// new tiles have been added in the top row of the board
			checkForSonarCells(NRect{0, 0, m_nBoardW, 1});
		}
	} else {
		const bool bDirIsLeft = (eDir == Direction::LEFT);
		if (bDirIsLeft) {
			// new tiles have been added in the rightmost column of the board
			checkForSonarCells(NRect{m_nBoardW - 1, 0, 1, m_nBoardH});
		} else {
			assert(eDir == Direction::RIGHT);
			// new tiles have been added in the leftmost column of the board
			checkForSonarCells(NRect{0, 0, 1, m_nBoardH});
		}
	}
}
void SonarEvent::removeSonarsIn(const NRect& oRect) noexcept
{
	for (int32_t nSonarIdx = 0; nSonarIdx < static_cast<int32_t>(m_aSonarCells.size()); ++nSonarIdx) {
		const NPoint& oXY = m_aSonarCells[nSonarIdx];
		if (oRect.containsPoint(oXY)) {
			removeSonar(nSonarIdx);
			// Since the last m_aSonarCells has moved to nSonarIdx redo it
			--nSonarIdx;
		}
	}
}
void SonarEvent::removeSonarsIn(const Coords& oCoords) noexcept
{
	for (int32_t nSonarIdx = 0; nSonarIdx < static_cast<int32_t>(m_aSonarCells.size()); ++nSonarIdx) {
		const NPoint& oXY = m_aSonarCells[nSonarIdx];
		if (oCoords.contains(oXY)) {
			removeSonar(nSonarIdx);
			// Since the last m_aSonarCells has moved to nSonarIdx redo it
			--nSonarIdx;
		}
	}
}
void SonarEvent::boabloPreFreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void SonarEvent::boabloPostFreeze(const Coords& oCoords) noexcept
{
	removeSonarsIn(oCoords);
	checkForSonarCells(oCoords);
	//oLevel.gameStatusTechnical(std::vector<std::string>{"SonarEvent::boabloPostFreeze", "Unsupported"});
}
void SonarEvent::boabloPreUnfreeze(const Coords& oCoords) noexcept
{
	removeSonarsIn(oCoords);
	//oLevel.gameStatusTechnical(std::vector<std::string>{"SonarEvent::boabloPostFreeze", "Unsupported"});
}
void SonarEvent::boabloPostUnfreeze(LevelBlock& oBlock) noexcept
{
	checkForSonarCells(oBlock);
}
void SonarEvent::boardPreInsert(Direction::VALUE eDir, NRect oRect, const shared_ptr<TileRect>& /*refTiles*/) noexcept
{
	// remove all sonars that will be deleted by insertion
	NRect oRemoveRect = Helpers::boardInsertRemove(oRect, eDir);
	removeSonarsIn(oRemoveRect);
	// remove all sonars that will be scrolled outside the area
	const int32_t nDeltaX = Direction::deltaX(eDir);
	const int32_t nDeltaY = Direction::deltaY(eDir);
	for (int32_t nSonarIdx = 0; nSonarIdx < static_cast<int32_t>(m_aSonarCells.size()); ++nSonarIdx) {
		const NPoint& oXY = m_aSonarCells[nSonarIdx];
		const int32_t nNewX = oXY.m_nX + nDeltaX;
		const int32_t nNewY = oXY.m_nY + nDeltaY;
		const bool bKeep = m_oData.m_oArea.containsPoint(NPoint{nNewX, nNewY});
		if (! bKeep) {
			removeSonar(nSonarIdx);
			// Since the last m_aSonarCells has moved to nSonarIdx redo it
			--nSonarIdx;
		}
	}
}
void SonarEvent::boardPostInsert(Direction::VALUE eDir, NRect oRect) noexcept
{
	// add new sonars
	NRect oAddRect = Helpers::boardInsertAdd(oRect, eDir);
	checkForSonarCells(oAddRect);
}
void SonarEvent::boardPreDestroy(const Coords& oCoords) noexcept
{
	removeSonarsIn(oCoords);
}
void SonarEvent::boardPostDestroy(const Coords& oCoords) noexcept
{
	// if we assume that empty tiles can't be sonars
	// the following is not needed
	checkForSonarCells(oCoords);
}
void SonarEvent::boardPreModify(const TileCoords& oTileCoords) noexcept
{
	if (m_nLockCount > 0) {
		if (m_nLockCount > 1) {
			level().gameStatusTechnical(std::vector<std::string>{"SonarEvent doesn't allow nested modifications"});
		}
		return;
	}
	removeSonarsIn(oTileCoords);
}
void SonarEvent::boardPostModify(const Coords& oCoords) noexcept
{
	if (m_nLockCount > 0) {
		if (m_nLockCount > 1) {
			level().gameStatusTechnical(std::vector<std::string>{"SonarEvent doesn't allow nested modifications"});
		}
		return;
	}
	checkForSonarCells(oCoords);
}

double SonarEvent::getElapsed01(int32_t nHash, int32_t nX, int32_t nY, int32_t /*nAni*/
								, int32_t /*nViewTick*/, int32_t /*nTotViewTicks*/) const noexcept
{
	int32_t nIdx = nHash;
	if ((nIdx < 0) || (nIdx >= static_cast<int32_t>(m_aSonarCells.size()) || (! (m_aSonarCells[nIdx] == NPoint{nX, nY})))) {
		nIdx = getSonarIdxFromPos(nX, nY);
		assert(nIdx >= 0);
	}
	const SonarData& oSonarData = m_aBlockData[nIdx];
	if (oSonarData.m_nCurLaunchDirIdx < 0) {
		return 0.0; //----------------------------------------------------------
	}
	assert(oSonarData.m_nCurLaunchDirIdx < static_cast<int32_t>(s_aDirDelta.size()));
	// activated and launches
	// expects theme to have a TileAniModifier (or similar) with 10 images of the same weight
	// image 0: empty
	// image 1: right
	// image 2: right-up
	// image 3: up
	// image 4: left-up
	// image 5: left
	// image 6: left-down
	// image 7: down
	// image 8: right-down
	// image 9: unused
	// Note: 0.15 means the middle (to avoid floating point precision errors) of the second slice
	return 0.15 +  0.1 * oSonarData.m_nCurLaunchDirIdx;
}
double SonarEvent::getElapsed01(int32_t /*nHash*/, const LevelBlock& /*oLevelBlock*/, int32_t /*nBrickIdx*/, int32_t /*nAni*/
								, int32_t /*nViewTick*/, int32_t /*nTotTicks*/) const noexcept
{
	assert(false);
	return -1.0;
}

} // namespace stmg
