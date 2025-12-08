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

#include "table_to_json.h"

#include "binding_table.h"
#include "gql/ast/algorithm.h"
#include "graph.h"

using json = nlohmann::json;

namespace gql_demo::helpers {

constexpr const char* kInvalidReference = "#invalid_reference#";

void ValueToJson(json& j, const Value& value, const Graph* graph = nullptr);
void TableToJson(json& j, const BindingTable& table, const Graph& graph);

void ElementToJson(json& dict, const Graph::Element& element) {
  auto& j = dict["labels"];
  j = json::array();
  for (auto label : element.labels) {
    j.push_back(label);
  }
  if (!element.properties.empty()) {
    auto& jsonProps = dict["properties"];
    for (auto& prop : element.properties) {
      ValueToJson(jsonProps[prop.first], prop.second);
    }
  }
}

void NodeToJson(json& jsonValue, Graph::NodeId nodeId, const Graph& graph) {
  if (auto* node = graph.GetNode(nodeId)) {
    ElementToJson(jsonValue, *node);
    jsonValue["id"] = nodeId;
  } else {
    jsonValue = kInvalidReference;
  }
}

void EdgeToJson(json& jsonValue, Graph::EdgeId edgeId, const Graph& graph) {
  if (auto* edge = graph.GetEdge(edgeId)) {
    ElementToJson(jsonValue, *edge);
    jsonValue["id"] = edgeId;
  } else {
    jsonValue = kInvalidReference;
  }
}

void ValueToJson(json& j, const Value& value, const Graph* graph) {
  gql::ast::variant_switch(
      value.v, [&](const Null&) { j["type"] = "null"; },
      [&](double val) {
        j["type"] = "double";
        j["value"] = val;
      },
      [&](bool val) {
        j["type"] = "bool";
        j["value"] = val;
      },
      [&](int64_t val) {
        j["type"] = "integer";
        j["value"] = val;
      },
      [&](const std::string& val) {
        j["type"] = "string";
        j["value"] = val;
      },
      [&](const NodeReference& ref) {
        assert(graph);
        j["type"] = "node";
        NodeToJson(j["value"], ref.nodeId, *graph);
      },
      [&](const EdgeReference& ref) {
        assert(graph);
        j["type"] = "edge";
        EdgeToJson(j["value"], ref.edgeId, *graph);
      },
      [&](const PathReference& ref) {
        assert(graph);
        j["type"] = "path";
        auto& jsonValue = j["value"];
        for (size_t i = 0; i < ref.nodeIds.size(); ++i) {
          if (i > 0) {
            auto& edgeValue = jsonValue.emplace_back();
            if (auto* edge = graph->GetEdge(ref.edgeIds[i - 1])) {
              edgeValue["id"] = edge->id;
              auto& direction = edgeValue["direction"];
              if (edge->directed) {
                direction = (edge->target == ref.nodeIds[i]) ? "right" : "left";
              } else {
                direction = "none";
              }
              ElementToJson(edgeValue, *edge);
            } else {
              jsonValue = kInvalidReference;
            }
          }
          NodeToJson(jsonValue.emplace_back(), ref.nodeIds[i], *graph);
        }
      },
      [&](const List& lst) {
        j["type"] = "list";
        auto& jsonValue = j["value"];
        jsonValue = json::array();
        for (auto& item : lst) {
          ValueToJson(jsonValue.emplace_back(), *item, graph);
        }
      },
      [&](const BindingTableReference& table) {
        assert(graph);
        j["type"] = "table";
        TableToJson(j["value"], *table, *graph);
      },
      [&](const Record& record) {
        j["type"] = "record";
        auto& jsonValue = j["value"];
        jsonValue = json::object();
        for (auto& [key, value] : record) {
          ValueToJson(jsonValue[key], value, graph);
        }
      });
}

void TableToJson(json& j, const BindingTable& table, const Graph& graph) {
  auto columnSequence = table.preferredColumnSequence();
  j["columns"] = columnSequence;
  auto& jsonRows = j["rows"];
  jsonRows = json::array();
  for (size_t rowIdx = 0; rowIdx < table.size(); ++rowIdx) {
    auto& jsonRow = jsonRows.emplace_back();
    for (auto& col : columnSequence) {
      ValueToJson(jsonRow.emplace_back(),
                  table.columns().find(col)->second[rowIdx], &graph);
    }
  }
}

void GraphToJson(json& j, const Graph& graph) {
  auto& jsonNodes = j["nodes"];
  jsonNodes = json::array();
  for (auto& node : graph.nodes()) {
    auto& jsonNode = jsonNodes.emplace_back();
    jsonNode["id"] = node.id;
    ElementToJson(jsonNode, node);
    auto& jsonNodeEdges = j["edges"];
    jsonNodeEdges = json::array();
    for (auto e : node.edges) {
      jsonNodeEdges.push_back(e);
    }
  }

  auto& jsonEdges = j["edges"];
  jsonEdges = json::array();
  for (auto& edge : graph.edges()) {
    auto& jsonEdge = jsonEdges.emplace_back();
    jsonEdge["id"] = edge.id;
    jsonEdge["directed"] = edge.directed;
    jsonEdge["source"] = edge.source;
    jsonEdge["target"] = edge.target;
    ElementToJson(jsonEdge, edge);
  }
}

}  // namespace gql_demo::helpers