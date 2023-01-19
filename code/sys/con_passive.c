
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"

#include <stdio.h>

/*
==================
CON_Shutdown
==================
*/
void CON_Shutdown( void )
{
}

/*
==================
CON_Init
==================
*/
void CON_Init( void )
{
}

/*
==================
CON_Input
==================
*/
char *CON_Input( void )
{
	return NULL;
}

/*
==================
CON_Print
==================
*/
void CON_Print( const char *msg )
{
	if( com_ansiColor && com_ansiColor->integer )
		Sys_AnsiColorPrint( msg );
	else
		fputs( msg, stderr );
}
