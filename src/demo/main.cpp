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

#include <httplib.h>
#undef TRUE
#undef FALSE
#undef MAX
#undef MIN
#include <fstream>
#include <iostream>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <ranges>
#include <sstream>

#include "error.h"
#include "gql/ast/print.h"
#include "gql/parser/parser.h"
#include "gql/standard/features.h"
#include "gql/syntax_analyzer/syntax_analyzer.h"
#include "helpers/table_to_json.h"
#include "timeout_watchdog.h"

using json = nlohmann::json;
using json_validator = nlohmann::json_schema::json_validator;

// The schema is defined based upon a string literal
static json RequestSchema = R"(
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "GQL Request",
  "description": "Schema for GQL request format",
  "type": "array",
  "items": {
    "type": "object",
    "properties": {
      "requests": {
        "type": "array",
        "description": "Array of GQL programs to execute on same graph instance",
        "items": {
          "type": "object",
          "properties": {
            "program": {
              "type": "string",
              "description": "The GQL program/query to execute"
            },
            "dump_graph": {
              "type": "boolean",
              "description": "Whether to dump the graph state after execution"
            }
          },
          "required": ["program"],
          "additionalProperties": false
        },
        "minItems": 1
      }
    },
    "required": ["requests"],
    "additionalProperties": false
  }
}

)"_json;

static auto GetUnsupportedFeatures() {
  using gql::standard::Feature;
  static const Feature unsupportedFeatures[] = {
      Feature::G005, Feature::G007, Feature::G012, Feature::GA05, Feature::GA06,
      Feature::GD04, Feature::GE06, Feature::GF01, Feature::GF02, Feature::GF03,
      Feature::GF11, Feature::GS01, Feature::GS02, Feature::GS03, Feature::GS04,
      Feature::GS05, Feature::GS06, Feature::GS07, Feature::GS15, Feature::GS16,
      Feature::GT01, Feature::GT02, Feature::GP04, Feature::GP11, Feature::GP16,
      Feature::GC01, Feature::GC02, Feature::GC04, Feature::GQ01, Feature::GQ14,
      Feature::GQ16, Feature::GQ23, Feature::GV35, Feature::GV39, Feature::GV41,
      Feature::GV45, Feature::GV60};
  return std::views::all(unsupportedFeatures);
}

int GetEnvIntOr(const char* key, int defaultValue) {
  if (const char* envStr = std::getenv(key)) {
    char* end;
    long value = std::strtol(envStr, &end, 10);  // Base 10
    if (*end == '\0') {
      return value;
    }
  }
  return defaultValue;
}

void ErrorToJson(json& j,
                 const char* query,
                 gql::ast::InputPosition inputPosition,
                 const std::string& msg,
                 int errorCode) {
  j = json{{"code", errorCode}, {"message", msg}};
  if (inputPosition.IsSet()) {
    j["position"] = {{"line", inputPosition.line}, {"col", inputPosition.col}};
  }

  std::stringstream str;
  if (inputPosition.IsSet()) {
    str << "Error in query at position " << inputPosition.line << ":"
        << inputPosition.col + 1 << "\n"
        << gql::FormatErrorMessageInQuery(query, inputPosition, msg);
  } else {
    str << "Error in query at unknown position: " << msg;
  }
  j["formatted"] = str.str();
}

bool RunRequest(const json& request,
                json& response,
                gql_demo::Graph& graph,
                gql::parser::ParserCache* parserCache,
                gql_demo::TimeoutWatchdog* timeoutWatchdog) {
  auto programStr = request["program"].get<std::string>();
  response["program"] = programStr;
  gql::ast::GQLProgram programAst;
  try {
    auto features = GetUnsupportedFeatures();
    gql::parser::ParserConfig parserConfig;
    parserConfig.unsupportedFeatures.insert(features.begin(), features.end());
    gql::SyntaxAnalyzerConfig syntaxAnalyzerConfig;
    syntaxAnalyzerConfig.unsupportedFeatures.insert(features.begin(),
                                                    features.end());
    syntaxAnalyzerConfig.rewriteCoalesceCase = true;
    syntaxAnalyzerConfig.rewriteNullIfCase = true;
    syntaxAnalyzerConfig.rewriteSimpleCase = true;

    // First, analyze the program while checking for unsupported features. If
    // the program uses an unsupported feature, analyze it again without
    // checking features. The goal is to always report standard violations, even
    // in features that are not supported.
    std::optional<
        std::tuple<gql::ast::InputPosition, std::string, int /*errorCode*/>>
        unsupportedFeatureError;
    while (true) {
      try {
        programAst = gql::parser::ParseProgram(programStr.c_str(), parserConfig,
                                               parserCache);
        gql::AnalyzeSyntax(programAst, syntaxAnalyzerConfig);

        response["rewrittenProgram"] = gql::ast::PrintTree(programAst);
        if (unsupportedFeatureError) {
          ErrorToJson(response["error"], programStr.c_str(),
                      std::get<0>(*unsupportedFeatureError),
                      std::get<1>(*unsupportedFeatureError),
                      std::get<2>(*unsupportedFeatureError));
          return false;
        }
        break;
      } catch (const gql::UnsupportedFeatureError& e) {
        parserConfig.unsupportedFeatures.clear();
        syntaxAnalyzerConfig.unsupportedFeatures.clear();
        unsupportedFeatureError.emplace(e.inputPosition(), e.message(),
                                        static_cast<int>(e.errorCode()));
      }
    }
  } catch (const gql::ParserError& e) {
    response["rewrittenProgram"] = gql::ast::PrintTree(programAst);
    ErrorToJson(response["error"], programStr.c_str(), e.inputPosition(),
                e.message(), static_cast<int>(e.errorCode()));
    return false;
  }

  gql_demo::Demo demo(graph);

  if (timeoutWatchdog) {
    demo.SetTimeoutWatchdog(*timeoutWatchdog);
  }

  try {
    auto result = demo.Process(programAst);

    if (request.value("dump_graph", false)) {
      gql_demo::helpers::GraphToJson(response["graph"], graph);
    }

    auto& jsonResult = response["result"];
    if (result) {
      gql_demo::helpers::TableToJson(jsonResult["table"], *result, graph);
    } else {
      jsonResult = json::object();
    }
  } catch (const gql_demo::GQLRuntimeError& e) {
    ErrorToJson(response["error"], programStr.c_str(), e.inputPosition(),
                e.message(), 10000);
    return false;
  }
  return true;
}

