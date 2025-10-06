#ifdef RAPIDJSON_YGGDRASIL

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

TEST(Type, Validation) {
  Document sd;
  sd.Parse("{ \"type\": \"integer\" }");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);

  {
    Document d;
    d.Parse("42");
    assert(!d.HasParseError());
    
    d.Accept(validator);
    assert(validator.IsValid());
  }
 
  validator.Reset();
  {
    Document d;
    d.Parse("\"hello\"");
    assert(!d.HasParseError());
    
    d.Accept(validator);
    assert(!validator.IsValid());
  }

  // validator.Reset();
  // {
  //   Document d;
  //   d.Parse("42.0");
  //   assert(!d.HasParseError());
  //   d.Accept(validator);
  //   assert(!validator.IsValid());

  //   SchemaNormalizer normalizer(s);
  //   d.Accept(normalizer);
  //   assert(normalizer.IsValid());
  //   const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
  //   assert(n.IsInt());
  // }
}

TEST(Keyword, AllowSingular) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"array\", "
      "  \"allowSingular\": true, "
      "  \"items\": {\"type\": \"string\"}"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.Parse("\"hello\"");
  assert(!d.HasParseError());
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  std::cout << normalizer.GetNormalized() << std::endl;
  // ["hello"]
}

TEST(Keyword, AllowWrapped) {
  Document sd;
  sd.Parse("{\"type\": \"string\", \"allowWrapped\": true}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.Parse("[\"hello\"]");
  assert(!d.HasParseError());
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  std::cout << normalizer.GetNormalized() << std::endl;
  // "hello"
}

TEST(Keyword, Default) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"object\","
      "  \"required\": [\"size\"],"
      "  \"properties\": {"
      "    \"size\": {"
      "      \"type\": \"number\","
      "      \"default\": 5"
      "    }"
      "  }"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.Parse("{}");
  assert(!d.HasParseError());
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  std::cout << normalizer.GetNormalized() << std::endl;
  // {"size": 5}
}

TEST(Keyword, Aliases) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"object\","
      "  \"properties\": {"
      "     \"street_address\": { \"type\": \"string\","
      "                           \"aliases\": [\"street\"] }},"
      "  \"required\": [\"street_address\"]"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.Parse("{ \"street\": \"1600 Pennsylvania Ave.\" }");
  assert(!d.HasParseError());
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  std::cout << normalizer.GetNormalized() << std::endl;
  // {"street_address": "1600 Pennsylvania Ave."}
}

TEST(Keyword, Deprecated) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"object\","
      "  \"properties\": {"
      "     \"old_valid\": {"
      "        \"type\": \"string\","
      "        \"deprecated\": \"Deprecation message\"},"
      "     \"valid\": {"
      "        \"type\": \"integer\"}"
      "  }"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.Parse("{\"old_valid\": \"string\", \"valid\": 0}");
  assert(!d.HasParseError());
  
  d.Accept(validator);
  assert(validator.IsValid());
  assert(validator.HasWarning());

  // Same for normalization
}

TEST(Keyword, PushProperties) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"object\","
      "  \"properties\": {"
      "    \"billing_address\": {"
      "      \"type\": \"object\","
      "      \"properties\": {"
      "        \"street_address\": {"
      "          \"type\": \"string\""
      "        },"
      "        \"city\": {"
      "          \"type\": \"string\""
      "        }"
      "      },"
      "      \"required\": [\"street_address\", \"city\"],"
      "      \"default\": {}"
      "    },"
      "    \"shipping_address\": {"
      "      \"type\": \"object\","
      "      \"properties\": {"
      "        \"street_address\": {"
      "          \"type\": \"string\""
      "        },"
      "        \"city\": {"
      "          \"type\": \"string\""
      "        }"
      "      },"
      "      \"required\": [\"street_address\", \"city\"],"
      "      \"pushProperties\": {"
      "        \"../billing_address\": true"
      "      }"
      "    }"
      "  },"
      "  \"required\": [\"shipping_address\", \"billing_address\"]"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.Parse(
      "{"
      "  \"shipping_address\": {"
      "    \"street_address\": \"1600 Pennsylvania Avenue NW\","
      "    \"city\": \"Washington\""
      "  }"
      "}");
  assert(!d.HasParseError());
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  std::cout << normalizer.GetNormalized() << std::endl;
  // {
  //   "shipping_address": {
  //     "street_address": "1600 Pennsylvania Ave.",
  //     "city": "Washington"
  //   },
  //   "billing_address": {
  //     "street_address": "1600 Pennsylvania Ave.",
  //     "city": "Washington"
  //   },
  // }
}

