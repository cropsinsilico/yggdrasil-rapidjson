// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#define RAPIDJSON_SCHEMA_VERBOSE 0
#define RAPIDJSON_HAS_STDSTRING 1

#include "unittest.h"
#include "rapidjson/schema.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(variadic-macros)
#elif defined(_MSC_VER)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4822) // local class member function does not have a body
#endif

using namespace rapidjson;

#define NORMALIZE(schema, json, expected, normalized)	\
{\
    SchemaNormalizer normalizer(schema);\
    Document d;\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    EXPECT_TRUE(expected == d.Accept(normalizer));	\
    EXPECT_TRUE(expected == normalizer.IsValid());\
    ValidateErrorCode code = normalizer.GetInvalidSchemaCode();\
    if (expected) {\
      EXPECT_TRUE(code == kValidateErrorNone);\
      EXPECT_TRUE(normalizer.GetInvalidSchemaKeyword() == 0);\
      EXPECT_TRUE(normalizer.WasNormalized());\
    }\
    if ((expected) && !normalizer.IsValid()) {\
        StringBuffer sb;\
        normalizer.GetInvalidSchemaPointer().StringifyUriFragment(sb);\
        printf("Invalid schema: %s\n", sb.GetString());\
        printf("Invalid keyword: %s\n", normalizer.GetInvalidSchemaKeyword());\
        printf("Invalid code: %d\n", code);\
        printf("Invalid message: %s\n", GetValidateError_En(code));\
        sb.Clear();\
        normalizer.GetInvalidDocumentPointer().StringifyUriFragment(sb);\
        printf("Invalid document: %s\n", sb.GetString());\
        sb.Clear();\
        Writer<StringBuffer> w(sb);\
        normalizer.GetError().Accept(w);\
        printf("Validation error: %s\n", sb.GetString());\
    }\
    Document n;\
    n.Parse(normalized);\
    EXPECT_FALSE(n.HasParseError());\
    if (normalizer.GetNormalized() != n) {\
        StringBuffer sb;\
        Writer<StringBuffer> w(sb);\
        normalizer.GetNormalized().Accept(w);\
        printf("GetNormalized() Expected: %s Actual: %s\n", normalized, sb.GetString());\
        ADD_FAILURE();\
    }\
}

#define FAILED_NORMALIZE(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error) \
{\
    FAILED_NORMALIZE_(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error, SchemaNormalizer, Pointer) \
}
#define FAILED_NORMALIZE_(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error, \
    SchemaNormalizerType, PointerType) \
{\
    SchemaNormalizerType normalizer(schema);\
    Document d;\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    d.Accept(normalizer);\
    EXPECT_FALSE(normalizer.IsValid());\
    ValidateErrorCode code = normalizer.GetInvalidSchemaCode();\
    ASSERT_TRUE(code != kValidateErrorNone);\
    ASSERT_TRUE(strcmp(GetValidateError_En(code), "Unknown error.") != 0);\
    if (normalizer.GetInvalidSchemaPointer() != PointerType(invalidSchemaPointer)) {\
        StringBuffer sb;\
        normalizer.GetInvalidSchemaPointer().Stringify(sb);\
        printf("GetInvalidSchemaPointer() Expected: %s Actual: %s\n", invalidSchemaPointer, sb.GetString());\
        ADD_FAILURE();\
    }\
    ASSERT_TRUE(normalizer.GetInvalidSchemaKeyword() != 0);\
    if (strcmp(normalizer.GetInvalidSchemaKeyword(), invalidSchemaKeyword) != 0) {\
        printf("GetInvalidSchemaKeyword() Expected: %s Actual %s\n", invalidSchemaKeyword, normalizer.GetInvalidSchemaKeyword());\
        ADD_FAILURE();\
    }\
    if (normalizer.GetInvalidDocumentPointer() != PointerType(invalidDocumentPointer)) {\
        StringBuffer sb;\
        normalizer.GetInvalidDocumentPointer().Stringify(sb);\
        printf("GetInvalidDocumentPointer() Expected: %s Actual: %s\n", invalidDocumentPointer, sb.GetString());\
        ADD_FAILURE();\
    }\
    Document e;\
    e.Parse(error);\
    if (normalizer.GetError() != e) {\
        StringBuffer sb;\
        Writer<StringBuffer> w(sb);\
        normalizer.GetError().Accept(w);\
        printf("GetError() Expected: %s Actual: %s\n", error, sb.GetString());\
        ADD_FAILURE();\
    }\
}

