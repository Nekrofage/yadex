/*
 *	r_images.cc
 *	AJA 2002-04-23 (based on textures.cc and flats.cc)
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Rapha�l Quinet and Brendon Wyber.

The rest of Yadex is Copyright � 1997-2000 Andr� Majorel.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#include <string>

#include "yadex.h"
#include <X11/Xlib.h>
#include "dialog.h"
#include "game.h"      /* yg_picture_format */
#include "gfx.h"
#include "levels.h"
#include "lists.h"
#include "patchdir.h"
#include "pic2img.h"
#include "sticker.h"
#include "flats.h"
#include "textures.h"
#include "wadfile.h"
#include "wads.h"
#include "wadres.h"
#include "wstructs.h"

#include "r_images.h"

using std::string;

/*
 *	flat_list_entry_match
 *	Function used by bsearch() to locate a particular 
 *	flat in the FTexture.
 */
static int flat_list_entry_match (const void *key, const void *flat_list_entry)
{
return y_strnicmp ((const char *) key,
      ((const flat_list_entry_t *) flat_list_entry)->name,
      WAD_FLAT_NAME);
}


/*
 *  load a flat into a new image.  NULL if not found.
 */

Img * Flat2Img (const wad_flat_name_t& fname)
{
char name[WAD_FLAT_NAME + 1];
strncpy (name, fname, WAD_FLAT_NAME);
name[WAD_FLAT_NAME] = 0;

flat_list_entry_t *flat = (flat_list_entry_t *)
   bsearch (name, flat_list, NumFTexture, sizeof *flat_list,
         flat_list_entry_match);

if (! flat)  // Not found in list
   return 0;

int width  = DOOM_FLAT_WIDTH;  // Big deal !
int height = DOOM_FLAT_HEIGHT;

const Wad_file *wadfile = flat->wadfile;
wadfile->seek (flat->offset);

Img *img = new Img (width, height, false);

wadfile->read_bytes (img->wbuf (), (long) width * height);

return img;
}


/*
 * load a wall texture ("TEXTURE1" or "TEXTURE2" object) into an image.
 * Returns NULL if not found or error.
 */

