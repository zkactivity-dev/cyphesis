// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000-2004 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#include "Inheritance.h"

#include "log.h"
#include "TypeNode.h"
#include "compose.hpp"

#include <Atlas/Objects/Entity.h>

using Atlas::Message::Element;
using Atlas::Message::ListType;

using Atlas::Objects::Root;

using Atlas::Objects::Operation::Login;
using Atlas::Objects::Operation::Logout;
using Atlas::Objects::Operation::Action;
using Atlas::Objects::Operation::Affect;
using Atlas::Objects::Operation::Create;
using Atlas::Objects::Operation::Delete;
using Atlas::Objects::Operation::Imaginary;
using Atlas::Objects::Operation::Info;
using Atlas::Objects::Operation::Move;
using Atlas::Objects::Operation::Set;
using Atlas::Objects::Operation::Get;
using Atlas::Objects::Operation::Sight;
using Atlas::Objects::Operation::Sound;
using Atlas::Objects::Operation::Touch;
using Atlas::Objects::Operation::Talk;
using Atlas::Objects::Operation::Use;
using Atlas::Objects::Operation::Wield;
using Atlas::Objects::Operation::Look;
using Atlas::Objects::Operation::Error;
using Atlas::Objects::Operation::Use;
using Atlas::Objects::Operation::Wield;

template<> Inheritance* Singleton<Inheritance>::ms_Singleton = nullptr;

Root atlasOpDefinition(const std::string & name, const std::string & parent)
{
    Atlas::Objects::Entity::Anonymous r;

    r->setParent(parent);
    r->setObjtype("op_definition");
    r->setId(name);

    return r;
}

Root atlasClass(const std::string & name, const std::string & parent)
{
    Atlas::Objects::Entity::Anonymous r;

    r->setParent(parent);
    r->setObjtype("class");
    r->setId(name);

    return r;
}

Root atlasType(const std::string & name,
               const std::string & parent,
               bool abstract)
{
    Atlas::Objects::Entity::Anonymous r;

    r->setParent(parent);
    r->setObjtype(abstract ? "data_type" : "type");
    r->setId(name);

    return r;
}

Inheritance::Inheritance() : noClass(nullptr)
{
    Atlas::Objects::Entity::Anonymous root_desc;

    root_desc->setObjtype("meta");
    root_desc->setId("root");

    auto root = new TypeNode("root", root_desc);

    atlasObjects["root"] = root;

    installStandardObjects(*this);
    installCustomOperations(*this);
    installCustomEntities(*this);
}


Inheritance::~Inheritance()
{
    flush();
}

void Inheritance::flush()
{
    for (const auto& entry : atlasObjects) {
        delete entry.second;
    }
    atlasObjects.clear();
}

const Root & Inheritance::getClass(const std::string & parent, Visibility visibility) const
{
    auto I = atlasObjects.find(parent);
    if (I == atlasObjects.end()) {
        return noClass;
    }
    return I->second->description(visibility);
}

int Inheritance::updateClass(const std::string & parent,
                             const Root & description)
{
    auto I = atlasObjects.find(parent);
    if (I == atlasObjects.end()) {
        return -1;
    }
    TypeNode * tn = I->second;
    if (tn->description(Visibility::PRIVATE)->getParent() != description->getParent()) {
        return -1;
    }
    tn->setDescription(description);
    return 0;
}

const TypeNode * Inheritance::getType(const std::string & parent) const
{
    auto I = atlasObjects.find(parent);
    if (I == atlasObjects.end()) {
        return nullptr;
    }
    return I->second;
}

bool Inheritance::hasClass(const std::string & parent)
{
    auto I = atlasObjects.find(parent);
    return !(I == atlasObjects.end());
}

TypeNode * Inheritance::addChild(const Root & obj)
{
    assert(obj.isValid() && !obj->getParent().empty());
    const std::string & child = obj->getId();
    const std::string & parent = obj->getParent();
    auto I = atlasObjects.find(child);
    auto Iend = atlasObjects.end();
    if (I != Iend) {

        const TypeNode * existingParent = I->second->parent();
        log(ERROR, String::compose("Installing %1 \"%2\"(parent \"%3\") "
                                   "which was already installed as a %4 with parent \"%5\"",
                                   obj->getObjtype(), child, parent,
                                   I->second->description(Visibility::PRIVATE)->getObjtype(),
                                   existingParent ? existingParent->name() : "NON"));
        return nullptr;
    }
    I = atlasObjects.find(parent);
    if (I == Iend) {
        log(ERROR, String::compose("Installing %1 \"%2\" "
                                   "which has unknown parent \"%3\".",
                                   obj->getObjtype(), child, parent));
        return nullptr;
    }
    Element children(ListType(1, child));

    auto description = I->second->description(Visibility::PRIVATE);

    if (description->copyAttr("children", children) == 0) {
        assert(children.isList());
        children.asList().push_back(child);
    }
    description->setAttr("children", children);
    I->second->setDescription(description);

    auto* type = new TypeNode(child, obj);
    type->setParent(I->second);

    atlasObjects.insert(std::make_pair(child, type));

    return type;
}

