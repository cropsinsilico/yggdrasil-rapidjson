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
#include "rapidjson/prettywriter.h"
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

#define TEST_HASHER(json1, json2, expected) \
{\
    Document d1, d2;\
    d1.Parse(json1);\
    ASSERT_FALSE(d1.HasParseError());\
    d2.Parse(json2);\
    ASSERT_FALSE(d2.HasParseError());\
    internal::Hasher<Value, CrtAllocator> h1, h2;\
    d1.Accept(h1);\
    d2.Accept(h2);\
    ASSERT_TRUE(h1.IsValid());\
    ASSERT_TRUE(h2.IsValid());\
    /*printf("%s: 0x%016llx\n%s: 0x%016llx\n\n", json1, h1.GetHashCode(), json2, h2.GetHashCode());*/\
    EXPECT_TRUE(expected == (h1.GetHashCode() == h2.GetHashCode()));\
}

TEST(SchemaValidator, Hasher) {
    TEST_HASHER("null", "null", true);

    TEST_HASHER("true", "true", true);
    TEST_HASHER("false", "false", true);
    TEST_HASHER("true", "false", false);
    TEST_HASHER("false", "true", false);
    TEST_HASHER("true", "null", false);
    TEST_HASHER("false", "null", false);

    TEST_HASHER("1", "1", true);
    TEST_HASHER("2147483648", "2147483648", true); // 2^31 can only be fit in unsigned
    TEST_HASHER("-2147483649", "-2147483649", true); // -2^31 - 1 can only be fit in int64_t
    TEST_HASHER("2147483648", "2147483648", true); // 2^31 can only be fit in unsigned
    TEST_HASHER("4294967296", "4294967296", true); // 2^32 can only be fit in int64_t
    TEST_HASHER("9223372036854775808", "9223372036854775808", true); // 2^63 can only be fit in uint64_t
    TEST_HASHER("1.5", "1.5", true);
    TEST_HASHER("1", "1.0", true);
    TEST_HASHER("1", "-1", false);
    TEST_HASHER("0.0", "-0.0", false);
    TEST_HASHER("1", "true", false);
    TEST_HASHER("0", "false", false);
    TEST_HASHER("0", "null", false);

    TEST_HASHER("\"\"", "\"\"", true);
    TEST_HASHER("\"\"", "\"\\u0000\"", false);
    TEST_HASHER("\"Hello\"", "\"Hello\"", true);
    TEST_HASHER("\"Hello\"", "\"World\"", false);
    TEST_HASHER("\"Hello\"", "null", false);
    TEST_HASHER("\"Hello\\u0000\"", "\"Hello\"", false);
    TEST_HASHER("\"\"", "null", false);
    TEST_HASHER("\"\"", "true", false);
    TEST_HASHER("\"\"", "false", false);

    TEST_HASHER("[]", "[ ]", true);
    TEST_HASHER("[1, true, false]", "[1, true, false]", true);
    TEST_HASHER("[1, true, false]", "[1, true]", false);
    TEST_HASHER("[1, 2]", "[2, 1]", false);
    TEST_HASHER("[[1], 2]", "[[1, 2]]", false);
    TEST_HASHER("[1, 2]", "[1, [2]]", false);
    TEST_HASHER("[]", "null", false);
    TEST_HASHER("[]", "true", false);
    TEST_HASHER("[]", "false", false);
    TEST_HASHER("[]", "0", false);
    TEST_HASHER("[]", "0.0", false);
    TEST_HASHER("[]", "\"\"", false);

    TEST_HASHER("{}", "{ }", true);
    TEST_HASHER("{\"a\":1}", "{\"a\":1}", true);
    TEST_HASHER("{\"a\":1}", "{\"b\":1}", false);
    TEST_HASHER("{\"a\":1}", "{\"a\":2}", false);
    TEST_HASHER("{\"a\":\"a\"}", "{\"b\":\"b\"}", false); // Key equals value hashing
    TEST_HASHER("{\"a\":\"a\", \"b\":\"b\"}", "{\"c\":\"c\", \"d\":\"d\"}", false);
    TEST_HASHER("{\"a\":\"a\"}", "{\"b\":\"b\", \"c\":\"c\"}", false);
    TEST_HASHER("{\"a\":1, \"b\":2}", "{\"b\":2, \"a\":1}", true); // Member order insensitive
    TEST_HASHER("{}", "null", false);
    TEST_HASHER("{}", "false", false);
    TEST_HASHER("{}", "true", false);
    TEST_HASHER("{}", "0", false);
    TEST_HASHER("{}", "0.0", false);
    TEST_HASHER("{}", "\"\"", false);
}

// Test cases following http://spacetelescope.github.io/understanding-json-schema

#define VALIDATE(schema, json, expected) \
{\
    VALIDATE_(schema, json, expected, true) \
}

#define VALIDATE_(schema, json, expected, expected2) \
{\
    EXPECT_TRUE(expected2 == schema.GetError().ObjectEmpty());\
    EXPECT_TRUE(schema.IsSupportedSpecification());\
    SchemaValidator validator(schema);\
    Document d;\
    /*printf("\n%s\n", json);*/\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    EXPECT_TRUE(expected == d.Accept(validator));\
    EXPECT_TRUE(expected == validator.IsValid());\
    ValidateErrorCode code = validator.GetInvalidSchemaCode();\
    if (expected) {\
      EXPECT_TRUE(code == kValidateErrorNone);\
      EXPECT_TRUE(validator.GetInvalidSchemaKeyword() == 0);\
    }\
    if ((expected) && !validator.IsValid()) {\
        StringBuffer sb;\
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);\
        printf("Invalid schema: %s\n", sb.GetString());\
        printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());\
        printf("Invalid code: %d\n", code);\
        printf("Invalid message: %s\n", GetValidateError_En(code));\
        sb.Clear();\
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);\
        printf("Invalid document: %s\n", sb.GetString());\
        sb.Clear();\
        PrettyWriter<StringBuffer> w(sb);\
        validator.GetError().Accept(w);\
        printf("Validation error: %s\n", sb.GetString());\
    }\
}

#ifdef RAPIDJSON_YGGDRASIL
#define SHOW_ERROR_MESSAGE_(name, instance)                             \
  RAPIDJSON_DEFAULT_ALLOCATOR error_msg_allocator;                      \
  Value e_msg;                                                          \
  if (!instance.Get ## name ## Msg(e_msg, error_msg_allocator)) {	\
    StringBuffer sb_t;                                                  \
    PrettyWriter<StringBuffer> w_t(sb_t);                               \
    printf("%sMsg = %s\n", #name, sb_t.GetString());                    \
    StringBuffer sb;                                                    \
    PrettyWriter<StringBuffer> w(sb);                                   \
    validator.GetError().Accept(w);                                     \
    printf("Get%s(): %s", #name, sb.GetString());                       \
    ADD_FAILURE();                                                      \
  }
#define SHOW_ERROR_MESSAGE(instance)            \
  SHOW_ERROR_MESSAGE_(Error, instance)
#define SHOW_WARNING_MESSAGE(instance)          \
  SHOW_ERROR_MESSAGE_(Warning, instance)
#else
#define SHOW_ERROR_MESSAGE(instance)
#define SHOW_WARNING_MESSAGE(instance)
#endif

#define INVALIDATE(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error) \
{\
    INVALIDATE_(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error, kValidateDefaultFlags, SchemaValidator, Pointer) \
}

#define INVALIDATE_(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error, \
    flags, SchemaValidatorType, PointerType) \
{\
    EXPECT_TRUE(schema.GetError().ObjectEmpty());\
    EXPECT_TRUE(schema.IsSupportedSpecification());\
    SchemaValidatorType validator(schema);\
    validator.SetValidateFlags(flags);\
    Document d;\
    /*printf("\n%s\n", json);*/\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    d.Accept(validator);\
    EXPECT_FALSE(validator.IsValid());\
    ValidateErrorCode code = validator.GetInvalidSchemaCode();\
    ASSERT_TRUE(code != kValidateErrorNone);\
    ASSERT_TRUE(strcmp(GetValidateError_En(code), "Unknown error.") != 0);\
    if (validator.GetInvalidSchemaPointer() != PointerType(invalidSchemaPointer)) {\
        StringBuffer sb;\
        validator.GetInvalidSchemaPointer().Stringify(sb);\
        printf("GetInvalidSchemaPointer() Expected: %s Actual: %s\n", invalidSchemaPointer, sb.GetString());\
        ADD_FAILURE();\
    }\
    ASSERT_TRUE(validator.GetInvalidSchemaKeyword() != 0);\
    if (strcmp(validator.GetInvalidSchemaKeyword(), invalidSchemaKeyword) != 0) {\
        printf("GetInvalidSchemaKeyword() Expected: %s Actual %s\n", invalidSchemaKeyword, validator.GetInvalidSchemaKeyword());\
        ADD_FAILURE();\
    }\
    if (validator.GetInvalidDocumentPointer() != PointerType(invalidDocumentPointer)) {\
        StringBuffer sb;\
        validator.GetInvalidDocumentPointer().Stringify(sb);\
        printf("GetInvalidDocumentPointer() Expected: %s Actual: %s\n", invalidDocumentPointer, sb.GetString());\
        ADD_FAILURE();\
    }\
    Document e;\
    e.Parse(error);\
    SHOW_ERROR_MESSAGE(validator)\
    if (validator.GetError() != e) {\
        StringBuffer sb;\
        PrettyWriter<StringBuffer> w(sb);\
        validator.GetError().Accept(w);\
        StringBuffer sb_e;\
        PrettyWriter<StringBuffer> w_e(sb_e);\
	e.Accept(w_e);\
        printf("GetError() Expected: %s Actual: %s\n", sb_e.GetString(), sb.GetString()); \
        ADD_FAILURE();\
    }\
}

#define VALIDATE_WARNING(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, warning) \
{\
    VALIDATE_WARNING_(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, warning, kValidateDefaultFlags, SchemaValidator, Pointer) \
}

#define VALIDATE_WARNING_(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, warning, \
    flags, SchemaValidatorType, PointerType) \
{\
    SchemaValidatorType validator(schema);\
    validator.SetValidateFlags(flags);\
    Document d;\
    /*printf("\n%s\n", json);*/\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    d.Accept(validator);\
    EXPECT_TRUE(validator.IsValid());\
    ValidateErrorCode code = validator.GetInvalidSchemaCode();\
    ASSERT_TRUE(code == kValidateWarnings);\
    ASSERT_TRUE(strcmp(GetValidateError_En(code), "Unknown error.") != 0);\
    if (validator.GetInvalidSchemaPointer() != PointerType(invalidSchemaPointer)) {\
        StringBuffer sb;\
        validator.GetInvalidSchemaPointer().Stringify(sb);\
        printf("GetInvalidSchemaPointer() Expected: %s Actual: %s\n", invalidSchemaPointer, sb.GetString());\
        ADD_FAILURE();\
    }\
    ASSERT_TRUE(validator.GetInvalidSchemaKeyword() != 0);\
    if (strcmp(validator.GetInvalidSchemaKeyword(), invalidSchemaKeyword) != 0) {\
        printf("GetInvalidSchemaKeyword() Expected: %s Actual %s\n", invalidSchemaKeyword, validator.GetInvalidSchemaKeyword());\
        ADD_FAILURE();\
    }\
    if (validator.GetInvalidDocumentPointer() != PointerType(invalidDocumentPointer)) {\
        StringBuffer sb;\
        validator.GetInvalidDocumentPointer().Stringify(sb);\
        printf("GetInvalidDocumentPointer() Expected: %s Actual: %s\n", invalidDocumentPointer, sb.GetString());\
        ADD_FAILURE();\
    }\
    Document e;\
    e.Parse(warning);\
    SHOW_WARNING_MESSAGE(validator)\
    if (validator.GetWarning() != e) {\
        StringBuffer sb;\
        PrettyWriter<StringBuffer> w(sb);		\
        validator.GetWarning().Accept(w);\
	StringBuffer sb_e;\
	PrettyWriter<StringBuffer> w_e(sb_e);\
	e.Accept(w_e);\
        printf("GetWarning() Expected: %s Actual: %s\n", sb_e.GetString(), sb.GetString()); \
        ADD_FAILURE();\
    }\
}

// Use for checking whether a compiled schema document contains errors
#define SCHEMAERROR(schema, error) \
{\
    Document e;\
    e.Parse(error);\
    if (schema.GetError() != e) {\
        StringBuffer sb;\
        Writer<StringBuffer> w(sb);\
        schema.GetError().Accept(w);\
        printf("GetError() Expected: %s Actual: %s\n", error, sb.GetString());\
        ADD_FAILURE();\
    }\
}

TEST(SchemaValidator, Typeless) {
    Document sd;
    sd.Parse("{}");
    SchemaDocument s(sd);
    
    VALIDATE(s, "42", true);
    VALIDATE(s, "\"I'm a string\"", true);
    VALIDATE(s, "{ \"an\": [ \"arbitrarily\", \"nested\" ], \"data\": \"structure\" }", true);
}

TEST(SchemaValidator, MultiType) {
    Document sd;
    sd.Parse("{ \"type\": [\"number\", \"string\"] }");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "\"Life, the universe, and everything\"", true);
    INVALIDATE(s, "[\"Life\", \"the universe\", \"and everything\"]", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\", \"number\"], \"actual\": \"array\""
        "}}");
}

TEST(SchemaValidator, Enum_Typed) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\" : [\"red\", \"amber\", \"green\"] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"blue\"", "", "enum", "",
        "{ \"enum\": { \"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#\", \"expected\": [\"red\", \"amber\", \"green\"] }}");
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"blue\"", "", "enum", "",
        "{ \"enum\": { \"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#\" }}");
#endif // RAPIDJSON_YGGDRASIL
}

TEST(SchemaValidator, Enum_Typeless) {
    Document sd;
    sd.Parse("{  \"enum\": [\"red\", \"amber\", \"green\", null, 42] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
    VALIDATE(s, "null", true);
    VALIDATE(s, "42", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "0", "", "enum", "",
        "{ \"enum\": { \"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#\", \"expected\": [\"red\", \"amber\", \"green\", null, 42] }}");
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "0", "", "enum", "",
        "{ \"enum\": { \"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#\" }}");
#endif // RAPIDJSON_YGGDRASIL
}

TEST(SchemaValidator, Enum_InvalidType) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\": [\"red\", \"amber\", \"green\", null] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
    INVALIDATE(s, "null", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"null\""
        "}}");
}

TEST(SchemaValidator, AllOf) {
    {
        Document sd;
        sd.Parse("{\"allOf\": [{ \"type\": \"string\" }, { \"type\": \"string\", \"maxLength\": 5 }]}");
        SchemaDocument s(sd);

        VALIDATE(s, "\"ok\"", true);
        INVALIDATE(s, "\"too long\"", "", "allOf", "",
            "{ \"allOf\": {"
            "    \"errors\": ["
            "      {},"
            "      {\"maxLength\": {\"errorCode\": 6, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/1\", \"expected\": 5, \"actual\": \"too long\"}}"
            "    ],"
            "    \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
            "}}");
    }
    {
        Document sd;
        sd.Parse("{\"allOf\": [{ \"type\": \"string\" }, { \"type\": \"number\" } ] }");
        SchemaDocument s(sd);

        VALIDATE(s, "\"No way\"", false);
        INVALIDATE(s, "-1", "", "allOf", "",
            "{ \"allOf\": {"
            "    \"errors\": ["
            "      {\"type\": { \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\", \"errorCode\": 20, \"expected\": [\"string\"], \"actual\": \"integer\"}},"
            "      {}"
            "    ],"
            "    \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
            "}}");
    }
}

TEST(SchemaValidator, AnyOf) {
    Document sd;
    sd.Parse("{\"anyOf\": [{ \"type\": \"string\" }, { \"type\": \"number\" } ] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"Yes\"", true);
    VALIDATE(s, "42", true);
    INVALIDATE(s, "{ \"Not a\": \"string or number\" }", "", "anyOf", "",
        "{ \"anyOf\": {"
        "    \"errorCode\": 24,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\", "
        "    \"errors\": ["
        "      { \"type\": {"
        "          \"errorCode\": 20,"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/anyOf/0\","
        "          \"expected\": [\"string\"], \"actual\": \"object\""
        "      }},"
        "      { \"type\": {"
        "          \"errorCode\": 20,"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/anyOf/1\","
        "          \"expected\": [\"number\"], \"actual\": \"object\""
        "      }}"
        "    ]"
        "}}");
}

TEST(SchemaValidator, OneOf) {
    Document sd;
    sd.Parse("{\"oneOf\": [{ \"type\": \"number\", \"multipleOf\": 5 }, { \"type\": \"number\", \"multipleOf\": 3 } ] }");
    SchemaDocument s(sd);

    VALIDATE(s, "10", true);
    VALIDATE(s, "9", true);
    INVALIDATE(s, "2", "", "oneOf", "",
        "{ \"oneOf\": {"
        "    \"errorCode\": 21,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"errors\": ["
        "      { \"multipleOf\": {"
        "          \"errorCode\": 1,"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/oneOf/0\","
        "          \"expected\": 5, \"actual\": 2"
        "      }},"
        "      { \"multipleOf\": {"
        "          \"errorCode\": 1,"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/oneOf/1\","
        "          \"expected\": 3, \"actual\": 2"
        "      }}"
        "    ]"
        "}}");
    INVALIDATE(s, "15", "", "oneOf", "",
        "{ \"oneOf\": { \"errorCode\": 22, \"instanceRef\": \"#\", \"schemaRef\": \"#\", \"matches\": [0,1]}}");
}

TEST(SchemaValidator, Not) {
    Document sd;
    sd.Parse("{\"not\":{ \"type\": \"string\"}}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "{ \"key\": \"value\" }", true);
    INVALIDATE(s, "\"I am a string\"", "", "not", "",
        "{ \"not\": { \"errorCode\": 25, \"instanceRef\": \"#\", \"schemaRef\": \"#\" }}");
}

TEST(SchemaValidator, Ref) {
    Document sd;
    sd.Parse(
        "{"
        "  \"$schema\": \"http://json-schema.org/draft-04/schema#\","
        ""
        "  \"definitions\": {"
        "    \"address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\" },"
        "        \"city\":           { \"type\": \"string\" },"
        "        \"state\":          { \"type\": \"string\" }"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\"]"
        "    }"
        "  },"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": { \"$ref\": \"#/definitions/address\" }"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\"}, \"billing_address\": {\"street_address\": \"1st Street SE\", \"city\": \"Washington\", \"state\": \"DC\"} }", true);
}

TEST(SchemaValidator, Ref_AllOf) {
    Document sd;
    sd.Parse(
        "{"
        "  \"$schema\": \"http://json-schema.org/draft-04/schema#\","
        ""
        "  \"definitions\": {"
        "    \"address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\" },"
        "        \"city\":           { \"type\": \"string\" },"
        "        \"state\":          { \"type\": \"string\" }"
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
        "          { \"type\": { \"enum\": [ \"residential\", \"business\" ] } },"
        "          \"required\": [\"type\"]"
        "        }"
        "      ]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);

    INVALIDATE(s, "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\"} }", "/properties/shipping_address", "allOf", "/shipping_address",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "      {},"
        "      {\"required\": {\"errorCode\": 15, \"instanceRef\": \"#/shipping_address\", \"schemaRef\": \"#/properties/shipping_address/allOf/1\", \"missing\": [\"type\"]}}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#/shipping_address\",\"schemaRef\":\"#/properties/shipping_address\""
        "}}");
    VALIDATE(s, "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"business\"} }", true);
}

TEST(SchemaValidator, String) {
    Document sd;
    sd.Parse("{\"type\":\"string\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"I'm a string\"", true);
    INVALIDATE(s, "42", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
    INVALIDATE(s, "2147483648", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}"); // 2^31 can only be fit in unsigned
    INVALIDATE(s, "-2147483649", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}"); // -2^31 - 1 can only be fit in int64_t
    INVALIDATE(s, "4294967296", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}"); // 2^32 can only be fit in int64_t
    INVALIDATE(s, "3.1415926", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"number\""
        "}}");
}

TEST(SchemaValidator, String_LengthRange) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"minLength\":2,\"maxLength\":3}");
    SchemaDocument s(sd);

    INVALIDATE(s, "\"A\"", "", "minLength", "",
        "{ \"minLength\": {"
        "    \"errorCode\": 7,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": \"A\""
        "}}");
    VALIDATE(s, "\"AB\"", true);
    VALIDATE(s, "\"ABC\"", true);
    INVALIDATE(s, "\"ABCD\"", "", "maxLength", "",
        "{ \"maxLength\": {"
        "    \"errorCode\": 6,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 3, \"actual\": \"ABCD\""
        "}}");
}

#if RAPIDJSON_SCHEMA_HAS_REGEX
TEST(SchemaValidator, String_Pattern) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"pattern\":\"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"555-1212\"", true);
    VALIDATE(s, "\"(888)555-1212\"", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"(888)555-1212 ext. 532\"", "", "pattern", "",
        "{ \"pattern\": {"
        "    \"errorCode\": 8,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"actual\": \"(888)555-1212 ext. 532\","
	"    \"expected\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
        "}}");
    INVALIDATE(s, "\"(800)FLOWERS\"", "", "pattern", "",
        "{ \"pattern\": {"
        "    \"errorCode\": 8,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"actual\": \"(800)FLOWERS\","
	"    \"expected\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
        "}}");
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"(888)555-1212 ext. 532\"", "", "pattern", "",
        "{ \"pattern\": {"
        "    \"errorCode\": 8,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"actual\": \"(888)555-1212 ext. 532\""
        "}}");
    INVALIDATE(s, "\"(800)FLOWERS\"", "", "pattern", "",
        "{ \"pattern\": {"
        "    \"errorCode\": 8,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"actual\": \"(800)FLOWERS\""
        "}}");
#endif // RAPIDJSON_YGGDRASIL
}

TEST(SchemaValidator, String_Pattern_Invalid) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"pattern\":\"a{0}\"}");
    SchemaDocument s(sd);
    SCHEMAERROR(s, "{\"RegexInvalid\":{\"errorCode\":9,\"instanceRef\":\"#/pattern\",\"value\":\"a{0}\"}}");

    VALIDATE_(s, "\"\"", true, false);
    VALIDATE_(s, "\"a\"", true, false);
    VALIDATE_(s, "\"aa\"", true, false);
}
#endif

TEST(SchemaValidator, Integer) {
    Document sd;
    sd.Parse("{\"type\":\"integer\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "-1", true);
    VALIDATE(s, "2147483648", true); // 2^31 can only be fit in unsigned
    VALIDATE(s, "-2147483649", true); // -2^31 - 1 can only be fit in int64_t
    VALIDATE(s, "2147483648", true); // 2^31 can only be fit in unsigned
    VALIDATE(s, "4294967296", true); // 2^32 can only be fit in int64_t
    INVALIDATE(s, "3.1415926", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"integer\"], \"actual\": \"number\""
        "}}");
    INVALIDATE(s, "\"42\"", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"integer\"], \"actual\": \"string\""
        "}}");
}

TEST(SchemaValidator, Integer_Range) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":0,\"maximum\":100,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-1", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0, \"actual\": -1"
        "}}");
    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "99", true);
    INVALIDATE(s, "100", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 100"
        "}}");
    INVALIDATE(s, "101", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 101"
        "}}");
}

TEST(SchemaValidator, Integer_Range64Boundary) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":-9223372036854775807,\"maximum\":9223372036854775806}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -9223372036854775807, \"actual\": -9223372036854775808"
        "}}");
    VALIDATE(s, "-9223372036854775807", true);
    VALIDATE(s, "-2147483648", true); // int min
    VALIDATE(s, "0", true);
    VALIDATE(s, "2147483647", true);  // int max
    VALIDATE(s, "2147483648", true);  // unsigned first
    VALIDATE(s, "4294967295", true);  // unsigned max
    VALIDATE(s, "9223372036854775806", true);
    INVALIDATE(s, "9223372036854775807", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 2,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775806, \"actual\": 9223372036854775807"
        "}}");
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 2,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775806, \"actual\": 18446744073709551615"
        "}}");   // uint64_t max
}

TEST(SchemaValidator, Integer_RangeU64Boundary) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":9223372036854775808,\"maximum\":18446744073709551614}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": -9223372036854775808"
        "}}");
    INVALIDATE(s, "9223372036854775807", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 9223372036854775807"
        "}}");
    INVALIDATE(s, "-2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": -2147483648"
        "}}"); // int min
    INVALIDATE(s, "0", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 0"
        "}}");
    INVALIDATE(s, "2147483647", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 2147483648"
        "}}");  // unsigned first
    INVALIDATE(s, "4294967295", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 4294967295"
        "}}");  // unsigned max
    VALIDATE(s, "9223372036854775808", true);
    VALIDATE(s, "18446744073709551614", true);
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 2,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 18446744073709551614, \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Integer_Range64BoundaryExclusive) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":-9223372036854775808,\"maximum\":18446744073709551615,\"exclusiveMinimum\":true,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 5,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -9223372036854775808, \"exclusiveMinimum\": true, "
        "    \"actual\": -9223372036854775808"
        "}}");
    VALIDATE(s, "-9223372036854775807", true);
    VALIDATE(s, "18446744073709551614", true);
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 18446744073709551615, \"exclusiveMaximum\": true, "
        "    \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Integer_MultipleOf) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"multipleOf\":10}");
    SchemaDocument s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "-10", true);
    VALIDATE(s, "20", true);
    INVALIDATE(s, "23", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10, \"actual\": 23"
        "}}");
    INVALIDATE(s, "-23", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10, \"actual\": -23"
        "}}");
}

TEST(SchemaValidator, Integer_MultipleOf64Boundary) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"multipleOf\":18446744073709551615}");
    SchemaDocument s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "18446744073709551615", true);
    INVALIDATE(s, "18446744073709551614", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 18446744073709551615, \"actual\": 18446744073709551614"
        "}}");
}

TEST(SchemaValidator, Number_Range) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":0,\"maximum\":100,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-1", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0, \"actual\": -1"
        "}}");
    VALIDATE(s, "0", true);
    VALIDATE(s, "0.1", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "99", true);
    VALIDATE(s, "99.9", true);
    INVALIDATE(s, "100", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 100"
        "}}");
    INVALIDATE(s, "100.0", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 100.0"
        "}}");
    INVALIDATE(s, "101.5", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 101.5"
        "}}");
}

TEST(SchemaValidator, Number_RangeInt) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":-100,\"maximum\":-1,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-101", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -100, \"actual\": -101"
        "}}");
    INVALIDATE(s, "-100.1", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -100, \"actual\": -100.1"
        "}}");
    VALIDATE(s, "-100", true);
    VALIDATE(s, "-2", true);
    INVALIDATE(s, "-1", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": -1"
        "}}");
    INVALIDATE(s, "-0.9", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": -0.9"
        "}}");
    INVALIDATE(s, "0", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 0"
        "}}");
    INVALIDATE(s, "2147483647", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 2147483648"
        "}}");  // unsigned first
    INVALIDATE(s, "4294967295", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 4294967295"
        "}}");  // unsigned max
    INVALIDATE(s, "9223372036854775808", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 9223372036854775808"
        "}}");
    INVALIDATE(s, "18446744073709551614", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551614"
        "}}");
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Number_RangeDouble) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":0.1,\"maximum\":100.1,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0.1, \"actual\": -9223372036854775808"
        "}}");
    INVALIDATE(s, "-2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0.1, \"actual\": -2147483648"
        "}}"); // int min
    INVALIDATE(s, "-1", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0.1, \"actual\": -1"
        "}}");
    VALIDATE(s, "0.1", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "99", true);
    VALIDATE(s, "100", true);
    INVALIDATE(s, "101", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 101"
        "}}");
    INVALIDATE(s, "101.5", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 101.5"
        "}}");
    INVALIDATE(s, "18446744073709551614", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551614"
        "}}");
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551615"
        "}}");
    INVALIDATE(s, "2147483647", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 2147483648"
        "}}");  // unsigned first
    INVALIDATE(s, "4294967295", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 4294967295"
        "}}");  // unsigned max
    INVALIDATE(s, "9223372036854775808", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 9223372036854775808"
        "}}");
    INVALIDATE(s, "18446744073709551614", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551614"
        "}}");
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 3,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Number_RangeDoubleU64Boundary) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":9223372036854775808.0,\"maximum\":18446744073709550000.0}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": -9223372036854775808"
        "}}");
    INVALIDATE(s, "-2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": -2147483648"
        "}}"); // int min
    INVALIDATE(s, "0", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": 0"
        "}}");
    INVALIDATE(s, "2147483647", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": 2147483648"
        "}}");  // unsigned first
    INVALIDATE(s, "4294967295", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": 4294967295"
        "}}");  // unsigned max
    VALIDATE(s, "9223372036854775808", true);
    VALIDATE(s, "18446744073709540000", true);
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"errorCode\": 2,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 18446744073709550000.0, \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Number_MultipleOf) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"multipleOf\":10.0}");
    SchemaDocument s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "-10", true);
    VALIDATE(s, "20", true);
    INVALIDATE(s, "23", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": 23"
        "}}");
    INVALIDATE(s, "-2147483648", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": -2147483648"
        "}}");  // int min
    VALIDATE(s, "-2147483640", true);
    INVALIDATE(s, "2147483647", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": 2147483648"
        "}}");  // unsigned first
    VALIDATE(s, "2147483650", true);
    INVALIDATE(s, "4294967295", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": 4294967295"
        "}}");  // unsigned max
    VALIDATE(s, "4294967300", true);
}

TEST(SchemaValidator, Number_MultipleOfOne) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"multipleOf\":1}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "42.0", true);
    INVALIDATE(s, "3.1415926", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 1, \"actual\": 3.1415926"
        "}}");
}

TEST(SchemaValidator, Object) {
    Document sd;
    sd.Parse("{\"type\":\"object\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "{\"key\":\"value\",\"another_key\":\"another_value\"}", true);
    VALIDATE(s, "{\"Sun\":1.9891e30,\"Jupiter\":1.8986e27,\"Saturn\":5.6846e26,\"Neptune\":10.243e25,\"Uranus\":8.6810e25,\"Earth\":5.9736e24,\"Venus\":4.8685e24,\"Mars\":6.4185e23,\"Mercury\":3.3022e23,\"Moon\":7.349e22,\"Pluto\":1.25e22}", true);    
    INVALIDATE(s, "[\"An\", \"array\", \"not\", \"an\", \"object\"]", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"object\"], \"actual\": \"array\""
        "}}");
    INVALIDATE(s, "\"Not an object\"", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"object\"], \"actual\": \"string\""
        "}}");
}

TEST(SchemaValidator, Object_Properties) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"number\": { \"type\": \"number\" },"
        "        \"street_name\" : { \"type\": \"string\" },"
        "        \"street_type\" : { \"type\": \"string\", \"enum\" : [\"Street\", \"Avenue\", \"Boulevard\"] }"
        "    }"
        "}");

    SchemaDocument s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    INVALIDATE(s, "{ \"number\": \"1600\", \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", "/properties/number", "type", "/number",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/number\", \"schemaRef\": \"#/properties/number\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}");
    INVALIDATE(s, "{ \"number\": \"One\", \"street_name\": \"Microsoft\", \"street_type\": \"Way\" }",
        "/properties/number", "type", "/number",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/number\", \"schemaRef\": \"#/properties/number\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}"); // fail fast
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\" }", true);
    VALIDATE(s, "{}", true);
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"direction\": \"NW\" }", true);
}

TEST(SchemaValidator, Object_AdditionalPropertiesBoolean) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "        \"properties\" : {"
        "        \"number\": { \"type\": \"number\" },"
        "            \"street_name\" : { \"type\": \"string\" },"
        "            \"street_type\" : { \"type\": \"string\","
        "            \"enum\" : [\"Street\", \"Avenue\", \"Boulevard\"]"
        "        }"
        "    },"
        "    \"additionalProperties\": false"
        "}");

    SchemaDocument s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    INVALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"direction\": \"NW\" }", "", "additionalProperties", "/direction",
        "{ \"additionalProperties\": {"
        "    \"errorCode\": 16,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"disallowed\": \"direction\""
        "}}");
}

TEST(SchemaValidator, Object_AdditionalPropertiesObject) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"number\": { \"type\": \"number\" },"
        "        \"street_name\" : { \"type\": \"string\" },"
        "        \"street_type\" : { \"type\": \"string\","
        "            \"enum\" : [\"Street\", \"Avenue\", \"Boulevard\"]"
        "        }"
        "    },"
        "    \"additionalProperties\": { \"type\": \"string\" }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"direction\": \"NW\" }", true);
    INVALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"office_number\": 201 }", "/additionalProperties", "type", "/office_number",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/office_number\", \"schemaRef\": \"#/additionalProperties\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
}

TEST(SchemaValidator, Object_Required) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"name\":      { \"type\": \"string\" },"
        "        \"email\" : { \"type\": \"string\" },"
        "        \"address\" : { \"type\": \"string\" },"
        "        \"telephone\" : { \"type\": \"string\" }"
        "    },"
        "    \"required\":[\"name\", \"email\"]"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"email\" : \"bill@stratford-upon-avon.co.uk\" }", true);
    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"email\" : \"bill@stratford-upon-avon.co.uk\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\", \"authorship\" : \"in question\"}", true);
    INVALIDATE(s, "{ \"name\": \"William Shakespeare\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\" }", "", "required", "",
        "{ \"required\": {"
        "    \"errorCode\": 15,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"email\"]"
        "}}");
    INVALIDATE(s, "{}", "", "required", "",
        "{ \"required\": {"
        "    \"errorCode\": 15,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"name\", \"email\"]"
        "}}");
}

TEST(SchemaValidator, Object_Required_PassWithDefault) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"name\":      { \"type\": \"string\", \"default\": \"William Shakespeare\" },"
        "        \"email\" : { \"type\": \"string\", \"default\": \"\" },"
        "        \"address\" : { \"type\": \"string\" },"
        "        \"telephone\" : { \"type\": \"string\" }"
        "    },"
        "    \"required\":[\"name\", \"email\"]"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"email\" : \"bill@stratford-upon-avon.co.uk\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\", \"authorship\" : \"in question\"}", true);
    INVALIDATE(s, "{ \"name\": \"William Shakespeare\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\" }", "", "required", "",
        "{ \"required\": {"
        "    \"errorCode\": 15,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"email\"]"
        "}}");
    INVALIDATE(s, "{}", "", "required", "",
        "{ \"required\": {"
        "    \"errorCode\": 15,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"email\"]"
        "}}");
}

TEST(SchemaValidator, Object_PropertiesRange) {
    Document sd;
    sd.Parse("{\"type\":\"object\", \"minProperties\":2, \"maxProperties\":3}");
    SchemaDocument s(sd);

    INVALIDATE(s, "{}", "", "minProperties", "",
        "{ \"minProperties\": {"
        "    \"errorCode\": 14,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": 0"
        "}}");
    INVALIDATE(s, "{\"a\":0}", "", "minProperties", "",
        "{ \"minProperties\": {"
        "    \"errorCode\": 14,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": 1"
        "}}");
    VALIDATE(s, "{\"a\":0,\"b\":1}", true);
    VALIDATE(s, "{\"a\":0,\"b\":1,\"c\":2}", true);
    INVALIDATE(s, "{\"a\":0,\"b\":1,\"c\":2,\"d\":3}", "", "maxProperties", "",
        "{ \"maxProperties\": {"
        "    \"errorCode\": 13,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\", "
        "    \"expected\": 3, \"actual\": 4"
        "}}");
}

TEST(SchemaValidator, Object_PropertyDependencies) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"name\": { \"type\": \"string\" },"
        "    \"credit_card\": { \"type\": \"number\" },"
        "    \"cvv_code\": { \"type\": \"number\" },"
        "    \"billing_address\": { \"type\": \"string\" }"
        "  },"
        "  \"required\": [\"name\"],"
        "  \"dependencies\": {"
        "    \"credit_card\": [\"cvv_code\", \"billing_address\"]"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"name\": \"John Doe\", \"credit_card\": 5555555555555555, \"cvv_code\": 777, "
        "\"billing_address\": \"555 Debtor's Lane\" }", true);
    INVALIDATE(s, "{ \"name\": \"John Doe\", \"credit_card\": 5555555555555555 }", "", "dependencies", "",
        "{ \"dependencies\": {"
        "    \"errorCode\": 18,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"errors\": {"
        "       \"credit_card\": {"
        "        \"required\": {"
        "          \"errorCode\": 15,"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/credit_card\","
        "          \"missing\": [\"cvv_code\", \"billing_address\"]"
        "    } } }"
        "}}");
    VALIDATE(s, "{ \"name\": \"John Doe\"}", true);
    VALIDATE(s, "{ \"name\": \"John Doe\", \"cvv_code\": 777, \"billing_address\": \"555 Debtor's Lane\" }", true);
}

TEST(SchemaValidator, Object_SchemaDependencies) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"name\": { \"type\": \"string\" },"
        "        \"credit_card\" : { \"type\": \"number\" }"
        "    },"
        "    \"required\" : [\"name\"],"
        "    \"dependencies\" : {"
        "        \"credit_card\": {"
        "            \"properties\": {"
        "                \"billing_address\": { \"type\": \"string\" }"
        "            },"
        "            \"required\" : [\"billing_address\"]"
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{\"name\": \"John Doe\", \"credit_card\" : 5555555555555555,\"billing_address\" : \"555 Debtor's Lane\"}", true);
    INVALIDATE(s, "{\"name\": \"John Doe\", \"credit_card\" : 5555555555555555 }", "", "dependencies", "",
        "{ \"dependencies\": {"
        "    \"errorCode\": 18,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"errors\": {"
        "      \"credit_card\": {"
        "        \"required\": {"
        "          \"errorCode\": 15,"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/credit_card\","
        "          \"missing\": [\"billing_address\"]"
        "    } } }"
        "}}");
    VALIDATE(s, "{\"name\": \"John Doe\", \"billing_address\" : \"555 Debtor's Lane\"}", true);
}

#if RAPIDJSON_SCHEMA_HAS_REGEX
TEST(SchemaValidator, Object_PatternProperties) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"patternProperties\": {"
        "    \"^S_\": { \"type\": \"string\" },"
        "    \"^I_\": { \"type\": \"integer\" }"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"S_25\": \"This is a string\" }", true);
    VALIDATE(s, "{ \"I_0\": 42 }", true);
    INVALIDATE(s, "{ \"S_0\": 42 }", "", "patternProperties", "/S_0",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/S_0\", \"schemaRef\": \"#/patternProperties/%5ES_\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
    INVALIDATE(s, "{ \"I_42\": \"This is a string\" }", "", "patternProperties", "/I_42",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/I_42\", \"schemaRef\": \"#/patternProperties/%5EI_\","
        "    \"expected\": [\"integer\"], \"actual\": \"string\""
        "}}");
    VALIDATE(s, "{ \"keyword\": \"value\" }", true);
}

TEST(SchemaValidator, Object_PatternProperties_ErrorConflict) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"patternProperties\": {"
        "    \"^I_\": { \"multipleOf\": 5 },"
        "    \"30$\": { \"multipleOf\": 6 }"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"I_30\": 30 }", true);
    INVALIDATE(s, "{ \"I_30\": 7 }", "", "patternProperties", "/I_30",
        "{ \"multipleOf\": ["
        "    {"
        "      \"errorCode\": 1,"
        "      \"instanceRef\": \"#/I_30\", \"schemaRef\": \"#/patternProperties/%5EI_\","
        "      \"expected\": 5, \"actual\": 7"
        "    }, {"
        "      \"errorCode\": 1,"
        "      \"instanceRef\": \"#/I_30\", \"schemaRef\": \"#/patternProperties/30%24\","
        "      \"expected\": 6, \"actual\": 7"
        "    }"
        "]}");
}

TEST(SchemaValidator, Object_Properties_PatternProperties) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"I_42\": { \"type\": \"integer\", \"minimum\": 73 }"
        "  },"
        "  \"patternProperties\": {"
        "    \"^I_\": { \"type\": \"integer\", \"multipleOf\": 6 }"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"I_6\": 6 }", true);
    VALIDATE(s, "{ \"I_42\": 78 }", true);
    INVALIDATE(s, "{ \"I_42\": 42 }", "", "patternProperties", "/I_42",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#/I_42\", \"schemaRef\": \"#/properties/I_42\","
        "    \"expected\": 73, \"actual\": 42"
        "}}");
    INVALIDATE(s, "{ \"I_42\": 7 }", "", "patternProperties", "/I_42",
        "{ \"minimum\": {"
        "    \"errorCode\": 4,"
        "    \"instanceRef\": \"#/I_42\", \"schemaRef\": \"#/properties/I_42\","
        "    \"expected\": 73, \"actual\": 7"
        "  },"
        "  \"multipleOf\": {"
        "    \"errorCode\": 1,"
        "    \"instanceRef\": \"#/I_42\", \"schemaRef\": \"#/patternProperties/%5EI_\","
        "    \"expected\": 6, \"actual\": 7"
        "  }"
        "}");
}

TEST(SchemaValidator, Object_PatternProperties_AdditionalPropertiesObject) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"builtin\": { \"type\": \"number\" }"
        "  },"
        "  \"patternProperties\": {"
        "    \"^S_\": { \"type\": \"string\" },"
        "    \"^I_\": { \"type\": \"integer\" }"
        "  },"
        "  \"additionalProperties\": { \"type\": \"string\" }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"builtin\": 42 }", true);
    VALIDATE(s, "{ \"keyword\": \"value\" }", true);
    INVALIDATE(s, "{ \"keyword\": 42 }", "/additionalProperties", "type", "/keyword",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/keyword\", \"schemaRef\": \"#/additionalProperties\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
}

// Replaces test Issue285 and tests failure as well as success
TEST(SchemaValidator, Object_PatternProperties_AdditionalPropertiesBoolean) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"patternProperties\": {"
        "    \"^S_\": { \"type\": \"string\" },"
        "    \"^I_\": { \"type\": \"integer\" }"
        "  },"
        "  \"additionalProperties\": false"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"S_25\": \"This is a string\" }", true);
    VALIDATE(s, "{ \"I_0\": 42 }", true);
    INVALIDATE(s, "{ \"keyword\": \"value\" }", "", "additionalProperties", "/keyword",
        "{ \"additionalProperties\": {"
        "    \"errorCode\": 16,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"disallowed\": \"keyword\""
        "}}");
}
#endif

TEST(SchemaValidator, Array) {
    Document sd;
    sd.Parse("{\"type\":\"array\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    VALIDATE(s, "[3, \"different\", { \"types\" : \"of values\" }]", true);
    INVALIDATE(s, "{\"Not\": \"an array\"}", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"array\"], \"actual\": \"object\""
        "}}");
}

TEST(SchemaValidator, Array_ItemsList) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"array\","
        "    \"items\" : {"
        "        \"type\": \"number\""
        "    }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    INVALIDATE(s, "[1, 2, \"3\", 4, 5]", "/items", "type", "/2",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/2\", \"schemaRef\": \"#/items\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}");
    VALIDATE(s, "[]", true);
}

TEST(SchemaValidator, Array_ItemsTuple) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"array\","
        "  \"items\": ["
        "    {"
        "      \"type\": \"number\""
        "    },"
        "    {"
        "      \"type\": \"string\""
        "    },"
        "    {"
        "      \"type\": \"string\","
        "      \"enum\": [\"Street\", \"Avenue\", \"Boulevard\"]"
        "    },"
        "    {"
        "      \"type\": \"string\","
        "      \"enum\": [\"NW\", \"NE\", \"SW\", \"SE\"]"
        "    }"
        "  ]"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\"]", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "[24, \"Sussex\", \"Drive\"]", "/items/2", "enum", "/2",
        "{ \"enum\": { \"errorCode\": 19, \"instanceRef\": \"#/2\", \"schemaRef\": \"#/items/2\", \"expected\": [\"Street\", \"Avenue\", \"Boulevard\"] }}");
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "[24, \"Sussex\", \"Drive\"]", "/items/2", "enum", "/2",
        "{ \"enum\": { \"errorCode\": 19, \"instanceRef\": \"#/2\", \"schemaRef\": \"#/items/2\" }}");
#endif // RAPIDJSON_YGGDRASIL 
    INVALIDATE(s, "[\"Palais de l'Elysee\"]", "/items/0", "type", "/0",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/0\", \"schemaRef\": \"#/items/0\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}");
    INVALIDATE(s, "[\"Twenty-four\", \"Sussex\", \"Drive\"]", "/items/0", "type", "/0",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/0\", \"schemaRef\": \"#/items/0\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}"); // fail fast
    VALIDATE(s, "[10, \"Downing\", \"Street\"]", true);
    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\", \"Washington\"]", true);
}

TEST(SchemaValidator, Array_AdditionalItems) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"array\","
        "  \"items\": ["
        "    {"
        "      \"type\": \"number\""
        "    },"
        "    {"
        "      \"type\": \"string\""
        "    },"
        "    {"
        "      \"type\": \"string\","
        "      \"enum\": [\"Street\", \"Avenue\", \"Boulevard\"]"
        "    },"
        "    {"
        "      \"type\": \"string\","
        "      \"enum\": [\"NW\", \"NE\", \"SW\", \"SE\"]"
        "    }"
        "  ],"
        "  \"additionalItems\": false"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\"]", true);
    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\"]", true);
    INVALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\", \"Washington\"]", "", "additionalItems", "/4",
        "{ \"additionalItems\": {"
        "    \"errorCode\": 12,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"disallowed\": 4"
        "}}");
}

TEST(SchemaValidator, Array_ItemsRange) {
    Document sd;
    sd.Parse("{\"type\": \"array\",\"minItems\": 2,\"maxItems\" : 3}");
    SchemaDocument s(sd);

    INVALIDATE(s, "[]", "", "minItems", "",
        "{ \"minItems\": {"
        "    \"errorCode\": 10,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": 0"
        "}}");
    INVALIDATE(s, "[1]", "", "minItems", "",
        "{ \"minItems\": {"
        "    \"errorCode\": 10,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": 1"
        "}}");
    VALIDATE(s, "[1, 2]", true);
    VALIDATE(s, "[1, 2, 3]", true);
    INVALIDATE(s, "[1, 2, 3, 4]", "", "maxItems", "",
        "{ \"maxItems\": {"
        "    \"errorCode\": 9,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 3, \"actual\": 4"
        "}}");
}

TEST(SchemaValidator, Array_UniqueItems) {
    Document sd;
    sd.Parse("{\"type\": \"array\", \"uniqueItems\": true}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    INVALIDATE(s, "[1, 2, 3, 3, 4]", "", "uniqueItems", "/3",
        "{ \"uniqueItems\": {"
        "    \"errorCode\": 11,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"duplicates\": [2, 3]"
        "}}");
    INVALIDATE(s, "[1, 2, 3, 3, 3]", "", "uniqueItems", "/3",
        "{ \"uniqueItems\": {"
        "    \"errorCode\": 11,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"duplicates\": [2, 3]"
        "}}"); // fail fast
    VALIDATE(s, "[]", true);
}

TEST(SchemaValidator, Boolean) {
    Document sd;
    sd.Parse("{\"type\":\"boolean\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "true", true);
    VALIDATE(s, "false", true);
    INVALIDATE(s, "\"true\"", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"boolean\"], \"actual\": \"string\""
        "}}");
    INVALIDATE(s, "0", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"boolean\"], \"actual\": \"integer\""
        "}}");
}

TEST(SchemaValidator, Null) {
    Document sd;
    sd.Parse("{\"type\":\"null\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "null", true);
    INVALIDATE(s, "false", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"null\"], \"actual\": \"boolean\""
        "}}");
    INVALIDATE(s, "0", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"null\"], \"actual\": \"integer\""
        "}}");
    INVALIDATE(s, "\"\"", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"null\"], \"actual\": \"string\""
        "}}");
}

#ifdef RAPIDJSON_YGGDRASIL
TEST(SchemaValidator, Array_UniqueItems_Yggdrasil) {
    Document sd;
    sd.Parse("{\"type\": \"array\", \"uniqueItems\": true}");
    SchemaDocument s(sd);
    
    VALIDATE(s, "[1, 2, 3, \"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoiZyJ9-YGG-DA==-YGG-\", \"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImludCIsInByZWNpc2lvbiI6MSwidW5pdHMiOiJnIn0=-YGG-DA==-YGG-\", \"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\"]", true);
    INVALIDATE(s, "[1, 2, \"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoiZyJ9-YGG-BQ==-YGG-\", \"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoiZyJ9-YGG-BQ==-YGG-\"]",
	       "", "uniqueItems", "/3",
        "{ \"uniqueItems\": {"
        "    \"errorCode\": 11,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"duplicates\": [2, 3]"
        "}}");
    INVALIDATE(s, "[1, 2, \"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\", \"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\", \"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\"]", "", "uniqueItems", "/3",
        "{ \"uniqueItems\": {"
        "    \"errorCode\": 11,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"duplicates\": [2, 3]"
        "}}"); // fail fast
}
TEST(SchemaValidator, ScalarInterop) {
  { // Double
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"float\","
	"  \"precision\": 8"
        "}");
    SchemaDocument s(sd);
    INVALIDATE(s, "1",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"float\"], \"actual\": \"uint\""
	       "}}");
    INVALIDATE(s, "-1",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"float\"], \"actual\": \"int\""
	       "}}");
    INVALIDATE(s, "4294967296",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"float\"], \"actual\": \"uint\""
	       "}}");
    INVALIDATE(s, "-2147483649",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"float\"], \"actual\": \"int\""
	       "}}");
    VALIDATE(s, "5.5", true);
  }
  { // Int
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"int\","
	"  \"precision\": 4"
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "1", true);
    VALIDATE(s, "-1", true);
    INVALIDATE(s, "4294967296",
	       "", "precision", "",
	       "{ \"precision\" : {"
	       "    \"errorCode\": 28,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": 4, \"actual\": 8"
	       "}}");
    INVALIDATE(s, "-2147483649",
	       "", "precision", "",
	       "{ \"precision\" : {"
	       "    \"errorCode\": 28,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": 4, \"actual\": 8"
	       "}}");
    INVALIDATE(s, "5.5",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"int\"], \"actual\": \"float\""
	       "}}");
  }
  { // Int64
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"int\","
	"  \"precision\": 8"
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "1", true);
    VALIDATE(s, "-1", true);
    VALIDATE(s, "4294967296", true);
    VALIDATE(s, "-2147483649", true);
    INVALIDATE(s, "5.5",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"int\"], \"actual\": \"float\""
	       "}}");
  }
  { // Uint64
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 8"
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "1", true);
    INVALIDATE(s, "-1",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"uint\"], \"actual\": \"int\""
	       "}}");
    VALIDATE(s, "4294967296", true);
    INVALIDATE(s, "-2147483649",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"uint\"], \"actual\": \"int\""
	       "}}");
    INVALIDATE(s, "5.5",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"uint\"], \"actual\": \"float\""
	       "}}");
  }
}
TEST(SchemaValidator, SubType) {
  { // Uint
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 1,"
	"  \"units\": \"g\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoiZyJ9-YGG-DA==-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImludCIsInByZWNpc2lvbiI6MSwidW5pdHMiOiJnIn0=-YGG-DA==-YGG-\"",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"uint\"], \"actual\": \"int\""
	       "}}");
    INVALIDATE(s, "0.15",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"uint\"], \"actual\": \"float\""
	       "}}");
  }
  { // Float
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"float\","
	"  \"precision\": 8,"
	"  \"units\": \"g\""
        "}");
    SchemaDocument s(sd);
    // VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoiZyJ9-YGG-DA==-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImludCIsInByZWNpc2lvbiI6MSwidW5pdHMiOiJnIn0=-YGG-DA==-YGG-\"",
	       "", "subtype", "",
	       "{ \"subtype\" : {"
	       "    \"errorCode\": 27,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [\"float\"], \"actual\": \"int\""
	       "}}");
  }
}
TEST(SchemaValidator, AnySubType) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"any\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "5.5", true);
    VALIDATE(s, "1", true);
    VALIDATE(s, "\"hello\"", true);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImludCIsInByZWNpc2lvbiI6MSwidW5pdHMiOiJnIn0=-YGG-DA==-YGG-\"", true);
}
TEST(SchemaValidator, MultiSubType) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": [\"float\", \"complex\"]"
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "5.5", true);
    INVALIDATE(s, "1",
               "", "subtype", "",
               "{ \"subtype\" : {"
               "    \"errorCode\": 27,"
               "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
               "    \"expected\": [\"float\", \"complex\"], "
               "\"actual\": \"uint\""
               "}}");
}
TEST(SchemaValidator, Precision) { // 28
  { // Scalar
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 2,"
	"  \"units\": \"g\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoiZyJ9-YGG-DA==-YGG-\"", true);
    INVALIDATE(s, "12",
	       "", "precision", "",
	       "{ \"precision\" : {"
	       "    \"errorCode\": 28,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": 2, \"actual\": 4"
	       "}}");
  }
  { // Array
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"ndarray\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 2,"
	"  \"shape\": [2, 3],"
	"  \"units\": \"g\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoyLCJ1bml0cyI6ImciLCJzaGFwZSI6WzIsM119-YGG-AAABAAIAAwAEAAUA-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjo4LCJ1bml0cyI6ImciLCJzaGFwZSI6WzIsM119-YGG-AAAAAAAAAAABAAAAAAAAAAIAAAAAAAAAAwAAAAAAAAAEAAAAAAAAAAUAAAAAAAAA-YGG-\"",
	       "", "precision", "",
	       "{ \"precision\" : {"
	       "    \"errorCode\": 28,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": 2, \"actual\": 8"
	       "}}");
  }
}
TEST(SchemaValidator, Units) { // 29
  { // Scalar
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 2,"
	"  \"units\": \"g\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjIsInVuaXRzIjoiZyJ9-YGG-DAA=-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjIsInVuaXRzIjoiY20ifQ==-YGG-DAA=-YGG-\"",
	       "", "units", "",
	       "{ \"units\" : {"
	       "    \"errorCode\": 29,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": \"g\", \"actual\": \"cm\""
	       "}}");
    // Test with units that have the same dimensions
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoia2cifQ==-YGG-DA==-YGG-\"", true);
  }
  { // Array
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"1darray\","
	"  \"subtype\": \"uint\","
	"  \"length\": 3,"
	"  \"precision\": 2,"
	"  \"units\": \"g\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoyLCJ1bml0cyI6ImciLCJzaGFwZSI6WzNdfQ==-YGG-AAABAAIA-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoyLCJ1bml0cyI6ImNtIiwic2hhcGUiOlszXX0=-YGG-AAABAAIA-YGG-\"",
	       "", "units", "",
	       "{ \"units\" : {"
	       "    \"errorCode\": 29,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": \"g\", \"actual\": \"cm\""
	       "}}");
    // Test with units that have the same dimensions
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoyLCJ1bml0cyI6ImtnIiwic2hhcGUiOlszXX0=-YGG-AAABAAIA-YGG-\"", true);
  }
}
TEST(SchemaValidator, Length) { // 30
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"1darray\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 2,"
	"  \"units\": \"g\","
	"  \"length\": 3"
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImciLCJzaGFwZSI6WzNdfQ==-YGG-AAEC-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImciLCJzaGFwZSI6WzRdfQ==-YGG-AAECAw==-YGG-\"",
	       "", "shape", "",
	       "{ \"shape\" : {"
	       "    \"errorCode\": 30,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [3], \"actual\": [4]"
	       "}}");
}
TEST(SchemaValidator, Shape) { // 30
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"ndarray\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 2,"
	"  \"units\": \"g\","
	"  \"shape\": [2, 3]"
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImciLCJzaGFwZSI6WzIsM119-YGG-AAECAwQF-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImciLCJzaGFwZSI6WzIsNF19-YGG-AAECAgMEBQY=-YGG-\"",
	       "", "shape", "",
	       "{ \"shape\" : {"
	       "    \"errorCode\": 30,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [2, 3], \"actual\": [2, 4]"
	       "}}");
}
TEST(SchemaValidator, NDim) { // 30
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"ndarray\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 2,"
	"  \"units\": \"g\","
	"  \"ndim\": 2"
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImciLCJzaGFwZSI6WzIsM119-YGG-AAECAwQF-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImciLCJzaGFwZSI6WzNdfQ==-YGG-AAEC-YGG-\"",
	       "", "shape", "",
	       "{ \"shape\" : {"
	       "    \"errorCode\": 30,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"expected\": [null, null], \"actual\": [3]"
	       "}}");
}
TEST(SchemaValidator, InvalidSchema) {
  { // scalar
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"scalar\","
	"  \"subtype\": \"uint\","
	"  \"precision\": 1,"
	"  \"units\": \"g\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoiZyJ9-YGG-BQ==-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImcifQ==-YGG-BQ==-YGG-\"",
	       "", "required", "",
	       "{ \"class\" : {"
	       "    \"errorCode\": 32,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"missing\": \"subtype\""
	       "}}");
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJ1bml0cyI6ImcifQ==-YGG-BQ==-YGG-\"",
	       "", "required", "",
	       "{ \"class\" : {"
	       "    \"errorCode\": 32,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"missing\": \"precision\""
	       "}}");
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjF9-YGG-BQ==-YGG-\"", true);
  }
  { // array
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"ndarray\","
	"  \"subtype\": \"float\","
	"  \"precision\": 8,"
	"  \"shape\": [2, 3],"
	"  \"units\": \"g\""
        "}");
    // {
    //   Value schema(kObjectType);
    //   Value::AllocatorType allocator;
    //   double val[2][3] = {{0.0, 1.0, 2.0},
    // 			  {3.0, 4.0, 5.5}};
    //   schema.AddMember(Value("type").Move(), Value("ndarray").Move(), allocator);
    //   schema.AddMember(Value("subtype").Move(), Value("float").Move(), allocator);
    //   schema.AddMember(Value("precision").Move(), Value(8).Move(), allocator);
    //   Value shape(kArrayType);
    //   shape.PushBack(Value(2).Move(), allocator);
    //   shape.PushBack(Value(3).Move(), allocator);
    //   schema.AddMember(Value("shape").Move(), shape, allocator);
    //   schema.AddMember(Value("units").Move(), Value("g").Move(), allocator);
    //   DISPLAY_STRING("valid", ((char*)(&val[0][0]), sizeof(double) * 6, schema));
    // }
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJmbG9hdCIsInByZWNpc2lvbiI6OCwic2hhcGUiOlsyLDNdLCJ1bml0cyI6ImcifQ==-YGG-AAAAAAAAAAAAAAAAAADwPwAAAAAAAABAAAAAAAAACEAAAAAAAAAQQAAAAAAAABZA-YGG-\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJmbG9hdCIsInByZWNpc2lvbiI6OCwidW5pdHMiOiJnIn0=-YGG-AAAAAAAAAAAAAAAAAADwPwAAAAAAAABAAAAAAAAACEAAAAAAAAAQQAAAAAAAABZA-YGG-\"",
	       "", "required", "",
	       "{ \"class\" : {"
	       "    \"errorCode\": 32,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"missing\": \"shape\""
	       "}}");
  }
  { // array, no shape in schema for validation
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"ndarray\","
	"  \"subtype\": \"float\","
	"  \"precision\": 8,"
	"  \"units\": \"g\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJmbG9hdCIsInByZWNpc2lvbiI6OCwic2hhcGUiOlsyLDNdLCJ1bml0cyI6ImcifQ==-YGG-AAAAAAAAAAAAAAAAAADwPwAAAAAAAABAAAAAAAAACEAAAAAAAAAQQAAAAAAAABZA-YGG-\"", true);
  }
  { // array invalid type
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"integer\""
        "}");
    SchemaDocument s(sd);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJmbG9hdCIsInByZWNpc2lvbiI6OCwic2hhcGUiOlsyLDNdLCJ1bml0cyI6ImcifQ==-YGG-AAAAAAAAAAAAAAAAAADwPwAAAAAAAABAAAAAAAAACEAAAAAAAAAQQAAAAAAAABZA-YGG-\"",
	       "", "type", "",
	       "{ \"type\": {"
	       "    \"expected\": [ \"integer\" ],"
	       "    \"actual\": \"ndarray\","
	       "    \"errorCode\": 20,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\""
	       "}}");
  }
}

