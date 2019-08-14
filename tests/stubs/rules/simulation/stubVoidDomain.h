// AUTOGENERATED file, created by the tool generate_stub.py, don't edit!
// If you want to add your own functionality, instead edit the stubVoidDomain_custom.h file.

#ifndef STUB_RULES_SIMULATION_VOIDDOMAIN_H
#define STUB_RULES_SIMULATION_VOIDDOMAIN_H

#include "rules/simulation/VoidDomain.h"
#include "stubVoidDomain_custom.h"

#ifndef STUB_VoidDomain_VoidDomain
//#define STUB_VoidDomain_VoidDomain
   VoidDomain::VoidDomain(LocatedEntity& entity)
    : Domain(entity)
  {
    
  }
#endif //STUB_VoidDomain_VoidDomain

#ifndef STUB_VoidDomain_isEntityVisibleFor
//#define STUB_VoidDomain_isEntityVisibleFor
  bool VoidDomain::isEntityVisibleFor(const LocatedEntity& observingEntity, const LocatedEntity& observedEntity) const
  {
    return false;
  }
#endif //STUB_VoidDomain_isEntityVisibleFor

#ifndef STUB_VoidDomain_getVisibleEntitiesFor
//#define STUB_VoidDomain_getVisibleEntitiesFor
  void VoidDomain::getVisibleEntitiesFor(const LocatedEntity& observingEntity, std::list<LocatedEntity*>& entityList) const
  {
    
  }
#endif //STUB_VoidDomain_getVisibleEntitiesFor

#ifndef STUB_VoidDomain_addEntity
//#define STUB_VoidDomain_addEntity
  void VoidDomain::addEntity(LocatedEntity& entity)
  {
    
  }
#endif //STUB_VoidDomain_addEntity

#ifndef STUB_VoidDomain_removeEntity
//#define STUB_VoidDomain_removeEntity
  void VoidDomain::removeEntity(LocatedEntity& entity)
  {
    
  }
#endif //STUB_VoidDomain_removeEntity

#ifndef STUB_VoidDomain_isEntityReachable
//#define STUB_VoidDomain_isEntityReachable
  bool VoidDomain::isEntityReachable(const LocatedEntity& reachingEntity, float reach, const LocatedEntity& queriedEntity, const WFMath::Point<3>& positionOnQueriedEntity) const
  {
    return false;
  }
#endif //STUB_VoidDomain_isEntityReachable


#endif