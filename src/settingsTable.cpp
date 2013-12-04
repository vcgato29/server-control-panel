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
#include "settingsTable.h"

SettingsTable::SettingsTable(QObject *parent) : QObject(parent)
{}

SettingsTable::SettingsTable(const SettingsTable &other, QObject *parent) : QObject(parent)
{
    // reparent
    setParent(other.parent());

    // just copy one container from another
    m_settings = other.m_settings;
}

SettingsTable::~SettingsTable()
{
    // clear properties
    m_settings.clear();
}

void SettingsTable::addProperty(const QString &name,
                                const QVariant &value,
                                const QVariant &defaultValue)
{
    Setting pD(name, value, defaultValue);
    m_settings[name] = pD;
}

QVariant SettingsTable::value(const QString &name) const
{
    if (m_settings.contains(name))
    {
        Setting setting = m_settings.value(name);
        return setting.value();
    }
    else
        return QVariant();
}

void SettingsTable::setValue(const QString &name, const QVariant &value)
{
    QHash<QString, Setting>::iterator i = m_settings.find(name);

    if(i != m_settings.end() && i.key() == name)
    {
        if(value.isValid())
            i.value().setValue(value);
        else
            i.value().resetToDefault();

        // emit signal about default changes
        emit settingChanged(name, i.value().value());
    }
}

void SettingsTable::resetToDefault()
{
    QHash<QString, Setting>::iterator i = m_settings.begin();

    while (i != m_settings.end())
    {
        // Reset the property value
        i.value().resetToDefault();

        // emit signal about property changes
        emit settingChanged(i.value().name(), i.value().value());
        ++i;
    }
}

QString SettingsTable::toString() const
{
    // sort settings by their names
    QMap<QString, QVariant> sortedSettings = settingsMap();

    // iterate over sorted settings
    QMap<QString, QVariant>::const_iterator i = sortedSettings.constBegin();
    QString ret;

    while (i != sortedSettings.constEnd())
    {
        // construct string using sorted setting values
        ret.append(QString("%1:\t%2\n").arg(i.key()).arg(i.value().toString()));
        ++i;
    }

    return ret;
}

QMap<QString, QVariant> SettingsTable::settingsMap() const
{
    QHash<QString, Setting>::const_iterator i = m_settings.constBegin();
    QMap<QString, QVariant> settingsMap;

    while (i != m_settings.constEnd())
    {
        Setting setting = i.value();
        settingsMap[setting.name()] = setting.value();
        ++i;
    }

    return settingsMap;
}
