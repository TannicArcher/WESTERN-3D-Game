#include "tr_local.h"
#include "../qcommon/qcommon.h"

#ifdef BUILD_FREETYPE
#include <ft2build.h>
#include <freetype/fterrors.h>
#include <freetype/ftsystem.h>
#include <freetype/ftimage.h>
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>

#define _FLOOR(x)  ((x) & -64)
#define _CEIL(x)   (((x)+63) & -64)
#define _TRUNC(x)  ((x) >> 6)

FT_Library ftLibrary = NULL;
#endif

#define MAX_FONTS 6
static int registeredFontCount = 0;
static fontInfo_t registeredFont[MAX_FONTS];

#ifdef BUILD_FREETYPE
void R_GetGlyphInfo(FT_GlyphSlot glyph, int *left, int *right, int *width, int *top, int *bottom, int *height, int *pitch) {

  *left  = _FLOOR( glyph->metrics.horiBearingX );
  *right = _CEIL( glyph->metrics.horiBearingX + glyph->metrics.width );
  *width = _TRUNC(*right - *left);

  *top    = _CEIL( glyph->metrics.horiBearingY );
  *bottom = _FLOOR( glyph->metrics.horiBearingY - glyph->metrics.height );
  *height = _TRUNC( *top - *bottom );
  *pitch  = ( qtrue ? (*width+3) & -4 : (*width+7) >> 3 );
}


FT_Bitmap *R_RenderGlyph(FT_GlyphSlot glyph, glyphInfo_t* glyphOut) {

  FT_Bitmap  *bit2;
  int left, right, width, top, bottom, height, pitch, size;

  R_GetGlyphInfo(glyph, &left, &right, &width, &top, &bottom, &height, &pitch);

  if ( glyph->format == ft_glyph_format_outline ) {
    size   = pitch*height;

    bit2 = Z_Malloc(sizeof(FT_Bitmap));

    bit2->width      = width;
    bit2->rows       = height;
    bit2->pitch      = pitch;
    bit2->pixel_mode = ft_pixel_mode_grays;
    //bit2->pixel_mode = ft_pixel_mode_mono;
    bit2->buffer     = Z_Malloc(pitch*height);
    bit2->num_grays = 256;

    Com_Memset( bit2->buffer, 0, size );

    FT_Outline_Translate( &glyph->outline, -left, -bottom );

    FT_Outline_Get_Bitmap( ftLibrary, &glyph->outline, bit2 );

    glyphOut->height = height;
    glyphOut->pitch = pitch;
    glyphOut->top = (glyph->metrics.horiBearingY >> 6) + 1;
    glyphOut->bottom = bottom;

    return bit2;
  }
  else {
    ri.Printf(PRINT_ALL, "Non-outline fonts are not supported\n");
  }
  return NULL;
}

void WriteTGA (char *filename, byte *data, int width, int height) {
	byte	*buffer;
	int		i, c;

	buffer = Z_Malloc(width*height*4 + 18);
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width&255;
	buffer[13] = width>>8;
	buffer[14] = height&255;
	buffer[15] = height>>8;
	buffer[16] = 32;	// pixel size

	// swap rgb to bgr
	c = 18 + width * height * 4;
	for (i=18 ; i<c ; i+=4)
	{
		buffer[i] = data[i-18+2];		// blue
		buffer[i+1] = data[i-18+1];		// green
		buffer[i+2] = data[i-18+0];		// red
		buffer[i+3] = data[i-18+3];		// alpha
	}

	ri.FS_WriteFile(filename, buffer, c);

	//f = fopen (filename, "wb");
	//fwrite (buffer, 1, c, f);
	//fclose (f);

	Z_Free (buffer);
}

