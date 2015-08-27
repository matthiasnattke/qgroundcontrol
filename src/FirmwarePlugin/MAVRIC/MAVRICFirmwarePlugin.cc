/*=====================================================================
 
 QGroundControl Open Source Ground Control Station
 
 (c) 2009 - 2015 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 
 This file is part of the QGROUNDCONTROL project
 
 QGROUNDCONTROL is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 QGROUNDCONTROL is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.
 
 ======================================================================*/

/// @file
///     @author Don Gagne <don@thegagnes.com>

#include "MAVRICFirmwarePlugin.h"

#include <QDebug>

IMPLEMENT_QGC_SINGLETON(MAVRICFirmwarePlugin, FirmwarePlugin)

struct Bit2Name {
    uint8_t     baseModeBit;
    uint8_t     fullModeBit;
    const char* name;
};

typedef enum
{
    MAV_MODE_PRE = 0,                       ///< 0b00*00000
    MAV_MODE_SAFE = 64,                     ///< 0b01*00000
    MAV_MODE_ATTITUDE_CONTROL = 192,        ///< 0b11*00000
    MAV_MODE_VELOCITY_CONTROL = 208,        ///< 0b11*10000
    MAV_MODE_POSITION_HOLD = 216,           ///< 0b11*11000
    MAV_MODE_GPS_NAVIGATION = 156           ///< 0b10*11100
} mav_mode_predefined_t;

static const struct Bit2Name rgBit2Name[] = {
    { MAV_MODE_FLAG_DECODE_POSITION_MANUAL,      MAV_MODE_ATTITUDE_CONTROL,    "ATTITUDE" },
    { MAV_MODE_FLAG_DECODE_POSITION_STABILIZE,   MAV_MODE_VELOCITY_CONTROL,    "VElOCITY" },
    { MAV_MODE_FLAG_DECODE_POSITION_GUIDED,      MAV_MODE_POSITION_HOLD,       "POSITION_HOLD" },
    { MAV_MODE_FLAG_DECODE_POSITION_AUTO,        MAV_MODE_GPS_NAVIGATION,      "GPS_NAVIGATION" },
};

enum MAVRIC_MAV_MODE_CUSTOM
{
    MAVRIC_CUSTOM_BASE_MODE = 0,
    MAVRIC_CUSTOM_CLIMB_TO_SAFE_ALT = 1,                ///< First critical behavior
    MAVRIC_CUSTOM_FLY_TO_HOME_WP = 2,                   ///< Second critical behavior, comes after CLIMB_TO_SAFE_ALT
    MAVRIC_CUSTOM_CRITICAL_LAND = 4,                    ///< Third critical behavior, comes after FLY_TO_HOME_WP

    MAVRIC_CUSTOM_DESCENT_TO_SMALL_ALTITUDE = 8,        ///< First auto landing behavior
    MAVRIC_CUSTOM_DESCENT_TO_GND = 16,                  ///< Second auto landing behavior, comes after DESCENT_TO_SMAL_ALTITUDE
    
    MAVRIC_CUSTOM_COLLISION_AVOIDANCE = 32,             ///< Collision avoidance

    MAVRIC_CUSTOM_BATTERY_LOW = 64,                     ///< Battery low flag
    MAVRIC_CUSTOM_FENCE_1 = 128,                        ///< Fence 1 violation flag
    MAVRIC_CUSTOM_FENCE_2 = 256,                        ///< Fence 2 violation flag
    MAVRIC_CUSTOM_HEARTBEAT_LOST = 512,                 ///< Heartbeat loss flag
    MAVRIC_CUSTOM_REMOTE_LOST = 1024,                   ///< Remote lost flag
    MAVRIC_CUSTOM_GPS_BAD = 2048                        ///< GPS loss flag
};

MAVRICFirmwarePlugin::MAVRICFirmwarePlugin(QObject* parent) :
    FirmwarePlugin(parent)
{
    
}

QList<VehicleComponent*> MAVRICFirmwarePlugin::componentsForVehicle(AutoPilotPlugin* vehicle)
{
    Q_UNUSED(vehicle);
    
    return QList<VehicleComponent*>();
}

QString MAVRICFirmwarePlugin::flightMode(uint8_t base_mode, uint32_t custom_mode)
{
    QString flightMode;
    
    if (base_mode == 0) {
        flightMode = "PREFLIGHT";
    }
    else
    {
        if (base_mode & MAV_MODE_FLAG_DECODE_POSITION_HIL)
        {
            flightMode += "HIL:";
        }
        for (size_t i=sizeof(rgBit2Name)/sizeof(rgBit2Name[0]); i>=1; i--) {

            if (base_mode & rgBit2Name[i-1].baseModeBit) {
                if (i-1 != 0) {
                    flightMode += " ";
                }
                flightMode += rgBit2Name[i-1].name;
                break;
            }
        }

        if (base_mode & MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)
        {
            if (custom_mode & (MAVRIC_CUSTOM_BATTERY_LOW & MAVRIC_CUSTOM_FENCE_1 & MAVRIC_CUSTOM_FENCE_2 & MAVRIC_CUSTOM_HEARTBEAT_LOST & MAVRIC_CUSTOM_REMOTE_LOST & MAVRIC_CUSTOM_GPS_BAD))
            {
                flightMode += "|SAFETY_VIOLATION";
            } else if (custom_mode & MAVRIC_CUSTOM_CLIMB_TO_SAFE_ALT)
            {
                flightMode += "|CLIMB_TO_SAFE_ALT";
            } else if(custom_mode & MAVRIC_CUSTOM_FLY_TO_HOME_WP)
            {
                flightMode += "|FLY_TO_HOME";
            } else if(custom_mode & MAVRIC_CUSTOM_CRITICAL_LAND)
            {
                flightMode += "|CRITICAL_LANDING";
            } else if(custom_mode & MAVRIC_CUSTOM_DESCENT_TO_SMALL_ALTITUDE)
            {
                flightMode += "|DESCENDING";
            } else if(custom_mode & MAVRIC_CUSTOM_DESCENT_TO_GND)
            {
                flightMode += "|LANDING";
            }else if(custom_mode & MAVRIC_CUSTOM_COLLISION_AVOIDANCE)
            {
                flightMode += "|COLL_AVOID";
            }
        }
    }
    
    return flightMode;
}

QStringList MAVRICFirmwarePlugin::flightModes(void)
{
    QStringList flightModes;
    
    // FIXME: fixed-wing/multi-rotor differences?
    
    for (size_t i=0; i<sizeof(rgBit2Name)/sizeof(rgBit2Name[0]); i++) {
        flightModes += rgBit2Name[i].name;
    }
    
    return flightModes;
}

bool MAVRICFirmwarePlugin::setFlightMode(const QString& flightMode, uint8_t* base_mode, uint32_t* custom_mode)
{
    *base_mode = 0;
    *custom_mode = 0;
    
    bool found = false;
    for (size_t i=0; i<sizeof(rgBit2Name)/sizeof(rgBit2Name[0]); i++)
    {
        if (flightMode.compare(rgBit2Name[i].name, Qt::CaseInsensitive) == 0)
        {
            *base_mode = rgBit2Name[i].fullModeBit;
            *custom_mode = 0;
            
            found = true;
            break;
        }
    }
    
    if (!found) {
        qWarning() << "Unknown flight Mode" << flightMode;
    }
    
    return found;
}