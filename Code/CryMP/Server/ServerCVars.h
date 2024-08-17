#ifndef __SERVERCVARS_H__
#define __SERVERCVARS_H__

struct IConsole;
struct ICVar;

struct ServerCVars {

	// -----------------------
	// Server
	int server_use_hit_queue;
	int server_use_explosion_queue;
	int server_classic_chat;
	int server_lag_resetmovement;
	int server_ghostbug_fix;
	int server_turrets_target_cloaked;
	int server_autodrop_rpg;
	int server_suit_cloakMeleeEnergyCost;
	int server_suit_cloakShootingEnergyCost;
	int server_suit_speedShootingEnergyCost;
	int server_suit_strengthShootingEnergyCost;
	int server_suit_strengthMeleeEnergyCost;
	int server_suit_speedMeleeEnergyCost;
	int server_c4_limit;
	int server_allow_c4Hits;
	int server_c4_stickToPlayers;
	int server_c4_stickToAllSpecies;
	int server_c4_stickLimitOne;

	// Commands
	ICVar *server_mapTitle;

	// -----------------------
	// CryMP (Synched)
	int			mp_crymp;
	float		mp_circleJump;
	float		mp_wallJump;
	int			mp_flyMode;
	int			mp_pickupObjects;
	int			mp_pickupVehicles;
	int			mp_weaponsOnBack;
	int	        mp_thirdPerson;
	int			mp_animationGrenadeSwitch;
	int			mp_ragdollUnrestricted;
	int			mp_killMessages;
	int			mp_rpgMod;
	int			mp_aaLockOn;
	float		mp_C4StrengthThrowMult;

	// Client (Unsynched, unused)
	int			mp_newSpectator;
	int			mp_usePostProcessAimDir;
	int			mp_messageCenterColor;	
	float		mp_animationWeaponMult;
	float		mp_animationWeaponMultSpeed;
	float		mp_animationModelMult;
	float		mp_animationModelMultSpeed;
	float		mp_menuSpeed;
	int			mp_hitIndicator;
	int			mp_chatHighResolution;
	float		mp_spectatorSlowMult;
	int			mp_buyPageKeepTime;
	int			mp_attachBoughtEquipment;
	float		mp_netAimLerpFactor;
	float		mp_netAimLerpFactorCrymp;
	int         mp_explosiveSilhouettes;

	// -----------------------
	ServerCVars();
	~ServerCVars();

	void InitCVars(IConsole* pConsole);
	void ReleaseCVars();
};

// -----------------------
extern struct ServerCVars* g_pServerCVars;

#endif // __SERVERCVARS_H__