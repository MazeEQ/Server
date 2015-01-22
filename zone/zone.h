/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef ZONE_H
#define ZONE_H

#include "../common/eqtime.h"
#include "../common/linked_list.h"
#include "../common/rulesys.h"
#include "../common/types.h"
#include "../common/random.h"
#include "qglobals.h"
#include "spawn2.h"
#include "spawngroup.h"

struct ZonePoint
{
	float x;
	float y;
	float z;
	float heading;
	uint16 number;
	float target_x;
	float target_y;
	float target_z;
	float target_heading;
	uint16 target_zone_id;
	int32 target_zone_instance;
	uint32 client_version_mask;
};
struct ZoneClientAuth_Struct {
	uint32	ip;			// client's IP address
	uint32	wid;		// client's WorldID#
	uint32	accid;
	int16	admin;
	uint32	charid;
	bool	tellsoff;
	char	charname[64];
	char	lskey[30];
	bool	stale;
};

struct ZoneEXPModInfo {
	float ExpMod;
	float AAExpMod;
};

struct item_tick_struct {
    uint32       itemid;
    uint32       chance;
    uint32       level;
    int16        bagslot;
    std::string qglobal;
};

class Client;
class Map;
class Mob;
class PathManager;
class WaterMap;
extern EntityList entity_list;
struct NPCType;
struct ServerZoneIncommingClient_Struct;

class Zone
{
public:
	static bool Bootup(uint32 iZoneID, uint32 iInstanceID, bool iStaticZone = false);
	static void Shutdown(bool quite = false);

	Zone(uint32 in_zoneid, uint32 in_instanceid, const char* in_short_name);
	~Zone();
	bool	Init(bool iStaticZone);
	bool	LoadZoneCFG(const char* filename, uint16 instance_id, bool DontLoadDefault = false);
	bool	SaveZoneCFG();
	bool	IsLoaded();
	bool	IsPVPZone() { return pvpzone; }
	inline const char*	GetLongName()	{ return long_name; }
	inline const char*	GetFileName()	{ return file_name; }
	inline const char*	GetShortName()	{ return short_name; }
	inline const uint32	GetZoneID() const { return zoneid; }
	inline const uint32	GetInstanceID() const { return instanceid; }
	inline const uint16	GetInstanceVersion() const { return instanceversion; }
	inline const bool IsInstancePersistent() const { return pers_instance; }
	inline const uint8	GetZoneType() const { return zone_type; }

	inline Timer* GetInstanceTimer() { return Instance_Timer; }

    inline xyz_location GetSafePoint() { return m_SafePoint; }
	inline const uint32& graveyard_zoneid()	{ return pgraveyard_zoneid; }
	inline xyz_heading GetGraveyardPoint() { return m_Graveyard; }
	inline const uint32& graveyard_id()	{ return pgraveyard_id; }

	inline const uint32& GetMaxClients() { return pMaxClients; }

	void	LoadAAs();
	int		GetTotalAAs() { return totalAAs; }
	SendAA_Struct*	GetAABySequence(uint32 seq) { return aas[seq]; }
	SendAA_Struct*	FindAA(uint32 id);
	uint8	GetTotalAALevels(uint32 skill_id);
	void	LoadZoneDoors(const char* zone, int16 version);
	bool	LoadZoneObjects();
	bool	LoadGroundSpawns();
	void	ReloadStaticData();

	uint32	CountSpawn2();
	ZonePoint* GetClosestZonePoint(const xyz_location& location, const char* to_name, Client *client, float max_distance = 40000.0f);
	ZonePoint* GetClosestZonePoint(const xyz_location& location, uint32	to, Client *client, float max_distance = 40000.0f);
	ZonePoint* GetClosestZonePointWithoutZone(float x, float y, float z, Client *client, float max_distance = 40000.0f);
	SpawnGroupList spawn_group_list;

	bool RemoveSpawnEntry(uint32 spawnid);
	bool RemoveSpawnGroup(uint32 in_id);

