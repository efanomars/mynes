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
 * File:   squarsorevent.cc
 */

#include "squarsorevent.h"

#include <stmm-games/block.h>
#include <stmm-games/appconfig.h>
#include <stmm-games/named.h>
#include <stmm-games/block.h>
#include <stmm-games/level.h>
#include <stmm-games/levelshow.h>
#include <stmm-games/util/namedindex.h>
#include <stmm-games/util/util.h>

#include <stmm-input-ev/pointerevent.h>

#include <cassert>
#include <iostream>
#include <typeinfo>
#include <cmath>

namespace stmg
{

static const std::string s_sSquarsorCharName = "SQUARSOR:SEL";
static const std::string s_sSquarsorHideCharName = "SQUARSOR:HIDE";
static constexpr const int32_t s_nZObjectZSquarsor = 2900;

const std::string SquarsorEvent::s_sPlayerOptionMoveOnKeyRelease = "MoveOnKeyRelease";

const std::string SquarsorEvent::s_sKeyActionLeft = "SquarsorEvent:Left";
const std::string SquarsorEvent::s_sKeyActionRight = "SquarsorEvent:Right";
const std::string SquarsorEvent::s_sKeyActionUp = "SquarsorEvent:Up";
const std::string SquarsorEvent::s_sKeyActionDown = "SquarsorEvent:Down";
const std::string SquarsorEvent::s_sKeyActionButtonA = "SquarsorEvent:ButtonA";
const std::string SquarsorEvent::s_sKeyActionButtonB = "SquarsorEvent:ButtonB";
const std::string SquarsorEvent::s_sKeyActionButtonC = "SquarsorEvent:ButtonC";
const std::string SquarsorEvent::s_sKeyActionNext = "SquarsorEvent:Next";

const int32_t SquarsorEvent::s_nAreaMinW = 1;
const int32_t SquarsorEvent::s_nAreaMinH = 2; // to allow top line hop

static const int32_t s_nInvalidXY = Util::packPointToInt32(NPoint{-1000,-1000});

static constexpr const int32_t s_nKeysBufferSize = 10;


SquarsorEvent::SquarsorEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, LevelBlock(false)
, m_oData(std::move(oInit))
, m_oKeys(s_nKeysBufferSize)
{
	// just checking
	static_assert(LISTENER_GROUP_BUTTON_A_PRESS == s_nGroupButtonFirst + s_nGroupRelButtonPress, "");
	static_assert(LISTENER_GROUP_BUTTON_A_RELEASE == s_nGroupButtonFirst + s_nGroupRelButtonRelease, "");
	static_assert(LISTENER_GROUP_BUTTON_A_RELEASE_CANCEL == s_nGroupButtonFirst + s_nGroupRelButtonReleaseCancel, "");
	static_assert(LISTENER_GROUP_BUTTON_A_MOVE == s_nGroupButtonFirst + s_nGroupRelButtonMove, "");
	static_assert(LISTENER_GROUP_BUTTON_B_PRESS == s_nGroupButtonFirst + s_nGroupButtonStep + s_nGroupRelButtonPress, "");
	static_assert(LISTENER_GROUP_BUTTON_C_RELEASE == s_nGroupButtonFirst + 2 * s_nGroupButtonStep + s_nGroupRelButtonRelease, "");
	commonInit();
}
void SquarsorEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}

