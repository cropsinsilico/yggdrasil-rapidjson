# Schema

JSON Schema is a draft standard for describing the format of JSON data. The schema itself is also JSON data. RapidJSON added support for [validating JSON](https://rapidjson.org/md_doc_schema.html) structures against JSON schema in v1.1.0. YggdrasilRapidJSON extended this by adding support for additional properties that can be specified (mostly pertaining to the added data types), normalization of JSON structures using JSON schema, and some additional utilities for interactiving with the schema classes.

Because YggdrasilRapidJSON JSON schema features are built upon RapidJSON's JSON schema classes & functions, refer to the [RapidJSON JSON schema](https://rapidjson.org/md_doc_schema.html) documentation for additional information on the schema implementation and examples.

Like RapidJSON's JSON schema features, YggdrasilRapidJSON's schema features implement [JSON Schema Draft v4](http://json-schema.org/documentation.html). If you are not familiar with JSON Schema, you may refer to [Understanding JSON Schema](http://spacetelescope.github.io/understanding-json-schema/).


[TOC]

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

In addition to the JSON schema standard implemented by RapidJSON, YggdrasilRapidJSON supports several additional schema properties for validating YggdrasilRapidJSON documents.

## Validation keywords for any instance type {#AnyTypes}

### enum {#enum}

This keyword has no additional properties
beyond `instanceRef` and `schemaRef`.

* The allowed values are not listed
  because `SchemaDocument` does not store them in original form.
* The violating value is not reported
  because it might be unwieldy.

If you need to report these details to your users,
you can access the necessary information
by following `instanceRef` and `schemaRef`.

### type {#type}

* `expected`: required array of one or more unique strings,
  each of which is one of the seven primitive types
  defined by the JSON Schema Draft 04 Core specification.
  Lists the types allowed by the `type` schema keyword.
* `actual`: required string, also one of seven primitive types.
  The primitive type of the instance.

### allOf, anyOf, and oneOf {#allOf-anyOf-oneOf}

* `errors`: required array of at least one object.
  There will be as many items as there are subschemas
  in the `allOf`, `anyOf` or `oneOf` schema keyword, respectively.
  Each item will be the error value
  produced by validating the instance
  against the corresponding subschema.

For `allOf`, at least one error value will be non-empty.
For `anyOf`, all error values will be non-empty.
For `oneOf`, either all error values will be non-empty,
or more than one will be empty.

### not {#not}

This keyword has no additional properties
apart from `instanceRef` and `schemaRef`.


## Keywords for numeric scalars (and ndarrays) {#Scalars}

### subtype {#subtype}

* `expected`: required array of one or more unique strings,
  each of which is one of the five yggdrasil subtypes for scalars and
  ndarrays (TODO: REFERENCE TO DOCS ON NEW TYPES)
  Lists the types allowed by the `subtype` schema keyword.
* `actual`: required string, also one of five yggdrasil subtypes.
  The scalar/ndarray subtype of the instance.

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


### precision {#precision}

* `expected`: required integer strictly greater than 0.
  The value of the `precision` keyword specified in the schema
  defining the minimum allowed precision (in bytes) for scalars or
  ndarray elements.
* `actual`: required integer.
  The instance precision.

For numbers, instances can be cast to precision/subtype if they are within the range allowable for the equivalent type in C++. For strings (both primative and scalars/arrays), an instance can only be normalized to a larger precision (it is padded with spaces).


### multipleOf {#multipleof}

* `expected`: required number strictly greater than 0.
  The value of the `multipleOf` keyword specified in the schema.
* `actual`: required number.
  The instance value.

### maximum {#maximum}

* `expected`: required number.
  The value of the `maximum` keyword specified in the schema.
* `exclusiveMaximum`: optional boolean.
  This will be true if the schema specified `"exclusiveMaximum": true`,
  and will be omitted otherwise.
* `actual`: required number.
  The instance value.

### minimum {#minimum}

* `expected`: required number.
  The value of the `minimum` keyword specified in the schema.
* `exclusiveMinimum`: optional boolean.
  This will be true if the schema specified `"exclusiveMinimum": true`,
  and will be omitted otherwise.
* `actual`: required number.
  The instance value.

## Validation keywords for strings {#Strings}

### maxLength {#maxLength}

* `expected`: required number greater than or equal to 0.
  The value of the `maxLength` keyword specified in the schema.
* `actual`: required string.
  The instance value.

### minLength {#minLength}

* `expected`: required number greater than or equal to 0.
  The value of the `minLength` keyword specified in the schema.
* `actual`: required string.
  The instance value.

### pattern {#pattern}

* `actual`: required string.
  The instance value.

(The expected pattern is not reported
because the internal representation in `SchemaDocument`
does not store the pattern in original string form.)

## Validation keywords for arrays {#Arrays}

### additionalItems {#additionalItems}

This keyword is reported
when the value of `items` schema keyword is an array,
the value of `additionalItems` is `false`,
and the instance is an array
with more items than specified in the `items` array.

* `disallowed`: required integer greater than or equal to 0.
  The index of the first item that has no corresponding schema.

### maxItems and minItems {#maxItems-minItems}

* `expected`: required integer greater than or equal to 0.
  The value of `maxItems` (respectively, `minItems`)
  specified in the schema.
* `actual`: required integer greater than or equal to 0.
  Number of items in the instance array.

### uniqueItems {#uniqueItems}

* `duplicates`: required array
  whose items are integers greater than or equal to 0.
  Indices of items of the instance that are equal.

(RapidJSON only reports the first two equal items,
for performance reasons.)

## Validation keywords for objects

### maxProperties and minProperties {#maxProperties-minProperties}

* `expected`: required integer greater than or equal to 0.
  The value of `maxProperties` (respectively, `minProperties`)
  specified in the schema.
* `actual`: required integer greater than or equal to 0.
  Number of properties in the instance object.

### required {#required}

* `missing`: required array of one or more unique strings.
  The names of properties
  that are listed in the value of the `required` schema keyword
  but not present in the instance object.

### additionalProperties {#additionalProperties}

This keyword is reported
when the schema specifies `additionalProperties: false`
and the name of a property of the instance is
neither listed in the `properties` keyword
nor matches any regular expression in the `patternProperties` keyword.

* `disallowed`: required string.
  Name of the offending property of the instance.

(For performance reasons,
RapidJSON only reports the first such property encountered.)

### dependencies {#dependencies}

* `errors`: required object with one or more properties.
  Names and values of its properties are described below.

Recall that JSON Schema Draft 04 supports
*schema dependencies*,
where presence of a named *controlling* property
requires the instance object to be valid against a subschema,
and *property dependencies*,
where presence of a controlling property
requires other *dependent* properties to be also present.

For a violated schema dependency,
`errors` will contain a property
with the name of the controlling property
and its value will be the error object
produced by validating the instance object
against the dependent schema.

For a violated property dependency,
`errors` will contain a property
with the name of the controlling property
and its value will be an array of one or more unique strings
listing the missing dependent properties.

