// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2004 Alistair Riddoch
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


#include "TerrainProperty.h"
#include "rules/LocatedEntity.h"
#include "rules/Domain.h"

#include "rules/simulation/BaseWorld.h"
#include "common/debug.h"
#include "common/custom.h"
#include "common/TypeNode.h"

#include "modules/TerrainContext.h"

#include <Mercator/Terrain.h>
#include <Mercator/Segment.h>
#include <Mercator/Surface.h>
#include <Mercator/TileShader.h>
#include <Mercator/FillShader.h>
#include <Mercator/ThresholdShader.h>
#include <Mercator/DepthShader.h>
#include <Mercator/GrassShader.h>

#include <Atlas/Objects/Anonymous.h>

static const bool debug_flag = false;

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;
using Atlas::Message::FloatType;
using Atlas::Objects::Entity::Anonymous;

typedef Mercator::Terrain::Pointstore Pointstore;
typedef Mercator::Terrain::Pointcolumn Pointcolumn;

TerrainProperty::TerrainProperty(const TerrainProperty& rhs) :
    PropertyBase(rhs),
    m_data(new Mercator::Terrain(Mercator::Terrain::SHADED))
{
    //Copy all points.
    for (auto& pointColumn : rhs.m_data->getPoints()) {
        for (auto& point : pointColumn.second) {
            m_data->setBasePoint(pointColumn.first, point.first, point.second);
        }
    }

    //Copy surface if available, as well as surface data.
    if (!rhs.m_surfaces.empty()) {
        m_surfaces = rhs.m_surfaces;
        m_tileShader = createShaders(m_surfaces);
        m_data->addShader(m_tileShader.get(), 0);
    }
}

/// \brief TerrainProperty constructor
TerrainProperty::TerrainProperty() :
      m_data(new Mercator::Terrain(Mercator::Terrain::SHADED))

{
}

TerrainProperty::~TerrainProperty() = default;

int TerrainProperty::get(Element & ent) const
{
    MapType & t = (ent = MapType()).asMap();
    MapType & terrain = (t["points"] = MapType()).asMap();

    const Pointstore & points = m_data->getPoints();
    for (const auto& column : points) {
        for (const auto point : column.second) {
            const Mercator::BasePoint& bp = point.second;
            std::stringstream key;
            key << column.first << "x" << point.first;
            size_t size = 3;
            bool sendRoughness = false;
            bool sendFalloff = false;
            if (bp.falloff() != Mercator::BasePoint::FALLOFF) {
                size = 5;
                sendRoughness = true;
                sendFalloff = true;
            } else if (bp.roughness() != Mercator::BasePoint::ROUGHNESS) {
                size = 4;
                sendRoughness = true;
            }
            ListType & pointElem = (terrain[key.str()] = ListType(size)).List();
            pointElem[0] = (FloatType)(column.first);
            pointElem[1] = (FloatType)(point.first);
            pointElem[2] = (FloatType)(bp.height());
            if (sendRoughness) {
                pointElem[3] = bp.roughness();
            }
            if (sendFalloff) {
                pointElem[4] = bp.falloff();
            }
        }
    }

    t["surfaces"] = m_surfaces;
    return 0;
}

