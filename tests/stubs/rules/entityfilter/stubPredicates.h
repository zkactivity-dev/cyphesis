// AUTOGENERATED file, created by the tool generate_stub.py, don't edit!
// If you want to add your own functionality, instead edit the stubPredicates_custom.h file.

#ifndef STUB_RULES_ENTITYFILTER_PREDICATES_H
#define STUB_RULES_ENTITYFILTER_PREDICATES_H

#include "rules/entityfilter/Predicates.h"
#include "stubPredicates_custom.h"

namespace EntityFilter {

#ifndef STUB_Predicate_isMatch
//#define STUB_Predicate_isMatch
  bool Predicate::isMatch(const QueryContext& context) const
  {
    return false;
  }
#endif //STUB_Predicate_isMatch


}  // namespace EntityFilter

namespace EntityFilter {

#ifndef STUB_ComparePredicate_ComparePredicate
//#define STUB_ComparePredicate_ComparePredicate
   ComparePredicate::ComparePredicate(std::shared_ptr<Consumer<QueryContext>> lhs, std::shared_ptr<Consumer<QueryContext>> rhs, Comparator comparator, std::shared_ptr<Consumer<QueryContext>> with )
    : Predicate(lhs, rhs, comparator, with)
  {
    
  }
#endif //STUB_ComparePredicate_ComparePredicate

#ifndef STUB_ComparePredicate_isMatch
//#define STUB_ComparePredicate_isMatch
  bool ComparePredicate::isMatch(const QueryContext& context) const
  {
    return false;
  }
#endif //STUB_ComparePredicate_isMatch


}  // namespace EntityFilter

namespace EntityFilter {

#ifndef STUB_DescribePredicate_DescribePredicate
//#define STUB_DescribePredicate_DescribePredicate
   DescribePredicate::DescribePredicate(std::string description, std::shared_ptr<Predicate> predicate)
    : Predicate(description, predicate)
  {
    
  }
#endif //STUB_DescribePredicate_DescribePredicate

#ifndef STUB_DescribePredicate_isMatch
//#define STUB_DescribePredicate_isMatch
  bool DescribePredicate::isMatch(const QueryContext& context) const
  {
    return false;
  }
#endif //STUB_DescribePredicate_isMatch


}  // namespace EntityFilter

namespace EntityFilter {

#ifndef STUB_AndPredicate_AndPredicate
//#define STUB_AndPredicate_AndPredicate
   AndPredicate::AndPredicate(std::shared_ptr<Predicate> lhs, std::shared_ptr<Predicate> rhs)
    : Predicate(lhs, rhs)
  {
    
  }
#endif //STUB_AndPredicate_AndPredicate

#ifndef STUB_AndPredicate_isMatch
//#define STUB_AndPredicate_isMatch
  bool AndPredicate::isMatch(const QueryContext& context) const
  {
    return false;
  }
#endif //STUB_AndPredicate_isMatch


}  // namespace EntityFilter

namespace EntityFilter {

#ifndef STUB_OrPredicate_OrPredicate
//#define STUB_OrPredicate_OrPredicate
   OrPredicate::OrPredicate(std::shared_ptr<Predicate> lhs, std::shared_ptr<Predicate> rhs)
    : Predicate(lhs, rhs)
  {
    
  }
#endif //STUB_OrPredicate_OrPredicate

#ifndef STUB_OrPredicate_isMatch
//#define STUB_OrPredicate_isMatch
  bool OrPredicate::isMatch(const QueryContext& context) const
  {
    return false;
  }
#endif //STUB_OrPredicate_isMatch


}  // namespace EntityFilter

namespace EntityFilter {

#ifndef STUB_NotPredicate_NotPredicate
//#define STUB_NotPredicate_NotPredicate
   NotPredicate::NotPredicate(std::shared_ptr<Predicate> pred)
    : Predicate(pred)
  {
    
  }
#endif //STUB_NotPredicate_NotPredicate

#ifndef STUB_NotPredicate_isMatch
//#define STUB_NotPredicate_isMatch
  bool NotPredicate::isMatch(const QueryContext& context) const
  {
    return false;
  }
#endif //STUB_NotPredicate_isMatch


}  // namespace EntityFilter

namespace EntityFilter {

#ifndef STUB_BoolPredicate_BoolPredicate
//#define STUB_BoolPredicate_BoolPredicate
   BoolPredicate::BoolPredicate(std::shared_ptr<Consumer<QueryContext>> consumer)
    : Predicate(consumer)
  {
    
  }
#endif //STUB_BoolPredicate_BoolPredicate

#ifndef STUB_BoolPredicate_isMatch
//#define STUB_BoolPredicate_isMatch
  bool BoolPredicate::isMatch(const QueryContext& context) const
  {
    return false;
  }
#endif //STUB_BoolPredicate_isMatch


}  // namespace EntityFilter

#endif