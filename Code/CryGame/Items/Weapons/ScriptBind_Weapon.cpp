/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$

 -------------------------------------------------------------------------
  History:
  - 27:10:2004   11:29 : Created by Márcio Martins

*************************************************************************/
#include "CryCommon/CrySystem/ISystem.h"
#include "ScriptBind_Weapon.h"
#include "../Item.h"
#include "Weapon.h"
#include "CryCommon/CryAction/IGameObject.h"
#include "CryGame/Actors/Actor.h"

// Server
#include "CryGame/Items/Weapons/GunTurret.h"


#define REUSE_VECTOR(table, name, value)	\
	{ if (table->GetValueType(name) != svtObject) \
	{ \
	table->SetValue(name, (value)); \
	} \
		else \
	{ \
	SmartScriptTable v; \
	table->GetValue(name, v); \
	v->SetValue("x", (value).x); \
	v->SetValue("y", (value).y); \
	v->SetValue("z", (value).z); \
	} \
	}


//------------------------------------------------------------------------
CScriptBind_Weapon::CScriptBind_Weapon(ISystem* pSystem, IGameFramework* pGameFramework)
	: m_pSystem(pSystem),
	m_pSS(pSystem->GetIScriptSystem()),
	m_pGameFW(pGameFramework)
{
	Init(m_pSS, m_pSystem, 1);

	RegisterMethods();
	RegisterGlobals();
}

//------------------------------------------------------------------------
CScriptBind_Weapon::~CScriptBind_Weapon()
{
}