static glyphInfo_t *RE_ConstructGlyphInfo(unsigned char *imageOut, int *xOut, int *yOut, int *maxHeight, FT_Face face, const unsigned char c, qboolean calcHeight) {
  int i;
  static glyphInfo_t glyph;
  unsigned char *src, *dst;
  float scaled_width, scaled_height;
  FT_Bitmap *bitmap = NULL;

  Com_Memset(&glyph, 0, sizeof(glyphInfo_t));
  // make sure everything is here
  if (face != NULL) {
    FT_Load_Glyph(face, FT_Get_Char_Index( face, c), FT_LOAD_DEFAULT );
    bitmap = R_RenderGlyph(face->glyph, &glyph);
    if (bitmap) {
      glyph.xSkip = (face->glyph->metrics.horiAdvance >> 6) + 1;
    } else {
      return &glyph;
    }

    if (glyph.height > *maxHeight) {
      *maxHeight = glyph.height;
    }

    if (calcHeight) {
      Z_Free(bitmap->buffer);
      Z_Free(bitmap);
      return &glyph;
    }

/*
    // need to convert to power of 2 sizes so we do not get
    // any scaling from the gl upload
  	for (scaled_width = 1 ; scaled_width < glyph.pitch ; scaled_width<<=1)
	  	;
  	for (scaled_height = 1 ; scaled_height < glyph.height ; scaled_height<<=1)
	  	;
*/

    scaled_width = glyph.pitch;
    scaled_height = glyph.height;

    // we need to make sure we fit
    if (*xOut + scaled_width + 1 >= 255) {
      if (*yOut + *maxHeight + 1 >= 255) {
        *yOut = -1;
        *xOut = -1;
        Z_Free(bitmap->buffer);
        Z_Free(bitmap);
        return &glyph;
      } else {
        *xOut = 0;
        *yOut += *maxHeight + 1;
      }
    } else if (*yOut + *maxHeight + 1 >= 255) {
      *yOut = -1;
      *xOut = -1;
      Z_Free(bitmap->buffer);
      Z_Free(bitmap);
      return &glyph;
    }


    src = bitmap->buffer;
    dst = imageOut + (*yOut * 256) + *xOut;

		if (bitmap->pixel_mode == ft_pixel_mode_mono) {
			for (i = 0; i < glyph.height; i++) {
				int j;
				unsigned char *_src = src;
				unsigned char *_dst = dst;
				unsigned char mask = 0x80;
				unsigned char val = *_src;
				for (j = 0; j < glyph.pitch; j++) {
					if (mask == 0x80) {
						val = *_src++;
					}
					if (val & mask) {
						*_dst = 0xff;
					}
					mask >>= 1;

					if ( mask == 0 ) {
						mask = 0x80;
					}
					_dst++;
				}

				src += glyph.pitch;
				dst += 256;

			}
		} else {
	    for (i = 0; i < glyph.height; i++) {
		    Com_Memcpy(dst, src, glyph.pitch);
			  src += glyph.pitch;
				dst += 256;
	    }
		}

    // we now have an 8 bit per pixel grey scale bitmap
    // that is width wide and pf->ftSize->metrics.y_ppem tall

    glyph.imageHeight = scaled_height;
    glyph.imageWidth = scaled_width;
    glyph.s = (float)*xOut / 256;
    glyph.t = (float)*yOut / 256;
    glyph.s2 = glyph.s + (float)scaled_width / 256;
    glyph.t2 = glyph.t + (float)scaled_height / 256;

    *xOut += scaled_width + 1;
  }

  Z_Free(bitmap->buffer);
  Z_Free(bitmap);

  return &glyph;
}
#endif

static int fdOffset;
static byte	*fdFile;

int readInt( void ) {
	int i = fdFile[fdOffset]+(fdFile[fdOffset+1]<<8)+(fdFile[fdOffset+2]<<16)+(fdFile[fdOffset+3]<<24);
	fdOffset += 4;
	return i;
}

typedef union {
	byte	fred[4];
	float	ffred;
} poor;

