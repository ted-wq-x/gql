// Copyright 2025 Oleg Maximenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "demo.h"

#include <cassert>
#include <cmath>
#include <functional>

#include "error.h"
#include "gql/ast/algorithm.h"
#include "gql/syntax_analyzer/aux_data.h"

namespace gql_demo {

inline bool IsNull(const Value& value) {
  return std::holds_alternative<Null>(value.v);
}

Value Demo::Evaluate(const gql::ast::ValueExpression& expr,
                     const ExecutionContext& context) {
  return gql::ast::variant_switch(
      expr.option,
      [](const gql::ast::NormalizedPredicate&) {
        // TODO: Implement
        return Value{Null{}};
      },
      [](const gql::ast::GraphExpression&) {
        // Feature disabled
        assert(false);
        return Value{Null{}};
      },
      [](const gql::ast::ValueFunction&) {
        // TODO: Implement
        return Value{Null{}};
      },
      [](const gql::ast::GeneralParameterReference&) {
        // TODO: Implement
        return Value{Null{}};
      },
      [](const gql::ast::SessionUserValue&) { return Value{"UserBob"}; },
      [](const gql::ast::PathValueConstructor&) {
        // Feature disabled
        assert(false);
        return Value{Null{}};
      },
      [&](const gql::ast::CaseExpression& expr) {
        return Evaluate(expr, context);
      },
      [](const gql::ast::CastSpecification&) {
        // Feature disabled
        assert(false);
        return Value{Null{}};
      },
      [&](const gql::ast::ProcedureBodyPtr& value) {
        return EvaluateValueQueryExpression(*value, context);
      },
      [&](const gql::ast::BindingVariableReference& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::BindingTableExpression& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::ValueExpression::Unary& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::ValueExpression::Binary& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::ValueExpression::Comparison& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::UnsignedLiteral& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::ValueExpression::BooleanTest& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::PropertyReference& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::AggregateFunction& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::ElementIdFunction& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::LetValueExpression& value) {
        return Evaluate(value, context);
      },
      [&](const gql::ast::Predicate& value) {
        return gql::ast::variant_switch(
            value,
            [](const gql::ast::ValueTypePredicate&) {
              // Feature disabled
              assert(false);
              return Value{Null{}};
            },
            [&](const gql::ast::AllElementsPredicate& value) {
              return Evaluate(value, context);
            },
            [](const gql::ast::ExistsPredicate&) {
              // Shall be rewritten.
              assert(false);
              return Value{Null{}};
            },
            [&](const gql::ast::NullPredicate& value) {
              auto operand = Evaluate(*value.expr, context);
              return Value{IsNull(operand) != value.isNot};
            },
            [&](const gql::ast::PropertyExistsPredicate& value) {
              return Evaluate(value, context);
            },
            [&](const gql::ast::DirectedPredicate& value) {
              return Evaluate(value, context);
            },
            [&](const gql::ast::LabeledPredicate& value) {
              return Evaluate(value, context);
            },
            [&](const gql::ast::SourceDestinationPredicate& value) {
              return Evaluate(value, context);
            });
      });
}

Value Demo::Evaluate(const gql::ast::BindingVariableBase& expr,
                     const ExecutionContext& context) {
  return context.workingRecord.fieldValue(expr.name);
}

Value Demo::Evaluate(const gql::ast::ValueExpression::BooleanTest& expr,
                     const ExecutionContext& context) {
  auto operand = Evaluate(*expr.expr, context);
  gql::ast::TruthValue actual;
  if (IsNull(operand)) {
    actual = gql::ast::TruthValue::UNKNOWN;
  } else if (auto* boolVal = std::get_if<bool>(&operand.v)) {
    actual =
        *boolVal ? gql::ast::TruthValue::TRUE : gql::ast::TruthValue::FALSE;
  } else {
    ThrowInvalidValueTypeError(*expr.expr);
  }
  return (actual == expr.value) != expr.isNot;
}

Value Demo::Evaluate(const gql::ast::UnsignedLiteral& expr,
                     const ExecutionContext& context) {
  return gql::ast::variant_switch(
      expr,
      [&](const gql::ast::UnsignedNumericLiteral& value) {
        return gql::ast::variant_switch(
            value, [&](double value) { return Value{value}; },
            [&](const gql::ast::ExactNumericLiteral& value) {
              if (value.scale > 0) {
                return Value{static_cast<double>(value.value) *
                             std::pow(10.0, -static_cast<double>(value.scale))};
              }
              return Value{value.value};
            });
      },
      [&](const gql::ast::GeneralLiteral& value) {
        return gql::ast::variant_switch(
            value,
            [](const gql::ast::ByteStringLiteral&) {
              // Feature disabled
              assert(false);
              return Value{Null{}};
            },
            [](const gql::ast::TemporalLiteral&) {
              // Feature disabled
              assert(false);
              return Value{Null{}};
            },
            [](const gql::ast::DurationLiteral&) {
              // Feature disabled
              assert(false);
              return Value{Null{}};
            },
            [&](const gql::ast::NullLiteral&) { return Value{Null{}}; },
            [&](const gql::ast::TruthValue& value) {
              switch (value) {
                case gql::ast::TruthValue::TRUE:
                  return Value{true};
                case gql::ast::TruthValue::FALSE:
                  return Value{false};
                default:
                case gql::ast::TruthValue::UNKNOWN:
                  return Value{Null{}};
              }
            },
            [&](const gql::ast::CharacterStringLiteral& value) {
              return Value{value.name};
            },
            [&](const gql::ast::ListLiteral& value) {
              List list;
              for (const auto& element : value.elements) {
                list.emplace_back() = Evaluate(*element, context);
              }
              return Value{list};
            },
            [&](const gql::ast::RecordLiteral& value) {
              Record record;
              for (const auto& f : value) {
                record[f.name.name] = Evaluate(*f.value, context);
              }
              return Value{record};
            });
      });
}

Value Demo::Evaluate(const gql::ast::PropertyReference& expr,
                     const ExecutionContext& context) {
  auto source = Evaluate(*expr.element, context);

  return gql::ast::variant_switch(
      source.v, [&](const auto&) -> Value { ThrowInvalidValueTypeError(expr); },
      [](Null) { return Value{Null{}}; },
      [&](const Record& value) {
        auto propIt = value.find(expr.property.name);
        return (propIt != value.end()) ? Value{propIt->second} : Value{Null{}};
      },
      [&](const NodeReference& value) {
        if (auto* node = graph.GetNode(value.nodeId)) {
          auto propIt = node->properties.find(expr.property.name);
          return (propIt != node->properties.end()) ? Value{propIt->second}
                                                    : Value{Null{}};
        }
        ThrowInvalidReferenceError(*expr.element);
      },
      [&](const EdgeReference& value) {
        if (auto* node = graph.GetEdge(value.edgeId)) {
          auto propIt = node->properties.find(expr.property.name);
          return (propIt != node->properties.end()) ? Value{propIt->second}
                                                    : Value{Null{}};
        }
        ThrowInvalidReferenceError(*expr.element);
      });
}

Value Demo::Evaluate(const gql::ast::ValueExpression::Unary& expr,
                     const ExecutionContext& context) {
  auto operand = Evaluate(*expr.expr, context);
  switch (expr.op) {
    case gql::ast::ValueExpression::Unary::Op::BoolNot: {
      auto* opBool = std::get_if<bool>(&operand.v);
      if (!opBool)
        return Null{};
      return !*opBool;
    }
    case gql::ast::ValueExpression::Unary::Op::Size:
      return gql::ast::variant_switch(
          operand.v,
          [&](const auto&) -> Value { ThrowInvalidValueTypeError(*expr.expr); },
          [&](Null) { return Value{Null{}}; },
          [&](const List& value) {
            return Value{static_cast<int64_t>(value.size())};
          });
    case gql::ast::ValueExpression::Unary::Op::Cardinality:
      return gql::ast::variant_switch(
          operand.v,
          [&](const auto&) -> Value { ThrowInvalidValueTypeError(*expr.expr); },
          [&](Null) { return Value{Null{}}; },
          [&](const List& value) {
            return Value{static_cast<int64_t>(value.size())};
          },
          [&](const PathReference& value) {
            return Value{static_cast<int64_t>(value.nodeIds.size() +
                                              value.edgeIds.size())};
          },
          [&](const BindingTableReference& value) {
            return Value{static_cast<int64_t>(value->size())};
          });
    case gql::ast::ValueExpression::Unary::Op::Negative:
      return gql::ast::variant_switch(
          operand.v,
          [&](const auto&) -> Value { ThrowInvalidValueTypeError(*expr.expr); },
          [&](Null) { return Value{Null{}}; },
          [&](double value) { return Value{-value}; },
          [&](int64_t value) { return Value{-value}; });
    case gql::ast::ValueExpression::Unary::Op::Positive:
      return operand;
    case gql::ast::ValueExpression::Unary::Op::CharLength:
      return gql::ast::variant_switch(
          operand.v,
          [&](const auto&) -> Value { ThrowInvalidValueTypeError(*expr.expr); },
          [&](Null) { return Value{Null{}}; },
          [&](const std::string& str) {
            return Value{static_cast<int64_t>(str.size())};
          });
    case gql::ast::ValueExpression::Unary::Op::PathLength:
      return gql::ast::variant_switch(
          operand.v,
          [&](const auto&) -> Value { ThrowInvalidValueTypeError(*expr.expr); },
          [&](Null) { return Value{Null{}}; },
          [&](const PathReference& path) {
            return Value{static_cast<int64_t>(path.edgeIds.size())};
          });
    case gql::ast::ValueExpression::Unary::Op::Elements:
      return gql::ast::variant_switch(
          operand.v,
          [&](const auto&) -> Value { ThrowInvalidValueTypeError(*expr.expr); },
          [&](Null) { return Value{Null{}}; },
          [&](const PathReference& path) {
            List lst;
            auto edgeIt = path.edgeIds.begin();
            for (auto id : path.nodeIds) {
              lst.emplace_back() = NodeReference{id};
              if (edgeIt != path.edgeIds.end()) {
                lst.emplace_back() = EdgeReference{*edgeIt};
                edgeIt++;
              }
            }
            return Value{lst};
          });
    case gql::ast::ValueExpression::Unary::Op::ByteLength:
    case gql::ast::ValueExpression::Unary::Op::Sin:
    case gql::ast::ValueExpression::Unary::Op::Cos:
    case gql::ast::ValueExpression::Unary::Op::Tan:
    case gql::ast::ValueExpression::Unary::Op::Cot:
    case gql::ast::ValueExpression::Unary::Op::Sinh:
    case gql::ast::ValueExpression::Unary::Op::Cosh:
    case gql::ast::ValueExpression::Unary::Op::Tanh:
    case gql::ast::ValueExpression::Unary::Op::Asin:
    case gql::ast::ValueExpression::Unary::Op::Acos:
    case gql::ast::ValueExpression::Unary::Op::Atan:
    case gql::ast::ValueExpression::Unary::Op::Degrees:
    case gql::ast::ValueExpression::Unary::Op::Radians:
    case gql::ast::ValueExpression::Unary::Op::Floor:
    case gql::ast::ValueExpression::Unary::Op::Ceiling:
    case gql::ast::ValueExpression::Unary::Op::SquareRoot:
    case gql::ast::ValueExpression::Unary::Op::Exponential:
    case gql::ast::ValueExpression::Unary::Op::NaturalLogarithm:
    case gql::ast::ValueExpression::Unary::Op::CommonLogarithm:
    case gql::ast::ValueExpression::Unary::Op::AbsoluteValue:
      // Feature disabled
      assert(false);
      return Null{};
    default:
      break;
  }
  assert(false);
  return {};
}

namespace {

// TODO: Check types for +,- and *,/
template <template <typename> class Op>
struct ArithmeticOperatorVisitor {
  template <typename T1, typename T2>
  Value operator()(const T1&, const T2&) const {
    ThrowInvalidValueTypeError(node_);
  }

