/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file cargopacket.h Base class for cargo packets. */

#ifndef CARGOPACKET_H
#define CARGOPACKET_H

#include "core/pool_type.hpp"
#include "economy_type.h"
#include "station_type.h"
#include "cargo_type.h"
#include "vehicle_type.h"
#include "order_type.h"
#include "cargotype.h"
#include <list>
#include <map>

/** Unique identifier for a single cargo packet. */
typedef uint32 CargoPacketID;
struct CargoPacket;

/** Type of the pool for cargo packets for a little over 16 million packets. */
typedef Pool<CargoPacket, CargoPacketID, 1024, 0xFFF000, PT_NORMAL, true, false> CargoPacketPool;
/** The actual pool with cargo packets. */
extern CargoPacketPool _cargopacket_pool;

template <class Tinst> class CargoList;
extern const struct SaveLoad *GetCargoPacketDesc();

/**
 * Container for cargo from the same location and time.
 */
struct CargoPacket : CargoPacketPool::PoolItem<&_cargopacket_pool> {
private:
	Money feeder_share;         ///< Value of feeder pickup to be paid for on delivery of cargo.
	uint16 count;               ///< The amount of cargo in this packet.
	byte days_in_transit;       ///< Amount of days this packet has been in transit.
	SourceTypeByte source_type; ///< Type of \c source_id.
	SourceID source_id;         ///< Index of source, INVALID_SOURCE if unknown/invalid.
	StationID source;           ///< The station where the cargo came from first.
	TileIndex source_xy;        ///< The origin of the cargo (first station in feeder chain).
	TileIndex loaded_at_xy;     ///< Location where this cargo has been loaded into the vehicle.
	TileIndex dest_xy;          ///< Destination tile or INVALID_TILE if no specific destination
	SourceID dest_id;           ///< Index of the destination.
	SourceTypeByte dest_type;   ///< Type of #dest_id.
	byte flags;                 ///< Flags influencing the routing decision of this packet, see #RouteFlags.
	OrderID next_order;         ///< Next desired hop.
	StationID next_station;     ///< Unload at this station next.

	/** The CargoList caches, thus needs to know about it. */
	template <class Tinst> friend class CargoList;
	friend class VehicleCargoList;
	friend class StationCargoList;
	/** We want this to be saved, right? */
	friend const struct SaveLoad *GetCargoPacketDesc();
	friend bool CargodestModeChanged(int32 p1);
public:
	/** Maximum number of items in a single cargo packet. */
	static const uint16 MAX_COUNT = UINT16_MAX;

	CargoPacket();
	CargoPacket(StationID source, TileIndex source_xy, uint16 count, SourceType source_type, SourceID source_id, TileIndex dest_xy = INVALID_TILE, SourceType dest_type = ST_INDUSTRY, SourceID dest_id = INVALID_SOURCE, OrderID next_order = INVALID_ORDER, StationID next_station = INVALID_STATION, byte flags = 0);
	CargoPacket(uint16 count, byte days_in_transit, StationID source, TileIndex source_xy, TileIndex loaded_at_xy, Money feeder_share = 0, SourceType source_type = ST_INDUSTRY, SourceID source_id = INVALID_SOURCE, TileIndex dest_xy = INVALID_TILE, SourceType dest_type = ST_INDUSTRY, SourceID dest_id = INVALID_SOURCE, OrderID next_order = INVALID_ORDER, StationID next_station = INVALID_STATION, byte flags = 0);

	/** Destroy the packet. */
	~CargoPacket() { }

	CargoPacket *Split(uint new_size);
	void Merge(CargoPacket *cp);

	/**
	 * Gets the number of 'items' in this packet.
	 * @return Item count.
	 */
	FORCEINLINE uint16 Count() const
	{
		return this->count;
	}

	/**
	 * Gets the amount of money already paid to earlier vehicles in
	 * the feeder chain.
	 * @return Feeder share.
	 */
	FORCEINLINE Money FeederShare() const
	{
		return this->feeder_share;
	}

	/**
	 * Gets the number of days this cargo has been in transit.
	 * This number isn't really in days, but in 2.5 days (CARGO_AGING_TICKS = 185 ticks) and
	 * it is capped at 255.
	 * @return Length this cargo has been in transit.
	 */
	FORCEINLINE byte DaysInTransit() const
	{
		return this->days_in_transit;
	}

	/**
	 * Gets the type of the cargo's source. industry, town or head quarter.
	 * @return Source type.
	 */
	FORCEINLINE SourceType SourceSubsidyType() const
	{
		return this->source_type;
	}

	/**
	 * Gets the ID of the cargo's source. An IndustryID, TownID or CompanyID.
	 * @return Source ID.
	 */
	FORCEINLINE SourceID SourceSubsidyID() const
	{
		return this->source_id;
	}

	/**
	 * Gets the ID of the station where the cargo was loaded for the first time.
	 * @return StationID.
	 */
	FORCEINLINE SourceID SourceStation() const
	{
		return this->source;
	}

