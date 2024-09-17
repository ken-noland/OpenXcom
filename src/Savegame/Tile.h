#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <vector>
#include <memory>
#include "../Engine/Surface.h"
#include "../Battlescape/Position.h"
#include "../Mod/MapData.h"

namespace OpenXcom
{

class MapData;
class BattleUnit;
class BattleItem;
class RuleInventory;
class SavedBattleGame;
class ScriptParserBase;

enum LightLayers : uint8_t { LL_AMBIENT, LL_FIRE, LL_ITEMS, LL_UNITS, LL_MAX };

enum TileUnitOverlapping : int
{
	/// Any unit overlapping tile will be returned
	TUO_NORMAL = 24,
	/// Only units that overlap by 2 or more voxel will be returned
	TUO_IGNORE_SMALL = 26,
	/// Take unit from lower even if it not overlap
	TUO_ALWAYS = 0,
};

/**
 * Basic element of which a battle map is build.
 * @sa http://www.ufopaedia.org/index.php?title=MAPS
 */
class Tile
{
public:

	/// Name of class used in script.
	static constexpr const char *ScriptName = "Tile";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);

	typedef struct SerializationKey
	{

		uint8_t index; // for indexing the actual tile array
		uint8_t _mapDataSetID;
		uint8_t _mapDataID;
		uint8_t _smoke;
		uint8_t _fire;
		uint8_t boolFields;
		uint16_t _lastExploredByHostile;
		uint16_t _lastExploredByNeutral;
		uint16_t _lastExploredByPlayer;
		uint32_t totalBytes; // per structure, including any data not mentioned here and accounting for all array members!

		static const SerializationKey defaultKey();
	} SerializationKey;

	static const int NOT_CALCULATED = -1;

	/**
	 * Cache of ID for tile parts used to save and load.
	 */
	struct TileMapDataCache
	{
		int ID[O_MAX];
		int SetID[O_MAX];
	};
	/**
	 * Cached data that belongs to each tile object
	 */
	struct TileObjectCache
	{
		int8_t offsetY;
		uint8_t currentFrame:4;
		uint8_t discovered:1;
		uint8_t isUfoDoor:1;
		uint8_t isDoor:1;
		uint8_t isBackTileObject:1;
	};
	/**
	 * Cached data that belongs to whole tile
	 */
	struct TileCache
	{
		int8_t terrainLevel = 0;
		uint8_t isNoFloor:1;
		uint8_t isGravLift:1;
		uint8_t isLadderOnObject:1;
		uint8_t isLadderOnNorth:1;
		uint8_t isLadderOnWest:1;
		uint8_t bigWall:1;
		uint8_t danger:1;
	};

protected:
	SavedBattleGame* _save;
	MapData *_objects[O_MAX];
	BattleUnit *_unit = nullptr;
	std::vector<BattleItem *> _inventory;
	std::unique_ptr<TileMapDataCache> _mapData = std::make_unique<TileMapDataCache>();
	SurfaceRaw<const uint8_t> _currentSurface[O_MAX] = { };
	TileObjectCache _objectsCache[O_MAX] = { };
	TileCache _cache = { };
	Position _pos;
	uint8_t _light[LL_MAX];
	uint8_t _fire = 0;
	uint8_t _smoke = 0;
	uint8_t _markerColor = 0;
	uint8_t _animationOffset = 0;
	uint8_t _obstacle = 0;
	uint8_t _explosiveType = 0;
	int16_t _explosive = 0;
	int16_t _visible = 0;
	int16_t _TUMarker = -1;
	int16_t _EnergyMarker = -1;
	int8_t _preview = -1;
	uint8_t _overlaps = 0;
	int _lastExploredByPlayer = 0;
	int _lastExploredByHostile = 0;
	int _lastExploredByNeutral = 0;


public:
	/// Creates a tile.
	Tile(Position pos, SavedBattleGame* save);
	/// Copy constructor.
	Tile(Tile&&) = default;
	/// Cleans up a tile.
	~Tile();
	/// Load the tile from yaml
	void load(const YAML::Node &node);
	/// Load the tile from binary buffer in memory
	void loadBinary(uint8_t *buffer, Tile::SerializationKey& serializationKey);
	/// Saves the tile to yaml
	YAML::Node save() const;
	/// Saves the tile to binary
	void saveBinary(uint8_t** buffer) const;

	/**
	 * Get the MapData pointer of a part of the tile.
	 * @param part TilePart whose data is needed.
	 * @return pointer to mapdata
	 */
	MapData *getMapData(TilePart part) const
	{
		return _objects[part];
	}