//------------------------------------------------------------------------
void CScriptBind_Weapon::AttachTo(CWeapon* pWeapon)
{
	IScriptTable* pScriptTable = ((CItem*)pWeapon)->GetEntity()->GetScriptTable();

	if (pScriptTable)
	{
		SmartScriptTable thisTable(m_pSS);

		thisTable->SetValue("__this", ScriptHandle(pWeapon->GetEntityId()));
		thisTable->Delegate(GetMethodsTable());

		pScriptTable->SetValue("weapon", thisTable);
	}
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::SetAmmoCount(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());

	if (pFireMode)
	{
		if (pH->GetParamType(2) != svtNumber)
			return pH->EndFunction();

		const char* ammoName = 0;
		if (pH->GetParamType(1) == svtString)
			pH->GetParam(1, ammoName);

		IEntityClass* pAmmoType = pFireMode->GetAmmoType();

		if (ammoName)
			pAmmoType = gEnv->pEntitySystem->GetClassRegistry()->FindClass(ammoName);

		int ammo = 0;
		pH->GetParam(2, ammo);

		pWeapon->SetAmmoCount(pAmmoType, ammo);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetAmmoCount(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());

	if (pFireMode)
		return pH->EndFunction(pFireMode->GetAmmoCount());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetClipSize(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());

	if (pFireMode)
		return pH->EndFunction(pFireMode->GetClipSize());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::IsZoomed(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IZoomMode* pZoomMode = pWeapon->GetZoomMode(pWeapon->GetCurrentZoomMode());

	if (pZoomMode)
		return pH->EndFunction(pZoomMode->IsZoomed());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::IsZooming(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IZoomMode* pZoomMode = pWeapon->GetZoomMode(pWeapon->GetCurrentZoomMode());

	if (pZoomMode)
		return pH->EndFunction(pZoomMode->IsZooming());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetDamage(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());

	if (pFireMode)
		return pH->EndFunction(pFireMode->GetDamage());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetAmmoType(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());

	if (pFireMode)
		if (IEntityClass* pCls = pFireMode->GetAmmoType())
			return pH->EndFunction(pCls->GetName());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetRecoil(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());

	if (pFireMode)
		return pH->EndFunction(pFireMode->GetRecoil());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetSpread(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());

	if (pFireMode)
		return pH->EndFunction(pFireMode->GetSpread());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetCrosshair(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());

	if (pFireMode)
		return pH->EndFunction(pFireMode->GetCrosshair());

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetCrosshairOpacity(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	return pH->EndFunction(pWeapon->GetCrosshairOpacity());
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::GetCrosshairVisibility(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	return pH->EndFunction(pWeapon->GetCrosshairVisibility());
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::SetCurrentFireMode(IFunctionHandler* pH, const char* name)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->SetCurrentFireMode(name);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::SetCurrentZoomMode(IFunctionHandler* pH, const char* name)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->SetCurrentZoomMode(name);

	return pH->EndFunction();
}

int CScriptBind_Weapon::ModifyCommit(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->RestoreLayers();
	pWeapon->ReAttachAccessories();

	return pH->EndFunction();
}

class ScheduleAttachClass
{
	CWeapon* m_weapon = nullptr;
	ItemString m_className;
	bool m_attach = false;

public:
	ScheduleAttachClass(CWeapon* weapon, const char* className, bool attach)
	: m_weapon(weapon), m_className(className), m_attach(attach)
	{
	}

	void execute(CItem* item)
	{
		m_weapon->AttachAccessory(m_className, m_attach, false);
	}
};

int CScriptBind_Weapon::ScheduleAttach(IFunctionHandler* pH, const char* className, bool attach)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->GetScheduler()->ScheduleAction(CSchedulerAction<ScheduleAttachClass>::Create(ScheduleAttachClass(pWeapon, className, attach)));

	return pH->EndFunction();
}

int CScriptBind_Weapon::SupportsAccessory(IFunctionHandler* pH, const char* accessoryName)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	CItem::SAccessoryParams* params = pWeapon->GetAccessoryParams(ItemString(accessoryName));
	return pH->EndFunction(params != 0);
}

int CScriptBind_Weapon::GetAccessory(IFunctionHandler* pH, const char* accessoryName)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	CItem* pItem = pWeapon->GetAccessory(ItemString(accessoryName));

	if (!pItem)
		return 0;

	IEntity* pEntity = pItem->GetEntity();

	if (!pEntity)
		return 0;

	IScriptTable* pScriptTable = pEntity->GetScriptTable();

	return pH->EndFunction(pScriptTable);
}

int CScriptBind_Weapon::AttachAccessory(IFunctionHandler* pH, const char* className, bool attach, bool force)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (className)
		pWeapon->AttachAccessory(ItemString(className), attach, true, force);

	return pH->EndFunction();
}

int CScriptBind_Weapon::SwitchAccessory(IFunctionHandler* pH, const char* className)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (className)
		pWeapon->SwitchAccessory(ItemString(className));

	return pH->EndFunction();
}

int CScriptBind_Weapon::IsFiring(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	int n = pWeapon->GetNumOfFireModes();
	for (int i = 0;i < n;i++)
	{
		if (IFireMode* pFireMode = pWeapon->GetFireMode(i))
		{
			if (pFireMode->IsEnabled() && pFireMode->IsFiring())
				return pH->EndFunction(true);
		}
	}

	return pH->EndFunction();
}

