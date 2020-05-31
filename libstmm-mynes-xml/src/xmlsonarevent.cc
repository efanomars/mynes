/*
 * File:   xmlsonarsevent.cc
 *
 * Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
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

#include "xmlsonarevent.h"

#include <stmm-mynes/sonarevent.h>

#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmltraitsparser.h>
#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>
#include <stmm-games-xml-base/xmlutil/xmlstrconv.h>

#include <stmm-games/level.h>

//#include <cassert>
//#include <iostream>

namespace stmg
{

const std::string XmlSonarEventParser::s_sEventSonarNodeName = "SonarEvent";
static const std::string s_sEventSonarBlockAttr = "block";
static const std::string s_sEventSonarShapeAttr = "shape";
static const std::string s_sEventSonarRelPosXAttr = "relPosX";
static const std::string s_sEventSonarRelPosYAttr = "relPosY";
static const std::string s_sEventSonarSonarNodeName = "Sonar";
static const std::string s_sEventSonarAreaXAttr = "areaX";
static const std::string s_sEventSonarAreaYAttr = "areaY";
static const std::string s_sEventSonarAreaWAttr = "areaW";
static const std::string s_sEventSonarAreaHAttr = "areaH";
static const std::string s_sEventSonarBlockRotateTicksAttr = "blockRotateTicks";
static const std::string s_sEventSonarBlockRotateTicksFromAttr = "blockRotateTicksFrom";
static const std::string s_sEventSonarBlockRotateTicksToAttr = "blockRotateTicksTo";
static const std::string s_sEventSonarBlockRotateMillisecAttr = "blockRotateMillisec";
static const std::string s_sEventSonarBlockRotateMillisecFromAttr = "blockRotateMillisecFrom";
static const std::string s_sEventSonarBlockRotateMillisecToAttr = "blockRotateMillisecTo";
static const std::string s_sEventSonarLaunchesAttr = "launches";
static const std::string s_sEventSonarDirRotateTicksAttr = "dirRotateTicks";
static const std::string s_sEventSonarDirRotateTicksFromAttr = "dirRotateTicksFrom";
static const std::string s_sEventSonarDirRotateTicksToAttr = "dirRotateTicksTo";
static const std::string s_sEventSonarDirRotateMillisecAttr = "dirRotateMillisec";
static const std::string s_sEventSonarDirRotateMillisecFromAttr = "dirRotateMillisecFrom";
static const std::string s_sEventSonarDirRotateMillisecToAttr = "dirRotateMillisecTo";
static const std::string s_sEventSonarAutoCreatedAttr = "auto";

static const constexpr int32_t s_nDefaultSonarBlockRotationMillisec = 1000;
static const constexpr int32_t s_nDefaultSonarDirRotationMillisec = 170;

XmlSonarEventParser::XmlSonarEventParser() noexcept
: XmlEventParser(s_sEventSonarNodeName)
{
}

Event* XmlSonarEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventSonar(oCtx, p0Element);
}

void XmlSonarEventParser::parseNTimeRange(GameCtx& oCtx, const xmlpp::Element* p0Element, NTimeRange& oInterval, int32_t nDefaultMillisec
										, const std::string& sTicksAttrName, const std::string& sTicksFromAttrName, const std::string& sTicksToAttrName
										, const std::string& sMillisecAttrName, const std::string& sMillisecFromAttrName, const std::string& sMillisecToAttrName)
{
	const bool bRotateTicks = getXmlConditionalParser().parseAttributeFromTo<int32_t>(oCtx, p0Element
											, sTicksAttrName, sTicksFromAttrName, sTicksToAttrName
											, false, true, 0, false, -1, oInterval.m_oTicks.m_nFrom, oInterval.m_oTicks.m_nTo);
	const bool bRotateMillisec = getXmlConditionalParser().parseAttributeFromTo<int32_t>(oCtx, p0Element
											, sMillisecAttrName, sMillisecFromAttrName, sMillisecToAttrName
											, false, true, 0, false, -1, oInterval.m_oMillisec.m_nFrom, oInterval.m_oMillisec.m_nTo);
	if ((!bRotateTicks) && (!bRotateMillisec)) {
		oInterval.m_oTicks.m_nFrom = 0;
		oInterval.m_oTicks.m_nTo = 0;
		oInterval.m_oMillisec.m_nFrom = nDefaultMillisec;
		oInterval.m_oMillisec.m_nTo = nDefaultMillisec;
	}
}
Event* XmlSonarEventParser::parseEventSonar(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventSonar" << std::endl;
	SonarEvent::Init oInit;
	oCtx.addChecker(p0Element);
	Level& oLevel = oCtx.level();
	parseEventBase(oCtx, p0Element, oInit);

	const auto oPairBlock = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSonarBlockAttr);
	if (!oPairBlock.first) {
		throw XmlCommonErrors::errorAttrNotFound(oCtx, p0Element, s_sEventSonarBlockAttr);
	}
	const std::string& sBlock = oPairBlock.second;
	const bool bFound = getBlock(oCtx, sBlock, oInit.m_oBlock);
	if (!bFound) {
		throw XmlCommonErrors::error(oCtx, p0Element, s_sEventSonarBlockAttr
									, Util::stringCompose("Attribute '%1': Block '%2' not defined", s_sEventSonarBlockAttr, sBlock));
	}
	const int32_t nTotRemovedShapes = oInit.m_oBlock.shapeRemoveAllInvisible();
	if (nTotRemovedShapes > 0) {
		throw XmlCommonErrors::error(oCtx, p0Element, s_sEventSonarBlockAttr
									, Util::stringCompose("Attribute '%1': Block '%2' cannot contain empty shapes", s_sEventSonarBlockAttr, sBlock));
	}
	if (oInit.m_oBlock.isEmpty()) {
		throw XmlCommonErrors::error(oCtx, p0Element, s_sEventSonarBlockAttr
									, Util::stringCompose("Attribute '%1': Block cannot be empty", s_sEventSonarBlockAttr));
	}
	//
	const auto oPairShape = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSonarShapeAttr);
	if (oPairShape.first) {
		const std::string& sShape = oPairShape.second;
		oInit.m_nInitialShape = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSonarShapeAttr, sShape, false
																	, true, 0, true, oInit.m_oBlock.totShapes() - 1);
	}
	//
	oInit.m_oRelPos.m_nX = - oInit.m_oBlock.maxWidth() / 2;
	const auto oPairRelPosX = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSonarRelPosXAttr);
	if (oPairRelPosX.first) {
		const std::string& sRelPosX = oPairRelPosX.second;
		oInit.m_oRelPos.m_nX = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSonarRelPosXAttr, sRelPosX, false
																	, false, -1, false, -1);
	}
	oInit.m_oRelPos.m_nY = - oInit.m_oBlock.maxHeight() / 2;
	const auto oPairRelPosY = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSonarRelPosYAttr);
	if (oPairRelPosY.first) {
		const std::string& sRelPosY = oPairRelPosY.second;
		oInit.m_oRelPos.m_nY = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSonarRelPosYAttr, sRelPosY, false
																	, false, -1, false, -1);
	}
	//
	parseNTimeRange(oCtx, p0Element, oInit.m_oBlockRotateInterval, s_nDefaultSonarBlockRotationMillisec
					, s_sEventSonarBlockRotateTicksAttr, s_sEventSonarBlockRotateTicksFromAttr, s_sEventSonarBlockRotateTicksToAttr
					, s_sEventSonarBlockRotateMillisecAttr, s_sEventSonarBlockRotateMillisecFromAttr, s_sEventSonarBlockRotateMillisecToAttr);
	//
	parseNTimeRange(oCtx, p0Element, oInit.m_oDirRotateInterval, s_nDefaultSonarDirRotationMillisec
					, s_sEventSonarDirRotateTicksAttr, s_sEventSonarDirRotateTicksFromAttr, s_sEventSonarDirRotateTicksToAttr
					, s_sEventSonarDirRotateMillisecAttr, s_sEventSonarDirRotateMillisecFromAttr, s_sEventSonarDirRotateMillisecToAttr);
	//
	const auto oPairLaunches = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSonarLaunchesAttr);
	if (oPairLaunches.first) {
		const std::string& sLaunches = oPairLaunches.second;
		oInit.m_bLaunches = XmlUtil::strToBool(oCtx, p0Element, s_sEventSonarLaunchesAttr, sLaunches);
	}
	//
	const auto oPairAutoCreated = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSonarAutoCreatedAttr);
	if (oPairAutoCreated.first) {
		const std::string& sAutoCreated = oPairAutoCreated.second;
		oInit.m_bAutoCreated = XmlUtil::strToBool(oCtx, p0Element, s_sEventSonarAutoCreatedAttr, sAutoCreated);
	}
	//
	XmlBasicParser oXmlBasicParser(getXmlConditionalParser());
	oInit.m_oArea = oXmlBasicParser.parseNRect(oCtx, p0Element
												, s_sEventSonarAreaXAttr, s_sEventSonarAreaYAttr, s_sEventSonarAreaWAttr, s_sEventSonarAreaHAttr
												, true, NRect{0, 0, oLevel.boardWidth(), oLevel.boardHeight()}
												, NSize{1, 1});
	//
	const xmlpp::Element* p0SonarElement = getXmlConditionalParser().parseUniqueElement(oCtx, p0Element, s_sEventSonarSonarNodeName, true);
	oInit.m_refSonarTile = getXmlTraitsParser().parseTileSelectorAnd(oCtx, p0SonarElement);
	//
	oCtx.removeChecker(p0Element, true);
	auto refSonarEvent = std::make_unique<SonarEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refSonarEvent), p0Element);
}

int32_t XmlSonarEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "ACTIVATE_SONAR_AT") {
		nMsg = SonarEvent::MESSAGE_ACTIVATE_SONAR_AT;
	} else {
		nMsg = XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlSonarEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
														, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "SONAR_CREATED") {
		nListenerGroup = SonarEvent::LISTENER_GROUP_SONAR_CREATED;
	} else if (sListenerGroupName == "SONAR_LAUNCHED") {
		nListenerGroup = SonarEvent::LISTENER_GROUP_SONAR_LAUNCHED;
	} else if (sListenerGroupName == "SONAR_REMOVED") {
		nListenerGroup = SonarEvent::LISTENER_GROUP_SONAR_REMOVED;
	} else {
		nListenerGroup = XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg
