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
 * File:   fillerevent.cc
 */

#include "fillerevent.h"

#include <stmm-games/gameproxy.h>
#include <stmm-games/level.h>
#include <stmm-games/tile.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/util.h>

#include <cassert>
//#include <iostream>

namespace stmg
{

FillerEvent::FillerEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
{
	commonInit(std::move(oInit));
}

void FillerEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	commonInit(std::move(oInit));
}
void FillerEvent::commonInit(LocalInit&& oInit) noexcept
{
	Level& oLevel = level();
	m_nBoardW = oLevel.boardWidth();
	m_nBoardH = oLevel.boardHeight();

	m_oTileBufferRecycler.create(m_refCurTileBuf, NSize{m_nBoardW, 1});

	m_eFillMode = oInit.m_oFillGen.m_eFillMode;
	assert((m_eFillMode >= FILL_MODE_REPLACE) && (m_eFillMode <= FILL_MODE_IGNORE));

	if (m_eFillMode != FILL_MODE_IGNORE) {
		m_refRandomTiles = std::make_unique<RandomTiles>(oLevel.game().getRandomSource(), std::move(oInit.m_oFillGen.m_oProbTileGen));
	}

	m_nTotTiles = oInit.m_oFillGen.m_nTotTiles;

	m_fFactorOfAvailablePositions = oInit.m_oFillGen.m_fFactorOfAvailablePositions;
	assert((m_fFactorOfAvailablePositions >= 0.0) && (m_fFactorOfAvailablePositions <= 1.0));

	initAllowedPositions(oInit.m_oCoords);

	m_refTargetTiles = std::move(oInit.m_refTargetTiles);

	resetRuntime();
}
void FillerEvent::initAllowedPositions(const Coords& oCoords) noexcept
{
	if (oCoords.isEmpty()) {
		for (int32_t nY = 0; nY < m_nBoardH; ++nY) {
			for (int32_t nX = 0; nX < m_nBoardW; ++nX) {
				m_aAllowedPositions.push_back(NPoint{nX, nY});
			}
		}
	} else {
		for (auto itXY = oCoords.begin(); itXY != oCoords.end(); itXY.next()) {
			m_aAllowedPositions.push_back(itXY.point());
		}
	}
}
void FillerEvent::resetRuntime() noexcept
{
	m_eState = FILLER_STATE_ACTIVATE;
	// default
	m_nFillType = MESSAGE_FILL;
	m_nFillParam = 0;
}

void FillerEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
	Level& oLevel = level();
	const int32_t nTimer = oLevel.game().gameElapsed();
//std::cout << "FillerEvent::trigger nTimer=" << nTimer << " tiggering=" << reinterpret_cast<int64_t>(p0TriggeringEvent) << " p=" << getPriority() << '\n';
	switch (m_eState)
	{
		case FILLER_STATE_ACTIVATE:
		{
			m_eState = FILLER_STATE_INIT;
		} //fallthrough
		case FILLER_STATE_INIT:
		{
			m_eState = FILLER_STATE_RUN;
		} //fallthrough
		case FILLER_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				switch (nMsg)
				{
					case MESSAGE_FILL:
					{
					} //fallthrough
					case MESSAGE_FILL_JUST_POS:
					{
					} //fallthrough
					case MESSAGE_FILL_BUT_POS:
					{
						m_nFillType = nMsg;
						m_nFillParam = nValue;
						oLevel.activateEvent(this, nTimer);
					}
					break;
				}
				return; //------------------------------------------------------
			}
			m_oTileCoords.reInit();
			const int32_t nMaxToFill = [&]() {
				if (m_nFillType == MESSAGE_FILL) {
					return m_nFillParam;
				} else if (m_nFillType == MESSAGE_FILL_JUST_POS) {
					return 1;
				} else {
					return 0;
				}
			}();
			const bool bNot = (m_nFillType == MESSAGE_FILL_BUT_POS);
			const NPoint oXY = ((m_nFillType != MESSAGE_FILL) ? Util::unpackPointFromInt32(m_nFillParam) : NPoint{-1, -1});
			const int32_t nTotFilled = fillSelectedPositions(nMaxToFill, bNot, oXY);
//std::cout << "FillerEvent::trigger 4 nTotFilled=" << nTotFilled << '\n';
			if (nTotFilled <= 0) {
				return; //------------------------------------------------------
			}
			oLevel.boardModify(m_oTileCoords);
