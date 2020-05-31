/*
 * File:   xmlpressevent.cc
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

#include "xmlpressevent.h"

#include <stmm-mynes/pressevent.h>

#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>

#include <stmm-games/level.h>
#include <stmm-games/named.h>
#include <stmm-games/util/namedindex.h>

#include <cassert>
//#include <iostream>
#include <utility>

namespace stmg
{

const std::string XmlPressEventParser::s_sEventPressNodeName = "PressEvent";
static const std::string s_sEventPressAniNameAttr = "aniName";
static const std::string s_sEventPressAreaXAttr = "areaX";
static const std::string s_sEventPressAreaYAttr = "areaY";
static const std::string s_sEventPressAreaWAttr = "areaW";
static const std::string s_sEventPressAreaHAttr = "areaH";

XmlPressEventParser::XmlPressEventParser() noexcept
: XmlEventParser(s_sEventPressNodeName)
{
}

Event* XmlPressEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventPress(oCtx, p0Element);
}

Event* XmlPressEventParser::parseEventPress(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventPress" << std::endl;
	PressEvent::Init oInit;
	oCtx.addChecker(p0Element);
	Level& oLevel = oCtx.level();
	parseEventBase(oCtx, p0Element, oInit);

	const auto oPairAniName = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventPressAniNameAttr);
	if (!oPairAniName.first) {
		throw XmlCommonErrors::errorAttrNotFound(oCtx, p0Element, s_sEventPressAniNameAttr);
	}
	const std::string& sAniName = oPairAniName.second;
	oInit.m_nAniNameIdx = oCtx.named().tileAnis().addName(sAniName);
	assert(oInit.m_nAniNameIdx >= 0);
	//
	XmlBasicParser oXmlBasicParser(getXmlConditionalParser());
	oInit.m_oArea = oXmlBasicParser.parseNRect(oCtx, p0Element
												, s_sEventPressAreaXAttr, s_sEventPressAreaYAttr, s_sEventPressAreaWAttr, s_sEventPressAreaHAttr
												, true, NRect{0, 0, oLevel.boardWidth(), oLevel.boardHeight()}
												, NSize{1, 1});
	//
	oCtx.removeChecker(p0Element, true);
	auto refPressEvent = std::make_unique<PressEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refPressEvent), p0Element);
}

int32_t XmlPressEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "BUTTON_PRESS") {
		nMsg = PressEvent::MESSAGE_BUTTON_PRESS;
	} else if (sMsgName == "BUTTON_RELEASE") {
		nMsg = PressEvent::MESSAGE_BUTTON_RELEASE;
	} else if (sMsgName == "BUTTON_RELEASE_CANCEL") {
		nMsg = PressEvent::MESSAGE_BUTTON_RELEASE_CANCEL;
	} else if (sMsgName == "BUTTON_MOVE") {
		nMsg = PressEvent::MESSAGE_BUTTON_MOVE;
	} else {
		nMsg = XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlPressEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
														, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "RELEASE") {
		nListenerGroup = PressEvent::LISTENER_GROUP_RELEASE;
	} else {
		nListenerGroup = XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg
