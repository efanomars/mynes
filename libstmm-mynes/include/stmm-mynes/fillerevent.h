/*
 * File:   fillerevent.h
 *
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

#ifndef STMG_FILLER_EVENT_H_
#define STMG_FILLER_EVENT_H_

#include <stmm-games/event.h>
#include <stmm-games/util/recycler.h>
#include <stmm-games/utile/randomtiles.h>
#include <stmm-games/utile/tileselector.h>
#include <stmm-games/utile/tilecoords.h>
#include <stmm-games/utile/tilebuffer.h>

#include <vector>
#include <memory>

#include <stdint.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

class FillerEvent : public Event
{
public:
	enum FILL_MODE
	{
		FILL_MODE_REPLACE = 0 /**< The generated tile replaces the board tile. */
		, FILL_MODE_OVER = 1 /**< The non empty traits of the generated tile replace the traits of the board tile. */
		, FILL_MODE_UNDER = 2 /**< The non empty traits of the generated tile replace the traits of the board tile only if they are empty. */
		, FILL_MODE_IGNORE = 3 /**< The board tile is left unchanged. Useful when this class is used to count tiles. */
	};
	struct FillGen
	{
		RandomTiles::ProbTileGen m_oProbTileGen; /**< The tile generator. */
		FILL_MODE m_eFillMode = FILL_MODE_REPLACE; /**< The fill mode. Default is FILL_MODE_REPLACE. */
		int32_t m_nTotTiles = 1; /**< The number of tiles to fill. Is added to the number of tiles calculated
									* with m_fFactorOfAvailablePositions. Can be negative. */
		double m_fFactorOfAvailablePositions = 0.0; /**< Factor of tiles selected by m_refTargetTiles that should be filled.
													 * m_nTotTiles is added on top of those.
													 * Must be &gt;= 0.0 and &lt;= 1.0. Default is 0.0. */
	};
	struct LocalInit
	{
		Coords m_oCoords; /**< The positions within the board into which tiles are set randomly. 
							 * If empty entire board is used. */
		FillGen m_oFillGen; /**< The fill generator. */
		unique_ptr<TileSelector> m_refTargetTiles; /**< Which tiles should be overwritten. If null only empty tiles are filled. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * If m_oFillGen.m_eFillMode is FILL_MODE_IGNORE m_oFillGen.m_oProbTileGen is ignored,
	 * otherwise at least one its trait set must contain a non empty value.
	 * @param oInit The initialization data.
	 */
	explicit FillerEvent(Init&& oInit) noexcept;

protected:
	/** Reinitialization.
	 * @param oInit The initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:

	//Event
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;

	// input
	enum {
		MESSAGE_FILL = 100 /**< Fill the rect. nValue contains the number of tiles to fill.
							 * If 0 calculated by Init::m_nTotTiles and Init::m_fFactorOfAvailablePositions.*/
		, MESSAGE_FILL_JUST_POS = 101 /**< Fill the position passed as nValue if it is within rect. */
		, MESSAGE_FILL_BUT_POS = 102 /**< Fill the rect except for the position in nValue packed with Util::packPointToInt32(x,y). */
	};
	// output
	enum {
		LISTENER_GROUP_FILLED = 10 /**< Fill happend with nValue containing the number of filled tiles. */
	};

private:
	void commonInit(LocalInit&& oInit) noexcept;
	void initAllowedPositions(const Coords& oCoords) noexcept;
	void resetRuntime() noexcept;

	int32_t fillSelectedPositions(int32_t nMaxToFill, bool bNot, const NPoint& oXY) noexcept;

private:
	int32_t m_nTileAniRemovingIndex;

	enum FILLER_STATE
	{
		FILLER_STATE_ACTIVATE = 0,
		FILLER_STATE_INIT = 1,
		FILLER_STATE_RUN = 2
	};
	FILLER_STATE m_eState;

	unique_ptr<RandomTiles> m_refRandomTiles;
	FILL_MODE m_eFillMode;
	int32_t m_nTotTiles; // can be 0 or negative
	double m_fFactorOfAvailablePositions; // 0.0 to 1.0
	//
	unique_ptr<TileSelector> m_refTargetTiles;

	int32_t m_nBoardW;
	int32_t m_nBoardH;

	int32_t m_nFillType;
	int32_t m_nFillParam;

	TileCoords m_oTileCoords;

	shared_ptr<TileBuffer> m_refCurTileBuf;
	Recycler<TileBuffer> m_oTileBufferRecycler;

	std::vector<NPoint> m_aAllowedPositions;
private:
	FillerEvent();
	FillerEvent(const FillerEvent& oSource);
	FillerEvent& operator=(const FillerEvent& oSource);
};

} // namespace stmg

#endif	/* STMG_FILLER_EVENT_H_ */

