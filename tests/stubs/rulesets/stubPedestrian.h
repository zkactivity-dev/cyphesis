// AUTOGENERATED file, created by the tool generate_stub.py, don't edit!
// If you want to add your own functionality, instead edit the stubPedestrian_custom.h file.

#include "rulesets/Pedestrian.h"
#include "stubPedestrian_custom.h"

#ifndef STUB_RULESETS_PEDESTRIAN_H
#define STUB_RULESETS_PEDESTRIAN_H

#ifndef STUB_Pedestrian_Pedestrian
//#define STUB_Pedestrian_Pedestrian
   Pedestrian::Pedestrian(LocatedEntity & body)
    : Movement(body)
  {
    
  }
#endif //STUB_Pedestrian_Pedestrian

#ifndef STUB_Pedestrian_Pedestrian_DTOR
//#define STUB_Pedestrian_Pedestrian_DTOR
   Pedestrian::~Pedestrian()
  {
    
  }
#endif //STUB_Pedestrian_Pedestrian_DTOR

#ifndef STUB_Pedestrian_getTickAddition
//#define STUB_Pedestrian_getTickAddition
  double Pedestrian::getTickAddition(const Point3D & coordinates, const Vector3D & velocity) const
  {
    return 0;
  }
#endif //STUB_Pedestrian_getTickAddition

#ifndef STUB_Pedestrian_getUpdatedLocation
//#define STUB_Pedestrian_getUpdatedLocation
  int Pedestrian::getUpdatedLocation(Location &)
  {
    return 0;
  }
#endif //STUB_Pedestrian_getUpdatedLocation

#ifndef STUB_Pedestrian_generateMove
//#define STUB_Pedestrian_generateMove
  Atlas::Objects::Operation::RootOperation Pedestrian::generateMove(const Location &)
  {
    return *static_cast<Atlas::Objects::Operation::RootOperation*>(nullptr);
  }
#endif //STUB_Pedestrian_generateMove


#endif