int CScriptBind_Weapon::AttachAccessoryPlaceHolder(IFunctionHandler* pH, SmartScriptTable accessory, bool attach)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	const char* accessoryName;
	accessory->GetValue("class", accessoryName);
	ScriptHandle id;
	accessory->GetValue("id", id);

	EntityId entId = id.n;
	//CryLogAlways("id = %d", entId);
	IEntity* attachment = gEnv->pEntitySystem->GetEntity(entId);


	if (accessoryName)
	{
		const ItemString accessoryNameString(accessoryName);

		//CryLogAlways("got name: %s", accessoryName);
		if (pWeapon->GetAccessoryPlaceHolder(accessoryNameString))
		{
			//CryLogAlways("found accessory place holder");
			pWeapon->AttachAccessoryPlaceHolder(accessoryNameString, attach);
		}
		else
		{
			//CryLogAlways("accessory place holder not found");
			CActor* pActor = pWeapon->GetOwnerActor();
			IEntity* wep = pWeapon->GetEntity();
			//IGameObject *pGameObject = pWeapon->GetOwnerActor()->GetGameObject();
			IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				//CryLogAlways("found inventory");
				if (attachment)
				{
					if (pInventory->FindItem(entId) != -1)
					{
						//CryLogAlways("found attachment in inventory already...");
					}
					else
					{
						//CryLogAlways("attachment not found in inventory, adding...");
					}
					//CryLogAlways("found attachment");


					//attachment->DetachThis(0);
					//attachment->SetParentId(0);
					//CItem *t = (CItem *)attachment;
					//t->SetParentId(0);
					//pWeapon->GetEntity()->AttachChild(attachment, false)
					pInventory->AddItem(attachment->GetId());
					//for (int i = 0; i < wep->GetChildCount(); i++)
					//{
					//	IEntity *cur = wep->GetChild(i);
					//	CryLogAlways("none of these should be %s", attachment->GetName());
					//	CryLogAlways(" %s", cur->GetName());
					//}
					pWeapon->AttachAccessoryPlaceHolder(accessoryNameString, attach);
					pInventory->RemoveItem(attachment->GetId());

				}
				else
				{
					//CryLogAlways("!attachment");
				}
			}
		}

	}
	return pH->EndFunction();
}

int CScriptBind_Weapon::GetAttachmentHelperPos(IFunctionHandler* pH, const char* helperName)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	Vec3 pos = pWeapon->GetSlotHelperPos(CItem::eIGS_FirstPerson, helperName, false);
	Vec3 tpos = pWeapon->GetSlotHelperPos(CItem::eIGS_FirstPerson, helperName, true);
	//gEnv->pRenderer->DrawPoint(tpos.x, tpos.y, tpos.z, 10.0f);
	//CryLogAlways("helperName: %s pos: x=%f y=%f z=%f", helperName, tpos.x, tpos.y, tpos.z);
	//gEnv->pRenderer->DrawBall(tpos, 0.2f);
	//gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(tpos, 0.075f, ColorB( 255, 10, 10, 255 ));
	//gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(pWeapon->GetOwner()->GetWorldPos(), ColorB(0, 0, 255, 255), tpos, ColorB(255, 255, 0, 255));
	return pH->EndFunction(tpos);
}

int CScriptBind_Weapon::GetShooter(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	IEntity* pOwner = pWeapon->GetOwner();
	if (!pOwner)
		return pH->EndFunction();

	return pH->EndFunction(pOwner->GetScriptTable());
}

//------------------------------------------------------------------------
int CScriptBind_Weapon::AutoShoot(IFunctionHandler* pH, int shots, bool autoReload)
{
	struct AutoShootHelper : public IWeaponEventListener
	{
		AutoShootHelper(int n, bool autoreload) : m_nshots(n), m_reload(autoreload) {};
		virtual ~AutoShootHelper() {};

		int		m_nshots;
		bool	m_reload;

		virtual void OnShoot(IWeapon* pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType,
			const Vec3& pos, const Vec3& dir, const Vec3& vel) {};
		virtual void OnStartFire(IWeapon* pWeapon, EntityId shooterId) {};
		virtual void OnStopFire(IWeapon* pWeapon, EntityId shooterId) {};
		virtual void OnStartReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {};
		virtual void OnEndReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {};
		virtual void OnOutOfAmmo(IWeapon* pWeapon, IEntityClass* pAmmoType)
		{
			if (m_reload)
				pWeapon->Reload(false);
			else
			{
				pWeapon->StopFire();
				pWeapon->RemoveEventListener(this);
			}
		};
		virtual void OnReadyToFire(IWeapon* pWeapon)
		{
			if (!(--m_nshots))
			{
				pWeapon->StopFire();
				pWeapon->RemoveEventListener(this);
			}
			else
			{
				pWeapon->StartFire();
				pWeapon->StopFire();
			}
		};
		virtual void OnPickedUp(IWeapon* pWeapon, EntityId actorId, bool destroyed) {}
		virtual void OnDropped(IWeapon* pWeapon, EntityId actorId) {}
		virtual void OnMelee(IWeapon* pWeapon, EntityId shooterId) {}
		virtual void OnStartTargetting(IWeapon* pWeapon) {}
		virtual void OnStopTargetting(IWeapon* pWeapon) {}
		virtual void OnSelected(IWeapon* pWeapon, bool select) {}
	};

	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->AddEventListener(new AutoShootHelper(shots, autoReload), __FUNCTION__); 	// FIXME: possible small memory leak here. 
	pWeapon->StartFire();
	pWeapon->StopFire();

	return pH->EndFunction();
}

