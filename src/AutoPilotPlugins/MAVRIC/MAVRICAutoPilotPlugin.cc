/*=====================================================================
 
 QGroundControl Open Source Ground Control Station
 
 (c) 2009 - 2014 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 
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

#include "MAVRICAutoPilotPlugin.h"
//#include "AirframeComponent.h"
//#include "RadioComponent.h"
//#include "SensorsComponent.h"
//#include "FlightModesComponent.h"
//#include "AutoPilotPluginManager.h"
//#include "UASManager.h"
//#include "QGCUASParamManagerInterface.h"

/// @file
///     @brief This is the AutoPilotPlugin implementatin for the MAVRIC type.
///     @author Nicolas Dousse <nic.dousse@gmail.com>

MAVRICAutoPilotPlugin::MAVRICAutoPilotPlugin(UASInterface* uas, QObject* parent) :
    AutoPilotPlugin(uas, parent)
{
    Q_ASSERT(uas);


    _parameterFacts = new GenericParameterFacts(this, uas, this);
    Q_CHECK_PTR(_parameterFacts);
    
    connect(_parameterFacts, &GenericParameterFacts::parametersReady, this, &MAVRICAutoPilotPlugin::_parametersReady);
    connect(_parameterFacts, &GenericParameterFacts::parameterListProgress, this, &MAVRICAutoPilotPlugin::parameterListProgress);
}

MAVRICAutoPilotPlugin::~MAVRICAutoPilotPlugin()
{
    
}

void MAVRICAutoPilotPlugin::clearStaticData(void)
{
    // No Static data yet
}

const QVariantList& MAVRICAutoPilotPlugin::vehicleComponents(void)
{
    static QVariantList emptyList;
    
    return emptyList;
}

void MAVRICAutoPilotPlugin::_parametersReady(void)
{
    _pluginReady = true;
    emit pluginReadyChanged(_pluginReady);
}