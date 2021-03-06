/*
 Copyright (C) 2018 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "CyPy_Server.h"

#include "CyPy_Entity.h"
#include "CyPy_Task.h"
#include "CyPy_World.h"
#include "CyPy_TerrainProperty.h"
#include "CyPy_UsageInstance.h"
#include "CyPy_EntityProps.h"
#include "CyPy_Domain.h"


CyPy_Server::CyPy_Server() : ExtensionModule("server")
{

    add_noargs_method("get_limbo_location", &CyPy_Server::get_limbo_location, "The limbo location, if set.");
    add_varargs_method("move_to_spawn", &CyPy_Server::move_to_spawn, "Configures the supplied Location for the named spawn.");


    CyPy_Entity::init_type();
    CyPy_EntityProps::init_type();
    CyPy_Task::init_type();
    CyPy_World::init_type();

    CyPy_TerrainProperty::init_type();

    CyPy_Usage::init_type();
    CyPy_UsageInstance::init_type();
    CyPy_Domain::init_type();

    initialize("server");

    Py::Dict d(moduleDictionary());
    d["Thing"] = CyPy_Entity::type();
    d["Task"] = CyPy_Task::type();

    d["OPERATION_IGNORED"] = Py::Long(OPERATION_IGNORED);
    d["OPERATION_HANDLED"] = Py::Long(OPERATION_HANDLED);
    d["OPERATION_BLOCKED"] = Py::Long(OPERATION_BLOCKED);


}

void CyPy_Server::registerWorld(BaseWorld* world)
{
    Py::Module server("server");
    server.setAttr("world", CyPy_World::wrap(world));
}

std::string CyPy_Server::init()
{
    PyImport_AppendInittab("server", []() {
        static auto server = new CyPy_Server();
        return server->module().ptr();
    });
    return "server";
}

Py::Object CyPy_Server::get_limbo_location()
{
    auto limboLocation = BaseWorld::instance().getLimboLocation();
    if (!limboLocation) {
        return Py::None();
    }
    return CyPy_LocatedEntity::wrap(Ref<LocatedEntity>(limboLocation));
}

Py::Object CyPy_Server::move_to_spawn(const Py::Tuple& args)
{
    args.verify_length(2);
    auto spawnName = verifyString(args.front());
    auto& location = verifyObject<CyPy_Location>(args[1]);

    BaseWorld::instance().moveToSpawn(spawnName, location);

    return Py::None();
}