  Value operator()(double l, double r) const { return Op<double>()(l, r); }
  Value operator()(int64_t l, int64_t r) const { return Op<int64_t>()(l, r); }
  Value operator()(int64_t l, double r) const {
    return Op<double>()(static_cast<double>(l), r);
  }
  Value operator()(double l, int64_t r) const {
    return Op<double>()(l, static_cast<double>(r));
  }

  const gql::ast::Node& node_;
};

// 20.21 Syntax Rule 1.b.iv "The precision and scale of the result of division
// are implementation-defined" Standard requires exact numeric result for
// division of exact numerics but we use |double| for exact numerics with
// non-zero scale.
struct DivideOperatorVisitor {
  template <typename T1, typename T2>
  Value operator()(const T1&, const T2&) const {
    ThrowInvalidValueTypeError(node_);
  }

  Value operator()(double l, double r) const { return l / r; }
  Value operator()(int64_t l, int64_t r) const {
    return static_cast<double>(l) / static_cast<double>(r);
  }
  Value operator()(int64_t l, double r) const {
    return static_cast<double>(l) / r;
  }
  Value operator()(double l, int64_t r) const {
    return l / static_cast<double>(r);
  }

  const gql::ast::Node& node_;
};

inline Value ValuesAdd(const Value& left,
                       const Value& right,
                       const gql::ast::Node& node) {
  return std::visit(ArithmeticOperatorVisitor<std::plus>{node}, left.v,
                    right.v);
}

inline Value ValuesDivide(const Value& left,
                          const Value& right,
                          const gql::ast::Node& node) {
  return std::visit(DivideOperatorVisitor{node}, left.v, right.v);
}

struct ConcatenateVisitor {
  template <typename T1, typename T2>
  Value operator()(const T1&, const T2&) const {
    ThrowInvalidValueTypeError(node_);
  }

