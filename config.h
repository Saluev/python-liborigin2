/***************************************************************************
    File                 : config.h.in
    --------------------------------------------------------------------
    Copyright            : (C) 2010 Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : compile-time configuration defines

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#define LIBORIGIN_VERSION_MAJOR ${LIBORIGIN_VERSION_MAJOR}
#define LIBORIGIN_VERSION_MINOR ${LIBORIGIN_VERSION_MINOR}
#define LIBORIGIN_VERSION_BUGFIX ${LIBORIGIN_VERSION_BUGFIX}
#define LIBORIGIN_VERSION ((LIBORIGIN_VERSION_MAJOR << 24) | \
                           (LIBORIGIN_VERSION_MINOR << 16) | \
                           (LIBORIGIN_VERSION_BUGFIX << 8) )
#define LIBORIGIN_VERSION_STRING "${LIBORIGIN_VERSION_MAJOR}.${LIBORIGIN_VERSION_MINOR}.${LIBORIGIN_VERSION_BUGFIX}"

#endif // ifndef CONFIG_H
