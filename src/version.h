/*
    WPN-XM Server Control Panel

    WPN-XM SCP is a tool to manage Nginx, PHP and MariaDb daemons under windows.
    It's a fork of Easy WEMP originally written by Yann Le Moigne and (c) 2010.
    WPN-XM SCP is written by Jens-Andre Koch and (c) 2011 - onwards.

    This file is part of WPN-XM Serverpack for Windows.

    WPN-XM SCP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    WPN-XM SCP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WPN-XM SCP. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VERSION_H_
#define VERSION_H_

// Application version.
// Its a token, replaced by Nant during the build process.
#define VERSION = @APPVERSION@
#define APP_VERSION "@APPVERSION@"

// Application name
#define APP_NAME "WPN-XM Server Control Panel"

// Application name and version
#define APP_NAME_AND_VERSION "WPN-XM Server Control Panel @APPVERSION@"

// Settings for application_win.rc
// These values are the attributes of the executable, shown when right-clicking on it.
#define VER_FILEVERSION             1,0,0,0
#define VER_FILEVERSION_STR         "1.0.0.0\0"

#define VER_PRODUCTVERSION          1,0,0,0
#define VER_PRODUCTVERSION_STR      "1.0\0"

#define VER_COMPANYNAME_STR         "Koch Softwaresystemtechnik"
#define VER_COMPANYDOMAIN_STR       "http://wpn-xm.org"
#define VER_FILEDESCRIPTION_STR     "WPN-XM Server Control Panel"
#define VER_INTERNALNAME_STR        "WPN-XM SCP"
#define VER_LEGALCOPYRIGHT_STR      "Copyright © 2013 Koch Softwaresystemtechnik"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "wpn-xm-scp.exe"
#define VER_PRODUCTNAME_STR         "WPN-XM Server Control Panel"

#endif /* VERSION_H_ */