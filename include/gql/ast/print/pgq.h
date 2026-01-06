#pragma once
namespace gql::ast::print {
template <>
struct Printer<PgqLabelAndProperties> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PgqLabelAndProperties& v) {
    variant_switch(
        v.label, [&os](const LabelName& v) { os << "LABEL" << v.name; },
        [&os](const PgqDefaultLabel& v) { os << "DEFAULT LABEL"; });
    if (v.properties) {
      os << *v.properties;
    }
  }
};

template <>
struct Printer<PgqColumnNameList> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PgqColumnNameList& v) {
    os << "(" << Sequence(v, ",") << ")";
  }
};

template <>
struct Printer<PgqDerivedProperty> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PgqDerivedProperty& v) {
    os << v.expr;
    if (v.name) {
      os << "AS " << *v.name;
    }
  }
};

template <>
struct Printer<PgqElementProperties> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PgqElementProperties& v) {
    variant_switch(
        v, [&os](const PgqNoProperties& v) { os << "NO PROPERTIES"; },
        [&os](const PgqPropertiesAre& v) {
          os << "PROPERTIES ALL COLUMNS";
          if (v.exceptColumnNames) {
            os << "EXCEPT" << *v.exceptColumnNames;
          }
        },
        [&os](const PgqDerivedPropertyList& v) {
          os << "PROPERTIES (" << Sequence(v, ",") << ")";
        });
  }
};

template <>
struct Printer<std::variant<PgqLabelAndPropertiesList, PgqElementProperties> > {
  template <typename OutputStream>
  static void Print(
      OutputStream& os,
      const std::variant<PgqLabelAndPropertiesList, PgqElementProperties>& v) {
    variant_switch(
        v,
        [&os](const PgqLabelAndPropertiesList& v) { os << Sequence(v, " "); },
        [&os](const PgqElementProperties& v) { os << v; });
  }
};

template <>
struct Printer<PgqElementKeys> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PgqElementKeys& v) {
    variant_switch(
        v,
        [&os](const PgqNodeElementKey& v) {
          if (v) {
            os << "KEY (" << Sequence(*v, ",") << ")";
          }
        },
        [&os](const PgqEdgeElementKeys& v) {
          if (v.key) {
            os << "KEY (" << Sequence(*v.key, ",") << ")";
          }
          // v.sourceKey
          os << "SOURCE KEY ";
          os << v.sourceKey.edgeColumnNameList;
          os << "REFERENCES";
          os << v.sourceKey.elementAliasReference;
          if (v.sourceKey.nodeColumnNameList) {
            os << *v.sourceKey.nodeColumnNameList;
          }
          // v.destinationKey
          os << "DESTINATION KEY ";
          os << v.destinationKey.edgeColumnNameList;
          os << "REFERENCES";
          os << v.destinationKey.elementAliasReference;
          if (v.destinationKey.nodeColumnNameList) {
            os << *v.destinationKey.nodeColumnNameList;
          }
        }

    );
  }
};

template <>
struct Printer<PgqElement> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PgqElement& v) {
    os << Sequence(v.elementName.path, ".");
    if (!v.elementName.path.empty()) {
      os << ".";
    }
    os << v.elementName.elementName;
    if (v.alias) {
      os << "AS" << *v.alias;
    }
    os << v.elementKeys;

    if (v.properties) {
      os << *v.properties;
    }
    if (v.dynamicLabel) {
      os << "DYNAMIC LABEL (" << v.dynamicLabel->name << ")";
    }
    if (v.dynamicProperties) {
      os << "DYNAMIC PROPERTIES (" << v.dynamicProperties->name << ")";
    }
  }
};

template <>
struct Printer<PgqCreateGraphStatement> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PgqCreateGraphStatement& v) {
    os << "CREATE";
    switch (v.createType) {
      case CreateGraphStatement::CreateType::Default:
        os << "PROPERTY GRAPH";
        break;
      case CreateGraphStatement::CreateType::IfNotExists:
        os << "PROPERTY GRAPH IF NOT EXISTS";
        break;
      case CreateGraphStatement::CreateType::OrReplace:
        os << "OR REPLACE PROPERTY GRAPH";
        break;
    }
    os << v.graph;
    os << "NODE TABLES (" << Sequence(v.nodeTables, ",") << ")";

    if (!v.edgeTables.empty()) {
      os << "EDGE TABLES (" << Sequence(v.edgeTables, ",") << ")";
    }
  }
};
}  // namespace gql::ast::print