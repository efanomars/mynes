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
 * File:   pressevent.cc
 */

#include "pressevent.h"

#include <stmm-games/named.h>
#include <stmm-games/level.h>
#include <stmm-games/levelblock.h>
#include <stmm-games/util/namedindex.h>
#include <stmm-games/util/util.h>
#include <stmm-games/utile/tilerect.h>
#include <stmm-games/util/coords.h>

#include <cassert>
#include <iostream>


namespace stmg
{

const int32_t PressEvent::s_nInvalidXY = Util::packPointToInt32(NPoint{-1000,-1000});

PressEvent::PressEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_oData(std::move(oInit))
{
	commonInit();
}
void PressEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}
void PressEvent::commonInit() noexcept
{
	m_nBoardW = level().boardWidth();
	m_nBoardH = level().boardHeight();

	if (m_oData.m_oArea.m_nW <= 0) {
		m_oData.m_oArea.m_nW = m_nBoardW - 1 - m_oData.m_oArea.m_nX;
	}
	if (m_oData.m_oArea.m_nH <= 0) {
		m_oData.m_oArea.m_nH = m_nBoardH - 1 - m_oData.m_oArea.m_nY;
	}
	assert((m_oData.m_oArea.m_nX >= 0) && (m_oData.m_oArea.m_nW > 0) && (m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW <= m_nBoardW));
	assert((m_oData.m_oArea.m_nY >= 0) && (m_oData.m_oArea.m_nH > 0) && (m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH <= m_nBoardH));

	#ifndef NDEBUG
	NamedIndex& oTileAnisIndex = level().getNamed().tileAnis();
	oTileAnisIndex.isIndex(m_oData.m_nAniNameIdx);
	#endif //NDEBUG
	m_nPressAXY = s_nInvalidXY;

	m_eState = PRESS_STATE_ACTIVATE;
}

void PressEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
//std::cout << "PressEvent::trigger" << '\n';
	//TODO
	// ACTIVATE activate event
	// INIT init and register
	// RUN
	// last RUN disactivate, unregister, state to INIT
	//          send messages to finished-group listeners
	switch (m_eState)
	{
		case PRESS_STATE_ACTIVATE:
		{
			m_eState = PRESS_STATE_INIT;
		} // fallthrough
		case PRESS_STATE_INIT:
		{
			m_eState = PRESS_STATE_RUN;
			level().boardAddListener(this);
		} // fallthrough
		case PRESS_STATE_RUN:
		{
			if (p0TriggeringEvent == nullptr) {
				return; //------------------------------------------------------
			}
			switch (nMsg) {
				case MESSAGE_BUTTON_MOVE: {
					if (nValue == m_nPressAXY) {
						return; //----------------------------------------------
					}
					unPress(true);
					press(nValue);
				} break;
				case MESSAGE_BUTTON_PRESS: {
					press(nValue);
				} break;
				case MESSAGE_BUTTON_RELEASE: {
					unPress(false);
				} break;
				case MESSAGE_BUTTON_RELEASE_CANCEL: {
					unPress(true);
				} break;
			}
			break; //switch
		}
		break;
	}
}
void PressEvent::unPress(bool bCancel) noexcept
{
	Level& oLevel = level();
	if (m_nPressAXY == s_nInvalidXY) {
		return;
	}
	const NPoint oOldXY = Util::unpackPointFromInt32(m_nPressAXY);
//std::cout << "PressEvent::unPress oldX=" << oOldXY.m_nX << " oldY=" << oOldXY.m_nY << '\n';
	const int32_t nOldX = oOldXY.m_nX;
	const int32_t nOldY = oOldXY.m_nY;
	if (isInActiveArea(nOldX, nOldY)) {
		assert(oLevel.boardGetTileAnimator(nOldX, nOldY, m_oData.m_nAniNameIdx) != nullptr);
		oLevel.boardSetTileAnimator(nOldX, nOldY, m_oData.m_nAniNameIdx, nullptr, 0);
		oLevel.boardAnimateTile(NPoint{nOldX, nOldY});
	}
	const int32_t nPressAXY  = m_nPressAXY;
	m_nPressAXY = s_nInvalidXY;
	if (! bCancel) {
		informListeners(LISTENER_GROUP_RELEASE, nPressAXY);
	}
}
void PressEvent::press(int32_t nPackedXY) noexcept
{
	Level& oLevel = level();
	if (m_nPressAXY != s_nInvalidXY) {
		oLevel.gameStatusTechnical(std::vector<std::string>{"ManesEvent::press", "Unpress first"});
		return; //--------------------------------------------------------------
	}

	const NPoint oNewXY = Util::unpackPointFromInt32(nPackedXY);
	const int32_t nNewX = oNewXY.m_nX;
	const int32_t nNewY = oNewXY.m_nY;
//std::cout << "PressEvent::press newX=" << oNewXY.m_nX << " newY=" << oNewXY.m_nY << '\n';
	if (!isInActiveArea(nNewX, nNewY)) {
		return; //--------------------------------------------------------------
	}
	if (oLevel.boardGetTileAnimator(nNewX, nNewY, m_oData.m_nAniNameIdx) != nullptr) {
		// Some other is already pressing
		return; //--------------------------------------------------------------
	}
	m_nPressAXY = nPackedXY;
	oLevel.boardSetTileAnimator(nNewX, nNewY, m_oData.m_nAniNameIdx, this, 0);
	oLevel.boardAnimateTile(NPoint{nNewX, nNewY});
}
bool PressEvent::isInActiveArea(int32_t nX, int32_t nY) const noexcept
{
	return m_oData.m_oArea.containsPoint({nX, nY});
}

void PressEvent::boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept
{
	NRect oRect;
	oRect.m_nW = m_nBoardW;
	oRect.m_nH = m_nBoardH;
	boardPreInsert(eDir, oRect, refTiles);
}
void PressEvent::boardPostScroll(Direction::VALUE eDir) noexcept
{
	NRect oRect;
	oRect.m_nW = m_nBoardW;
	oRect.m_nH = m_nBoardH;
	boardPostInsert(eDir, oRect);
}
void PressEvent::boardPreInsert(Direction::VALUE eDir, NRect /*oRect*/, const shared_ptr<TileRect>& refTiles) noexcept
{
	if (eDir == Direction::DOWN) {
		if (refTiles && !TileRect::isAllEmptyTiles(*refTiles)) {
			level().gameStatusTechnical(std::vector<std::string>{"PressEvent::boardPreInsert()","DOWN only supports empty tiles insertion"});
			return;
		}
	}
}
void PressEvent::boardPostInsert(Direction::VALUE eDir, NRect oRect) noexcept
{
	if (!((eDir == Direction::UP) || (eDir == Direction::DOWN))) {
		level().gameStatusTechnical(std::vector<std::string>{"PressEvent::boardPostInsert()","Only DOWN and UP supported"});
		return;
	}
	if (oRect.m_nY != 0) {
		level().gameStatusTechnical(std::vector<std::string>{"PressEvent::boardPostInsert()","Only nY=0 supported"});
		return;
	}
	if (eDir == Direction::DOWN) {
		boardPostDeleteDown(oRect.m_nY + oRect.m_nH - 1, oRect.m_nX, oRect.m_nW);
	} else{
		boardPostInsertUp(oRect.m_nY + oRect.m_nH - 1, oRect.m_nX, oRect.m_nW);
	}
}
void PressEvent::boardPostDeleteDown(int32_t nDelY, int32_t nDelX, int32_t nDelW) noexcept
{
	if (m_nPressAXY == s_nInvalidXY) {
		return; //--------------------------------------------------------------
	}
	const NPoint oPressedXY = Util::unpackPointFromInt32(m_nPressAXY);
	const int32_t nPressedX = oPressedXY.m_nX;
	if ((nPressedX < nDelX) || (nPressedX >= nDelX + nDelW)) {
		return; //--------------------------------------------------------------
	}
	Level& oLevel = level();
	const int32_t nPressedY = oPressedXY.m_nY;
	if (nPressedY > nDelY) {
		return; //--------------------------------------------------------------
	}
	int32_t nAniH;
	if (nPressedY == nDelY) {
		// The TileAnimator was removed
		nAniH = 1;
	} else {
		// The TileAnimator was moved down, remove
		assert(oLevel.boardGetTileAnimator(nPressedX, nPressedY + 1, m_oData.m_nAniNameIdx) != nullptr);
		oLevel.boardSetTileAnimator(nPressedX, nPressedY + 1, m_oData.m_nAniNameIdx, nullptr, 0);
		nAniH = 2;
	}
	// set again
	assert(oLevel.boardGetTileAnimator(nPressedX, nPressedY, m_oData.m_nAniNameIdx) == nullptr);
	oLevel.boardSetTileAnimator(nPressedX, nPressedY, m_oData.m_nAniNameIdx, this, 0);
	// redraw tile(s)
	NRect oRect;
	oRect.m_nX = nPressedX;
	oRect.m_nY = nPressedY;
	oRect.m_nW = 1;
	oRect.m_nH = nAniH;
	oLevel.boardAnimateTiles(oRect);
}
void PressEvent::boardPostInsertUp(int32_t nInsY, int32_t nInsX, int32_t nInsW) noexcept
{
	if (m_nPressAXY == s_nInvalidXY) {
		return; //--------------------------------------------------------------
	}
	Level& oLevel = level();
	const NPoint oPressedXY = Util::unpackPointFromInt32(m_nPressAXY);
	const int32_t nPressedX = oPressedXY.m_nX;
	if ((nPressedX < nInsX) || (nPressedX >= nInsX + nInsW)) {
		return; //--------------------------------------------------------------
	}
	const int32_t nPressedY = oPressedXY.m_nY;
	if (nPressedY > nInsY) {
		return; //--------------------------------------------------------------
	}
//std::cout << "PressEvent::boardPostInsertUp  nPressedX=" << nPressedX << "  nPressedY=" << nPressedY << '\n';
	int32_t nAniH;
	if (nPressedY == 0) {
		// The TileAnimator was removed
		nAniH = 1;
	} else {
		// The TileAnimator was moved up, remove
		assert(oLevel.boardGetTileAnimator(nPressedX, nPressedY - 1, m_oData.m_nAniNameIdx) != nullptr);
		oLevel.boardSetTileAnimator(nPressedX, nPressedY - 1, m_oData.m_nAniNameIdx, nullptr, 0);
		nAniH = 2;
//std::cout << "PressEvent::boardPostInsertUp  was set nPressedX=" << nPressedX << "  nPressedY - 1=" << nPressedY - 1 << '\n';
	}
	// set again
	assert(oLevel.boardGetTileAnimator(nPressedX, nPressedY, m_oData.m_nAniNameIdx) == nullptr);
	oLevel.boardSetTileAnimator(nPressedX, nPressedY, m_oData.m_nAniNameIdx, this, 0);
	// redraw tile(s)
	NRect oRect;
	oRect.m_nX = nPressedX;
	oRect.m_nY = nPressedY + 1 - nAniH;
	oRect.m_nW = 1;
	oRect.m_nH = nAniH;
	oLevel.boardAnimateTiles(oRect);
}
void PressEvent::boabloPreFreeze(LevelBlock& /*oLevelBlock*/) noexcept
{
}
void PressEvent::boabloPostFreeze(const Coords& oCoords) noexcept
{
	boardPostModify(oCoords);
}
void PressEvent::boabloPreUnfreeze(const Coords& /*oCoords*/) noexcept
{
}
void PressEvent::boabloPostUnfreeze(LevelBlock& oLevelBlock) noexcept
{
	const Coords oCoords = LevelBlock::getCoords(oLevelBlock);
	boabloPostFreeze(oCoords);
}
void PressEvent::boardPreDestroy(const Coords& /*oCoords*/) noexcept
{
}
void PressEvent::boardPostDestroy(const Coords& oCoords) noexcept
{
	boabloPostFreeze(oCoords);
}
void PressEvent::boardPreModify(const TileCoords& /*oTileCoords*/) noexcept
{
	//if ((&oTileCoords) != (&m_oTileCoords)) {
	//	removeCoordsTileAnis(oTileCoords);
	//}
}
void PressEvent::boardPostModify(const Coords& /*oCoords*/) noexcept
{
}
double PressEvent::getElapsed01(int32_t /*nHash*/, int32_t /*nX*/, int32_t /*nY*/, int32_t
								#ifndef NDEBUG
								nAni
								#endif //NDEBUG
								, int32_t /*nViewTick*/, int32_t /*nTotViewTicks*/) const noexcept
{
	assert(nAni == m_oData.m_nAniNameIdx);
	return 1.0;
}
double PressEvent::getElapsed01(int32_t /*nHash*/, const LevelBlock& /*oLevelBlock*/, int32_t /*nBrickIdx*/, int32_t /*nAni*/
								, int32_t /*nViewTick*/, int32_t /*nTotTicks*/) const noexcept
{
	assert(false);
	return -1.0;
}

} // namespace stmg