	/**
	 * Get special tile type of floor part.
	 * @return Type of Tile.
	 */
	SpecialTileType getFloorSpecialTileType() const
	{
		return _objects[O_FLOOR] ? _objects[O_FLOOR]->getSpecialType() : TILE;
	}

	/**
	 * Get special tile type of object part.
	 * @return Type of Tile.
	 */
	SpecialTileType getObjectSpecialTileType() const
	{
		return _objects[O_OBJECT] ? _objects[O_OBJECT]->getSpecialType() : TILE;
	}

	/// Get saved battle game that tile belongs.
	const SavedBattleGame* getSavedGame() const { return _save; }
	/// Get saved battle game that tile belongs.
	SavedBattleGame* getSavedGame() { return _save; }


	/// Sets the pointer to the mapdata for a specific part of the tile
	void setMapData(MapData *dat, int mapDataID, int mapDataSetID, TilePart part);
	/// Gets the IDs to the mapdata for a specific part of the tile
	void getMapData(int *mapDataID, int *mapDataSetID, TilePart part) const;
	/// Gets whether this tile has no objects
	bool isVoid() const;
	/// Get the TU cost to walk over a certain part of the tile.
	int getTUCost(int part, MovementType movementType) const;
	/// Checks if this tile has a floor.
	bool hasNoFloor(const SavedBattleGame *savedBattleGame = nullptr) const;
	/// Checks if this tile has a GravLift floor.
	bool hasGravLiftFloor() const { return _cache.isGravLift; }
	/// Check if this tile has a Ladder (similar to GravLift but on wall).
	bool hasLadder() const { return _cache.isLadderOnObject || _cache.isLadderOnNorth || _cache.isLadderOnWest; }
	/// Check if this tile object part has a Ladder (similar to GravLift but on wall).
	bool hasLadderOnObject() const { return _cache.isLadderOnObject; }
	/// Check if this tile wall part has a Ladder (similar to GravLift but on wall).
	bool hasLadderOnNorthWall() const { return _cache.isLadderOnNorth; }
	/// Check if this tile wall part has a Ladder (similar to GravLift but on wall).
	bool hasLadderOnWestWall() const { return _cache.isLadderOnWest; }

	/**
	 * Whether this tile has a big wall.
	 * @return bool
	 */
	bool isBigWall() const
	{
		return _cache.bigWall;
	}

	/**
	 * Gets the height of the terrain (dirt/stairs/etc.) on this tile.
	 * @return the height in voxels (more negative values are higher, e.g. -8 = lower stairs, -16 = higher stairs)
	 */
	int getTerrainLevel() const
	{
		return _cache.terrainLevel;
	}

	/**
	 * Gets the tile's position.
	 * @return position
	 */
	Position getPosition() const
	{
		return _pos;
	}

	/// Gets the floor object footstep sound.
	int getFootstepSound(Tile *tileBelow) const;
	/// Open a door, returns the ID, 0(normal), 1(ufo) or -1 if no door opened.
	int openDoor(TilePart part, BattleUnit *unit = 0, BattleActionType reserve = BA_NONE, bool rClick = false);

	/**
	 * Check if the ufo door is open or opening. Used for visibility/light blocking checks.
	 * This function assumes that there never are 2 doors on 1 tile or a door and another wall on 1 tile.
	 * @param part Tile part to look for door
	 * @return bool
	 */
	bool isUfoDoorOpen(TilePart tp) const
	{
		return (_objectsCache[tp].isUfoDoor && _objectsCache[tp].currentFrame);
	}

	/**
	 * Check if part is ufo door.
	 * @param tp Part to check
	 * @return True if part is ufo door.
	 */
	bool isUfoDoor(TilePart tp) const
	{
		return _objectsCache[tp].isUfoDoor;
	}

	/**
	 * Check if part is door.
	 * @param tp Part to check
	 * @return True if part is door.
	 */
	bool isDoor(TilePart tp) const
	{
		return _objectsCache[tp].isDoor;
	}

	/**
	 * Check if an object should be drawn behind or in frontValue of a unit.
	 * @param tp Part to check
	 * @return True if its back object.
	 */
	bool isBackTileObject(TilePart tp) const
	{
		return _objectsCache[tp].isBackTileObject;
	}

	/**
	 * Gets surface Y offset.
	 * @param tp Part for offset.
	 * @return Offset value.
	 */
	int getYOffset(TilePart tp) const
	{
		return _objectsCache[tp].offsetY;
	}

