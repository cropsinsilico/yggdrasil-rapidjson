# Schema

JSON Schema is a draft standard for describing the format of JSON data. The schema itself is also JSON data. RapidJSON added support for [validating JSON](https://rapidjson.org/md_doc_schema.html) structures against JSON schema in v1.1.0. YggdrasilRapidJSON extended this by adding support for additional properties that can be specified (mostly pertaining to the added data types), normalization of JSON structures using JSON schema, and some additional utilities for interactiving with the schema classes.

Because YggdrasilRapidJSON JSON schema features are built upon RapidJSON's JSON schema classes & functions, refer to the [RapidJSON JSON schema](https://rapidjson.org/md_doc_schema.html) documentation for additional information on the schema implementation and examples.

Like RapidJSON's JSON schema features, YggdrasilRapidJSON's schema features implement [JSON Schema Draft v4](http://json-schema.org/documentation.html). If you are not familiar with JSON Schema, you may refer to [Understanding JSON Schema](http://spacetelescope.github.io/understanding-json-schema/).


[TOC]

# Validation

Validation allows for JSON documents to be validated against JSON schema to determine if the JSON conforms to the expected format. For a description of validation, see [RapidJSON's documentation](https://rapidjson.org/md_doc_schema.html).

# Normalization

JSON normalization modifies the JSON structure (when possible) to be compliant with a JSON schema. If the structure cannot be normalized, validation errors will be set.

The normalizer can be used in all of the same ways as the RapidJSON validator (e.g. serialization, with remote schemas). Use of normalizer as in the examples from the RapidJSON documentation will be the same with the replacement of "Validator" with "Normalizer". The one exception of this is the lack of a normalization equivalent to the `SchemaValidatingReader` helper class which will be implemented in a future release.

## Basic Usage {#Basic}

First of all, you need to parse a JSON Schema into `Document`, and then compile the `Document` into a `SchemaDocument`.

Secondly, construct a `SchemaNormalizer` with the `SchemaDocument`. It is similar to a `Writer` in the sense of handling SAX events. So, you can use `document.Accept(normalizer)` to normalize a document, check the validity, and then get the normalized document if it is valid.

~~~cpp
#include "rapidjson/schema.h"

// ...

Document sd;
if (sd.Parse(schemaJson).HasParseError()) {
    // the schema is not a valid JSON.
    // ...       
}

SchemaDocument schema(sd); // Compile a Document to SchemaDocument
if (!schema.GetError().ObjectEmpty()) {
    // there was a problem compiling the schema
    StringBuffer sb;
    Writer<StringBuffer> w(sb);
    schema.GetError().Accept(w);
    printf("Invalid schema: %s\n", sb.GetString());
}
// sd is no longer needed here.

Document d;
if (d.Parse(inputJson).HasParseError()) {
    // the input is not a valid JSON.
    // ...       
}

SchemaNormalizer normalizer(schema);
if (!d.Accept(normalizer)) {
    // Input JSON is invalid according to the schema
    // Output diagnostic information
    StringBuffer sb;
    normalizer.GetInvalidSchemaPointer().StringifyUriFragment(sb);
    printf("Invalid schema: %s\n", sb.GetString());
    printf("Invalid keyword: %s\n", normalizer.GetInvalidSchemaKeyword());
    sb.Clear();
    normalizer.GetInvalidDocumentPointer().StringifyUriFragment(sb);
    printf("Invalid document: %s\n", sb.GetString());
}
const Value& normalized = normalizer.GetNormalized();

~~~


# Extension properties {#Extension}

In addition to the JSON schema standard implemented by RapidJSON, YggdrasilRapidJSON supports several additional schema keywords & keyword values for validating/normalizing YggdrasilRapidJSON documents.

## Validation keywords for any instance type {#AnyTypes}

### type {#type}

#### Description

`[string, array]` Expected type or types. In addition to the types supported by RapidJSON from the JSON Schema Draft 04 Core specification, YggdrasilRapidJSON allows for several additional types to be specified:

* `scalar`: Single numeric or string value with defined `subtype`, `precision`, and optional `units`. Schemas for scalars with `string` subtype can also optionally have an `encoding` keyword.
* `ndarray`: Uniform N-dimensional array of numeric or string values with defined `subtype`, `precision`, `ndim`, `shape`, and optional `units`. Schemas for ND arrays with `string` subtype can also optionally have an `encoding` keyword.
* `1darray`: Uniform 1-dimensional array of numeric or string values with defined `subtype`, `precision`, `length`, and optional `units`. Schemas for 1D arrays with `string` subtype can also optionally have an `encoding` keyword. Internally 1D arrays are stored as ND arrays and not a unique type, but this type is supported for convenience.
* `python_class`: Python class on the current path.
* `python_function`: Python function on the current path.
* `python_instance`: Python instance that can be constructed from a `python_class` on the current path.
* `obj`: ObjWavefront 3D mesh.
* `ply`: Ply 3D mesh.
* `schema`: JSON schema.
* `any`: Explicitly allow any type (from JSON core or YggdrasilRapidJSON extension types).

#### Role in validation

If the type of the JSON document does not match the type(s) specified, the following error will be set:

`Property has a type '%actual' that is not in the following list: '%expected'.`

* `expected`: The types allowed by the `type` schema keyword.
* `actual`: The primitive type of the instance.

~~~cpp
Document sd;
sd.Parse("{ \"type\": \"integer\" }");
assert(!sd.HasParseError());
SchemaDocument s(sd);
SchemaValidator validator(s);
Document d;
d.Parse("42");
assert(!d.HasParseError());
d.Accept(validator);
assert(validator.IsValid());
~~~

#### Role in normalization

Some transformation between JSON primitive types and scalar/ndarray types are allowed during normalization. See the [subtype section](@ref subtype) below for details on those transformations.

## Schema keywords for collections (arrays & objects)

### allowSingular {#allowSingular}

#### Description

`[boolean, string]` If `true`, JSON that is valid for the first `item` schema (for array schemas) or `property` schema will also be considered valid. For objects, this can also be a property name, in which case the schema for that property will be used.

#### Role in validation

JSON documents validated against schemas with `allowSingular` set will be valid if they match either the wrapped or unwrapped schemas.

~~~cpp
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
~~~

#### Role in normalization

If the normalized JSON validates against the "unwrapped" version of the schema (outside an array/object), the normalized document will consist of the "wrapped" version with the wrapping array/object added.

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
std::cout << normalizer.GetNormalized() << std::endl;
// ["hello"]
~~~

### allowWrapped {#allowWrapped}

#### Description

`[boolean, string]` If `true`, JSON that is valid for an array or object containing the JSON (as a single item/property) will also be considered valid. For objects, this should be a string specifying the property name that will be considered valid.

#### Role in validation

JSON documents validated against schemas with `allowWrapped` set will be valid if they match either the wrapped or unwrapped schemas.

~~~cpp
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
~~~

#### Role in normalization

If the normalized JSON validates against the "wrapped" version of the schema (inside an array/object), the normalized document will consist of the "unwrapped" version with the wrapping array/object removed.

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
std::cout << normalizer.GetNormalized() << std::endl;
// "hello"
~~~

## Schema keywords for objects {#Objects}

### default {#default}

#### Description

`[any]` Default value for a property. While RapidJSON allowed for this to be specified for `string` properties and validated JSON objects missing required properties if a `default` was specified, YggdrasilRapidJSON allows the default to be of any type and uses this keyword in normalization.

#### Role in validation

If a required property is missing from a validated JSON object, but a default is provided, the document will be considered valid.

~~~cpp
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
~~~

#### Role in normalization

If a required property is missing from a normalized JSON object, the default will be added.

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
std::cout << normalizer.GetNormalized() << std::endl;
// {"size": 5}
~~~

### aliases {#aliases}

#### Description

`[array]` List of alternate property names that should also validate against the schema that contains the `aliases` keyword.

#### Role in validation

Aliased properties will validate against JSON schema containing the `aliases` property if the aliased properties validate against the unaliased property schema.

~~~cpp
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
~~~

#### Role in normalization

Aliased properties will be migrated to the unaliased property name.

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
std::cout << normalizer.GetNormalized() << std::endl;
// {"street_address": "1600 Pennsylvania Ave."}
~~~

### deprecated {#deprecated}

#### Description

`[boolean, string]` Mark a property as deprecated so that a warning is emitted when a deprecated property is present during validation. The value of the `deprecated` keyword can contain the warning message that should be emitted as a string.

#### Role in validation

A warning will be issued if a deprecated property is present.

~~~cpp
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
~~~

#### Role in normalization

Same as validation.

### pushProperties {#pushProperties}

#### Description

`[object]` Allow properties from JSON objects to be pushed to other location in the document to satisfy missing required properties. Keys in the `pushProperties` object should be the location that properties specified in the value should be pushed to. These can be absolute schema addresses or relative to the address of the schema containing the `pushProperties` keyword. Values can be boolean to specify all (`true`) or none (`false`) of the properties in the current schema, or an array of names of properties that should be pushed.

#### Role in validation

If missing required properties can be satified by pushing the properties as specified, the document will be considered valid.

~~~cpp
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
~~~

#### Role in normalization

The pushed properties will be copied to the specified locations in the normalized document to satisfy missing required properties.

~~~cpp
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
~~~

### pullProperties {#pullProperties}

#### Description

`[object]` Allow properties from JSON objects to be pulled from other location in the document to satisfy missing required properties. Keys in the `pullProperties` object should be the location that properties specified in the value should be pulled from. These can be absolute schema addresses or relative to the address of the schema containing the `pullProperties` keyword. Values can be boolean to specify all (`true`) or none (`false`) of the properties in the current schema, or an array of names of properties that should be pulled.

#### Role in validation

If missing required properties can be satified by pulling the properties as specified, the document will be considered valid.

~~~cpp
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
~~~

#### Role in normalization

The pulled properties will be copied from the specified locations in the normalized document to satisfy missing required properties.

~~~cpp
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
~~~

## Keywords for scalars (and ndarrays) {#Scalars}

### subtype {#subtype}

#### Description

`[string, array]` The base type for `scalar` JSON values or elements in `ndarray` JSON values. Valid values include:

* `float`: Floating point number
* `int`: Signed integer
* `uint`: Unsigned integer
* `complex`: Complex number
* `string`: String
* `bytes`: Special case of `string` with `encoding="ASCII"`
* `unicode`: Special case of `string` with `encoding="UCS4"`
* `any`: Any subtype is allowed

#### Role in validation

If the subtype of the JSON document does not match the subtype(s) specified, the following error will be set:

`Property has a subtype '%actual' that is not in the following list '%expected'.`

* `expected`: The subtypes allowed by the `subtype` schema keyword.
* `actual`: The scalar/ndarray subtype of the instance.

~~~cpp
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
~~~

#### Role in normalization

The following type transformation are allowed to normalize a document to the subtype(s) specified by a schema if the precision schema property can also be normalized. When the instance satisfies multiple specified by the schema under normalization, the first one will be used in the normalized document.

#### Normalizable to float subtype

* `{"type": "scalar", "subtype": "int"}`
* `{"type": "scalar", "subtype": "uint"}`
* `{"type": "number"}`
* `{"type": "integer"}`

#### Normalizable to int subtype

* `{"type": "scalar", "subtype": "uint"}`
* `{"type": "integer"}`
* `{"type": "scalar", "subtype": "float"}` if the instance is a whole number
* `{"type": "number"}` if the instance is a whole number

#### Normalizable to uint subtype
* `{"type": "scalar", "subtype": "int"}` if the instance is >=0
* `{"type": "integer"}` if the instance is >=0
* `{"type": "scalar", "subtype": "float"}` if the instance is a positive whole number
* `{"type": "number"}` if the instance is a positive whole number

#### Normalizable to string subtype
* `{"type": "string"}`

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
assert(n.IsScalar<double>());
assert(!n.IsInt64());
~~~

### precision {#precision}

#### Description

`[number]` The maximum size of the `scalar` or `ndarray` elements in bytes. For string subtypes, this is the length of the string.

#### Role in validation

If a JSON document has a larger precision that the one specified by the schema, the following error will be produced:

`Property has a precision of %actual that is incompatible with the schema precision %expected.`

* `expected`: Maximum precision (in bytes) allowed by the `precision` schema keyword.
* `actual`: The precision (in bytes) of the instance.

~~~cpp
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
~~~

#### Role in normalization

For numbers, instances can be cast to precision/subtype if they are within the range allowable for the equivalent type in C++. For strings (both primative and scalars/arrays), an instance can only be normalized to a larger precision (it is padded with spaces).

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
assert(n.IsScalar<double>());
assert(n.GetPrecision() == 8);
~~~

### units {#units}

#### Description

`[string]` The physical units that `scalar` or `ndarray` elements should have. A description of how units can be specified can be found in the [units documentation](@ref Units).

#### Role in validation

If a JSON document has units that are incompatible (not having the same dimensionality) with those specified by the validation schema, the following error will be set:

`Property has units '%actual' that are not compatible with the schema '%expected'.`

* `expected`: Units allowed by the `units` schema keyword.
* `actual`: The units of the instance.

~~~cpp
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
~~~

#### Role in normalization

If the units of the JSON instance are compatible with the schema units, the scalar/ndarray will be converted to the schema units in the normalized document.

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
assert(n.IsScalar<double>());
assert(n.HasUnits());
assert(n.GetUnits() == "g");
~~~

### shape {#shape}

#### Description

`[array]` The size in each dimension that `ndarray` instance should have.

#### Role in validation

If the instance does not have the same shape as specified in the schema, the following error will be set:

`Property has a shape %actual that does not match the schema %expected.`

* `expected`: Shape allowed by the `shape` schema keyword.
* `actual`: The shape of the instance.

~~~cpp
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
~~~

#### Role in normalization

This keyword does not apply to normalization.

### ndim {#ndim}

#### Description

`[integer]` The number of dimensions that `ndarray` instances should have.

#### Role in validation

If the instance does not have the name number of dimensions as specified in the schema, the shape error will be set (see above), but the `expected` shape will be set to an array of `null` with `ndim` items.

~~~cpp
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
~~~

#### Role in normalization

This keyword does not apply to normalization.

### length {#length}

#### Description

`[integer]` The number of elements that 1-dimensional `ndarray` instance should have.

#### Role in validation

If the instance does not have a shape of `[length]`, the shape error will be set (see above), but the `expected` shape will be set to `[length]`.

~~~cpp
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
~~~

#### Role in normalization

This keyword does not apply to normalization.

### encoding {#encoding}

#### Description

`[string]` Encoding that string `scalar` or `ndarray` instance elements should have. Available values include:

* `ASCII`
* `UCS4`
* `UTF8`
* `UTF16`
* `UTF32`

#### Role in validation

If the instance encoding does not match the encoding specified by the schema, the following error will be set:

`Property has an encoding '%actual' that does not match the schema '%expected'.`

* `expected`: Encoding allowed by the `encoding` schema keyword.
* `actual`: The encoding of the instance.

~~~cpp
TODO
~~~

#### Role in normalization

If the instance has a different encoding that the schema, it will be transcoded into the schema encoding.

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
assert(n.HasEncoding());
assert(n.GetEncoding() == "UCS4");
~~~

## Keywords for Python classes, functions, & instances

### class {#class}

#### Description

`[class]` Python class that Python instances should be a subclass of.

#### Role in validation

If an instance is not a Python instance of the specified class, the following error will be set:

`Property is not a Python instance of the class '%expected' specified in the schema (actual = '%actual').`

* `expected`: Python class allowed by the `class` schema keyword.
* `actual`: The class of the instance.

~~~cpp
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
~~~

#### Role in normalization

If a JSON object contains the required information to reconstruct a Python instance, the normalized document will be a Python instance.

~~~cpp
SchemaNormalizer normalizer(s);
d.Accept(normalizer);
assert(normalizer.IsValid());
const SchemaNormalizer::ValueType& n = normalizer.GetNormalized();
assert(n.IsPythonInstance());
~~~

