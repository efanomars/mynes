/*
 * File:   xmlmynesevent.cc
 *
 * Copyright © 2019  Stefano Marsili, <stemars@gmx.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */

#include "xmlmynesevent.h"

#include <stmm-mynes/mynesevent.h>

#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>

#include <stmm-games/level.h>

//#include <cassert>
//#include <iostream>
#include <memory>

namespace stmg
{

const std::string XmlMynesEventParser::s_sEventMynesNodeName = "MynesEvent";
static const std::string s_sEventMynesPlayingAreaXAttr = "playAreaX";
static const std::string s_sEventMynesPlayingAreaYAttr = "playAreaY";
static const std::string s_sEventMynesPlayingAreaWAttr = "playAreaW";
static const std::string s_sEventMynesPlayingAreaHAttr = "playAreaH";
static const std::string s_sEventMynesCoverAreaXAttr = "coverAreaX";
static const std::string s_sEventMynesCoverAreaYAttr = "coverAreaY";
static const std::string s_sEventMynesCoverAreaWAttr = "coverAreaW";
static const std::string s_sEventMynesCoverAreaHAttr = "coverAreaH";
static const std::string s_sEventMynesLockedAreaXAttr = "lockedAreaX";
static const std::string s_sEventMynesLockedAreaYAttr = "lockedAreaY";
static const std::string s_sEventMynesLockedAreaWAttr = "lockedAreaW";
static const std::string s_sEventMynesLockedAreaHAttr = "lockedAreaH";

XmlMynesEventParser::XmlMynesEventParser() noexcept
: XmlEventParser(s_sEventMynesNodeName)
{
}

Event* XmlMynesEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventMynes(oCtx, p0Element);
}

Event* XmlMynesEventParser::parseEventMynes(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventMynes" << std::endl;
	MynesEvent::Init oInit;
	oCtx.addChecker(p0Element);
	parseEventBase(oCtx, p0Element, oInit);

	XmlBasicParser oXmlBasicParser(getXmlConditionalParser());
	oInit.m_oPlayingArea = oXmlBasicParser.parseNRect(oCtx, p0Element
													, s_sEventMynesPlayingAreaXAttr, s_sEventMynesPlayingAreaYAttr, s_sEventMynesPlayingAreaWAttr, s_sEventMynesPlayingAreaHAttr
													, true, NRect{0, 0, oCtx.level().boardWidth(), oCtx.level().boardHeight()}
													, NSize{MynesEvent::s_nMinCoveredW, MynesEvent::s_nMinCoveredH});
	//
	oInit.m_oCoverArea = oXmlBasicParser.parseNRect(oCtx, p0Element
													, s_sEventMynesCoverAreaXAttr, s_sEventMynesCoverAreaYAttr, s_sEventMynesCoverAreaWAttr, s_sEventMynesCoverAreaHAttr
													, true, oInit.m_oPlayingArea
													, NSize{MynesEvent::s_nMinCoveredW, MynesEvent::s_nMinCoveredH});
	//
	oInit.m_oLockedArea = oXmlBasicParser.parseNRect(oCtx, p0Element
													, s_sEventMynesLockedAreaXAttr, s_sEventMynesLockedAreaYAttr, s_sEventMynesLockedAreaWAttr, s_sEventMynesLockedAreaHAttr
													, false, oInit.m_oCoverArea
													, NSize{1, 1});
	//
	oCtx.removeChecker(p0Element, true);
	auto refMynesEvent = std::make_unique<MynesEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refMynesEvent), p0Element);
}

int32_t XmlMynesEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "UNCOVER") {
		nMsg = MynesEvent::MESSAGE_UNCOVER;
	} else if (sMsgName == "TOGGLE_FLAG") {
		nMsg = MynesEvent::MESSAGE_TOGGLE_FLAG;
	} else if (sMsgName == "FINISH") {
		nMsg = MynesEvent::MESSAGE_FINISH;
	} else if (sMsgName == "FINISH_UNCOVER") {
		nMsg = MynesEvent::MESSAGE_FINISH_UNCOVER;
	} else if (sMsgName == "FINISH_SHOW_MINES") {
		nMsg = MynesEvent::MESSAGE_FINISH_SHOW_MINES;
	} else {
		nMsg = XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlMynesEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
														, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "UNCOVERED_FLAGGED_MINE") {
		nListenerGroup = MynesEvent::LISTENER_GROUP_UNCOVERED_FLAGGED_MINE;
	} else if (sListenerGroupName == "UNCOVERED_UNFLAGGED_MINE") {
		nListenerGroup = MynesEvent::LISTENER_GROUP_UNCOVERED_UNFLAGGED_MINE;
	} else if (sListenerGroupName == "UNCOVERED_FLAGGED_NON_MINE") {
		nListenerGroup = MynesEvent::LISTENER_GROUP_UNCOVERED_FLAGGED_NON_MINE;
	} else if (sListenerGroupName == "ADD_FLAG") {
		nListenerGroup = MynesEvent::LISTENER_GROUP_ADD_FLAG;
	} else if (sListenerGroupName == "REMOVE_FLAG") {
		nListenerGroup = MynesEvent::LISTENER_GROUP_REMOVE_FLAG;
	} else if (sListenerGroupName == "COVERED_MINES_ADDED") {
		nListenerGroup = MynesEvent::LISTENER_GROUP_COVERED_MINES_ADDED;
	} else {
		nListenerGroup = XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg
