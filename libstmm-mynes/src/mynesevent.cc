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
 * File:   mynesevent.cc
 */

#include "mynesevent.h"

#include <stmm-games/level.h>
#include <stmm-games/named.h>
#include <stmm-games/util/namedindex.h>
#include <stmm-games/animations/staticgridanimation.h>
#include <stmm-games/util/util.h>
#include <stmm-games/util/helpers.h>
#include <stmm-games/util/namedindex.h>
#include <stmm-games/levelblock.h>
#include <stmm-games/tile.h>

#include <cassert>
#include <iostream>


namespace stmg
{

const int32_t MynesEvent::s_nMinCoveredW = 2;
const int32_t MynesEvent::s_nMinCoveredH = 2;

const int32_t MynesEvent::s_nZObjectZUncoverArea = 100;
const int32_t MynesEvent::s_nZObjectZMineExplosion = 9000;

static const std::string s_sMynesEventCharMineName = "MINE";
static const std::string s_sMynesEventCharMineBangName = "MINE-BANG";
static const std::string s_sMynesEventCharFlagWrongName = "FLAG-WRONG";

static const std::string s_sMynesEventFontFlagNoneName = "MYNES-FLAG-NONE";
static const std::string s_sMynesEventFontFlagSetName = "MYNES-FLAG";
static const std::string s_sMynesEventFontFlagMaybeName = "MYNES-FLAG-MAYBE";

static const std::string s_sMynesEventTileAniCoverName = "TILEANI:COVER";

Recycler<MynesEvent::PrivateExplosionAnimation, ExplosionAnimation> MynesEvent::s_oBoardExplosionRecycler{};

MynesEvent::MynesEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_oData(std::move(oInit))
{
	commonInit();
}
void MynesEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}
int32_t MynesEvent::getNamedIndex(NamedIndex& oNamedIndex, const std::string& sName, const std::string&
									#ifndef NDEBUG
									sDescType
									#endif //NDEBUG
									) noexcept
{
	#ifndef NDEBUG
	int32_t nIndex = oNamedIndex.getIndex(sName);
	if (nIndex < 0) {
		std::cout << "Warning! MynesEvent: " << sDescType << " '" << sName << "' not defined!" << '\n';
		nIndex = oNamedIndex.addName(sName);
	}
	#else //NDEBUG
	const int32_t nIndex = oNamedIndex.addName(sName);
	#endif //NDEBUG
	return nIndex;
}
void MynesEvent::commonInit() noexcept
{
	m_nBoardW = level().boardWidth();
	m_nBoardH = level().boardHeight();

	assert((m_oData.m_oPlayingArea.m_nX >= 0) && (m_oData.m_oPlayingArea.m_nX < m_nBoardW));
	assert((m_oData.m_oPlayingArea.m_nY >= 0) && (m_oData.m_oPlayingArea.m_nY < m_nBoardH));
	if (m_oData.m_oPlayingArea.m_nW <= 0) {
		m_oData.m_oPlayingArea.m_nW = m_nBoardW - m_oData.m_oPlayingArea.m_nX;
	}
	if (m_oData.m_oPlayingArea.m_nH <= 0) {
		m_oData.m_oPlayingArea.m_nH = m_nBoardH - m_oData.m_oPlayingArea.m_nY;
	}

	assert((m_oData.m_oCoverArea.m_nW > 0) && (m_oData.m_oCoverArea.m_nH > 0));

	assert((m_oData.m_oCoverArea.m_nX >= m_oData.m_oPlayingArea.m_nX) && (m_oData.m_oCoverArea.m_nX < m_oData.m_oPlayingArea.m_nX + m_oData.m_oPlayingArea.m_nW));
	assert((m_oData.m_oCoverArea.m_nY >= m_oData.m_oPlayingArea.m_nY) && (m_oData.m_oCoverArea.m_nY < m_oData.m_oPlayingArea.m_nY + m_oData.m_oPlayingArea.m_nH));
	assert(m_oData.m_oCoverArea.m_nX + m_oData.m_oCoverArea.m_nW <= m_oData.m_oPlayingArea.m_nX + m_oData.m_oPlayingArea.m_nW);
	assert(m_oData.m_oCoverArea.m_nY + m_oData.m_oCoverArea.m_nH <= m_oData.m_oPlayingArea.m_nY + m_oData.m_oPlayingArea.m_nH);

	if ((m_oData.m_oLockedArea.m_nW <= 0) || (m_oData.m_oLockedArea.m_nH <= 0)) {
		m_oData.m_oLockedArea.m_nW = 0;
		m_oData.m_oLockedArea.m_nH = 0;
	} else {
		m_oData.m_oLockedArea = NRect::intersectionRect(m_oData.m_oLockedArea, m_oData.m_oCoverArea);
	}
	m_nToX = m_oData.m_oPlayingArea.m_nX + m_oData.m_oPlayingArea.m_nW - 1;

	const bool bHasUncoverCells = ! ((m_oData.m_oCoverArea.m_nW == m_oData.m_oPlayingArea.m_nW) && (m_oData.m_oCoverArea.m_nH == m_oData.m_oPlayingArea.m_nH));
	if (bHasUncoverCells) {
		const int32_t nTopH = m_oData.m_oCoverArea.m_nY - m_oData.m_oPlayingArea.m_nY;
		if (nTopH > 0) {
			m_aUncoverAreas.push_back(NRect{m_oData.m_oPlayingArea.m_nX, m_oData.m_oPlayingArea.m_nY, m_oData.m_oPlayingArea.m_nW, nTopH});
		}
		const int32_t nBottomH = (m_oData.m_oPlayingArea.m_nY + m_oData.m_oPlayingArea.m_nH) - (m_oData.m_oCoverArea.m_nY + m_oData.m_oCoverArea.m_nH);
		if (nBottomH > 0) {
			m_aUncoverAreas.push_back(NRect{m_oData.m_oPlayingArea.m_nX, m_oData.m_oCoverArea.m_nY + m_oData.m_oCoverArea.m_nH, m_oData.m_oPlayingArea.m_nW, nBottomH});
		}
		const int32_t nLeftW = m_oData.m_oCoverArea.m_nX - m_oData.m_oPlayingArea.m_nX;
		if (nLeftW > 0) {
			m_aUncoverAreas.push_back(NRect{m_oData.m_oPlayingArea.m_nX, m_oData.m_oCoverArea.m_nY, nLeftW, m_oData.m_oCoverArea.m_nH});
		}
		const int32_t nRightW = (m_oData.m_oPlayingArea.m_nX + m_oData.m_oPlayingArea.m_nW) - (m_oData.m_oCoverArea.m_nX + m_oData.m_oCoverArea.m_nW);
		if (nRightW > 0) {
			m_aUncoverAreas.push_back(NRect{m_oData.m_oCoverArea.m_nX + m_oData.m_oCoverArea.m_nW, m_oData.m_oCoverArea.m_nY, nRightW, m_oData.m_oCoverArea.m_nH});
		}
	}

	NamedIndex& oCharsIndex = level().getNamed().chars();
	m_nCharMineIndex = getNamedIndex(oCharsIndex, s_sMynesEventCharMineName, "char");
	m_nCharMineBangIndex = getNamedIndex(oCharsIndex,  s_sMynesEventCharMineBangName, "char");
	m_nCharFlagWrongIndex = getNamedIndex(oCharsIndex, s_sMynesEventCharFlagWrongName, "char");

	NamedIndex& oFontsIndex = level().getNamed().fonts();
	m_nFontFlagNoneIndex = oFontsIndex.addName(s_sMynesEventFontFlagNoneName);
	m_nFontFlagSetIndex = getNamedIndex(oFontsIndex, s_sMynesEventFontFlagSetName, "font");
	m_nFontFlagMaybeIndex = getNamedIndex(oFontsIndex, s_sMynesEventFontFlagMaybeName, "font");

	NamedIndex& oTileAnisIndex = level().getNamed().tileAnis();
	m_nTileAniCoverIndex = getNamedIndex(oTileAnisIndex, s_sMynesEventTileAniCoverName, "tile ani");

	m_eState = MYNES_STATE_ACTIVATE;
	m_nLockCount = 0;
	m_bInhibitCover = false;

	m_bDirty = false;
	m_oToCoverCoords.reInit();
	m_aToCoverRects.clear();
	m_oToUncoverCoords.reInit();
	m_aToUncoverRects.clear();
}

