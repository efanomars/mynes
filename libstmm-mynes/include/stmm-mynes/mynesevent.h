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
 * File:   mynesevent.h
 */

#ifndef STMG_MYNES_EVENT_H
#define STMG_MYNES_EVENT_H

#include <stmm-games/event.h>
#include <stmm-games/levellisteners.h>
#include <stmm-games/tileanimator.h>
#include <stmm-games/util/coords.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/recycler.h>
#include <stmm-games/util/direction.h>
#include <stmm-games/animations/explosionanimation.h>

#include <string>
#include <vector>
#include <memory>

#include <stdint.h>

namespace stmg { class NamedIndex; }
namespace stmg { class LevelBlock; }
namespace stmg { class Tile; }
namespace stmg { class TileCoords; }
namespace stmg { class TileFont; }
namespace stmg { class TileRect; }

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

class MynesEvent : public Event, public BoardListener, public TileAnimator
{
public:
	/** Initialization
	 *      +----------------+
	 *      +  pppppppppp    +
	 *      +  pppppppppp    +
	 *      +  cccccccccp    +
	 *      +  cccccccccp    +
	 *      +  cccccccccp    +
	 *      +  cccccccccp    +
	 *      +  cccccccccp    +
	 *      +  lllllllllp    +
	 *      +  lllllllllp    +
	 *      +----------------+
	 *
	 *      c+l+p: m_oPlayingArea
	 *      c+l: m_oCoverArea
	 *      l: m_oLockedArea (cannot be clicked)
	 * 	    p: uncover area
	 *         (flags checked, unflagged bombs exploded)
	 */
	struct LocalInit
	{
		NRect m_oPlayingArea; /**< Where empty cells are numbered. Must be within board. If empty the whole board is chosen. */
		NRect m_oCoverArea; /**< The area where cells are covered. Must be within the playing area. Cannot be empty. */
		NRect m_oLockedArea; /**< The area where cells cannot be uncovered by the player. Can be empty. */
		//TODO int32_t m_bAllowBadLuck = true; /**< Whether to allow bad luck when not possible to solve. Default is true. */
		//TODO if false it should change the bomb position when clicking on it (only in simple situation, it's probably NP-hard problem)
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit Initialization data.
	 */
	explicit MynesEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * @param oInit Initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	// Event
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

public:
	// Inputs
	enum {
		MESSAGE_UNCOVER = 100 /**< Cell with coords Util::unpackPointFromInt32(nValue) should be uncovered. */
		, MESSAGE_TOGGLE_FLAG = 101 /**< Cell with coords Util::unpackPointFromInt32(nValue) should be flag toggled. */
		, MESSAGE_FINISH = 150 /**< Stops working. */
		, MESSAGE_FINISH_UNCOVER = 151 /**< Uncovers covered area and stops working. */
		, MESSAGE_FINISH_SHOW_MINES = 152 /**< Shows mines and stops working. */
	};
	/** Outputs.
	 * All messages have the cell coordinates (packed with Util::packPointToInt32)
	 * in the associated nValue (unless stated otherwise).
	 *
	 * Group LISTENER_GROUP_REMOVE_FLAG is only triggered when the flag is removed
	 * explicitely by the player. Listeners that also want to track flag removal
	 * caused by uncovering should also listen to LISTENER_GROUP_UNCOVERED_FLAGGED_MINE
	 * and LISTENER_GROUP_UNCOVERED_FLAGGED_NON_MINE.
	 */
	enum {
		LISTENER_GROUP_UNCOVERED_FLAGGED_MINE = 10 /**< A correctly flagged mine is uncovered. */
		, LISTENER_GROUP_UNCOVERED_UNFLAGGED_MINE = 11 /**< A mine that wasn't flagged is uncovered. */
		, LISTENER_GROUP_UNCOVERED_FLAGGED_NON_MINE = 12 /**< A flagged cell not containing a mine is uncovered. */
		, LISTENER_GROUP_ADD_FLAG = 20 /**< Add a flag (flag maybe is like unflagged). */
		, LISTENER_GROUP_REMOVE_FLAG = 21 /**< Remove a flag (flag maybe is like unflagged). */
		, LISTENER_GROUP_COVERED_MINES_ADDED = 24 /**< Covered mines where added/removed by scrolling or wrongfully uncovering them
													 * (nValue contains the number of added minus removed). */
	};

	static const int32_t s_nMinCoveredW;
	static const int32_t s_nMinCoveredH;

private:
	int32_t getNamedIndex(NamedIndex& oNamedIndex, const std::string& sName, const std::string& sDescType) noexcept;
	void commonInit() noexcept;

