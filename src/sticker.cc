/*
 *	sticker.cc - Sticker class
 *	AYM 2000-07-06
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Rapha�l Quinet and Brendon Wyber.

The rest of Yadex is Copyright � 1997-2003 Andr� Majorel and others.

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


#include "yadex.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // XDestroyImage
#include "gcolour2.h"
#include "gcolour3.h"
#include "gfx.h"
#include "img.h"
#include "sticker.h"


class Sticker_priv
{
  public :
    Sticker_priv ();
    ~Sticker_priv ();
    void clear ();
    void load (const Img &img, bool opaque);
    XImage *make_ximage (const Img& img);
    XImage *make_bitmap (const Img& img);
    bool opaque;
    bool has_data;
    int  width;
    int  height;
    XImage *ximage;	// Used only if opaque
    Pixmap pixmap;	// Used only if not opaque
    Pixmap mask;	// Used only if not opaque
};


/*
 *	Sticker::Sticker - create an empty Sticker
 */
Sticker::Sticker ()
{
  priv = new Sticker_priv ();
}


/*
 *	Sticker::Sticker - create a Sticker from an Img
 */
Sticker::Sticker (const Img& img, bool opaque)
{
  priv = new Sticker_priv ();
  priv->load (img, opaque);
}


/*
 *	Sticker::~Sticker - destroy a Sticker
 */
Sticker::~Sticker ()
{
  delete priv;
} 


/*
 *	Sticker::is_clear - tells whether a sprite is "empty"
 */
bool Sticker::is_clear ()
{
  return ! priv->has_data;
}


/*
 *	Sticker::clear - clear a Sticker
 */
void Sticker::clear ()
{
  priv->clear ();
}


/*
 *	Sticker::load - load an Img into a Sticker
 */
void Sticker::load (const Img& img, bool opaque)
{
  priv->load (img, opaque);
}


/*
 *	Sticker::draw - draw a Sticker
 */
void Sticker::draw (Drawable drw, char grav, int x, int y)
{
  if (! priv->has_data)
    return;
  int x0 = grav == 'c' ? x - priv->width  / 2 : x;
  int y0 = grav == 'c' ? y - priv->height / 2 : y;
  if (priv->opaque)
  {
    XPutImage (dpy, drw, gc, priv->ximage, 0, 0,
      x0, y0, priv->width, priv->height);
  }
  else
  {
    XSetClipMask (dpy, gc, priv->mask);
    XSetClipOrigin (dpy, gc, x0, y0);
    XCopyArea (dpy, priv->pixmap, drw, gc, 0, 0, priv->width,
      priv->height, x0, y0);
    XSetClipMask (dpy, gc, None);
  }
}


Sticker_priv::Sticker_priv ()
{
  has_data = false;
}


Sticker_priv::~Sticker_priv ()
{
  clear ();
}


void Sticker_priv::clear ()
{
  if (has_data)
  {
    if (opaque)
    {
      XDestroyImage (ximage);  // Also frees buf.
    }
    else
    {
      XFreePixmap (dpy, pixmap);
      XFreePixmap (dpy, mask);
    }
    has_data = false;
  }
}

 
void Sticker_priv::load (const Img& img, bool opaque)
{
  this->opaque = opaque;
  clear ();

  width  = img.width ();
  height = img.height ();
  if (width < 1 || height < 1)
    return;  // Can't create Pixmaps with null dimensions...

  if (opaque)
  {
    ximage = make_ximage (img);
    if (ximage != 0)
      has_data = true;
  }
  else
  {
    // Create a pixmap and copy the image onto it.
    pixmap = XCreatePixmap (dpy, win, width, height, win_depth);
    XImage *image = make_ximage (img);
    if (image != 0)
    {
      XPutImage (dpy, pixmap, gc, image, 0, 0, 0, 0, width, height);
      XDestroyImage (image);
    }

    // Create the mask pixmap and copy the silhouette onto it.
    mask = XCreatePixmap (dpy, win, width, height, 1);
    XImage *mask_image = make_bitmap (img);
    if (mask_image != 0)
    {
      GC mask_gc = XCreateGC (dpy, mask, 0, 0);
      XSetBackground (dpy, mask_gc, 0);
      XSetForeground (dpy, mask_gc, 1);
      XPutImage (dpy, mask, mask_gc, mask_image, 0, 0, 0, 0, width, height);
      XFreeGC (dpy, mask_gc);
      XDestroyImage (mask_image);
    }

    has_data = true;
  }
}