void MynesEvent::reActivateNow()
{
	const int32_t nTimer = level().game().gameElapsed();
	level().activateEvent(this, nTimer);
}
bool MynesEvent::hasUncoverCells() const noexcept
{
	return ! m_aUncoverAreas.empty();
}
void MynesEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
//std::cout << "MynesEvent::trigger nTick=" << level().game().gameElapsed() << "  triggering=" << reinterpret_cast<int64_t>(p0TriggeringEvent) << '\n';
	switch (m_eState)
	{
		case MYNES_STATE_ACTIVATE:
		{
//level().dump(true, true, false, true);
			m_eState = MYNES_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				reActivateNow();
				break;
			}
		} // fallthrough
		case MYNES_STATE_INIT:
		{
			m_eState = MYNES_STATE_RUN;
			level().boardAddListener(this);
			m_bDirty = true;
			m_aToCoverRects.push_back(m_oData.m_oCoverArea);
			numberArea(m_oData.m_oPlayingArea, false);
			if (hasUncoverCells()) {
				for (const NRect& oUncoverRect : m_aUncoverAreas) {
					m_aToUncoverRects.push_back(oUncoverRect);
				}
			}
//level().dump(true, true, false, true);
		} // fallthrough
		case MYNES_STATE_RUN:
		{
			if (p0TriggeringEvent == nullptr) {
				int32_t nCoveredMines = 0;
				while (m_bDirty) {
					m_bDirty = false;
					while (! m_oToCoverCoords.isEmpty()) {
						auto it = m_oToCoverCoords.begin();
						const int32_t nX = it.x();
						const int32_t nY = it.y();
						m_oToCoverCoords.remove(it);
						nCoveredMines += coverPosition(nX, nY);
						numberAreaAround(nX, nY);
//std::cout << "MynesEvent::trigger CC nCoveredMines=" << nCoveredMines << " nX=" << nX << " nY=" << nY << '\n';
					}
					while (! m_aToCoverRects.empty()) {
						NRect oRect = m_aToCoverRects.back();
						m_aToCoverRects.pop_back();
						nCoveredMines += coverArea(oRect);
						numberArea(oRect, true);
					}
					while (! m_oToUncoverCoords.isEmpty()) {
						auto it = m_oToUncoverCoords.begin();
						int32_t nX = it.x();
						bool bNeighbours = false;
						if (nX < 0) {
							nX = - (nX + 1);
							bNeighbours = true;
						}
						const int32_t nY = it.y();
						m_oToUncoverCoords.remove(it);
						nCoveredMines -= uncoverPosition(nX, nY, bNeighbours);
					}
					while (! m_aToUncoverRects.empty()) {
						NRect oRect = m_aToUncoverRects.back();
						m_aToUncoverRects.pop_back();
						nCoveredMines -= uncoverArea(oRect);
//std::cout << "MynesEvent::trigger UR nCoveredMines=" << nCoveredMines << " nX=" << oRect.m_nX << " nY=" << oRect.m_nY << " w=" << oRect.m_nW << " h=" << oRect.m_nH << '\n';
					}
				}
				if (nCoveredMines != 0) {
					informListeners(LISTENER_GROUP_COVERED_MINES_ADDED, nCoveredMines);
				}
				return; //------------------------------------------------------
			}
			switch (nMsg) {
				case MESSAGE_UNCOVER: {
					const NPoint oNewXY = Util::unpackPointFromInt32(nValue);
//std::cout << "MynesEvent::trigger MESSAGE_UNCOVER oNewXY x=" << oNewXY.m_nX << " y=" << oNewXY.m_nY << '\n';
					if (isInCoverArea(oNewXY.m_nX, oNewXY.m_nY)) {
						m_oToUncoverCoords.add(- (oNewXY.m_nX + 1), oNewXY.m_nY);
						m_bDirty = true;
						reActivateNow();
					}
				} break;
				case MESSAGE_TOGGLE_FLAG: {
					const NPoint oNewXY = Util::unpackPointFromInt32(nValue);
					if (isInCoverArea(oNewXY.m_nX, oNewXY.m_nY)) {
						toggleFlag(oNewXY.m_nX, oNewXY.m_nY);
					}
				} break;
				case MESSAGE_FINISH: {
					m_eState = MYNES_STATE_FINISHED;
					informListeners(LISTENER_GROUP_FINISHED, 0);
				} break;
				case MESSAGE_FINISH_UNCOVER: {
					m_eState = MYNES_STATE_FINISHING;
					reActivateNow();
				} break;
				case MESSAGE_FINISH_SHOW_MINES: {
					m_eState = MYNES_STATE_FINISHED;
					showMines();
					informListeners(LISTENER_GROUP_FINISHED, 0);
				} break;
			}
			break; //switch
		}
		break;
		case MYNES_STATE_FINISHING:
		{
			if (p0TriggeringEvent != nullptr) {
				reActivateNow();
				return; //------------------------------------------------------
			}
			m_eState = MYNES_STATE_FINISHED;
			uncoverArea(m_oData.m_oCoverArea);
			informListeners(LISTENER_GROUP_FINISHED, 0);
		} // fallthrough
		case MYNES_STATE_FINISHED:
		{
		}
		break;
	}
}
void MynesEvent::toggleFlag(int32_t nX, int32_t nY) noexcept
{
	Tile oTile = level().boardGetTile(nX, nY);
	TileFont& oFont = oTile.getTileFont();
	int32_t nFontIndex;
	if (!isCovered(oFont, nFontIndex)) {
		return; //--------------------------------------------------------------
	}
	int32_t nGroup = -1;
	if (nFontIndex == m_nFontFlagNoneIndex) {
		nFontIndex = m_nFontFlagSetIndex;
		nGroup = LISTENER_GROUP_ADD_FLAG;
	} else if (nFontIndex == m_nFontFlagSetIndex) {
		nFontIndex = m_nFontFlagMaybeIndex;
		nGroup = LISTENER_GROUP_REMOVE_FLAG;
	} else if (nFontIndex == m_nFontFlagMaybeIndex) {
		nFontIndex = m_nFontFlagNoneIndex;
	} else {
		return; //--------------------------------------------------------------
	}
	oFont.setFontIndex(nFontIndex);
	lockedSetTile(nX, nY, oTile);
	if (nGroup >= 0) {
		const int32_t nValue = Util::packPointToInt32(NPoint{nX, nY});
		informListeners(nGroup, nValue);
	}
}
bool MynesEvent::isInPlayingArea(int32_t nX, int32_t nY) const noexcept
{
	return m_oData.m_oPlayingArea.containsPoint(NPoint{nX, nY});
}
bool MynesEvent::isInPlayingAreaX(int32_t nX) const noexcept
{
	return ((nX >= m_oData.m_oPlayingArea.m_nX) && (nX <= m_nToX));
}
bool MynesEvent::isInPlayingAreaY(int32_t nY) const noexcept
{
	return ((nY >= 0) && (nY < m_nBoardH));
}
bool MynesEvent::isInCoverArea(int32_t nX, int32_t nY) const noexcept
{
	return m_oData.m_oCoverArea.containsPoint(NPoint{nX, nY});
}
bool MynesEvent::isCovered(int32_t nFontIdx) const noexcept
{
	return ((nFontIdx == m_nFontFlagNoneIndex) || (nFontIdx == m_nFontFlagSetIndex)
				|| (nFontIdx == m_nFontFlagMaybeIndex));
}
bool MynesEvent::isCovered(const TileFont& oFont, int32_t& nFontIdx) const noexcept
{
	const bool bFontDefined = ! oFont.isEmpty();
	if (bFontDefined) {
		nFontIdx = oFont.getFontIndex();
		if (isCovered(nFontIdx)) {
			return true;
		}
	}
	nFontIdx = -1;
	return false;
}
bool MynesEvent::isCovered(const TileFont& oFont) const noexcept
{
	int32_t nFontIdx;
	return isCovered(oFont, nFontIdx);
}
bool MynesEvent::isMine(const Tile& oTile) const noexcept
{
	const TileChar& oChar = oTile.getTileChar();
	const TileFont& oFont = oTile.getTileFont();
	const bool bFontDefined = ! oFont.isEmpty();
	if (bFontDefined) {
		const int32_t nFontIdx = oFont.getFontIndex();
		if (!isCovered(nFontIdx)) {
			return false; //----------------------------------------------------
		}
	}
	const bool bCharDefined = !oChar.isEmpty();
	if (bCharDefined) {
		if (oChar.isCharIndex()) {
			const int32_t nCharIdx = oChar.getCharIndex();
			if (nCharIdx == m_nCharMineIndex) {
				return true; //-------------------------------------------------
			}
		}
	}
	return false;
}
int32_t MynesEvent::coverPosition(int32_t nX, int32_t nY) noexcept
{
	assert(!m_bInhibitCover);
	assert(isInCoverArea(nX, nY));
	Level& oLevel = level();
	Tile oTile = oLevel.boardGetTile(nX, nY);
	TileChar& oChar = oTile.getTileChar();
	TileFont& oFont = oTile.getTileFont();
	bool bDontCover = false;
	bool bForceToMynes = false;
		bool bCharToEmpty = false;
		bool bCharToMine = false;
	const bool bFontDefined = ! oFont.isEmpty();
	if (bFontDefined) {
		const int32_t nFontIdx = oFont.getFontIndex();
		if (isCovered(nFontIdx)) {
			bForceToMynes = true;
//std::cout << "MynesEvent::coverPosition  nX=" << nX << " nY=" << nY << "  ??? 1" << '\n';
		} else {
			bDontCover = true;
		}
	}
	int32_t nCovered = 0;
	const bool bCharDefined = !oChar.isEmpty();
	if (bForceToMynes) {
		// already covered, change char to mine or empty if necessary
		if (bCharDefined) {
			if (oChar.isCharIndex()) {
				const int32_t nCharIdx = oChar.getCharIndex();
				if ((nCharIdx == m_nCharMineBangIndex) || (nCharIdx == m_nCharFlagWrongIndex)) {
					bCharToEmpty = true;
				} else if (nCharIdx == m_nCharMineIndex) {
					// leave unchanged
				} else {
					// any other char index is transformed to a mine
					bCharToMine = true;
				}
			} else {
				const uint32_t nChar = oChar.getChar();
				if ((nChar >= '0') && (nChar <= '8')) {
					// transform to empty, numbering will be done later
					bCharToEmpty = true;
				} else {
					// any other char becomes a mine
					bCharToMine = true;
				}
			}
		} else {
			bCharToEmpty = true;
		}
	} else if (! bDontCover) {
		// cover, but only if mine or empty
		if (bCharDefined) {
			if (oChar.isCharIndex()) {
				const int32_t nCharIdx = oChar.getCharIndex();
				if (nCharIdx == m_nCharMineIndex) {
//std::cout << "MynesEvent::coverPosition  nX=" << nX << " nY=" << nY << "  MINE" << '\n';
					bForceToMynes = true;
				} else {
					bDontCover = true;
				}
			} else {
				const uint32_t nChar = oChar.getChar();
//std::cout << "MynesEvent::coverPosition  nX=" << nX << " nY=" << nY << "  nChar=" << nChar << '\n';
				if ((nChar >= '0') && (nChar <= '8')) {
					bForceToMynes = true;
					bCharToEmpty = true;
				} else {
					bDontCover = true;
				}
			}
		} else {
			bForceToMynes = true;
			bCharToEmpty = true;
//std::cout << "MynesEvent::coverPosition  nX=" << nX << " nY=" << nY << "  ??? 2" << '\n';
//oTile.dump();
		}
	}
	assert(bForceToMynes != bDontCover);
	if (bForceToMynes) {
//std::cout << "MynesEvent::coverPosition  nX=" << nX << " nY=" << nY << "  ??? 3" << '\n';
		if (bCharToEmpty) {
			oChar.setChar('0');
		} else if (bCharToMine) {
			oChar.setCharIndex(m_nCharMineIndex);
			++nCovered;
		} else {
			assert((!oChar.isEmpty()) && oChar.isCharIndex() && (static_cast<int32_t>(oChar.getCharIndex()) == m_nCharMineIndex));
			++nCovered;
		}
		oFont.setFontIndex(m_nFontFlagNoneIndex);
		// TileColor and TileAlpha untouched!
		lockedSetTile(nX, nY, oTile);
		if (oLevel.boardGetTileAnimator(nX, nY, m_nTileAniCoverIndex) != nullptr) {
			if (oLevel.boardGetTileAnimator(nX, nY, m_nTileAniCoverIndex) != this) {
				level().gameStatusTechnical(std::vector<std::string>{"MynesEvent doesn't allow other events to set", s_sMynesEventTileAniCoverName});
				return nCovered; //---------------------------------------------
			}
		}
		oLevel.boardSetTileAnimator(nX, nY, m_nTileAniCoverIndex, this, 0);
		oLevel.boardAnimateTile(NPoint{nX, nY});
	}
	return nCovered;
}
int32_t MynesEvent::coverCoords(const Coords& oCoords) noexcept
{
	if (m_bInhibitCover) {
		return 0;
	}
	int32_t nCoveredMines = 0;
	for (auto it = oCoords.begin(); it != oCoords.end(); it.next()) {
		const int32_t nX = it.x();
		const int32_t nY = it.y();
		if (! isInCoverArea(nX, nY)) {
			continue; // for nY
		}
		nCoveredMines += coverPosition(nX, nY);
	}
	return nCoveredMines;
}
int32_t MynesEvent::coverArea(const NRect& oArea) noexcept
{
	return coverArea(oArea.m_nX, oArea.m_nY, oArea.m_nW, oArea.m_nH);
}
int32_t MynesEvent::coverArea(int32_t nAreaX, int32_t nAreaY, int32_t nAreaW, int32_t nAreaH) noexcept
{
	if (m_bInhibitCover) {
		return 0;
	}
//std::cout << "MynesEvent::coverArea  nAreaX=" << nAreaX << " nAreaY=" << nAreaY << " nAreaW=" << nAreaW << " nAreaH=" << nAreaH<< '\n';
			
	int32_t nCoveredMines = 0;
	for (int32_t nY = nAreaY; nY < nAreaY + nAreaH; ++nY) {
		for (int32_t nX = nAreaX; nX < nAreaX + nAreaW; ++nX) {
			if (! isInCoverArea(nX, nY)) {
				continue; // for nX
			}
			nCoveredMines += coverPosition(nX, nY);
		}
	}
	return nCoveredMines;
}
void MynesEvent::numberPosition(int32_t nX, int32_t nY) noexcept
{
	assert(m_bInhibitCover);
	assert(isInPlayingArea(nX, nY));
	Level& oLevel = level();
	Tile oTile = oLevel.boardGetTile(nX, nY);
	TileChar& oChar = oTile.getTileChar();
	TileFont& oFont = oTile.getTileFont();
	const bool bFontDefined = ! oFont.isEmpty();
	if (bFontDefined) {
		const int32_t nFontIdx = oFont.getFontIndex();
		if (!isCovered(nFontIdx)) {
			// a non mynes font, skip
			return; //----------------------------------------------------------
		}
	}
	const bool bCharDefined = !oChar.isEmpty();
	if (bCharDefined) {
		if (oChar.isCharIndex()) {
			return; //----------------------------------------------------------
		}
		const uint32_t nChar = oChar.getChar();
		if (!((nChar >= '0') && (nChar <= '8'))) {
			return; //----------------------------------------------------------
		}
	}
//std::cout << "    MynesEvent::numberPosition  nX=" << nX << " nY=" << nY;
	int32_t nAdjMines = 0;
	for (int32_t nDx = -1; nDx <= 1; ++ nDx) {
		for (int32_t nDy = -1; nDy <= 1; ++ nDy) {
			if (!((nDx == 0) && (nDy == 0))) {
				const int32_t nAdjX = nX + nDx;
				const int32_t nAdjY = nY + nDy;
				if (isInPlayingArea(nAdjX, nAdjY)) {
					const Tile& oAdjTile = oLevel.boardGetTile(nAdjX, nAdjY);
					if (isMine(oAdjTile)) {
						++nAdjMines;
					}
				}
			}
		}
	}
//std::cout << "   nAdjMines=" << nAdjMines << '\n';
	oChar.setChar(static_cast<int32_t>('0') + nAdjMines);
	lockedSetTile(nX, nY, oTile);
}
void MynesEvent::numberCoords(const Coords& oCoords, bool bAlsoNeighbours) noexcept
{
	m_bInhibitCover = true;
	for (auto it = oCoords.begin(); it != oCoords.end(); it.next()) {
		const int32_t nX = it.x();
		const int32_t nY = it.y();
		if (bAlsoNeighbours) {
			int32_t nCurX = nX - 1;
			int32_t nCurY = nY;
			if (isInPlayingAreaX(nCurX) && ! oCoords.contains(nCurX, nCurY)) {
				numberPosition(nCurX, nCurY);
			}
			nCurX = nX + 1;
			nCurY = nY;
			if (isInPlayingAreaX(nCurX) && ! oCoords.contains(nCurX, nCurY)) {
				numberPosition(nCurX, nCurY);
			}
			nCurX = nX;
			nCurY = nY - 1;
			if (isInPlayingAreaY(nCurY) && ! oCoords.contains(nCurX, nCurY)) {
				numberPosition(nCurX, nCurY);
			}
			nCurX = nX;
			nCurY = nY + 1;
			if (isInPlayingAreaY(nCurY) && ! oCoords.contains(nCurX, nCurY)) {
				numberPosition(nCurX, nCurY);
			}
		}
		numberPosition(nX, nY);
	}
	m_bInhibitCover = false;
}
void MynesEvent::numberAreaAround(int32_t nX, int32_t nY) noexcept
{
	numberArea(nX, nY, 1, 1, true);
}
void MynesEvent::numberArea(const NRect& oArea, bool bAlsoNeighbours) noexcept
{
	numberArea(oArea.m_nX, oArea.m_nY, oArea.m_nW, oArea.m_nH, bAlsoNeighbours);
}
void MynesEvent::numberArea(int32_t nAreaX, int32_t nAreaY, int32_t nAreaW, int32_t nAreaH, bool bAlsoNeighbours) noexcept
{
//std::cout << "MynesEvent::numberArea  nAreaX=" << nAreaX << " nAreaY=" << nAreaY << "  nAreaW=" << nAreaW << " nAreaH=" << nAreaH << '\n';
	m_bInhibitCover = true;
	if (bAlsoNeighbours) {
		--nAreaX;
		nAreaW += 2;
		--nAreaY;
		nAreaH += 2;
	}
	for (int32_t nX = nAreaX; nX < nAreaX + nAreaW; ++nX) {
		for (int32_t nY = nAreaY; nY < nAreaY + nAreaH; ++nY) {
			if (bAlsoNeighbours) {
				if (!isInPlayingArea(nX, nY)) {
					continue; // for nY
				}
			}
			numberPosition(nX, nY);
		}
	}
	m_bInhibitCover = false;
}

