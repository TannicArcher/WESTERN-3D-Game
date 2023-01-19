
#ifndef TR_GLSLPROGS_H
#define TR_GLSLPROGS_H

//basicly the idea here is to just store fragments of glsl source files so that
//we can reuse the code in different programs. the problem with glsl is that
//there is no defined way of 'including' code from other places. so we will
//just have to live with this for now

typedef struct {
	const char *name;
	const char *program;
	const char *default_program;
} gslsProg_t ;

void R_GLSLProgs_Init( void );
void R_GLSLProgs_Delete( void );
const char *R_GLSLGetProgByName( const char *name );

#endif //TR_GLSLPROGS_H
