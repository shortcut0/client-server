/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2005.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: 
	Silhouettes manager

-------------------------------------------------------------------------
History:
- 20:07:2007: Created by Julien Darre

*************************************************************************/
#ifndef __HUDSILHOUETTES_H__
#define __HUDSILHOUETTES_H__

//-----------------------------------------------------------------------------------------------------

class CHUD;

class CHUDSilhouettes
{
public:

	explicit CHUDSilhouettes(CHUD* pHUD);
	~	CHUDSilhouettes();

	void SetSilhouette(IActor *pActor,ColorF color,float fDuration,bool bHighlightCurrentItem=true,bool bHighlightAccessories=true);
	void SetSilhouette(IItem *pItem,ColorF color,float fDuration,bool bHighlightAccessories=true);
	void SetSilhouette(IVehicle	*pVehicle,ColorF color,float fDuration);
	void SetSilhouette(IEntity *pEntity,ColorF color,float fDuration);
	void SetFlowGraphSilhouette(IEntity *pEntity,ColorF color,float fDuration);

	void ResetSilhouette(EntityId uiEntityId);
	void ResetFlowGraphSilhouette(EntityId uiEntityId);

	void SetType(int iType);

	void Serialize(TSerialize &ser);

	void Update(float frameTime);

private:

	void SetVisionParams(EntityId uiEntityId, ColorF color);

	void ResetVisionParams(EntityId uiEntityId);

	std::map<EntityId, Vec3>::iterator GetFGSilhouette(EntityId id);

	struct SSilhouette
	{
		EntityId uiEntityId;
		float fTime;
		bool bValid;
		float r, g, b, a;

		SSilhouette() : bValid(false)
		{
		}

		void Serialize(TSerialize &ser)
		{
			ser.Value("uiEntityId", uiEntityId);
			ser.Value("fTime", fTime);
			ser.Value("bValid", bValid);
			ser.Value("r", r);
			ser.Value("g", g);
			ser.Value("b", b);
			ser.Value("a", a);
		}
	};

	typedef std::vector<SSilhouette> TSilhouettesVector;
	TSilhouettesVector m_silhouettesVector;
	std::map<EntityId, Vec3> m_silhouettesFGVector;
	CHUD* m_pHUD = nullptr;
};

//-----------------------------------------------------------------------------------------------------

#endif

//-----------------------------------------------------------------------------------------------------