std::string RunTask(const json& jsonSessions,
                    gql::parser::ParserCache* parserCache = nullptr) {
  json_validator validator;
  validator.set_root_schema(RequestSchema);
  validator.validate(jsonSessions);

  json jsonReport;
  jsonReport = json::array();
  gql_demo::Graph graph;

  std::optional<gql_demo::TimeoutWatchdog> timeoutWatchdog;
  if (auto timeout = GetEnvIntOr("GQL_DEMO_TIMEOUT", -1); timeout > 0) {
    timeoutWatchdog.emplace(timeout * 1000);
  }
  for (auto& session : jsonSessions) {
    auto& outSession = jsonReport.emplace_back();
    outSession = json::array();
    for (auto& request : session["requests"]) {
      if (!RunRequest(request, outSession.emplace_back(), graph, parserCache,
                      timeoutWatchdog ? &*timeoutWatchdog : nullptr)) {
        break;
      }
    }
  }
  std::stringstream str;
  str << std::setw(4) << jsonReport;
  return str.str();
}

int main(int argc, char* argv[]) {
  // Check for --http mode first
  if (argc >= 2 && strcmp(argv[1], "--http") == 0) {
    httplib::Server svr;
    if (auto timeout = GetEnvIntOr("GQL_HTTP_KEEP_ALIVE_TIMEOUT", -1);
        timeout >= 0) {
      svr.set_keep_alive_timeout(timeout);
    }
    if (auto nThreads = GetEnvIntOr("GQL_HTTP_SERVER_THREADS", -1);
        nThreads > 0) {
      svr.new_task_queue = [nThreads] {
        return new httplib::ThreadPool(nThreads);
      };
    }

    svr.set_logger(
        [](const httplib::Request& req, const httplib::Response& res) {
          std::cout << req.method << " " << req.path << " -> " << res.status
                    << std::endl;
        });

    svr.set_exception_handler(
        [](const auto&, auto& res, std::exception_ptr ep) {
          res.status = httplib::StatusCode::InternalServerError_500;
          json errorResponse;
          try {
            std::rethrow_exception(ep);
          } catch (const gql_demo::TimeoutException& e) {
            errorResponse = {{"error", e.what()}, {"status", 503}};
            res.status = httplib::StatusCode::ServiceUnavailable_503;
          } catch (const std::exception& e) {
            errorResponse = {{"error", e.what()}, {"status", 500}};
          } catch (...) {
            errorResponse = {{"error", "Unknown Exception"}, {"status", 500}};
          }
          res.set_content(errorResponse.dump(), "application/json");
        });

    // Handler for both GET and POST requests
    auto queryHandler = [](const httplib::Request& req,
                           httplib::Response& res) {
      thread_local static gql::parser::ParserCache parserCache;
      auto reportStr = RunTask(json::parse(req.body), &parserCache);
      res.set_content(reportStr, "application/json");
    };

    svr.Get("/api/query", queryHandler);
    svr.Post("/api/query", queryHandler);

    // Root path for health checks
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
      res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    int port = GetEnvIntOr("GQL_HTTP_PORT", 8080);
    std::cout << "HTTP server listening on port " << port << "\n";
    return svr.listen("0.0.0.0", port) ? 0 : 1;
  }

  // Read from file if filename provided, otherwise read from stdin
  try {
    json inputJson;
    if (argc >= 2) {
      // Read from file
      std::ifstream file(argv[1], std::ios::binary);
      if (!file) {
        std::cerr << "Error: Cannot open file " << argv[1] << "\n";
        return 1;
      }
      inputJson = json::parse(file);
    } else {
      // Read from stdin
      inputJson = json::parse(std::cin);
    }

    auto reportStr = RunTask(inputJson);
    std::cout << reportStr;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }
}
