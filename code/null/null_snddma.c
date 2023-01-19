

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

qboolean SNDDMA_Init(void)
{
	return qfalse;
}

int	SNDDMA_GetDMAPos(void)
{
	return 0;
}

void SNDDMA_Shutdown(void)
{
}

void SNDDMA_BeginPainting (void)
{
}

void SNDDMA_Submit(void)
{
}

sfxHandle_t S_RegisterSound( const char *name, qboolean compressed )
{
	return 0;
}

void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum ) {
}

void S_ClearSoundBuffer( void ) {
}