//
//------------------------------------------------------------------------
int CScriptBind_Weapon::Reload(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (pWeapon)
		pWeapon->Reload();

	return pH->EndFunction();
}


//------------------------------------------------------------------------
void CScriptBind_Weapon::RegisterGlobals()
{
}

//------------------------------------------------------------------------
void CScriptBind_Weapon::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_Weapon::

	SCRIPT_REG_TEMPLFUNC(SetAmmoCount, "")
		SCRIPT_REG_TEMPLFUNC(GetAmmoCount, "")
		SCRIPT_REG_TEMPLFUNC(GetClipSize, "")
		SCRIPT_REG_TEMPLFUNC(IsZoomed, "")
		SCRIPT_REG_TEMPLFUNC(IsZooming, "")
		SCRIPT_REG_TEMPLFUNC(GetDamage, "")
		SCRIPT_REG_TEMPLFUNC(GetAmmoType, "")

		SCRIPT_REG_TEMPLFUNC(GetRecoil, "");
	SCRIPT_REG_TEMPLFUNC(GetSpread, "");
	SCRIPT_REG_TEMPLFUNC(GetCrosshair, "");
	SCRIPT_REG_TEMPLFUNC(GetCrosshairOpacity, "");
	SCRIPT_REG_TEMPLFUNC(GetCrosshairVisibility, "");
	SCRIPT_REG_TEMPLFUNC(ModifyCommit, "");
	SCRIPT_REG_TEMPLFUNC(SupportsAccessory, "accessoryName");
	SCRIPT_REG_TEMPLFUNC(GetAccessory, "accessoryName");
	SCRIPT_REG_TEMPLFUNC(AttachAccessoryPlaceHolder, "accessory, attach");
	SCRIPT_REG_TEMPLFUNC(GetAttachmentHelperPos, "helperName");
	SCRIPT_REG_TEMPLFUNC(GetShooter, "");
	SCRIPT_REG_TEMPLFUNC(ScheduleAttach, "accessoryName, attach");
	SCRIPT_REG_TEMPLFUNC(AttachAccessory, "accessoryName, attach, force");
	SCRIPT_REG_TEMPLFUNC(SwitchAccessory, "accessoryName");

	SCRIPT_REG_TEMPLFUNC(IsFiring, "");
	SCRIPT_REG_TEMPLFUNC(SetCurrentFireMode, "name")
	SCRIPT_REG_TEMPLFUNC(SetCurrentZoomMode, "name")
	SCRIPT_REG_TEMPLFUNC(AutoShoot, "nshots, autoReload");
	SCRIPT_REG_TEMPLFUNC(Reload, "")

	SCRIPT_REG_TEMPLFUNC(ActivateLamLaser, "activate");
	SCRIPT_REG_TEMPLFUNC(ActivateLamLight, "activate");

	// Server
	SCRIPT_REG_TEMPLFUNC(SvRemoveAccessory, "name");
	SCRIPT_REG_TEMPLFUNC(SvChangeAccessory, "name");
	SCRIPT_REG_FUNC(GetAttachedAccessories);
	SCRIPT_REG_TEMPLFUNC(SetProjectileVelocitySpeedScale, "scale"); // such long name..

	SCRIPT_REG_TEMPLFUNC(Sv_SetOwnerID, "id")
	SCRIPT_REG_TEMPLFUNC(Sv_SetFiringInfo,"");
	SCRIPT_REG_TEMPLFUNC(Sv_RequestStartFire,"");
	SCRIPT_REG_TEMPLFUNC(Sv_SetRMIPlanting, "mode");
	SCRIPT_REG_TEMPLFUNC(Sv_SetPseudoOwnerId, "id");
	SCRIPT_REG_TEMPLFUNC(Sv_StopFireGunTurret, "sec");
	SCRIPT_REG_TEMPLFUNC(Sv_StartFireGunTurret, "sec");
	SCRIPT_REG_TEMPLFUNC(Sv_GunTurretTargetEntity, "id, sec");
	SCRIPT_REG_TEMPLFUNC(Sv_GunTurretSetLookAt, "z, x");
	SCRIPT_REG_TEMPLFUNC(Sv_GunTurretEnableServerFiring, "enable");
	SCRIPT_REG_FUNC(Sv_GunTurretResetLookAt);

	SCRIPT_REG_FUNC(Sv_Melee);
	SCRIPT_REG_FUNC(Sv_IsFiring);
	SCRIPT_REG_FUNC(Sv_RequestStopFire);
	SCRIPT_REG_FUNC(Sv_ResetFiringInfo);
	SCRIPT_REG_FUNC(Sv_GetFireModeName);
	SCRIPT_REG_FUNC(Sv_Update);
	SCRIPT_REG_FUNC(Sv_UpdateFM);
	SCRIPT_REG_FUNC(Sv_ResetGunTurret);		// reset only script data
	SCRIPT_REG_FUNC(Sv_ResetGunTurretAll);	// reset everything
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_SetPseudoOwnerId(IFunctionHandler* pH, ScriptHandle id)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	EntityId entityId;
	entityId = id.n;

	if (!entityId)
	{
	//	return pH->EndFunction(); // allow null
	}

	pWeapon->Sv_SetPseudoOwnerId(entityId);
	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_SetRMIPlanting(IFunctionHandler* pH, bool mode)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->sv_is_rmi_planting = mode;
	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_StopFireGunTurret(IFunctionHandler* pH, bool sec)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
	{
		CryLogAlways("updated...");
		pGT->Sv_StopFire(sec);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_GunTurretEnableServerFiring(IFunctionHandler* pH, bool enable)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
	{
		pGT->Sv_SetServerFiring(enable);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_GunTurretSetLookAt(IFunctionHandler* pH, float z, float x)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
	{
		pGT->Sv_SetLookAt(z, x);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_GunTurretResetLookAt(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
	{
		pGT->Sv_ResetLookAt();
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_StartFireGunTurret(IFunctionHandler* pH, bool sec)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
	{
		CryLogAlways("updated...");
		pGT->Sv_StartFire(sec);
	}

	return pH->EndFunction();
}


//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_GunTurretTargetEntity(IFunctionHandler* pH, ScriptHandle id, float sec)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	EntityId entityId;
	entityId = id.n;

	if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
	{
		CryLogAlways("updated...");
		pGT->Sv_SetTarget(entityId, sec);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_ResetGunTurret(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
	{
		CryLogAlways("updated...");
		pGT->Sv_OnReset();
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_ResetGunTurretAll(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
	{
		pGT->OnReset();
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_UpdateFM(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (IFireMode* pFM = pWeapon->GetActiveFireMode())
	{
		pFM->Update(gEnv->pTimer->GetFrameTime(), gEnv->pRenderer->GetFrameID());
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_Update(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	
	SEntityUpdateContext ctx;
	ctx.fFrameTime = gEnv->pTimer->GetFrameTime();
	ctx.nFrameID = gEnv->pRenderer->GetFrameID();
	pWeapon->Update(ctx, eIUS_FireMode);
	
	//pWeapon->RequireUpdate(eIUS_FireMode);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_SetOwnerID(IFunctionHandler* pH, ScriptHandle ownerId)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	EntityId pOwnerID = ownerId.n;
	pWeapon->SetOwnerId(pOwnerID);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_RequestStopFire(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	//CActor* pActor = pWeapon->GetOwnerActor();
//	if (!pActor)
	//	return pH->EndFunction();

	//Vec3 pos = pWeapon->GetEntity()->GetWorldPos();
	//Vec3 dir = pWeapon->GetEntity()->GetWorldRotation().GetRow0();


	pWeapon->Sv_IsFiring = false;
	Sv_ResetFiringInfo(pH);

	pWeapon->NetStopFire();
	pWeapon->SvRequestStopFire();
	pWeapon->RequestStopFire();
	
	return pH->EndFunction();
}


//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_GetFireModeName(IFunctionHandler* pH)
{

	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	if (IFireMode* pFM = pWeapon->GetActiveFireMode())
	{
		return pH->EndFunction(pFM->GetName());
	}

	return pH->EndFunction();
}


//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_IsFiring(IFunctionHandler* pH)
{

	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	return pH->EndFunction(pWeapon->Sv_IsFiring);
}



//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_ResetFiringInfo(IFunctionHandler* pH)
{

	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->Sv_FireDir.Set(0, 0, 0);
	pWeapon->Sv_FirePos.Set(0, 0, 0);
	pWeapon->Sv_FireHit.Set(0, 0, 0);
	pWeapon->Sv_FireRate = -1.f;

	return pH->EndFunction();
}


//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_SetFiringInfo(IFunctionHandler* pH)
{

	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	//if (!pWeapon->Sv_IsFiring)
	//	return pH->EndFunction();

	Vec3 dir(0, 0, 0);
	Vec3 pos(0, 0, 0);
	Vec3 hit(0, 0, 0);

	if (pH->GetParamCount() >= 1)
		if (pH->GetParam(1, dir) && dir.GetLength() > 0)
			pWeapon->Sv_FireDir = dir;

	if (pH->GetParamCount() >= 2)
		if (pH->GetParam(2, pos) && pos.GetLength() > 0)
			pWeapon->Sv_FirePos = pos;

	if (pH->GetParamCount() >= 3)
		if (pH->GetParam(3, hit) && hit.GetLength() > 0)
			pWeapon->Sv_FireHit = hit;

	float rate = -1;
	if (pH->GetParamCount() >= 4)
	{
		if (pH->GetParam(4, rate))
		{
			//CryLogAlways("hello, set to %f", rate);
			pWeapon->Sv_FireRate = rate;
		}
	}

	//CryLogAlways("rate= %f", pWeapon->Sv_FireRate);


	//CryLogAlways("Provided Dir: X=%f,y=%f,z=%f", dir.x, dir.y, dir.z);
	//CryLogAlways("Provided Pos: X=%f,y=%f,z=%f", pos.x, pos.y, pos.z);
	//CryLogAlways("Provided Hit: X=%f,y=%f,z=%f", hit.x, hit.y, hit.z);

	return pH->EndFunction();
}


//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_RequestStartFire(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	//CActor* pActor = pWeapon->GetOwnerActor();
//	if (!pActor)
	//	return pH->EndFunction();

	//Vec3 pos = pWeapon->GetEntity()->GetWorldPos();
	//Vec3 dir = pWeapon->GetEntity()->GetWorldRotation().GetRow0();

	pWeapon->Sv_IsFiring = true;
	if (IFireMode* pFM = pWeapon->GetActiveFireMode())
	{
	}

	Sv_ResetFiringInfo(pH);
	Sv_SetFiringInfo(pH);

	IEntityClass* pAmmoType = gEnv->pEntitySystem->GetClassRegistry()->FindClass("bullet");
	Vec3 pos = pWeapon->Sv_FirePos;
	Vec3 dir = pWeapon->Sv_FireDir;
	Vec3 vel = pWeapon->Sv_FireDir;
	Vec3 hit = pWeapon->Sv_FireHit;

	//pWeapon->RequestShoot(pAmmoType, pos, dir, vel, hit, 0, 0, pWeapon->GetShootSeqN(), 0, false);
	pWeapon->StartFire();
	pWeapon->NetStartFire();
	pWeapon->SvRequestStartFire();
	pWeapon->RequestStartFire();
	
	//if (IFireMode* pFM = pWeapon->GetActiveFireMode())
		//if (strcmp(pFM->GetName(), "single") == 0) {

		//	CryLogAlways("hot single in my area detected");
		//}

	//CryLogAlways("%s", pWeapon->GetActiveFireMode()->GetName());

	return pH->EndFunction();
}


//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::Sv_Melee(IFunctionHandler* pH) 
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	CActor* pActor = pWeapon->GetOwnerActor();
	if (!pActor)
		return pH->EndFunction();

	Vec3 pos(ZERO);
	Vec3 dir(ZERO);
	IMovementController* pMC = pActor->GetMovementController();
	if (!pMC)
		return pH->EndFunction();

	SMovementState info;
	pMC->GetMovementState(info);
	pos = info.eyePosition;
	dir = info.eyeDirection;

	//m_pWeapon->RequestMeleeAttack(m_pWeapon->GetMeleeFireMode() == this, pos, dir, m_pWeapon->GetShootSeqN());
	pWeapon->RequestMeleeAttack(true, pos, dir, pWeapon->GetShootSeqN());
	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::SetProjectileVelocitySpeedScale(IFunctionHandler* pH, float scale) // such long name..
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->m_projectileVelocityScale = scale;

	return pH->EndFunction();
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::GetAttachedAccessories(IFunctionHandler* pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	const CItem::TAccessoryMap* pMap = pWeapon->GetAttachedAccessories();
	SmartScriptTable pAttached(gEnv->pScriptSystem->CreateTable());

	bool getClass = false;
	if (pH->GetParamCount() >= 1)
		pH->GetParam(1, getClass);

	for (CItem::TAccessoryMap::const_iterator it = pMap->begin(); it != pMap->end(); it++) {
		if (getClass)
			pAttached->PushBack(it->first.c_str());
		else
			pAttached->PushBack(ScriptHandle(it->second));
	}

	return pH->EndFunction(pAttached);
}

//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::SvRemoveAccessory(IFunctionHandler* pH, const char* name)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->SvRemoveAccessory(ItemString(name));

	return pH->EndFunction();
}


//------------------------------------------------------------------------
// Server
int CScriptBind_Weapon::SvChangeAccessory(IFunctionHandler* pH, const char* name)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->SvChangeAccessory(ItemString(name));

	return pH->EndFunction();
}

//------------------------------------------------------------------------
CItem* CScriptBind_Weapon::GetItem(IFunctionHandler* pH)
{
	void* pThis = pH->GetThis();

	if (pThis)
	{
		IItem* pItem = m_pGameFW->GetIItemSystem()->GetItem((EntityId)(UINT_PTR)pThis);
		if (pItem)
			return static_cast<CItem*>(pItem);
	}

	return 0;
}

//------------------------------------------------------------------------
CWeapon* CScriptBind_Weapon::GetWeapon(IFunctionHandler* pH)
{
	void* pThis = pH->GetThis();

	if (pThis)
	{
		IItem* pItem = m_pGameFW->GetIItemSystem()->GetItem((EntityId)(UINT_PTR)pThis);
		if (pItem)
		{
			IWeapon* pWeapon = pItem->GetIWeapon();
			if (pWeapon)
				return static_cast<CWeapon*>(pWeapon);
		}
	}

	return 0;
}

//-----------------------------------------------------------------------
int CScriptBind_Weapon::ActivateLamLaser(IFunctionHandler* pH, bool activate)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->ActivateLamLaser(activate);

	return pH->EndFunction();
}

//-------------------------------------------------------------------------
int CScriptBind_Weapon::ActivateLamLight(IFunctionHandler* pH, bool activate)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (!pWeapon)
		return pH->EndFunction();

	pWeapon->ActivateLamLight(activate);

	return pH->EndFunction();

}

#undef REUSE_VECTOR
