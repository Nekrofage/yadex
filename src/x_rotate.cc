/*
 *	x_rotate.cc
 *	Rotate and scale a group of objects
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
#include <math.h>
#include "l_vertices.h"
#include "levels.h"
#include "objid.h"
#include "s_vertices.h"
#include "selectn.h"
#include "t_centre.h"
#include "v_centre.h"
#include "x_rotate.h"

/*
   rotate and scale a group of objects around the centre of gravity
*/

void RotateAndScaleObjects (int objtype, SelPtr obj, double angle, double scale) /* SWAP! */
{
    int    dx, dy;
    int    centerx, centery;
    SelPtr cur, vertices;

    if (obj == NULL)
        return;

    switch (objtype)
    {
        case OBJ_THINGS:
            centre_of_things (obj, &centerx, &centery);
            for (cur = obj; cur; cur = cur->next)
            {
                dx = Things[cur->objnum].xpos - centerx;
                dy = Things[cur->objnum].ypos - centery;
                RotateAndScaleCoords (&dx, &dy, angle, scale);
                Things[cur->objnum].xpos = centerx + dx;
                Things[cur->objnum].ypos = centery + dy;
            }
            MadeChanges = 1;
            break;

        case OBJ_VERTICES:
            centre_of_vertices (obj, &centerx, &centery);
            for (cur = obj; cur; cur = cur->next)
            {
                dx = Vertices[cur->objnum].x - centerx;
                dy = Vertices[cur->objnum].y - centery;
                RotateAndScaleCoords (&dx, &dy, angle, scale);
                Vertices[cur->objnum].x = (centerx + dx + 2) & ~ 3;
                Vertices[cur->objnum].y = (centery + dy + 2) & ~ 3;
            }
            MadeChanges = 1;
            MadeMapChanges = 1;
            break;

        case OBJ_LINEDEFS:
            vertices = list_vertices_of_linedefs (obj);
            RotateAndScaleObjects (OBJ_VERTICES, vertices, angle, scale);
            ForgetSelection (&vertices);
            break;

        case OBJ_SECTORS:
            vertices = list_vertices_of_sectors (obj);
            RotateAndScaleObjects (OBJ_VERTICES, vertices, angle, scale);
            ForgetSelection (&vertices);
            break;
    }
}
