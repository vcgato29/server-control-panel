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

// local includes
#include "settings.h"

// global includes
#include <QSettings>
#include <QApplication>
#include <QDebug>
#include <QDir>

const QString appSettingsFileName("wpn-xm.ini");

Settings::Settings(QObject *parent) :
    SettingsTable(parent)
{
}

Settings::~Settings()
{
}

QString Settings::file() const
{
    QString file = QCoreApplication::applicationDirPath() + '/' + appSettingsFileName;
    file = QDir::toNativeSeparators(file);
    qDebug() << "[Using Settings File]" << file;
    return file;
}

void Settings::saveSettings() const
{
    qDebug() << "[Save Settings]";

    QHash<QString, Setting>::const_iterator i = settings.constBegin();

    // Create a QSettings object. Use the INI format to store data.
    QSettings settingsWriter(file(), QSettings::IniFormat);

    while (i != settings.constEnd())
    {
        qDebug() << i.value().name();
        settingsWriter.setValue(i.value().name(), i.value().value());
        ++i;
    }
}

void Settings::readSettings()
{
     qDebug() << "[Read Settings]";

    // Create a QSettings object. Use the INI format to read data.
    QSettings settingsReader(file(), QSettings::IniFormat);

    QHash<QString, Setting>::iterator i = settings.begin();

    while (i != settings.end())
    {
        QString settingName = i.key();
        QVariant settingValue = settingsReader.value(settingName);

        // set the new value we read from the file to the SettingsTable
        setValue(settingName, settingValue);
        ++i;
    }
}
