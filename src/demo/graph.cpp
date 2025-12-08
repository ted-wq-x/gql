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

#include "graph.h"

namespace gql_demo {

Graph::Node& Graph::AddNode() {
  NodeId id = ++lastNodeId_;
  auto [nodeIt, inserted] = nodes_.emplace(id, std::make_unique<Node>());
  assert(inserted);
  Node* node = nodeIt->second.get();
  node->id = id;
  return *node;
}

Graph::Edge& Graph::AddEdge(Node* source, Node* target, bool directed) {
  EdgeId id = ++lastEdgeId_;
  auto [edgeIt, inserted] = edges_.emplace(id, std::make_unique<Edge>());
  assert(inserted);
  Edge* edge = edgeIt->second.get();
  edge->id = id;
  edge->source = source->id;
  edge->target = target->id;
  edge->directed = directed;
  source->edges.insert(id);
  target->edges.insert(id);
  return *edge;
}

bool Graph::DeleteNode(NodeId id) {
  auto it = nodes_.find(id);
  if (it == nodes_.end()) {
    return false;
  }
  assert(it->second->edges.empty());
  nodes_.erase(it);
  return true;
}

bool Graph::DeleteEdge(EdgeId id) {
  auto it = edges_.find(id);
  if (it == edges_.end()) {
    return false;
  }
  nodes_[it->second->source]->edges.erase(id);
  nodes_[it->second->target]->edges.erase(id);
  edges_.erase(it);
  return true;
}

}  // namespace gql_demo