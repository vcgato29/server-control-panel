/*
    WPN-XM Server Control Panel

    WPN-XM SCP is a tool to manage Nginx, PHP and MariaDB daemons under windows.
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

#ifndef PROPERTYTABLE_H
#define PROPERTYTABLE_H

// global includes
#include <QHash>
#include <QVariant>

/// Settings Table class implements a settings container.
/*!
    This is the general purpose class that designed to store the properties of
    objects. The number and types of properties can be vary depending on the object.
    The API allows to add, store and query any type of properties.
*/
class Setting;

class SettingsTable : public QObject
{
    Q_OBJECT

public:

    /// Default constructor
    SettingsTable(QObject *parent = 0);

    /// Copy constructor
    SettingsTable(const SettingsTable &other);

    /// Destructor performs clean up
    virtual ~SettingsTable();

    /// Adds new property with the given name and value.
    void addProperty(const QString &name,
                     const QVariant &value,
                     const QVariant &defaultValue = QVariant());

    /// Set/change the property value.
    /*!
        This function looks up for the property and, if it is found, sets the
        new property value if the value is valid, otherwise it resets the
        property to its default value and emits the signal about changes.
    */
    void setValue(const QString &name, const QVariant &value);

    /// Returns the property value.
    QVariant value(const QString &name) const;

    /// Reset the properties values to their default state.
    void resetToDefault();

    /// Returns the formated string with the properties and their values.
    /*!
        This function returns formated string that contains property-value pairs
        in format <name>:<tab><values><new_line>. The properties are
        alphabethically sorted by names.
    */
    QString toString() const;

    /// Returns the properties names mapped to the values.
    QMap<QString, QVariant> settingsMap() const;

signals:

    /// This signal emitted when the property values is changed
    void settingChanged(const QString &name, const QVariant &newValue);

protected:

    /// The internal class Setting represents the Setting itself.
    /// Name : Value : DefaultValue
    class Setting
    {

    public:

        /// Constructs the property with the given parameters.
        Setting(const QString &name,
                 const QVariant &value,
                 const QVariant &defaultValue)
            :
                m_name(name),
                m_value(value),
                m_defaultValue(defaultValue)
        {}

        /// Default constructor.
        Setting()
        {}

        /// Copy constructor
        Setting(const Setting &prop)
        {
            m_name = prop.m_name;
            m_value = prop.m_value;
            m_defaultValue = prop.m_defaultValue;
        }

        /// Destructor
        ~Setting()
        {}

        /// Returns property name.
        QString name() const
        {
            return m_name;
        }

        /// Returns property value.
        QVariant value() const
        {
            QString s = m_value.toString();
            return m_value;
        }

        /// Change the property value.
        void setValue(const QVariant &value)
        {
            m_value = value;
        }

        /// Reset property value to its default.
        void resetToDefault()
        {
            m_value = m_defaultValue;
        }

    private:

        /// Store the name.
        QString m_name;

        /// Store the value.
        QVariant m_value;

        /// Store the default value.
        QVariant m_defaultValue;
    };

    /// Store the properties
    QHash<QString, Setting> m_settings;
};

#endif // SETTINGSTABLE_H
