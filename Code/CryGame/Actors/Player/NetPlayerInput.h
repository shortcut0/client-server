// -------------------------------------------------------------------------
// Crytek Source File.
// Copyright (C) Crytek GmbH, 2001-2008.
// -------------------------------------------------------------------------
#ifndef __NETPLAYERINPUT_H__
#define __NETPLAYERINPUT_H__

#pragma once

#include "IPlayerInput.h"

class CPlayer;

class CNetPlayerInput : public IPlayerInput
{
public:

	// -----------------------------
	// Server
	float m_lastPseudoSpeed = 0.0f;
	bool m_lastSprint = false;
	bool m_spectatorSprint = false;
	bool m_moveReset = false;
	Vec3 m_spectatorPos;
	Vec3 m_spectatorDir;
	Vec3 m_deltaLast;


	CNetPlayerInput( CPlayer * pPlayer );

	// IPlayerInput
	virtual void PreUpdate();
	virtual void Update();
	virtual void PostUpdate();

	virtual void SetState( const SSerializedPlayerInput& input );
	virtual void GetState( SSerializedPlayerInput& input );

	virtual void OnAction(  const ActionId& action, int activationMode, float value  ) {};

	virtual void Reset();
	virtual void DisableXI(bool disabled);

	virtual void GetMemoryStatistics(ICrySizer * s) {s->Add(*this);}

	virtual EInputType GetType() const
	{
		return NETPLAYER_INPUT;
	};

	ILINE virtual uint32 GetMoveButtonsState() const { return 0; }
	ILINE virtual uint32 GetActions() const { return 0; }
	// ~IPlayerInput

private:
	CPlayer * m_pPlayer;
	SSerializedPlayerInput m_curInput;

	void DoSetState( const SSerializedPlayerInput& input );

	CTimeValue m_lastUpdate;

	struct SPrevPos
	{
		CTimeValue when;
		Vec3 where;
		Vec3 howFast;
	};
	typedef MiniQueue<SPrevPos, 32> TPreviousData;
	TPreviousData m_previousData;
};

#endif