	bool	Process();
	void	Despawn(uint32 spawngroupID);

	bool	Depop(bool StartSpawnTimer = false);
	void	Repop(uint32 delay = 0);
	void	ClearNPCTypeCache(int id);
	void	SpawnStatus(Mob* client);
	void	ShowEnabledSpawnStatus(Mob* client);
	void	ShowDisabledSpawnStatus(Mob* client);
	void	ShowSpawnStatusByID(Mob* client, uint32 spawnid);
	void	StartShutdownTimer(uint32 set_time = (RuleI(Zone, AutoShutdownDelay)));
	void    ChangeWeather();
	bool	HasWeather();
	void	AddAuth(ServerZoneIncommingClient_Struct* szic);
	void	RemoveAuth(const char* iCharName);
	void	ResetAuth();
	bool	GetAuth(uint32 iIP, const char* iCharName, uint32* oWID = 0, uint32* oAccID = 0, uint32* oCharID = 0, int16* oStatus = 0, char* oLSKey = 0, bool* oTellsOff = 0);
	uint32	CountAuth();

	void		AddAggroMob()			{ aggroedmobs++; }
	void		DelAggroMob()			{ aggroedmobs--; }
	bool		AggroLimitReached()		{ return (aggroedmobs>10)?true:false; } // change this value, to allow more NPCs to autoaggro
	int32		MobsAggroCount()		{ return aggroedmobs; }
	inline bool InstantGrids()			{ return(!initgrids_timer.Enabled()); }
	void		SetStaticZone(bool sz)	{ staticzone = sz; }
	inline bool	IsStaticZone()			{ return staticzone; }
	inline void	GotCurTime(bool time)	{ gottime = time; }

	void	SpawnConditionChanged(const SpawnCondition &c, int16 old_value);

	void	GetMerchantDataForZoneLoad();
	void	LoadNewMerchantData(uint32 merchantid);
	void	LoadTempMerchantData();
	uint32	GetTempMerchantQuantity(uint32 NPCID, uint32 Slot);
	int		SaveTempItem(uint32 merchantid, uint32 npcid, uint32 item, int32 charges, bool sold=false);
	void LoadMercTemplates();
	void LoadMercSpells();
	void LoadLevelEXPMods();
	MercTemplate* GetMercTemplate( uint32 template_id );

	void SetInstanceTimer(uint32 new_duration);
	void LoadLDoNTraps();
	void LoadLDoNTrapEntries();
	void LoadAdventureFlavor();

	std::map<uint32,NPCType *> npctable;
	std::map<uint32,NPCType *> merctable;
	std::map<uint32,std::list<MerchantList> > merchanttable;
	std::map<uint32,std::list<TempMerchantList> > tmpmerchanttable;
	std::map<uint32,std::string> adventure_entry_list_flavor;
	std::map<uint32,LDoNTrapTemplate*> ldon_trap_list;
	std::map<uint32,std::list<LDoNTrapTemplate*> > ldon_trap_entry_list;
	std::map<uint32,std::list<MercStanceInfo> > merc_stance_list;
	std::map<uint32, MercTemplate> merc_templates;
	std::map<uint32,std::list<MercSpellEntry> > merc_spells_list;
	std::map<uint32, ZoneEXPModInfo> level_exp_mod;
	std::list<InternalVeteranReward> VeteranRewards;
	std::list<AltCurrencyDefinition_Struct> AlternateCurrencies;
	char *adv_data;
	bool did_adventure_actions;

	void	DoAdventureCountIncrease();
	void	DoAdventureAssassinationCountIncrease();
	void	DoAdventureActions();
	void	LoadVeteranRewards();
	void	LoadAlternateCurrencies();
	void	LoadNPCEmotes(LinkedList<NPC_Emote_Struct*>* NPCEmoteList);
	void	ReloadWorld(uint32 Option);

	Map*	zonemap;
	WaterMap* watermap;
	PathManager *pathing;
	NewZone_Struct	newzone_data;

	SpawnConditionManager spawn_conditions;