float readFloat( void ) {
	poor	me;
#if defined Q3_BIG_ENDIAN
	me.fred[0] = fdFile[fdOffset+3];
	me.fred[1] = fdFile[fdOffset+2];
	me.fred[2] = fdFile[fdOffset+1];
	me.fred[3] = fdFile[fdOffset+0];
#elif defined Q3_LITTLE_ENDIAN
	me.fred[0] = fdFile[fdOffset+0];
	me.fred[1] = fdFile[fdOffset+1];
	me.fred[2] = fdFile[fdOffset+2];
	me.fred[3] = fdFile[fdOffset+3];
#endif
	fdOffset += 4;
	return me.ffred;
}

void RE_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font) {
#ifdef BUILD_FREETYPE
  FT_Face face;
  int j, k, xOut, yOut, lastStart, imageNumber;
  int scaledSize, newSize, maxHeight, left, satLevels;
  unsigned char *out, *imageBuff;
  glyphInfo_t *glyph;
  image_t *image;
  qhandle_t h;
	float max;
#endif
  void *faceData;
	int i, len;
  char name[1024];
	float dpi = 72;											//
	float glyphScale =  72.0f / dpi; 		// change the scale to be relative to 1 based on 72 dpi ( so dpi of 144 means a scale of .5 )


  if (!fontName) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: called with empty name\n");
    return;
  }

	if (pointSize <= 0) {
		pointSize = 12;
	}
	// we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
	glyphScale *= 48.0f / pointSize;

	// make sure the render thread is stopped
	R_SyncRenderThread();

  if (registeredFontCount >= MAX_FONTS) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: Too many fonts registered already.\n");
    return;
  }

	Com_sprintf(name, sizeof(name), "fonts/fontImage_%i.dat",pointSize);
	for (i = 0; i < registeredFontCount; i++) {
		if (Q_stricmp(name, registeredFont[i].name) == 0) {
			Com_Memcpy(font, &registeredFont[i], sizeof(fontInfo_t));
			return;
		}
	}

	len = ri.FS_ReadFile(name, NULL);
	if (len == sizeof(fontInfo_t)) {
		ri.FS_ReadFile(name, &faceData);
		fdOffset = 0;
		fdFile = faceData;
		for(i=0; i<GLYPHS_PER_FONT; i++) {
			font->glyphs[i].height		= readInt();
			font->glyphs[i].top			= readInt();
			font->glyphs[i].bottom		= readInt();
			font->glyphs[i].pitch		= readInt();
			font->glyphs[i].xSkip		= readInt();
			font->glyphs[i].imageWidth	= readInt();
			font->glyphs[i].imageHeight = readInt();
			font->glyphs[i].s			= readFloat();
			font->glyphs[i].t			= readFloat();
			font->glyphs[i].s2			= readFloat();
			font->glyphs[i].t2			= readFloat();
			font->glyphs[i].glyph		= readInt();
			Com_Memcpy(font->glyphs[i].shaderName, &fdFile[fdOffset], 32);
			fdOffset += 32;
		}
		font->glyphScale = readFloat();
		Com_Memcpy(font->name, &fdFile[fdOffset], MAX_QPATH);

//		Com_Memcpy(font, faceData, sizeof(fontInfo_t));
		Q_strncpyz(font->name, name, sizeof(font->name));
		for (i = GLYPH_START; i < GLYPH_END; i++) {
			font->glyphs[i].glyph = RE_RegisterShaderNoMip(font->glyphs[i].shaderName);
		}
	  Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));
		return;
	}

#ifndef BUILD_FREETYPE
    ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType code not available\n");
