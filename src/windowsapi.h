#ifndef WINDOWSAPI_H
#define WINDOWSAPI_H

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
}

#endif // WINDOWSAPI_H
