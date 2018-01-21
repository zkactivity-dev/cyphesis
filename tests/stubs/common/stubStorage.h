// AUTOGENERATED file, created by the tool generate_stub.py, don't edit!
// If you want to add your own functionality, instead edit the stubStorage_custom.h file.

#include "common/Storage.h"
#include "stubStorage_custom.h"

#ifndef STUB_COMMON_STORAGE_H
#define STUB_COMMON_STORAGE_H

#ifndef STUB_Storage_init
//#define STUB_Storage_init
  int Storage::init()
  {
    return 0;
  }
#endif //STUB_Storage_init

#ifndef STUB_Storage_putAccount
//#define STUB_Storage_putAccount
  int Storage::putAccount(const Atlas::Message::MapType & o)
  {
    return 0;
  }
#endif //STUB_Storage_putAccount

#ifndef STUB_Storage_modAccount
//#define STUB_Storage_modAccount
  int Storage::modAccount(const Atlas::Message::MapType & o, const std::string & accountId)
  {
    return 0;
  }
#endif //STUB_Storage_modAccount

#ifndef STUB_Storage_delAccount
//#define STUB_Storage_delAccount
  int Storage::delAccount(const std::string & account)
  {
    return 0;
  }
#endif //STUB_Storage_delAccount

#ifndef STUB_Storage_getAccount
//#define STUB_Storage_getAccount
  int Storage::getAccount(const std::string & username, Atlas::Message::MapType & o)
  {
    return 0;
  }
#endif //STUB_Storage_getAccount

#ifndef STUB_Storage_storeInRules
//#define STUB_Storage_storeInRules
  void Storage::storeInRules(const Atlas::Message::MapType & rule, const std::string & key)
  {
    
  }
#endif //STUB_Storage_storeInRules

#ifndef STUB_Storage_clearRules
//#define STUB_Storage_clearRules
  int Storage::clearRules()
  {
    return 0;
  }
#endif //STUB_Storage_clearRules

#ifndef STUB_Storage_setRuleset
//#define STUB_Storage_setRuleset
  void Storage::setRuleset(const std::string & n)
  {
    
  }
#endif //STUB_Storage_setRuleset


#endif