TEST(SchemaNormalizer, Default) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\","
	"                              \"default\": \"default_address\"},"
        "        \"city\":           { \"type\": \"string\","
	"                              \"default\": \"default_city\"},"
        "        \"state\":          { \"type\": \"string\","
	"                              \"default\": \"default_state\"},"
        "        \"type\":           { \"enum\": [ \"residential\", \"business\" ],"
	"                              \"default\": \"residential\" }"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\", \"type\"]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);
    NORMALIZE(s,
	      "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\"} }",
	      true,
	      "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }");
}

TEST(SchemaNormalizer, DefaultNested) {
    Document sd;
    sd.Parse(
        "{"
        "  \"$schema\": \"http://json-schema.org/draft-04/schema#\","
        ""
        "  \"definitions\": {"
        "    \"address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\","
	"                              \"default\": \"default_address\"},"
        "        \"city\":           { \"type\": \"string\","
	"                              \"default\": \"default_city\"},"
        "        \"state\":          { \"type\": \"string\","
	"                              \"default\": \"default_state\"}"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\"]"
        "    }"
        "  },"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"allOf\": ["
        "        { \"$ref\": \"#/definitions/address\" },"
        "        { \"properties\":"
        "          { \"type\": { \"enum\": [ \"residential\", \"business\" ],"
	"                        \"default\": \"residential\" } },"
        "          \"required\": [\"type\"]"
        "        }"
        "      ]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);
    NORMALIZE(s,
	      "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\"} }",
	      true,
	      "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"default_state\", \"type\": \"residential\"} }");
}

TEST(SchemaNormalizer, Alias) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\","
	"                              \"default\": \"default_address\","
	"                              \"aliases\": [\"street\"]},"
        "        \"city\":           { \"type\": \"string\","
	"                              \"default\": \"default_city\"},"
        "        \"state\":          { \"type\": \"string\","
	"                              \"default\": \"default_state\"},"
        "        \"type\":           { \"enum\": [ \"residential\", \"business\" ],"
	"                              \"default\": \"residential\" }"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\", \"type\"]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);
    NORMALIZE(s,
	      "{\"shipping_address\": {\"street\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }",
	      true,
	      "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }");
    FAILED_NORMALIZE(s,
		     "{\"shipping_address\": {\"street\": \"1600 Pennsylvania Avenue NW\", \"street_address\": \"1700 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }",
		     "/properties/shipping_address", "aliases",
		     "/shipping_address/street_address",
		     "{ \"aliases\": {"
		     "    \"errorCode\": 33,"
		     "    \"instanceRef\": \"#/shipping_address\","
		     "    \"schemaRef\": \"#/properties/shipping_address\","
		     "    \"duplicates\": [\"street\", \"street_address\"]"
		     "}}");
}

TEST(SchemaNormalizer, SingularAlias) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
	"  \"properties\": {"
	"     \"streets\": { \"type\": \"array\","
	"                    \"items\": {\"type\": \"string\"},"
	"                    \"allowSingular\": true,"
	"                    \"aliases\": [\"street\"] }},"
	"  \"required\": [\"streets\"]"
        "}");
  SchemaDocument s(sd);
  NORMALIZE(s, "{ \"streets\": \"1600 Pennsylvania Ave.\" }", true,
	    "{ \"streets\": [\"1600 Pennsylvania Ave.\"] }");
  NORMALIZE(s, "{ \"street\": \"1600 Pennsylvania Ave.\" }", true,
	    "{ \"streets\": [\"1600 Pennsylvania Ave.\"] }");
}

