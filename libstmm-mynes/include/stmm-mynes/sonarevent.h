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
 * File:   sonarevent.h
 */

#ifndef STMG_SONAR_EVENT_H_
#define STMG_SONAR_EVENT_H_

#include <stmm-games/event.h>
#include <stmm-games/levellisteners.h>
#include <stmm-games/tileanimator.h>
#include <stmm-games/block.h>
#include <stmm-games/utile/tileselector.h>
#include <stmm-games/util/recycler.h>
#include <stmm-games/util/namedindex.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/direction.h>

#include <vector>
#include <memory>

#include <stdint.h>

namespace stmg { class Named; }
namespace stmg { class NamedIndex; }
namespace stmg { class Coords; }
namespace stmg { class GameProxy; }
namespace stmg { class LevelBlock; }
namespace stmg { class SonarBlock; }
namespace stmg { class Tile; }
namespace stmg { class TileCoords; }
namespace stmg { class TileRect; }

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

class SonarEvent : public Event, public BoardListener, public TileAnimator
{
public:
	struct LocalInit
	{
		Block m_oBlock; /**< The block. The number of mines covered by the (current shape's) visible bricks is shown
						 * in the owning sonar tile. Cannot be empty.
						 *
						 * The block's shapes must have incremental ids
						 * from 0 to m_oBlock.totShapes() - 1. */
		int32_t m_nInitialShape = -1; /**< The initial m_oBlock shape shown. Must be valid or -1 if randomly chosen. Default is -1. */
		NPoint m_oRelPos; /**< The position of the block relative to the sonar cell.*/
		NTimeRange m_oBlockRotateInterval; /**< The interval range between m_oBlock shape changes. Is always corrected to at least one game tick.*/
		bool m_bLaunches = true; /**< Whether the current block's shape is launched if sonar cell is clicked after being activated. */
		NTimeRange m_oDirRotateInterval; /**< The interval range between launch direction shape changes. Is always always corrected to at least one game tick.*/
		NRect m_oArea; /**< The area in which sonar tiles are detected. */
		unique_ptr<TileSelector> m_refSonarTile; /**< The tiles for which a sonar is created. Cannot be null. */
		bool m_bAutoCreated = true; /**< Whether sonars are created automatically (or through MESSAGE_ACTIVATE_SONAR_AT). Default is true. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * Example: block with 4 shapes, relative position (-1,-1)
	 *     SSS   S     S
	 *      X   XS  X  SX
	 *           S SSS S
	 * @param oInit Initialization data.
	 */
	explicit SonarEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * @param oInit Initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;
	// BoardListener
	void boabloPreFreeze(LevelBlock& oBlock) noexcept override;
	void boabloPostFreeze(const Coords& oCoords) noexcept override;
	void boabloPreUnfreeze(const Coords& oCoords) noexcept override;
	void boabloPostUnfreeze(LevelBlock& oBlock) noexcept override;
	void boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostScroll(Direction::VALUE eDir) noexcept override;
	void boardPreInsert(Direction::VALUE eDir, NRect oRect, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostInsert(Direction::VALUE eDir, NRect oRect) noexcept override;
	void boardPreDestroy(const Coords& oCoords) noexcept override;
	void boardPostDestroy(const Coords& oCoords) noexcept override;
	void boardPreModify(const TileCoords& oTileCoords) noexcept override;
	void boardPostModify(const Coords& oCoords) noexcept override;

	// TileAnimator
	double getElapsed01(int32_t nHash, int32_t nX, int32_t nY, int32_t nAni, int32_t nViewTick, int32_t nTotViewTicks) const noexcept override;
	double getElapsed01(int32_t nHash, const LevelBlock& oLevelBlock, int32_t nBrickIdx
						, int32_t nAni, int32_t nViewTick, int32_t nTotTicks) const noexcept override;

	// Inputs
	enum {
		MESSAGE_ACTIVATE_SONAR_AT = 100 /**< A sonar should be created at cell with coords Util::unpackPointFromInt32(nValue). */
	};
	// Outputs
	enum {
		LISTENER_GROUP_SONAR_CREATED = 10 /**< A sonar was created at position nValue = Util::packPointToInt32(nX, nY). */
		, LISTENER_GROUP_SONAR_LAUNCHED = 11 /**< A sonar was launched at position nValue = Util::packPointToInt32(nX, nY). */
		, LISTENER_GROUP_SONAR_REMOVED = 12 /**< A sonar was removed at position nValue = Util::packPointToInt32(nX, nY). */
	};

private:
	int32_t getNamedIndex(NamedIndex& oNamedIndex, const std::string& sName, const std::string& sDescType) noexcept;
	void commonInit() noexcept;
	void runtimeInit() noexcept;

	void reactivateEvent() noexcept;
	void setSonarCellValue(int32_t nSonarIdx) noexcept;
	void checkForNumberUpdates() noexcept;
	void checkForBlockRotations(GameProxy& oGame, int32_t nTimer) noexcept;
	void checkForDirRotations(GameProxy& oGame, int32_t nTimer) noexcept;
	void checkBlockMoved() noexcept;

	int32_t getSonarIdxFromPos(int32_t nX, int32_t nY) const noexcept;
	void maybeCreateSonarCell(int32_t nX, int32_t nY) noexcept;
	void checkActiveSonarsCovering(int32_t nX, int32_t nY) noexcept;
	void checkActiveSonarsCovering(const NRect& oRect) noexcept;
	void removeSonarsIn(const NRect& oRect) noexcept;
	void removeSonarsIn(const Coords& oCoords) noexcept;

	void checkForSonarCell(int32_t nX, int32_t nY) noexcept;
	void checkForSonarCells(const NRect& oRect) noexcept;
	void checkForSonarCells(const Coords& oCoords) noexcept;
	void checkForSonarCells(const LevelBlock& oBlock) noexcept;
	//
	void createSonar(int32_t nX, int32_t nY) noexcept;
	void activateSonar(int32_t nX, int32_t nY, int32_t nNewIdx) noexcept;
	void launchSonar(int32_t nNewIdx) noexcept;
	void removeSonar(int32_t nSonarIdx) noexcept;
	void rotateBlock(int32_t nSonarIdx) noexcept;
	void rotateDir(int32_t nSonarIdx) noexcept;
	int32_t geTicksToBlockRotation(GameProxy& oGame) noexcept;
	int32_t geTicksToDirRotation(GameProxy& oGame) noexcept;
	int32_t geTicksFromTimeRange(const NTimeRange& oTimeRange, GameProxy& oGame) noexcept;
	//
	void lockedSetTile(int32_t nX, int32_t nY, const Tile& oTile) noexcept;

private:
	LocalInit m_oData;

	int32_t m_nTileAniSonarIndex;
	int32_t m_nCharMineIndex;

	int32_t m_nBoardW;
	int32_t m_nBoardH;

	enum SONAR_STATE
	{
		SONAR_STATE_ACTIVATE = 0,
		SONAR_STATE_INIT = 1,
		SONAR_STATE_RUN = 2
	};
	SONAR_STATE m_eState;
	int32_t m_nLockCount;
	int32_t m_nActiveSonars;

	int32_t m_nLastBlockRotationTick;

	//std::vector<TileChar> m_aSonarCellChar; // Save the char part of the sonar cell to be restored when the sonar cell exits area
	std::vector<NPoint> m_aSonarCells; // All the sonar cells within the area, active and inactive, Value: the position in the board
	// The game tick the next block rotation should take place
	std::vector< int32_t > m_aNextBlockRotationTicks; // Size: m_aSonarCells.size(), Value: -1 if inactive or launched
	// The game tick the next dir rotation should take place
	std::vector< int32_t > m_aNextDirRotationTicks; // Size: m_aSonarCells.size(), Value: -1 if inactive, not launchable or launched
	struct SonarData
	{
		shared_ptr<SonarBlock> m_refBlock; // empty if inactive
		int32_t m_nCurShape = -1; // -1 means not activated
		// Current direction: 0 is East, 1 NE, 2 North, 3 NW, 4 West, 5 SW, 6 South, 7 SE
		int32_t m_nCurLaunchDirIdx = -1; // -1 means not activated or not m_oData.m_bLaunches
		bool m_bLaunched = false;
	};
	std::vector< SonarData > m_aBlockData; // Size: m_aSonarCells.size()
	// Whether the number of mines should be set in sonar cell as soon as possible
	std::vector< bool > m_aUpdateMinesNumber; // Size: m_aSonarCells.size()

	Recycler<SonarBlock> m_oSonarBlockRecycler;

	//int32_t m_nCounter;
private:
	SonarEvent() = delete;
	SonarEvent(const SonarEvent& oSource) = delete;
	SonarEvent& operator=(const SonarEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_SONAR_EVENT_H_ */

