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
 * File:   squarsorevent.h
 */

#ifndef STMG_SQUARSOR_EVENT_H
#define STMG_SQUARSOR_EVENT_H

#include <stmm-games/event.h>
#include <stmm-games/levelblock.h>
#include <stmm-games/keyactionevent.h>
#include <stmm-games/util/direction.h>
#include <stmm-games/util/circularbuffer.h>
#include <stmm-games/util/basictypes.h>

#include <stmm-input/stmm-input.h>
#include <stmm-input-ev/pointerevent.h>

#include <vector>
#include <string>
#include <array>

#include <stdint.h>

namespace stmg { class Tile; }
namespace stmi { class Event; }
namespace stmg { class Named; }
namespace stmg { class NamedIndex; }

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

//TODO Fade Block when mouse is used, unfade when key actions are used
class SquarsorEvent : public Event, public LevelBlock
{
public:
	struct LocalInit
	{
		NPoint m_oInitPos; /**< The initial position of the squarsor. Must be within m_oArea. Default (0,0). */
		NRect m_oArea; /**< The board area the squarsor is allowed to move in. */
		int32_t m_nTicksToLive = -1; /**< Game ticks after which the squarsor (LevelBlock) is removed. Default is -1 (means never). */
		int32_t m_nLevelTeam = -1; /**< The level team that can take control of the squarsor or -1 if any. Default is -1. */
		bool m_bReleaseOnKeyPress = false; /**< Whether a button is immediately released after it was pressed. Default is false. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit Initialization data.
	 */
	explicit SquarsorEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * @param oInit Initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	// Event
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;
	// LevelBlock
	void handleInput(const shared_ptr<stmi::Event>& refEvent) noexcept override;
	void handleKeyActionInput(const shared_ptr<KeyActionEvent>& refEvent) noexcept override;
	void handleTimer() noexcept override;
	int32_t blockPosZ() const noexcept override;
	void fall() noexcept override;
	bool remove() noexcept override;
	bool destroy() noexcept override;
	bool freeze() noexcept override;
	bool fuseTo(LevelBlock& oLevelBlock) noexcept override;
	bool canFuseWith(LevelBlock& oLevelBlock) const noexcept override;
	bool removeBrick(int32_t nBrickId) noexcept override;
	bool destroyBrick(int32_t nBrickId) noexcept override;

	LevelBlock::QUERY_ATTACK_TYPE queryAttack(LevelBlock& /*oAttacker*/, int32_t /*nBoardX3*/, int32_t /*nBoardY*/
											, const Tile& /*oTile*/) const noexcept override
	{
		return LevelBlock::QUERY_ATTACK_TYPE_NOTHING;
	}
	bool attack(LevelBlock& /*oAttacker*/, int32_t /*nBoardX*/, int32_t /*nBoardY*/, const Tile& /*oTile*/) noexcept override { return false; }

	// Inputs
	// Inversion change only takes place after release of all (involved?) buttons
	enum {
		MESSAGE_INVERT_POINTER_AB_ON = 10 /**< Inverts A and B pointer buttons. */
		, MESSAGE_INVERT_POINTER_AB_OFF = 11 /**< Don't invert A and B pointer buttons. */
		, MESSAGE_INVERT_POINTER_AB_TOGGLE = 12 /**< Toggles A and B pointer buttons inversion. */
	};
	// Outputs
	enum {
		// nValue: cell coordinates (packed with Util::packPointToInt32)
		LISTENER_GROUP_BUTTON_A_PRESS = 10
		, LISTENER_GROUP_BUTTON_A_RELEASE = 11
		, LISTENER_GROUP_BUTTON_A_RELEASE_CANCEL = 12
		, LISTENER_GROUP_BUTTON_A_MOVE = 15
		, LISTENER_GROUP_BUTTON_A_OOB_PRESS = 18
		, LISTENER_GROUP_BUTTON_B_PRESS = 20
		, LISTENER_GROUP_BUTTON_B_RELEASE = 21
		, LISTENER_GROUP_BUTTON_B_RELEASE_CANCEL = 22
		, LISTENER_GROUP_BUTTON_B_MOVE = 25
		, LISTENER_GROUP_BUTTON_B_OOB_PRESS = 28
		, LISTENER_GROUP_BUTTON_C_PRESS = 30
		, LISTENER_GROUP_BUTTON_C_RELEASE = 31
		, LISTENER_GROUP_BUTTON_C_RELEASE_CANCEL = 32
		, LISTENER_GROUP_BUTTON_C_MOVE = 35
		, LISTENER_GROUP_BUTTON_C_OOB_PRESS = 38
		, LISTENER_GROUP_POINTER_HOVER = 50
	};