int32_t MynesEvent::uncoverArea(const NRect& oArea) noexcept
{
	return uncoverArea(oArea.m_nX, oArea.m_nY, oArea.m_nW, oArea.m_nH, false);
}
int32_t MynesEvent::uncoverArea(int32_t nAreaX, int32_t nAreaY, int32_t nAreaW, int32_t nAreaH) noexcept
{
	return uncoverArea(nAreaX, nAreaY, nAreaW, nAreaH, false);
}
int32_t MynesEvent::uncoverArea(int32_t nAreaX, int32_t nAreaY, int32_t nAreaW, int32_t nAreaH, bool bButtonPress) noexcept
{
//std::cout << "MynesEvent::uncoverArea  nAreaX=" << nAreaX << " nAreaY=" << nAreaY << "  nAreaW=" << nAreaW << " nAreaH=" << nAreaH << '\n';
	int32_t nUncoveredMines = 0;
	for (int32_t nX = nAreaX; nX < nAreaX + nAreaW; ++nX) {
		for (int32_t nY = nAreaY; nY < nAreaY + nAreaH; ++nY) {
			nUncoveredMines += uncoverPosition(nX, nY, bButtonPress);
		}
	}
	return nUncoveredMines;
}
int32_t MynesEvent::uncoverPosition(int32_t nX, int32_t nY, bool bButtonPress) noexcept
{
	return uncoverPosition(nX, nY, bButtonPress, false);
}
int32_t MynesEvent::uncoverPosition(int32_t nX, int32_t nY, bool bButtonPress, bool bRecursive) noexcept
{
//std::cout << "MynesEvent::uncoverPosition  nX=" << nX << " nY=" << nY << '\n';
	Level& oLevel = level();
	Tile oTile = oLevel.boardGetTile(nX, nY);
	TileChar& oChar = oTile.getTileChar();
	TileFont& oFont = oTile.getTileFont();
	int32_t nFontIndex = -1;
	if (! isCovered(oFont, nFontIndex)) {
//std::cout << "          ::uncoverPosition  nX=" << nX << " nY=" << nY << "   ALREADY UNCOVERED" << '\n';
		return 0; //------------------------------------------------------------
	}
	if (bRecursive && (nFontIndex != m_nFontFlagNoneIndex)) {
		// recursive uncovering stops at flag or at maybe
		return 0; //------------------------------------------------------------
	}
	const bool bFinished = (m_eState == MYNES_STATE_FINISHED);
	int32_t nUncoveredMines = 0;
	const bool bIsFlagged = (nFontIndex == m_nFontFlagSetIndex);
	// clear font for possible explosion
	oFont.clear();
	bool bUncoverNeighbours = false;
	bool bNumberNeighbours = false;
	assert(!oChar.isEmpty());
	const int32_t nPackedXY = Util::packPointToInt32(NPoint{nX, nY});
	if (oChar.isCharIndex()) {
		const int32_t nCharIdx = oChar.getCharIndex();
		if ((nCharIdx == m_nCharMineIndex)) {
			if (bButtonPress || (! bIsFlagged)) {
				auto& oGame = oLevel.game();
				ExplosionAnimation::Init oEAInit;
				oEAInit.m_nZ = s_nZObjectZMineExplosion;
				oEAInit.m_oPos = FPoint{- 0.5 + nX, - 0.5 + nY};
				oEAInit.m_oSize = FSize{2.0, 2.0};
				oEAInit.m_fDuration = oGame.random(800, 1200);
				oEAInit.m_oTile = oTile;
				oEAInit.m_nLevelPlayer = 0;
				shared_ptr<ExplosionAnimation> refBoardExplosion;
				s_oBoardExplosionRecycler.create(refBoardExplosion, std::move(oEAInit));
				oLevel.animationAddScrolled(refBoardExplosion, oGame.random(0, 200));
				oChar.setCharIndex(m_nCharMineBangIndex);
				bNumberNeighbours = true;
				if (bIsFlagged) {
					informListeners(LISTENER_GROUP_REMOVE_FLAG, nPackedXY);
				}
				informListeners(LISTENER_GROUP_UNCOVERED_UNFLAGGED_MINE, nPackedXY);
			} else {
				informListeners(LISTENER_GROUP_UNCOVERED_FLAGGED_MINE, nPackedXY);
			}
			++nUncoveredMines;
		} else if (bButtonPress && bIsFlagged) {
			// pressing button on a flagged non mine cell
			informListeners(LISTENER_GROUP_REMOVE_FLAG, nPackedXY);
		}
	} else {
		const uint32_t nChar = oChar.getChar();
		assert((nChar >= '0') && (nChar <= '8'));
		if (bButtonPress) {
			if (bIsFlagged) {
				// pressing button on a flagged non mine cell
				informListeners(LISTENER_GROUP_REMOVE_FLAG, nPackedXY);
			}
			if (nChar == '0') {
				bUncoverNeighbours = true;
			}
		} else if (bIsFlagged) {
			oChar.setCharIndex(m_nCharFlagWrongIndex);
			const int32_t nPackedXY = Util::packPointToInt32(NPoint{nX, nY});
			informListeners(LISTENER_GROUP_UNCOVERED_FLAGGED_NON_MINE, nPackedXY);
		} else if (bFinished) {
			// keep the cover
			return 0; //--------------------------------------------------------
		}
	}
	lockedSetTile(nX, nY, oTile);
	assert(oLevel.boardGetTileAnimator(nX, nY, m_nTileAniCoverIndex) == this);
	oLevel.boardSetTileAnimator(nX, nY, m_nTileAniCoverIndex, nullptr, 0);
	oLevel.boardAnimateTile(NPoint{nX, nY});
	if (bNumberNeighbours) {
		numberAreaAround(nX, nY);
	}
	if (bUncoverNeighbours) {
		for (int32_t nDx = -1; nDx <= 1; ++nDx) {
			for (int32_t nDy = -1; nDy <= 1; ++nDy) {
				if (!((nDx == 0) && (nDy == 0))) {
					const int32_t nAdjX = nX + nDx;
					const int32_t nAdjY = nY + nDy;
					if (m_oData.m_oCoverArea.containsPoint(NPoint{nAdjX, nAdjY})) {
						nUncoveredMines += uncoverPosition(nAdjX, nAdjY, true, true);
					}
				}
			}
		}
	}
	return nUncoveredMines;
}
void MynesEvent::showMines() noexcept
{
	Level& oLevel = level();
	for (int32_t nX = 0; nX < m_oData.m_oCoverArea.m_nW; ++ nX) {
		for (int32_t nY = 0; nY < m_oData.m_oCoverArea.m_nH; ++ nY) {
			Tile oTile = oLevel.boardGetTile(nX, nY);
			TileChar& oChar = oTile.getTileChar();
			if (oChar.isCharIndex()) {
				const int32_t nCharIdx = oChar.getCharIndex();
				if ((nCharIdx == m_nCharMineIndex)) {
					oLevel.boardSetTileAnimator(nX, nY, m_nTileAniCoverIndex, nullptr, 0);
				}
			}
		}
	}
	oLevel.boardAnimateTiles(m_oData.m_oCoverArea);
}
void MynesEvent::lockedSetTile(int32_t nX, int32_t nY, const Tile& oTile) noexcept
{
	assert(m_nLockCount == 0);
	++m_nLockCount;
	level().boardSetTile(nX, nY, oTile);
	--m_nLockCount;
}