bool Inheritance::isTypeOf(const std::string & instance,
                           const std::string & base_type) const
{
    auto I = atlasObjects.find(instance);
    auto Iend = atlasObjects.end();
    if (I == Iend) {
        return false;
    }
    return I->second->isTypeOf(base_type);
}

bool Inheritance::isTypeOf(const TypeNode * instance,
                           const std::string & base_type) const
{
    return instance->isTypeOf(base_type);
}

bool Inheritance::isTypeOf(const TypeNode * instance,
                           const TypeNode * base_type) const
{
    return instance->isTypeOf(base_type);
}

using Atlas::Objects::Operation::RootOperation;
using Atlas::Objects::Operation::Perception;
using Atlas::Objects::Operation::Communicate;
using Atlas::Objects::Operation::Perceive;
using Atlas::Objects::Operation::Smell;
using Atlas::Objects::Operation::Feel;
using Atlas::Objects::Operation::Listen;
using Atlas::Objects::Operation::Sniff;

using Atlas::Objects::Entity::RootEntity;
using Atlas::Objects::Entity::AdminEntity;
using Atlas::Objects::Entity::Game;
using Atlas::Objects::Entity::GameEntity;

void installStandardObjects(TypeStore & i)
{
    i.addChild(atlasOpDefinition("root_operation", "root"));
    i.addChild(atlasOpDefinition("action", "root_operation"));
    i.addChild(atlasOpDefinition("create", "action"));
    i.addChild(atlasOpDefinition("delete", "action"));
    i.addChild(atlasOpDefinition("info", "root_operation"));
    i.addChild(atlasOpDefinition("set", "action"));
    i.addChild(atlasOpDefinition("get", "action"));
    i.addChild(atlasOpDefinition("perception", "info"));
    i.addChild(atlasOpDefinition("error", "info"));
    i.addChild(atlasOpDefinition("communicate", "create"));
    i.addChild(atlasOpDefinition("move", "set"));
    i.addChild(atlasOpDefinition("affect", "set"));
    i.addChild(atlasOpDefinition("perceive", "get"));
    i.addChild(atlasOpDefinition("login", "get"));
    i.addChild(atlasOpDefinition("logout", "login"));
    i.addChild(atlasOpDefinition("sight", "perception"));
    i.addChild(atlasOpDefinition("sound", "perception"));
    i.addChild(atlasOpDefinition("smell", "perception"));
    i.addChild(atlasOpDefinition("feel", "perception"));
    i.addChild(atlasOpDefinition("imaginary", "action"));
    i.addChild(atlasOpDefinition("talk", "communicate"));
    i.addChild(atlasOpDefinition("look", "perceive"));
    i.addChild(atlasOpDefinition("listen", "perceive"));
    i.addChild(atlasOpDefinition("sniff", "perceive"));
    i.addChild(atlasOpDefinition("touch", "perceive"));
    i.addChild(atlasOpDefinition("appearance", "sight"));
    i.addChild(atlasOpDefinition("disappearance", "sight"));
    i.addChild(atlasOpDefinition("use", "action"));
    i.addChild(atlasOpDefinition("wield", "set"));

    i.addChild(atlasClass("root_entity", "root"));
    i.addChild(atlasClass("admin_entity", "root_entity"));
    i.addChild(atlasClass("account", "admin_entity"));
    i.addChild(atlasClass("player", "account"));
    i.addChild(atlasClass("admin", "account"));
    i.addChild(atlasClass("game", "admin_entity"));
    i.addChild(atlasClass("game_entity", "root_entity"));

    i.addChild(atlasClass("root_type", "root"));

    // And from here on we need to define the hierarchy as found in the C++
    // base classes. Script classes defined in rulesets need to be added
    // at runtime.
}