	/// Close ufo door.
	int closeUfoDoor();
	/// Sets the black fog of war status of this tile.
	void setDiscovered(bool flag, TilePart part);
	/// Refreshes the exploration-turn of this tile to the current turn for the faction given.
	void setLastExplored(UnitFaction faction);
	/// Returns when the given faction has last explored this tile.
	int getLastExplored(UnitFaction faction);
	/// Gets the black fog of war status of this tile.
	bool isDiscovered(TilePart part) const;
	/// Reset light to zero for this tile.
	void resetLight(LightLayers layer);
	/// Reset light to zero for this tile and multiple layers.
	void resetLightMulti(LightLayers layer);
	/// Add light to this tile.
	void addLight(int light, LightLayers layer);
	/// Get max light to this tile.
	int getLight(LightLayers layer) const;
	/// Get max light to this tile and multiple layers.
	int getLightMulti(LightLayers layer) const;
	/// Get the shade amount.
	int getShade() const;
	/// Destroy a tile part.
	bool destroy(TilePart part, SpecialTileType type);
	/// Damage a tile part.
	bool damage(TilePart part, int power, SpecialTileType type);
	/// Set a "virtual" explosive on this tile, to detonate later.
	void setExplosive(int power, int damageType, bool force = false);
	/// Get explosive power of this tile.
	int getExplosive() const;
	/// Get explosive power of this tile.
	int getExplosiveType() const;
	/// Animated the tile parts.
	void animate();
	/// Update cached value of sprite.
	void updateSprite(TilePart part);
	/// Get object sprites.
	SurfaceRaw<const uint8_t> getSprite(TilePart part) const
	{
		return _currentSurface[part];
	}

	/**
	 * Set a unit on this tile.
	 */
	void setUnit(BattleUnit *unit)
	{
		_unit = unit;
	}

	/**
	 * Get the (alive) unit on this tile.
	 * @return BattleUnit.
	 */
	BattleUnit *getUnit() const
	{
		return _unit;
	}

	/// Get unit from this tile or from tile below.
	BattleUnit *getOverlappingUnit(const SavedBattleGame *saveBattleGame, TileUnitOverlapping range = TUO_NORMAL) const;
	/// Set fire, does not increment overlaps.
	void setFire(int fire);
	/// Get fire.
	int getFire() const;
	/// Add smoke, increments overlap.
	void addSmoke(int smoke);
	/// Set smoke, does not increment overlaps.
	void setSmoke(int smoke);
	/// Get smoke.
	int getSmoke() const;
	/// Get flammability.
	int getFlammability() const;
	/// Get turns to burn
	int getFuel() const;
	/// Get flammability of part.
	int getFlammability(TilePart part) const;
	/// Get turns to burn of part
	int getFuel(TilePart part) const;
	/// attempt to set the tile on fire, sets overlaps to one if successful.
	void ignite(int power);
	/// Get fire and smoke animation offset.
	int getAnimationOffset() const;
	/// Add item
	void addItem(BattleItem *item, const RuleInventory *ground);
	/// Remove item
	void removeItem(BattleItem *item);
	/// Get top-most item
	BattleItem* getTopItem();
	/// New turn preparations.
	void prepareNewTurn(bool smokeDamage);
	/// Get inventory on this tile.
	std::vector<BattleItem *>& getInventory();
	/// Set the tile marker color.
	void setMarkerColor(int color);
	/// Get the tile marker color.
	int getMarkerColor() const;
	/// Set the tile visible flag.
	void setVisible(int visibility);
	/// Get the tile visible flag.
	int getVisible() const;
	/// set the direction (used for path previewing)
	void setPreview(int dir);
	/// retrieve the direction stored by the pathfinding.
	int getPreview() const;
	/// set the number to be displayed for pathfinding preview.
	void setTUMarker(int tu);
	/// get the number to be displayed for pathfinding preview.
	int getTUMarker() const;
    /// set the number to be displayed for pathfinding preview.
    void setEnergyMarker(int tu);
    /// get the number to be displayed for pathfinding preview.
    int getEnergyMarker() const;
	/// how many times has this tile been overlapped with smoke/fire (runtime only)
	int getOverlaps() const;
	/// increment the overlap value on this tile.
	void addOverlap();
	/// set the danger flag on this tile (so the AI will avoid it).
	void setDangerous(bool danger);
	/// check the danger flag on this tile.
	bool getDangerous() const;

	/// sets single obstacle flag.
	void setObstacle(int part);
	/// gets single obstacle flag.
	bool getObstacle(int part) const
	{
		return _obstacle & (1 << part);
	}
	/// does the tile have obstacle flag set for at least one part?
	bool isObstacle(void) const
	{
		return _obstacle != 0;
	}
	/// reset obstacle flags
	void resetObstacle(void);
};

}