void SquarsorEvent::commonInit() noexcept
{
//std::cout << "SquarsorEvent(" << getId() << ")::trigger" << '\n';
//std::cout << "  nInitPosX=" << nInitPosX << "  nInitPosY=" << nInitPosY << '\n';
//std::cout << "  nAreaX=" << nAreaX << "  nAreaY=" << nAreaY << "  nAreaW=" << nAreaW << "  nAreaH=" << nAreaH << '\n';
	m_bCurInvertPointerButtonsAB = false;
	m_bNextInvertPointerButtonsAB = false;

	m_nBoardWidth = level().boardWidth();
	m_nBoardHeight = level().boardHeight();

	assert((m_oData.m_nLevelTeam == -1) || (m_oData.m_nLevelTeam >= 0));
	assert((m_oData.m_oArea.m_nX >= 0) && (m_oData.m_oArea.m_nW >= s_nAreaMinW) && (m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW <= m_nBoardWidth));
	assert((m_oData.m_oArea.m_nY >= 0) && (m_oData.m_oArea.m_nH >= s_nAreaMinH) && (m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH <= m_nBoardHeight));
	assert((m_oData.m_oInitPos.m_nX >= m_oData.m_oArea.m_nX) && (m_oData.m_oInitPos.m_nX + 1 <= m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW));
	assert((m_oData.m_oInitPos.m_nY >= m_oData.m_oArea.m_nY) && (m_oData.m_oInitPos.m_nY + 1 <= m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH));

	const auto& refAppConfig = level().prefs().getAppConfig();
	m_nKeyActionLeft = refAppConfig->getKeyActionId(s_sKeyActionLeft);
	m_nKeyActionRight = refAppConfig->getKeyActionId(s_sKeyActionRight);
	m_nKeyActionUp = refAppConfig->getKeyActionId(s_sKeyActionUp);
	m_nKeyActionDown = refAppConfig->getKeyActionId(s_sKeyActionDown);
	m_nKeyActionButtonA = refAppConfig->getKeyActionId(s_sKeyActionButtonA);
	m_nKeyActionButtonB = refAppConfig->getKeyActionId(s_sKeyActionButtonB);
	m_nKeyActionButtonC = refAppConfig->getKeyActionId(s_sKeyActionButtonC);
	m_nKeyActionNext = refAppConfig->getKeyActionId(s_sKeyActionNext);
	#ifndef NDEBUG
	m_nDebugLastTimerMove = -1;
	#endif

	m_eState= SQUARSOR_EVENT_STATE_ACTIVATE;
	m_nTickStarted = -1;
	m_bPointerMode = false;
	m_nTotPressedButtons = 0;
	m_aButtonPressed.fill(false);
	m_fMouseShowXRel = -1.0;
	m_fMouseShowYRel = -1.0;
	m_nHoverXY = s_nInvalidXY;
	m_nLastLevelPlayer = -1;
	m_bMoveOnKeyRelease = false;

	m_oKeys.clear();

	initBlock();
}
static int32_t getNamedIndex(NamedIndex& oNamedIndex, const std::string& sName, const std::string&
																				#ifndef NDEBUG
																				sDescType
																				#endif //NDEBUG
																				) noexcept
{
	int32_t nIndex = oNamedIndex.getIndex(sName);
	if (nIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! SquarsorEvent: " << sDescType << " '" << sName << "' not defined!" << '\n';
		#endif //NDEBUG
		nIndex = oNamedIndex.addName(sName);
	}
	return nIndex;
}
void SquarsorEvent::initBlock() noexcept
{
	NamedIndex& oCharsIndex = level().getNamed().chars();
	const int32_t nSquarsorCharIndex = getNamedIndex(oCharsIndex, s_sSquarsorCharName, "char");
	const int32_t nSquarsorHideCharIndex = getNamedIndex(oCharsIndex, s_sSquarsorHideCharName, "char");

	std::vector<Tile> aBrick(2, Tile());
	aBrick[0].getTileChar().setCharIndex(nSquarsorCharIndex);
	aBrick[1].getTileChar().setCharIndex(nSquarsorHideCharIndex);
	std::vector< std::tuple<bool, int32_t, int32_t> > aBrickPos(2);
	aBrickPos[0] = std::make_tuple(true, 0, 0);
	aBrickPos[1] = std::make_tuple(false, 0, 0);
	std::vector< std::tuple<bool, int32_t, int32_t> > aBrickPosB(2);
	aBrickPosB[0] = std::make_tuple(false, 0, 0);
	aBrickPosB[1] = std::make_tuple(true, 0, 0);
	Block oTempBlock(2, aBrick, 2, {aBrickPos, aBrickPosB});
	//
	blockInitialSet(oTempBlock, 0, m_oData.m_oInitPos, true, m_oData.m_nLevelTeam);
}

