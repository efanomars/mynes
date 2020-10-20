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
 * File:   pressevent.h
 */

#ifndef STMG_PRESS_EVENT_H
#define STMG_PRESS_EVENT_H

#include <stmm-games/event.h>
#include <stmm-games/levellisteners.h>
#include <stmm-games/tileanimator.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/direction.h>
#include <stmm-games/util/coords.h>

#include <string>
#include <memory>

#include <stdint.h>

namespace stmg { class Coords; }
namespace stmg { class LevelBlock; }
namespace stmg { class TileCoords; }
namespace stmg { class TileRect; }

namespace stmg
{

using std::shared_ptr;

class PressEvent : public Event, public BoardListener, public TileAnimator
{
public:
	struct LocalInit
	{
		NRect m_oArea; /**< The area this event allows to press. If a width or height is &lt;= 0 sets it to the corresponding board width or height. */
		int32_t m_nAniNameIdx = -1; /**< The "cell pressed" tile animation id. Must be valid in Named::tileAnis(). */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit Initialization data.
	 */
	explicit PressEvent(Init&& oInit) noexcept;
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

	// Inputs
	enum {
		MESSAGE_BUTTON_PRESS = 110 /**< Button A press on cell with coords Util::unpackPointFromInt32(nValue). */
		, MESSAGE_BUTTON_RELEASE = 111 /**< Button A release on cell with coords Util::unpackPointFromInt32(nValue). */
		, MESSAGE_BUTTON_RELEASE_CANCEL = 112 /**< Button A release cancel. */
		, MESSAGE_BUTTON_MOVE = 115  /**< Button A move to cell with coords Util::unpackPointFromInt32(nValue). */
	};
	/** Outputs.
	 * All messages have the cell coordinates (packed with Util::packPointToInt32)
	 * in the associated nValue (unless stated otherwise).
	 */
	enum {
		LISTENER_GROUP_RELEASE = 10 /**< Cell's press was released. */
	};

private:
	void commonInit() noexcept;

	bool isInActiveArea(int32_t nX, int32_t nY) const noexcept;
	void press(int32_t nPackedXY) noexcept;
	void unPress(bool bCancel) noexcept;

	void boardPostDeleteDown(int32_t nY, int32_t nX, int32_t nW) noexcept;
	void boardPostInsertUp(int32_t nY, int32_t nX, int32_t nW) noexcept;
private:
	LocalInit m_oData;

	enum PRESS_STATE
	{
		PRESS_STATE_ACTIVATE = 0
		, PRESS_STATE_INIT = 1
		, PRESS_STATE_RUN = 2
	};
	PRESS_STATE m_eState;

	int32_t m_nBoardW;
	int32_t m_nBoardH;

	int32_t m_nPressAXY; // The press position, if invalid not pressed or outside active area

	static const int32_t s_nInvalidXY;
private:
	PressEvent() = delete;
	PressEvent(const PressEvent& oSource) = delete;
	PressEvent& operator=(const PressEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_PRESS_EVENT_H */