Img * Tex2Img (const string& texname)
{
MDirPtr  dir = 0;	/* main directory pointer to the TEXTURE* entries */
int32_t     *offsets;	/* array of offsets to texture names */
int      n;		/* general counter */
int16_t      width, height;	/* size of the texture */
int16_t      npatches;	/* number of wall patches used to build this texture */
int32_t      numtex;	/* number of texture names in TEXTURE* list */
int32_t      texofs;	/* offset in the wad file to the texture data */
char     tname[WAD_TEX_NAME + 1];	/* texture name */
char     picname[WAD_PIC_NAME + 1];	/* wall patch name */
bool     have_dummy_bytes;
int      header_size;
int      item_size;

char name[WAD_TEX_NAME + 1];
strncpy (name, texname.c_str(), WAD_TEX_NAME);
name[WAD_TEX_NAME] = 0;

// Iwad-dependant details
if (yg_texture_format == YGTF_NAMELESS)
   {
   have_dummy_bytes = true;
   header_size      = 14;
   item_size        = 10;
   }
else if (yg_texture_format == YGTF_NORMAL)
   {
   have_dummy_bytes = true;
   header_size      = 14;
   item_size        = 10;
   }
else if (yg_texture_format == YGTF_STRIFE11)
   {
   have_dummy_bytes = false;
   header_size      = 10;
   item_size        = 6;
   }
else
   {
   nf_bug ("Bad texture format %d.", (int) yg_texture_format);
   return 0;
   }

/* offset for texture we want. */
texofs = 0;
// Doom alpha 0.4 : "TEXTURES", no names
if (yg_texture_lumps == YGTL_TEXTURES && yg_texture_format == YGTF_NAMELESS)
   {
   dir = FindMasterDir (MasterDir, "TEXTURES");
   if (dir != NULL)
      {
      dir->wadfile->seek (dir->dir.start);
      dir->wadfile->read_int32_t (&numtex);
      if (WAD_TEX_NAME < 7) nf_bug ("WAD_TEX_NAME too small");  // Sanity
      if (! y_strnicmp (name, "TEX", 3)
            && isdigit (name[3])
            && isdigit (name[4])
            && isdigit (name[5])
            && isdigit (name[6])
            && name[7] == '\0')
         {
         long num;
         if (sscanf (name + 3, "%4ld", &num) == 1
               && num >= 0 && num < numtex)
            {
            dir->wadfile->seek (dir->dir.start + 4 + 4 * num);
            dir->wadfile->read_int32_t (&texofs);
            texofs += dir->dir.start;
            }
         }
      }
   }
// Doom alpha 0.5 : only "TEXTURES"
else if (yg_texture_lumps == YGTL_TEXTURES
      && (yg_texture_format == YGTF_NORMAL || yg_texture_format == YGTF_STRIFE11))
   {
   // Is it in TEXTURES ?
   dir = FindMasterDir (MasterDir, "TEXTURES");
   if (dir != NULL)  // (Theoretically, it should always exist)
      {
      dir->wadfile->seek (dir->dir.start);
      dir->wadfile->read_int32_t (&numtex);
      /* read in the offsets for texture1 names and info. */
      offsets = (int32_t *) malloc((long) numtex * 4);
      dir->wadfile->read_int32_t (offsets, numtex);
      for (n = 0; n < numtex && !texofs; n++)
         {
         dir->wadfile->seek (dir->dir.start + offsets[n]);
         dir->wadfile->read_bytes (&tname, WAD_TEX_NAME);
         if (!y_strnicmp (tname, name, WAD_TEX_NAME))
            texofs = dir->dir.start + offsets[n];
         }
      free(offsets);
      }
   }
// Other iwads : "TEXTURE1" and "TEXTURE2"
else if (yg_texture_lumps == YGTL_NORMAL
      && (yg_texture_format == YGTF_NORMAL || yg_texture_format == YGTF_STRIFE11))
   {
   // Is it in TEXTURE1 ?
   dir = FindMasterDir (MasterDir, "TEXTURE1");
   if (dir != NULL)  // (Theoretically, it should always exist)
      {
      dir->wadfile->seek (dir->dir.start);
      dir->wadfile->read_int32_t (&numtex);
      /* read in the offsets for texture1 names and info. */
      offsets = (int32_t *) malloc((long) numtex * 4);
      dir->wadfile->read_int32_t (offsets, numtex);
      for (n = 0; n < numtex && !texofs; n++)
         {
         dir->wadfile->seek (dir->dir.start + offsets[n]);
         dir->wadfile->read_bytes (&tname, WAD_TEX_NAME);
         if (!y_strnicmp (tname, name, WAD_TEX_NAME))
            texofs = dir->dir.start + offsets[n];
         }
      free(offsets);
      }
   // Well, then is it in TEXTURE2 ?
   if (texofs == 0)
      {
      dir = FindMasterDir (MasterDir, "TEXTURE2");
      if (dir != NULL)  // Doom II has no TEXTURE2
         {
         dir->wadfile->seek (dir->dir.start);
         dir->wadfile->read_int32_t (&numtex);
         /* read in the offsets for texture2 names */
         offsets = (int32_t *) malloc((long) numtex * 4);
         dir->wadfile->read_int32_t (offsets, numtex);
         for (n = 0; n < numtex && !texofs; n++)
            {
            dir->wadfile->seek (dir->dir.start + offsets[n]);
            dir->wadfile->read_bytes (&tname, WAD_TEX_NAME);
            if (!y_strnicmp (tname, name, WAD_TEX_NAME))
               texofs = dir->dir.start + offsets[n];
            }
         free(offsets);
         }
      }
   }
else
   nf_bug ("Invalid texture_format/texture_lumps combination.");

/* texture name not found */
if (texofs == 0)
   return 0;

/* read the info for this texture */
int32_t header_ofs;
if (yg_texture_format == YGTF_NAMELESS)
   header_ofs = texofs;
else
   header_ofs = texofs + WAD_TEX_NAME;
dir->wadfile->seek (header_ofs + 4);
dir->wadfile->read_int16_t (&width);
dir->wadfile->read_int16_t (&height);
if (have_dummy_bytes)
   {
   int16_t dummy;
   dir->wadfile->read_int16_t (&dummy);
   dir->wadfile->read_int16_t (&dummy);
   }
dir->wadfile->read_int16_t (&npatches);

/* Compose the texture */
Img *texbuf = new Img (width, height, false);

/* Paste onto the buffer all the patches that the texture is
   made of. */
for (n = 0; n < npatches; n++)
   {
   int16_t xofs, yofs;	// offset in texture space for the patch
   int16_t pnameind;	// index of patch in PNAMES

   dir->wadfile->seek (header_ofs + header_size + (long) n * item_size);
   dir->wadfile->read_int16_t (&xofs);
   dir->wadfile->read_int16_t (&yofs);
   dir->wadfile->read_int16_t (&pnameind);

   if (have_dummy_bytes)
      {
      int16_t stepdir;
      int16_t colormap;
      dir->wadfile->read_int16_t (&stepdir);   // Always 1, unused.
      dir->wadfile->read_int16_t (&colormap);  // Always 0, unused.
      }

   /* AYM 1998-08-08: Yes, that's weird but that's what Doom
      does. Without these two lines, the few textures that have
      patches with negative y-offsets (BIGDOOR7, SKY1, TEKWALL1,
      TEKWALL5 and a few others) would not look in the texture
      viewer quite like in Doom. This should be mentioned in
      the UDS, by the way. */
   if (yofs < 0)
      yofs = 0;

   Lump_loc loc;
      {
      wad_pic_name_t *wname = patch_dir.name_for_num (pnameind);
      if (wname == 0)
         {
         warn ("texture \"%.*s\": patch %2d has bad index %d.\n",
               WAD_TEX_NAME, tname, (int) n, (int) pnameind);
         continue;
         }
      patch_dir.loc_by_name ((const char *) *wname, loc);
      *picname = '\0';
      strncat (picname, (const char *) *wname, sizeof picname - 1);
      }

   if (LoadPicture (*texbuf, picname, loc, xofs, yofs, 0, 0))
      warn ("texture \"%.*s\": patch \"%.*s\" not found.\n",
            WAD_TEX_NAME, tname, WAD_PIC_NAME, picname);
   }

return texbuf;
}