TEST(Keyword, PullProperties) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"object\","
      "  \"properties\": {"
      "    \"billing_address\": {"
      "      \"type\": \"object\","
      "      \"properties\": {"
      "        \"street_address\": {"
      "          \"type\": \"string\""
      "        },"
      "        \"city\": {"
      "          \"type\": \"string\""
      "        }"
      "      },"
      "      \"required\": [\"street_address\", \"city\"],"
      "      \"default\": {},"
      "      \"pullProperties\": {"
      "        \"../shipping_address\": true"
      "      }"
      "    },"
      "    \"shipping_address\": {"
      "      \"type\": \"object\","
      "      \"properties\": {"
      "        \"street_address\": {"
      "          \"type\": \"string\""
      "        },"
      "        \"city\": {"
      "          \"type\": \"string\""
      "        }"
      "      },"
      "      \"required\": [\"street_address\", \"city\"]"
      "    }"
      "  },"
      "  \"required\": [\"shipping_address\", \"billing_address\"]"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.Parse(
      "{"
      "  \"shipping_address\": {"
      "    \"street_address\": \"1600 Pennsylvania Avenue NW\","
      "    \"city\": \"Washington\""
      "  }"
      "}");
  assert(!d.HasParseError());
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  std::cout << normalizer.GetNormalized() << std::endl;
  // {
  //   "shipping_address": {
  //     "street_address": "1600 Pennsylvania Ave.",
  //     "city": "Washington"
  //   },
  //   "billing_address": {
  //     "street_address": "1600 Pennsylvania Ave.",
  //     "city": "Washington"
  //   },
  // }
}

TEST(Keyword, Subtype) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"scalar\","
      "  \"subtype\": \"float\","
      "  \"precision\": 8"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.SetInt(5);
  assert(d.IsInt());
  
  d.Accept(validator);
  assert(!validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
  assert(n.IsScalar<double>());
  assert(!n.IsInt64());
}

TEST(Keyword, Precision) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"scalar\","
      "  \"subtype\": \"float\","
      "  \"precision\": 8"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.SetScalar((float16_t)1.6, d.GetAllocator());
  assert(d.IsScalar<float16_t>());
  assert(d.GetPrecision() == 2);
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
  assert(n.IsScalar<double>());
  assert(n.GetPrecision() == 8);
}

TEST(Keyword, Units) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"scalar\","
      "  \"subtype\": \"float\","
      "  \"precision\": 8,"
      "  \"units\": \"g\""
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.SetScalar(1.6, "kg", d.GetAllocator());
  assert(d.IsScalar<double>());
  assert(d.HasUnits());
  assert(d.GetUnits() == "kg");
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
  assert(n.IsScalar<double>());
  assert(n.HasUnits());
  assert(n.GetUnits() == "g");
}

TEST(Keyword, Shape) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"ndarray\","
      "  \"subtype\": \"float\","
      "  \"precision\": 8,"
      "  \"shape\": [2, 3]"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);

  Document d;
  d.SetNDArray({{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}}, d.GetAllocator());
  assert(d.IsNDArray<double>());
  
  d.Accept(validator);
  assert(validator.IsValid());

  // TODO: Allow reshape?
  // validator.Reset();
  // {
  //   Document d;
  //   d.SetNDArray({{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}}, d.GetAllocator());
  //   d.Accept(validator);
  //   assert(!validator.IsValid());

  //   SchemaNormalizer normalizer(s);
  //   d.Accept(normalizer);
  //   assert(normalizer.IsValid());
  //   const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
  //   assert(n.GetShape().Size() == 2);
  //   assert(n.GetShape()[0] == 2);
  //   assert(n.GetShape()[1] == 3);
  // }
}

TEST(Keyword, NDim) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"ndarray\","
      "  \"subtype\": \"float\","
      "  \"precision\": 8,"
      "  \"ndim\": 2"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);

  Document d;
  d.SetNDArray({{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}}, d.GetAllocator());
  assert(d.IsNDArray<double>());
  
  d.Accept(validator);
  assert(validator.IsValid());
}

TEST(Keyword, Length) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"ndarray\","
      "  \"subtype\": \"float\","
      "  \"precision\": 8,"
      "  \"length\": 3"
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.SetNDArray({1.0, 2.0, 3.0}, d.GetAllocator());
  assert(d.IsNDArray<double>());
  
  d.Accept(validator);
  assert(validator.IsValid());
}

TEST(Keyword, Encoding) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"scalar\","
      "  \"subtype\": \"string\","
      "  \"encoding\": \"UCS4\""
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.SetScalar("hello", "ASCII", d.GetAllocator());
  assert(d.IsScalar<Document::Ch>());
  assert(d.HasEncoding());
  assert(d.GetEncoding() == "ASCII");
  
  d.Accept(validator);
  assert(!validator.IsValid()); // TODO: should be false
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
  assert(n.HasEncoding());
  assert(n.GetEncoding() == "UCS4");
}

TEST(Keyword, Class) {
  Document sd;
  sd.Parse(
      "{"
      "  \"type\": \"instance\","
      "  \"class\": \"example_python:ExampleSubClass\""
      "}");
  assert(!sd.HasParseError());
  SchemaDocument s(sd);
  SchemaValidator validator(s);
  
  Document d;
  d.Parse(
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
      "}");
  assert(!d.HasParseError());
  assert(d.IsObject());
  
  d.Accept(validator);
  assert(validator.IsValid());
  
  SchemaNormalizer normalizer(s);
  d.Accept(normalizer);
  assert(normalizer.IsValid());
  const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
  assert(n.IsPythonInstance());
}

#endif // RAPIDJSON_YGGDRASIL