#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
TEST(SchemaValidator, PythonClass) { // 32
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"class\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-ZXhhbXBsZV9weXRob246RXhhbXBsZUNsYXNz-YGG-\"", true);
    VALIDATE(s, "\"example_python:ExampleSubClass\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-aW52YWxpZA==-YGG-\"",
	       "", "class", "",
	       "{ \"class\" : {"
	       "    \"errorCode\": 32,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"disallowed\": \"invalid\""
	       "}}");
}

TEST(SchemaValidator, PythonFunction) { // 32
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"function\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoiZnVuY3Rpb24ifQ==-YGG-ZXhhbXBsZV9weXRob246ZXhhbXBsZV9mdW5jdGlvbgA=-YGG-\"", true);
    VALIDATE(s, "\"example_python:example_function\"", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-aW52YWxpZA==-YGG-\"",
	       "", "type", "",
	       "{ \"type\" : {"
	       "    \"expected\": [\"function\"],"
	       "    \"actual\": \"class\","
	       "    \"errorCode\": 20,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\""
	       "}}");
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoiZnVuY3Rpb24ifQ==-YGG-aW52YWxpZA==-YGG-\"",
	       "", "class", "",
	       "{ \"class\" : {"
	       "    \"errorCode\": 32,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"disallowed\": \"invalid\""
	       "}}");
}

TEST(SchemaValidator, PythonInstance) { // 32
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"instance\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\"", true);
    VALIDATE(s,
	     "{"
	     "  \"class\": \"example_python:ExampleSubClass\","
	     "  \"args\": ["
	     "    \"hello\","
	     "    0.5"
	     "  ],"
	     "  \"kwargs\": {"
	     "    \"a\": \"world\","
	     "    \"b\": 1"
	     "  }"
	     "}", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImludmFsaWQiLCJhcmdzIjpbImhlbGxvIiwwLjVdLCJrd2FyZ3MiOnsiYSI6IndvcmxkIiwiYiI6MX19-YGG-\"",
	       "", "schema", "",
	       "{ \"schema\": {"
	       "    \"errorCode\": 35,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"errors\": {"
	       "        \"anyOf\": {"
	       "            \"errorCode\": 24,"
	       "            \"instanceRef\": \"#/class\","
	       "            \"schemaRef\": \"#/properties/class\","
	       "            \"errors\": ["
	       "                { \"class\" : {"
	       "                  \"errorCode\": 32,"
	       "                  \"instanceRef\": \"#\", "
	       "                  \"schemaRef\": \"#/properties/class/anyOf/0\","
	       "                  \"disallowed\": \"invalid\"}}, "
	       "                { \"type\" : {"
	       "                  \"expected\": [ \"array\" ],"
	       "                  \"actual\": \"string\","
	       "                  \"errorCode\": 20,"
	       "                  \"instanceRef\": \"#/class\", "
	       "                  \"schemaRef\": \"#/properties/class/anyOf/1\"}}]"
	       "}}}}");
    // No kwargs
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6e319-YGG-\"", true);
    // No args
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOltdLCJrd2FyZ3MiOnsiYSI6IndvcmxkIiwiYiI6MX19-YGG-\"", true);
    // Schema
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiaW50IiwicHJlY2lzaW9uIjo4fQ==-YGG-\"",
	       "", "type", "",
	       "{"
	       "  \"type\": {"
	       "    \"expected\": [ \"instance\" ],"
	       "    \"actual\": \"schema\","
	       "    \"errorCode\": 20,"
	       "    \"instanceRef\": \"#\","
	       "    \"schemaRef\": \"#\""
	       "  }"
	       "}");
}

TEST(SchemaValidator, PythonInstanceClass) { // 34
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"instance\","
	"  \"class\": \"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-ZXhhbXBsZV9weXRob246RXhhbXBsZUNsYXNz-YGG-\""
        "}");
    SchemaDocument s(sd);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\"", true);
    VALIDATE(s, "\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVTdWJDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\"", true);
    VALIDATE(s,
	     "{"
	     "  \"class\": \"example_python:ExampleSubClass\","
	     "  \"args\": ["
	     "    \"hello\","
	     "    0.5"
	     "  ],"
	     "  \"kwargs\": {"
	     "    \"a\": \"world\","
	     "    \"b\": 1"
	     "  }"
	     "}", true);
    INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOk90aGVyQ2xhc3MiLCJhcmdzIjpbImhlbGxvIiwwLjVdLCJrd2FyZ3MiOnsiYSI6IndvcmxkIiwiYiI6MX19-YGG-\"",
	       "", "schema", "",
	       "{ \"schema\": {"
	       "    \"errorCode\": 35,"
	       "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	       "    \"errors\": {"
	       "        \"anyOf\": {"
	       "            \"errorCode\": 24,"
	       "            \"instanceRef\": \"#/class\","
	       "            \"schemaRef\": \"#/properties/class\","
	       "            \"errors\": ["
	       "                { \"class\" : {"
	       "                  \"errorCode\": 33,"
	       "                  \"instanceRef\": \"#\", "
	       "                  \"schemaRef\": \"#/properties/class/anyOf/0\","
	       "                  \"expected\": \"example_python:ExampleClass\","
	       "                  \"actual\": \"example_python:OtherClass\"}}, "
	       "                { \"type\" : {"
	       "                  \"expected\": [ \"array\" ],"
	       "                  \"actual\": \"string\","
	       "                  \"errorCode\": 20,"
	       "                  \"instanceRef\": \"#/class\", "
	       "                  \"schemaRef\": \"#/properties/class/anyOf/1\"}}]"
	       "}}}}");
}
#endif // YGGDRASIL_DISABLE_PYTHON_C_API

TEST(SchemaValidator, Schema) { // 34
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"schema\""
        "}");
  SchemaDocument s(sd);
  VALIDATE(s, "{\"type\": \"string\"}", true);
  VALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiaW50IiwicHJlY2lzaW9uIjo4fQ==-YGG-\"", true);
  INVALIDATE(s, "{\"type\": \"invalid\"}",
	     "", "schema", "",
	     "{ \"schema\": {"
	     "    \"errorCode\": 35,"
	     "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	     "    \"errors\": {"
	     "        \"anyOf\": {"
	     "            \"errorCode\": 24,"
	     "            \"instanceRef\": \"#/type\","
	     "            \"schemaRef\": \"#/properties/type\","
	     "            \"errors\": ["
	     "                {\"enum\":{\"errorCode\":19,\"instanceRef\":\"#/type\",\"schemaRef\":\"#/definitions/simpleTypes\","
	     "                           \"expected\": " SIMPLE_TYPES_STRING
	     "}},"
	     "                {\"type\":{\"expected\":[\"array\"],\"actual\":\"string\",\"errorCode\":20,\"instanceRef\":\"#/type\",\"schemaRef\":\"#/properties/type/anyOf/1\"}}"
	     "            ]"
	     "}}}}");
  INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiaW52YWxpZCJ9-YGG-\"",
	     "", "schema", "",
	     "{ \"schema\": {"
	     "    \"errorCode\": 35,"
	     "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	     "    \"errors\": {"
	     "        \"anyOf\": {"
	     "            \"errorCode\": 24,"
	     "            \"instanceRef\": \"#/type\","
	     "            \"schemaRef\": \"#/properties/type\","
	     "            \"errors\": ["
	     "                {\"enum\":{\"errorCode\":19,\"instanceRef\":\"#/type\",\"schemaRef\":\"#/definitions/simpleTypes\","
	     "                           \"expected\": " SIMPLE_TYPES_STRING
	     "}},"
	     "                {\"type\":{\"expected\":[\"array\"],\"actual\":\"string\",\"errorCode\":20,\"instanceRef\":\"#/type\",\"schemaRef\":\"#/properties/type/anyOf/1\"}}"
	     "            ]"
	     "}}}}");
#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
  // Python instnace
  INVALIDATE(s, "\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\"",
	     "", "type", "",
	     "{"
	     "  \"type\": {"
	     "    \"expected\": [ \"schema\" ],"
	     "    \"actual\": \"instance\","
	     "    \"errorCode\": 20,"
	     "    \"instanceRef\": \"#\","
	     "    \"schemaRef\": \"#\""
	     "  }"
	     "}");
#endif // YGGDRASIL_DISABLE_PYTHON_C_API
}

TEST(SchemaValidator, Ply) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"ply\""
        "}");
  SchemaDocument s(sd);
  VALIDATE(s,
	   "\"-YGG-eyJ0eXBlIjoicGx5In0=-YGG-cGx5CmZvcm1hdCBhc2NpaSAxLjAKZWxlbWVudCB2ZXJ0ZXggOApwcm9wZXJ0eSBkb3VibGUgeApwcm9wZXJ0eSBkb3VibGUgeQpwcm9wZXJ0eSBkb3VibGUgegplbGVtZW50IGZhY2UgMgpwcm9wZXJ0eSBsaXN0IHVjaGFyIGludCB2ZXJ0ZXhfaW5kZXgKZWxlbWVudCBlZGdlIDUKcHJvcGVydHkgaW50IHZlcnRleDEKcHJvcGVydHkgaW50IHZlcnRleDIKZW5kX2hlYWRlcgowIDAgMAowIDAgMQowIDEgMQowIDEgMAoxIDAgMAoxIDAgMQoxIDEgMQoxIDEgMAozIDMgMCAxCjMgMyAwIDIKMCAxCjEgMgoyIDMKMyAwCjIgMAo=-YGG-\"",
	   true);
}

TEST(SchemaValidator, ObjWavefront) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"obj\""
        "}");
  SchemaDocument s(sd);
  VALIDATE(s,
	   "\"-YGG-eyJ0eXBlIjoib2JqIn0=-YGG-diAwIDAgMAp2IDAgMCAxCnYgMCAxIDEKdiAwIDEgMAp2IDEgMCAwCnYgMSAwIDEKdiAxIDEgMQp2IDEgMSAwCmYgNCAxIDIKZiA0IDEgMwpsIDEgMgpsIDIgMwpsIDMgNApsIDQgMQpsIDMgMQoK-YGG-\"",
	   true);
}

TEST(SchemaValidator, Alias) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
	"  \"properties\": {"
	"     \"street_address\": { \"type\": \"string\","
	"                           \"aliases\": [\"street\"] }},"
	"  \"required\": [\"street_address\"]"
        "}");
  SchemaDocument s(sd);
  VALIDATE(s, "{ \"street\": \"1600 Pennsylvania Ave.\" }", true);
}

TEST(SchemaValidator, ArrayWrapped) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
	"  \"properties\": {"
	"     \"streets\": { \"type\": \"string\","
	"                    \"allowWrapped\": true,"
	"                    \"aliases\": [\"street\"] }},"
	"  \"required\": [\"streets\"]"
        "}");
  SchemaDocument s(sd);
  VALIDATE(s, "{ \"streets\": [\"1600 Pennsylvania Ave.\"] }", true);
  VALIDATE(s, "{ \"street\": [\"1600 Pennsylvania Ave.\"] }", true);
}

TEST(SchemaValidator, ObjectWrapped) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
	"  \"properties\": {"
	"     \"streets\": { \"type\": \"string\","
	"                    \"allowWrapped\": \"key\" }},"
	"  \"required\": [\"streets\"]"
        "}");
  SchemaDocument s(sd);
  VALIDATE(s, "{ \"streets\": { \"key\": \"1600 Pennsylvania Ave.\" } }", true);
}

TEST(SchemaValidator, SingularArray) {
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
  VALIDATE(s, "{ \"streets\": \"1600 Pennsylvania Ave.\" }", true);
  VALIDATE(s, "{ \"street\": \"1600 Pennsylvania Ave.\" }", true);
}

TEST(SchemaValidator, SingularArrayError) {
  Document sd;
  sd.Parse("{"
	   "  \"type\": \"array\","
	   "  \"allowSingular\": true,"
	   "  \"items\": [{\"type\": \"string\"}]"
	   "}");
  SchemaDocument s(sd);
  INVALIDATE(s, "{\"Not\": \"an array\"}", "", "type", "",
	     "{ \"type\": {"
	     "    \"errorCode\": 20,"
	     "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	     "    \"expected\": [\"array\"], \"actual\": \"object\""
	     "  },"
	     "  \"singular\": {"
	     "    \"type\": {"
	     "      \"errorCode\": 20,"
	     "      \"instanceRef\": \"#\", \"schemaRef\": \"#/items/0\","
	     "      \"expected\": [\"string\"], \"actual\": \"object\""
	     "    }"
	     "  }"
	     "}");
  VALIDATE(s, "\"string\"", true);
}

TEST(SchemaValidator, SingularArraySchema) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"array\","
        "  \"allowSingular\": true,"
        "  \"items\": {\"type\": \"schema\"}"
        "}");
  SchemaDocument s(sd);
  VALIDATE(s, "{\"type\": \"string\"}", true);
  INVALIDATE(s, "{\"type\": \"invalid\"}",
	     "", "type", "",
	     "{"
	     "  \"type\": {"
	     "    \"errorCode\": 20,"
	     "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	     "    \"expected\": [\"array\"], \"actual\": \"object\""
	     "  },"
	     "  \"singular\": { \"schema\": {"
	     "    \"errorCode\": 35,"
	     "    \"instanceRef\": \"#\", \"schemaRef\": \"#/items\","
	     "    \"errors\": {"
	     "        \"anyOf\": {"
	     "            \"errorCode\": 24,"
	     "            \"instanceRef\": \"#/type\","
	     "            \"schemaRef\": \"#/properties/type\","
	     "            \"errors\": ["
	     "                {\"enum\":{\"errorCode\":19,\"instanceRef\":\"#/type\",\"schemaRef\":\"#/definitions/simpleTypes\","
	     "                           \"expected\": " SIMPLE_TYPES_STRING
	     "}},"
	     "                {\"type\":{\"expected\":[\"array\"],\"actual\":\"string\",\"errorCode\":20,\"instanceRef\":\"#/type\",\"schemaRef\":\"#/properties/type/anyOf/1\"}}"
	     "            ]"
	     "  }}}}"
	     "}");
}

TEST(SchemaValidator, SingularObject) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
	"  \"properties\": {"
	"     \"streets\": { \"type\": \"array\","
	"                    \"items\": {\"type\": \"string\"},"
	"                    \"allowSingular\": true }},"
	"  \"allowSingular\": true,"
	"  \"required\": [\"streets\"]"
        "}");
  SchemaDocument s(sd);
  VALIDATE(s, "\"1600 Pennsylvania Ave.\"", true);
}

TEST(SchemaValidator, SingularObjectError) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
	"  \"properties\": {"
	"     \"streets\": { \"type\": \"array\","
	"                    \"items\": {\"type\": \"string\"},"
	"                    \"allowSingular\": true }},"
	"  \"allowSingular\": true,"
	"  \"required\": [\"streets\"]"
        "}");
  SchemaDocument s(sd);
  INVALIDATE(s, "true", "", "type", "",
	     "{ \"type\": {"
	     "    \"errorCode\": 20,"
	     "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
	     "    \"expected\": [\"object\"], \"actual\": \"boolean\""
	     "  },"
	     "  \"singular\": {"
	     "    \"type\": {"
	     "      \"errorCode\": 20,"
	     "      \"instanceRef\": \"#\", \"schemaRef\": \"#/properties/streets\","
	     "      \"expected\": [\"array\"], \"actual\": \"boolean\""
	     "    },"
	     "    \"singular\": {"
	     "      \"type\": {"
	     "        \"errorCode\": 20,"
	     "        \"instanceRef\": \"#\", \"schemaRef\": \"#/properties/streets/items\","
	     "        \"expected\": [\"string\"], \"actual\": \"boolean\""
	     "      }"
	     "    }"
	     "  }"
	     "}");
  VALIDATE(s, "[ \"string\" ]", true);
}

TEST(SchemaValidator, SingularObjectSchema) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"allowSingular\": true,"
        "  \"properties\": {\"key\": {\"type\": \"schema\"}},"
	"  \"additionalProperties\": false"
        "}");
  SchemaDocument s(sd);
  VALIDATE(s, "{\"type\": \"string\"}", true);
  INVALIDATE(s, "{\"type\": \"invalid\"}",
	     "", "additionalProperties", "",
	     "{"
	     "  \"additionalProperties\": {"
	     "    \"disallowed\": \"type\","
	     "    \"errorCode\": 16,"
	     "    \"instanceRef\": \"#\","
	     "    \"schemaRef\": \"#\""
	     "  },"
	     "  \"singular\": { \"schema\": {"
	     "    \"errorCode\": 35,"
	     "    \"instanceRef\": \"#\", \"schemaRef\": \"#/properties/key\","
	     "    \"errors\": {"
	     "        \"anyOf\": {"
	     "            \"errorCode\": 24,"
	     "            \"instanceRef\": \"#/type\","
	     "            \"schemaRef\": \"#/properties/type\","
	     "            \"errors\": ["
	     "                {\"enum\":{\"errorCode\":19,\"instanceRef\":\"#/type\",\"schemaRef\":\"#/definitions/simpleTypes\","
	     "                           \"expected\": " SIMPLE_TYPES_STRING
	     "}},"
	     "                {\"type\":{\"expected\":[\"array\"],\"actual\":\"string\",\"errorCode\":20,\"instanceRef\":\"#/type\",\"schemaRef\":\"#/properties/type/anyOf/1\"}}"
	     "            ]"
	     "}}}}}");
}

TEST(SchemaValidator, CircularAliases) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
	"  \"properties\": {"
	"     \"street_address\": { \"type\": \"string\","
	"                           \"aliases\": [\"street\"] },"
	"     \"street\":         { \"type\": \"string\","
	"                           \"aliases\": [\"street_address\"] }},"
	"  \"required\": [\"street_address\"]"
        "}");
  SchemaDocument s(sd);
  INVALIDATE(s, "{ \"street\": \"1600 Pennsylvania Ave.\" }",
	     "", "aliases", "/street",
	     "{ \"aliases\": {"
	     "    \"errorCode\": 39,"
	     "    \"instanceRef\": \"#\","
	     "    \"schemaRef\": \"#\","
	     "    \"circular\": [\"street\", \"street_address\"]"
	     "}}");
}

TEST(SchemaValidator, ConflictingAliases) {
  Document sd;
  sd.Parse(
        "{"
        "  \"type\": \"object\","
	"  \"properties\": {"
	"     \"street_address\": { \"type\": \"string\","
	"                           \"aliases\": [\"street\"] },"
	"     \"address\":        { \"type\": \"string\","
	"                           \"aliases\": [\"street\"] }},"
	"  \"required\": [\"street_address\"]"
        "}");
  SchemaDocument s(sd);
  INVALIDATE(s, "{ \"street\": \"1600 Pennsylvania Ave.\" }",
	     "", "aliases", "/street",
	     "{ \"aliases\": {"
	     "    \"errorCode\": 40,"
	     "    \"instanceRef\": \"#\","
	     "    \"schemaRef\": \"#\","
	     "    \"conflicting\": \"street\","
	     "    \"expected\": \"street_address\","
	     "    \"actual\": \"address\""
	     "}}");
}

TEST(SchemaValidator, Deprecating) {
  Document sd;
  sd.Parse("{"
	   "  \"type\": \"object\","
	   "  \"properties\": {"
	   "     \"deprecated\": {"
	   "        \"type\": \"string\","
	   "        \"deprecated\": \"Deprecation message\"},"
	   "     \"valid\": {"
	   "        \"type\": \"integer\"}"
	   "  }"
	   "}");
  SchemaDocument s(sd);
  VALIDATE_WARNING(s, "{\"deprecated\": \"string\", \"valid\": 0}",
		   "", "warnings", "",
		   "{ \"deprecated\": {"
		   "    \"errorCode\": 43,"
		   "    \"instanceRef\": \"#/deprecated\","
		   "    \"schemaRef\": \"#/properties/deprecated\","
		   "    \"warning\": \"Deprecation message\""
		   "  }"
		   "}")
}

TEST(SchemaValidator, DeprecatingBool) {
  Document sd;
  sd.Parse("{"
	   "  \"type\": \"object\","
	   "  \"properties\": {"
	   "     \"deprecated\": {"
	   "        \"type\": \"string\","
	   "        \"deprecated\": true},"
	   "     \"valid\": {"
	   "        \"type\": \"integer\"}"
	   "  }"
	   "}");
  SchemaDocument s(sd);
  VALIDATE_WARNING(s, "{\"deprecated\": \"string\", \"valid\": 0}",
		   "", "warnings", "",
		   "{ \"deprecated\": {"
		   "    \"errorCode\": 43,"
		   "    \"instanceRef\": \"#/deprecated\","
		   "    \"schemaRef\": \"#/properties/deprecated\""
		   "  }"
		   "}")
}

TEST(SchemaValidator, DeprecatingArray) {
  Document sd;
  sd.Parse("{"
	   "  \"type\": \"object\","
	   "  \"allOf\": ["
	   "    {"
	   "      \"properties\": {"
	   "         \"deprecated\": {"
	   "            \"type\": \"string\","
	   "            \"deprecated\": true},"
	   "         \"valid\": {"
	   "            \"type\": \"integer\"}"
	   "      }"
	   "    },"
	   "    {"
	   "      \"allOf\": ["
	   "        {"
	   "          \"properties\": {"
	   "             \"deprecated2\": {"
	   "                \"type\": \"string\","
	   "                \"deprecated\": true}"
	   "          }"
	   "        },"
	   "        {"
	   "          \"properties\": {"
	   "             \"deprecated3\": {"
	   "                \"type\": \"string\","
	   "                \"deprecated\": true}"
	   "          }"
	   "        }"
	   "      ]"
	   "    }"
	   "  ]"
	   "}");
  SchemaDocument s(sd);
  VALIDATE_WARNING(s, "{\"deprecated\": \"string\", \"valid\": 0, \"deprecated2\": \"string\"}",
		   "", "warnings", "",
		   "{ \"deprecated\": ["
		   "  {"
		   "    \"errorCode\": 43,"
		   "    \"instanceRef\": \"#/deprecated\","
		   "    \"schemaRef\": \"#/allOf/0/properties/deprecated\""
		   "  },"
		   "  {"
		   "    \"errorCode\": 43,"
		   "    \"instanceRef\": \"#/deprecated2\","
		   "    \"schemaRef\": \"#/allOf/1/allOf/0/properties/deprecated2\""
		   "  }"
		   "]}")
  VALIDATE_WARNING(s, "{\"deprecated\": \"string\", \"valid\": 0, \"deprecated2\": \"string\", \"deprecated3\": \"string\"}",
		   "", "warnings", "",
		   "{ \"deprecated\": ["
		   "  {"
		   "    \"errorCode\": 43,"
		   "    \"instanceRef\": \"#/deprecated\","
		   "    \"schemaRef\": \"#/allOf/0/properties/deprecated\""
		   "  },"
		   "  {"
		   "    \"errorCode\": 43,"
		   "    \"instanceRef\": \"#/deprecated2\","
		   "    \"schemaRef\": \"#/allOf/1/allOf/0/properties/deprecated2\""
		   "  },"
		   "  {"
		   "    \"errorCode\": 43,"
		   "    \"instanceRef\": \"#/deprecated3\","
		   "    \"schemaRef\": \"#/allOf/1/allOf/1/properties/deprecated3\""
		   "  }"
		   "]}")
}

#endif // RAPIDJSON_YGGDRASIL

// Additional tests

TEST(SchemaValidator, ObjectInArray) {
    Document sd;
    sd.Parse("{\"type\":\"array\", \"items\": { \"type\":\"string\" }}");
    SchemaDocument s(sd);

    VALIDATE(s, "[\"a\"]", true);
    INVALIDATE(s, "[1]", "/items", "type", "/0",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/0\", \"schemaRef\": \"#/items\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
    INVALIDATE(s, "[{}]", "/items", "type", "/0",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/0\", \"schemaRef\": \"#/items\","
        "    \"expected\": [\"string\"], \"actual\": \"object\""
        "}}");
}

TEST(SchemaValidator, MultiTypeInObject) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\":\"object\","
        "    \"properties\": {"
        "        \"tel\" : {"
        "            \"type\":[\"integer\", \"string\"]"
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"tel\": 999 }", true);
    VALIDATE(s, "{ \"tel\": \"123-456\" }", true);
    INVALIDATE(s, "{ \"tel\": true }", "/properties/tel", "type", "/tel",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/tel\", \"schemaRef\": \"#/properties/tel\","
        "    \"expected\": [\"string\", \"integer\"], \"actual\": \"boolean\""
        "}}");
}

TEST(SchemaValidator, MultiTypeWithObject) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": [\"object\",\"string\"],"
        "    \"properties\": {"
        "        \"tel\" : {"
        "            \"type\": \"integer\""
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"Hello\"", true);
    VALIDATE(s, "{ \"tel\": 999 }", true);
    INVALIDATE(s, "{ \"tel\": \"fail\" }", "/properties/tel", "type", "/tel",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/tel\", \"schemaRef\": \"#/properties/tel\","
        "    \"expected\": [\"integer\"], \"actual\": \"string\""
        "}}");
}

TEST(SchemaValidator, AllOf_Nested) {
    Document sd;
    sd.Parse(
    "{"
    "    \"allOf\": ["
    "        { \"type\": \"string\", \"minLength\": 2 },"
    "        { \"type\": \"string\", \"maxLength\": 5 },"
    "        { \"allOf\": [ { \"enum\" : [\"ok\", \"okay\", \"OK\", \"o\"] }, { \"enum\" : [\"ok\", \"OK\", \"o\"]} ] }"
    "    ]"
    "}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"ok\"", true);
    VALIDATE(s, "\"OK\"", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"okay\"", "", "allOf", "",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "    {},{},"
        "    { \"allOf\": {"
        "      \"errors\": ["
        "        {},"
        "        { \"enum\": {\"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\", \"expected\": [\"ok\", \"OK\", \"o\"] }}"
        "      ],"
        "      \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2\""
        "    }}],"
        "    \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "}}");
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"okay\"", "", "allOf", "",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "    {},{},"
        "    { \"allOf\": {"
        "      \"errors\": ["
        "        {},"
        "        { \"enum\": {\"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\" }}"
        "      ],"
        "      \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2\""
        "    }}],"
        "    \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "}}");
#endif // RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"o\"", "", "allOf", "",
        "{ \"allOf\": {"
        "  \"errors\": ["
        "    { \"minLength\": {\"actual\": \"o\", \"expected\": 2, \"errorCode\": 7, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\" }},"
        "    {},{}"
        "  ],"
        "  \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "}}");
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"n\"", "", "allOf", "",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "      { \"minLength\": {\"actual\": \"n\", \"expected\": 2, \"errorCode\": 7, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\" }},"
        "      {},"
        "      { \"allOf\": {"
        "          \"errors\": ["
        "            { \"enum\": {\"errorCode\": 19 ,\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\", \"expected\": [\"ok\", \"okay\", \"OK\", \"o\"]}},"
        "            { \"enum\": {\"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\", \"expected\": [\"ok\", \"OK\", \"o\"]}}"
        "          ],"
        "          \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2\""
        "      }}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#\",\"schemaRef\":\"#\""
        "}}");
    INVALIDATE(s, "\"too long\"", "", "allOf", "",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "      {},"
        "      { \"maxLength\": {\"actual\": \"too long\", \"expected\": 5, \"errorCode\": 6, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/1\" }},"
        "      { \"allOf\": {"
        "          \"errors\": ["
        "            { \"enum\": {\"errorCode\": 19 ,\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\", \"expected\": [\"ok\", \"okay\", \"OK\", \"o\"] }},"
        "            { \"enum\": {\"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\", \"expected\": [\"ok\", \"OK\", \"o\"]}}"
        "          ],"
        "          \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2\""
        "      }}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#\",\"schemaRef\":\"#\""
        "}}");
    INVALIDATE(s, "123", "", "allOf", "",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "      {\"type\": {\"expected\": [\"string\"], \"actual\": \"integer\", \"errorCode\": 20, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\"}},"
        "      {\"type\": {\"expected\": [\"string\"], \"actual\": \"integer\", \"errorCode\": 20, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/1\"}},"
        "      { \"allOf\": {"
        "          \"errors\": ["
        "            { \"enum\": {\"errorCode\": 19 ,\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\", \"expected\": [\"ok\", \"okay\", \"OK\", \"o\"] }},"
        "            { \"enum\": {\"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\", \"expected\": [\"ok\", \"OK\", \"o\"] }}"
        "          ],"
        "          \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2\""
        "      }}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#\",\"schemaRef\":\"#\""
        "}}");
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE(s, "\"n\"", "", "allOf", "",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "      { \"minLength\": {\"actual\": \"n\", \"expected\": 2, \"errorCode\": 7, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\" }},"
        "      {},"
        "      { \"allOf\": {"
        "          \"errors\": ["
        "            { \"enum\": {\"errorCode\": 19 ,\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\"}},"
        "            { \"enum\": {\"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\"}}"
        "          ],"
        "          \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2\""
        "      }}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#\",\"schemaRef\":\"#\""
        "}}");
    INVALIDATE(s, "\"too long\"", "", "allOf", "",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "      {},"
        "      { \"maxLength\": {\"actual\": \"too long\", \"expected\": 5, \"errorCode\": 6, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/1\" }},"
        "      { \"allOf\": {"
        "          \"errors\": ["
        "            { \"enum\": {\"errorCode\": 19 ,\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\"}},"
        "            { \"enum\": {\"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\"}}"
        "          ],"
        "          \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2\""
        "      }}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#\",\"schemaRef\":\"#\""
        "}}");
    INVALIDATE(s, "123", "", "allOf", "",
        "{ \"allOf\": {"
        "    \"errors\": ["
        "      {\"type\": {\"expected\": [\"string\"], \"actual\": \"integer\", \"errorCode\": 20, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\"}},"
        "      {\"type\": {\"expected\": [\"string\"], \"actual\": \"integer\", \"errorCode\": 20, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/1\"}},"
        "      { \"allOf\": {"
        "          \"errors\": ["
        "            { \"enum\": {\"errorCode\": 19 ,\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\"}},"
        "            { \"enum\": {\"errorCode\": 19, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\"}}"
        "          ],"
        "          \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2\""
        "      }}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#\",\"schemaRef\":\"#\""
        "}}");
#endif // RAPIDJSON_YGGDRASIL
}

TEST(SchemaValidator, EscapedPointer) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"~/\": { \"type\": \"number\" }"
        "  }"
        "}");
    SchemaDocument s(sd);
    INVALIDATE(s, "{\"~/\":true}", "/properties/~0~1", "type", "/~0~1",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/~0~1\", \"schemaRef\": \"#/properties/~0~1\","
        "    \"expected\": [\"number\"], \"actual\": \"boolean\""
        "}}");
}

TEST(SchemaValidator, SchemaPointer) {
    Document sd;
    sd.Parse(
        "{"
        "  \"swagger\": \"2.0\","
        "  \"paths\": {"
        "    \"/some/path\": {"
        "      \"post\": {"
        "        \"parameters\": ["
        "          {"
        "            \"in\": \"body\","
        "            \"name\": \"body\","
        "            \"schema\": {"
        "              \"properties\": {"
        "                \"a\": {"
        "                  \"$ref\": \"#/definitions/Prop_a\""
        "                },"
        "                \"b\": {"
        "                  \"type\": \"integer\""
        "                }"
        "              },"
        "              \"type\": \"object\""
        "            }"
        "          }"
        "        ],"
        "        \"responses\": {"
        "          \"200\": {"
        "            \"schema\": {"
        "              \"$ref\": \"#/definitions/Resp_200\""
        "            }"
        "          }"
        "        }"
        "      }"
        "    }"
        "  },"
        "  \"definitions\": {"
        "    \"Prop_a\": {"
        "      \"properties\": {"
        "        \"c\": {"
        "          \"enum\": ["
        "            \"C1\","
        "            \"C2\","
        "            \"C3\""
        "          ],"
        "          \"type\": \"string\""
        "        },"
        "        \"d\": {"
        "          \"$ref\": \"#/definitions/Prop_d\""
        "        },"
        "        \"s\": {"
        "          \"type\": \"string\""
        "        }"
        "      },"
        "      \"required\": [\"c\"],"
        "      \"type\": \"object\""
        "    },"
        "    \"Prop_d\": {"
        "      \"properties\": {"
        "        \"a\": {"
        "          \"$ref\": \"#/definitions/Prop_a\""
        "        },"
        "        \"c\": {"
        "          \"$ref\": \"#/definitions/Prop_a/properties/c\""
        "        }"
        "      },"
        "      \"type\": \"object\""
        "    },"
        "    \"Resp_200\": {"
        "      \"properties\": {"
        "        \"e\": {"
        "          \"type\": \"string\""
        "        },"
        "        \"f\": {"
        "          \"type\": \"boolean\""
        "        }"
        "      },"
        "      \"type\": \"object\""
        "    }"
        "  }"
        "}");
    SchemaDocument s1(sd, NULL, 0, NULL, NULL, Pointer("#/paths/~1some~1path/post/parameters/0/schema"));
    VALIDATE(s1,
        "{"
        "  \"a\": {"
        "    \"c\": \"C1\","
        "    \"d\": {"
        "      \"a\": {"
        "        \"c\": \"C2\""
        "      },"
        "      \"c\": \"C3\""
        "    }"
        "  },"
        "  \"b\": 123"
        "}",
         true);
    INVALIDATE(s1,
        "{"
        "  \"a\": {"
        "    \"c\": \"C1\","
        "    \"d\": {"
        "      \"a\": {"
        "        \"c\": \"C2\""
        "      },"
        "      \"c\": \"C3\""
        "    }"
        "  },"
        "  \"b\": \"should be an int\""
        "}",
        "#/paths/~1some~1path/post/parameters/0/schema/properties/b", "type", "#/b",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\":\"#/b\","
        "    \"schemaRef\":\"#/paths/~1some~1path/post/parameters/0/schema/properties/b\","
        "    \"expected\": [\"integer\"], \"actual\":\"string\""
        "}}");
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE(s1,
        "{"
        "  \"a\": {"
        "    \"c\": \"C1\","
        "    \"d\": {"
        "      \"a\": {"
        "        \"c\": \"should be within enum\""
        "      },"
        "      \"c\": \"C3\""
        "    }"
        "  },"
        "  \"b\": 123"
        "}",
        "#/definitions/Prop_a/properties/c", "enum", "#/a/d/a/c",
        "{ \"enum\": {"
        "    \"errorCode\": 19,"
        "    \"instanceRef\":\"#/a/d/a/c\","
        "    \"schemaRef\":\"#/definitions/Prop_a/properties/c\""
	"    , \"expected\": [\"C1\", \"C2\", \"C3\"]"
        "}}");
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE(s1,
        "{"
        "  \"a\": {"
        "    \"c\": \"C1\","
        "    \"d\": {"
        "      \"a\": {"
        "        \"c\": \"should be within enum\""
        "      },"
        "      \"c\": \"C3\""
        "    }"
        "  },"
        "  \"b\": 123"
        "}",
        "#/definitions/Prop_a/properties/c", "enum", "#/a/d/a/c",
        "{ \"enum\": {"
        "    \"errorCode\": 19,"
        "    \"instanceRef\":\"#/a/d/a/c\","
        "    \"schemaRef\":\"#/definitions/Prop_a/properties/c\""
        "}}");
#endif // RAPIDJSON_YGGDRASIL
    INVALIDATE(s1,
        "{"
        "  \"a\": {"
        "    \"c\": \"C1\","
        "    \"d\": {"
        "      \"a\": {"
        "        \"s\": \"required 'c' is missing\""
        "      }"
        "    }"
        "  },"
        "  \"b\": 123"
        "}",
        "#/definitions/Prop_a", "required", "#/a/d/a",
        "{ \"required\": {"
        "    \"errorCode\": 15,"
        "    \"missing\":[\"c\"],"
        "    \"instanceRef\":\"#/a/d/a\","
        "    \"schemaRef\":\"#/definitions/Prop_a\""
        "}}");
    SchemaDocument s2(sd, NULL, 0, NULL, NULL, Pointer("#/paths/~1some~1path/post/responses/200/schema"));
    VALIDATE(s2,
        "{ \"e\": \"some string\", \"f\": false }",
        true);
    INVALIDATE(s2,
        "{ \"e\": true, \"f\": false }",
        "#/definitions/Resp_200/properties/e", "type", "#/e",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\":\"#/e\","
        "    \"schemaRef\":\"#/definitions/Resp_200/properties/e\","
        "    \"expected\": [\"string\"], \"actual\":\"boolean\""
        "}}");
    INVALIDATE(s2,
        "{ \"e\": \"some string\", \"f\": 123 }",
        "#/definitions/Resp_200/properties/f", "type", "#/f",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\":\"#/f\","
        "    \"schemaRef\":\"#/definitions/Resp_200/properties/f\","
        "    \"expected\": [\"boolean\"], \"actual\":\"integer\""
        "}}");
}

template <typename Allocator>
static char* ReadFile(const char* filename, Allocator& allocator) {
    const char *paths[] = {
        "",
        "bin/",
        "../bin/",
        "../../bin/",
        "../../../bin/"
    };
    char buffer[1024];
    FILE *fp = 0;
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        snprintf(buffer, 1024, "%s%s", paths[i], filename);
        fp = fopen(buffer, "rb");
        if (fp)
            break;
    }

    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    size_t length = static_cast<size_t>(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    char* json = reinterpret_cast<char*>(allocator.Malloc(length + 1));
    size_t readLength = fread(json, 1, length, fp);
    json[readLength] = '\0';
    fclose(fp);
    return json;
}

TEST(SchemaValidator, ValidateMetaSchema) {
    CrtAllocator allocator;
    char* json = ReadFile("draft-04/schema", allocator);
    Document d;
    d.Parse(json);
    ASSERT_FALSE(d.HasParseError());
    SchemaDocument sd(d);
    SchemaValidator validator(sd);
    d.Accept(validator);
    if (!validator.IsValid()) {
        StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        printf("Invalid schema: %s\n", sb.GetString());
        printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        printf("Invalid code: %d\n", validator.GetInvalidSchemaCode());
        printf("Invalid message: %s\n", GetValidateError_En(validator.GetInvalidSchemaCode()));
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        printf("Invalid document: %s\n", sb.GetString());
        sb.Clear();
        Writer<StringBuffer> w(sb);
        validator.GetError().Accept(w);
        printf("Validation error: %s\n", sb.GetString());
        ADD_FAILURE();
    }
    CrtAllocator::Free(json);
}

TEST(SchemaValidator, ValidateMetaSchema_UTF16) {
    typedef GenericDocument<UTF16<> > D;
    typedef GenericSchemaDocument<D::ValueType> SD;
    typedef GenericSchemaValidator<SD> SV;

    CrtAllocator allocator;
    char* json = ReadFile("draft-04/schema", allocator);

    D d;
    StringStream ss(json);
    d.ParseStream<0, UTF8<> >(ss);
    ASSERT_FALSE(d.HasParseError());
    SD sd(d);
    SV validator(sd);
    d.Accept(validator);
    if (!validator.IsValid()) {
        GenericStringBuffer<UTF16<> > sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        wprintf(L"Invalid schema: %ls\n", sb.GetString());
        wprintf(L"Invalid keyword: %ls\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        wprintf(L"Invalid document: %ls\n", sb.GetString());
        sb.Clear();
        Writer<GenericStringBuffer<UTF16<> >, UTF16<> > w(sb);
        validator.GetError().Accept(w);
        printf("Validation error: %ls\n", sb.GetString());
        ADD_FAILURE();
    }
    CrtAllocator::Free(json);
}

template <typename SchemaDocumentType = SchemaDocument>
class RemoteSchemaDocumentProvider : public IGenericRemoteSchemaDocumentProvider<SchemaDocumentType> {
public:
    RemoteSchemaDocumentProvider() : 
        documentAllocator_(documentBuffer_, sizeof(documentBuffer_)), 
        schemaAllocator_(schemaBuffer_, sizeof(schemaBuffer_)) 
    {
        const char* filenames[kCount] = {
            "jsonschema/remotes/integer.json",
            "jsonschema/remotes/subSchemas.json",
            "jsonschema/remotes/folder/folderInteger.json",
            "draft-04/schema",
            "unittestschema/address.json"
        };
        const char* uris[kCount] = {
            "http://localhost:1234/integer.json",
            "http://localhost:1234/subSchemas.json",
            "http://localhost:1234/folder/folderInteger.json",
            "http://json-schema.org/draft-04/schema",
            "http://localhost:1234/address.json"
        };

        for (size_t i = 0; i < kCount; i++) {
            sd_[i] = 0;

            char jsonBuffer[8192];
            MemoryPoolAllocator<> jsonAllocator(jsonBuffer, sizeof(jsonBuffer));
            char* json = ReadFile(filenames[i], jsonAllocator);
            if (!json) {
                printf("json remote file %s not found", filenames[i]);
                ADD_FAILURE();
            }
            else {
                char stackBuffer[4096];
                MemoryPoolAllocator<> stackAllocator(stackBuffer, sizeof(stackBuffer));
                DocumentType d(&documentAllocator_, 1024, &stackAllocator);
                d.Parse(json);
                sd_[i] = new SchemaDocumentType(d, uris[i], static_cast<SizeType>(strlen(uris[i])), 0, &schemaAllocator_);
                MemoryPoolAllocator<>::Free(json);
            }
        };
    }

    ~RemoteSchemaDocumentProvider() {
        for (size_t i = 0; i < kCount; i++)
            delete sd_[i];
    }

    virtual const SchemaDocumentType* GetRemoteDocument(const char* uri, SizeType length) {
        //printf("GetRemoteDocument : %s\n", uri);
        for (size_t i = 0; i < kCount; i++)
            if (typename SchemaDocumentType::GValue(uri, length) == sd_[i]->GetURI()) {
                //printf("Matched document");
                return sd_[i];
            }
        //printf("No matched document");
        return 0;
    }

private:
    typedef GenericDocument<typename SchemaDocumentType::EncodingType, MemoryPoolAllocator<>, MemoryPoolAllocator<> > DocumentType;

    RemoteSchemaDocumentProvider(const RemoteSchemaDocumentProvider&);
    RemoteSchemaDocumentProvider& operator=(const RemoteSchemaDocumentProvider&);

    static const size_t kCount = 5;
    SchemaDocumentType* sd_[kCount];
    typename DocumentType::AllocatorType documentAllocator_;
    typename SchemaDocumentType::AllocatorType schemaAllocator_;
    char documentBuffer_[16384];
    char schemaBuffer_[128u * 1024];
};

TEST(SchemaValidator, TestSuite) {
    const char* filenames[] = {
        "additionalItems.json",
        "additionalProperties.json",
        "allOf.json",
        "anyOf.json",
        "default.json",
        "definitions.json",
        "dependencies.json",
        "enum.json",
        "items.json",
        "maximum.json",
        "maxItems.json",
        "maxLength.json",
        "maxProperties.json",
        "minimum.json",
        "minItems.json",
        "minLength.json",
        "minProperties.json",
        "multipleOf.json",
        "not.json",
        "oneOf.json",
        "pattern.json",
        "patternProperties.json",
        "properties.json",
        "ref.json",
        "refRemote.json",
        "required.json",
        "type.json",
        "uniqueItems.json"
    };

    const char* onlyRunDescription = 0;
    //const char* onlyRunDescription = "a string is a string";

    unsigned testCount = 0;
    unsigned passCount = 0;

    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;

    char jsonBuffer[65536];
    char documentBuffer[65536];
    char documentStackBuffer[65536];
    char schemaBuffer[65536];
    char validatorBuffer[65536];
    MemoryPoolAllocator<> jsonAllocator(jsonBuffer, sizeof(jsonBuffer));
    MemoryPoolAllocator<> documentAllocator(documentBuffer, sizeof(documentBuffer));
    MemoryPoolAllocator<> documentStackAllocator(documentStackBuffer, sizeof(documentStackBuffer));
    MemoryPoolAllocator<> schemaAllocator(schemaBuffer, sizeof(schemaBuffer));
    MemoryPoolAllocator<> validatorAllocator(validatorBuffer, sizeof(validatorBuffer));

    for (size_t i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
        char filename[FILENAME_MAX];
        snprintf(filename, FILENAME_MAX, "jsonschema/tests/draft4/%s", filenames[i]);
        char* json = ReadFile(filename, jsonAllocator);
        if (!json) {
            printf("json test suite file %s not found", filename);
            ADD_FAILURE();
        }
        else {
            //printf("\njson test suite file %s parsed ok\n", filename);
            GenericDocument<UTF8<>, MemoryPoolAllocator<>, MemoryPoolAllocator<> > d(&documentAllocator, 1024, &documentStackAllocator);
            d.Parse(json);
            if (d.HasParseError()) {
                printf("json test suite file %s has parse error", filename);
                ADD_FAILURE();
            }
            else {
                for (Value::ConstValueIterator schemaItr = d.Begin(); schemaItr != d.End(); ++schemaItr) {
                    {
                        const char* description1 = (*schemaItr)["description"].GetString();
                        //printf("\ncompiling schema for json test %s \n", description1);
                        SchemaDocumentType schema((*schemaItr)["schema"], filenames[i], static_cast<SizeType>(strlen(filenames[i])), &provider, &schemaAllocator);
                        GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > validator(schema, &validatorAllocator);
                        const Value& tests = (*schemaItr)["tests"];
                        for (Value::ConstValueIterator testItr = tests.Begin(); testItr != tests.End(); ++testItr) {
                            const char* description2 = (*testItr)["description"].GetString();
                            //printf("running json test %s \n", description2);
                            if (!onlyRunDescription || strcmp(description2, onlyRunDescription) == 0) {
                                const Value& data = (*testItr)["data"];
                                bool expected = (*testItr)["valid"].GetBool();
                                testCount++;
                                validator.Reset();
                                data.Accept(validator);
                                bool actual = validator.IsValid();
                                if (expected != actual)
                                    printf("Fail: %30s \"%s\" \"%s\"\n", filename, description1, description2);
                                else {
                                    //printf("Passed: %30s \"%s\" \"%s\"\n", filename, description1, description2);
                                    passCount++;
                                }
                            }
                        }
                        //printf("%zu %zu %zu\n", documentAllocator.Size(), schemaAllocator.Size(), validatorAllocator.Size());
                    }
                    schemaAllocator.Clear();
                    validatorAllocator.Clear();
                }
            }
        }
        documentAllocator.Clear();
        MemoryPoolAllocator<>::Free(json);
        jsonAllocator.Clear();
    }
    printf("%u / %u passed (%2u%%)\n", passCount, testCount, passCount * 100 / testCount);
    if (passCount != testCount)
        ADD_FAILURE();
}

TEST(SchemaValidatingReader, Simple) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\" : [\"red\", \"amber\", \"green\"] }");
    SchemaDocument s(sd);

    Document d;
    StringStream ss("\"red\"");
    SchemaValidatingReader<kParseDefaultFlags, StringStream, UTF8<> > reader(ss, s);
    d.Populate(reader);
    EXPECT_TRUE(reader.GetParseResult());
    EXPECT_TRUE(reader.IsValid());
    EXPECT_TRUE(d.IsString());
    EXPECT_STREQ("red", d.GetString());
}

TEST(SchemaValidatingReader, Invalid) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"minLength\":2,\"maxLength\":3}");
    SchemaDocument s(sd);

    Document d;
    StringStream ss("\"ABCD\"");
    SchemaValidatingReader<kParseDefaultFlags, StringStream, UTF8<> > reader(ss, s);
    d.Populate(reader);
    EXPECT_FALSE(reader.GetParseResult());
    EXPECT_FALSE(reader.IsValid());
    EXPECT_EQ(kParseErrorTermination, reader.GetParseResult().Code());
    EXPECT_STREQ("maxLength", reader.GetInvalidSchemaKeyword());
    EXPECT_TRUE(reader.GetInvalidSchemaCode() == kValidateErrorMaxLength);
    EXPECT_TRUE(reader.GetInvalidSchemaPointer() == SchemaDocument::PointerType(""));
    EXPECT_TRUE(reader.GetInvalidDocumentPointer() == SchemaDocument::PointerType(""));
    EXPECT_TRUE(d.IsNull());
    Document e;
    e.Parse(
        "{ \"maxLength\": {"
        "     \"errorCode\": 6,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 3, \"actual\": \"ABCD\""
        "}}");
    if (e != reader.GetError()) {
        ADD_FAILURE();
    }
}

TEST(SchemaValidatingWriter, Simple) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"minLength\":2,\"maxLength\":3}");
    SchemaDocument s(sd);

    Document d;
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    GenericSchemaValidator<SchemaDocument, Writer<StringBuffer> > validator(s, writer);

    d.Parse("\"red\"");
    EXPECT_TRUE(d.Accept(validator));
    EXPECT_TRUE(validator.IsValid());
    EXPECT_STREQ("\"red\"", sb.GetString());

    sb.Clear();
    validator.Reset();
    d.Parse("\"ABCD\"");
    EXPECT_FALSE(d.Accept(validator));
    EXPECT_FALSE(validator.IsValid());
    EXPECT_TRUE(validator.GetInvalidSchemaPointer() == SchemaDocument::PointerType(""));
    EXPECT_TRUE(validator.GetInvalidDocumentPointer() == SchemaDocument::PointerType(""));
    EXPECT_TRUE(validator.GetInvalidSchemaCode() == kValidateErrorMaxLength);
    Document e;
    e.Parse(
        "{ \"maxLength\": {"
"            \"errorCode\": 6,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 3, \"actual\": \"ABCD\""
        "}}");
    EXPECT_EQ(e, validator.GetError());
}

TEST(Schema, Issue848) {
    rapidjson::Document d;
    rapidjson::SchemaDocument s(d);
    rapidjson::GenericSchemaValidator<rapidjson::SchemaDocument, rapidjson::Document> v(s);
}

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS

static SchemaDocument ReturnSchemaDocument() {
    Document sd;
    sd.Parse("{ \"type\": [\"number\", \"string\"] }");
    SchemaDocument s(sd);
    return s;
}

TEST(Schema, Issue552) {
    SchemaDocument s = ReturnSchemaDocument();
    VALIDATE(s, "42", true);
    VALIDATE(s, "\"Life, the universe, and everything\"", true);
    INVALIDATE(s, "[\"Life\", \"the universe\", \"and everything\"]", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\", \"number\"], \"actual\": \"array\""
        "}}");
}

#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS

TEST(SchemaValidator, Issue608) {
    Document sd;
    sd.Parse("{\"required\": [\"a\", \"b\"] }");
    SchemaDocument s(sd);

    VALIDATE(s, "{\"a\" : null, \"b\": null}", true);
    INVALIDATE(s, "{\"a\" : null, \"a\" : null}", "", "required", "",
        "{ \"required\": {"
        "    \"errorCode\": 15,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"b\"]"
        "}}");
}

// Fail to resolve $ref in allOf causes crash in SchemaValidator::StartObject()
TEST(SchemaValidator, Issue728_AllOfRef) {
    Document sd;
    sd.Parse("{\"allOf\": [{\"$ref\": \"#/abc\"}]}");
    SchemaDocument s(sd);
    SCHEMAERROR(s, "{\"RefUnknown\":{\"errorCode\":5,\"instanceRef\":\"#/allOf/0\",\"value\":\"#/abc\"}}");

    VALIDATE_(s, "{\"key1\": \"abc\", \"key2\": \"def\"}", true, false);
}

TEST(SchemaValidator, Issue1017_allOfHandler) {
    Document sd;
    sd.Parse("{\"allOf\": [{\"type\": \"object\",\"properties\": {\"cyanArray2\": {\"type\": \"array\",\"items\": { \"type\": \"string\" }}}},{\"type\": \"object\",\"properties\": {\"blackArray\": {\"type\": \"array\",\"items\": { \"type\": \"string\" }}},\"required\": [ \"blackArray\" ]}]}");
    SchemaDocument s(sd);
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    GenericSchemaValidator<SchemaDocument, Writer<StringBuffer> > validator(s, writer);
    EXPECT_TRUE(validator.StartObject());
    EXPECT_TRUE(validator.Key("cyanArray2", 10, false));
    EXPECT_TRUE(validator.StartArray());    
    EXPECT_TRUE(validator.EndArray(0));    
    EXPECT_TRUE(validator.Key("blackArray", 10, false));
    EXPECT_TRUE(validator.StartArray());    
    EXPECT_TRUE(validator.EndArray(0));    
    EXPECT_TRUE(validator.EndObject(0));
    EXPECT_TRUE(validator.IsValid());
    EXPECT_STREQ("{\"cyanArray2\":[],\"blackArray\":[]}", sb.GetString());
}

