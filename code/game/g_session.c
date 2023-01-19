
#include "g_local.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;

	s = va("%i %i %i %i %i %i %i",
		client->sess.sessionTeam,
		client->sess.spectatorNum,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
		client->sess.teamLeader
		);

	var = va( "session%i", (int)(client - level.clients) );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;
	int teamLeader;
	int spectatorState;
	int sessionTeam;

	var = va( "session%i", (int)(client - level.clients) );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i",
		&sessionTeam,
		&client->sess.spectatorNum,
		&spectatorState,
		&client->sess.spectatorClient,
		&client->sess.wins,
		&client->sess.losses,
		&teamLeader
		);

	client->sess.sessionTeam = (team_t)sessionTeam;
	client->sess.spectatorState = (spectatorState_t)spectatorState;
	client->sess.teamLeader = (qboolean)teamLeader;

	//make all players spectators at start, to rechoose their weapons
#ifdef WESTERN3D
	if ( g_gametype.integer >= GT_RTP || g_gametype.integer == GT_DUEL){

		client->sess.sessionTeam = TEAM_SPECTATOR;

		client->sess.spectatorState = SPECTATOR_FREE;
		AddTournamentQueue(client);
		client->realspec = qtrue;

		// the bots join at once
		if(g_entities[client-level.clients].r.svFlags & SVF_BOT)
			client->realspec = qfalse;
		//G_WriteClientSessionData( client );
	}
#endif
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
#ifndef WESTERN3D
void G_InitSessionData( gclient_t *client, char *userinfo ) {
#else
void G_InitSessionData( gclient_t *client, char *userinfo, qboolean isBot ) {
#endif
	clientSession_t	*sess;
	const char		*value;

#ifdef WESTERN3D
	int			clientNum;
	clientNum = client - level.clients;
#endif

	sess = &client->sess;

	// initial team determination
#ifndef WESTERN3D
	if ( g_gametype.integer >= GT_TEAM ) {
#else
	if ( g_gametype.integer == GT_TEAM ) {
#endif
		if ( g_teamAutoJoin.integer ) {
			sess->sessionTeam = PickTeam( -1 );
#ifdef WESTERN3D
			if ( sess->sessionTeam == TEAM_RED )  PushMinilogf( "JOIN: %i => lawmen" , clientNum ) ;
			else if ( sess->sessionTeam == TEAM_BLUE )  PushMinilogf( "JOIN: %i => outlaws" , clientNum ) ;
#endif
			BroadcastTeamChange( client, -1 );
		} else {
			// always spawn as spectator in team games
			sess->sessionTeam = TEAM_SPECTATOR;
#ifdef WESTERN3D
			PushMinilogf( "JOIN: %i => spec" , clientNum ) ;
#endif
		}
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 &&
					level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
#ifdef WESTERN3D
					PushMinilogf( "JOIN: %i => spec" , clientNum ) ;
#endif
				} else {
					sess->sessionTeam = TEAM_FREE;
#ifdef WESTERN3D
					PushMinilogf( "JOIN: %i => free" , clientNum ) ;
#endif
				}
				break;
#ifndef WESTERN3D
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
#else
			case GT_RTP:
			case GT_BR:
				if ( g_teamAutoJoin.integer ) {
					sess->sessionTeam = PickTeam( -1 );
					BroadcastTeamChange( client, -1 );
				} else {
					// always spawn as spectator in team games
					sess->sessionTeam = TEAM_SPECTATOR;
					PushMinilogf( "JOIN: %i => spec" , clientNum ) ;
				}
				break;
			case GT_DUEL:

				// bots do always join
				if(isBot){
					client->realspec = qfalse;
				} else {
					client->realspec = qtrue;
				}

				sess->sessionTeam = TEAM_SPECTATOR;
				PushMinilogf( "JOIN: %i => spec" , clientNum ) ;
#endif
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	AddTournamentQueue(client);

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int		gt;
#ifdef WESTERN3D
	int		i;
#endif

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );

	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		G_Printf( "Gametype changed, clearing session data.\n" );
	}

	// init the data
#ifdef WESTERN3D
	switch ( g_gametype.integer ) {
	case GT_RTP:
	case GT_BR:
		g_round = 0;
		g_goldescaped = qfalse;
		g_robbed = qfalse;
		break;
	case GT_DUEL:
		g_round = 1;
		g_session = 0;

		// clear old duel data
		ClearDuelData(qtrue);

		// also clear other scores
		for(i = 0; i < MAX_CLIENTS; i++){
			gentity_t *ent = &g_entities[i];

			if(!g_entities[i].client)
				continue;

			if(level.clients[i].pers.connected != CON_CONNECTED)
				continue;

			ent->client->ps.stats[STAT_WINS] = 0;
			ent->client->sess.wins = 0;
			ent->client->sess.losses = 0;

			ClientUserinfoChanged(i);
		}
		break;
	}
#endif
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	trap_Cvar_Set( "session", va("%i", g_gametype.integer) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
