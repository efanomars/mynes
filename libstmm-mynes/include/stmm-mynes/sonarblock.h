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
 * File:   sonarblock.h
 */

#ifndef STMG_SONAR_BLOCK_H_
#define STMG_SONAR_BLOCK_H_

#include <stmm-games/levelblock.h>

#include <memory>

#include <stdint.h>

namespace stmg { class Block; }
namespace stmg { class Tile; }
namespace stmg { struct NPoint; }
namespace stmg { struct NRect; }

namespace stmg
{

using std::shared_ptr;

class SonarBlock : public LevelBlock
{
public:
	SonarBlock(const Block& oBlock, int32_t nInitialShape, const NPoint& oPos) noexcept;

	void reInit(const Block& oBlock, int32_t nInitialShape, const NPoint& oPos) noexcept;

	using LevelBlock::blockGetShapeId;
	using LevelBlock::blockMoveRotate;

	int32_t blockPosZ() const noexcept override { return s_nZObjectZSonarBlock; }

	void handleTimer() noexcept override;
	void fall() noexcept override;

	bool remove() noexcept override;
	bool destroy() noexcept override;
	bool freeze() noexcept override;
	bool fuseTo(LevelBlock& oLevelBlock) noexcept override;
	bool removeBrick(int32_t nBrickId) noexcept override;
	bool destroyBrick(int32_t nBrickId) noexcept override;

	LevelBlock::QUERY_ATTACK_TYPE queryAttack(LevelBlock& oAttacker, int32_t nBoardX, int32_t nBoardY, const Tile& oTile) const noexcept override;
	bool attack(LevelBlock& oAttacker, int32_t nBoardX, int32_t nBoardY, const Tile& oTile) noexcept override;

	/** Launch the block.
	 * This will make this block at each call to fall() in a certain direction oDeltaXY
	 * until the block as long as its position is within oArea.
	 * @param oDeltaXY The movement delta. Cannot be (0,0).
	 * @param oArea The area from which the block cannot exit.
	 */
	void launch(const NPoint oDeltaXY, const NRect& oArea) noexcept;
	/** Whether block canged position since last call.
	 * @return Whether this LevlBlock changed position.
	 */
	bool changedPosition() noexcept;
private:
	void commonInit(const Block& oBlock, int32_t nInitialShape, const NPoint& oPos) noexcept;
private:
	NPoint m_oDeltaXY;
	NRect m_oArea;
	bool m_bChangedPosition;
	static const int32_t s_nZObjectZSonarBlock;
private:
	SonarBlock() = delete;
	SonarBlock(const SonarBlock& oSource) = delete;
	SonarBlock& operator=(const SonarBlock& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_SONAR_BLOCK_H_ */

