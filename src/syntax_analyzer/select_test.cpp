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

#include <gtest/gtest.h>

#include "test_helpers.h"

using gql::test::Field;

TEST(SyntaxAnalyzer, SelectRewrite) {
  GQL_TEST_PARSE("SELECT s FROM g MATCH (s:Account WHERE s.isBlocked)")
      .ExpectTableResult()
      .WithFields({Field("s").NodeReference().Unconditional()});
  GQL_TEST_PARSE("SELECT s FROM {MATCH (s:Account WHERE s.isBlocked) RETURN s}")
      .ExpectTableResult()
      .WithFields({Field("s").NodeReference().Unconditional()});
  GQL_TEST_PARSE(
      "SELECT s, e, t FROM g MATCH ((s:Account) -[e:transfer]-> (t:Account) "
      "WHERE s <> t)")
      .ExpectTableResult()
      .WithFields({Field("s").NodeReference().Unconditional(),
                   Field("e").EdgeReference().Unconditional(),
                   Field("t").NodeReference().Unconditional()});

#if 0
  // Standard doesn't make clear HAVING behavior without aggregation and grouping
  GQL_TEST_PARSE(
      "SELECT s FROM g MATCH (s:Account) -[e:transfer]-> (t:Account) HAVING "
      "s.isBlocked AND s.name = 'Steven'")
      .ExpectTableResult()
      .WithFields({Field("s").NodeReference().Unconditional()});
#endif

  GQL_TEST_PARSE(
      "SELECT COUNT(e) AS cnt, STDDEV_SAMP(e.amount) AS amountDev, "
      "AVG(e.amount) AS "
      "amountAvg FROM g MATCH (s:Account)-[e:transfer|isLocatedIn]->(t)")
      .ExpectTableResult()
      .WithFields({Field("cnt"), Field("amountDev"), Field("amountAvg")});

  GQL_TEST_PARSE(
      "SELECT a, sum(e.amount) as amount1, sum(e2.amount) as amount2 FROM g "
      "MATCH (a:Account)-[e:transfer]->(b:Account)-[e2:transfer]->(c:Account) "
      "GROUP BY a")
      .ExpectTableResult()
      .WithFields({Field("a"), Field("amount1"), Field("amount2")});

  GQL_TEST_PARSE(
      "SELECT s FROM g MATCH (s:Account) ORDER BY s.name OFFSET 1 LIMIT 3")
      .ExpectTableResult()
      .WithFields({Field("s"), Field::Any()});

  GQL_TEST_PARSE(
      "SELECT s.name AS sName, e.amount as amt, t FROM g MATCH (s:Account) "
      "-[e:transfer]-> (t:Account) ORDER BY amt, s.name LIMIT 1")
      .ExpectTableResult()
      .WithFields({Field("sName"), Field("amt"), Field("t"), Field::Any()});

  GQL_TEST_PARSE(
      "SELECT s.name AS cityName FROM g MATCH (s:City) ORDER BY "
      "s.name")
      .ExpectTableResult()
      .WithFields({Field("cityName"), Field::Any()});

  GQL_TEST_PARSE(
      "SELECT DISTINCT s.name AS cityName FROM g MATCH (s:City) ORDER BY "
      "s.name")
      .ExpectTableResult()
      .WithFields({Field("cityName"), Field::Any()});

  GQL_TEST_PARSE(
      "MATCH (person:Person)-[:KNOWS]-(friend:Person) "
      "RETURN DISTINCT friend "
      "NEXT MATCH (friend)<-[membership:hasMember]-(forum:Forum) "
      "RETURN forum, collect_list(friend) AS friends GROUP BY forum "
      "NEXT RETURN forum.title AS forumName, friends "
      "ORDER BY forum.forumid")
      .ExpectTableResult()
      .WithFields({Field("forumName"), Field("friends")});
}

TEST(SyntaxAnalyzer, NamedProcedureCallCurrentSupport) {
  GQL_TEST_PARSE("CALL foo()").ExpectOmittedResult();
  GQL_TEST_PARSE("CALL foo(1) YIELD x RETURN x")
      .ExpectTableResult()
      .WithFields({Field("x")});
  GQL_TEST_PARSE("CALL foo(1) YIELD x AS y RETURN y")
      .ExpectTableResult()
      .WithFields({Field("y")});
  GQL_TEST_PARSE("MATCH (n) CALL foo() RETURN n")
      .ExpectTableResult()
      .WithFields({Field("n").NodeReference().Unconditional()});
  GQL_TEST_PARSE("MATCH (n) CALL foo(1) YIELD x RETURN n, x")
      .ExpectTableResult()
      .WithFields({Field("n").NodeReference().Unconditional(), Field("x")});
  GQL_TEST_PARSE("MATCH (n) OPTIONAL CALL foo(1) YIELD x RETURN n, x")
      .ExpectTableResult()
      .WithFields({Field("n").NodeReference().Unconditional(), Field("x")});
}
