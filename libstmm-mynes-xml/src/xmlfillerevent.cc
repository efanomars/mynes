/*
 * File:   xmlfillerevent.cc
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

#include "xmlfillerevent.h"

#include <stmm-mynes/fillerevent.h>

#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlcommonerrors.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmltraitsparser.h>
#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>
#include <stmm-games-xml-game/xmlutile/xmlprobtilegenparser.h>

#include <stmm-games/level.h>
#include <stmm-games/util/coords.h>

//#include <cassert>
//#include <iostream>
#include <vector>
#include <utility>

namespace stmg
{

const std::string XmlFillerEventParser::s_sEventFillerNodeName = "FillerEvent";
static const std::string s_sEventFillerFillGenTotTilesAttr = "totTiles";
static const std::string s_sEventFillerFillGenFactorAttr = "availablePositionsFactor";
static const std::string s_sEventFillerModeAttr = "mode";
static const std::string s_sEventFillerModeAttrReplace = "REPLACE";
static const std::string s_sEventFillerModeAttrOver = "OVER";
static const std::string s_sEventFillerModeAttrUnder = "UNDER";
static const std::string s_sEventFillerModeAttrIgnore = "IGNORE";
static const std::string s_sEventFillerAreaNodeName = "Rect";
static const std::string s_sEventFillerAreaXAttr = "x";
static const std::string s_sEventFillerAreaYAttr = "y";
static const std::string s_sEventFillerAreaWAttr = "w";
static const std::string s_sEventFillerAreaHAttr = "h";
static const std::string s_sEventFillerSingleNodeName = "Single";
static const std::string s_sEventFillerSingleXAttr = "x";
static const std::string s_sEventFillerSingleYAttr = "y";
static const std::string s_sEventFillerSelectNodeName = "Select";
static const std::string s_sEventFillerRandomTileNodeName = "RandomTile";

XmlFillerEventParser::XmlFillerEventParser() noexcept
: XmlEventParser(s_sEventFillerNodeName)
{
}

Event* XmlFillerEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventFiller(oCtx, p0Element);
}

Event* XmlFillerEventParser::parseEventFiller(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventFiller" << std::endl;
	FillerEvent::Init oInit;
	oCtx.addChecker(p0Element);
	parseEventBase(oCtx, p0Element, oInit);

	const xmlpp::Element* p0SelectElement = getXmlConditionalParser().parseUniqueElement(oCtx, p0Element, s_sEventFillerSelectNodeName, false);
	if (p0SelectElement != nullptr) {
		oInit.m_refTargetTiles = getXmlTraitsParser().parseTileSelectorAnd(oCtx, p0SelectElement);
	}

	XmlProbTileGenParser oXmlProbTileGenParser{s_sEventFillerRandomTileNodeName, getXmlConditionalParser(), getXmlTraitsParser()};
	oInit.m_oFillGen.m_oProbTileGen = std::move(oXmlProbTileGenParser.parseProbTileGen(oCtx, p0Element));

	const auto oPairTotTiles = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventFillerFillGenTotTilesAttr);
	if (oPairTotTiles.first) {
		oInit.m_oFillGen.m_nTotTiles = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventFillerFillGenTotTilesAttr, oPairTotTiles.second, false
																	, false, -1, false, -1);
	}

	const auto oPairFactor = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventFillerFillGenFactorAttr);
	if (oPairFactor.first) {
		oInit.m_oFillGen.m_fFactorOfAvailablePositions = XmlUtil::strToNumber<double>(oCtx, p0Element, s_sEventFillerFillGenFactorAttr, oPairFactor.second, true
																					, true, 0.0, true, 1.0);
	}

	static std::vector<char const *> s_aModeEnumString{s_sEventFillerModeAttrReplace.c_str(), s_sEventFillerModeAttrOver.c_str(), s_sEventFillerModeAttrUnder.c_str(), s_sEventFillerModeAttrIgnore.c_str()};
	static const std::vector<FillerEvent::FILL_MODE> s_aModeEnumValue{FillerEvent::FILL_MODE_REPLACE, FillerEvent::FILL_MODE_OVER, FillerEvent::FILL_MODE_UNDER, FillerEvent::FILL_MODE_IGNORE};
	//
	const int32_t nMode = getXmlConditionalParser().getEnumAttributeValue(oCtx, p0Element, s_sEventFillerModeAttr, s_aModeEnumString);
	if (nMode >= 0) {
		oInit.m_oFillGen.m_eFillMode = s_aModeEnumValue[nMode];
	}

	getXmlConditionalParser().visitElementChildren(oCtx, p0Element, [&](const xmlpp::Element* p0Filler)
	{
		const std::string sFillerName = p0Filler->get_name();
//std::cout << "sFillerName=" << sFillerName << '\n';
		if (sFillerName == s_sEventFillerSingleNodeName) {
			parseFillerSingle(oCtx, p0Filler, oInit.m_oCoords);
		} else if (sFillerName == s_sEventFillerAreaNodeName) {
			parseFillerRect(oCtx, p0Filler, oInit.m_oCoords);
		} else if (sFillerName == s_sEventFillerSelectNodeName) {
			//
		} else if (sFillerName == s_sEventFillerRandomTileNodeName) {
			//
		} else if (isReservedChildElementOfEvent(sFillerName)) {
			//
		} else {
			throw XmlCommonErrors::errorElementInvalid(oCtx, p0Element, sFillerName);
		}
		oCtx.addValidChildElementName(p0Element, sFillerName);
	});
	// Child elements already checked in visitElementChildren
	oCtx.removeChecker(p0Element, true);

	auto refFillerEvent = std::make_unique<FillerEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refFillerEvent), p0Element);
}
void XmlFillerEventParser::parseFillerSingle(GameCtx& oCtx, const xmlpp::Element* p0Element, Coords& oCoords)
{
	oCtx.addChecker(p0Element);
	//
	const auto oPairSingleX = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventFillerSingleXAttr);
	int32_t nX = 0;
	if (oPairSingleX.first) {
		nX = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventFillerSingleXAttr, oPairSingleX.second, false
										, true, 0, true, oCtx.level().boardWidth() - 1);
	} else {
		throw XmlCommonErrors::errorAttrNotFound(oCtx, p0Element, s_sEventFillerSingleXAttr);
	}
	//
	const auto oPairSingleY = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventFillerSingleYAttr);
	int32_t nY = 0;
	if (oPairSingleY.first) {
		nY = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventFillerSingleYAttr, oPairSingleY.second, false
										, true, 0, true, oCtx.level().boardHeight() - 1);
	} else {
		throw XmlCommonErrors::errorAttrNotFound(oCtx, p0Element, s_sEventFillerSingleYAttr);
	}
	oCoords.add(nX, nY);
	oCtx.removeChecker(p0Element, true);
}
void XmlFillerEventParser::parseFillerRect(GameCtx& oCtx, const xmlpp::Element* p0Element, Coords& oCoords)
{
	oCtx.addChecker(p0Element);
	//
	XmlBasicParser oXmlBasicParser(getXmlConditionalParser());
	const NRect oArea = oXmlBasicParser.parseNRect(oCtx, p0Element
													, s_sEventFillerAreaXAttr, s_sEventFillerAreaYAttr, s_sEventFillerAreaWAttr, s_sEventFillerAreaHAttr
													, true, NRect{0, 0, oCtx.level().boardWidth(), oCtx.level().boardHeight()}
													, NSize{1, 1});
	//
	oCoords.addRect(oArea);
	oCtx.removeChecker(p0Element, true);
}
int32_t XmlFillerEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "FILL") {
		nMsg = FillerEvent::MESSAGE_FILL;
	} else if (sMsgName == "FILL_JUST_POS") {
		nMsg = FillerEvent::MESSAGE_FILL_JUST_POS;
	} else if (sMsgName == "FILL_BUT_POS") {
		nMsg = FillerEvent::MESSAGE_FILL_BUT_POS;
	} else {
		nMsg = XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlFillerEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
														, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "FILLED") {
		nListenerGroup = FillerEvent::LISTENER_GROUP_FILLED;
	} else {
		nListenerGroup = XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg
