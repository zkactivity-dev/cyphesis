#include "Predicates.h"

#include "../../common/TypeNode.h"

#include <algorithm>

namespace EntityFilter
{
ComparePredicate::ComparePredicate(const Consumer<QueryContext>* lhs,
                                   const Consumer<QueryContext>* rhs,
                                   Comparator comparator) :
        m_lhs(lhs), m_rhs(rhs), m_comparator(comparator)
{
    //make sure rhs and lhs exist
    if (!m_lhs || !m_rhs) {
        throw std::invalid_argument(
                "At least one side of the comparison operator is invalid");
    }
    if (m_comparator == Comparator::INSTANCE_OF) {
        //make sure that left hand side returns an entity and right hand side a typenode
        if ((m_lhs->getType() != &typeid(const LocatedEntity*))
                || (m_rhs->getType() != &typeid(const TypeNode*))) {
            throw std::invalid_argument(
                    "When using the 'instanceof' comparator, left statement must return an entity and right a TypeNode. For example, 'entity instance_of types.world'.");
        }
    }

}

bool ComparePredicate::isMatch(const QueryContext& context) const
{
    switch (m_comparator) {
    case Comparator::EQUALS:
    {
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        m_rhs->value(right, context);
        return left == right;
    }
    case Comparator::NOT_EQUALS:
    {
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        m_rhs->value(right, context);
        return left != right;
    }
    case Comparator::LESS:
    {
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        if (left.isNum()) {
            m_rhs->value(right, context);
            if (right.isNum()) {
                return left.asNum() < right.asNum();
            }
        }
        return false;
    }
    case Comparator::LESS_EQUAL:
    {
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        if (left.isNum()) {
            m_rhs->value(right, context);
            if (right.isNum()) {
                return left.asNum() <= right.asNum();
            }
        }
        return false;
    }
    case Comparator::GREATER:
    {
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        if (left.isNum()) {
            m_rhs->value(right, context);
            if (right.isNum()) {
                return left.asNum() > right.asNum();
            }
        }
        return false;
    }
    case Comparator::GREATER_EQUAL:
    {
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        if (left.isNum()) {
            m_rhs->value(right, context);
            if (right.isNum()) {
                return left.asNum() >= right.asNum();
            }
        }
        return false;
    }
    case Comparator::INSTANCE_OF:
    {
        //We know that left returns an entity and right returns a type, since we checked in the constructor.
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        if (left.isPtr()) {
            auto leftType = static_cast<const LocatedEntity*>(left.Ptr())->getType();
            if (leftType) {
                m_rhs->value(right, context);
                if (right.isPtr()) {
                    auto rightType = static_cast<const TypeNode*>(right.Ptr());
                    if (rightType) {
                        return leftType->isTypeOf(rightType);
                    }
                }
            }
        }
        return false;
    }
    case Comparator::IN:
    {
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        if (!left.isNone()) {
            m_rhs->value(right, context);
            if (right.isList()) {
                const auto& right_end = right.List().end();
                const auto& right_begin = right.List().begin();
                return std::find(right_begin, right_end, left) != right_end;
            }
        }
        return false;
    }
    case Comparator::CONTAINS:
    {
        Atlas::Message::Element left, right;
        m_lhs->value(left, context);
        if (left.isList()) {
            m_rhs->value(right, context);
            if (!right.isNone()) {
                const auto& left_end = left.List().end();
                const auto& left_begin = left.List().begin();
                return std::find(left_begin, left_end, right) != left_end;
            }
        }
        return false;
    }
    }
    return false;
}

AndPredicate::AndPredicate(const Predicate* lhs, const Predicate* rhs) :
        m_lhs(lhs), m_rhs(rhs)
{
}
bool AndPredicate::isMatch(const QueryContext& context) const
{
    return m_lhs->isMatch(context) && m_rhs->isMatch(context);

}

OrPredicate::OrPredicate(const Predicate* lhs, const Predicate* rhs) :
        m_lhs(lhs), m_rhs(rhs)
{
}
bool OrPredicate::isMatch(const QueryContext& context) const
{
    return m_lhs->isMatch(context) || m_rhs->isMatch(context);

}

NotPredicate::NotPredicate(const Predicate* pred) :
        m_pred(pred)
{
}

bool NotPredicate::isMatch(const QueryContext& context) const
{
    return !m_pred->isMatch(context);
}
}
