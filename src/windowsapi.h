#ifndef WINDOWSAPI_H
#define WINDOWSAPI_H

#include "windows.h"
#include <QtCore>

namespace WindowsAPI
{
    HRESULT CreateShellLink(QString target_app_path,
                            QString app_args,
                            QString description,
                            QString icon_path,
                            int icon_index,
                            QString working_dir,
                            QString linkShortcut);

    bool running_on_64_bits_os();
}

#endif // WINDOWSAPI_H