TEST(SchemaValidator, Ref_remote) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"$ref\": \"http://localhost:1234/subSchemas.json#/integer\"}");
    SchemaDocumentType s(sd, 0, 0, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "null", "/integer", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\","
        "    \"schemaRef\": \"http://localhost:1234/subSchemas.json#/integer\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// Merge with id where $ref is full URI
TEST(SchemaValidator, Ref_remote_change_resolution_scope_uri) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"id\": \"http://ignore/blah#/ref\", \"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"http://localhost:1234/subSchemas.json#/integer\"}}}");
    SchemaDocumentType s(sd, 0, 0, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"myInt\": null}", "/integer", "type", "/myInt",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/myInt\","
        "    \"schemaRef\": \"http://localhost:1234/subSchemas.json#/integer\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// Merge with id where $ref is a relative path
TEST(SchemaValidator, Ref_remote_change_resolution_scope_relative_path) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"id\": \"http://localhost:1234/\", \"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"subSchemas.json#/integer\"}}}");
    SchemaDocumentType s(sd, 0, 0, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"myInt\": null}", "/integer", "type", "/myInt",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/myInt\","
        "    \"schemaRef\": \"http://localhost:1234/subSchemas.json#/integer\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// Merge with id where $ref is an absolute path
TEST(SchemaValidator, Ref_remote_change_resolution_scope_absolute_path) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"id\": \"http://localhost:1234/xxxx\", \"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"/subSchemas.json#/integer\"}}}");
    SchemaDocumentType s(sd, 0, 0, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"myInt\": null}", "/integer", "type", "/myInt",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/myInt\","
        "    \"schemaRef\": \"http://localhost:1234/subSchemas.json#/integer\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// Merge with id where $ref is an absolute path, and the document has a base URI
TEST(SchemaValidator, Ref_remote_change_resolution_scope_absolute_path_document) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"/subSchemas.json#/integer\"}}}");
    SchemaDocumentType s(sd, "http://localhost:1234/xxxx", 26, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"myInt\": null}", "/integer", "type", "/myInt",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/myInt\","
        "    \"schemaRef\": \"http://localhost:1234/subSchemas.json#/integer\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// $ref is a non-JSON pointer fragment and there a matching id
TEST(SchemaValidator, Ref_internal_id_1) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt1\": {\"$ref\": \"#myId\"}, \"myStr\": {\"type\": \"string\", \"id\": \"#myStrId\"}, \"myInt2\": {\"type\": \"integer\", \"id\": \"#myId\"}}}");
    SchemaDocumentType s(sd);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"myInt1\": null}", "/properties/myInt2", "type", "/myInt1",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/myInt1\","
        "    \"schemaRef\": \"#/properties/myInt2\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// $ref is a non-JSON pointer fragment and there are two matching ids so we take the first
TEST(SchemaValidator, Ref_internal_id_2) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt1\": {\"$ref\": \"#myId\"}, \"myInt2\": {\"type\": \"integer\", \"id\": \"#myId\"}, \"myStr\": {\"type\": \"string\", \"id\": \"#myId\"}}}");
    SchemaDocumentType s(sd);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"myInt1\": null}", "/properties/myInt2", "type", "/myInt1",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/myInt1\","
        "    \"schemaRef\": \"#/properties/myInt2\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// $ref is a non-JSON pointer fragment and there is a matching id within array
TEST(SchemaValidator, Ref_internal_id_in_array) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt1\": {\"$ref\": \"#myId\"}, \"myInt2\": {\"anyOf\": [{\"type\": \"string\", \"id\": \"#myStrId\"}, {\"type\": \"integer\", \"id\": \"#myId\"}]}}}");
    SchemaDocumentType s(sd);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"myInt1\": null}", "/properties/myInt2/anyOf/1", "type", "/myInt1",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/myInt1\","
        "    \"schemaRef\": \"#/properties/myInt2/anyOf/1\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// $ref is a non-JSON pointer fragment and there is a matching id, and the schema is embedded in the document
TEST(SchemaValidator, Ref_internal_id_and_schema_pointer) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{ \"schema\": {\"type\": \"object\", \"properties\": {\"myInt1\": {\"$ref\": \"#myId\"}, \"myInt2\": {\"anyOf\": [{\"type\": \"integer\", \"id\": \"#myId\"}]}}}}");
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    SchemaDocumentType s(sd, 0, 0, 0, 0, PointerType("/schema"));
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    INVALIDATE_(s, "{\"myInt1\": null}", "/schema/properties/myInt2/anyOf/0", "type", "/myInt1",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#/myInt1\","
        "    \"schemaRef\": \"#/schema/properties/myInt2/anyOf/0\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        kValidateDefaultFlags, SchemaValidatorType, PointerType);
}

// Test that $refs are correctly resolved when intermediate multiple ids are present
// Includes $ref to a part of the document with a different in-scope id, which also contains $ref..
TEST(SchemaValidator, Ref_internal_multiple_ids) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    //RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/idandref.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocumentType s(sd, "http://xyz", 10/*, &provider*/);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"PA1\": \"s\", \"PA2\": \"t\", \"PA3\": \"r\", \"PX1\": 1, \"PX2Y\": 2, \"PX3Z\": 3, \"PX4\": 4, \"PX5\": 5, \"PX6\": 6, \"PX7W\": 7, \"PX8N\": { \"NX\": 8}}", "#", "errors", "#",
        "{ \"type\": ["
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PA1\", \"schemaRef\": \"http://xyz#/definitions/A\", \"expected\": [\"integer\"], \"actual\": \"string\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PA2\", \"schemaRef\": \"http://xyz#/definitions/A\", \"expected\": [\"integer\"], \"actual\": \"string\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PA3\", \"schemaRef\": \"http://xyz#/definitions/A\", \"expected\": [\"integer\"], \"actual\": \"string\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PX1\", \"schemaRef\": \"http://xyz#/definitions/B/definitions/X\", \"expected\": [\"boolean\"], \"actual\": \"integer\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PX2Y\", \"schemaRef\": \"http://xyz#/definitions/B/definitions/X\", \"expected\": [\"boolean\"], \"actual\": \"integer\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PX3Z\", \"schemaRef\": \"http://xyz#/definitions/B/definitions/X\", \"expected\": [\"boolean\"], \"actual\": \"integer\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PX4\", \"schemaRef\": \"http://xyz#/definitions/B/definitions/X\", \"expected\": [\"boolean\"], \"actual\": \"integer\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PX5\", \"schemaRef\": \"http://xyz#/definitions/B/definitions/X\", \"expected\": [\"boolean\"], \"actual\": \"integer\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PX6\", \"schemaRef\": \"http://xyz#/definitions/B/definitions/X\", \"expected\": [\"boolean\"], \"actual\": \"integer\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PX7W\", \"schemaRef\": \"http://xyz#/definitions/B/definitions/X\", \"expected\": [\"boolean\"], \"actual\": \"integer\"},"
        "    {\"errorCode\": 20, \"instanceRef\": \"#/PX8N/NX\", \"schemaRef\": \"http://xyz#/definitions/B/definitions/X\", \"expected\": [\"boolean\"], \"actual\": \"integer\"}"
        "]}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidatorType, PointerType);
    CrtAllocator::Free(schema);
}

TEST(SchemaValidator, Ref_remote_issue1210) {
    class SchemaDocumentProvider : public IRemoteSchemaDocumentProvider {
        SchemaDocument** collection;

        // Dummy private copy constructor & assignment operator.
        // Function bodies added so that they compile in MSVC 2019.
        SchemaDocumentProvider(const SchemaDocumentProvider&) : collection(NULL) {
        }
        SchemaDocumentProvider& operator=(const SchemaDocumentProvider&) {
            return *this;
        }

        public:
          SchemaDocumentProvider(SchemaDocument** collection) : collection(collection) { }
          virtual const SchemaDocument* GetRemoteDocument(const char* uri, SizeType length) {
            int i = 0;
            while (collection[i] && SchemaDocument::GValue(uri, length) != collection[i]->GetURI()) ++i;
            return collection[i];
          }
    };
    SchemaDocument* collection[] = { 0, 0, 0 };
    SchemaDocumentProvider provider(collection);

    Document x, y, z;
    x.Parse("{\"properties\":{\"country\":{\"$ref\":\"y.json#/definitions/country_remote\"}},\"type\":\"object\"}");
    y.Parse("{\"definitions\":{\"country_remote\":{\"$ref\":\"z.json#/definitions/country_list\"}}}");
    z.Parse("{\"definitions\":{\"country_list\":{\"enum\":[\"US\"]}}}");

    SchemaDocument sz(z, "z.json", 6, &provider);
    collection[0] = &sz;
    SchemaDocument sy(y, "y.json", 6, &provider);
    collection[1] = &sy;
    SchemaDocument sx(x, "x.json", 6, &provider);

    VALIDATE(sx, "{\"country\":\"UK\"}", false);
    VALIDATE(sx, "{\"country\":\"US\"}", true);
}