void TerrainProperty::set(const Element & ent)
{
    if (!ent.isMap()) {
        return;
    }
    const MapType & t = ent.asMap();
    debug_print("TerrainProperty::setTerrain()"
                   )

    const Pointstore & base_points = m_data->getPoints();

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();

    auto I = t.find("points");
    if (I != t.end() && I->second.isMap()) {
        const MapType & points = I->second.asMap();
        auto Iend = points.end();
        for (auto pointsI = points.begin(); pointsI != Iend; ++pointsI) {
            if (!pointsI->second.isList()) {
                continue;
            }
            const ListType & point = pointsI->second.asList();
            if (point.size() < 3) {
                continue;
            }
            if (!point[0].isNum() || !point[1].isNum() || !point[2].isNum()) {
                continue;
            }

            int x = (int)point[0].asNum();
            int y = (int)point[1].asNum();
            double h = point[2].asNum();
            double roughness;
            double falloff;
            if (point.size() > 3) {
                roughness = point[3].asFloat();
            } else {
                roughness = Mercator::BasePoint::ROUGHNESS;
            }
            if (point.size() > 4) {
                falloff = point[4].asFloat();
            } else {
                falloff = Mercator::BasePoint::FALLOFF;
            }

            Mercator::BasePoint bp(h, roughness, falloff);

            auto J = base_points.find(x);
            if (J == base_points.end() ||
                J->second.find(y) == J->second.end()) {
                // Newly added point.
                m_createdTerrain[x].insert(y);
            } else {
                // Modified point
                PointSet::const_iterator K = m_createdTerrain.find(x);
                if (K == m_createdTerrain.end() ||
                    K->second.find(y) == K->second.end()) {
                    // Already in database
                    m_modifiedTerrain[x].insert(y);
                }
                // else do nothing, as its currently waiting to be added.
            }
            

            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);

            m_data->setBasePoint(x, y, bp);

        }
    }

    if (minX != std::numeric_limits<int>::max()) {
        float spacing = m_data->getSpacing();
        WFMath::Point<2> minCorner((minX * spacing) - (spacing * 0.5f), (minY * spacing)  - (spacing * 0.5f));
        WFMath::Point<2> maxCorner((maxX * spacing) + (spacing * 0.5f), (maxY * spacing) + (spacing * 0.5f));
        WFMath::AxisBox<2> changedArea(minCorner, maxCorner);
        m_changedAreas.push_back(changedArea);
    }

    I = t.find("surfaces");
    if (I != t.end() && I->second.isList()) {
        //Only alter shader if the definition has changed.
        if (m_surfaces != I->second.List()) {
            auto shader = createShaders(I->second.List());
            if (m_tileShader) {
                m_data->removeShader(m_tileShader.get(), 0);
            }
            m_tileShader = std::move(shader);
            if (m_tileShader) {
                m_data->addShader(m_tileShader.get(), 0);
            }
            m_surfaces = I->second.List();
        }
    }

}

void TerrainProperty::apply(LocatedEntity* entity) {

    if (!m_changedAreas.empty()) {
        Domain* domain = entity->getDomain();
        if (domain) {
            domain->refreshTerrain(m_changedAreas);
            m_changedAreas.clear();
        }
    }
}

std::unique_ptr<Mercator::TileShader> TerrainProperty::createShaders(const Atlas::Message::ListType& surfaceList) {
    m_surfaceNames.clear();
    if (!surfaceList.empty()) {
        auto tileShader = std::make_unique<Mercator::TileShader>();
        int layer = 0;
        for (auto& surfaceElement : surfaceList) {
            if (!surfaceElement.isMap()) {
                continue;
            }
            auto& surfaceMap = surfaceElement.Map();

            auto patternI = surfaceMap.find("pattern");
            if (patternI == surfaceMap.end() || !patternI->second.isString()) {
                log(WARNING, "Surface has no 'pattern'.");
                continue;
            }

            auto nameI = surfaceMap.find("name");
            if (nameI == surfaceMap.end() || !nameI->second.isString()) {
                log(WARNING, "Surface has no 'name'.");
                continue;
            }
            m_surfaceNames.push_back(nameI->second.String());


            Mercator::Shader::Parameters shaderParams;
            auto paramsI = surfaceMap.find("params");
            if (paramsI != surfaceMap.end() && paramsI->second.isMap()) {
                auto params = paramsI->second.Map();
                for (auto& entry : params) {
                    if (entry.second.isNum()) {
                        shaderParams.insert(std::make_pair(entry.first, (float)entry.second.asNum()));
                    } else {
                        log(WARNING, "'terrain.shaders...params' entry must be a map of floats..");
                    }
                }
            }

            auto& pattern = patternI->second.String();
            Mercator::Shader* shader = nullptr;
            if (pattern == "fill") {
                shader=new Mercator::FillShader(shaderParams);
            } else if (pattern == "band") {
                shader=new Mercator::BandShader(shaderParams);
            } else if (pattern == "grass") {
                shader=new Mercator::GrassShader(shaderParams);
            } else if (pattern == "depth") {
                shader=new Mercator::DepthShader(shaderParams);
            } else if (pattern == "high") {
                shader = new Mercator::HighShader(shaderParams);
            }

            if (shader) {
                tileShader->addShader(shader, layer);
            } else {
                log(WARNING, String::compose("Could not recognize surface with pattern '%1'", pattern));
            }
            layer++;
        }
        return tileShader;
    }
    return nullptr;
}