/*
 *	Sticker_priv:ximage - create XImage from Img
 *
 *	Return pointer to XImage on success, null pointer on
 *	failure.
 */
XImage *Sticker_priv::make_ximage (const Img& img)
{
  // How many bytes per line do we need ?
  size_t bytes_per_line = width * ximage_bpp;
  if (ximage_quantum == 0)  // Paranoia
  {
    nf_bug ("ximage_quantum == 0");
    ximage_quantum = 1;
  }
  size_t padding = 0;
  while (bytes_per_line % ximage_quantum)
  {
    bytes_per_line++;
    padding++;
  }
  uint8_t *buf = (uint8_t *) malloc((unsigned long) bytes_per_line * height);
  if (! buf)
  {
    err ("Not enough memory to display %dx%d image", width, height);
    return 0;
  }

  // Copy the Img onto the XImage.
  const img_pixel_t *image_end = img.buf () + width * height;
  if (ximage_bpp == 1)
  {
    const img_pixel_t *image_ptr;
    uint8_t *buf_ptr = (uint8_t *) buf;
    if (padding == 0)
    {
      for (image_ptr = img.buf (); image_ptr < image_end; image_ptr++)
	*buf_ptr++ = (uint8_t) game_colour[*image_ptr];
    }
    else
    {
      const img_pixel_t *image_line_end;
      int img_width = width;
      for (image_ptr = image_line_end = img.buf (); image_ptr < image_end;)
      {
	image_line_end += img_width;
	for (; image_ptr < image_line_end; image_ptr++)
	  *buf_ptr++ = (uint8_t) game_colour[*image_ptr];
	buf_ptr += padding;  // Line padding
      }
    }
  }

  else if (ximage_bpp == 2)
  {
    const img_pixel_t *image_ptr;
    uint16_t *buf_ptr = (uint16_t *) buf;
    if (cpu_big_endian == x_server_big_endian)
    {
      if (padding == 0)
      {
	for (image_ptr = img.buf (); image_ptr < image_end; image_ptr++)
	  *buf_ptr++ = (uint16_t) game_colour[*image_ptr];
      }
      else
      {
	const img_pixel_t *image_line_end;
	int img_width = width;
	for (image_ptr = image_line_end = img.buf (); image_ptr < image_end;)
	{
	  image_line_end += img_width;
	  for (; image_ptr < image_line_end; image_ptr++)
	    *buf_ptr++ = (uint16_t) game_colour[*image_ptr];
	  buf_ptr = (uint16_t *) ((char *) buf_ptr + padding);  // Line padding
	}
      }
    }
    else  // Different endiannesses so swap bytes
    {
      if (padding == 0)
      {
	for (image_ptr = img.buf (); image_ptr < image_end; image_ptr++)
	   *buf_ptr++ = (uint16_t) (
		 (game_colour[*image_ptr] >> 8)  // Assume game_colour unsigned
	       | (game_colour[*image_ptr] << 8));
      }
      else
      {
	const img_pixel_t *image_line_end;
	int img_width = width;
	for (image_ptr = image_line_end = img.buf (); image_ptr < image_end;)
	{
	  image_line_end += img_width;
	  for (; image_ptr < image_line_end; image_ptr++)
	     *buf_ptr++ = (uint16_t) (
		   (game_colour[*image_ptr] >> 8)  // Assume game_colour uns.
		 | (game_colour[*image_ptr] << 8));
	  buf_ptr = (uint16_t *) ((char *) buf_ptr + padding);  // Line padding
	}
      }
    }
  }

  else if (ximage_bpp == 3)
  {
    const pv24_t *const pixel_value = game_colour_24.lut ();
    const img_pixel_t *image_ptr;
    pv24_t *buf_ptr = (pv24_t *) buf;
    if (padding == 0)
    {
      for (image_ptr = img.buf (); image_ptr < image_end; image_ptr++)
	memcpy (buf_ptr++, pixel_value + *image_ptr, sizeof *buf_ptr);
    }
    else
    {
      const img_pixel_t *image_line_end;
      int img_width = width;
      for (image_ptr = image_line_end = img.buf (); image_ptr < image_end;)
      {
	image_line_end += img_width;
	for (; image_ptr < image_line_end; image_ptr++)
	  memcpy (buf_ptr++, pixel_value + *image_ptr, sizeof *buf_ptr);
	buf_ptr = (pv24_t *) ((char *) buf_ptr + padding);  // Line padding
      }
    }
  }

  else if (ximage_bpp == 4)
  {
    const img_pixel_t *image_ptr;
    uint32_t *buf_ptr = (uint32_t *) buf;
    if (cpu_big_endian == x_server_big_endian)
    {
      if (padding == 0)
      {
	for (image_ptr = img.buf (); image_ptr < image_end; image_ptr++)
	  *buf_ptr++ = (uint32_t) game_colour[*image_ptr];
      }
      else
      {
	const img_pixel_t *image_line_end;
	int img_width = width;
	for (image_ptr = image_line_end = img.buf (); image_ptr < image_end;)
	{
	  image_line_end += img_width;
	  for (; image_ptr < image_line_end; image_ptr++)
	    *buf_ptr++ = (uint32_t) game_colour[*image_ptr];
	  buf_ptr = (uint32_t *) ((char *) buf_ptr + padding);  // Line padding
	}
      }
    }
    else  // Different endiannesses so swap bytes
    {
      if (padding == 0)
      {
	for (image_ptr = img.buf (); image_ptr < image_end; image_ptr++)
	  *buf_ptr++ = (uint32_t) (
	      (game_colour[*image_ptr] >> 24)  // Assume game_colour unsigned
	    | ((game_colour[*image_ptr] >> 8) & 0x0000ff00)
	    | ((game_colour[*image_ptr] << 8) & 0x00ff0000)
	    | (game_colour[*image_ptr] << 24));
      }
      else
      {
	const img_pixel_t *image_line_end;
	int img_width = width;
	for (image_ptr = image_line_end = img.buf (); image_ptr < image_end;)
	{
	  image_line_end += img_width;
	  for (; image_ptr < image_line_end; image_ptr++)
	    *buf_ptr++ = (uint32_t) (
		(game_colour[*image_ptr] >> 24)  // Assume game_colour uns.
	      | ((game_colour[*image_ptr] >> 8) & 0x0000ff00)
	      | ((game_colour[*image_ptr] << 8) & 0x00ff0000)
	      | (game_colour[*image_ptr] << 24));
	  buf_ptr = (uint32_t *) ((char *) buf_ptr + padding);  // Line padding
	}
      }
    }
  }

  XImage *ximage = XCreateImage (dpy, win_vis, win_depth, ZPixmap, 0,
    (char *) buf, width, height, 8, bytes_per_line);
  if (ximage == 0)
  {
    err ("XCreateImage() returned NULL");
    free(buf);
  }
  else
  {
    if ((ximage->byte_order == LSBFirst && ! x_server_big_endian)
     || (ximage->byte_order == MSBFirst && x_server_big_endian))
      ;  // OK
    else
      warn ("image byte_order %d doesn't match X server endianness\n",
	  ximage->byte_order);
  }
  return ximage;
}