	bool hasUncoverCells() const noexcept;
	bool isInPlayingArea(int32_t nX, int32_t nY) const noexcept;
	bool isInPlayingAreaX(int32_t nX) const noexcept;
	bool isInPlayingAreaY(int32_t nY) const noexcept;
	//bool isInUncoverAreaY(int32_t nY) const noexcept;
	bool isInCoverArea(int32_t nX, int32_t nY) const noexcept;
	bool isCovered(int32_t nFontIndex) const noexcept;
	bool isCovered(const TileFont& oFont) const noexcept;
	bool isCovered(const TileFont& oFont, int32_t& nFontIndex) const noexcept;
	bool isMine(const Tile& oTile) const noexcept;
	int32_t coverPosition(int32_t nX, int32_t nY) noexcept;
	int32_t coverCoords(const Coords& oCoords) noexcept;
	int32_t coverArea(int32_t nAreaX, int32_t nAreaY, int32_t nAreaW, int32_t nAreaH) noexcept;
	int32_t coverArea(const NRect& oArea) noexcept;
	void numberPosition(int32_t nX, int32_t nY) noexcept;
	void numberCoords(const Coords& oCoords, bool bAlsoNeighbours) noexcept;
	void numberAreaAround(int32_t nX, int32_t nY) noexcept;
	void numberArea(const NRect& oArea, bool bAlsoNeighbours) noexcept;
	void numberArea(int32_t nAreaX, int32_t nAreaY, int32_t nAreaW, int32_t nAreaH, bool bAlsoNeighbours) noexcept;
	int32_t uncoverPosition(int32_t nX, int32_t nY, bool bButtonPress) noexcept;
	int32_t uncoverPosition(int32_t nX, int32_t nY, bool bButtonPress, bool bRecursive) noexcept;
	int32_t uncoverArea(const NRect& oArea) noexcept;
	int32_t uncoverArea(int32_t nAreaX, int32_t nAreaY, int32_t nAreaW, int32_t nAreaH) noexcept;
	int32_t uncoverArea(int32_t nAreaX, int32_t nAreaY, int32_t nAreaW, int32_t nAreaH, bool bButtonPress) noexcept;
	void toggleFlag(int32_t nX, int32_t nY) noexcept;

	void showMines() noexcept;

	void lockedSetTile(int32_t nX, int32_t nY, const Tile& oTile) noexcept;

	void reActivateNow();

private:
	LocalInit m_oData;
	int32_t m_nToX; // Shortcut of m_oData.m_nAreaX + m_oData.m_nAreaW - 1

	enum MYNES_STATE
	{
		MYNES_STATE_ACTIVATE = 0
		, MYNES_STATE_INIT = 1
		, MYNES_STATE_RUN = 2
		, MYNES_STATE_FINISHING = 3
		, MYNES_STATE_FINISHED = 4
	};
	MYNES_STATE m_eState;

	int32_t m_nBoardW;
	int32_t m_nBoardH;

	int32_t m_nCharMineIndex;
	int32_t m_nCharMineBangIndex;
	int32_t m_nCharFlagWrongIndex;

	int32_t m_nFontFlagNoneIndex;
	int32_t m_nFontFlagSetIndex;
	int32_t m_nFontFlagMaybeIndex;

	int32_t m_nTileAniCoverIndex;

	std::vector<NRect> m_aUncoverAreas; // All the cells in m_oData.m_oPlayingArea but not in m_oData.m_oCoverArea

	// If scroll while m_bDirty is true is technicalGameOver
	bool m_bDirty; // Whether there are cells to cover or uncover
	Coords m_oToCoverCoords; // It's elements are in m_oData.m_oCoverArea. Filled by callbacks and handled in trigger()
	std::vector<NRect> m_aToCoverRects; // The rects must be completely within m_oData.m_oCoverArea. Filled by callbacks and handled in trigger()
	Coords m_oToUncoverCoords; // It's elements are in one of the m_aUncoverAreas. Filled by callbacks and handled in trigger()
	std::vector<NRect> m_aToUncoverRects; // The rects are all within one of the m_aUncoverAreas. Filled by callbacks and handled in trigger()

	//TODO // Positions of tiles that were changed by a Listener's callback,
	//TODO // also have TileAnimator m_nTileAniCoverIndex set
	//TODO Coords m_oDirty;

	//TODO All the recursions are a mess, simplify!
	int32_t m_nLockCount;
	bool m_bInhibitCover;

	class PrivateExplosionAnimation : public ExplosionAnimation
	{
	public:
		using ExplosionAnimation::ExplosionAnimation;
		void reInit(const ExplosionAnimation::Init& oInit) noexcept
		{
			ExplosionAnimation::reInit(oInit);
		}
	};
	static Recycler<PrivateExplosionAnimation, ExplosionAnimation> s_oBoardExplosionRecycler;

	static const int32_t s_nZObjectZUncoverArea;
	static const int32_t s_nZObjectZMineExplosion;
private:
	MynesEvent() = delete;
	MynesEvent(const MynesEvent& oSource) = delete;
	MynesEvent& operator=(const MynesEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_MYNES_EVENT_H */