  Value operator()(const List& l, const List& r) const {
    auto res = l;
    res.insert(res.end(), r.begin(), r.end());
    return res;
  }

  Value operator()(const std::string& l, const std::string& r) const {
    return l + r;
  }

  Value operator()(const PathReference& l, const PathReference& r) const {
    if (l.nodeIds.back() != r.nodeIds.front()) {
      throw GQLRuntimeError(node_.inputPosition(), "malformed path");
    }
    PathReference result = l;
    result.nodeIds.insert(l.nodeIds.end(), r.nodeIds.begin() + 1,
                          r.nodeIds.end());
    result.edgeIds.insert(result.edgeIds.end(), r.edgeIds.begin(),
                          r.edgeIds.end());
    return result;
  }

  const gql::ast::Node& node_;
};

}  // namespace

Value Demo::Evaluate(const gql::ast::ValueExpression::Binary& expr,
                     const ExecutionContext& context) {
  auto left = Evaluate(*expr.left, context);
  auto right = Evaluate(*expr.right, context);
  switch (expr.op) {
    case gql::ast::ValueExpression::Binary::Op::BoolAnd:
    case gql::ast::ValueExpression::Binary::Op::BoolOr:
    case gql::ast::ValueExpression::Binary::Op::BoolXor: {
      auto* leftBool = std::get_if<bool>(&left.v);
      auto* rightBool = std::get_if<bool>(&right.v);
      assert(leftBool || IsNull(left));
      assert(rightBool || IsNull(right));
      switch (expr.op) {
        case gql::ast::ValueExpression::Binary::Op::BoolAnd:
          if (leftBool && !*leftBool) {
            return false;
          }
          if (rightBool && !*rightBool) {
            return false;
          }
          if (leftBool && *leftBool && rightBool && *rightBool) {
            return true;
          }
          return Null{};
        case gql::ast::ValueExpression::Binary::Op::BoolOr:
          if (leftBool && *leftBool) {
            return true;
          }
          if (rightBool && *rightBool) {
            return true;
          }
          if (leftBool && !*leftBool && rightBool && !*rightBool) {
            return false;
          }
          return Null{};
        case gql::ast::ValueExpression::Binary::Op::BoolXor:
          if (!leftBool || !rightBool) {
            return Null{};
          }
          return *leftBool ^ *rightBool;
      }
      assert(false);
      return Null{};
    }
    default:
      break;
  }

  if (IsNull(left) || IsNull(right)) {
    return Null{};
  }
  switch (expr.op) {
    case gql::ast::ValueExpression::Binary::Op::Multiply:
      return std::visit(ArithmeticOperatorVisitor<std::multiplies>{*expr.left},
                        left.v, right.v);
    case gql::ast::ValueExpression::Binary::Op::Divide:
      return std::visit(DivideOperatorVisitor{*expr.left}, left.v, right.v);
    case gql::ast::ValueExpression::Binary::Op::Add:
      return std::visit(ArithmeticOperatorVisitor<std::plus>{*expr.left},
                        left.v, right.v);
    case gql::ast::ValueExpression::Binary::Op::Subtract:
      return std::visit(ArithmeticOperatorVisitor<std::minus>{*expr.left},
                        left.v, right.v);
    case gql::ast::ValueExpression::Binary::Op::Concatenate:
      return std::visit(ConcatenateVisitor(*expr.left), left.v, right.v);
    case gql::ast::ValueExpression::Binary::Op::Power:
    case gql::ast::ValueExpression::Binary::Op::GeneralLogarithm:
    case gql::ast::ValueExpression::Binary::Op::Modulus:
      // Feature disabled
      assert(false);
      break;
    case gql::ast::ValueExpression::Binary::Op::TrimList:
      // TODO:
      break;
  }
  return Null{};
}

namespace {

// TODO: Check for null comparison etc
struct EqualsVisitor {
  template <typename T1, typename T2>
  bool operator()(const T1&, const T2&) const {
    return false;
  }