	/**
	 * Gets the coordinates of the cargo's source station.
	 * @return Source station's coordinates.
	 */
	FORCEINLINE TileIndex SourceStationXY() const
	{
		return this->source_xy;
	}

	/**
	 * Gets the coordinates of the cargo's last loading station.
	 * @return Last loading station's coordinates.
	 */
	FORCEINLINE TileIndex LoadedAtXY() const
	{
		return this->loaded_at_xy;
	}

	/**
	 * Gets the coordinates of the cargo's destination.
	 * @return The destination tile.
	 */
	FORCEINLINE TileIndex DestinationXY() const
	{
		return this->dest_xy;
	}

	/**
	 * Gets the ID of the destination of the cargo.
	 * @return The destination ID.
	 */
	FORCEINLINE SourceID DestinationID() const
	{
		return this->dest_id;
	}

	/**
	 * Gets the type of the destination of the cargo.
	 * @return The destination type.
	 */
	FORCEINLINE SourceType DestinationType() const
	{
		return this->dest_type;
	}

	/**
	 * Gets the routing behaviour flags of this packet.
	 * @return The routing flags.
	 */
	FORCEINLINE byte Flags() const
	{
		return this->flags;
	}

	/**
	 * Gets the order ID of the next desired hop.
	 * @return The order ID of the next desired hop.
	 */
	FORCEINLINE OrderID NextHop() const
	{
		return this->next_order;
	}

	/**
	 * Gets the station ID where the packet should be unloaded next.
	 * @return The station ID where the packet should be unloaded.
	 */
	FORCEINLINE StationID NextStation() const
	{
		return this->next_station;
	}

	static void InvalidateAllFrom(SourceType src_type, SourceID src);
	static void InvalidateAllFrom(StationID sid);
	static void AfterLoad();
};

/**
 * Iterate over all _valid_ cargo packets from the given start.
 * @param var   Variable used as "iterator".
 * @param start Cargo packet ID of the first packet to iterate over.
 */
#define FOR_ALL_CARGOPACKETS_FROM(var, start) FOR_ALL_ITEMS_FROM(CargoPacket, cargopacket_index, var, start)

/**
 * Iterate over all _valid_ cargo packets from the begin of the pool.
 * @param var   Variable used as "iterator".
 */
#define FOR_ALL_CARGOPACKETS(var) FOR_ALL_CARGOPACKETS_FROM(var, 0)

/**
 * Simple collection class for a list of cargo packets.
 * @tparam Tinst Actual instantation of this cargo list.
 */
template <class Tinst>
class CargoList {
public:
	/** Container with cargo packets. */
	typedef std::list<CargoPacket *> List;
	/** The iterator for our container. */
	typedef List::iterator Iterator;
	/** The const iterator for our container. */
	typedef List::const_iterator ConstIterator;

	/** Kind of actions that could be done with packets on move. */
	enum MoveToAction {
		MTA_FINAL_DELIVERY, ///< "Deliver" the packet to the final destination, i.e. destroy the packet.
		MTA_CARGO_LOAD,     ///< Load the packet onto a vehicle, i.e. set the last loaded station ID.
		MTA_TRANSFER,       ///< The cargo is moved as part of a transfer.
		MTA_UNLOAD,         ///< The cargo is moved as part of a forced unload.
		MTA_NO_ACTION,      ///< The station doesn't accept the cargo, so do nothing (only applicable to cargo without destination)
	};

	friend bool CargodestModeChanged(int32 p1);

protected:
	uint count;                 ///< Cache for the number of cargo entities.
	uint cargo_days_in_transit; ///< Cache for the sum of number of days in transit of each entity; comparable to man-hours.

	List packets;               ///< The cargo packets in this list.

	void AddToCache(const CargoPacket *cp);

	void RemoveFromCache(const CargoPacket *cp);

	void RemoveFromCacheLocal(const CargoPacket *cp, uint amount) {}

	virtual bool UpdateCargoNextHop(CargoPacket *cp, Station *st, CargoID cid)
	{
		return true;
	}

public:
	/** Create the cargo list. */
	CargoList() {}

	virtual ~CargoList();

	void OnCleanPool();

	/**
	 * Returns a pointer to the cargo packet list (so you can iterate over it etc).
	 * @return Pointer to the packet list.
	 */
	FORCEINLINE const List *Packets() const
	{
		return &this->packets;
	}

	/**
	 * Checks whether this list is empty.
	 * @return True if and only if the list is empty.
	 */
	FORCEINLINE bool Empty() const
	{
		return this->count == 0;
	}

	/**
	 * Returns the number of cargo entities in this list.
	 * @return The before mentioned number.
	 */
	FORCEINLINE uint Count() const
	{
		return this->count;
	}

	/**
	 * Returns source of the first cargo packet in this list.
	 * @return The before mentioned source.
	 */
	FORCEINLINE StationID Source() const
	{
		return this->Empty() ? INVALID_STATION : this->packets.front()->source;
	}