TEST(SchemaNormalizer, AliasCircular) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\","
	"                              \"default\": \"default_address\","
	"                              \"aliases\": [\"street\"]},"
        "        \"street\":         { \"type\": \"string\","
	"                              \"default\": \"default_address\","
	"                              \"aliases\": [\"street_address\"]},"
        "        \"city\":           { \"type\": \"string\","
	"                              \"default\": \"default_city\"},"
        "        \"state\":          { \"type\": \"string\","
	"                              \"default\": \"default_state\"},"
        "        \"type\":           { \"enum\": [ \"residential\", \"business\" ],"
	"                              \"default\": \"residential\" }"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\", \"type\"]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);
    FAILED_NORMALIZE(s,
		     "{\"shipping_address\": {\"street\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }",
		     "/properties/shipping_address", "aliases",
		     "/shipping_address/street",
		     "{ \"aliases\": {"
		     "    \"errorCode\": 34,"
		     "    \"instanceRef\": \"#/shipping_address\","
		     "    \"schemaRef\": \"#/properties/shipping_address\","
		     "    \"circular\": [\"street\", \"street_address\"]"
		     "}}");
}

TEST(SchemaNormalizer, AliasConflicting) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\","
	"                              \"default\": \"default_address\","
	"                              \"aliases\": [\"street\"]},"
        "        \"address\":        { \"type\": \"string\","
	"                              \"default\": \"default_address\","
	"                              \"aliases\": [\"street\"]},"
        "        \"city\":           { \"type\": \"string\","
	"                              \"default\": \"default_city\"},"
        "        \"state\":          { \"type\": \"string\","
	"                              \"default\": \"default_state\"},"
        "        \"type\":           { \"enum\": [ \"residential\", \"business\" ],"
	"                              \"default\": \"residential\" }"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\", \"type\"]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);
    FAILED_NORMALIZE(s,
		     "{\"shipping_address\": {\"street\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }",
		     "/properties/shipping_address", "aliases",
		     "/shipping_address/street",
		     "{ \"aliases\": {"
		     "    \"errorCode\": 35,"
		     "    \"instanceRef\": \"#/shipping_address\","
		     "    \"schemaRef\": \"#/properties/shipping_address\","
		     "    \"conflicting\": \"street\","
		     "    \"expected\": \"street_address\","
		     "    \"actual\": \"address\""
		     "}}");
}