/* --- ImageCache methods --- */


Img *ImageCache::GetFlat (const wad_flat_name_t& fname)
{
std::string f_str = WadToString(fname);

flat_map_t::iterator P = flats.find (f_str);

if (P != flats.end ())
   return P->second;

// flat not in the list yet.  Add it.

Img *result = Flat2Img (fname);
flats[f_str] = result;

// note that a NULL return from Flat2Img is OK, it means that no
// such flat exists.  Our renderer will revert to using a solid
// colour.

return result;
}


Img *ImageCache::GetTex (const string& tname) {
	if (tname == "" || tname == "-")
		return NULL;

	tex_map_t::iterator P = textures.find(tname);

	if (P != textures.end ())
		return P->second;

	// texture not in the list yet.  Add it.

	Img *result = Tex2Img(tname);
	textures[tname] = result;

	// note that a NULL return from Tex2Img is OK, it means that no
	// such texture exists.  Our renderer will revert to using a solid
	// colour.

	return result;
}


Img *ImageCache::GetSprite (const wad_ttype_t& type)
{
sprite_map_t::iterator P = sprites.find (type);

if (P != sprites.end ())
   return P->second;

// sprite not in the list yet.  Add it.

Img *result = 0;

const char *sprite_root = get_thing_sprite (type);
if (sprite_root)
   {
   Lump_loc loc;
   wad_res.sprites.loc_by_root (sprite_root, loc);
   result = new Img ();

   if (LoadPicture (*result, sprite_root, loc, 0, 0) != 0)
      {
      delete result;
      result = 0;
      }
   }

// note that a NULL image is OK.  Our renderer will just ignore the
// missing sprite.

sprites[type] = result;
return result;
}