// Test that when kValidateContinueOnErrorFlag is set, all errors are reported.
TEST(SchemaValidator, ContinueOnErrors) {
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    VALIDATE(s, "{\"version\": 1.0, \"address\": {\"number\": 24, \"street1\": \"The Woodlands\", \"street3\": \"Ham\", \"city\": \"Romsey\", \"area\": \"Kent\", \"country\": \"UK\", \"postcode\": \"SO51 0GP\"}, \"phones\": [\"0111-222333\", \"0777-666888\"], \"names\": [\"Fred\", \"Bloggs\"]}", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{\"version\": 1.01, \"address\": {\"number\": 0, \"street2\": false,  \"street3\": \"Ham\", \"city\": \"RomseyTownFC\", \"area\": \"BC\", \"country\": \"USA\", \"postcode\": \"999ABC\"}, \"phones\": [], \"planet\": \"Earth\", \"extra\": {\"S_xxx\": 123}}", "#", "errors", "#",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1, \"instanceRef\": \"#/version\", \"schemaRef\": \"#/definitions/decimal_type\", \"expected\": 1.0, \"actual\": 1.01"
        "  },"
        "  \"minimum\": {"
        "    \"errorCode\": 5, \"instanceRef\": \"#/address/number\", \"schemaRef\": \"#/definitions/positiveInt_type\", \"expected\": 0, \"actual\": 0, \"exclusiveMinimum\": true"
        "  },"
        "  \"type\": ["
        "    {\"expected\": [\"null\", \"string\"], \"actual\": \"boolean\", \"errorCode\": 20, \"instanceRef\": \"#/address/street2\", \"schemaRef\": \"#/definitions/address_type/properties/street2\"},"
        "    {\"expected\": [\"string\"], \"actual\": \"integer\", \"errorCode\": 20, \"instanceRef\": \"#/extra/S_xxx\", \"schemaRef\": \"#/properties/extra/patternProperties/%5ES_\"}"
        "  ],"
        "  \"maxLength\": {"
        "    \"actual\": \"RomseyTownFC\", \"expected\": 10, \"errorCode\": 6, \"instanceRef\": \"#/address/city\", \"schemaRef\": \"#/definitions/address_type/properties/city\""
        "  },"
        "  \"anyOf\": {"
        "    \"errors\":["
        "      {\"pattern\": {\"actual\": \"999ABC\", \"expected\": \"^[A-Z]{2}[0-9]{1,2} [0-9][A-Z]{2}$\", \"errorCode\": 8, \"instanceRef\": \"#/address/postcode\", \"schemaRef\": \"#/definitions/address_type/properties/postcode/anyOf/0\"}},"
        "      {\"pattern\": {\"actual\": \"999ABC\", \"expected\": \"^[0-9]{5}$\", \"errorCode\": 8, \"instanceRef\": \"#/address/postcode\", \"schemaRef\": \"#/definitions/address_type/properties/postcode/anyOf/1\"}}"
        "    ],"
        "    \"errorCode\": 24, \"instanceRef\": \"#/address/postcode\", \"schemaRef\": \"#/definitions/address_type/properties/postcode\""
        "  },"
        "  \"allOf\": {"
        "    \"errors\":["
        "      {\"enum\":{\"errorCode\":19,\"instanceRef\":\"#/address/country\",\"schemaRef\":\"#/definitions/country_type\", \"expected\": [\"UK\", \"Canada\"]}}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#/address/country\",\"schemaRef\":\"#/definitions/address_type/properties/country\""
        "  },"
        "  \"minItems\": {"
        "    \"actual\": 0, \"expected\": 1, \"errorCode\": 10, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\""
        "  },"
        "  \"additionalProperties\": {"
        "    \"disallowed\": \"planet\", \"errorCode\": 16, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "  },"
        "  \"required\": {"
        "    \"missing\": [\"street1\"], \"errorCode\": 15, \"instanceRef\": \"#/address\", \"schemaRef\": \"#/definitions/address_type\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
    INVALIDATE_(s, "{\"address\": {\"number\": 200, \"street1\": {}, \"street3\": null, \"city\": \"Rom\", \"area\": \"Dorset\", \"postcode\": \"SO51 0GP\"}, \"phones\": [\"0111-222333\", \"0777-666888\", \"0777-666888\"], \"names\": [\"Fred\", \"S\", \"M\", \"Bloggs\"]}", "#", "errors", "#",
        "{ \"maximum\": {"
        "    \"errorCode\": 3, \"instanceRef\": \"#/address/number\", \"schemaRef\": \"#/definitions/positiveInt_type\", \"expected\": 100, \"actual\": 200, \"exclusiveMaximum\": true"
        "  },"
        "  \"type\": {"
        "    \"expected\": [\"string\"], \"actual\": \"object\", \"errorCode\": 20, \"instanceRef\": \"#/address/street1\", \"schemaRef\": \"#/definitions/address_type/properties/street1\""
        "  },"
        "  \"not\": {"
        "    \"errorCode\": 25, \"instanceRef\": \"#/address/street3\", \"schemaRef\": \"#/definitions/address_type/properties/street3\""
        "  },"
        "  \"minLength\": {"
        "    \"actual\": \"Rom\", \"expected\": 4, \"errorCode\": 7, \"instanceRef\": \"#/address/city\", \"schemaRef\": \"#/definitions/address_type/properties/city\""
        "  },"
        "  \"maxItems\": {"
        "    \"actual\": 3, \"expected\": 2, \"errorCode\": 9, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\""
        "  },"
        "  \"uniqueItems\": {"
        "    \"duplicates\": [1, 2], \"errorCode\": 11, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\""
        "  },"
        "  \"minProperties\": {\"actual\": 6, \"expected\": 7, \"errorCode\": 14, \"instanceRef\": \"#/address\", \"schemaRef\": \"#/definitions/address_type\""
        "  },"
        "  \"additionalItems\": ["
        "    {\"disallowed\": 2, \"errorCode\": 12, \"instanceRef\": \"#/names\", \"schemaRef\": \"#/properties/names\"},"
        "    {\"disallowed\": 3, \"errorCode\": 12, \"instanceRef\": \"#/names\", \"schemaRef\": \"#/properties/names\"}"
        "  ],"
        "  \"dependencies\": {"
        "    \"errors\": {"
        "      \"address\": {\"required\": {\"missing\": [\"version\"], \"errorCode\": 15, \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/address\"}},"
        "      \"names\": {\"required\": {\"missing\": [\"version\"], \"errorCode\": 15, \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/names\"}}"
        "    },"
        "    \"errorCode\": 18, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "  },"
        "  \"oneOf\": {"
        "    \"errors\": ["
        "      {\"enum\": {\"errorCode\": 19, \"instanceRef\": \"#/address/area\", \"schemaRef\": \"#/definitions/county_type\", \"expected\": [\"Sussex\", \"Surrey\", \"Kent\", \"Narnia\"] }},"
        "      {\"enum\": {\"errorCode\": 19, \"instanceRef\": \"#/address/area\", \"schemaRef\": \"#/definitions/province_type\", \"expected\": [\"Quebec\", \"Narnia\", \"BC\", \"Alberta\"] }}"
        "    ],"
        "    \"errorCode\": 21, \"instanceRef\": \"#/address/area\", \"schemaRef\": \"#/definitions/address_type/properties/area\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{\"version\": 1.01, \"address\": {\"number\": 0, \"street2\": false,  \"street3\": \"Ham\", \"city\": \"RomseyTownFC\", \"area\": \"Narnia\", \"country\": \"USA\", \"postcode\": \"999ABC\"}, \"phones\": [], \"planet\": \"Earth\", \"extra\": {\"S_xxx\": 123}}", "#", "errors", "#",
        "{ \"multipleOf\": {"
        "    \"errorCode\": 1, \"instanceRef\": \"#/version\", \"schemaRef\": \"#/definitions/decimal_type\", \"expected\": 1.0, \"actual\": 1.01"
        "  },"
        "  \"minimum\": {"
        "    \"errorCode\": 5, \"instanceRef\": \"#/address/number\", \"schemaRef\": \"#/definitions/positiveInt_type\", \"expected\": 0, \"actual\": 0, \"exclusiveMinimum\": true"
        "  },"
        "  \"type\": ["
        "    {\"expected\": [\"null\", \"string\"], \"actual\": \"boolean\", \"errorCode\": 20, \"instanceRef\": \"#/address/street2\", \"schemaRef\": \"#/definitions/address_type/properties/street2\"},"
        "    {\"expected\": [\"string\"], \"actual\": \"integer\", \"errorCode\": 20, \"instanceRef\": \"#/extra/S_xxx\", \"schemaRef\": \"#/properties/extra/patternProperties/%5ES_\"}"
        "  ],"
        "  \"maxLength\": {"
        "    \"actual\": \"RomseyTownFC\", \"expected\": 10, \"errorCode\": 6, \"instanceRef\": \"#/address/city\", \"schemaRef\": \"#/definitions/address_type/properties/city\""
        "  },"
        "  \"anyOf\": {"
        "    \"errors\":["
        "      {\"pattern\": {\"actual\": \"999ABC\", \"errorCode\": 8, \"instanceRef\": \"#/address/postcode\", \"schemaRef\": \"#/definitions/address_type/properties/postcode/anyOf/0\"}},"
        "      {\"pattern\": {\"actual\": \"999ABC\", \"errorCode\": 8, \"instanceRef\": \"#/address/postcode\", \"schemaRef\": \"#/definitions/address_type/properties/postcode/anyOf/1\"}}"
        "    ],"
        "    \"errorCode\": 24, \"instanceRef\": \"#/address/postcode\", \"schemaRef\": \"#/definitions/address_type/properties/postcode\""
        "  },"
        "  \"allOf\": {"
        "    \"errors\":["
        "      {\"enum\":{\"errorCode\":19,\"instanceRef\":\"#/address/country\",\"schemaRef\":\"#/definitions/country_type\"}}"
        "    ],"
        "    \"errorCode\":23,\"instanceRef\":\"#/address/country\",\"schemaRef\":\"#/definitions/address_type/properties/country\""
        "  },"
        "  \"minItems\": {"
        "    \"actual\": 0, \"expected\": 1, \"errorCode\": 10, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\""
        "  },"
        "  \"additionalProperties\": {"
        "    \"disallowed\": \"planet\", \"errorCode\": 16, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "  },"
        "  \"required\": {"
        "    \"missing\": [\"street1\"], \"errorCode\": 15, \"instanceRef\": \"#/address\", \"schemaRef\": \"#/definitions/address_type\""
        "  },"
        "  \"oneOf\": {"
        "    \"matches\": [0, 1], \"errorCode\": 22, \"instanceRef\": \"#/address/area\", \"schemaRef\": \"#/definitions/address_type/properties/area\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
    INVALIDATE_(s, "{\"address\": {\"number\": 200, \"street1\": {}, \"street3\": null, \"city\": \"Rom\", \"area\": \"Dorset\", \"postcode\": \"SO51 0GP\"}, \"phones\": [\"0111-222333\", \"0777-666888\", \"0777-666888\"], \"names\": [\"Fred\", \"S\", \"M\", \"Bloggs\"]}", "#", "errors", "#",
        "{ \"maximum\": {"
        "    \"errorCode\": 3, \"instanceRef\": \"#/address/number\", \"schemaRef\": \"#/definitions/positiveInt_type\", \"expected\": 100, \"actual\": 200, \"exclusiveMaximum\": true"
        "  },"
        "  \"type\": {"
        "    \"expected\": [\"string\"], \"actual\": \"object\", \"errorCode\": 20, \"instanceRef\": \"#/address/street1\", \"schemaRef\": \"#/definitions/address_type/properties/street1\""
        "  },"
        "  \"not\": {"
        "    \"errorCode\": 25, \"instanceRef\": \"#/address/street3\", \"schemaRef\": \"#/definitions/address_type/properties/street3\""
        "  },"
        "  \"minLength\": {"
        "    \"actual\": \"Rom\", \"expected\": 4, \"errorCode\": 7, \"instanceRef\": \"#/address/city\", \"schemaRef\": \"#/definitions/address_type/properties/city\""
        "  },"
        "  \"maxItems\": {"
        "    \"actual\": 3, \"expected\": 2, \"errorCode\": 9, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\""
        "  },"
        "  \"uniqueItems\": {"
        "    \"duplicates\": [1, 2], \"errorCode\": 11, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\""
        "  },"
        "  \"minProperties\": {\"actual\": 6, \"expected\": 7, \"errorCode\": 14, \"instanceRef\": \"#/address\", \"schemaRef\": \"#/definitions/address_type\""
        "  },"
        "  \"additionalItems\": ["
        "    {\"disallowed\": 2, \"errorCode\": 12, \"instanceRef\": \"#/names\", \"schemaRef\": \"#/properties/names\"},"
        "    {\"disallowed\": 3, \"errorCode\": 12, \"instanceRef\": \"#/names\", \"schemaRef\": \"#/properties/names\"}"
        "  ],"
        "  \"dependencies\": {"
        "    \"errors\": {"
        "      \"address\": {\"required\": {\"missing\": [\"version\"], \"errorCode\": 15, \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/address\"}},"
        "      \"names\": {\"required\": {\"missing\": [\"version\"], \"errorCode\": 15, \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/names\"}}"
        "    },"
        "    \"errorCode\": 18, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "  },"
        "  \"oneOf\": {"
        "    \"errors\": ["
        "      {\"enum\": {\"errorCode\": 19, \"instanceRef\": \"#/address/area\", \"schemaRef\": \"#/definitions/county_type\"}},"
        "      {\"enum\": {\"errorCode\": 19, \"instanceRef\": \"#/address/area\", \"schemaRef\": \"#/definitions/province_type\"}}"
        "    ],"
        "    \"errorCode\": 21, \"instanceRef\": \"#/address/area\", \"schemaRef\": \"#/definitions/address_type/properties/area\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
#endif // RAPIDJSON_YGGDRASIL

        CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, it is not propagated to oneOf sub-validator so we only get the first error.
TEST(SchemaValidator, ContinueOnErrors_OneOf) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/oneOf_address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocumentType s(sd, 0, 0, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"version\": 1.01, \"address\": {\"number\": 0, \"street2\": false,  \"street3\": \"Ham\", \"city\": \"RomseyTownFC\", \"area\": \"BC\", \"country\": \"USA\", \"postcode\": \"999ABC\"}, \"phones\": [], \"planet\": \"Earth\", \"extra\": {\"S_xxx\": 123}}", "#", "errors", "#",
        "{ \"oneOf\": {"
        "    \"errors\": [{"
        "      \"multipleOf\": {"
        "        \"errorCode\": 1, \"instanceRef\": \"#/version\", \"schemaRef\": \"http://localhost:1234/address.json#/definitions/decimal_type\", \"expected\": 1.0, \"actual\": 1.01"
        "      }"
        "    }],"
        "    \"errorCode\": 21, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidatorType, PointerType);
    CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, it is not propagated to allOf sub-validator so we only get the first error.
TEST(SchemaValidator, ContinueOnErrors_AllOf) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/allOf_address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocumentType s(sd, 0, 0, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"version\": 1.01, \"address\": {\"number\": 0, \"street2\": false,  \"street3\": \"Ham\", \"city\": \"RomseyTownFC\", \"area\": \"BC\", \"country\": \"USA\", \"postcode\": \"999ABC\"}, \"phones\": [], \"planet\": \"Earth\", \"extra\": {\"S_xxx\": 123}}", "#", "errors", "#",
        "{ \"allOf\": {"
        "    \"errors\": [{"
        "      \"multipleOf\": {"
        "        \"errorCode\": 1, \"instanceRef\": \"#/version\", \"schemaRef\": \"http://localhost:1234/address.json#/definitions/decimal_type\", \"expected\": 1.0, \"actual\": 1.01"
        "      }"
        "    }],"
        "    \"errorCode\": 23, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidatorType, PointerType);
    CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, it is not propagated to anyOf sub-validator so we only get the first error.
TEST(SchemaValidator, ContinueOnErrors_AnyOf) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/anyOf_address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocumentType s(sd, 0, 0, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "{\"version\": 1.01, \"address\": {\"number\": 0, \"street2\": false,  \"street3\": \"Ham\", \"city\": \"RomseyTownFC\", \"area\": \"BC\", \"country\": \"USA\", \"postcode\": \"999ABC\"}, \"phones\": [], \"planet\": \"Earth\", \"extra\": {\"S_xxx\": 123}}", "#", "errors", "#",
        "{ \"anyOf\": {"
        "    \"errors\": [{"
        "      \"multipleOf\": {"
        "        \"errorCode\": 1, \"instanceRef\": \"#/version\", \"schemaRef\": \"http://localhost:1234/address.json#/definitions/decimal_type\", \"expected\": 1.0, \"actual\": 1.01"
        "      }"
        "    }],"
        "    \"errorCode\": 24, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidatorType, PointerType);

    CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, arrays with uniqueItems:true are correctly processed when an item is invalid.
// This tests that we don't blow up if a hasher does not get created.
TEST(SchemaValidator, ContinueOnErrors_UniqueItems) {
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    VALIDATE(s, "{\"phones\":[\"12-34\",\"56-78\"]}", true);
    INVALIDATE_(s, "{\"phones\":[\"12-34\",\"12-34\"]}", "#", "errors", "#",
        "{\"uniqueItems\": {\"duplicates\": [0,1], \"errorCode\": 11, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\"}}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{\"phones\":[\"ab-34\",\"cd-78\"]}", "#", "errors", "#",
        "{\"pattern\": ["
        "  {\"actual\": \"ab-34\", \"expected\": \"^[0-9]*-[0-9]*\", \"errorCode\": 8, \"instanceRef\": \"#/phones/0\", \"schemaRef\": \"#/definitions/phone_type\"},"
        "  {\"actual\": \"cd-78\", \"expected\": \"^[0-9]*-[0-9]*\", \"errorCode\": 8, \"instanceRef\": \"#/phones/1\", \"schemaRef\": \"#/definitions/phone_type\"}"
        "]}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{\"phones\":[\"ab-34\",\"cd-78\"]}", "#", "errors", "#",
        "{\"pattern\": ["
        "  {\"actual\": \"ab-34\", \"errorCode\": 8, \"instanceRef\": \"#/phones/0\", \"schemaRef\": \"#/definitions/phone_type\"},"
        "  {\"actual\": \"cd-78\", \"errorCode\": 8, \"instanceRef\": \"#/phones/1\", \"schemaRef\": \"#/definitions/phone_type\"}"
        "]}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
#endif // RAPIDJSON_YGGDRASIL
    CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, an enum field is correctly processed when it has an invalid value.
// This tests that we don't blow up if a hasher does not get created.
TEST(SchemaValidator, ContinueOnErrors_Enum) {
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    VALIDATE(s, "{\"gender\":\"M\"}", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{\"gender\":\"X\"}", "#", "errors", "#",
        "{\"enum\": {\"errorCode\": 19, \"instanceRef\": \"#/gender\", \"schemaRef\": \"#/properties/gender\", \"expected\": [\"M\", \"F\"]}}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{\"gender\":\"X\"}", "#", "errors", "#",
        "{\"enum\": {\"errorCode\": 19, \"instanceRef\": \"#/gender\", \"schemaRef\": \"#/properties/gender\"}}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
#endif // RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{\"gender\":1}", "#", "errors", "#",
        "{\"type\": {\"expected\":[\"string\"], \"actual\": \"integer\", \"errorCode\": 20, \"instanceRef\": \"#/gender\", \"schemaRef\": \"#/properties/gender\"}}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
    CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, an array appearing for an object property is handled
// This tests that we don't blow up when there is a type mismatch.
TEST(SchemaValidator, ContinueOnErrors_RogueArray) {
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    INVALIDATE_(s, "{\"address\":[{\"number\": 0}]}", "#", "errors", "#",
        "{\"type\": {\"expected\":[\"object\"], \"actual\": \"array\", \"errorCode\": 20, \"instanceRef\": \"#/address\", \"schemaRef\": \"#/definitions/address_type\"},"
        "  \"dependencies\": {"
        "    \"errors\": {"
        "      \"address\": {\"required\": {\"missing\": [\"version\"], \"errorCode\": 15, \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/address\"}}"
        "    },\"errorCode\": 18, \"instanceRef\": \"#\", \"schemaRef\": \"#\"}}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
    CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, an object appearing for an array property is handled
// This tests that we don't blow up when there is a type mismatch.
TEST(SchemaValidator, ContinueOnErrors_RogueObject) {
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    INVALIDATE_(s, "{\"phones\":{\"number\": 0}}", "#", "errors", "#",
        "{\"type\": {\"expected\":[\"array\"], \"actual\": \"object\", \"errorCode\": 20, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\"}}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
    CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, a string appearing for an array or object property is handled
// This tests that we don't blow up when there is a type mismatch.
TEST(SchemaValidator, ContinueOnErrors_RogueString) {
    CrtAllocator allocator;
    char* schema = ReadFile("unittestschema/address.json", allocator);
    Document sd;
    sd.Parse(schema);
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    INVALIDATE_(s, "{\"address\":\"number\"}", "#", "errors", "#",
        "{\"type\": {\"expected\":[\"object\"], \"actual\": \"string\", \"errorCode\": 20, \"instanceRef\": \"#/address\", \"schemaRef\": \"#/definitions/address_type\"},"
        "  \"dependencies\": {"
        "    \"errors\": {"
        "      \"address\": {\"required\": {\"missing\": [\"version\"], \"errorCode\": 15, \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/address\"}}"
        "    },\"errorCode\": 18, \"instanceRef\": \"#\", \"schemaRef\": \"#\"}}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
    INVALIDATE_(s, "{\"phones\":\"number\"}", "#", "errors", "#",
        "{\"type\": {\"expected\":[\"array\"], \"actual\": \"string\", \"errorCode\": 20, \"instanceRef\": \"#/phones\", \"schemaRef\": \"#/properties/phones\"}}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
    CrtAllocator::Free(schema);
}

// Test that when kValidateContinueOnErrorFlag is set, an incorrect simple type with a sub-schema is handled correctly.
// This tests that we don't blow up when there is a type mismatch but there is a sub-schema present
TEST(SchemaValidator, ContinueOnErrors_BadSimpleType) {
    Document sd;
    sd.Parse("{\"type\":\"string\", \"anyOf\":[{\"maxLength\":2}]}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    VALIDATE(s, "\"AB\"", true);
    INVALIDATE_(s, "\"ABC\"", "#", "errors", "#",
        "{ \"anyOf\": {"
        "    \"errors\": [{"
        "      \"maxLength\": {"
        "        \"errorCode\": 6, \"instanceRef\": \"#\", \"schemaRef\": \"#/anyOf/0\", \"expected\": 2, \"actual\": \"ABC\""
        "      }"
        "    }],"
        "    \"errorCode\": 24, \"instanceRef\": \"#\", \"schemaRef\": \"#\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
    // Invalid type
    INVALIDATE_(s, "333", "#", "errors", "#",
        "{ \"type\": {"
        "    \"errorCode\": 20, \"instanceRef\": \"#\", \"schemaRef\": \"#\", \"expected\": [\"string\"], \"actual\": \"integer\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateContinueOnErrorFlag, SchemaValidator, Pointer);
}


TEST(SchemaValidator, UnknownValidationError) {
    ASSERT_TRUE(SchemaValidator::SchemaType::GetValidateErrorKeyword(kValidateErrors).GetString() == std::string("null"));
}

// The first occurrence of a duplicate keyword is taken
TEST(SchemaValidator, DuplicateKeyword) {
    Document sd;
    sd.Parse("{ \"title\": \"test\",\"type\": \"number\", \"type\": \"string\" }");
    EXPECT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    VALIDATE(s, "42", true);
    INVALIDATE(s, "\"Life, the universe, and everything\"", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}");
}


// SchemaDocument tests

// Specification (schema draft, open api version)
TEST(SchemaValidator, Schema_SupportedNotObject) {
    Document sd;
    sd.Parse("true");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_SupportedNoSpec) {
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_SupportedNoSpecStatic) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    Specification spec = SchemaDocumentType::GetSpecification(sd);
    ASSERT_FALSE(spec.IsSupported());
    ASSERT_TRUE(spec.draft == kDraftNone);
    ASSERT_TRUE(spec.oapi == kVersionNone);
}

TEST(SchemaValidator, Schema_SupportedDraft5Static) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-05/schema#\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    Specification spec = SchemaDocumentType::GetSpecification(sd);
    ASSERT_TRUE(spec.IsSupported());
    ASSERT_TRUE(spec.draft == kDraft05);
    ASSERT_TRUE(spec.oapi == kVersionNone);
}

TEST(SchemaValidator, Schema_SupportedDraft4) {
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-04/schema#\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_SupportedDraft4NoFrag) {
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-04/schema\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_SupportedDraft5) {
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-05/schema#\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft05);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_SupportedDraft5NoFrag) {
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-05/schema\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft05);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_IgnoreDraftEmbedded) {
    Document sd;
    sd.Parse("{\"root\": {\"$schema\":\"http://json-schema.org/draft-05/schema#\", \"type\": \"integer\"}}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd, 0, 0, 0, 0, SchemaDocument::PointerType("/root"));
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_SupportedDraftOverride) {
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd, 0, 0, 0, 0, 0, Specification(kDraft04));
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_UnknownDraftOverride) {
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd, 0, 0, 0, 0, 0, Specification(kDraftUnknown));
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraftUnknown);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    SCHEMAERROR(s, "{\"SpecUnknown\":{\"errorCode\":10,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnsupportedDraftOverride) {
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd, 0, 0, 0, 0, 0, Specification(kDraft03));
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft03);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    SCHEMAERROR(s, "{\"SpecUnsupported\":{\"errorCode\":11,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnknownDraft) {
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-xxx/schema#\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraftUnknown);
     ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
   SCHEMAERROR(s, "{\"SpecUnknown\":{\"errorCode\":10,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnknownDraftNotString) {
    Document sd;
    sd.Parse("{\"$schema\": 4, \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraftUnknown);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    SCHEMAERROR(s, "{\"SpecUnknown\":{\"errorCode\":10,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnsupportedDraft3) {
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-03/schema#\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft03);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    SCHEMAERROR(s, "{\"SpecUnsupported\":{\"errorCode\":11,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnsupportedDraft6) {
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-06/schema#\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft06);
    SCHEMAERROR(s, "{\"SpecUnsupported\":{\"errorCode\":11,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnsupportedDraft7) {
    Document sd;
    sd.Parse("{\"$schema\":\"http://json-schema.org/draft-07/schema#\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft07);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    SCHEMAERROR(s, "{\"SpecUnsupported\":{\"errorCode\":11,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnsupportedDraft2019_09) {
    Document sd;
    sd.Parse("{\"$schema\":\"https://json-schema.org/draft/2019-09/schema\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft2019_09);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    SCHEMAERROR(s, "{\"SpecUnsupported\":{\"errorCode\":11,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnsupportedDraft2020_12) {
    Document sd;
    sd.Parse("{\"$schema\":\"https://json-schema.org/draft/2020-12/schema\", \"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().draft == kDraft2020_12);
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionNone);
    SCHEMAERROR(s, "{\"SpecUnsupported\":{\"errorCode\":11,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_SupportedVersion20Static) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"swagger\":\"2.0\"}");
    ASSERT_FALSE(sd.HasParseError());
    Specification spec = SchemaDocumentType::GetSpecification(sd);
    ASSERT_TRUE(spec.IsSupported());
    ASSERT_TRUE(spec.draft == kDraft04);
    ASSERT_TRUE(spec.oapi == kVersion20);
}

TEST(SchemaValidator, Schema_SupportedVersion20) {
    Document sd;
    sd.Parse("{\"swagger\":\"2.0\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersion20);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_SupportedVersion30x) {
    Document sd;
    sd.Parse("{\"openapi\":\"3.0.0\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersion30);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft05);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_SupportedVersionOverride) {
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd, 0, 0, 0, 0, 0, Specification(kVersion20));
    ASSERT_TRUE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersion20);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    EXPECT_TRUE(s.GetError().ObjectEmpty());
}

TEST(SchemaValidator, Schema_UnknownVersionOverride) {
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd, 0, 0, 0, 0, 0, Specification(kVersionUnknown));
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionUnknown);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    SCHEMAERROR(s, "{\"SpecUnknown\":{\"errorCode\":10,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnsupportedVersionOverride) {
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd, 0, 0, 0, 0, 0, Specification(kVersion31));
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersion31);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft2020_12);
    SCHEMAERROR(s, "{\"SpecUnsupported\":{\"errorCode\":11,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnknownVersion) {
    Document sd;
    sd.Parse("{\"openapi\":\"1.0\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionUnknown);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    SCHEMAERROR(s, "{\"SpecUnknown\":{\"errorCode\":10,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnknownVersionShort) {
    Document sd;
    sd.Parse("{\"openapi\":\"3.0.\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionUnknown);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    SCHEMAERROR(s, "{\"SpecUnknown\":{\"errorCode\":10,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnknownVersionNotString) {
    Document sd;
    sd.Parse("{\"swagger\": 2}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersionUnknown);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft04);
    SCHEMAERROR(s, "{\"SpecUnknown\":{\"errorCode\":10,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_UnsupportedVersion31) {
    Document sd;
    sd.Parse("{\"openapi\":\"3.1.0\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_FALSE(s.IsSupportedSpecification());
    ASSERT_TRUE(s.GetSpecification().oapi == kVersion31);
    ASSERT_TRUE(s.GetSpecification().draft == kDraft2020_12);
    SCHEMAERROR(s, "{\"SpecUnsupported\":{\"errorCode\":11,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_DraftAndVersion) {
    Document sd;
    sd.Parse("{\"swagger\": \"2.0\", \"$schema\": \"http://json-schema.org/draft-04/schema#\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    ASSERT_TRUE(s.IsSupportedSpecification());
    SCHEMAERROR(s, "{\"SpecIllegal\":{\"errorCode\":12,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, Schema_StartUnknown) {
    Document sd;
    sd.Parse("{\"type\": \"integer\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd, 0, 0, 0, 0, SchemaDocument::PointerType("/nowhere"));
    SCHEMAERROR(s, "{\"StartUnknown\":{\"errorCode\":1,\"instanceRef\":\"#\", \"value\":\"#/nowhere\"}}");
}

TEST(SchemaValidator, Schema_MultipleErrors) {
    Document sd;
    sd.Parse("{\"swagger\": \"foo\", \"$schema\": \"bar\"}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s(sd);
    SCHEMAERROR(s, "{ \"SpecUnknown\": {\"errorCode\":10,\"instanceRef\":\"#\"},"
                   "  \"SpecIllegal\": {\"errorCode\":12,\"instanceRef\":\"#\"}"
                   "}");
}

// $ref is a non-JSON pointer fragment - not allowed when OpenAPI
TEST(SchemaValidator, Schema_RefPlainNameOpenApi) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"swagger\": \"2.0\", \"type\": \"object\", \"properties\": {\"myInt1\": {\"$ref\": \"#myId\"}, \"myStr\": {\"type\": \"string\", \"id\": \"#myStrId\"}, \"myInt2\": {\"type\": \"integer\", \"id\": \"#myId\"}}}");
    SchemaDocumentType s(sd);
    SCHEMAERROR(s, "{\"RefPlainName\":{\"errorCode\":2,\"instanceRef\":\"#/properties/myInt1\",\"value\":\"#myId\"}}");
}

// $ref is a non-JSON pointer fragment - not allowed when remote document
TEST(SchemaValidator, Schema_RefPlainNameRemote) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"/subSchemas.json#plainname\"}}}");
    SchemaDocumentType s(sd, "http://localhost:1234/xxxx", 26, &provider);
    SCHEMAERROR(s, "{\"RefPlainName\":{\"errorCode\":2,\"instanceRef\":\"#/properties/myInt\",\"value\":\"#plainname\"}}");
}

// $ref is an empty string
TEST(SchemaValidator, Schema_RefEmptyString) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt1\": {\"$ref\": \"\"}}}");
    SchemaDocumentType s(sd);
    SCHEMAERROR(s, "{\"RefInvalid\":{\"errorCode\":3,\"instanceRef\":\"#/properties/myInt1\"}}");
}

// $ref is remote but no provider
TEST(SchemaValidator, Schema_RefNoRemoteProvider) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"/subSchemas.json#plainname\"}}}");
    SchemaDocumentType s(sd, "http://localhost:1234/xxxx", 26, 0);
    SCHEMAERROR(s, "{\"RefNoRemoteProvider\":{\"errorCode\":7,\"instanceRef\":\"#/properties/myInt\"}}");
}

// $ref is remote but no schema returned
TEST(SchemaValidator, Schema_RefNoRemoteSchema) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"/will-not-resolve.json\"}}}");
    SchemaDocumentType s(sd, "http://localhost:1234/xxxx", 26, &provider);
    SCHEMAERROR(s, "{\"RefNoRemoteSchema\":{\"errorCode\":8,\"instanceRef\":\"#/properties/myInt\",\"value\":\"http://localhost:1234/will-not-resolve.json\"}}");
}

// $ref pointer is invalid
TEST(SchemaValidator, Schema_RefPointerInvalid) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"#/&&&&&\"}}}");
    SchemaDocumentType s(sd);
    SCHEMAERROR(s, "{\"RefPointerInvalid\":{\"errorCode\":4,\"instanceRef\":\"#/properties/myInt\",\"value\":\"#/&&&&&\",\"offset\":2}}");
}

// $ref is remote and pointer is invalid
TEST(SchemaValidator, Schema_RefPointerInvalidRemote) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"/subSchemas.json#/abc&&&&&\"}}}");
    SchemaDocumentType s(sd, "http://localhost:1234/xxxx", 26, &provider);
    SCHEMAERROR(s, "{\"RefPointerInvalid\":{\"errorCode\":4,\"instanceRef\":\"#/properties/myInt\",\"value\":\"#/abc&&&&&\",\"offset\":5}}");
}

// $ref is unknown non-pointer
TEST(SchemaValidator, Schema_RefUnknownPlainName) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"#plainname\"}}}");
    SchemaDocumentType s(sd);
    SCHEMAERROR(s, "{\"RefUnknown\":{\"errorCode\":5,\"instanceRef\":\"#/properties/myInt\",\"value\":\"#plainname\"}}");
}

/// $ref is unknown pointer
TEST(SchemaValidator, Schema_RefUnknownPointer) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"#/a/b\"}}}");
    SchemaDocumentType s(sd);
    SCHEMAERROR(s, "{\"RefUnknown\":{\"errorCode\":5,\"instanceRef\":\"#/properties/myInt\",\"value\":\"#/a/b\"}}");
}

// $ref is remote and unknown pointer
TEST(SchemaValidator, Schema_RefUnknownPointerRemote) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {\"myInt\": {\"$ref\": \"/subSchemas.json#/a/b\"}}}");
    SchemaDocumentType s(sd, "http://localhost:1234/xxxx", 26, &provider);
    SCHEMAERROR(s, "{\"RefUnknown\":{\"errorCode\":5,\"instanceRef\":\"#/properties/myInt\",\"value\":\"http://localhost:1234/subSchemas.json#/a/b\"}}");
}

// $ref is cyclical
TEST(SchemaValidator, Schema_RefCyclical) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    Document sd;
    sd.Parse("{\"type\": \"object\", \"properties\": {"
             "    \"cyclic_source\": {"
             "         \"$ref\": \"#/properties/cyclic_target\""
             "    },"
             "    \"cyclic_target\": {"
             "        \"$ref\": \"#/properties/cyclic_source\""
             "    }"
             "}}");
    SchemaDocumentType s(sd);
    SCHEMAERROR(s, "{\"RefCyclical\":{\"errorCode\":6,\"instanceRef\":\"#/properties/cyclic_target\",\"value\":\"#/properties/cyclic_source\"}}");
}

TEST(SchemaValidator, Schema_ReadOnlyAndWriteOnly) {
    Document sd;
    sd.Parse("{\"type\": \"integer\", \"readOnly\": true, \"writeOnly\": true}");
    ASSERT_FALSE(sd.HasParseError());
    SchemaDocument s1(sd, 0, 0, 0, 0, 0, Specification(kDraft04));
    EXPECT_TRUE(s1.GetError().ObjectEmpty());
    SchemaDocument s2(sd, 0, 0, 0, 0, 0, Specification(kVersion30));
    SCHEMAERROR(s2, "{\"ReadOnlyAndWriteOnly\":{\"errorCode\":13,\"instanceRef\":\"#\"}}");
}

TEST(SchemaValidator, ReadOnlyWhenWriting) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\":\"object\","
        "    \"properties\": {"
        "        \"rprop\" : {"
        "            \"type\": \"string\","
        "            \"readOnly\": true"
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd, 0, 0, 0, 0, 0, Specification(kVersion20));
    VALIDATE(s, "{ \"rprop\": \"hello\" }", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{ \"rprop\": \"hello\" }", "/properties/rprop", "readOnly", "/rprop",
        "{ \"readOnly\": {"
        "    \"errorCode\": 48, \"instanceRef\": \"#/rprop\", \"schemaRef\": \"#/properties/rprop\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateWriteFlag, SchemaValidator, Pointer);
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{ \"rprop\": \"hello\" }", "/properties/rprop", "readOnly", "/rprop",
        "{ \"readOnly\": {"
        "    \"errorCode\": 26, \"instanceRef\": \"#/rprop\", \"schemaRef\": \"#/properties/rprop\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateWriteFlag, SchemaValidator, Pointer);
#endif // RAPIDJSON_YGGDRASIL
}

TEST(SchemaValidator, WriteOnlyWhenReading) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\":\"object\","
        "    \"properties\": {"
        "        \"wprop\" : {"
        "            \"type\": \"boolean\","
        "            \"writeOnly\": true"
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd, 0, 0, 0, 0, 0, Specification(kVersion30));
    VALIDATE(s, "{ \"wprop\": true }", true);
#ifdef RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{ \"wprop\": true }", "/properties/wprop", "writeOnly", "/wprop",
        "{ \"writeOnly\": {"
        "    \"errorCode\": 49, \"instanceRef\": \"#/wprop\", \"schemaRef\": \"#/properties/wprop\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateReadFlag, SchemaValidator, Pointer);
#else // RAPIDJSON_YGGDRASIL
    INVALIDATE_(s, "{ \"wprop\": true }", "/properties/wprop", "writeOnly", "/wprop",
        "{ \"writeOnly\": {"
        "    \"errorCode\": 27, \"instanceRef\": \"#/wprop\", \"schemaRef\": \"#/properties/wprop\""
        "  }"
        "}",
        kValidateDefaultFlags | kValidateReadFlag, SchemaValidator, Pointer);
#endif // RAPIDJSON_YGGDRASIL
}

TEST(SchemaValidator, NullableTrue) {
    Document sd;
    sd.Parse("{\"type\": \"string\", \"nullable\": true}");
    SchemaDocument s(sd, 0, 0, 0, 0, 0, kVersion20);

    VALIDATE(s, "\"hello\"", true);
    INVALIDATE(s, "null", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"null\""
        "}}");
    INVALIDATE(s, "false", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"boolean\""
        "}}");

    SchemaDocument s30(sd, 0, 0, 0, 0, 0, kVersion30);

    VALIDATE(s30, "\"hello\"", true);
    VALIDATE(s30, "null", true);
    INVALIDATE(s30, "false", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"null\", \"string\"], \"actual\": \"boolean\""
        "}}");
}

TEST(SchemaValidator, NullableFalse) {
    Document sd;
    sd.Parse("{\"type\": \"string\", \"nullable\": false}");
    SchemaDocument s(sd, 0, 0, 0, 0, 0, kVersion20);

    VALIDATE(s, "\"hello\"", true);
    INVALIDATE(s, "null", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"null\""
        "}}");
    INVALIDATE(s, "false", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"boolean\""
        "}}");

    SchemaDocument s30(sd, 0, 0, 0, 0, 0, kVersion30);

    VALIDATE(s30, "\"hello\"", true);
    INVALIDATE(s, "null", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"null\""
        "}}");
    INVALIDATE(s30, "false", "", "type", "",
        "{ \"type\": {"
        "    \"errorCode\": 20,"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"boolean\""
        "}}");
}

#ifdef RAPIDJSON_YGGDRASIL
#define VALIDATE_ENCODED(obj, exp, exp_min)				\
  Document d_obj;							\
  d_obj.Parse(obj);							\
  EXPECT_FALSE(d_obj.HasParseError());					\
  {									\
    SchemaEncoder encoder;						\
    EXPECT_TRUE(d_obj.Accept(encoder));					\
    Document d_exp;							\
    d_exp.Parse(exp);							\
    EXPECT_FALSE(d_exp.HasParseError());				\
    if (encoder.GetSchema() != d_exp) {					\
      StringBuffer sb;							\
      Writer<StringBuffer> w(sb);					\
      encoder.GetSchema().Accept(w);					\
      printf("GetSchema() Expected: %s Actual: %s\n", exp, sb.GetString()); \
      ADD_FAILURE();							\
    }									\
    }									\
  {									\
   SchemaEncoder encoder(true);						\
    EXPECT_TRUE(d_obj.Accept(encoder));					\
    Document d_exp;							\
    d_exp.Parse(exp_min);						\
    EXPECT_FALSE(d_exp.HasParseError());				\
    if (encoder.GetSchema() != d_exp) {					\
      StringBuffer sb;							\
      Writer<StringBuffer> w(sb);					\
      encoder.GetSchema().Accept(w);					\
      printf("GetSchema() Expected: %s Actual: %s\n", exp_min, sb.GetString()); \
      ADD_FAILURE();							\
    }									\
  }
TEST(SchemaEncoder, Boolean) {
  VALIDATE_ENCODED("true",
		   "{\"type\":\"boolean\"}",
		   "{\"type\":\"boolean\"}");
}
TEST(SchemaEncoder, Null) {
  VALIDATE_ENCODED("null",
		   "{\"type\":\"null\"}",
		   "{\"type\":\"null\"}");
}
TEST(SchemaEncoder, Int) {
  VALIDATE_ENCODED("1",
		   "{\"type\":\"integer\"}",
		   "{\"type\":\"integer\"}");
}
TEST(SchemaEncoder, Double) {
  VALIDATE_ENCODED("1.5",
		   "{\"type\":\"number\"}",
		   "{\"type\":\"number\"}");
}
TEST(SchemaEncoder, String) {
  VALIDATE_ENCODED("\"hello\"",
		   "{\"type\":\"string\"}",
		   "{\"type\":\"string\"}");
}
TEST(SchemaEncoder, Array) {
  VALIDATE_ENCODED("[1, \"hello\"]",
		   "{"
		   "  \"type\":\"array\","
		   "  \"items\": ["
		   "    {\"type\": \"integer\"},"
		   "    {\"type\": \"string\"}"
		   "  ]"
		   "}",
		   "{"
		   "  \"type\":\"array\","
		   "  \"items\": ["
		   "    {\"type\": \"integer\"},"
		   "    {\"type\": \"string\"}"
		   "  ]"
		   "}");
}
TEST(SchemaEncoder, Object) {
  VALIDATE_ENCODED("{\"a\": 1, \"b\": \"hello\"}",
		   "{"
		   "  \"type\": \"object\","
		   "  \"properties\": {"
		   "    \"a\": {\"type\": \"integer\"},"
		   "    \"b\": {\"type\": \"string\"}"
		   "  }"
		   "}",
		   "{"
		   "  \"type\": \"object\","
		   "  \"properties\": {"
		   "    \"a\": {\"type\": \"integer\"},"
		   "    \"b\": {\"type\": \"string\"}"
		   "  }"
		   "}");
}
TEST(SchemaEncoder, Scalar) {
  VALIDATE_ENCODED("\"-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjEsInVuaXRzIjoiZyJ9-YGG-DA==-YGG-\"",
		   "{"
		   "  \"type\": \"scalar\","
		   "  \"subtype\": \"uint\","
		   "  \"precision\": 1,"
		   "  \"units\": \"g\""
		   "}",
		   "{"
		   "  \"type\": \"scalar\","
		   "  \"subtype\": \"uint\","
		   "  \"precision\": 1,"
		   "  \"units\": \"g\""
		   "}");
}
TEST(SchemaEncoder, OneDArray) {
  VALIDATE_ENCODED("\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImciLCJzaGFwZSI6WzNdfQ==-YGG-AAEC-YGG-\"",
		   "{"
		   "  \"type\": \"ndarray\","
		   "  \"subtype\": \"uint\","
		   "  \"precision\": 1,"
		   "  \"units\": \"g\","
		   "  \"shape\": [3]"
		   "}",
		   "{"
		   "  \"type\": \"ndarray\","
		   "  \"subtype\": \"uint\","
		   "  \"precision\": 1,"
		   "  \"units\": \"g\""
		   "}");
}
TEST(SchemaEncoder, NDArray) {
  VALIDATE_ENCODED("\"-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJ1bml0cyI6ImciLCJzaGFwZSI6WzIsM119-YGG-AAECAwQF-YGG-\"",
		   "{"
		   "  \"type\": \"ndarray\","
		   "  \"subtype\": \"uint\","
		   "  \"precision\": 1,"
		   "  \"units\": \"g\","
		   "  \"shape\": [2, 3]"
		   "}",
		   "{"
		   "  \"type\": \"ndarray\","
		   "  \"subtype\": \"uint\","
		   "  \"precision\": 1,"
		   "  \"units\": \"g\""
		   "}");
}
#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
TEST(SchemaEncoder, PythonClass) {
  VALIDATE_ENCODED("\"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-ZXhhbXBsZV9weXRob246RXhhbXBsZUNsYXNz-YGG-\"",
		   "{"
		   "  \"type\": \"class\""
		   "}",
		   "{"
		   "  \"type\": \"class\""
		   "}");
}
TEST(SchemaEncoder, PythonFunction) {
  VALIDATE_ENCODED("\"-YGG-eyJ0eXBlIjoiZnVuY3Rpb24ifQ==-YGG-ZXhhbXBsZV9weXRob246ZXhhbXBsZV9mdW5jdGlvbgA=-YGG-\"",
		   "{"
		   "  \"type\": \"function\""
		   "}",
		   "{"
		   "  \"type\": \"function\""
		   "}");
}
TEST(SchemaEncoder, PythonInstance) {
  VALIDATE_ENCODED("\"-YGG-eyJ0eXBlIjoiaW5zdGFuY2UifQ==-YGG-eyJjbGFzcyI6ImV4YW1wbGVfcHl0aG9uOkV4YW1wbGVDbGFzcyIsImFyZ3MiOlsiaGVsbG8iLDAuNV0sImt3YXJncyI6eyJhIjoid29ybGQiLCJiIjoxfX0=-YGG-\"",
		   "{"
		   "  \"type\": \"instance\""
		   "}",
		   "{"
		   "  \"type\": \"instance\""
		   "}");
}
#endif // YGGDRASIL_DISABLE_PYTHON_C_API
TEST(SchemaEncoder, Schema) {
  VALIDATE_ENCODED("\"-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiaW50IiwicHJlY2lzaW9uIjo4fQ==-YGG-\"",
		   "{"
		   "  \"type\": \"schema\""
		   "}",
		   "{"
		   "  \"type\": \"schema\""
		   "}");
}

#define COMPARE_SHOW_DOCS                                               \
  {                                                                     \
    StringBuffer sb_lhs, sb_rhs;                                        \
    PrettyWriter<StringBuffer> w_lhs(sb_lhs), w_rhs(sb_rhs);            \
    sd_lhs.Accept(w_lhs);                                               \
    sd_rhs.Accept(w_rhs);                                               \
    printf("LHS:\n%s\nRHS:\n%s\n",                                      \
           sb_lhs.GetString(), sb_rhs.GetString());                     \
  }
                                                        
#define COMPARE(lhs, rhs)						\
  {									\
    Document sd_lhs, sd_rhs;						\
    sd_lhs.Parse(lhs);							\
    sd_rhs.Parse(rhs);							\
    SchemaDocument s_lhs(sd_lhs);					\
    SchemaDocument s_rhs(sd_rhs);					\
    SchemaValidator validator_lhs(s_lhs);				\
    SchemaValidator validator_rhs(s_rhs);				\
    bool result = validator_lhs.Compare(validator_rhs);			\
    EXPECT_TRUE(result);						\
    EXPECT_TRUE(validator_lhs.IsValid());				\
    ValidateErrorCode code = validator_lhs.GetInvalidSchemaCode();	\
    EXPECT_TRUE(code == kValidateErrorNone);				\
    EXPECT_TRUE(validator_lhs.GetInvalidSchemaKeyword() == 0);		\
    if (!result) {							\
      StringBuffer sb;                                                  \
      PrettyWriter<StringBuffer> w(sb);                                 \
      validator_lhs.GetError().Accept(w);				\
      COMPARE_SHOW_DOCS;                                                \
      printf("Comparison error:\n%s\n", sb.GetString());                \
    }									\
  }
#define INVALID_COMPARE(lhs, rhs, error)				\
  {									\
    Document sd_lhs, sd_rhs;						\
    sd_lhs.Parse(lhs);							\
    sd_rhs.Parse(rhs);							\
    SchemaDocument s_lhs(sd_lhs);					\
    SchemaDocument s_rhs(sd_rhs);					\
    SchemaValidator validator_lhs(s_lhs);				\
    SchemaValidator validator_rhs(s_rhs);				\
    bool result = validator_lhs.Compare(validator_rhs);			\
    EXPECT_FALSE(result);						\
    Document e;								\
    e.Parse(error);							\
    RAPIDJSON_DEFAULT_ALLOCATOR error_msg_allocator;			\
    Value e_msg;							\
    if (!validator_lhs.GetErrorMsg(e_msg, error_msg_allocator)) {	\
      StringBuffer sb_t;						\
      PrettyWriter<StringBuffer> w_t(sb_t);				\
      printf("ErrorMsg = %s\n", sb_t.GetString());			\
      StringBuffer sb_lhs;                                              \
      PrettyWriter<StringBuffer> w_lhs(sb_lhs);                         \
      validator_lhs.GetError().Accept(w_lhs);				\
      StringBuffer sb_rhs;                                              \
      PrettyWriter<StringBuffer> w_rhs(sb_rhs);                         \
      validator_rhs.GetError().Accept(w_rhs);				\
      printf("lhs.GetError(): %s\nrhs.GetError(): %s", sb_lhs.GetString(), sb_rhs.GetString()); \
      ADD_FAILURE();							\
    }									\
    if (validator_lhs.GetError() != e) {				\
      COMPARE_SHOW_DOCS;                                                \
      StringBuffer sb_lhs;                                              \
      PrettyWriter<StringBuffer> w_lhs(sb_lhs);                         \
      validator_lhs.GetError().Accept(w_lhs);				\
      StringBuffer sb_rhs;						\
      PrettyWriter<StringBuffer> w_rhs(sb_rhs);                         \
      validator_rhs.GetError().Accept(w_rhs);                           \
      StringBuffer sb_e;						\
      PrettyWriter<StringBuffer> w_e(sb_e);				\
      e.Accept(w_e);							\
      printf("lhs.GetError()\nExpected: %s\nActual RHS: %s\nActual LHS: %s\n", sb_e.GetString(), sb_lhs.GetString(), sb_rhs.GetString()); \
      ADD_FAILURE();							\
    }									\
  }
#define COMPARE_SYMMETRIC(lhs, rhs)                     \
  COMPARE(lhs, rhs);                                    \
  COMPARE(rhs, lhs)
#define INVALID_COMPARE_SYMMETRIC(lhs, rhs, error)      \
  INVALID_COMPARE(lhs, rhs, error);                     \
  INVALID_COMPARE(rhs, lhs, error)
#define INVALID_COMPARE_ASYMMETRIC(lhs, rhs, error_lhs, error_rhs)      \
  INVALID_COMPARE(lhs, rhs, error_lhs);                                 \
  INVALID_COMPARE(rhs, lhs, error_rhs)
  
TEST(SchemaCompare, Type) {
  COMPARE("{"
	  "  \"type\": \"schema\""
	  "}",
	  "{"
	  "  \"type\": \"schema\""
	  "}");
  COMPARE("{"
	  "  \"type\": \"schema\""
	  "}",
	  "{"
	  "  \"type\": [\"number\", \"schema\"]"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"schema\""
		  "}",
		  "{"
		  "  \"type\": \"string\""
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"schema\"],"
                  "    \"actual\": [\"string\"]"
		  "}}");
}
TEST(SchemaCompare, SubType) {
  COMPARE("{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8"
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 8"
		  "}",
		  "{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"int\","
		  "  \"precision\": 8"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"subtype\","
		  "    \"expected\": [\"float\"], \"actual\": [\"int\"]"
		  "}}");
}
TEST(SchemaCompare, Precision) {
  INVALID_COMPARE("{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 8"
		  "}",
		  "{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 4"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"precision\","
		  "    \"expected\": 8, \"actual\": 4"
		  "}}");
}
TEST(SchemaCompare, Units) {
  COMPARE("{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"units\": \"cm\""
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"units\": \"cm\""
	  "}");
  COMPARE("{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"units\": \"cm\""
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 8,"
		  "  \"units\": \"cm\""
		  "}",
		  "{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 8,"
		  "  \"units\": \"kg\""
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"units\","
		  "    \"expected\": \"cm\", \"actual\": \"kg\""
		  "}}");
}
TEST(SchemaCompare, Shape) {
  COMPARE("{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"shape\": [2, 3]"
	  "}",
	  "{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"shape\": [2, 3]"
	  "}");
  COMPARE("{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"shape\": [2, 3]"
	  "}",
	  "{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"ndarray\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 8,"
		  "  \"shape\": [2, 3]"
		  "}",
		  "{"
		  "  \"type\": \"ndarray\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 8,"
		  "  \"shape\": [4, 5]"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"shape\","
		  "    \"expected\": [2, 3], \"actual\": [4, 5]"
		  "}}");
}
TEST(SchemaCompare, NDim) {
  COMPARE("{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"ndim\": 2"
	  "}",
	  "{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"ndim\": 2"
	  "}");
  COMPARE("{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"ndim\": 2"
	  "}",
	  "{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8"
	  "}");
  COMPARE("{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"ndim\": 2"
	  "}",
	  "{"
	  "  \"type\": \"ndarray\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 8,"
	  "  \"shape\": [2, 3]"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"ndarray\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 8,"
		  "  \"ndim\": 2"
		  "}",
		  "{"
		  "  \"type\": \"ndarray\","
		  "  \"subtype\": \"float\","
		  "  \"precision\": 8,"
		  "  \"ndim\": 3"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"ndim\","
		  "    \"expected\": 2, \"actual\": 3"
		  "}}");
}
TEST(SchemaCompare, Encoding) {
  COMPARE("{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"string\","
	  "  \"precision\": 8,"
	  "  \"encoding\": \"UTF8\""
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"string\","
	  "  \"precision\": 8,"
	  "  \"encoding\": \"UTF8\""
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"string\","
		  "  \"precision\": 8,"
		  "  \"encoding\": \"UTF8\""
		  "}",
		  "{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"string\","
		  "  \"precision\": 8"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"encoding\","
		  "    \"expected\": \"UTF8\", \"actual\": \"null\""
		  "}}");
}
#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
TEST(SchemaCompare, Class) {
  COMPARE("{"
	  "  \"type\": \"instance\","
	  "  \"class\": \"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-ZXhhbXBsZV9weXRob246RXhhbXBsZUNsYXNz-YGG-\""
	  "}",
	  "{"
	  "  \"type\": \"instance\","
	  "  \"class\": \"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-ZXhhbXBsZV9weXRob246RXhhbXBsZUNsYXNz-YGG-\""
	  "}");
  COMPARE("{"
	  "  \"type\": \"instance\","
	  "  \"class\": \"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-ZXhhbXBsZV9weXRob246RXhhbXBsZUNsYXNz-YGG-\""
	  "}",
	  "{"
	  "  \"type\": \"instance\","
	  "  \"class\": \"example_python:ExampleClass\""
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"instance\","
		  "  \"class\": \"-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-ZXhhbXBsZV9weXRob246RXhhbXBsZUNsYXNz-YGG-\""
		  "}",
		  "{"
		  "  \"type\": \"instance\","
		  "  \"class\": \"example_python:OtherClass\""
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"class\","
		  "    \"expected\": \"example_python:ExampleClass\", \"actual\": \"example_python:OtherClass\""
		  "}}");
}
#endif // YGGDRASIL_DISABLE_PYTHON_C_API
TEST(SchemaCompare, Enum) {
  COMPARE("{"
	  "  \"enum\": [\"NW\", \"NE\", \"SW\"]"
	  "}",
	  "{"
	  "  \"enum\": [\"SW\", \"SE\"]"
	  "}");
  INVALID_COMPARE("{"
		  "  \"enum\": [\"NW\", \"NE\"]"
		  "}",
		  "{"
		  "  \"enum\": [\"SW\", \"SE\"]"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"enum\","
		  "    \"expected\": [\"NW\", \"NE\"], \"actual\": [\"SW\", \"SE\"]"
		  "}}");
}
TEST(SchemaCompare, Not) {
  COMPARE("{"
	  "  \"type\": \"boolean\""
	  "}",
	  "{"
	  "  \"not\": { \"type\": \"string\" }"
	  "}");
  COMPARE("{"
	  "  \"not\": { \"type\": \"string\" }"
	  "}",
	  "{"
	  "  \"not\": { \"type\": \"string\" }"
	  "}");
  COMPARE("{"
	  "  \"type\": \"schema\","
	  "  \"not\": { \"type\": \"string\" }"
	  "}",
	  "{"
	  "  \"type\": \"schema\""
	  "}");
  INVALID_COMPARE("{"
		  "  \"not\": { \"type\": \"string\" }"
		  "}",
		  "{"
		  "  \"not\": { \"type\": \"schema\" }"
		  "}",
		  "{ \"not\": {"
		  "  \"errorCode\": 25,"
		  "  \"schemaIteratorRef\": \"#\""
                  // "  \"schemaHandlerRef\": \"#\""
		  "}}");
}
TEST(SchemaCompare, AllOf) {
  COMPARE("{"
	  "  \"type\": \"string\""
	  "}",
	  "{"
	  "  \"allOf\": ["
	  "    { \"type\": \"string\" },"
	  "    { \"maxLength\": 5 }"
	  "  ]"
	  "}");
  COMPARE("{"
	  "  \"allOf\": ["
	  "    { \"type\": \"string\" },"
	  "    { \"maxLength\": 5 }"
	  "  ]"
	  "}",
	  "{"
	  "  \"allOf\": ["
	  "    { \"type\": \"string\" },"
	  "    { \"maxLength\": 5 }"
	  "  ]"
	  "}");
  INVALID_COMPARE("{"
		  "  \"allOf\": ["
		  "    { \"type\": \"string\" },"
		  "    { \"maxLength\": 5 }"
		  "  ]"
		  "}",
		  "{"
		  "  \"allOf\": ["
		  "    { \"type\": \"string\","
		  "      \"maxLength\": 3 }"
		  "  ]"
		  "}",
                  "{"
                  "  \"allOf\": {"
                  "    \"errors\": ["
                  "      {},"
                  "      {"
                  "        \"allOf\": {"
                  "          \"errors\": [{"
		  "            \"compare\": {"
		  "              \"property\": \"maxLength\","
		  "              \"expected\": 5, \"actual\": 3,"
		  "              \"errorCode\": 45,"
		  "              \"schemaIteratorRef\": \"#/allOf/1\","
		  "              \"schemaHandlerRef\": \"#/allOf/0\""
                  "            }"
                  "          }],"
                  "          \"errorCode\": 23,"
		  "          \"schemaIteratorRef\": \"#/allOf/1\","
		  "          \"schemaHandlerRef\": \"#\""
                  "        }"
                  "      }"
                  "    ],"
                  "    \"errorCode\": 23,"
                  "    \"schemaIteratorRef\": \"#\""
                  "  }"
                  "}");
  INVALID_COMPARE("{"
		  "  \"allOf\": ["
		  "    { \"type\": \"string\" },"
		  "    { \"maxLength\": 5 }"
		  "  ]"
		  "}",
		  "{"
		  "  \"allOf\": ["
		  "    { \"type\": \"string\" },"
		  "    { \"maxLength\": 3 }"
		  "  ]"
		  "}",
                  "{"
                  "  \"allOf\": {"
                  "    \"errors\": ["
                  "      {},"
                  "      {"
                  "        \"allOf\": {"
                  "          \"errors\": ["
                  "            {},"
                  "            {"
		  "              \"compare\": {"
		  "                \"property\": \"maxLength\","
		  "                \"expected\": 5, \"actual\": 3,"
		  "                \"errorCode\": 45,"
		  "                \"schemaIteratorRef\": \"#/allOf/1\","
		  "                \"schemaHandlerRef\": \"#/allOf/1\""
                  "              }"
                  "            }],"
                  "          \"errorCode\": 23,"
		  "          \"schemaIteratorRef\": \"#/allOf/1\","
		  "          \"schemaHandlerRef\": \"#\""
                  "        }"
                  "      }"
                  "    ],"
                  "    \"errorCode\": 23,"
                  "    \"schemaIteratorRef\": \"#\""
                  "  }"
                  "}");
}
TEST(SchemaCompare, AnyOf) {
  COMPARE("{"
          "  \"type\": \"string\""
          "}",
          "{"
          "  \"anyOf\": ["
          "    { \"type\": \"string\" },"
          "    { \"maxLength\": 5 }"
          "  ]"
          "}");
  COMPARE("{"
          "  \"anyOf\": ["
          "    { \"type\": \"string\" },"
          "    { \"maxLength\": 5 }"
          "  ]"
          "}",
          "{"
          "  \"anyOf\": ["
          "    { \"type\": \"string\" },"
          "    { \"maxLength\": 5 }"
          "  ]"
          "}");
  INVALID_COMPARE("{"
        	  "  \"anyOf\": ["
        	  "    { \"type\": \"boolean\" },"
        	  "    { \"type\": \"string\","
        	  "      \"maxLength\": 5 }"
        	  "  ]"
        	  "}",
        	  "{"
        	  "  \"anyOf\": ["
        	  "    { \"type\": \"string\","
        	  "      \"maxLength\": 3 }"
        	  "  ]"
        	  "}",
                  "{"
                  "  \"anyOf\": {"
                  "    \"errors\": ["
                  "      {"
                  "        \"anyOf\": {"
                  "          \"errors\": [{"
        	  "            \"compare\": {"
        	  "              \"property\": \"type\","
        	  "              \"expected\": [\"boolean\"],"
                  "              \"actual\": [\"string\"],"
        	  "              \"errorCode\": 45,"
        	  "              \"schemaIteratorRef\": \"#/anyOf/0\","
        	  "              \"schemaHandlerRef\": \"#/anyOf/0\""
                  "            }"
                  "          }],"
                  "          \"errorCode\": 24,"
                  "          \"schemaIteratorRef\": \"#/anyOf/0\","
                  "          \"schemaHandlerRef\": \"#\""
                  "        }"
                  "      },"
                  "      {"
                  "        \"anyOf\": {"
                  "          \"errors\": [{"
        	  "            \"compare\": {"
        	  "              \"property\": \"maxLength\","
        	  "              \"expected\": 5, \"actual\": 3,"
        	  "              \"errorCode\": 45,"
        	  "              \"schemaIteratorRef\": \"#/anyOf/1\","
        	  "              \"schemaHandlerRef\": \"#/anyOf/0\""
                  "            }"
                  "          }],"
                  "          \"errorCode\": 24,"
                  "          \"schemaIteratorRef\": \"#/anyOf/1\","
                  "          \"schemaHandlerRef\": \"#\""
                  "        }"
                  "      }"
                  "    ],"
                  "    \"errorCode\": 24,"
                  "    \"schemaIteratorRef\": \"#\""
                  "  }"
        	  "}");
}
TEST(SchemaCompare, OneOf) {
  COMPARE("{"
	  "  \"type\": \"string\""
	  "}",
	  "{"
	  "  \"anyOf\": ["
	  "    { \"type\": \"boolean\" },"
	  "    { \"maxLength\": 5 }"
	  "  ]"
	  "}");
  COMPARE("{"
	  "  \"oneOf\": ["
	  "    { \"type\": \"boolean\" },"
	  "    { \"type\": \"string\", \"maxLength\": 5 }"
	  "  ]"
	  "}",
	  "{"
	  "  \"oneOf\": ["
	  "    { \"type\": \"boolean\" },"
	  "    { \"type\": \"string\", \"maxLength\": 3 }"
	  "  ]"
	  "}");
  INVALID_COMPARE("{"
		  "  \"oneOf\": ["
		  "    { \"type\": \"string\" },"
		  "    { \"maxLength\": 5 }"
		  "  ]"
		  "}",
		  "{"
		  "  \"oneOf\": ["
		  "    { \"type\": \"string\" }"
		  "  ]"
		  "}",
		  "{ \"oneOf\": {"
		  "    \"errorCode\": 22,"
                  "    \"matches\": [0, 1],"
		  "    \"schemaIteratorRef\": \"#\""
		  "}}");
}
TEST(SchemaCompare, Properties) {
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"string\"}"
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/properties/b\", \"schemaHandlerRef\": \"#/properties/b\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"boolean\"], \"actual\": [\"string\"]"
		  "}}");
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"},"
	  "    \"c\": {\"type\": \"boolean\"}"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  }"
	  "}");
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"},"
	  "    \"c\": {\"type\": \"boolean\"}"
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"},"
		  "    \"c\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
                  "  \"additionalProperties\": false"
		  "}",
		  "{ \"additionalProperties\": {"
		  "    \"errorCode\": 16,"
		  "    \"schemaIteratorRef\": \"#\","
                  "    \"schemaHandlerRef\": \"#\","
		  "    \"disallowed\": \"c\""
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\", \"c\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"},"
		  "    \"c\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{ \"required\": {"
                  "    \"missing\": [\"c\"],"
		  "    \"errorCode\": 15,"
		  "    \"schemaIteratorRef\": \"#\""
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\", \"c\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"},"
		  "    \"c\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{ \"required\": {"
                  "    \"missing\": [\"c\"],"
		  "    \"errorCode\": 15,"
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"schemaHandlerRef\": \"#\""
		  "}}");
}
TEST(SchemaCompare, AdditionalProperties) {
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"boolean\""
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/additionalProperties\", \"schemaHandlerRef\": \"#/additionalProperties\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"string\"], \"actual\": [\"boolean\"]"
		  "}}");
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": false"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": false"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": false"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": true"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"additionalProperties\","
		  "    \"expected\": false, \"actual\": true"
		  "}}");
}
TEST(SchemaCompare, PatternProperties) {
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"patternProperties\": {"
	  "    \"^S_\": { \"type\": \"string\" },"
	  "    \"^I_\": { \"type\": \"integer\" }"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"patternProperties\": {"
	  "    \"^S_\": { \"type\": \"string\" },"
	  "    \"^I_\": { \"type\": \"integer\" }"
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"patternProperties\": {"
		  "    \"^S_\": { \"type\": \"string\" },"
		  "    \"^I_\": { \"type\": \"integer\" }"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"patternProperties\": {"
		  "    \"^S_\": { \"type\": \"boolean\" },"
		  "    \"^I_\": { \"type\": \"integer\" }"
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/patternProperties/%5ES_\", \"schemaHandlerRef\": \"#/patternProperties/%5ES_\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"string\"], \"actual\": [\"boolean\"]"
		  "}}");
}
TEST(SchemaCompare, PropertiesCount) {
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"minProperties\": 2,"
	  "  \"maxProperties\": 10"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"minProperties\": 2,"
	  "  \"maxProperties\": 10"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minProperties\": 2,"
		  "  \"maxProperties\": 10"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minProperties\": 3,"
		  "  \"maxProperties\": 10"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"minProperties\","
		  "    \"expected\": 2, \"actual\": 3"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minProperties\": 2,"
		  "  \"maxProperties\": 10"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minProperties\": 2,"
		  "  \"maxProperties\": 15"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxProperties\","
		  "    \"expected\": 10, \"actual\": 15"
		  "}}");
  // Estimate min based on propertyCount
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"string\"},"
	  "    \"c\": {\"type\": \"string\"}"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"minProperties\": 2"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"}"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minProperties\": 2"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"minProperties\","
		  "    \"expected\": 1, \"actual\": 2"
		  "}}");
  // Estimate max based on propertyCount
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"string\"},"
	  "    \"c\": {\"type\": \"string\"}"
	  "  },"
	  "  \"additionalProperties\": false"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"maxProperties\": 3"
	  "}");
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"additionalProperties\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"maxProperties\": 3"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"string\"},"
	  "    \"c\": {\"type\": \"string\"}"
	  "  },"
	  "  \"additionalProperties\": false"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"string\"},"
		  "    \"c\": {\"type\": \"string\"}"
		  "  },"
		  "  \"additionalProperties\": true"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"maxProperties\": 3"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxProperties\","
		  "    \"expected\": 4294967295, \"actual\": 3"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"maxProperties\": 3"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"string\"},"
		  "    \"c\": {\"type\": \"string\"}"
		  "  },"
		  "  \"additionalProperties\": true"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxProperties\","
		  "    \"expected\": 3, \"actual\": 4294967295"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"string\"},"
		  "    \"c\": {\"type\": \"string\"}"
		  "  },"
		  "  \"additionalProperties\": false"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"maxProperties\": 2"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxProperties\","
		  "    \"expected\": 3, \"actual\": 2"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"maxProperties\": 2"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"string\"},"
		  "    \"c\": {\"type\": \"string\"}"
		  "  },"
		  "  \"additionalProperties\": false"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxProperties\","
		  "    \"expected\": 2, \"actual\": 3"
		  "}}");
}
TEST(SchemaCompare, PropertiesComplex) {
  // Missing property not required, but additional properties disallowed
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"additionalProperties\": false"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"},"
		  "    \"c\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{ \"additionalProperties\": {"
		  "    \"errorCode\": 16,"
		  "    \"schemaIteratorRef\": \"#\","
                  "    \"schemaHandlerRef\": \"#\","
		  "    \"disallowed\": [\"c\"]"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"},"
		  "    \"c\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"additionalProperties\": false"
		  "}",
		  "{ \"additionalProperties\": {"
		  "    \"errorCode\": 16,"
		  "    \"schemaIteratorRef\": \"#\","
                  "    \"schemaHandlerRef\": \"#\","
		  "    \"disallowed\": \"c\""
		  "}}");
  // Missing property matches additional properties
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"},"
	  "    \"c\": {\"type\": \"boolean\"}"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  },"
	  "  \"additionalProperties\": {"
	  "    \"type\": \"boolean\""
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"},"
		  "    \"c\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"additionalProperties\": {"
		  "    \"type\": \"string\""
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/properties/c\","
		  "    \"schemaHandlerRef\": \"#/additionalProperties\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"boolean\"], \"actual\": [\"string\"]"
		  "}}");
  // Missing property matches pattern
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"},"
	  "    \"I_0\": {\"type\": \"boolean\"}"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  },"
	  "  \"patternProperties\": {"
	  "    \"^I_\": { \"type\": \"boolean\" }"
	  "  }"
	  "}");
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  },"
	  "  \"patternProperties\": {"
	  "    \"^I_\": { \"type\": \"boolean\" }"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"},"
	  "    \"I_0\": {\"type\": \"boolean\"}"
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"},"
		  "    \"I_0\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"patternProperties\": {"
		  "    \"^I_\": { \"type\": \"string\" }"
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/properties/I_0\","
		  "    \"schemaHandlerRef\": \"#/patternProperties/%5EI_\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"boolean\"], \"actual\": [\"string\"]"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"patternProperties\": {"
		  "    \"^I_\": { \"type\": \"string\" }"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"},"
		  "    \"I_0\": {\"type\": \"boolean\"}"
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/patternProperties/%5EI_\","
		  "    \"schemaHandlerRef\": \"#/properties/I_0\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"string\"], \"actual\": [\"boolean\"]"
		  "}}");
  // Pattern matches additional properties
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  },"
	  "  \"additionalProperties\": {"
	  "    \"type\": \"boolean\""
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  },"
	  "  \"patternProperties\": {"
	  "    \"^I_\": { \"type\": \"boolean\" }"
	  "  }"
	  "}");
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"required\": [\"a\", \"b\"],"
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  },"
	  "  \"patternProperties\": {"
	  "    \"^I_\": { \"type\": \"boolean\" }"
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"},"
	  "    \"b\": {\"type\": \"boolean\"}"
	  "  },"
	  "  \"additionalProperties\": {"
	  "    \"type\": \"boolean\""
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"additionalProperties\": {"
		  "    \"type\": \"boolean\""
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"patternProperties\": {"
		  "    \"^I_\": { \"type\": \"string\" }"
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/additionalProperties\","
		  "    \"schemaHandlerRef\": \"#/patternProperties/%5EI_\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"boolean\"], \"actual\": [\"string\"]"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"required\": [\"a\", \"b\"],"
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"patternProperties\": {"
		  "    \"^I_\": { \"type\": \"string\" }"
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"string\"},"
		  "    \"b\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"additionalProperties\": {"
		  "    \"type\": \"boolean\""
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/patternProperties/%5EI_\","
		  "    \"schemaHandlerRef\": \"#/additionalProperties\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"string\"], \"actual\": [\"boolean\"]"
		  "}}");
}
TEST(SchemaCompare, ItemsList) {
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"items\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"boolean\""
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"string\""
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#/items\","
		  "    \"schemaIteratorRef\": \"#/items\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"boolean\"], \"actual\": [\"string\"]"
		  "}}");
}
TEST(SchemaCompare, ItemsTuple) {
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"boolean\"},"
	  "    {\"type\": \"integer\"}"
	  "  ]"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"boolean\"},"
	  "    {\"type\": \"integer\"}"
	  "  ]"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"boolean\"},"
		  "    {\"type\": \"integer\"}"
		  "  ]"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"boolean\"},"
		  "    {\"type\": \"string\"}"
		  "  ]"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#/items/2\","
		  "    \"schemaIteratorRef\": \"#/items/2\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"integer\"], \"actual\": [\"string\"]"
		  "}}");
}
TEST(SchemaCompare, ItemsListTuple) {
  COMPARE_SYMMETRIC(
          "{"
	  "  \"type\": \"array\","
	  "  \"items\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"}"
	  "  ]"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"string\""
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"boolean\"}"
		  "  ]"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/items\","
		  "    \"schemaHandlerRef\": \"#/items/2\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"string\"], \"actual\": [\"boolean\"]"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"boolean\"}"
		  "  ]"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"string\""
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/items/2\","
		  "    \"schemaHandlerRef\": \"#/items\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"boolean\"], \"actual\": [\"string\"]"
		  "}}");
}
TEST(SchemaCompare, AdditionalItems) {
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}");
  std::cerr << "HERE" << std::endl;
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"boolean\""
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/additionalItems\", \"schemaHandlerRef\": \"#/additionalItems\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"string\"], \"actual\": [\"boolean\"]"
		  "}}");
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": false"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": false"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": false"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": true"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\", \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"additionalItems\","
		  "    \"expected\": false, \"actual\": true"
		  "}}");
  // Missing item with & without additional items
  COMPARE_SYMMETRIC(
          "{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"boolean\"}"
	  "  ],"
	  "  \"additionalItems\": true"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"boolean\"},"
	  "    {\"type\": \"integer\"}"
	  "  ],"
	  "  \"additionalItems\": true"
	  "}");
  INVALID_COMPARE_ASYMMETRIC(
          "{"
          "  \"type\": \"array\","
          "  \"items\": ["
          "    {\"type\": \"string\"},"
          "    {\"type\": \"boolean\"}"
          "  ],"
          "  \"additionalItems\": false"
          "}",
          "{"
          "  \"type\": \"array\","
          "  \"items\": ["
          "    {\"type\": \"string\"},"
          "    {\"type\": \"boolean\"},"
          "    {\"type\": \"integer\"}"
          "  ]"
          "}",
          "{ \"minItems\": {"
          "    \"errorCode\": 10,"
          "    \"schemaHandlerRef\": \"#\","
          "    \"schemaIteratorRef\": \"#\","
          "    \"expected\": 3, \"actual\": 2"
          "}}",
          "{ \"additionalItems\": {"
          "    \"errorCode\": 12,"
          "    \"schemaIteratorRef\": \"#/items\","
          "    \"schemaHandlerRef\": \"#\","
          "    \"disallowed\": 2"
          "}}");
}
TEST(SchemaCompare, ItemsCount) {
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"minItems\": 2,"
	  "  \"maxItems\": 10"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"minItems\": 2,"
	  "  \"maxItems\": 10"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minItems\": 2,"
		  "  \"maxItems\": 10"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minItems\": 3,"
		  "  \"maxItems\": 10"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"minItems\","
		  "    \"expected\": 2, \"actual\": 3"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minItems\": 2,"
		  "  \"maxItems\": 10"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minItems\": 2,"
		  "  \"maxItems\": 15"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxItems\","
		  "    \"expected\": 10, \"actual\": 15"
		  "}}");
  // Estimate min based on itemsTuple
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"}"
	  "  ]"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"minItems\": 2"
	  "}");
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"minItems\": 2"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"}"
	  "  ]"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"}"
		  "  ]"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minItems\": 2"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"minItems\","
		  "    \"expected\": 1, \"actual\": 2"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"minItems\": 2"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"}"
		  "  ]"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"minItems\","
		  "    \"expected\": 2, \"actual\": 1"
		  "}}");
  // Estimate max based on itemsTuple
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"}"
	  "  ],"
	  "  \"additionalItems\": false"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"maxItems\": 3"
	  "}");
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"maxItems\": 3"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"string\"}"
	  "  ],"
	  "  \"additionalItems\": false"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"}"
		  "  ],"
		  "  \"additionalItems\": true"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"maxItems\": 3"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxItems\","
		  "    \"expected\": 4294967295, \"actual\": 3"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"maxItems\": 3"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"}"
		  "  ],"
		  "  \"additionalItems\": true"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxItems\","
		  "    \"expected\": 3, \"actual\": 4294967295"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"}"
		  "  ],"
		  "  \"additionalItems\": false"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"maxItems\": 2"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxItems\","
		  "    \"expected\": 3, \"actual\": 2"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"maxItems\": 2"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"string\"}"
		  "  ],"
		  "  \"additionalItems\": false"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxItems\","
		  "    \"expected\": 2, \"actual\": 3"
		  "}}");
}
TEST(SchemaCompare, ItemsComplex) {
  // Items matches additional items
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"additionalItems\": {"
	  "    \"type\": \"string\""
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"boolean\""
		  "  }"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"additionalItems\": {"
		  "    \"type\": \"string\""
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/items\","
		  "    \"schemaHandlerRef\": \"#/additionalItems\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"boolean\"], \"actual\": [\"string\"]"
		  "}}");
  // Missing item matches additional items
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"boolean\"},"
	  "    {\"type\": \"integer\"}"
	  "  ]"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"items\": ["
	  "    {\"type\": \"string\"},"
	  "    {\"type\": \"boolean\"}"
	  "  ],"
	  "  \"additionalItems\": {"
	  "    \"type\": \"integer\""
	  "  }"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"boolean\"},"
		  "    {\"type\": \"integer\"}"
		  "  ]"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": ["
		  "    {\"type\": \"string\"},"
		  "    {\"type\": \"boolean\"}"
		  "  ],"
		  "  \"additionalItems\": {"
		  "    \"type\": \"boolean\""
		  "  }"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#/items/2\","
		  "    \"schemaHandlerRef\": \"#/additionalItems\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"integer\"], \"actual\": [\"boolean\"]"
		  "}}");
}
TEST(SchemaCompare, UniqueItems) {
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"uniqueItems\": true"
	  "}",
	  "{"
	  "  \"type\": \"array\","
	  "  \"items\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"uniqueItems\": true"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"uniqueItems\": true"
		  "}",
		  "{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"string\""
		  "  },"
		  "  \"uniqueItems\": false"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"uniqueItems\","
		  "    \"expected\": true, \"actual\": false"
		  "}}");
}
TEST(SchemaCompare, Minimum) {
  COMPARE("{"
	  "  \"type\": \"number\","
	  "  \"minimum\": 0"
	  "}",
	  "{"
	  "  \"type\": \"number\","
	  "  \"minimum\": 0"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"number\","
		  "  \"minimum\": 0"
		  "}",
		  "{"
		  "  \"type\": \"number\","
		  "  \"minimum\": 5"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"minimum\","
		  "    \"expected\": 0, \"actual\": 5"
		  "}}");
}
TEST(SchemaCompare, Maximum) {
  COMPARE("{"
	  "  \"type\": \"number\","
	  "  \"maximum\": 0"
	  "}",
	  "{"
	  "  \"type\": \"number\","
	  "  \"maximum\": 0"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"number\","
		  "  \"maximum\": 0"
		  "}",
		  "{"
		  "  \"type\": \"number\","
		  "  \"maximum\": 5"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maximum\","
		  "    \"expected\": 0, \"actual\": 5"
		  "}}");
}
TEST(SchemaCompare, MultipleOf) {
  COMPARE("{"
	  "  \"type\": \"number\","
	  "  \"multipleOf\": 1"
	  "}",
	  "{"
	  "  \"type\": \"number\","
	  "  \"multipleOf\": 1"
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"number\","
		  "  \"multipleOf\": 1"
		  "}",
		  "{"
		  "  \"type\": \"number\","
		  "  \"multipleOf\": 5"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"multipleOf\","
		  "    \"expected\": 1, \"actual\": 5"
		  "}}");
}
TEST(SchemaCompare, String) {
  COMPARE("{"
	  "  \"type\": \"string\","
	  "  \"minLength\": 1,"
	  "  \"maxLength\": 4,"
	  "  \"pattern\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
	  "}",
	  "{"
	  "  \"type\": \"string\","
	  "  \"minLength\": 1,"
	  "  \"maxLength\": 4,"
	  "  \"pattern\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"string\","
		  "  \"minLength\": 1,"
		  "  \"maxLength\": 4,"
		  "  \"pattern\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
		  "}",
		  "{"
		  "  \"type\": \"string\","
		  "  \"minLength\": 3,"
		  "  \"maxLength\": 4,"
		  "  \"pattern\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"minLength\","
		  "    \"expected\": 1, \"actual\": 3"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"string\","
		  "  \"minLength\": 1,"
		  "  \"maxLength\": 4,"
		  "  \"pattern\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
		  "}",
		  "{"
		  "  \"type\": \"string\","
		  "  \"minLength\": 1,"
		  "  \"maxLength\": 10,"
		  "  \"pattern\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"maxLength\","
		  "    \"expected\": 4, \"actual\": 10"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"string\","
		  "  \"minLength\": 1,"
		  "  \"maxLength\": 4,"
		  "  \"pattern\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\""
		  "}",
		  "{"
		  "  \"type\": \"string\","
		  "  \"minLength\": 1,"
		  "  \"maxLength\": 4,"
		  "  \"pattern\": \"^[0-9]{3}-[0-9]{4}$\""
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"pattern\","
		  "    \"expected\": \"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\","
		  "    \"actual\": \"^[0-9]{3}-[0-9]{4}$\""
		  "}}");
}
TEST(SchemaCompare, AllowSingular) {
  COMPARE("{"
	  "  \"type\": \"array\","
	  "  \"items\": {"
	  "    \"type\": \"string\""
	  "  },"
	  "  \"allowSingular\": true"
	  "}",
	  "{"
	  "  \"type\": \"string\""
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"array\","
		  "  \"items\": {"
		  "    \"type\": \"boolean\""
		  "  },"
		  "  \"allowSingular\": true"
		  "}",
		  "{"
		  "  \"type\": \"string\""
		  "}",
		  "{"
                  "  \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"array\"],"
                  "    \"actual\": [\"string\"]"
		  "  },"
                  "  \"singular\": {"
                  "    \"compare\": {"
		  "      \"errorCode\": 45,"
		  "      \"schemaHandlerRef\": \"#\","
		  "      \"schemaIteratorRef\": \"#/items\","
		  "      \"property\": \"type\","
		  "      \"expected\": [\"boolean\"],"
                  "      \"actual\": [\"string\"]"
		  "    }"
                  "  }"
                  "}");
  COMPARE("{"
	  "  \"type\": \"object\","
	  "  \"properties\": {"
	  "    \"a\": {\"type\": \"string\"}"
	  "  },"
	  "  \"allowSingular\": \"a\""
	  "}",
	  "{"
	  "  \"type\": \"string\""
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"object\","
		  "  \"properties\": {"
		  "    \"a\": {\"type\": \"boolean\"}"
		  "  },"
		  "  \"allowSingular\": \"a\""
		  "}",
		  "{"
		  "  \"type\": \"string\""
		  "}",
		  "{"
                  "  \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaIteratorRef\": \"#\","
                  "    \"schemaHandlerRef\": \"#\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"object\"],"
                  "    \"actual\": [\"string\"]"
		  "  },"
                  "  \"singular\": {"
                  "    \"compare\": {"
		  "      \"errorCode\": 45,"
		  "      \"schemaIteratorRef\": \"#/properties/a\","
                  "      \"schemaHandlerRef\": \"#\","
		  "      \"property\": \"type\","
		  "      \"expected\": [\"boolean\"],"
                  "      \"actual\": [\"string\"]"
		  "    }"
                  "  }"
                  "}");
}
TEST(SchemaCompare, NativeScalars) {
  COMPARE("{"
	  "  \"type\": \"number\""
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"float\""
	  "}");
  COMPARE("{"
	  "  \"type\": \"number\""
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"float\","
	  "  \"precision\": 4"
	  "}");
  COMPARE("{"
	  "  \"type\": \"integer\""
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"int\""
	  "}");
  COMPARE("{"
	  "  \"type\": \"integer\""
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"int\","
	  "  \"precision\": 8"
	  "}");
  COMPARE("{"
	  "  \"type\": \"string\""
	  "}",
	  "{"
	  "  \"type\": \"scalar\","
	  "  \"subtype\": \"string\""
	  "}");
  INVALID_COMPARE("{"
		  "  \"type\": \"integer\""
		  "}",
		  "{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"float\""
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"integer\"],"
		  "    \"actual\": [\"scalar\"]"
		  "}}");
  INVALID_COMPARE("{"
		  "  \"type\": \"number\""
		  "}",
		  "{"
		  "  \"type\": \"scalar\","
		  "  \"subtype\": \"uint\","
		  "  \"precision\": 1"
		  "}",
		  "{ \"compare\": {"
		  "    \"errorCode\": 45,"
		  "    \"schemaHandlerRef\": \"#\","
		  "    \"schemaIteratorRef\": \"#\","
		  "    \"property\": \"type\","
		  "    \"expected\": [\"number\"],"
		  "    \"actual\": [\"scalar\"]"
		  "}}");
}