	/**
	 * Returns average number of days in transit for a cargo entity.
	 * @return The before mentioned number.
	 */
	FORCEINLINE uint DaysInTransit() const
	{
		return this->count == 0 ? 0 : this->cargo_days_in_transit / this->count;
	}


	void Append(CargoPacket *cp);
	void Truncate(uint max_remaining);

	template <class Tother_inst>
	bool MoveTo(Tother_inst *dest, uint count, MoveToAction mta, CargoPayment *payment, StationID st = INVALID_STATION, OrderID cur_order = INVALID_ORDER, CargoID cid = INVALID_CARGO, bool *did_transfer = NULL);

	void InvalidateCache();
};

/**
 * CargoList that is used for vehicles.
 */
class VehicleCargoList : public CargoList<VehicleCargoList> {
protected:
	/** The (direct) parent of this class. */
	typedef CargoList<VehicleCargoList> Parent;

	Money feeder_share; ///< Cache for the feeder share.

	void AddToCache(const CargoPacket *cp);
	void RemoveFromCache(const CargoPacket *cp);

public:
	/** The super class ought to know what it's doing. */
	friend class CargoList<VehicleCargoList>;
	/** The vehicles have a cargo list (and we want that saved). */
	friend const struct SaveLoad *GetVehicleDescription(VehicleType vt);

	/**
	 * Returns total sum of the feeder share for all packets.
	 * @return The before mentioned number.
	 */
	FORCEINLINE Money FeederShare() const
	{
		return this->feeder_share;
	}

	void AgeCargo();

	void InvalidateCache();

	void InvalidateNextStation();

	/**
	 * Are two the two CargoPackets mergeable in the context of
	 * a list of CargoPackets for a Vehicle?
	 * @param cp1 First CargoPacket.
	 * @param cp2 Second CargoPacket.
	 * @return True if they are mergeable.
	 */
	static bool AreMergable(const CargoPacket *cp1, const CargoPacket *cp2)
	{
		return cp1->source_xy    == cp2->source_xy &&
				cp1->days_in_transit == cp2->days_in_transit &&
				cp1->source_type     == cp2->source_type &&
				cp1->source_id       == cp2->source_id &&
				cp1->loaded_at_xy    == cp2->loaded_at_xy &&
				cp1->dest_xy         == cp2->dest_xy &&
				cp1->dest_type       == cp2->dest_type &&
				cp1->dest_id         == cp2->dest_id &&
				cp1->next_order      == cp2->next_order &&
				cp1->next_station    == cp2->next_station &&
				cp1->flags           == cp2->flags;
	}
};

/**
 * CargoList that is used for stations.
 */
class StationCargoList : public CargoList<StationCargoList> {
public:
	typedef std::map<OrderID, int> OrderMap;

protected:
	/** The (direct) parent of this class. */
	typedef CargoList<StationCargoList> Parent;

	OrderMap order_cache;
	uint32 next_start;        ///< Packet number to start the next hop update loop from.

	void AddToCache(const CargoPacket *cp);
	void RemoveFromCache(const CargoPacket *cp);
	void RemoveFromCacheLocal(const CargoPacket *cp, uint amount);

	/* virtual */ bool UpdateCargoNextHop(CargoPacket *cp, Station *st, CargoID cid);

public:
	/** The super class ought to know what it's doing. */
	friend class CargoList<StationCargoList>;
	/** The stations, via GoodsEntry, have a CargoList. */
	friend const struct SaveLoad *GetGoodsDesc();

	void InvalidateCache();

	void UpdateCargoNextHop(Station *st, CargoID cid);

	/**
	 * Gets the cargo counts per next hop.
	 * @return Cargo counts.
	 */
	const OrderMap& CountForNextHop() const
	{
		return this->order_cache;
	}

	/**
	 * Gets the cargo count for a next hop.
	 * @param order The next hop.
	 * @return The cargo count for the specified next hop.
	 */
	int CountForNextHop(OrderID order) const
	{
		OrderMap::const_iterator i = this->order_cache.find(order);
		return i != this->order_cache.end() ? i->second : 0;
	}

	/**
	 * Are two the two CargoPackets mergeable in the context of
	 * a list of CargoPackets for a Vehicle?
	 * @param cp1 First CargoPacket.
	 * @param cp2 Second CargoPacket.
	 * @return True if they are mergeable.
	 */
	static bool AreMergable(const CargoPacket *cp1, const CargoPacket *cp2)
	{
		return cp1->source_xy    == cp2->source_xy &&
				cp1->days_in_transit == cp2->days_in_transit &&
				cp1->source_type     == cp2->source_type &&
				cp1->source_id       == cp2->source_id &&
				cp1->dest_xy         == cp2->dest_xy &&
				cp1->dest_type       == cp2->dest_type &&
				cp1->dest_id         == cp2->dest_id &&
				cp1->next_order      == cp2->next_order &&
				cp1->next_station    == cp2->next_station &&
				cp1->flags           == cp2->flags;
	}

	static void InvalidateAllTo(OrderID order, StationID st_unload);
	static void InvalidateAllTo(SourceType type, SourceID dest);
};

#endif /* CARGOPACKET_H */