//#ifndef NDEBUG
//std::cout << "Level:" << '\n';
//oLevel.dump(true, false, false, false, false, false);
//#endif //NDEBUG
			informListeners(LISTENER_GROUP_FILLED, nTotFilled);
		}
		break;
	}
}
int32_t FillerEvent::fillSelectedPositions(int32_t nMaxToFill, bool bNot, const NPoint& oTheXY) noexcept
{
	std::vector<NPoint> aPositions;

	const bool bSelectEmptyTiles = (m_refTargetTiles.get() == nullptr);
	const bool bOne = (oTheXY.m_nX >= 0);
	const bool bJustOne = bOne && ! bNot;
//std::cout << "FillerEvent::fillSelectedPositions  nMaxToFill=" << nMaxToFill << "  nX=" << oTheXY.m_nX << "  nY=" << oTheXY.m_nY << '\n';
//std::cout << "FillerEvent::fillSelectedPositions  bSelectEmptyTiles=" << bSelectEmptyTiles << "  bOne=" << bOne << "  bJustOne=" << bJustOne << '\n';

	Level& oLevel = level();
//#ifndef NDEBUG
//std::cout << "Level:" << '\n';
//oLevel.dump(true, false, false, false, false, false);
//#endif //NDEBUG
	auto& oGame = oLevel.game();
	for (const NPoint& oXY : m_aAllowedPositions) {
//std::cout << "FillerEvent::fillSelectedPositions  nX=" << oXY.m_nX << "  nY=" << oXY.m_nY << '\n';
		if (oXY == oTheXY) {
			if (bNot) {
				// skip the one excluded
				continue; //---- for
			}
		} else if (bJustOne) {
			// skip all positions except the one in oTheXY
			continue; //---- for
		}
		const Tile& oTile = oLevel.boardGetTile(oXY.m_nX, oXY.m_nY);
		if (bSelectEmptyTiles) {
//std::cout << "FillerEvent::fillSelectedPositions  "; oTile.dump(); std::cout << '\n';
			if (oTile.isEmpty()) {
				aPositions.push_back(oXY);
			}
		} else {
			if (m_refTargetTiles->select(oTile)) {
				aPositions.push_back(oXY);
			}
		}
	}
//for (auto& oTmpXY : aPositions) {
//std::cout << "  nX=" << oTmpXY.m_nX << "  nY=" << oTmpXY.m_nY << '\n';
//}
	// aPositions has all the position that satisfy the selector
	const int32_t nTotPositions = static_cast<int32_t>(aPositions.size());
	if (nTotPositions <= 0) {
		return 0; //------------------------------------------------------------
	}
	//
	if (nMaxToFill <= 0) {
		nMaxToFill = static_cast<int32_t>(m_fFactorOfAvailablePositions * nTotPositions + m_nTotTiles);
		if (nMaxToFill <= 0) {
			return 0; //--------------------------------------------------------
		}
	}
	const int32_t nTotToFill = std::min(nTotPositions, nMaxToFill);

	if (m_eFillMode != FILL_MODE_IGNORE) {
		RandomTiles& oRandomTiles = *(m_refRandomTiles);
		int32_t nCurSize = nTotPositions;
		int32_t nCount = 0;
		while (nCount < nTotToFill) {
			const int32_t nIdx = oGame.random(0, nCurSize - 1);
			NPoint oXY = aPositions[nIdx];
			--nCurSize;
			if (nIdx != nCurSize) {
				aPositions[nIdx] = aPositions[nCurSize];
			}
			aPositions.pop_back();
			++nCount;
			//
			Tile oRandomTile = oRandomTiles.createTile();
			if (m_eFillMode == FILL_MODE_REPLACE) {
				//
			} else if (m_eFillMode == FILL_MODE_OVER) {
				const Tile& oTile = oLevel.boardGetTile(oXY.m_nX, oXY.m_nY);
				if (oRandomTile.getTileChar().isEmpty()) {
					oRandomTile.getTileChar() = oTile.getTileChar();
				}
				if (oRandomTile.getTileColor().isEmpty()) {
					oRandomTile.getTileColor() = oTile.getTileColor();
				}
				if (oRandomTile.getTileFont().isEmpty()) {
					oRandomTile.getTileFont() = oTile.getTileFont();
				}
				if (oRandomTile.getTileAlpha().isEmpty()) {
					oRandomTile.getTileAlpha() = oTile.getTileAlpha();
				}
			} else if (m_eFillMode == FILL_MODE_UNDER) {
				const Tile& oTile = oLevel.boardGetTile(oXY.m_nX, oXY.m_nY);
				if (! oTile.getTileChar().isEmpty()) {
					oRandomTile.getTileChar() = oTile.getTileChar();
				}
				if (! oTile.getTileColor().isEmpty()) {
					oRandomTile.getTileColor() = oTile.getTileColor();
				}
				if (! oTile.getTileFont().isEmpty()) {
					oRandomTile.getTileFont() = oTile.getTileFont();
				}
				if (! oTile.getTileAlpha().isEmpty()) {
					oRandomTile.getTileAlpha() = oTile.getTileAlpha();
				}
			}
			m_oTileCoords.add(oXY, oRandomTile);
		}
	}
	return nTotToFill;
}


} // namespace stmg