#undef INVALID_COMPARE_SYMMETRIC
#undef INVALID_COMPARE_ASYMMETRIC
#undef INVALID_COMPARE
#undef COMPARE_SYMMETRIC
#undef COMPARE_SHOW_DOCS

#define GENERATE(schema, s_expected)					\
  {									\
    Document sd, expected, actual;					\
    sd.Parse(schema);							\
    expected.Parse(s_expected);						\
    SchemaDocument s(sd);						\
    SchemaValidator validator(s);					\
    bool result = validator.GenerateData(actual);			\
    EXPECT_TRUE(result);						\
    EXPECT_TRUE(validator.IsValid());					\
    ValidateErrorCode code = validator.GetInvalidSchemaCode();		\
    EXPECT_TRUE(code == kValidateErrorNone);				\
    EXPECT_TRUE(validator.GetInvalidSchemaKeyword() == 0);		\
    if (!result) {							\
      StringBuffer sb;							\
      PrettyWriter<StringBuffer> w(sb);					\
      validator.GetError().Accept(w);					\
      printf("Comparison error: %s\n", sb.GetString());			\
    }									\
    if (actual != expected) {						\
      StringBuffer sb;							\
      PrettyWriter<StringBuffer> w(sb);					\
      actual.Accept(w);							\
      StringBuffer sb0;							\
      PrettyWriter<StringBuffer> w0(sb0);				\
      expected.Accept(w0);						\
      printf("GenerateData() Expected: %s Actual: %s\n", sb0.GetString(), sb.GetString()); \
      ADD_FAILURE();							\
    }									\
  }