TerrainProperty * TerrainProperty::copy() const
{
    return new TerrainProperty(*this);
}

void TerrainProperty::addMod(long id, const Mercator::TerrainMod *mod) const
{
    m_data->updateMod(id, mod);
}

void TerrainProperty::updateMod(long id, const Mercator::TerrainMod *mod) const
{
    m_data->updateMod(id, mod);
}

void TerrainProperty::removeMod(long id) const
{
    m_data->updateMod(id, nullptr);
}

void TerrainProperty::clearMods(float x, float y)
{
    Mercator::Segment *s = m_data->getSegmentAtPos(x,y);
    if(s != nullptr) {
        s->clearMods();
        //log(INFO, "Mods cleared!");
    } 
}

/// \brief Return the height and normal to the surface at the given point
bool TerrainProperty::getHeightAndNormal(float x,
                                         float y,
                                         float & height,
                                         Vector3D & normal) const
{
    auto s = m_data->getSegmentAtPos(x, y);
    if (s && !s->isValid()) {
        s->populate();
    }
    return m_data->getHeightAndNormal(x, y, height, normal);
}

bool TerrainProperty::getHeight(float x, float y, float& height) const
{
    auto s = m_data->getSegmentAtPos(x, y);
    if (s && !s->isValid()) {
        s->populate();
    }
    return m_data->getHeight(x, y, height);
}


/// \brief Get a number encoding the surface type at the given x,z coordinates
///
/// @param pos the x,z coordinates of the point on the terrain
/// @param material a reference to the integer to be used to store the
/// material identifier at this location.
boost::optional<int> TerrainProperty::getSurface(float x, float z) const
{
    Mercator::Segment * segment = m_data->getSegmentAtPos(x, z);
    if (segment == nullptr) {
        debug(std::cerr << "No terrain at this point" << std::endl << std::flush;);
        return boost::none;
    }
    if (!segment->isValid()) {
        segment->populate();
    }
    x -= segment->getXRef();
    z -= segment->getZRef();
    assert(x <= segment->getSize());
    assert(z <= segment->getSize());
    const Mercator::Segment::Surfacestore & surfaces = segment->getSurfaces();
    WFMath::Vector<3> normal;
    float height = -23;
    segment->getHeightAndNormal(x, z, height, normal);
    debug(std::cout << "At the point " << x << "," << z
                    << " of the segment the height is " << height << std::endl;
          std::cout << "The segment has " << surfaces.size()
                    << std::endl << std::flush;);
    if (surfaces.empty()) {
        log(ERROR, "The terrain has no surface data");
        return boost::none;
    }
    Mercator::Surface & tile_surface = *surfaces.begin()->second;
    if (!tile_surface.isValid()) {
        tile_surface.populate();
    }
    return tile_surface((int)x, (int)z, 0);
}

boost::optional<std::vector<LocatedEntity*>> TerrainProperty::findMods(float x, float z) const
{
    Mercator::Segment * seg = m_data->getSegmentAtPos(x, z);
    if (seg == nullptr) {
        return boost::none;
    }
    std::vector<LocatedEntity*> ret;
    auto& seg_mods = seg->getMods();
    for (auto& entry : seg_mods) {
        const Mercator::TerrainMod * mod = entry.second;
        WFMath::AxisBox<2> mod_box = mod->bbox();
        if (x > mod_box.lowCorner().x() && x < mod_box.highCorner().x() &&
            z > mod_box.lowCorner().y() && z < mod_box.highCorner().y()) {
            Mercator::Effector::Context * c = mod->context();
            if (c == nullptr) {
                log(WARNING, "Terrrain mod with no context");
                continue;
            }
            debug(std::cout << "Context has id" << c->id() << std::endl;);
            auto tc = dynamic_cast<TerrainContext *>(c);
            if (tc == nullptr) {
                log(WARNING, "Terrrain mod with non Cyphesis context");
                continue;
            }
            debug(std::cout << "Context has pointer " << tc->entity().get()
                            << std::endl;);
            ret.push_back(tc->entity().get());
        }
    }
    return ret;
}

Mercator::Terrain& TerrainProperty::getData()
{
    return *m_data;
}

Mercator::Terrain& TerrainProperty::getData() const
{
    return *m_data;
}


