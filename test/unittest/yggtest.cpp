#include "unittest.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/memorybuffer.h"
#include "rapidjson/yggdrasil.h"

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(c++98-compat)
#endif

using namespace rapidjson;

#ifdef RAPIDJSON_YGGDRASIL

#define TEST_ENCODE(schema, json, head)					\
  {									\
    Document d;								\
    Document sd;							\
    Document hd;							\
    hd.Parse(head);							\
    d.Parse(json, hd);							\
    sd.Parse(schema);							\
    SchemaDocument s(sd);						\
    SchemaValidator validator(s);					\
    EXPECT_FALSE(d.HasParseError());					\
    EXPECT_TRUE(d.Accept(validator));					\
    EXPECT_TRUE(validator.IsValid());					\
    if (!validator.IsValid()) {						\
      StringBuffer sb;							\
      validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);	\
      printf("Invalid schema: %s\n", sb.GetString());			\
      printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword()); \
      sb.Clear();							\
      validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);	\
      printf("Invalid document: %s\n", sb.GetString());			\
      sb.Clear();							\
      Writer<StringBuffer> w(sb);					\
      validator.GetError().Accept(w);					\
      printf("Validation error: %s\n", sb.GetString());			\
    }									\
    /* Test encode */							\
    Document generated;							\
    StringBuffer generated_buffer;					\
    Writer<StringBuffer> generated_writer(generated_buffer);		\
    SchemaEncoder encoder(d);						\
    generated.Populate(encoder);					\
    generated.Accept(generated_writer);					\
    EXPECT_STREQ(schema, generated_buffer.GetString());			\
  }

// json -> parse -> writer -> json
/*
#define TEST_ROUNDTRIP(json, schema)		\
  {							\
    Document schema_d;					\
    Document json_d;					\
    schema_d.Parse(schema);				\
    json_d.Parse(json);					\
    SchemaDocument schema_sd(schema_d);			\
    SchemaNormalizer normalizer(schema_sd);		\
    EXPECT_FALSE(schema_d.HasParseError());		\
    EXPECT_FALSE(json_d.HasParseError());		\
    EXPECT_TRUE(json_d.Accept(normalizer));		\
    EXPECT_TRUE(normalizer.IsValid());			\
    StringBuffer buffer1;				\
    Writer<StringBuffer> writer1(buffer1);		\
    EXPECT_TRUE(json_d.Accept(writer1));		\
    EXPECT_STREQ(json, buffer1.GetString());		\
    StringBuffer buffer2;				\
    Writer<StringBuffer> writer2(buffer2);			 \
    EXPECT_TRUE(normalizer.GetNormalized().Accept(writer2));	 \
    EXPECT_STREQ(json, buffer2.GetString());			 \
  }
*/

TEST(SchemaEncoder, Numbers) {
  TEST_ENCODE("{\"type\":\"integer\"}", "1", "");
  TEST_ENCODE("{\"type\":\"number\"}", "1.0", "");
}

TEST(SchemaEncoder, String) {
  TEST_ENCODE("{\"type\":\"string\"}", "\"hello\"", "");
}

TEST(SchemaEncoder, Null) {
  TEST_ENCODE("{\"type\":\"null\"}", "null", "");
}

TEST(SchemaEncoder, Boolean) {
  TEST_ENCODE("{\"type\":\"boolean\"}", "true", "");
  TEST_ENCODE("{\"type\":\"boolean\"}", "false", "");
}

TEST(SchemaEncoder, Array) {
  TEST_ENCODE("{\"type\":\"array\",\"items\":["
	      "{\"type\":\"integer\"},{\"type\":\"string\"}]}",
	      "[1,\"hello\"]", "");
}

TEST(SchemaEncoder, Object) {
  TEST_ENCODE("{\"type\":\"object\",\"properties\":{"
	      "\"a\":{\"type\":\"integer\"},\"b\":{\"type\":\"string\"}}}",
	      "{\"a\":1,\"b\":\"hello\"}", "");
}

/*
TEST(SchemaEncoder, ScalarInt) {
  TEST_ENCODE("{\"type\":\"scalar\", \"subtype\":\"int\", \"precision\":32}",
              "\"AQAAAA==\n\"",
		 "{\"type\":\"scalar\", \"subtype\":\"int\", \"precision\":32}");
}
*/

#endif // RAPIDJSON_YGGDRASIL

#ifdef __clang__
RAPIDJSON_DIAG_POP
#endif