TEST(SchemaNormalizer, AliasNested) {
    Document sd;
    sd.Parse(
        "{"
        "  \"$schema\": \"http://json-schema.org/draft-04/schema#\","
        ""
        "  \"definitions\": {"
        "    \"address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\","
	"                              \"default\": \"default_address\","
	"                              \"aliases\": [\"street\"]},"
        "        \"city\":           { \"type\": \"string\","
	"                              \"default\": \"default_city\"},"
        "        \"state\":          { \"type\": \"string\","
	"                              \"default\": \"default_state\"}"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\"]"
        "    }"
        "  },"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"allOf\": ["
        "        { \"$ref\": \"#/definitions/address\" },"
        "        { \"properties\":"
        "          { \"type\": { \"enum\": [ \"residential\", \"business\" ],"
	"                        \"default\": \"residential\" } },"
        "          \"required\": [\"type\"]"
        "        }"
        "      ]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);
    NORMALIZE(s,
	      "{\"shipping_address\": {\"street\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }",
	      true,
	      "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }");
    FAILED_NORMALIZE(s,
		     "{\"shipping_address\": {\"street\": \"1600 Pennsylvania Avenue NW\", \"street_address\": \"1700 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }",
		     "/properties/shipping_address", "allOf",
		     "/shipping_address",
		     "{ \"allOf\": {"
		     "    \"errors\": ["
		     "       { \"aliases\": {"
		     "           \"errorCode\": 33,"
		     "           \"instanceRef\": \"#/shipping_address\","
		     "           \"schemaRef\": \"#/definitions/address\","
		     "           \"duplicates\": [\"street\", \"street_address\"]"
		     "       }},"
		     "       {}],"
		     "    \"errorCode\": 23,"
		     "    \"instanceRef\": \"#/shipping_address\","
		     "    \"schemaRef\": \"#/properties/shipping_address\""
		     "}}");
}

TEST(SchemaNormalizer, AliasNestedCircular) {
    Document sd;
    sd.Parse(
        "{"
        "  \"$schema\": \"http://json-schema.org/draft-04/schema#\","
        ""
        "  \"definitions\": {"
        "    \"address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\","
	"                              \"default\": \"default_address\","
	"                              \"aliases\": [\"street\"]},"
        "        \"city\":           { \"type\": \"string\","
	"                              \"default\": \"default_city\"},"
        "        \"state\":          { \"type\": \"string\","
	"                              \"default\": \"default_state\"}"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\"]"
        "    }"
        "  },"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"allOf\": ["
        "        { \"$ref\": \"#/definitions/address\" },"
        "        { \"properties\":"
        "          { \"type\":   { \"enum\": [ \"residential\", \"business\" ],"
	"                          \"default\": \"residential\" },"
        "            \"street\": { \"type\": \"string\","
	"                          \"default\": \"default_address\","
	"                          \"aliases\": [\"street_address\"] }},"
        "          \"required\": [\"type\"]"
        "        }"
        "      ]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);
    FAILED_NORMALIZE(s,
		     "{\"shipping_address\": {\"street\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }",
		     "/properties/shipping_address", "aliases",
		     "/shipping_address",
		     "{ \"aliases\": {"
		     "    \"errorCode\": 34,"
		     "    \"instanceRef\": \"#\","
		     "    \"schemaRef\": \"#/properties/shipping_address\","
		     "    \"circular\": [\"street\", \"street_address\"]"
		     "}}");
}

TEST(SchemaNormalizer, AliasNestedConflicting) {
    Document sd;
    sd.Parse(
        "{"
        "  \"$schema\": \"http://json-schema.org/draft-04/schema#\","
        ""
        "  \"definitions\": {"
        "    \"address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\","
	"                              \"default\": \"default_address\","
	"                              \"aliases\": [\"street\"]},"
        "        \"city\":           { \"type\": \"string\","
	"                              \"default\": \"default_city\"},"
        "        \"state\":          { \"type\": \"string\","
	"                              \"default\": \"default_state\"}"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\"]"
        "    }"
        "  },"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"allOf\": ["
        "        { \"$ref\": \"#/definitions/address\" },"
        "        { \"properties\":"
        "          { \"type\":    { \"enum\": [ \"residential\", \"business\" ],"
	"                           \"default\": \"residential\" },"
        "            \"address\": { \"type\": \"string\","
	"                           \"default\": \"default_address\","
	"                           \"aliases\": [\"street\"] }},"
        "          \"required\": [\"type\"]"
        "        }"
        "      ]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);
    FAILED_NORMALIZE(s,
		     "{\"shipping_address\": {\"street\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"residential\"} }",
		     "/properties/shipping_address", "aliases",
		     "/shipping_address",
		     "{ \"aliases\": {"
		     "    \"errorCode\": 35,"
		     "    \"instanceRef\": \"#\","
		     "    \"schemaRef\": \"#/properties/shipping_address\","
		     "  \"conflicting\": \"street\","
		     "  \"expected\": \"address\","
		     "  \"actual\": \"street_address\""
		     "}}");
}

#if defined(_MSC_VER) || defined(__clang__)
RAPIDJSON_DIAG_POP
#endif