#else
  if (ftLibrary == NULL) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType not initialized.\n");
    return;
  }

  len = ri.FS_ReadFile(fontName, &faceData);
  if (len <= 0) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: Unable to read font file\n");
    return;
  }

  // allocate on the stack first in case we fail
  if (FT_New_Memory_Face( ftLibrary, faceData, len, 0, &face )) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType2, unable to allocate new face.\n");
    return;
  }


  if (FT_Set_Char_Size( face, pointSize << 6, pointSize << 6, dpi, dpi)) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType2, Unable to set face char size.\n");
    return;
  }

  //*font = &registeredFonts[registeredFontCount++];

  // make a 256x256 image buffer, once it is full, register it, clean it and keep going
  // until all glyphs are rendered

  out = Z_Malloc(1024*1024);
  if (out == NULL) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: Z_Malloc failure during output image creation.\n");
    return;
  }
  Com_Memset(out, 0, 1024*1024);

  maxHeight = 0;

  for (i = GLYPH_START; i < GLYPH_END; i++) {
    glyph = RE_ConstructGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qtrue);
  }

  xOut = 0;
  yOut = 0;
  i = GLYPH_START;
  lastStart = i;
  imageNumber = 0;

  while ( i <= GLYPH_END ) {

    glyph = RE_ConstructGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qfalse);

    if (xOut == -1 || yOut == -1 || i == GLYPH_END)  {
      // ran out of room
      // we need to create an image from the bitmap, set all the handles in the glyphs to this point
      //

      scaledSize = 256*256;
      newSize = scaledSize * 4;
      imageBuff = Z_Malloc(newSize);
      left = 0;
      max = 0;
      satLevels = 255;
      for ( k = 0; k < (scaledSize) ; k++ ) {
        if (max < out[k]) {
          max = out[k];
        }
      }

			if (max > 0) {
				max = 255/max;
			}

      for ( k = 0; k < (scaledSize) ; k++ ) {
        imageBuff[left++] = 255;
        imageBuff[left++] = 255;
        imageBuff[left++] = 255;

        imageBuff[left++] = ((float)out[k] * max);
      }

			Com_sprintf (name, sizeof(name), "fonts/fontImage_%i_%i.tga", imageNumber++, pointSize);
			if (r_saveFontData->integer) {
			  WriteTGA(name, imageBuff, 256, 256);
			}

    	//Com_sprintf (name, sizeof(name), "fonts/fontImage_%i_%i", imageNumber++, pointSize);
      image = R_CreateImage(name, imageBuff, 256, 256, qfalse, qfalse, GL_CLAMP_TO_EDGE);
      h = RE_RegisterShaderFromImage(name, LIGHTMAP_2D, image, qfalse);
      for (j = lastStart; j < i; j++) {
        font->glyphs[j].glyph = h;
				Q_strncpyz(font->glyphs[j].shaderName, name, sizeof(font->glyphs[j].shaderName));
      }
      lastStart = i;
		  Com_Memset(out, 0, 1024*1024);
      xOut = 0;
      yOut = 0;
      Z_Free(imageBuff);
			i++;
    } else {
      Com_Memcpy(&font->glyphs[i], glyph, sizeof(glyphInfo_t));
      i++;
    }
  }

	registeredFont[registeredFontCount].glyphScale = glyphScale;
	font->glyphScale = glyphScale;
  Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));

	if (r_saveFontData->integer) {
		ri.FS_WriteFile(va("fonts/fontImage_%i.dat", pointSize), font, sizeof(fontInfo_t));
	}

  Z_Free(out);

  ri.FS_FreeFile(faceData);
#endif
}



void R_InitFreeType(void) {
#ifdef BUILD_FREETYPE
  if (FT_Init_FreeType( &ftLibrary )) {
    ri.Printf(PRINT_ALL, "R_InitFreeType: Unable to initialize FreeType.\n");
  }
#endif
  registeredFontCount = 0;
}


void R_DoneFreeType(void) {
#ifdef BUILD_FREETYPE
  if (ftLibrary) {
    FT_Done_FreeType( ftLibrary );
    ftLibrary = NULL;
  }
#endif
	registeredFontCount = 0;
}

