
#include "keycodes.h"

typedef struct {
	qboolean	down;
	int			repeats;		// if > 1, it is autorepeating
	char		*binding;
} qkey_t;

extern	qboolean	key_overstrikeMode;
extern	qkey_t		keys[MAX_KEYS];

// NOTE TTimo the declaration of field_t and Field_Clear is now in qcommon/qcommon.h
void Field_KeyDownEvent( field_t *edit, int key );
void Field_CharEvent( field_t *edit, int ch );
void Field_Draw( field_t *edit, int x, int y, int width, qboolean showCursor, qboolean noColorEscape );
void Field_BigDraw( field_t *edit, int x, int y, int width, qboolean showCursor, qboolean noColorEscape );

#define		COMMAND_HISTORY		32
extern	field_t	historyEditLines[COMMAND_HISTORY];

extern	field_t	g_consoleField;
extern	field_t	chatField;
extern	int				anykeydown;
extern	qboolean	chat_team;
extern	int			chat_playerNum;

void Key_WriteBindings( fileHandle_t f );
void Key_SetBinding( int keynum, const char *binding );
char *Key_GetBinding( int keynum );
qboolean Key_IsDown( int keynum );
qboolean Key_GetOverstrikeMode( void );
void Key_SetOverstrikeMode( qboolean state );
void Key_ClearStates( void );
int Key_GetKey(const char *binding);