	static const std::string s_sPlayerOptionMoveOnKeyRelease;

	static const std::string s_sKeyActionLeft;
	static const std::string s_sKeyActionRight;
	static const std::string s_sKeyActionUp;
	static const std::string s_sKeyActionDown;
	static const std::string s_sKeyActionButtonA;
	static const std::string s_sKeyActionButtonB;
	static const std::string s_sKeyActionButtonC;
	static const std::string s_sKeyActionNext;

	static const int32_t s_nAreaMinW;
	static const int32_t s_nAreaMinH;

protected:
	void onScrolled(Direction::VALUE eDir) noexcept override;
	void onPlayerChanged() noexcept override;

	void handlePointerInput(stmi::PointerEvent::POINTER_INPUT_TYPE eType, double fNewX, double fNewY, int32_t nButton, bool bInformMove) noexcept;
private:
	//TODO pass Tile to use in the single Brick Block
	void commonInit() noexcept;
	void initBlock() noexcept;
	void privateOnRemove() noexcept;
	void doKeyAction(int32_t nKeyAction, KeyActionEvent::AS_KEY_INPUT_TYPE eType) noexcept;
	void doMoveKeyAction(int32_t nPosX, int32_t nPosY, int32_t nDx, int32_t nDy) noexcept;
	void handleMsg(int32_t nMsg) noexcept;
	int32_t getMsgFromButton(int32_t nButton, int32_t nType) const noexcept;
	int32_t getMsgFromKeyButton(int32_t nButton, int32_t nType) const noexcept;
	int32_t getMsgFromPointerButton(int32_t nButton, int32_t nType) const noexcept;

	enum SQUARSOR_EVENT_STATE
	{
		SQUARSOR_EVENT_STATE_ACTIVATE = 0,
		SQUARSOR_EVENT_STATE_INIT = 1,
		SQUARSOR_EVENT_STATE_WAIT = 2,
		SQUARSOR_EVENT_STATE_DEAD = 3
	};
private:
	LocalInit m_oData;

	bool m_bCurInvertPointerButtonsAB;
	bool m_bNextInvertPointerButtonsAB;

	int32_t m_nBoardWidth;
	int32_t m_nBoardHeight;

	int32_t m_nTileAniRemovingIndex;

	bool m_bMoveOnKeyRelease;
	int32_t m_nLastLevelPlayer;

	SQUARSOR_EVENT_STATE m_eState;
	int32_t m_nTickStarted;

	struct BufKey
	{
		int32_t m_nKeyActionId = -1;
		stmi::Event::AS_KEY_INPUT_TYPE m_eInputType;
	};
	CircularBuffer<BufKey> m_oKeys;

	static constexpr const int32_t s_nTotButtons = 3;
	static constexpr const int32_t s_nGroupButtonFirst = 10;
	static constexpr const int32_t s_nGroupButtonStep = 10;
	static constexpr const int32_t s_nGroupRelButtonPress = 0;
	static constexpr const int32_t s_nGroupRelButtonRelease = 1;
	static constexpr const int32_t s_nGroupRelButtonReleaseCancel = 2;
	static constexpr const int32_t s_nGroupRelButtonMove = 5;
	static constexpr const int32_t s_nGroupRelButtonOOBPress = 8;

	bool m_bPointerMode; // if true in touch or mouse mode, if false in keyboard mode
	// switching mode is only possible if no button is pressed
	// in pointer mode shape 1 of the squarsor block is shown,
	// in keyboard mode shape 0 is shown
	int32_t m_nTotPressedButtons;
	std::array<bool, s_nTotButtons> m_aButtonPressed;
	double m_fMouseShowXRel;
	double m_fMouseShowYRel;

	int32_t m_nHoverXY;

	int32_t m_nKeyActionLeft;
	int32_t m_nKeyActionRight;
	int32_t m_nKeyActionUp;
	int32_t m_nKeyActionDown;
	int32_t m_nKeyActionButtonA;
	int32_t m_nKeyActionButtonB;
	int32_t m_nKeyActionButtonC;
	int32_t m_nKeyActionNext;

#ifndef NDEBUG
	int32_t m_nDebugLastTimerMove;
#endif

private:
	SquarsorEvent() = delete;
	SquarsorEvent(const SquarsorEvent& oSource) = delete;
	SquarsorEvent& operator=(const SquarsorEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_SQUARSOR_EVENT_H */