void MynesEvent::boardPreScroll(Direction::VALUE /*eDir*/, const shared_ptr<TileRect>& /*refTiles*/) noexcept
{
}
void MynesEvent::boardPostScroll(Direction::VALUE eDir) noexcept
{
	if (m_eState == MYNES_STATE_FINISHED) {
		return;
	}
	if (m_bDirty) {
		level().gameStatusTechnical(std::vector<std::string>{"MynesEvent::boardPostScroll()","Nesting scrolling while modifying not allowed"});
		return;
	}
	const NRect oAddRect = Helpers::boardScrollAdd(NSize{m_nBoardW, m_nBoardH}, eDir);
	// Check whether some of the added tiles need to be covered
	const NRect oIntersectCoverRect = NRect::intersectionRect(oAddRect, m_oData.m_oCoverArea);
	if (oIntersectCoverRect.m_nW > 0) {
		m_aToCoverRects.push_back(oIntersectCoverRect);
		m_bDirty = true;
	}
	if (hasUncoverCells()) {
		NRect oMovedCovered = m_oData.m_oCoverArea;
		oMovedCovered.m_nX += Direction::deltaX(eDir);
		oMovedCovered.m_nY += Direction::deltaY(eDir);
		for (const NRect& oUncoverRect : m_aUncoverAreas) {
			// Check whether moved tiles that were covered are now uncovered
			const NRect oIntersectMovedRect = NRect::intersectionRect(oMovedCovered, oUncoverRect);
			if (oIntersectMovedRect.m_nW > 0) {
				m_aToUncoverRects.push_back(oIntersectMovedRect);
				m_bDirty = true;
			}
			// Check whether some of the added tiles are uncovered
			const NRect oIntersectAddRect = NRect::intersectionRect(oMovedCovered, oUncoverRect);
			if (oIntersectAddRect.m_nW > 0) {
				m_aToUncoverRects.push_back(oIntersectAddRect);
				m_bDirty = true;
			}
		}
	}
	if (m_bDirty) {
		reActivateNow();
	}
}
void MynesEvent::boardPreInsert(Direction::VALUE /*eDir*/, NRect /*oRect*/, const shared_ptr<TileRect>& /*refTiles*/) noexcept
{
}
void MynesEvent::boardPostInsert(Direction::VALUE eDir, NRect oRect) noexcept
{
	if (m_eState == MYNES_STATE_FINISHED) {
		return;
	}
	if (m_bDirty) {
		level().gameStatusTechnical(std::vector<std::string>{"MynesEvent::boardPostInsert()","Nesting insert while modifying not allowed"});
		return;
	}
	const NRect oAddRect = Helpers::boardInsertAdd(oRect, eDir);
	// Check whether some of the added tiles need to be covered
	const NRect oIntersectCoverRect = NRect::intersectionRect(oAddRect, m_oData.m_oCoverArea);
	if (oIntersectCoverRect.m_nW > 0) {
		m_aToCoverRects.push_back(std::move(oIntersectCoverRect));
		m_bDirty = true;
	}
	if (hasUncoverCells()) {
		NRect oMovedCovered = m_oData.m_oCoverArea;
		oMovedCovered.m_nX += Direction::deltaX(eDir);
		oMovedCovered.m_nY += Direction::deltaY(eDir);
		//
		oMovedCovered = NRect::intersectionRect(oMovedCovered, oRect);
		//
		for (const NRect& oUncoverRect : m_aUncoverAreas) {
			// Check whether moved tiles that were covered are now uncovered
			const NRect oIntersectMovedRect = NRect::intersectionRect(oMovedCovered, oUncoverRect);
			if (oIntersectMovedRect.m_nW > 0) {
				m_aToUncoverRects.push_back(oIntersectMovedRect);
				m_bDirty = true;
			}
			// Check whether some of the added tiles are uncovered
			const NRect oIntersectAddRect = NRect::intersectionRect(oMovedCovered, oUncoverRect);
			if (oIntersectAddRect.m_nW > 0) {
				m_aToUncoverRects.push_back(oIntersectAddRect);
				m_bDirty = true;
			}
		}
	}
	if (m_bDirty) {
		reActivateNow();
	}
}
void MynesEvent::boabloPreFreeze(LevelBlock& /*oLevelBlock*/) noexcept
{
}
void MynesEvent::boabloPostFreeze(const Coords& oCoords) noexcept
{
	boardPostModify(oCoords);
}
void MynesEvent::boabloPreUnfreeze(const Coords& /*oCoords*/) noexcept
{
}
void MynesEvent::boabloPostUnfreeze(LevelBlock& oLevelBlock) noexcept
{
	const Coords oCoords = LevelBlock::getCoords(oLevelBlock);
	boardPostModify(oCoords);
}
void MynesEvent::boardPreDestroy(const Coords& /*oCoords*/) noexcept
{
}
void MynesEvent::boardPostDestroy(const Coords& oCoords) noexcept
{
	boardPostModify(oCoords);
}
void MynesEvent::boardPreModify(const TileCoords& /*oTileCoords*/) noexcept
{
}
void MynesEvent::boardPostModify(const Coords& oCoords) noexcept
{
	if (m_eState == MYNES_STATE_FINISHED) {
		return;
	}
	if (m_nLockCount > 0) {
		if (m_nLockCount > 1) {
			level().gameStatusTechnical(std::vector<std::string>{"MynesEvent doesn't allow nested modifications"});
		}
		return; //--------------------------------------------------------------
	}
	for (auto it = oCoords.begin(); it != oCoords.end(); it.next()) {
		const NPoint oXY = it.point();
		if (m_oData.m_oCoverArea.containsPoint(oXY)) {
			m_oToCoverCoords.add(oXY);
			m_bDirty = true;
		} else if (m_oData.m_oPlayingArea.containsPoint(oXY)) {
			// it's within one of the uncover areas
			m_oToUncoverCoords.add(oXY);
			m_bDirty = true;
		}
	}
	if (m_bDirty) {
		reActivateNow();
	}
}
double MynesEvent::getElapsed01(int32_t /*nHash*/, int32_t /*nX*/, int32_t /*nY*/, int32_t /*nAni*/
								, int32_t /*nViewTick*/, int32_t /*nTotViewTicks*/) const noexcept
{
	//Tile& oTile = level().boardGetTile(nX, nY);
	return 1.0;
}
double MynesEvent::getElapsed01(int32_t /*nHash*/, const LevelBlock& /*oLevelBlock*/, int32_t /*nBrickIdx*/, int32_t /*nAni*/
								, int32_t /*nViewTick*/, int32_t /*nTotTicks*/) const noexcept
{
	assert(false);
	return -1.0;
}

} // namespace stmg
