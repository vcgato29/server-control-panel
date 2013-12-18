/*
    WPN-XM Server Control Panel

    WPN-XM SCP is a GUI tool for managing server daemons under Windows.
    It's a fork of Easy WEMP written by Yann Le Moigne and (c) 2010.
    WPN-XM SCP is written by Jens-Andre Koch and (c) 2011 - onwards.

    This file is part of WPN-XM Server Stack for Windows.

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

#include "QApplication"

// this is needed for "createShellLink"
#include <windows.h>
#include <objbase.h>
#include <shlobj.h> // type defintion for IShellLink

/*
 * Some access methods for Windows libraries
 */
namespace WindowsAPI
{
    /*
     * http://msdn.microsoft.com/en-us/library/windows/desktop/bb776891%28v=vs.85%29.aspx
     */
    IShellLink* CreateShellLink(QString target_app_path, QString app_args, QString description,
                                QString icon_path, int icon_index, QString working_dir,
                                QString linkShortcut)
    {
        IShellLink* shell_link = NULL;

        HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
                                      reinterpret_cast<void**> (&(shell_link)));

        if(SUCCEEDED(hres)) {

            IPersistFile* persistFile = NULL;

            shell_link->SetPath(target_app_path.toStdWString().c_str());
            shell_link->SetArguments(app_args.toStdWString().c_str());
            shell_link->SetIconLocation(icon_path.toStdWString().c_str(), icon_index);
            shell_link->SetDescription(description.toStdWString().c_str());
            shell_link->SetWorkingDirectory(working_dir.toStdWString().c_str());

            // Query IShellLink for the IPersistFile interface,
            // used for saving the shortcut in persistent storage.
            hres = shell_link->QueryInterface(IID_IPersistFile, reinterpret_cast<void**> (&(persistFile)));

            if (SUCCEEDED(hres)) {

                // Save the link by calling IPersistFile::Save.
                hres = persistFile->Save((LPCOLESTR)linkShortcut.toStdWString().c_str(), STGM_WRITE);

                // Release the pointer to the IPersistFile interface.
                persistFile->Release();
            }

            // Release the pointer to the IShellLink interface.
            shell_link->Release();
        }

        return shell_link;
    }
}
