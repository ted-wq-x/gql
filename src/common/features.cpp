#include "gql/standard/features.h"

#include <cassert>

namespace gql::standard {

const char* GetFeatureDescription(Feature feature) {
  switch (feature) {
    case Feature::G002:
      return "Different-edges match mode";
    case Feature::G003:
      return "Explicit REPEATABLE ELEMENTS keyword";
    case Feature::G004:
      return "Path variables";
    case Feature::G005:
      return "Path search prefix in a path pattern";
    case Feature::G006:
      return "Graph pattern KEEP clause: path mode prefix";
    case Feature::G007:
      return "Graph pattern KEEP clause: path search prefix";
    case Feature::G010:
      return "Explicit WALK keyword";
    case Feature::G011:
      return "Advanced path modes: TRAIL";
    case Feature::G012:
      return "Advanced path modes: SIMPLE";
    case Feature::G013:
      return "Advanced path modes: ACYCLIC";
    case Feature::G014:
      return "Explicit PATH/PATHS keywords";
    case Feature::G015:
      return "All path search: explicit ALL keyword";
    case Feature::G016:
      return "Any path search";
    case Feature::G017:
      return "All shortest path search";
    case Feature::G018:
      return "Any shortest path search";
    case Feature::G019:
      return "Counted shortest path search";
    case Feature::G020:
      return "Counted shortest group search";
    case Feature::G030:
      return "Path multiset alternation";
    case Feature::G031:
      return "Path multiset alternation: variable length path operands";
    case Feature::G032:
      return "Path pattern union";
    case Feature::G033:
      return "Path pattern union: variable length path operands";
    case Feature::G035:
      return "Quantified paths";
    case Feature::G036:
      return "Quantified edges";
    case Feature::G037:
      return "Questioned paths";
    case Feature::G038:
      return "Parenthesized path pattern expression";
    case Feature::G039:
      return "Simplified path pattern expression: full defaulting";
    case Feature::G041:
      return "Non-local element pattern predicates";
    case Feature::G043:
      return "Complete full edge patterns";
    case Feature::G044:
      return "Basic abbreviated edge patterns";
    case Feature::G045:
      return "Complete abbreviated edge patterns";
    case Feature::G046:
      return "Relaxed topological consistency: adjacent vertex patterns";
    case Feature::G047:
      return "Relaxed topological consistency: concise edge patterns";
    case Feature::G048:
      return "Parenthesized path pattern: subpath variable declaration";
    case Feature::G049:
      return "Parenthesized path pattern: path mode prefix";
    case Feature::G050:
      return "Parenthesized path pattern: WHERE clause";
    case Feature::G051:
      return "Parenthesized path pattern: non-local predicates";
    case Feature::G060:
      return "Bounded graph pattern quantifiers";
    case Feature::G061:
      return "Unbounded graph pattern quantifiers";
    case Feature::G074:
      return "Label expression: wildcard label";
    case Feature::G080:
      return "Simplified path pattern expression: basic defaulting";
    case Feature::G081:
      return "Simplified path pattern expression: full overrides";
    case Feature::G082:
      return "Simplified path pattern expression: basic overrides";
    case Feature::G100:
      return "ELEMENT_ID function";
    case Feature::G110:
      return "IS DIRECTED predicate";
    case Feature::G111:
      return "IS LABELED predicate";
    case Feature::G112:
      return "IS SOURCE and IS DESTINATION predicate";
    case Feature::G113:
      return "ALL_DIFFERENT predicate";
    case Feature::G114:
      return "SAME predicate";
    case Feature::G115:
      return "PROPERTY_EXISTS predicate";
    case Feature::GA01:
      return "IEEE 754 floating point operations";
    case Feature::GA03:
      return "Explicit ordering of nulls";
    case Feature::GA04:
      return "Universal comparison";
    case Feature::GA05:
      return "Cast specification";
    case Feature::GA06:
      return "Value type predicate";
    case Feature::GA07:
      return "Ordering by discarded binding variables";
    case Feature::GA08:
      return "GQL-status objects with diagnostic records";
    case Feature::GA09:
      return "Comparison of paths";
    case Feature::GB01:
      return "Long identifiers";
    case Feature::GB02:
      return "Double minus sign comments";
    case Feature::GB03:
      return "Double solidus comments";
    case Feature::GC01:
      return "Graph schema management";
    case Feature::GC02:
      return "Graph schema management: IF [ NOT ] EXISTS";
    case Feature::GC03:
      return "Graph type: IF [ NOT ] EXISTS";
    case Feature::GC04:
      return "Graph management";
    case Feature::GC05:
      return "Graph management: IF [ NOT ] EXISTS";
    case Feature::GD01:
      return "Updatable graphs";
    case Feature::GD02:
      return "Graph label set changes";
    case Feature::GD03:
      return "DELETE statement: subquery support";
    case Feature::GD04:
      return "DELETE statement: simple expression support";
    case Feature::GE01:
      return "Graph reference value expressions";
    case Feature::GE02:
      return "Binding table reference value expressions";
    case Feature::GE03:
      return "Let-binding of variables in expressions";
    case Feature::GE04:
      return "Graph parameters";
    case Feature::GE05:
      return "Binding table parameters";
    case Feature::GE06:
      return "Path value construction";
    case Feature::GE07:
      return "Boolean XOR";
    case Feature::GE08:
      return "Reference parameters";
    case Feature::GE09:
      return "Horizontal aggregation";
    case Feature::GF01:
      return "Enhanced numeric functions";
    case Feature::GF02:
      return "Trigonometric functions";
    case Feature::GF03:
      return "Logarithmic functions";
    case Feature::GF04:
      return "Enhanced path functions";
    case Feature::GF05:
      return "Multi-character TRIM function";
    case Feature::GF06:
      return "Explicit TRIM function";
    case Feature::GF07:
      return "Byte string TRIM function";
    case Feature::GF10:
      return "Advanced aggregate functions: general set functions";
    case Feature::GF11:
      return "Advanced aggregate functions: binary set functions";
    case Feature::GF12:
      return "CARDINALITY function";
    case Feature::GF13:
      return "SIZE function";
    case Feature::GF20:
      return "Aggregate functions in sort keys";
    case Feature::GG01:
      return "Graph with an open graph type";
    case Feature::GG02:
      return "Graph with a closed graph type";
    case Feature::GG03:
      return "Graph type inline specification";
    case Feature::GG04:
      return "Graph type like a graph";
    case Feature::GG05:
      return "Graph from a graph source";
    case Feature::GG20:
      return "Explicit element type names";
    case Feature::GG21:
      return "Explicit element type key label sets";
    case Feature::GG22:
      return "Element type key label set inference";
    case Feature::GG23:
      return "Optional element type key label sets";
    case Feature::GG24:
      return "Relaxed structural consistency";
    case Feature::GG25:
      return "Relaxed key label set uniqueness for edge types";
    case Feature::GG26:
      return "Relaxed property value type consistency";
    case Feature::GH01:
      return "External object references";
    case Feature::GH02:
      return "Undirected edge patterns";
    case Feature::GL01:
      return "Hexadecimal literals";
    case Feature::GL02:
      return "Octal literals";
    case Feature::GL03:
      return "Binary literals";
    case Feature::GL04:
      return "Exact number in common notation without suffix";
    case Feature::GL05:
      return "Exact number in common notation or as decimal integer with "
             "suffix";
    case Feature::GL06:
      return "Exact number in scientific notation with suffix";
    case Feature::GL07:
      return "Approximate number in common notation or as decimal integer with "
             "suffix";
    case Feature::GL08:
      return "Approximate number in scientific notation with suffix";
    case Feature::GL09:
      return "Optional float number suffix";
    case Feature::GL10:
      return "Optional double number suffix";
    case Feature::GL11:
      return "Opt-out character escaping";
    case Feature::GL12:
      return "SQL datetime and interval formats";
    case Feature::GP01:
      return "Inline procedure";
    case Feature::GP02:
      return "Inline procedure with implicit nested variable scope";
    case Feature::GP03:
      return "Inline procedure with explicit nested variable scope";
    case Feature::GP04:
      return "Named procedure calls";
    case Feature::GP05:
      return "Procedure-local value variable definitions";
    case Feature::GP06:
      return "Procedure-local value variable definitions: value variables "
             "based on simple expressions";
    case Feature::GP07:
      return "Procedure-local value variable definitions: value variable based "
             "on subqueries";
    case Feature::GP08:
      return "Procedure-local binding table variable definitions";
    case Feature::GP09:
      return "Procedure-local binding table variable definitions: binding "
             "table variables based on simple expressions or references";
    case Feature::GP10:
      return "Procedure-local binding table variable definitions: binding "
             "table variables based on subqueries";
    case Feature::GP11:
      return "Procedure-local graph variable definitions";
    case Feature::GP12:
      return "Procedure-local graph variable definitions: graph variables "
             "based on simple expressions or references";
    case Feature::GP13:
      return "Procedure-local graph variable definitions: graph variables "
             "based on subqueries";
    case Feature::GP14:
      return "Binding tables as procedure arguments";
    case Feature::GP15:
      return "Graphs as procedure arguments";
    case Feature::GP16:
      return "AT schema clause";
    case Feature::GP17:
      return "Binding variable definition block";
    case Feature::GP18:
      return "Catalog and data statement mixing";
    case Feature::GQ01:
      return "USE graph clause";
    case Feature::GQ02:
      return "Composite query: OTHERWISE";
    case Feature::GQ03:
      return "Composite query: UNION";
    case Feature::GQ04:
      return "Composite query: EXCEPT DISTINCT";
    case Feature::GQ05:
      return "Composite query: EXCEPT ALL";
    case Feature::GQ06:
      return "Composite query: INTERSECT DISTINCT";
    case Feature::GQ07:
      return "Composite query: INTERSECT ALL";
    case Feature::GQ08:
      return "FILTER statement";
    case Feature::GQ09:
      return "LET statement";
    case Feature::GQ10:
      return "FOR statement: list value support";
    case Feature::GQ11:
      return "FOR statement: WITH ORDINALITY";
    case Feature::GQ12:
      return "ORDER BY and page statement: OFFSET clause";
    case Feature::GQ13:
      return "ORDER BY and page statement: LIMIT clause";
    case Feature::GQ14:
      return "Complex expressions in sort keys";
    case Feature::GQ15:
      return "GROUP BY clause";
    case Feature::GQ16:
      return "Pre-projection aliases in sort keys";
    case Feature::GQ17:
      return "Element-wise group variable operations";
    case Feature::GQ18:
      return "Scalar subqueries";
    case Feature::GQ19:
      return "Graph pattern YIELD clause";
    case Feature::GQ20:
      return "Advanced linear composition with NEXT";
    case Feature::GQ21:
      return "OPTIONAL: Multiple MATCH statements";
    case Feature::GQ22:
      return "EXISTS predicate: multiple MATCH statements";
    case Feature::GQ23:
      return "FOR statement: binding table support";
    case Feature::GQ24:
      return "FOR statement: WITH OFFSET";
    case Feature::GS01:
      return "SESSION SET command: session-local graph parameters";
    case Feature::GS02:
      return "SESSION SET command: session-local binding table parameters";
    case Feature::GS03:
      return "SESSION SET command: session-local value parameters";
    case Feature::GS04:
      return "SESSION RESET command: reset all characteristics";
    case Feature::GS05:
      return "SESSION RESET command: reset session schema";
    case Feature::GS06:
      return "SESSION RESET command: reset session graph";
    case Feature::GS07:
      return "SESSION RESET command: reset time zone displacement";
    case Feature::GS08:
      return "SESSION RESET command: reset all session parameters";
    case Feature::GS10:
      return "SESSION SET command: session-local binding table parameters "
             "based on subqueries";
    case Feature::GS11:
      return "SESSION SET command: session-local value parameters based on "
             "subqueries";
    case Feature::GS12:
      return "SESSION SET command: session-local graph parameters based on "
             "simple expressions or references";
    case Feature::GS13:
      return "SESSION SET command: session-local binding table parameters "
             "based on simple expressions or references";
    case Feature::GS14:
      return "SESSION SET command: session-local value parameters based on "
             "simple expressions";
    case Feature::GS15:
      return "SESSION SET command: set time zone displacement";
    case Feature::GS16:
      return "SESSION RESET command: reset individual session parameters";
    case Feature::GT01:
      return "Explicit transaction commands";
    case Feature::GT02:
      return "Specified transaction characteristics";
    case Feature::GT03:
      return "Use of multiple graphs in a transaction";
    case Feature::GV01:
      return "8 bit unsigned integer numbers";
    case Feature::GV02:
      return "8 bit signed integer numbers";
    case Feature::GV03:
      return "16 bit unsigned integer numbers";
    case Feature::GV04:
      return "16 bit signed integer numbers";
    case Feature::GV05:
      return "Small unsigned integer numbers";
    case Feature::GV06:
      return "32 bit unsigned integer numbers";
    case Feature::GV07:
      return "32 bit signed integer numbers";
    case Feature::GV08:
      return "Regular unsigned integer numbers";
    case Feature::GV09:
      return "Specified integer number precision";
    case Feature::GV10:
      return "Big unsigned integer numbers";
    case Feature::GV11:
      return "64 bit unsigned integer numbers";
    case Feature::GV12:
      return "64 bit signed integer numbers";
    case Feature::GV13:
      return "128 bit unsigned integer numbers";
    case Feature::GV14:
      return "128 bit signed integer numbers";
    case Feature::GV15:
      return "256 bit unsigned integer numbers";
    case Feature::GV16:
      return "256 bit signed integer numbers";
    case Feature::GV17:
      return "Decimal numbers";
    case Feature::GV18:
      return "Small signed integer numbers";
    case Feature::GV19:
      return "Big signed integer numbers";
    case Feature::GV20:
      return "16 bit floating point numbers";
    case Feature::GV21:
      return "32 bit floating point numbers";
    case Feature::GV22:
      return "Specified floating point number precision";
    case Feature::GV23:
      return "Floating point type name synonyms";
    case Feature::GV24:
      return "64 bit floating point numbers";
    case Feature::GV25:
      return "128 bit floating point numbers";
    case Feature::GV26:
      return "256 bit floating point numbers";
    case Feature::GV30:
      return "Specified character string minimum length";
    case Feature::GV31:
      return "Specified character string maximum length";
    case Feature::GV32:
      return "Specified character string fixed length";
    case Feature::GV35:
      return "Byte string types";
    case Feature::GV36:
      return "Specified byte string minimum length";
    case Feature::GV37:
      return "Specified byte string maximum length";
    case Feature::GV38:
      return "Specified byte string fixed length";
    case Feature::GV39:
      return "Temporal types: date, local datetime and local time support";
    case Feature::GV40:
      return "Temporal types: zoned datetime and zoned time support";
    case Feature::GV41:
      return "Temporal types: duration support";
    case Feature::GV45:
      return "Record types";
    case Feature::GV46:
      return "Closed record types";
    case Feature::GV47:
      return "Open record types";
    case Feature::GV48:
      return "Nested record types";
    case Feature::GV50:
      return "List value types";
    case Feature::GV55:
      return "Path value types";
    case Feature::GV60:
      return "Graph reference value types";
    case Feature::GV61:
      return "Binding table reference value types";
    case Feature::GV65:
      return "Dynamic union types";
    case Feature::GV66:
      return "Open dynamic union types";
    case Feature::GV67:
      return "Closed dynamic union types";
    case Feature::GV68:
      return "Dynamic property value types";
    case Feature::GV70:
      return "Immaterial value types";
    case Feature::GV71:
      return "Immaterial value types: null type support";
    case Feature::GV72:
      return "Immaterial value types: empty type support";
    case Feature::GV90:
      return "Explicit value type nullability";
    default:
      assert(false);
      return "";
  }
}

const char* GetFeatureCodeString(Feature feature) {
  switch (feature) {
    case Feature::G002:
      return "G002";
    case Feature::G003:
      return "G003";
    case Feature::G004:
      return "G004";
    case Feature::G005:
      return "G005";
    case Feature::G006:
      return "G006";
    case Feature::G007:
      return "G007";
    case Feature::G010:
      return "G010";
    case Feature::G011:
      return "G011";
    case Feature::G012:
      return "G012";
    case Feature::G013:
      return "G013";
    case Feature::G014:
      return "G014";
    case Feature::G015:
      return "G015";
    case Feature::G016:
      return "G016";
    case Feature::G017:
      return "G017";
    case Feature::G018:
      return "G018";
    case Feature::G019:
      return "G019";
    case Feature::G020:
      return "G020";
    case Feature::G030:
      return "G030";
    case Feature::G031:
      return "G031";
    case Feature::G032:
      return "G032";
    case Feature::G033:
      return "G033";
    case Feature::G035:
      return "G035";
    case Feature::G036:
      return "G036";
    case Feature::G037:
      return "G037";
    case Feature::G038:
      return "G038";
    case Feature::G039:
      return "G039";
    case Feature::G041:
      return "G041";
    case Feature::G043:
      return "G043";
    case Feature::G044:
      return "G044";
    case Feature::G045:
      return "G045";
    case Feature::G046:
      return "G046";
    case Feature::G047:
      return "G047";
    case Feature::G048:
      return "G048";
    case Feature::G049:
      return "G049";
    case Feature::G050:
      return "G050";
    case Feature::G051:
      return "G051";
    case Feature::G060:
      return "G060";
    case Feature::G061:
      return "G061";
    case Feature::G074:
      return "G074";
    case Feature::G080:
      return "G080";
    case Feature::G081:
      return "G081";
    case Feature::G082:
      return "G082";
    case Feature::G100:
      return "G100";
    case Feature::G110:
      return "G110";
    case Feature::G111:
      return "G111";
    case Feature::G112:
      return "G112";
    case Feature::G113:
      return "G113";
    case Feature::G114:
      return "G114";
    case Feature::G115:
      return "G115";
    case Feature::GA01:
      return "GA01";
    case Feature::GA03:
      return "GA03";
    case Feature::GA04:
      return "GA04";
    case Feature::GA05:
      return "GA05";
    case Feature::GA06:
      return "GA06";
    case Feature::GA07:
      return "GA07";
    case Feature::GA08:
      return "GA08";
    case Feature::GA09:
      return "GA09";
    case Feature::GB01:
      return "GB01";
    case Feature::GB02:
      return "GB02";
    case Feature::GB03:
      return "GB03";
    case Feature::GC01:
      return "GC01";
    case Feature::GC02:
      return "GC02";
    case Feature::GC03:
      return "GC03";
    case Feature::GC04:
      return "GC04";
    case Feature::GC05:
      return "GC05";
    case Feature::GD01:
      return "GD01";
    case Feature::GD02:
      return "GD02";
    case Feature::GD03:
      return "GD03";
    case Feature::GD04:
      return "GD04";
    case Feature::GE01:
      return "GE01";
    case Feature::GE02:
      return "GE02";
    case Feature::GE03:
      return "GE03";
    case Feature::GE04:
      return "GE04";
    case Feature::GE05:
      return "GE05";
    case Feature::GE06:
      return "GE06";
    case Feature::GE07:
      return "GE07";
    case Feature::GE08:
      return "GE08";
    case Feature::GE09:
      return "GE09";
    case Feature::GF01:
      return "GF01";
    case Feature::GF02:
      return "GF02";
    case Feature::GF03:
      return "GF03";
    case Feature::GF04:
      return "GF04";
    case Feature::GF05:
      return "GF05";
    case Feature::GF06:
      return "GF06";
    case Feature::GF07:
      return "GF07";
    case Feature::GF10:
      return "GF10";
    case Feature::GF11:
      return "GF11";
    case Feature::GF12:
      return "GF12";
    case Feature::GF13:
      return "GF13";
    case Feature::GF20:
      return "GF20";
    case Feature::GG01:
      return "GG01";
    case Feature::GG02:
      return "GG02";
    case Feature::GG03:
      return "GG03";
    case Feature::GG04:
      return "GG04";
    case Feature::GG05:
      return "GG05";
    case Feature::GG20:
      return "GG20";
    case Feature::GG21:
      return "GG21";
    case Feature::GG22:
      return "GG22";
    case Feature::GG23:
      return "GG23";
    case Feature::GG24:
      return "GG24";
    case Feature::GG25:
      return "GG25";
    case Feature::GG26:
      return "GG26";
    case Feature::GH01:
      return "GH01";
    case Feature::GH02:
      return "GH02";
    case Feature::GL01:
      return "GL01";
    case Feature::GL02:
      return "GL02";
    case Feature::GL03:
      return "GL03";
    case Feature::GL04:
      return "GL04";
    case Feature::GL05:
      return "GL05";
    case Feature::GL06:
      return "GL06";
    case Feature::GL07:
      return "GL07";
    case Feature::GL08:
      return "GL08";
    case Feature::GL09:
      return "GL09";
    case Feature::GL10:
      return "GL10";
    case Feature::GL11:
      return "GL11";
    case Feature::GL12:
      return "GL12";
    case Feature::GP01:
      return "GP01";
    case Feature::GP02:
      return "GP02";
    case Feature::GP03:
      return "GP03";
    case Feature::GP04:
      return "GP04";
    case Feature::GP05:
      return "GP05";
    case Feature::GP06:
      return "GP06";
    case Feature::GP07:
      return "GP07";
    case Feature::GP08:
      return "GP08";
    case Feature::GP09:
      return "GP09";
    case Feature::GP10:
      return "GP10";
    case Feature::GP11:
      return "GP11";
    case Feature::GP12:
      return "GP12";
    case Feature::GP13:
      return "GP13";
    case Feature::GP14:
      return "GP14";
    case Feature::GP15:
      return "GP15";
    case Feature::GP16:
      return "GP16";
    case Feature::GP17:
      return "GP17";
    case Feature::GP18:
      return "GP18";
    case Feature::GQ01:
      return "GQ01";
    case Feature::GQ02:
      return "GQ02";
    case Feature::GQ03:
      return "GQ03";
    case Feature::GQ04:
      return "GQ04";
    case Feature::GQ05:
      return "GQ05";
    case Feature::GQ06:
      return "GQ06";
    case Feature::GQ07:
      return "GQ07";
    case Feature::GQ08:
      return "GQ08";
    case Feature::GQ09:
      return "GQ09";
    case Feature::GQ10:
      return "GQ10";
    case Feature::GQ11:
      return "GQ11";
    case Feature::GQ12:
      return "GQ12";
    case Feature::GQ13:
      return "GQ13";
    case Feature::GQ14:
      return "GQ14";
    case Feature::GQ15:
      return "GQ15";
    case Feature::GQ16:
      return "GQ16";
    case Feature::GQ17:
      return "GQ17";
    case Feature::GQ18:
      return "GQ18";
    case Feature::GQ19:
      return "GQ19";
    case Feature::GQ20:
      return "GQ20";
    case Feature::GQ21:
      return "GQ21";
    case Feature::GQ22:
      return "GQ22";
    case Feature::GQ23:
      return "GQ23";
    case Feature::GQ24:
      return "GQ24";
    case Feature::GS01:
      return "GS01";
    case Feature::GS02:
      return "GS02";
    case Feature::GS03:
      return "GS03";
    case Feature::GS04:
      return "GS04";
    case Feature::GS05:
      return "GS05";
    case Feature::GS06:
      return "GS06";
    case Feature::GS07:
      return "GS07";
    case Feature::GS08:
      return "GS08";
    case Feature::GS10:
      return "GS10";
    case Feature::GS11:
      return "GS11";
    case Feature::GS12:
      return "GS12";
    case Feature::GS13:
      return "GS13";
    case Feature::GS14:
      return "GS14";
    case Feature::GS15:
      return "GS15";
    case Feature::GS16:
      return "GS16";
    case Feature::GT01:
      return "GT01";
    case Feature::GT02:
      return "GT02";
    case Feature::GT03:
      return "GT03";
    case Feature::GV01:
      return "GV01";
    case Feature::GV02:
      return "GV02";
    case Feature::GV03:
      return "GV03";
    case Feature::GV04:
      return "GV04";
    case Feature::GV05:
      return "GV05";
    case Feature::GV06:
      return "GV06";
    case Feature::GV07:
      return "GV07";
    case Feature::GV08:
      return "GV08";
    case Feature::GV09:
      return "GV09";
    case Feature::GV10:
      return "GV10";
    case Feature::GV11:
      return "GV11";
    case Feature::GV12:
      return "GV12";
    case Feature::GV13:
      return "GV13";
    case Feature::GV14:
      return "GV14";
    case Feature::GV15:
      return "GV15";
    case Feature::GV16:
      return "GV16";
    case Feature::GV17:
      return "GV17";
    case Feature::GV18:
      return "GV18";
    case Feature::GV19:
      return "GV19";
    case Feature::GV20:
      return "GV20";
    case Feature::GV21:
      return "GV21";
    case Feature::GV22:
      return "GV22";
    case Feature::GV23:
      return "GV23";
    case Feature::GV24:
      return "GV24";
    case Feature::GV25:
      return "GV25";
    case Feature::GV26:
      return "GV26";
    case Feature::GV30:
      return "GV30";
    case Feature::GV31:
      return "GV31";
    case Feature::GV32:
      return "GV32";
    case Feature::GV35:
      return "GV35";
    case Feature::GV36:
      return "GV36";
    case Feature::GV37:
      return "GV37";
    case Feature::GV38:
      return "GV38";
    case Feature::GV39:
      return "GV39";
    case Feature::GV40:
      return "GV40";
    case Feature::GV41:
      return "GV41";
    case Feature::GV45:
      return "GV45";
    case Feature::GV46:
      return "GV46";
    case Feature::GV47:
      return "GV47";
    case Feature::GV48:
      return "GV48";
    case Feature::GV50:
      return "GV50";
    case Feature::GV55:
      return "GV55";
    case Feature::GV60:
      return "GV60";
    case Feature::GV61:
      return "GV61";
    case Feature::GV65:
      return "GV65";
    case Feature::GV66:
      return "GV66";
    case Feature::GV67:
      return "GV67";
    case Feature::GV68:
      return "GV68";
    case Feature::GV70:
      return "GV70";
    case Feature::GV71:
      return "GV71";
    case Feature::GV72:
      return "GV72";
    case Feature::GV90:
      return "GV90";
    default:
      assert(false);
      return "";
  }
}

}  // namespace gql::standard