#define GENERATE_NOCMP(schema)						\
  {									\
    Document sd, actual;						\
    sd.Parse(schema);							\
    SchemaDocument s(sd);						\
    SchemaValidator validator(s);					\
    bool result = validator.GenerateData(actual);			\
    EXPECT_TRUE(result);						\
    EXPECT_TRUE(validator.IsValid());					\
    ValidateErrorCode code = validator.GetInvalidSchemaCode();		\
    EXPECT_TRUE(code == kValidateErrorNone);				\
    EXPECT_TRUE(validator.GetInvalidSchemaKeyword() == 0);		\
    if (!result) {							\
      StringBuffer sb;							\
      PrettyWriter<StringBuffer> w(sb);					\
      validator.GetError().Accept(w);					\
      printf("Comparison error: %s\n", sb.GetString());			\
    }									\
  }
#define INVALID_GENERATE(schema, error)					\
  {									\
    Document sd, actual;						\
    sd.Parse(schema);							\
    SchemaDocument s(sd);						\
    SchemaValidator validator(s);					\
    bool result = validator.GenerateData(actual);			\
    EXPECT_FALSE(result);						\
    Document e;								\
    e.Parse(error);							\
    SHOW_ERROR_MESSAGE(validator)                                       \
    if (validator.GetError() != e) {					\
      StringBuffer sb;							\
      PrettyWriter<StringBuffer> w(sb);					\
      validator.GetError().Accept(w);					\
      StringBuffer sb_e;						\
      PrettyWriter<StringBuffer> w_e(sb_e);				\
      e.Accept(w_e);							\
      printf("GetError() Expected: %s Actual: %s\n", sb_e.GetString(), sb.GetString()); \
      ADD_FAILURE();							\
    }									\
  }

TEST(SchemaGenerateData, Null) {
  GENERATE("{"
	   "  \"type\": \"null\""
	   "}",
	   "null");
}
  
TEST(SchemaGenerateData, Boolean) {
  GENERATE("{"
	   "  \"type\": \"boolean\""
	   "}",
	   "true");
}
  
TEST(SchemaGenerateData, Integer) {
  GENERATE("{"
	   "  \"type\": \"integer\""
	   "}",
	   "0");
  GENERATE("{"
	   "  \"type\": \"integer\","
	   "  \"minimum\": 3"
	   "}",
	   "4");
  GENERATE("{"
	   "  \"type\": \"integer\","
	   "  \"maximum\": 3"
	   "}",
	   "2");
  GENERATE("{"
	   "  \"type\": \"integer\","
	   "  \"minimum\": 1,"
	   "  \"maximum\": 3"
	   "}",
	   "2");
  GENERATE("{"
	   "  \"type\": \"integer\","
	   "  \"multipleOf\": 3"
	   "}",
	   "15");
  GENERATE("{"
	   "  \"type\": \"integer\","
	   "  \"minimum\": 1,"
	   "  \"multipleOf\": 3"
	   "}",
	   "3");
  GENERATE("{"
	   "  \"type\": \"integer\","
	   "  \"minimum\": 1,"
	   "  \"maximum\": 10,"
	   "  \"multipleOf\": 3"
	   "}",
	   "9");
}
  
TEST(SchemaGenerateData, Number) {
  GENERATE("{"
	   "  \"type\": \"number\""
	   "}",
	   "0.0");
  GENERATE("{"
	   "  \"type\": \"number\","
	   "  \"minimum\": 3"
	   "}",
	   "3.5");
  GENERATE("{"
	   "  \"type\": \"number\","
	   "  \"maximum\": 3"
	   "}",
	   "2.5");
  GENERATE("{"
	   "  \"type\": \"number\","
	   "  \"minimum\": 1.5,"
	   "  \"maximum\": 3.5"
	   "}",
	   "2.5");
  GENERATE("{"
	   "  \"type\": \"number\","
	   "  \"multipleOf\": 2.5"
	   "}",
	   "12.5");
  GENERATE("{"
	   "  \"type\": \"number\","
	   "  \"minimum\": 1.5,"
	   "  \"multipleOf\": 1.3"
	   "}",
	   "2.6");
  GENERATE("{"
	   "  \"type\": \"number\","
	   "  \"minimum\": 1.5,"
	   "  \"maximum\": 10,"
	   "  \"multipleOf\": 1.3"
	   "}",
	   "9.1");
}
TEST(SchemaGenerateData, String) {
  GENERATE("{"
	   "  \"type\": \"string\""
	   "}",
	   "\"abcde\"");
  GENERATE("{"
	   "  \"type\": \"string\","
	   "  \"minLength\": 3"
	   "}",
	   "\"abcd\"");
  GENERATE("{"
	   "  \"type\": \"string\","
	   "  \"maxLength\": 3"
	   "}",
	   "\"ab\"");
  GENERATE("{"
	   "  \"type\": \"string\","
	   "  \"minLength\": 2,"
	   "  \"maxLength\": 4"
	   "}",
	   "\"abc\"");
}
TEST(SchemaGenerateData, Object) {
  GENERATE("{"
	   "  \"type\": \"object\""
	   "}",
	   "{}");
  GENERATE("{"
	   "  \"type\": \"object\","
	   "  \"properties\": {"
	   "    \"a\": {"
	   "       \"type\": \"integer\""
	   "    }"
	   "  }"
	   "}",
	   "{"
	   "  \"a\": 0"
	   "}");
  GENERATE("{"
	   "  \"type\": \"object\","
	   "  \"additionalProperties\": {"
	   "     \"type\": \"integer\""
	   "  }"
	   "}",
	   "{"
	   "  \"a\": 0"
	   "}");
  GENERATE("{"
	   "  \"type\": \"object\","
	   "  \"properties\": {"
	   "    \"a\": {"
	   "       \"type\": \"integer\""
	   "    }"
	   "  },"
	   "  \"additionalProperties\": {"
	   "     \"type\": \"integer\""
	   "  }"
	   "}",
	   "{"
	   "  \"a\": 0,"
	   "  \"b\": 0"
	   "}");
  GENERATE("{"
	   "  \"type\": \"object\","
	   "  \"properties\": {"
	   "    \"a\": {"
	   "       \"type\": \"integer\""
	   "    }"
	   "  },"
	   "  \"additionalProperties\": {"
	   "     \"type\": \"integer\""
	   "  },"
	   "  \"minProperties\": 3"
	   "}",
	   "{"
	   "  \"a\": 0,"
	   "  \"b\": 0,"
	   "  \"c\": 0,"
	   "  \"d\": 0"
	   "}");
  GENERATE("{"
	   "  \"type\": \"object\","
	   "  \"properties\": {"
	   "    \"a\": {"
	   "       \"type\": \"integer\""
	   "    }"
	   "  },"
	   "  \"minProperties\": 3"
	   "}",
	   "{"
	   "  \"a\": 0,"
	   "  \"b\": null,"
	   "  \"c\": null,"
	   "  \"d\": null"
	   "}");
}
TEST(SchemaGenerateData, Array) {
  GENERATE("{"
	   "  \"type\": \"array\""
	   "}",
	   "[]");
  GENERATE("{"
	   "  \"type\": \"array\","
	   "  \"items\": ["
	   "    {"
	   "      \"type\": \"integer\""
	   "    }"
	   "  ]"
	   "}",
	   "[0]");
  GENERATE("{"
	   "  \"type\": \"array\","
	   "  \"items\": {"
	   "    \"type\": \"integer\""
	   "  }"
	   "}",
	   "[0]");
  GENERATE("{"
	   "  \"type\": \"array\","
	   "  \"items\": {"
	   "    \"type\": \"integer\""
	   "  },"
	   "  \"minItems\": 3"
	   "}",
	   "[0, 0, 0, 0]");
  GENERATE("{"
	   "  \"type\": \"array\","
	   "  \"minItems\": 3"
	   "}",
	   "[null, null, null, null]");
}

#define MAKE_TEST_SCALAR_(subT, prec, result_scalar, result_array)	\
  TEST(SchemaGenerateData, Scalar_ ## subT ## prec) {			\
    GENERATE("{"							\
	     "  \"type\": \"scalar\","					\
	     "  \"subtype\": \"" #subT "\","				\
	     "  \"precision\": " #prec					\
	     "}",							\
	     "\"" result_scalar "\"");					\
  }									\
  TEST(SchemaGenerateData, NDArray_ ## subT ## prec) {			\
    GENERATE("{"							\
	     "  \"type\": \"ndarray\","					\
	     "  \"subtype\": \"" #subT "\","				\
	     "  \"precision\": " #prec ","				\
	     "  \"shape\": [2, 3]"					\
	     "}",							\
	     "\"" result_array "\"");					\
  }
MAKE_TEST_SCALAR_(int, 1, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImludCIsInByZWNpc2lvbiI6MX0=-YGG-AA==-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJpbnQiLCJwcmVjaXNpb24iOjEsInNoYXBlIjpbMiwzXX0=-YGG-AAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(int, 2, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImludCIsInByZWNpc2lvbiI6Mn0=-YGG-AAA=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJpbnQiLCJwcmVjaXNpb24iOjIsInNoYXBlIjpbMiwzXX0=-YGG-AAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(int, 4, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImludCIsInByZWNpc2lvbiI6NH0=-YGG-AAAAAA==-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJpbnQiLCJwcmVjaXNpb24iOjQsInNoYXBlIjpbMiwzXX0=-YGG-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(int, 8, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImludCIsInByZWNpc2lvbiI6OH0=-YGG-AAAAAAAAAAA=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJpbnQiLCJwcmVjaXNpb24iOjgsInNoYXBlIjpbMiwzXX0=-YGG-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(uint, 1, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjF9-YGG-AA==-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoxLCJzaGFwZSI6WzIsM119-YGG-AAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(uint, 2, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjJ9-YGG-AAA=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjoyLCJzaGFwZSI6WzIsM119-YGG-AAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(uint, 4, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjR9-YGG-AAAAAA==-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjo0LCJzaGFwZSI6WzIsM119-YGG-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(uint, 8, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InVpbnQiLCJwcmVjaXNpb24iOjh9-YGG-AAAAAAAAAAA=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJ1aW50IiwicHJlY2lzaW9uIjo4LCJzaGFwZSI6WzIsM119-YGG-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(float, 2, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImZsb2F0IiwicHJlY2lzaW9uIjoyfQ==-YGG-AAA=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJmbG9hdCIsInByZWNpc2lvbiI6Miwic2hhcGUiOlsyLDNdfQ==-YGG-AAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(float, 4, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImZsb2F0IiwicHJlY2lzaW9uIjo0fQ==-YGG-AAAAAA==-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJmbG9hdCIsInByZWNpc2lvbiI6NCwic2hhcGUiOlsyLDNdfQ==-YGG-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(float, 8, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImZsb2F0IiwicHJlY2lzaW9uIjo4fQ==-YGG-AAAAAAAAAAA=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJmbG9hdCIsInByZWNpc2lvbiI6OCwic2hhcGUiOlsyLDNdfQ==-YGG-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(complex, 8, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImNvbXBsZXgiLCJwcmVjaXNpb24iOjh9-YGG-AAAAAAAAAAA=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJjb21wbGV4IiwicHJlY2lzaW9uIjo4LCJzaGFwZSI6WzIsM119-YGG-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(complex, 16, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImNvbXBsZXgiLCJwcmVjaXNpb24iOjE2fQ==-YGG-AAAAAAAAAAAAAAAAAAAAAA==-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJjb21wbGV4IiwicHJlY2lzaW9uIjoxNiwic2hhcGUiOlsyLDNdfQ==-YGG-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-YGG-")
MAKE_TEST_SCALAR_(string, 5, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6InN0cmluZyIsInByZWNpc2lvbiI6NX0=-YGG-YWJjZGU=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJzdHJpbmciLCJwcmVjaXNpb24iOjUsInNoYXBlIjpbMiwzXX0=-YGG-YWJjZGVhYmNkZWFiY2RlYWJjZGVhYmNkZWFiY2Rl-YGG-")
#undef MAKE_TEST_SCALAR_

#define MAKE_TEST_SCALAR_FULL_(subT, prec, result_scalar, result_array)	\
  TEST(SchemaGenerateData, FullScalar_ ## subT ## prec) {		\
    GENERATE("{"							\
	     "  \"type\": \"scalar\","					\
	     "  \"subtype\": \"" #subT "\","				\
	     "  \"precision\": " #prec ","				\
	     "  \"units\": \"cm\","					\
	     "  \"minimum\": 1.5,"					\
	     "  \"maximum\": 10,"					\
	     "  \"multipleOf\": 1.3"					\
	     "}",							\
	     "\"" result_scalar "\"");					\
  }									\
  TEST(SchemaGenerateData, FullNDArray_ ## subT ## prec) {		\
    GENERATE("{"							\
	     "  \"type\": \"ndarray\","					\
	     "  \"subtype\": \"" #subT "\","				\
	     "  \"precision\": " #prec ","				\
	     "  \"units\": \"cm\","					\
	     "  \"shape\": [2, 3],"					\
	     "  \"minimum\": 1.5,"					\
	     "  \"maximum\": 10,"					\
	     "  \"multipleOf\": 1.3"					\
	     "}",							\
	     "\"" result_array "\"");					\
  }
MAKE_TEST_SCALAR_FULL_(float, 8, "-YGG-eyJ0eXBlIjoic2NhbGFyIiwic3VidHlwZSI6ImZsb2F0IiwicHJlY2lzaW9uIjo4LCJ1bml0cyI6ImNtIn0=-YGG-MzMzMzMzIkA=-YGG-", "-YGG-eyJ0eXBlIjoibmRhcnJheSIsInN1YnR5cGUiOiJmbG9hdCIsInByZWNpc2lvbiI6OCwidW5pdHMiOiJjbSIsInNoYXBlIjpbMiwzXX0=-YGG-MzMzMzMzIkAzMzMzMzMiQDMzMzMzMyJAMzMzMzMzIkAzMzMzMzMiQDMzMzMzMyJA-YGG-")
#undef MAKE_TEST_SCALAR_FULL_
TEST(SchemaGenerateData, ObjWavefront) {
  GENERATE("{"
	   "  \"type\": \"obj\""
	   "}",
	   "\"-YGG-eyJ0eXBlIjoib2JqIn0=-YGG-diAwLjAgMC4wIDAuMAp2IDAuMCAwLjAgMS4wCnYgMC4wIDEuMCAxLjAKdiAwLjAgMS4wIDAuMAp2IDEuMCAwLjAgMC4wCnYgMS4wIDAuMCAxLjAKdiAxLjAgMS4wIDEuMAp2IDEuMCAxLjAgMC4wCmYgNCAxIDIKZiA0IDEgMwpsIDEgMgpsIDIgMwpsIDMgNApsIDQgMQpsIDMgMQo=-YGG-\"");
}
TEST(SchemaGenerateData, Ply) {
  GENERATE("{"
	   "  \"type\": \"ply\""
	   "}",
	   "\"-YGG-eyJ0eXBlIjoicGx5In0=-YGG-cGx5CmZvcm1hdCBhc2NpaSAxLjAKZWxlbWVudCB2ZXJ0ZXggOApwcm9wZXJ0eSBkb3VibGUgeApwcm9wZXJ0eSBkb3VibGUgeQpwcm9wZXJ0eSBkb3VibGUgegplbGVtZW50IGZhY2UgMgpwcm9wZXJ0eSBsaXN0IHVjaGFyIGludCB2ZXJ0ZXhfaW5kZXgKZWxlbWVudCBlZGdlIDUKcHJvcGVydHkgaW50IHZlcnRleDEKcHJvcGVydHkgaW50IHZlcnRleDIKZW5kX2hlYWRlcgowIDAgMAowIDAgMQowIDEgMQowIDEgMAoxIDAgMAoxIDAgMQoxIDEgMQoxIDEgMAozIDMgMCAxCjMgMyAwIDIKMCAxCjEgMgoyIDMKMyAwCjIgMAo=-YGG-\"");
}
#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
TEST(SchemaGenerateData, PythonClass) {
  GENERATE_NOCMP("{"
		 "  \"type\": \"class\""
		 "}");
}
TEST(SchemaGenerateData, PythonFunction) {
  GENERATE_NOCMP("{"
		 "  \"type\": \"function\""
		 "}");
}
TEST(SchemaGenerateData, PythonInstance) {
  GENERATE_NOCMP("{"
		 "  \"type\": \"instance\""
		 "}");
}
#endif // YGGDRASIL_DISABLE_PYTHON_C_API
#endif // RAPIDJSON_YGGDRASIL

#if defined(_MSC_VER) || defined(__clang__)
RAPIDJSON_DIAG_POP
#endif