	EQTime	zone_time;
	void	GetTimeSync();
	void	SetDate(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute);
	void	SetTime(uint8 hour, uint8 minute);

	void	weatherSend();
	bool	CanBind() const { return(can_bind); }
	bool	IsCity() const { return(is_city); }
	bool	CanDoCombat() const { return(can_combat); }
	bool	CanLevitate() const {return(can_levitate); } // Magoth78
	bool	CanCastOutdoor() const {return(can_castoutdoor);} //qadar
	bool	AllowMercs() const {return(allow_mercs);}
	bool	IsHotzone() const { return(is_hotzone); }
	inline	bool BuffTimersSuspended() const { return newzone_data.SuspendBuffs != 0; };

	time_t	weather_timer;
	uint8	weather_intensity;
	uint8	zone_weather;

	uint8 loglevelvar;
	uint8 merchantvar;
	uint8 tradevar;
	uint8 lootvar;

	bool	HasGraveyard();
	void	SetGraveyard(uint32 zoneid, const xyz_heading& graveyardPosition);

	void		LoadBlockedSpells(uint32 zoneid);
	void		ClearBlockedSpells();
	bool		IsSpellBlocked(uint32 spell_id, const xyz_location& location);
	const char *GetSpellBlockedMessage(uint32 spell_id, const xyz_location& location);
	int			GetTotalBlockedSpells() { return totalBS; }
	inline bool HasMap() { return zonemap != nullptr; }
	inline bool HasWaterMap() { return watermap != nullptr; }

	QGlobalCache *GetQGlobals() { return qGlobals; }
	QGlobalCache *CreateQGlobals() { qGlobals = new QGlobalCache(); return qGlobals; }
	void	UpdateQGlobal(uint32 qid, QGlobal newGlobal);
	void	DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID);

	LinkedList<Spawn2*> spawn2_list;
	LinkedList<ZonePoint*> zone_point_list;
	uint32	numzonepoints;

	LinkedList<NPC_Emote_Struct*> NPCEmoteList;

    void    LoadTickItems();
    uint32  GetSpawnKillCount(uint32 in_spawnid);
    void    UpdateHotzone();
	std::unordered_map<int, item_tick_struct> tick_items;

	// random object that provides random values for the zone
	EQEmu::Random random;

	//MODDING HOOKS
	void mod_init();
	void mod_repop();

private:
	uint32	zoneid;
	uint32	instanceid;
	uint16	instanceversion;
	bool pers_instance;
	char*	short_name;
	char	file_name[16];
	char*	long_name;
	char*	map_name;
	bool pvpzone;
	xyz_location m_SafePoint;
	uint32	pMaxClients;
	bool	can_bind;
	bool	is_city;
	bool	can_combat;
	bool	can_castoutdoor;
	bool	can_levitate;
	bool	is_hotzone;
	uint8	zone_type;
	bool	allow_mercs;
	uint32	pgraveyard_id, pgraveyard_zoneid;
	xyz_heading m_Graveyard;
	int		default_ruleset;

	int	totalBS;
	ZoneSpellsBlocked *blocked_spells;

	int		totalAAs;
	SendAA_Struct **aas;	//array of AA structs

	/*
		Spawn related things
	*/
	int32	aggroedmobs;
	Timer initgrids_timer;	//delayed loading of initial grids.


	bool	staticzone;
	bool	gottime;

	uint32 pQueuedMerchantsWorkID;
	uint32 pQueuedTempMerchantsWorkID;

	Timer	autoshutdown_timer;
	Timer	clientauth_timer;
	Timer	spawn2_timer;
	Timer	qglobal_purge_timer;
	Timer*	Weather_Timer;
	Timer*	Instance_Timer;
	Timer*	Instance_Shutdown_Timer;
	Timer*	Instance_Warning_timer;
	LinkedList<ZoneClientAuth_Struct*> client_auth_list;
	QGlobalCache *qGlobals;

	Timer	hotzone_timer;
};

#endif