  template <typename T>
  bool operator()(const T& l, const T& r) const {
    return l == r;
  }
};

template <template <typename> class Comp>
struct CompareVisitor {
  template <typename T1, typename T2>
  bool operator()(const T1&, const T2&) const {
    ThrowInvalidValueTypeError(node_);
  }

  bool operator()(const std::string& l, const std::string& r) const {
    return Comp<std::string>()(l, r);
  }
  bool operator()(const List& l, const List& r) const {
    // TODO: Implement
    return false;
  }

  bool operator()(double l, double r) const { return Comp<double>()(l, r); }
  bool operator()(int64_t l, int64_t r) const { return Comp<int64_t>()(l, r); }
  bool operator()(int64_t l, double r) const {
    return Comp<double>()(static_cast<double>(l), r);
  }
  bool operator()(double l, int64_t r) const {
    return Comp<double>()(l, static_cast<double>(r));
  }

  const gql::ast::Node& node_;
};

inline bool ValueLessThan(const Value& left,
                          const Value& right,
                          const gql::ast::Node& node) {
  return std::visit(CompareVisitor<std::less>{node}, left.v, right.v);
}

}  // namespace

Value Demo::Evaluate(const gql::ast::ValueExpression::Comparison& expr,
                     const ExecutionContext& context) {
  auto left = Evaluate(*expr.left, context);
  auto right = Evaluate(*expr.right, context);
  switch (expr.op) {
    case gql::ast::CompOp::Equals:
      return std::visit(EqualsVisitor(), left.v, right.v);
    case gql::ast::CompOp::NotEquals:
      return !std::visit(EqualsVisitor(), left.v, right.v);
    default:
      break;
  }
  if (IsNull(left) || IsNull(right)) {
    return Null{};
  }
  switch (expr.op) {
    case gql::ast::CompOp::LessThan:
      return std::visit(CompareVisitor<std::less>{*expr.left}, left.v, right.v);
    case gql::ast::CompOp::GreaterThan:
      return std::visit(CompareVisitor<std::greater>{*expr.left}, left.v,
                        right.v);
    case gql::ast::CompOp::LessThanOrEquals:
      return std::visit(CompareVisitor<std::less_equal>{*expr.left}, left.v,
                        right.v);
    case gql::ast::CompOp::GreaterThanOrEquals:
      return std::visit(CompareVisitor<std::greater_equal>{*expr.left}, left.v,
                        right.v);
  }
  assert(false);
  return Null{};
}

Value Demo::Evaluate(const gql::ast::ElementIdFunction& expr,
                     const ExecutionContext& context) {
  auto value = Evaluate(expr.variable, context);
  return gql::ast::variant_switch(
      value.v, [&](const auto&) -> Value { ThrowInvalidValueTypeError(expr); },
      [&](Null) { return Value{Null{}}; },
      [&](const NodeReference& value) { return Value{value.nodeId}; },
      [&](const EdgeReference& value) { return Value{value.edgeId}; });
}

Value Demo::Evaluate(const gql::ast::AggregateFunction& expr,
                     const ExecutionContext& context) {
  return gql::ast::variant_switch(
      expr,
      [&](const gql::ast::CountAsteriskValue&) {
        return Value{static_cast<int64_t>(context.workingTable.size())};
      },
      [&](const gql::ast::GeneralSetFunction& func) -> Value {
        List values;
        auto appendValue = [&](Value&& value) {
          if (IsNull(value)) {
            // warning - null value eliminated in set function (01G11)
            return;
          }
          if (func.quantifier == gql::ast::SetQuantifier::DISTINCT) {
            for (const auto& v : values) {
              if (*v == value) {
                return;
              }
            }
          }
          values.emplace_back() = std::move(value);
        };
        if (func.auxData->groupListVariable) {
          auto& value = context.workingRecord.fieldValue(
              *func.auxData->groupListVariable);
          if (auto* listPtr = std::get_if<List>(&value.v)) {
            BindingTable itemTable;
            auto inserter = itemTable.CreateColumnInserter();
            auto& itemValue =
                inserter.Emplace(*func.auxData->groupListVariable);
            auto childContext = context.MakeChild();
            childContext.workingRecord.table = &itemTable;

            for (auto& item : *listPtr) {
              itemValue = *item;
              appendValue(Evaluate(*func.value, childContext));
            }
          } else {
            ThrowInvalidValueTypeError(func);
          }
        } else {
          for (size_t i = 0; i < context.workingTable.size(); i++) {
            auto childContext = context.MakeAmended(i);
            appendValue(Evaluate(*func.value, childContext));
          }
        }

        switch (func.type) {
          default:
          case gql::ast::GeneralSetFunctionType::COLLECT_LIST:
            return values;
          case gql::ast::GeneralSetFunctionType::STDDEV_SAMP:
          case gql::ast::GeneralSetFunctionType::STDDEV_POP:
            // TODO: Implement
            break;
          case gql::ast::GeneralSetFunctionType::COUNT:
            return static_cast<int64_t>(values.size());
          case gql::ast::GeneralSetFunctionType::MIN: {
            if (values.empty()) {
              return Null{};
            }
            Value mn = *values.front();
            for (auto valueIt = values.begin() + 1; valueIt != values.end();
                 valueIt++) {
              if (ValueLessThan(**valueIt, mn, func)) {
                mn = **valueIt;
              }
            }
            return mn;
          }
          case gql::ast::GeneralSetFunctionType::MAX: {
            if (values.empty()) {
              return Null{};
            }
            Value mx = *values.front();
            for (auto valueIt = values.begin() + 1; valueIt != values.end();
                 valueIt++) {
              if (ValueLessThan(mx, **valueIt, func)) {
                mx = **valueIt;
              }
            }
            return mx;
          }
          case gql::ast::GeneralSetFunctionType::SUM:
          case gql::ast::GeneralSetFunctionType::AVG: {
            if (values.empty()) {
              return Null{};
            }
            Value sum = *values.front();
            for (auto valueIt = values.begin() + 1; valueIt != values.end();
                 valueIt++) {
              sum = ValuesAdd(sum, **valueIt, func);
            }
            if (func.type == gql::ast::GeneralSetFunctionType::AVG) {
              return ValuesDivide(
                  sum, Value{static_cast<int64_t>(values.size())}, func);
            }
            return sum;
          }
        }
        return Null{};
      },
      [&](const gql::ast::BinarySetFunction&) {
        // Feature disabled
        assert(false);
        return Value{Null{}};
      });
}

Value Demo::Evaluate(const gql::ast::LetValueExpression& expr,
                     const ExecutionContext& context) {
  BindingTable record;
  auto inserter = record.CreateColumnInserter();
  for (auto& item : expr.definitions) {
    inserter.Set(item.var.name, Evaluate(*item.expr, context));
  }

  auto letContext = context.MakeChild();
  letContext.workingRecord.table = &record;
  letContext.workingRecord.index = 0;
  return Evaluate(*expr.expression, letContext);
}

Value Demo::EvaluateValueQueryExpression(const gql::ast::ProcedureBody& expr,
                                         const ExecutionContext& context) {
  auto childContext = context.MakeCopy();
  auto result = ProcessAndCheckType(expr, childContext);
  if (!result.has_value()) {
    assert(false);
    return Null{};
  }
  if (result->size() == 0) {
    return Null{};
  }
  if (result->size() == 1 && result->columns().size() == 1) {
    return result->columns().begin()->second.front();
  } else {
    assert(false);
    return Null{};
  }
}

Value Demo::Evaluate(const gql::ast::BindingTableExpression& expr,
                     const ExecutionContext& context) {
  return gql::ast::variant_switch(
      expr.option,
      [&](const gql::ast::ProcedureBodyPtr& expr) -> Value {
        auto childContext = context.MakeCopy();
        auto result = ProcessAndCheckType(*expr, childContext);
        if (!result.has_value()) {
          return Null{};
        }
        return std::make_shared<const BindingTable>(std::move(*result));
      },
      [&](const gql::ast::ValueExpressionPtr& expr) {
        // TODO: Check type
        return Evaluate(*expr, context);
      },
      [&](const gql::ast::BindingTableReference&) {
        // TODO:
        return Value{Null{}};
      },
      [&](const gql::ast::ObjectNameOrBindingVariable&) {
        // TODO:
        return Value{Null{}};
      });
}

Value Demo::Evaluate(const gql::ast::CaseExpression& expr,
                     const ExecutionContext& context) {
  auto* searchedCase = std::get_if<gql::ast::SearchedCase>(&expr);
  if (!searchedCase) {
    // Should be rewritten
    assert(false);
    return Value{Null{}};
  }
  auto processResult = [&](const gql::ast::Result& result) {
    return gql::ast::variant_switch(
        result,
        [&](const gql::ast::ResultExpression& expr) {
          return Evaluate(*expr, context);
        },
        [&](const gql::ast::NullLiteral&) { return Value{Null{}}; });
  };
  for (auto& c : searchedCase->when) {
    auto value = Evaluate(*c.condition, context);
    if (auto* boolPtr = std::get_if<bool>(&value.v)) {
      if (*boolPtr) {
        return processResult(c.result);
      }
    }
  }
  if (searchedCase->else_) {
    return processResult(*searchedCase->else_);
  }
  return Null{};
}

}  // namespace gql_demo