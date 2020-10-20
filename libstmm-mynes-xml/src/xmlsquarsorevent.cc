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
 * File:   xmlsquarsorevent.cc
 */

#include "xmlsquarsorevent.h"

#include <stmm-mynes/squarsorevent.h>

#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>

#include <stmm-games/game.h>

#include <string>
#include <utility>
//#include <cassert>
//#include <iostream>

namespace stmg
{

static const std::string s_sEventSquarsorNodeName = "SquarsorEvent";
	//static const std::string s_sEventSquarsorTeam = "team";
static const std::string s_sEventSquarsorInitXAttr = "posX";
static const std::string s_sEventSquarsorInitYAttr = "posY";
static const std::string s_sEventSquarsorAreaXAttr = "areaX";
static const std::string s_sEventSquarsorAreaYAttr = "areaY";
static const std::string s_sEventSquarsorAreaWAttr = "areaW";
static const std::string s_sEventSquarsorAreaHAttr = "areaH";
static const std::string s_sEventSquarsorTicksToLiveAttr = "ticksToLive";
static const std::string s_sEventSquarsorReleaseOnKeyPressAttr = "releaseOnKeyPress";

XmlSquarsorEventParser::XmlSquarsorEventParser()
: XmlEventParser(s_sEventSquarsorNodeName)
{
}
Event* XmlSquarsorEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventSquarsor(oCtx, p0Element);
}
Event* XmlSquarsorEventParser::parseEventSquarsor(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventSquarsor" << std::endl;
	SquarsorEvent::Init oInit;
	oCtx.addChecker(p0Element);
	Level& oLevel = oCtx.level();
	parseEventBase(oCtx, p0Element, oInit);

	const auto oTupleTeam = getXmlConditionalParser().parseOwnerExists(oCtx, p0Element);
	const bool bExists = std::get<0>(oTupleTeam);
	// Note: ignores mate even if defined because blocks can currently only
	//       be restricted to a team not a player
	// Note 2: in OneTeamPerLevel mode it doesn't matter if a block can only
	//         be controlled by players of a certain team since there is only
	//         one team in the level anyway
	const int32_t nTeam = (bExists ? std::get<1>(oTupleTeam) : -1);
	if (oCtx.game().isAllTeamsInOneLevel()) {
		oInit.m_nLevelTeam = nTeam;
	} else {
		oInit.m_nLevelTeam = 0;
	}
	;
	XmlBasicParser oXmlBasicParser(getXmlConditionalParser());
	oInit.m_oArea = oXmlBasicParser.parseNRect(oCtx, p0Element
												, s_sEventSquarsorAreaXAttr, s_sEventSquarsorAreaYAttr, s_sEventSquarsorAreaWAttr, s_sEventSquarsorAreaHAttr
												,  true, NRect{0, 0, oLevel.boardWidth(), oLevel.boardHeight()}
												, NSize{SquarsorEvent::s_nAreaMinW, SquarsorEvent::s_nAreaMinH});
	;
	oInit.m_oInitPos.m_nX = oInit.m_oArea.m_nX + (oInit.m_oArea.m_nW - 2) / 2;
	const auto oPairInitPosX = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSquarsorInitXAttr);
	if (oPairInitPosX.first) {
		const std::string& sInitPosX = oPairInitPosX.second;
		oInit.m_oInitPos.m_nX = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSquarsorInitXAttr, sInitPosX, false
															, true, oInit.m_oArea.m_nX, true, oInit.m_oArea.m_nX + oInit.m_oArea.m_nW - 2);
	}
	oInit.m_oInitPos.m_nY = oInit.m_oArea.m_nY + (oInit.m_oArea.m_nH - 1) / 2;
	const auto oPairInitPosY = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSquarsorInitYAttr);
	if (oPairInitPosY.first) {
		const std::string& sInitPosY = oPairInitPosY.second;
		oInit.m_oInitPos.m_nY = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSquarsorInitYAttr, sInitPosY, false
															, true, oInit.m_oArea.m_nY, true, oInit.m_oArea.m_nY + oInit.m_oArea.m_nH - 1);
	}
	;
	const auto oPairTicksToLive = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSquarsorTicksToLiveAttr);
	if (oPairTicksToLive.first) {
		const std::string& sTicksToLive = oPairTicksToLive.second;
		oInit.m_nTicksToLive = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSquarsorTicksToLiveAttr, sTicksToLive, false
																	, true, -1, false, -1);
	}
	;
	const auto oPairReleaseOnKeyPress = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSquarsorReleaseOnKeyPressAttr);
	if (oPairReleaseOnKeyPress.first) {
		const std::string& sReleaseOnKeyPress = oPairReleaseOnKeyPress.second;
		oInit.m_bReleaseOnKeyPress = XmlUtil::strToBool(oCtx, p0Element, s_sEventSquarsorReleaseOnKeyPressAttr, sReleaseOnKeyPress);
	}
	;
	oCtx.removeChecker(p0Element, true);
	auto refSquarsorEvent = std::make_unique<SquarsorEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refSquarsorEvent), p0Element);
}

int32_t XmlSquarsorEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "INVERT_POINTER_AB_ON") {
		nMsg = SquarsorEvent::MESSAGE_INVERT_POINTER_AB_ON;
	} else if (sMsgName == "_INVERT_POINTER_AB_OFF") {
		nMsg = SquarsorEvent::MESSAGE_INVERT_POINTER_AB_OFF;
	} else if (sMsgName == "INVERT_POINTER_AB_TOGGLE") {
		nMsg = SquarsorEvent::MESSAGE_INVERT_POINTER_AB_TOGGLE;
	} else {
		nMsg = XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlSquarsorEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
															, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "BUTTON_A_PRESS") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_A_PRESS;
	} else if (sListenerGroupName == "BUTTON_A_RELEASE") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_A_RELEASE;
	} else if (sListenerGroupName == "BUTTON_A_RELEASE_CANCEL") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_A_RELEASE_CANCEL;
	} else if (sListenerGroupName == "BUTTON_A_MOVE") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_A_MOVE;
	} else if (sListenerGroupName == "BUTTON_A_OOB_PRESS") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_A_OOB_PRESS;
	} else if (sListenerGroupName == "BUTTON_B_PRESS") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_B_PRESS;
	} else if (sListenerGroupName == "BUTTON_B_RELEASE") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_B_RELEASE;
	} else if (sListenerGroupName == "BUTTON_B_RELEASE_CANCEL") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_B_RELEASE_CANCEL;
	} else if (sListenerGroupName == "BUTTON_B_MOVE") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_B_MOVE;
	} else if (sListenerGroupName == "BUTTON_B_OOB_PRESS") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_B_OOB_PRESS;
	} else if (sListenerGroupName == "BUTTON_C_PRESS") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_C_PRESS;
	} else if (sListenerGroupName == "BUTTON_C_RELEASE") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_C_RELEASE;
	} else if (sListenerGroupName == "BUTTON_C_RELEASE_CANCEL") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_C_RELEASE_CANCEL;
	} else if (sListenerGroupName == "BUTTON_C_MOVE") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_C_MOVE;
	} else if (sListenerGroupName == "BUTTON_C_OOB_PRESS") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_BUTTON_C_OOB_PRESS;
	} else if (sListenerGroupName == "POINTER_HOVER") {
		nListenerGroup = SquarsorEvent::LISTENER_GROUP_POINTER_HOVER;
	} else {
		nListenerGroup = XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg
