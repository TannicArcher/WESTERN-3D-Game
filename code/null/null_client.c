
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

cvar_t *cl_shownet;

void CL_Shutdown( char *finalmsg ) {
}

void CL_Init( void ) {
	cl_shownet = Cvar_Get ("cl_shownet", "0", CVAR_TEMP );
}

void CL_MouseEvent( int dx, int dy, int time ) {
}

void Key_WriteBindings( fileHandle_t f ) {
}

void CL_Frame ( int msec ) {
}

void CL_PacketEvent( netadr_t from, msg_t *msg ) {
}

void CL_CharEvent( int key ) {
}

void CL_Disconnect( qboolean showMainMenu ) {
}

void CL_MapLoading( void ) {
}

qboolean CL_GameCommand( void ) {
  return qfalse;
}

void CL_KeyEvent (int key, qboolean down, unsigned time) {
}

qboolean UI_GameCommand( void ) {
	return qfalse;
}

void CL_ForwardCommandToServer( const char *string ) {
}

#ifndef WESTERN3D
void CL_ConsolePrint( char *txt ) {
}
#else
char *CL_ConsolePrint( char *txt ) {
	return txt;
}
#endif

void CL_JoystickEvent( int axis, int value, int time ) {
}

void CL_InitKeyCommands( void ) {
}

void CL_CDDialog( void ) {
}

void CL_FlushMemory( void ) {
}

void CL_StartHunkUsers( qboolean rendererOnly ) {
}

void CL_Snd_Restart(void)
{
}

void CL_ShutdownAll(void) {}

qboolean CL_CDKeyValidate( const char *key, const char *checksum ) { return qtrue; }
