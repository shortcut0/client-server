/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: Script Binding for Weapon

 -------------------------------------------------------------------------
  History:
  - 25:11:2004   11:30 : Created by Márcio Martins

*************************************************************************/
#ifndef __SCRIPTBIND_WEAPON_H__
#define __SCRIPTBIND_WEAPON_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include "CryCommon/CryScriptSystem/IScriptSystem.h"


struct IItemSystem;
struct IGameFramework;
class CItem;
class CWeapon;


class CScriptBind_Weapon :
	public CScriptableBase
{
public:
	CScriptBind_Weapon(ISystem* pSystem, IGameFramework* pGameFramework);
	virtual ~CScriptBind_Weapon();

	void AttachTo(CWeapon* pWeapon);

	// Server
	int SvRemoveAccessory(IFunctionHandler* pH, const char* name);
	int SvChangeAccessory(IFunctionHandler* pH, const char* name);
	int GetAttachedAccessories(IFunctionHandler* pH);
	int SetProjectileVelocitySpeedScale(IFunctionHandler* pH, float scale); // such long name..

	int Sv_Melee(IFunctionHandler* pH);
	int Sv_SetFiringInfo(IFunctionHandler* pH);
	int Sv_RequestStartFire(IFunctionHandler* pH);
	int Sv_RequestStopFire(IFunctionHandler* pH);
	int Sv_ResetFiringInfo(IFunctionHandler* pH);
	int Sv_GetFireModeName(IFunctionHandler* pH);
	int Sv_IsFiring(IFunctionHandler* pH);
	int Sv_Update(IFunctionHandler* pH);
	int Sv_UpdateFM(IFunctionHandler* pH);
	int Sv_SetOwnerID(IFunctionHandler* pH, ScriptHandle ownerId);
	

	int Sv_SetRMIPlanting(IFunctionHandler* pH, bool mode);
	int Sv_SetPseudoOwnerId(IFunctionHandler* pH, ScriptHandle id);

	// ===========================================================================
	// Gun Turret
	int Sv_StartFireGunTurret(IFunctionHandler* pH, bool mode);
	int Sv_StopFireGunTurret(IFunctionHandler* pH, bool mode);
	int Sv_GunTurretTargetEntity(IFunctionHandler* pH, ScriptHandle id, float sec);
	int Sv_ResetGunTurret(IFunctionHandler* pH);
	int Sv_ResetGunTurretAll(IFunctionHandler* pH);
	int Sv_GunTurretResetLookAt(IFunctionHandler* pH);
	int Sv_GunTurretSetLookAt(IFunctionHandler* pH, float z, float x);
	int Sv_GunTurretEnableServerFiring(IFunctionHandler* pH, bool enable);

	// ...

	int SetAmmoCount(IFunctionHandler* pH);
	int GetAmmoCount(IFunctionHandler* pH);
	int GetClipSize(IFunctionHandler* pH);

	int IsZoomed(IFunctionHandler* pH);
	int IsZooming(IFunctionHandler* pH);
	int GetDamage(IFunctionHandler* pH);
	int GetAmmoType(IFunctionHandler* pH);

	int GetRecoil(IFunctionHandler* pH);
	int GetSpread(IFunctionHandler* pH);
	int GetCrosshair(IFunctionHandler* pH);
	int GetCrosshairOpacity(IFunctionHandler* pH);
	int GetCrosshairVisibility(IFunctionHandler* pH);
	int ModifyCommit(IFunctionHandler* pH);
	int SupportsAccessory(IFunctionHandler* pH, const char* accessoryName);
	int GetAccessory(IFunctionHandler* pH, const char* accessoryName);
	int AttachAccessoryPlaceHolder(IFunctionHandler* pH, SmartScriptTable accessory, bool attach);
	int GetAttachmentHelperPos(IFunctionHandler* pH, const char* helperName);
	int GetShooter(IFunctionHandler* pH);
	int ScheduleAttach(IFunctionHandler* pH, const char* className, bool attach);
	int AttachAccessory(IFunctionHandler* pH, const char* className, bool attach, bool force);
	int SwitchAccessory(IFunctionHandler* pH, const char* className);

	int IsFiring(IFunctionHandler* pH);


	int SetCurrentFireMode(IFunctionHandler* pH, const char* name);
	int SetCurrentZoomMode(IFunctionHandler* pH, const char* name);

	int AutoShoot(IFunctionHandler* pH, int nshots, bool autoReload);

	int Reload(IFunctionHandler* pH);

	int ActivateLamLaser(IFunctionHandler* pH, bool activate);
	int ActivateLamLight(IFunctionHandler* pH, bool activate);

private:
	void RegisterGlobals();
	void RegisterMethods();

	CItem* GetItem(IFunctionHandler* pH);
	CWeapon* GetWeapon(IFunctionHandler* pH);

	ISystem* m_pSystem;
	IScriptSystem* m_pSS;
	IGameFramework* m_pGameFW;
};


#endif //__SCRIPTBIND_ITEM_H__
