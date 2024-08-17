#include "CryGame/Actors/Actor.h"

class ServerAnticheat 
{

public:
	bool CheckLongpoke(CActor* pActor, int seq) {return seq == 1;}
};