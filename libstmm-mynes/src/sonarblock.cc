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
 * File:   sonarblock.cc
 */

#include "sonarblock.h"

#include <stmm-games/util/basictypes.h>

//#include <cassert>
//#include <iostream>

namespace stmg { class Block; }
namespace stmg { class Tile; }

namespace stmg
{

const int32_t SonarBlock::s_nZObjectZSonarBlock = 208;

SonarBlock::SonarBlock(const Block& oBlock, int32_t nInitialShape, const NPoint& oPos) noexcept
: LevelBlock(false)
{
	commonInit(oBlock, nInitialShape, oPos);
}
void SonarBlock::reInit(const Block& oBlock, int32_t nInitialShape, const NPoint& oPos) noexcept
{
	commonInit(oBlock, nInitialShape, oPos);
}
void SonarBlock::commonInit(const Block& oBlock, int32_t nInitialShape, const NPoint& oPos) noexcept
{
	LevelBlock::blockInitialSet(oBlock, nInitialShape, oPos, false, 0);
	m_oDeltaXY = NPoint{0,0};
	m_oArea = NRect{0,0,1,1};
	m_bChangedPosition = true;
}

void SonarBlock::handleTimer() noexcept
{
}
void SonarBlock::launch(const NPoint oDeltaXY, const NRect& oArea) noexcept
{
	m_oDeltaXY = oDeltaXY;
	m_oArea = oArea;
}
void SonarBlock::fall() noexcept
{
	if ((m_oDeltaXY.m_nX != 0) || (m_oDeltaXY.m_nY != 0)) {
		const NPoint oPos = LevelBlock::blockPos();
		if (m_oArea.containsPoint(NPoint{oPos.m_nX + m_oDeltaXY.m_nX, oPos.m_nY + m_oDeltaXY.m_nY})) {
			LevelBlock::blockMove(m_oDeltaXY.m_nX, m_oDeltaXY.m_nY);
			m_bChangedPosition = true;
		}
	}
}
bool SonarBlock::changedPosition() noexcept
{
	const bool bChangedPosition = m_bChangedPosition;
	m_bChangedPosition = false;
	return bChangedPosition;
}
bool SonarBlock::remove() noexcept
{
	return LevelBlock::remove();
}
bool SonarBlock::destroy() noexcept
{
	return remove();
}
bool SonarBlock::freeze() noexcept
{
	return remove();
}
bool SonarBlock::fuseTo(LevelBlock& /*oLevelBlock*/) noexcept
{
	return false;
}
bool SonarBlock::removeBrick(int32_t /*nBrickId*/) noexcept
{
	return false;
}
bool SonarBlock::destroyBrick(int32_t /*nBrickId*/) noexcept
{
	return false;
}

LevelBlock::QUERY_ATTACK_TYPE SonarBlock::queryAttack(LevelBlock& /*oAttacker*/
														, int32_t /*nBoardX*/, int32_t /*nBoardY*/, const Tile& /*oTile*/) const noexcept
{
	return LevelBlock::QUERY_ATTACK_TYPE_NOTHING;
}
bool SonarBlock::attack(LevelBlock& /*oAttacker*/, int32_t /*nBoardX*/, int32_t /*nBoardY*/, const Tile& /*oTile*/) noexcept
{
	return false;
}

} // namespace stmg
