/*
 *	s_prop.cc
 *	Sector properties
 *	Some of this was originally in editobj.c. It was moved here to
 *	improve overlay granularity (therefore memory consumption).
 *	AYM 1998-02-07
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
#include "entry.h"
#include "flats.h"
#include "game.h"
#include "gfx.h"
#include "levels.h"
#include "menudata.h"
#include "objid.h"
#include "oldmenus.h"
#include "selectn.h"


class Menu_data_st : public Menu_data
{
  public :
    Menu_data_st (al_llist_t *list);
    virtual size_t nitems () const;
    virtual const char *operator[] (size_t n) const;

  private :
    mutable char buf[100];
    al_llist_t *list;
};


/*
 *	Menu_data_st::Menu_data_st - ctor
 */
Menu_data_st::Menu_data_st (al_llist_t *list) : list (list)
{
  al_lrewind (this->list);
}


/*
 *	Menu_data_st::nitems - return the number of items
 */
size_t Menu_data_st::nitems () const
{
  return al_lcount (list);
}


/*
 *	Menu_data_st::operator[] - return the nth item
 */
const char *Menu_data_st::operator[] (size_t n) const
{
  if (al_lseek (list, n, SEEK_SET) != 0)
  {
    snprintf (buf, sizeof(buf), "BUG: al_lseek(%p, %lu): %s",
      (void *) list, 
      (unsigned long) n,
      al_astrerror (al_aerrno));
    return buf;
  }
  const stdef_t *ptr = (const stdef_t *) al_lptr (list);
  if (ptr == NULL)
    snprintf (buf, sizeof(buf), "BUG: al_lptr(%p): %s",
      (void *) list,
      al_astrerror (al_aerrno));
  else
    snprintf (buf, sizeof(buf), "%2d - %.70s", ptr->number, ptr->longdesc);
  return buf;
}


/*
 *	SectorProperties
 *	Sector properties "dialog"
 */
void SectorProperties (int x0, int y0, SelPtr obj)
{
  char  *menustr[30];
  string texname;
  int    n, val;
  SelPtr cur;
  int    subwin_y0;

#define ILEN 60
  for (n = 0; n < 8; n++)
    menustr[n] = (char *) malloc(ILEN);
  snprintf (menustr[7], ILEN, "Edit sector #%d", obj->objnum);
  snprintf (menustr[0], ILEN, "Change floor height     (Current: %d)",
	  Sectors[obj->objnum].floorh);
  snprintf (menustr[1], ILEN, "Change ceiling height   (Current: %d)",
	  Sectors[obj->objnum].ceilh);
  snprintf (menustr[2], ILEN, "Change floor texture    (Current: %.*s)",
	  (int) WAD_FLAT_NAME, Sectors[obj->objnum].floort);
  snprintf (menustr[3], ILEN, "Change ceiling texture  (Current: %.*s)",
	  (int) WAD_FLAT_NAME, Sectors[obj->objnum].ceilt);
  snprintf (menustr[4], ILEN, "Change light level      (Current: %d)",
	  Sectors[obj->objnum].light);
  snprintf (menustr[5], ILEN, "Change type             (Current: %d)",
	  Sectors[obj->objnum].special);
  snprintf (menustr[6], ILEN, "Change linedef tag      (Current: %d)",
	  Sectors[obj->objnum].tag);
  val = vDisplayMenu (x0, y0, menustr[7],
    menustr[0], YK_, 0,
    menustr[1], YK_, 0,
    menustr[2], YK_, 0,
    menustr[3], YK_, 0,
    menustr[4], YK_, 0,
    menustr[5], YK_, 0,
    menustr[6], YK_, 0,
    NULL);
  for (n = 0; n < 8; n++)
    free(menustr[n]);
  subwin_y0 = y0 + BOX_BORDER + (2 + val) * FONTH;
  switch (val)
  {
    case 1:
      val = InputIntegerValue (x0 + 42, subwin_y0, -32768, 32767,
	Sectors[obj->objnum].floorh);
      if (val != IIV_CANCEL)
      {
	for (cur = obj; cur; cur = cur->next)
	  Sectors[cur->objnum].floorh = val;
	MadeChanges = 1;
      }
      break;

    case 2:
      val = InputIntegerValue (x0 + 42, subwin_y0, -32768, 32767,
	Sectors[obj->objnum].ceilh);
      if (val != IIV_CANCEL)
      {
	for (cur = obj; cur; cur = cur->next)
	  Sectors[cur->objnum].ceilh = val;
	MadeChanges = 1;
      }
      break;

	 case 3:
		{
			vector<string> flat_names;
			for (size_t n = 0; n < NumFTexture; n++) {
				flat_names.push_back(string(flat_list[n].name));
			}
			texname = ChooseFloorTexture(x0 + 42, subwin_y0, "Choose a floor texture",
					flat_names, string(Sectors[obj->objnum].floort));
			if (texname != "") {
				for (cur = obj; cur; cur = cur->next)
					strncpy (Sectors[cur->objnum].floort, texname.c_str(), WAD_FLAT_NAME);
				MadeChanges = 1;
			}
			break;
		}

	 case 4:
		{
			vector<string> flat_names;
			for (size_t n = 0; n < NumFTexture; n++) {
				flat_names.push_back(string(flat_list[n].name));
			}
			texname = ChooseFloorTexture(x0 + 42, subwin_y0,
					"Choose a ceiling texture", flat_names, string(Sectors[obj->objnum].ceilt));
			if (texname != "") {
				for (cur = obj; cur; cur = cur->next)
					strncpy (Sectors[cur->objnum].ceilt, texname.c_str(), WAD_FLAT_NAME);
				MadeChanges = 1;
			}
			break;
		}
    case 5:
      val = InputIntegerValue (x0 + 42, subwin_y0, 0, 255,
	Sectors[obj->objnum].light);
      if (val != IIV_CANCEL)
      {
	for (cur = obj; cur; cur = cur->next)
	  Sectors[cur->objnum].light = val;
	MadeChanges = 1;
      }
      break;

    case 6:
      {
	val = 0;
	Menu_data_st menudata (stdef);
	if (DisplayMenuList (x0 + 42, subwin_y0, "Select type", menudata, &val)
	  < 0)
	  break;
	// KLUDGE last element of stdef means "enter value"
	if (val == al_lcount (stdef) - 1)
	{
	  val = InputIntegerValue (x0 + 84,
	    subwin_y0 + BOX_BORDER + (3 + val) * FONTH,
	    -32768, 32767, 0);
	  if (val == IIV_CANCEL)  // [Esc]
	    break;
	}
	else
	{
	  if (al_lseek (stdef, val, SEEK_SET))
	    fatal_error ("%s SP1 (%s)\n",
	      msg_unexpected, al_astrerror (al_aerrno));
	  val = CUR_STDEF->number;
	}
	for (cur = obj; cur; cur = cur->next)
	  Sectors[cur->objnum].special = val;
	MadeChanges = 1;
	break;
      }

    case 7:
      val = InputIntegerValue (x0 + 42, subwin_y0, -32768, 32767,
	Sectors[obj->objnum].tag);
      if (val != IIV_CANCEL)
      {
	for (cur = obj; cur; cur = cur->next)
	  Sectors[cur->objnum].tag = val;
	MadeChanges = 1;
      }
      break;
  }
}