/*
 *	Sticker_priv::make_bitmap - create 1-bpp XImage from Img
 *
 *	Return pointer on XImage on success, null pointer on
 *	failure.
 */
XImage *Sticker_priv::make_bitmap (const Img& img)
{
  int width  = img.width ();
  int height = img.height ();

  // How many bytes per line do we need ?
  if (CHAR_BIT != 8)  // Pure paranoia
  {
    nf_bug ("Panic: CHAR_BIT != 8 (%d)", (int) CHAR_BIT);
    return 0;
  }
  const size_t bitmap_pad     = BitmapPad (dpy);
  const size_t bytes_per_line = (width + bitmap_pad - 1) / bitmap_pad
				* bitmap_pad / CHAR_BIT;
  uint8_t *buf = (uint8_t *) malloc((unsigned long) bytes_per_line * height);
  if (! buf)
  {
    err ("Not enough memory to display %dx%d image", width, height);
    return 0;
  }

  // Copy the "profile" of the Img onto the XImage
  {
    int bitmap_bit_order = BitmapBitOrder (dpy);
    uint8_t *buf_ptr = (uint8_t *) buf;
    const int IMG_TRANSP = 0;
    const img_pixel_t       *image_ptr = img.buf ();
    const img_pixel_t *const image_end = image_ptr + width * height;

    while (image_ptr < image_end)
    {
      const uint8_t *src   = image_ptr;
      const uint8_t *const    stop1 = src + width - 7;
      const uint8_t *const    stop2 = image_ptr + width;
      uint8_t* dest = buf_ptr;
      if (bitmap_bit_order == LSBFirst)
      {
	while (src < stop1)
	{
	  uint8_t d = 0;
	  if (*src++ != IMG_TRANSP) d |= 1;
	  if (*src++ != IMG_TRANSP) d |= 2;
	  if (*src++ != IMG_TRANSP) d |= 4;
	  if (*src++ != IMG_TRANSP) d |= 8;
	  if (*src++ != IMG_TRANSP) d |= 16;
	  if (*src++ != IMG_TRANSP) d |= 32;
	  if (*src++ != IMG_TRANSP) d |= 64;
	  if (*src++ != IMG_TRANSP) d |= 128;
	  *dest++ = d;
	}
	if (src < stop2)
	{
	  uint8_t d = 0;
	  if (*src++ != IMG_TRANSP) d |= 1;   if (src >= stop2) goto eol_le;
	  if (*src++ != IMG_TRANSP) d |= 2;   if (src >= stop2) goto eol_le;
	  if (*src++ != IMG_TRANSP) d |= 4;   if (src >= stop2) goto eol_le;
	  if (*src++ != IMG_TRANSP) d |= 8;   if (src >= stop2) goto eol_le;
	  if (*src++ != IMG_TRANSP) d |= 16;  if (src >= stop2) goto eol_le;
	  if (*src++ != IMG_TRANSP) d |= 32;  if (src >= stop2) goto eol_le;
	  if (*src++ != IMG_TRANSP) d |= 64;  if (src >= stop2) goto eol_le;
	  if (*src++ != IMG_TRANSP) d |= 128;
	  eol_le:
	  *dest = d;
	}
      }
      else if (bitmap_bit_order == MSBFirst)
      {
	while (src < stop1)
	{
	  uint8_t d = 0;
	  if (*src++ != IMG_TRANSP) d |= 128;
	  if (*src++ != IMG_TRANSP) d |= 64;
	  if (*src++ != IMG_TRANSP) d |= 32;
	  if (*src++ != IMG_TRANSP) d |= 16;
	  if (*src++ != IMG_TRANSP) d |= 8;
	  if (*src++ != IMG_TRANSP) d |= 4;
	  if (*src++ != IMG_TRANSP) d |= 2;
	  if (*src++ != IMG_TRANSP) d |= 1;
	  *dest++ = d;
	}
	if (src < stop2)
	{
	  uint8_t d = 0;
	  if (*src++ != IMG_TRANSP) d |= 128; if (src >= stop2) goto eol_be;
	  if (*src++ != IMG_TRANSP) d |= 64;  if (src >= stop2) goto eol_be;
	  if (*src++ != IMG_TRANSP) d |= 32;  if (src >= stop2) goto eol_be;
	  if (*src++ != IMG_TRANSP) d |= 16;  if (src >= stop2) goto eol_be;
	  if (*src++ != IMG_TRANSP) d |= 8;   if (src >= stop2) goto eol_be;
	  if (*src++ != IMG_TRANSP) d |= 4;   if (src >= stop2) goto eol_be;
	  if (*src++ != IMG_TRANSP) d |= 2;   if (src >= stop2) goto eol_be;
	  if (*src++ != IMG_TRANSP) d |= 1;
	  eol_be:
	  *dest = d;
	}
      }
      buf_ptr += bytes_per_line;
      image_ptr += width;
    }
  }

  XImage *ximage = XCreateImage (dpy, win_vis, 1, XYBitmap, 0,
    (char *) buf, width, height, bitmap_pad, bytes_per_line);
  if (ximage == 0)
  {
    err ("XCreateImage() returned NULL");
    free(buf);
    return 0;
  }
  return ximage;
}