void SquarsorEvent::trigger(int32_t nMsg, int32_t /*nValue*/, Event* p0TriggeringEvent) noexcept
{
//std::cout << "SquarsorEvent(" << getId() << ")::trigger" << '\n';
	//TODO
	// ACTIVATE activate event
	// INIT     init block
	// FALL     just live
	const int32_t nTimer = level().game().gameElapsed();
//std::cout << "SquarsorEvent(" << getId() << ")::trigger timer=" << nTimer << " state=" << (int32_t)m_eState << '\n';
	switch (m_eState)
	{
		case SQUARSOR_EVENT_STATE_ACTIVATE:
		{
			m_eState = SQUARSOR_EVENT_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				handleMsg(nMsg);
				level().activateEvent(this, nTimer);
				break;
			}
		} // fallthrough
		case SQUARSOR_EVENT_STATE_INIT:
		{
			if (p0TriggeringEvent != nullptr) {
				handleMsg(nMsg);
				return; //------------------------------------------------------
			}
			m_nHoverXY = s_nInvalidXY;
			const NPoint oPos = LevelBlock::blockPos();
			blockMove(m_oData.m_oInitPos.m_nX - oPos.m_nX, m_oData.m_oInitPos.m_nY - oPos.m_nY);
			level().blockAdd(this, LevelBlock::MGMT_TYPE_AUTO_SCROLL);
			m_eState = SQUARSOR_EVENT_STATE_WAIT;
			m_nTickStarted = nTimer;
			if (m_oData.m_nTicksToLive > 0) {
				level().activateEvent(this, nTimer + m_oData.m_nTicksToLive);
			}
			break;
		}
		case SQUARSOR_EVENT_STATE_WAIT:
		{
//std::cout << "SquarsorEvent(" << getId() << ")::trigger WAIT" << '\n';
			if (p0TriggeringEvent != nullptr) {
//std::cout << "SquarsorEvent(" << getId() << ")::trigger m_nTicksToLive=" << m_nTicksToLive << '\n';
				handleMsg(nMsg);
				if (m_oData.m_nTicksToLive > 0) {
					const int32_t nRestToLive = m_nTickStarted + m_oData.m_nTicksToLive - nTimer;
					level().activateEvent(this, std::min<int32_t>(0, nRestToLive));
					break;
				}
				return; //------------------------------------------------------
			}
//std::cout << "SquarsorEvent(" << getId() << ")::trigger DEAD" << '\n';
			m_eState = SQUARSOR_EVENT_STATE_DEAD;
			#ifndef NDEBUG
			const bool bRemoved =
			#endif //NDEBUG
			remove();
			assert(bRemoved);
		}
		break;
		case SQUARSOR_EVENT_STATE_DEAD:
		{
		}
		break;
	}
}
void SquarsorEvent::handleMsg(int32_t nMsg) noexcept
{
	if (nMsg == MESSAGE_INVERT_POINTER_AB_TOGGLE) {
		m_bNextInvertPointerButtonsAB = ! m_bNextInvertPointerButtonsAB;
	} else if (nMsg == MESSAGE_INVERT_POINTER_AB_ON) {
		m_bNextInvertPointerButtonsAB = true;
	} else if (nMsg == MESSAGE_INVERT_POINTER_AB_OFF) {
		m_bNextInvertPointerButtonsAB = false;
	}
}
void SquarsorEvent::onScrolled(Direction::VALUE eDir) noexcept
{
//std::cout << "SquarsorEvent(" << LevelBlock::blockGetId() << ")::onScrolled eDir= " << ((eDir == Direction::UP) ? "UP" : "DOWN") << '\n';
	if (!level().blockMoveIsWithinArea(*this, 0, 0, m_oData.m_oArea.m_nX, m_oData.m_oArea.m_nY, m_oData.m_oArea.m_nW, m_oData.m_oArea.m_nH)) {
		// was scrolled outside area, move back
		const Direction::VALUE eOppDir = Direction::opposite(eDir);
		const int32_t nDeltaX = Direction::deltaX(eOppDir);
		const int32_t nDeltaY = Direction::deltaY(eOppDir);
		blockMove(nDeltaX, nDeltaY);
		return; //--------------------------------------------------------------
	}
	if (m_bPointerMode) {
		if (m_nTotPressedButtons > 0) {
//std::cout << "SquarsorEvent::onScrolled eDir= " << ((eDir == Direction::UP) ? "UP" : "DOWN") << '\n';
			handlePointerInput(stmi::PointerEvent::POINTER_MOVE, m_fMouseShowXRel, m_fMouseShowYRel, -1, true);
		}
	} else {
		const NPoint oPos = LevelBlock::blockPos();
		const int32_t nPosX = oPos.m_nX;
		const int32_t nPosY = oPos.m_nY;
		doMoveKeyAction(nPosX, nPosY, 0, 0);
	}
}
void SquarsorEvent::handleTimer() noexcept
{
//std::cout << "SquarsorEvent(" << getId() << ")::handleTimer(" << level().game().gameElapsed() << ")"<< '\n';
	const bool bKeysPresent = ! m_oKeys.isEmpty();
	if (bKeysPresent && m_bPointerMode) {
		if (m_nTotPressedButtons > 0) {
			// can't switch to key mode while pointer buttons still pressed
		} else {
			m_bPointerMode = false;
//std::cout << "SquarsorEvent::handleTimer() to key mode" << '\n';
			// move to visible shape
			blockMoveRotate(0, 0, 0);
		}
	}
	if (m_bPointerMode) {
		// flush keys
		m_oKeys.clear();
		// a pointer motion is going on
		// check whether the scrolling board has brought the perfectly still (not generating POINTER_MOVE)
		// pointer into another cell
		handlePointerInput(stmi::PointerEvent::POINTER_MOVE, m_fMouseShowXRel, m_fMouseShowYRel, -1, false);
	} else if (bKeysPresent) {
		do {
			const BufKey oBufKey = m_oKeys.read();
			// if not in key mode just flush the buffer
			const int32_t nKeyAction = oBufKey.m_nKeyActionId;
			const stmi::Event::AS_KEY_INPUT_TYPE eType = oBufKey.m_eInputType;
			doKeyAction(nKeyAction, eType);
		} while (! m_oKeys.isEmpty());
	}
}
void SquarsorEvent::handleKeyActionInput(const shared_ptr<KeyActionEvent>& refEvent) noexcept
{
	KeyActionEvent* p0AEv = refEvent.get();
	const int32_t nKeyAction = p0AEv->getKeyAction();
	if (nKeyAction == m_nKeyActionNext) {
		const stmi::Event::AS_KEY_INPUT_TYPE eType = p0AEv->getType();
		if (eType == stmi::Event::AS_KEY_PRESS) {
			level().blockCycleControl(this);
		}
		return; //--------------------------------------------------------------
	}
	if (m_oKeys.isFull()) {
//std::cout << "SquarsorEvent::handleKeyActionInput m_oKeys.isFull()" << '\n';
		return; //--------------------------------------------------------------
	}
//std::cout << "SquarsorEvent::handleKeyActionInput m_oKeys add nKeyAction=" << nKeyAction << '\n';
	const stmi::Event::AS_KEY_INPUT_TYPE eType = p0AEv->getType();
	m_oKeys.write({nKeyAction, eType});
}
void SquarsorEvent::handleInput(const shared_ptr<stmi::Event>& refEvent) noexcept
{
//std::cout << "SquarsorEvent::handleInput" << '\n';
	const stmi::Event::Class& oClass = refEvent->getEventClass();
	if (oClass.isXYEvent()) {
		const std::type_info& oType = oClass.getTypeInfo();
		if (oType == typeid(stmi::PointerEvent)) {
			if (! m_bPointerMode) {
				if (m_nTotPressedButtons > 0) {
					// can't switch to pointer mode while keys still pressed
					return; //--------------------------------------------------
				}
				m_bPointerMode = true;
//std::cout << "SquarsorEvent::handleTimer() to pointer mode" << '\n';
				// show hidden shape
				blockMoveRotate(1, 0, 0);
			} else {
				if (m_nTotPressedButtons == 0) {
					m_bCurInvertPointerButtonsAB = m_bNextInvertPointerButtonsAB;
				}
			}
			stmi::PointerEvent* p0PointerEvent = static_cast<stmi::PointerEvent*>(refEvent.get());
			handlePointerInput(p0PointerEvent->getType(), p0PointerEvent->getX(), p0PointerEvent->getY(), p0PointerEvent->getButton(), false);
		}
	}
}
int32_t SquarsorEvent::getMsgFromPointerButton(int32_t nButton, int32_t nType) const noexcept
{
	if (m_bCurInvertPointerButtonsAB) {
		if (nButton <= 2) {
//std::cout << "SquarsorEvent::getMsgFromPointerButton inverting nButton=" << nButton << '\n';
			nButton = 3 - nButton;
		}
	}
	return s_nGroupButtonFirst + (nButton - 1) * s_nGroupButtonStep + nType;
}
int32_t SquarsorEvent::getMsgFromKeyButton(int32_t nButton, int32_t nType) const noexcept
{
	return s_nGroupButtonFirst + (nButton - 1) * s_nGroupButtonStep + nType;
}
int32_t SquarsorEvent::getMsgFromButton(int32_t nButton, int32_t nType) const noexcept
{
	if (m_bPointerMode) {
		return getMsgFromPointerButton(nButton, nType);
	} else {
		return getMsgFromKeyButton(nButton, nType);
	}
}
void SquarsorEvent::handlePointerInput(stmi::PointerEvent::POINTER_INPUT_TYPE eType, double fNewX, double fNewY
										, int32_t nButton, bool bInformMove) noexcept
{
	const NPoint oPos = LevelBlock::blockPos();
	const int32_t nPosX = oPos.m_nX;
	const int32_t nPosY = oPos.m_nY;
	const bool bPress = (eType == stmi::PointerEvent::BUTTON_PRESS);
	const bool bRelease = (eType == stmi::PointerEvent::BUTTON_RELEASE);
	const bool bReleaseCancel = (eType == stmi::PointerEvent::BUTTON_RELEASE_CANCEL);
	const bool bPointerMove = (eType == stmi::PointerEvent::POINTER_MOVE) || (eType == stmi::PointerEvent::POINTER_HOVER);
	// get LevelShow relative coords
	Level& oLevel = level();
	const bool bSubshowMode = oLevel.subshowMode();
	if (bSubshowMode) {
		if (m_nLastLevelPlayer < 0) {
			return; //----------------------------------------------------------
		}
	}
	LevelShow& oLevelShow = [&]() -> LevelShow&
	{
		if (bSubshowMode) {
			return oLevel.subshowGet(m_nLastLevelPlayer);
		} else {
			return oLevel.showGet();
		}
	}();
	const FPoint oBoardNew = oLevelShow.getBoardPos(fNewX, fNewY);
	const int32_t nNewX = oBoardNew.m_fX;
	const int32_t nNewY = oBoardNew.m_fY;
	const int32_t nOldValue = Util::packPointToInt32(NPoint{nPosX, nPosY});
	const bool bButton = ((nButton >= 1) && (nButton <= s_nTotButtons));

	if (bReleaseCancel || !m_oData.m_oArea.containsPoint(NPoint{nNewX, nNewY})) {
		if (bRelease || bReleaseCancel) {
			// send the last valid position with the cancel
			if (bButton && m_aButtonPressed[nButton - 1]) {
				m_aButtonPressed[nButton - 1] = false;
				--m_nTotPressedButtons;
				informListeners(getMsgFromPointerButton(nButton, s_nGroupRelButtonReleaseCancel), nOldValue);
			}
		} else if (bPress) {
			informListeners(getMsgFromPointerButton(nButton, s_nGroupRelButtonOOBPress), nOldValue);
		}
		return; //--------------------------------------------------------------
	}
	// the new position is valid
	// store show/subshow area relative coords
	m_fMouseShowXRel = fNewX;
	m_fMouseShowYRel = fNewY;
	const int32_t nNewValue = Util::packPointToInt32(NPoint{nNewX, nNewY});
	const int32_t nDx = nNewX - nPosX;
	const int32_t nDy = nNewY - nPosY;
	if (bPointerMove) {
		bool bHover = true;
		for (int32_t nCurB = 1; nCurB <= s_nTotButtons; ++nCurB) {
			if (m_aButtonPressed[nCurB - 1]) {
				bHover = false;
				break;
			}
		}
		if (!bHover) {
			if (nNewValue != nOldValue) {
//std::cout << "SquarsorEvent::handlePointerInput bPointerMove nNewX = " << nNewX << "   nNewY=" << nNewY << '\n';
//std::cout << "                                               nDx = " << nDx << "   nDy=" << nDy << '\n';
				blockMove(nDx, nDy);
				bInformMove = true;
			}
			if (bInformMove) {
				// Beware! if both buttons are pressed two move messages are emitted!
				for (int32_t nCurB = 1; nCurB <= s_nTotButtons; ++nCurB) {
					if (m_aButtonPressed[nCurB - 1]) {
						informListeners(getMsgFromPointerButton(nCurB, s_nGroupRelButtonMove), nNewValue);
					}
				}
//std::cout << "SquarsorEvent::handlePointerInput AFTER bPointerMove nNewX = " << nNewX << "   nNewY=" << nNewY << '\n';
			}
		} else { // bHover
			if (nNewValue != m_nHoverXY) {
				m_nHoverXY = nNewValue;
				// contrary to key movement the block isn't actually moved!
				informListeners(LISTENER_GROUP_POINTER_HOVER, nNewValue);
//std::cout << "SquarsorEvent::handlePointerInput bPointerMove Hover nNewX = " << nNewX << "   nNewY=" << nNewY << '\n';
			}
		}
	} else {
		if (bPress) {
			if (bButton) {
				if (m_aButtonPressed[nButton - 1]) {
					// if key press or an orphan (of release) mouse press, cancel
					informListeners(getMsgFromPointerButton(nButton, s_nGroupRelButtonReleaseCancel), nOldValue);
				} else {
					m_aButtonPressed[nButton - 1] = true;
					++m_nTotPressedButtons;
				}
				if (nNewValue != nOldValue) {
					blockMove(nDx, nDy);
				}
				informListeners(getMsgFromPointerButton(nButton, s_nGroupRelButtonPress), nNewValue);
			}
		} else {
			assert(bRelease);
			if (bButton) {
				if (m_aButtonPressed[nButton - 1]) {
					m_aButtonPressed[nButton - 1] = false;
					--m_nTotPressedButtons;
					if (nNewValue != nOldValue) {
						blockMove(nDx, nDy);
					}
					informListeners(getMsgFromPointerButton(nButton, s_nGroupRelButtonRelease), nNewValue);
				}
			}
		}
	}
}
void SquarsorEvent::doMoveKeyAction(int32_t nPosX, int32_t nPosY, int32_t nDx, int32_t nDy) noexcept
{
	const int32_t nNewX = nPosX + nDx;
	const int32_t nNewY = nPosY + nDy;
	if (! m_oData.m_oArea.containsPoint(NPoint{nNewX, nNewY})) {
		// moving outside allowed area: ignore
		return; //------------------------------------------------------
	}
	const int32_t nNewValue = Util::packPointToInt32(NPoint{nNewX, nNewY});
//std::cout << "SquarsorEvent::doMoveKeyAction  nPosX=" << nPosX << " nPosY=" << nPosY << " nDx=" << nDx << " nDy=" << nDy << '\n';
	blockMove(nDx, nDy);
	const bool bHover = (m_nTotPressedButtons == 0);
	if (!bHover) {
		// Beware! A move message is emitted for each pressed button!
		for (int32_t nCurB = 1; nCurB <= s_nTotButtons; ++nCurB) {
			if (m_aButtonPressed[nCurB - 1]) {
				informListeners(getMsgFromButton(nCurB, s_nGroupRelButtonMove), nNewValue);
			}
		}
	} else { // bHover
		informListeners(LISTENER_GROUP_POINTER_HOVER, nNewValue);
	}
}
void SquarsorEvent::doKeyAction(int32_t nKeyAction, KeyActionEvent::AS_KEY_INPUT_TYPE eType) noexcept
{
//std::cout << "SquarsorEvent(" << blockGetId() << ")::doKeyAction(" << nKeyAction << "," << static_cast<int32_t>(eType) << ")"<< '\n';
	const bool bPress = (eType == KeyActionEvent::AS_KEY_PRESS);
	const bool bRelease = (eType == KeyActionEvent::AS_KEY_RELEASE);
	int32_t nDx = 0;
	int32_t nDy = 0;
	if (nKeyAction == m_nKeyActionLeft) {
		nDx = -1;
	} else if (nKeyAction == m_nKeyActionRight) {
		nDx = +1;
	} else if (nKeyAction == m_nKeyActionUp) {
		nDy = -1;
	} else if (nKeyAction == m_nKeyActionDown) {
		nDy = +1;
	}
	const NPoint oPos = LevelBlock::blockPos();
	const int32_t nPosX = oPos.m_nX;
	const int32_t nPosY = oPos.m_nY;
	const bool bButton = ((nDx == 0) && (nDy == 0));
	if (! bButton) {
		if ((m_bMoveOnKeyRelease && bRelease) || ((!m_bMoveOnKeyRelease) && bPress)) {
			doMoveKeyAction(nPosX, nPosY, nDx, nDy);
		}
		return; //--------------------------------------------------------------
	}
	const bool bReleaseCancel = (eType == KeyActionEvent::AS_KEY_RELEASE_CANCEL);

	int32_t nButton = -1;
	if (nKeyAction == m_nKeyActionButtonA) {
		nButton = 1;
	} else if (nKeyAction == m_nKeyActionButtonB) {
		nButton = 2;
	} else if (nKeyAction == m_nKeyActionButtonC) {
		nButton = 3;
	} else {
		// Don't call this function with key action m_nKeyActionNext
		assert(false);
		return; //--------------------------------------------------------------
	}

	const int32_t nOldValue = Util::packPointToInt32(NPoint{nPosX, nPosY});
	if (bReleaseCancel) {
		if (m_aButtonPressed[nButton - 1]) {
			m_aButtonPressed[nButton - 1] = false;
			--m_nTotPressedButtons;
			informListeners(getMsgFromKeyButton(nButton, s_nGroupRelButtonReleaseCancel), nOldValue);
		}
		return; //--------------------------------------------------------------
	}

	if (bPress) {
		if (m_aButtonPressed[nButton - 1]) {
			// repeated presses (ignored)
		} else {
			m_aButtonPressed[nButton - 1] = true;
			++m_nTotPressedButtons;
			informListeners(getMsgFromKeyButton(nButton, s_nGroupRelButtonPress), nOldValue);
			if (m_oData.m_bReleaseOnKeyPress) {
				m_aButtonPressed[nButton - 1] = false;
				--m_nTotPressedButtons;
				informListeners(getMsgFromKeyButton(nButton, s_nGroupRelButtonRelease), nOldValue);
				// the real key release will be orphaned!
			}
		}
		return; //--------------------------------------------------------------
	}
	assert(bRelease);
	if (m_aButtonPressed[nButton - 1]) {
		m_aButtonPressed[nButton - 1] = false;
		--m_nTotPressedButtons;
		informListeners(getMsgFromKeyButton(nButton, s_nGroupRelButtonRelease), nOldValue);
	}
}
void SquarsorEvent::onPlayerChanged() noexcept
{
	auto aButtonPressed = m_aButtonPressed;
	for (int32_t nCurB = 1; nCurB <= s_nTotButtons; ++nCurB) {
		m_aButtonPressed[nCurB - 1] = false;
	}
	for (int32_t nCurB = 1; nCurB <= s_nTotButtons; ++nCurB) {
		if (aButtonPressed[nCurB - 1]) {
			informListeners(getMsgFromButton(nCurB, s_nGroupRelButtonReleaseCancel), -1);
		}
	}
	const int32_t nLevelPlayer = getPlayer();
	if (nLevelPlayer < 0) {
		// not controlled
		return; //--------------------------------------------------------------
	}
	if (m_nLastLevelPlayer == nLevelPlayer) {
		return; //--------------------------------------------------------------
	}
	Level& oLevel = level();
	const int32_t nPlayer = oLevel.game().getPlayer(level().getLevel(), nLevelPlayer);
	const auto oVal = oLevel.prefs().getPlayer(nPlayer)->getOptionValue(s_sPlayerOptionMoveOnKeyRelease);
	assert(oVal.getType() == Variant::TYPE_BOOL);
	m_bMoveOnKeyRelease = oVal.getBool();
	m_nLastLevelPlayer = nLevelPlayer;
}

int32_t SquarsorEvent::blockPosZ() const noexcept
{
	return s_nZObjectZSquarsor;
}
void SquarsorEvent::fall() noexcept
{
//std::cout << "SquarsorEvent(" << getId() << ")::fall" << '\n';
}
void SquarsorEvent::privateOnRemove() noexcept
{
	informListeners(LISTENER_GROUP_FINISHED, 0);
}
bool SquarsorEvent::remove() noexcept
{
	const bool bRemoved = LevelBlock::remove();
	if (bRemoved) {
		privateOnRemove();
	}
	return bRemoved;
}
bool SquarsorEvent::destroy() noexcept
{
	return false;
}
bool SquarsorEvent::freeze() noexcept
{
	return false;
}
bool SquarsorEvent::fuseTo(LevelBlock& /*oLevelBlock*/) noexcept
{
	return false;
}
bool SquarsorEvent::canFuseWith(LevelBlock& /*oLevelBlock*/) const noexcept
{
	return false;
}
bool SquarsorEvent::removeBrick(int32_t /*nBrickId*/) noexcept
{
	return false;
}
bool SquarsorEvent::destroyBrick(int32_t /*nBrickId*/) noexcept
{
	return false;
}

} // namespace stmg
