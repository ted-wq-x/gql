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

#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "value.h"

namespace gql_demo {

class Graph {
 public:
  using EdgeId = int64_t;
  using NodeId = int64_t;
  using EdgeOrNodeId = int64_t;
  using LabelSet = std::unordered_set<std::string>;
  using Properties = std::unordered_map<std::string, Value>;

  struct Element {
    LabelSet labels;
    Properties properties;

    Element() = default;
    Element(const Element&) = delete;
    void operator=(const Element&) = delete;
  };

  struct Node : Element {
    NodeId id;
    std::unordered_set<EdgeId> edges;

    Node() = default;
    Node(const Node&) = delete;
  };

  struct Edge : Element {
    EdgeId id;
    bool directed = false;
    NodeId source;
    NodeId target;

    Edge() = default;
    Edge(const Edge&) = delete;
  };

  auto nodes() const {
    return nodes_ | std::views::transform([](const auto& pair) -> const Node& {
             return *pair.second.get();
           });
  }
  auto edges() const {
    return edges_ | std::views::transform([](const auto& pair) -> const Edge& {
             return *pair.second.get();
           });
  }

  Node& AddNode();
  Edge& AddEdge(Node* source, Node* target, bool directed);
  bool DeleteNode(NodeId id);
  bool DeleteEdge(EdgeId id);

  const Node* GetNode(NodeId id) const;
  Node* GetNode(NodeId id);
  const Edge* GetEdge(EdgeId id) const;
  Edge* GetEdge(EdgeId id);

 private:
  std::unordered_map<NodeId, std::unique_ptr<Node>> nodes_;
  std::unordered_map<EdgeId, std::unique_ptr<Edge>> edges_;

  NodeId lastNodeId_ = 0;
  EdgeId lastEdgeId_ = 0;
};

inline const Graph::Node* Graph::GetNode(NodeId id) const {
  auto it = nodes_.find(id);
  return (it != nodes_.end()) ? it->second.get() : nullptr;
}

inline Graph::Node* Graph::GetNode(NodeId id) {
  auto it = nodes_.find(id);
  return (it != nodes_.end()) ? it->second.get() : nullptr;
}

inline const Graph::Edge* Graph::GetEdge(EdgeId id) const {
  auto it = edges_.find(id);
  return (it != edges_.end()) ? it->second.get() : nullptr;
}

inline Graph::Edge* Graph::GetEdge(EdgeId id) {
  auto it = edges_.find(id);
  return (it != edges_.end()) ? it->second.get() : nullptr;
}

}  // namespace gql_demo