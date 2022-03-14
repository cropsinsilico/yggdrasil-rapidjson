// Tencent is pleased to support the open source community by making RapidJSON available->
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip-> All rights reserved->
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License-> You may obtain a copy of the License at
//
// http://opensource->org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied-> See the License for the 
// specific language governing permissions and limitations under the License->

#ifndef RAPIDJSON_SCHEMA_H_
#define RAPIDJSON_SCHEMA_H_

#include "document.h"
#include "pointer.h"
#include "stringbuffer.h"
#include "error/en.h"
#include "uri.h"
#include <cmath> // abs, floor
#ifdef RAPIDJSON_YGGDRASIL
#include "units.h"
#include "metaschema.h"
#endif // RAPIDJSON_YGGDRASIL

#if !defined(RAPIDJSON_SCHEMA_USE_INTERNALREGEX)
#define RAPIDJSON_SCHEMA_USE_INTERNALREGEX 1
#else
#define RAPIDJSON_SCHEMA_USE_INTERNALREGEX 0
#endif

#if !RAPIDJSON_SCHEMA_USE_INTERNALREGEX && defined(RAPIDJSON_SCHEMA_USE_STDREGEX) && (__cplusplus >=201103L || (defined(_MSC_VER) && _MSC_VER >= 1800))
#define RAPIDJSON_SCHEMA_USE_STDREGEX 1
#else
#define RAPIDJSON_SCHEMA_USE_STDREGEX 0
#endif

#if RAPIDJSON_SCHEMA_USE_INTERNALREGEX
#include "internal/regex.h"
#elif RAPIDJSON_SCHEMA_USE_STDREGEX
#include <regex>
#endif

#if RAPIDJSON_SCHEMA_USE_INTERNALREGEX || RAPIDJSON_SCHEMA_USE_STDREGEX
#define RAPIDJSON_SCHEMA_HAS_REGEX 1
#else
#define RAPIDJSON_SCHEMA_HAS_REGEX 0
#endif

#ifndef RAPIDJSON_SCHEMA_VERBOSE
#define RAPIDJSON_SCHEMA_VERBOSE 0
#endif

#if RAPIDJSON_SCHEMA_VERBOSE
#include "stringbuffer.h"
#endif

RAPIDJSON_DIAG_PUSH

#if defined(__GNUC__)
RAPIDJSON_DIAG_OFF(effc++)
#endif

#ifdef __clang__
RAPIDJSON_DIAG_OFF(weak-vtables)
RAPIDJSON_DIAG_OFF(exit-time-destructors)
RAPIDJSON_DIAG_OFF(c++98-compat-pedantic)
RAPIDJSON_DIAG_OFF(variadic-macros)
#elif defined(_MSC_VER)
RAPIDJSON_DIAG_OFF(4512) // assignment operator could not be generated
#endif

RAPIDJSON_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
// Verbose Utilities

#if RAPIDJSON_SCHEMA_VERBOSE

namespace internal {

inline void PrintInvalidKeyword(const char* keyword) {
    printf("Fail keyword: %s\n", keyword);
}

inline void PrintInvalidKeyword(const wchar_t* keyword) {
    wprintf(L"Fail keyword: %ls\n", keyword);
}

inline void PrintInvalidDocument(const char* document) {
    printf("Fail document: %s\n\n", document);
}

inline void PrintInvalidDocument(const wchar_t* document) {
    wprintf(L"Fail document: %ls\n\n", document);
}

inline void PrintValidatorPointers(unsigned depth, const char* s, const char* d) {
    printf("S: %*s%s\nD: %*s%s\n\n", depth * 4, " ", s, depth * 4, " ", d);
}

inline void PrintValidatorPointers(unsigned depth, const wchar_t* s, const wchar_t* d) {
    wprintf(L"S: %*ls%ls\nD: %*ls%ls\n\n", depth * 4, L" ", s, depth * 4, L" ", d);
}

} // namespace internal

#endif // RAPIDJSON_SCHEMA_VERBOSE

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_INVALID_KEYWORD_RETURN

#if RAPIDJSON_SCHEMA_VERBOSE
#define RAPIDJSON_INVALID_KEYWORD_VERBOSE(keyword) internal::PrintInvalidKeyword(keyword)
#else
#define RAPIDJSON_INVALID_KEYWORD_VERBOSE(keyword)
#endif

#define RAPIDJSON_INVALID_KEYWORD_RETURN(code)\
RAPIDJSON_MULTILINEMACRO_BEGIN\
    context.invalidCode = code;\
    context.invalidKeyword = SchemaType::GetValidateErrorKeyword(code).GetString();\
    RAPIDJSON_INVALID_KEYWORD_VERBOSE(context.invalidKeyword);\
    return false;\
RAPIDJSON_MULTILINEMACRO_END

///////////////////////////////////////////////////////////////////////////////
// ValidateFlag

/*! \def RAPIDJSON_VALIDATE_DEFAULT_FLAGS
    \ingroup RAPIDJSON_CONFIG
    \brief User-defined kValidateDefaultFlags definition.

    User can define this as any \c ValidateFlag combinations.
*/
#ifndef RAPIDJSON_VALIDATE_DEFAULT_FLAGS
#define RAPIDJSON_VALIDATE_DEFAULT_FLAGS kValidateNoFlags
#endif

//! Combination of validate flags
/*! \see
 */
enum ValidateFlag {
    kValidateNoFlags = 0,                                       //!< No flags are set.
    kValidateContinueOnErrorFlag = 1,                           //!< Don't stop after first validation error.
    kValidateDefaultFlags = RAPIDJSON_VALIDATE_DEFAULT_FLAGS    //!< Default validate flags. Can be customized by defining RAPIDJSON_VALIDATE_DEFAULT_FLAGS
};

///////////////////////////////////////////////////////////////////////////////
// Forward declarations

template <typename ValueType, typename Allocator>
class GenericSchemaDocument;

namespace internal {

template <typename SchemaDocumentType>
class Schema;

#ifdef RAPIDJSON_YGGDRASIL
template <typename SchemaDocumentType, typename StackAllocator>
class GenericNormalizedDocument;
#endif // RAPIDJSON_YGGDRASIL

///////////////////////////////////////////////////////////////////////////////
// ISchemaValidator

class ISchemaValidator {
public:
    virtual ~ISchemaValidator() {}
    virtual bool IsValid() const = 0;
    virtual void SetValidateFlags(unsigned flags) = 0;
    virtual unsigned GetValidateFlags() const = 0;
#ifdef RAPIDJSON_YGGDRASIL
    virtual unsigned GetValidatorID() const { return 0; }
#endif // RAPIDJSON_YGGDRASIL
};

///////////////////////////////////////////////////////////////////////////////
// ISchemaStateFactory

template <typename SchemaType>
class ISchemaStateFactory {
public:
    virtual ~ISchemaStateFactory() {}
    virtual ISchemaValidator* CreateSchemaValidator(const SchemaType&, const bool inheritContinueOnErrors) = 0;
    virtual void DestroySchemaValidator(ISchemaValidator* validator) = 0;
    virtual void* CreateHasher() = 0;
    virtual uint64_t GetHashCode(void* hasher) = 0;
    virtual void DestroryHasher(void* hasher) = 0;
    virtual void* MallocState(size_t size) = 0;
    virtual void FreeState(void* p) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// IValidationErrorHandler

template <typename SchemaType>
class IValidationErrorHandler {
public:
    typedef typename SchemaType::Ch Ch;
    typedef typename SchemaType::SValue SValue;

    virtual ~IValidationErrorHandler() {}

    virtual void NotMultipleOf(int64_t actual, const SValue& expected) = 0;
    virtual void NotMultipleOf(uint64_t actual, const SValue& expected) = 0;
    virtual void NotMultipleOf(double actual, const SValue& expected) = 0;
    virtual void AboveMaximum(int64_t actual, const SValue& expected, bool exclusive) = 0;
    virtual void AboveMaximum(uint64_t actual, const SValue& expected, bool exclusive) = 0;
    virtual void AboveMaximum(double actual, const SValue& expected, bool exclusive) = 0;
    virtual void BelowMinimum(int64_t actual, const SValue& expected, bool exclusive) = 0;
    virtual void BelowMinimum(uint64_t actual, const SValue& expected, bool exclusive) = 0;
    virtual void BelowMinimum(double actual, const SValue& expected, bool exclusive) = 0;

    virtual void TooLong(const Ch* str, SizeType length, SizeType expected) = 0;
    virtual void TooShort(const Ch* str, SizeType length, SizeType expected) = 0;
    virtual void DoesNotMatch(const Ch* str, SizeType length) = 0;

    virtual void DisallowedItem(SizeType index) = 0;
    virtual void TooFewItems(SizeType actualCount, SizeType expectedCount) = 0;
    virtual void TooManyItems(SizeType actualCount, SizeType expectedCount) = 0;
    virtual void DuplicateItems(SizeType index1, SizeType index2) = 0;

    virtual void TooManyProperties(SizeType actualCount, SizeType expectedCount) = 0;
    virtual void TooFewProperties(SizeType actualCount, SizeType expectedCount) = 0;
    virtual void StartMissingProperties() = 0;
    virtual void AddMissingProperty(const SValue& name) = 0;
    virtual bool EndMissingProperties() = 0;
    virtual void PropertyViolations(ISchemaValidator** subvalidators, SizeType count) = 0;
    virtual void DisallowedProperty(const Ch* name, SizeType length) = 0;

    virtual void StartDependencyErrors() = 0;
    virtual void StartMissingDependentProperties() = 0;
    virtual void AddMissingDependentProperty(const SValue& targetName) = 0;
    virtual void EndMissingDependentProperties(const SValue& sourceName) = 0;
    virtual void AddDependencySchemaError(const SValue& souceName, ISchemaValidator* subvalidator) = 0;
    virtual bool EndDependencyErrors() = 0;

    virtual void DisallowedValue(const ValidateErrorCode code) = 0;
    virtual void StartDisallowedType() = 0;
    virtual void AddExpectedType(const typename SchemaType::ValueType& expectedType) = 0;
    virtual void EndDisallowedType(const typename SchemaType::ValueType& actualType) = 0;
    virtual void NotAllOf(ISchemaValidator** subvalidators, SizeType count) = 0;
    virtual void NoneOf(ISchemaValidator** subvalidators, SizeType count) = 0;
    virtual void NotOneOf(ISchemaValidator** subvalidators, SizeType count, bool matched) = 0;
    virtual void Disallowed() = 0;
  
#ifdef RAPIDJSON_YGGDRASIL
  virtual void MissingRequiredSchemaProperty(const typename SchemaType::ValueType& name) = 0;
  virtual void IncorrectSubType(const typename SchemaType::ValueType& actual, const typename SchemaType::ValueType& expected) = 0;
  virtual void IncorrectPrecision(const typename SchemaType::ValueType& actual, const SValue& expected) = 0;
  virtual void IncorrectUnits(const typename SchemaType::ValueType& actual, const SValue& expected) = 0;
  virtual void IncorrectShape(const SValue& actual, const SValue& expected) = 0;
  virtual void InvalidPythonImport(const Ch* str, SizeType len) = 0;
  virtual void InvalidSchema(ValidateErrorCode code, ISchemaValidator* subvalidator) = 0;
  // Normalization errors
  virtual void DuplicateAlias(const SValue& base, const SValue& alias) = 0;
  virtual void CircularAlias(const SValue& alias) = 0;
  virtual void ConflictingAliases(const SValue& alias, const SValue& base1, const SValue& base2) = 0;
#endif // RAPIDJSON_YGGDRASIL
  
};


///////////////////////////////////////////////////////////////////////////////
// Hasher

// For comparison of compound value
template<typename Encoding, typename Allocator>
class Hasher {
public:
    typedef typename Encoding::Ch Ch;
    typedef GenericValue<Encoding, Allocator> ValueType;

    Hasher(Allocator* allocator = 0, size_t stackCapacity = kDefaultSize) : stack_(allocator, stackCapacity) {}

    bool Null() { return WriteType(kNullType); }
    bool Bool(bool b) { return WriteType(b ? kTrueType : kFalseType); }
    bool Int(int i) { Number n; n.u.i = i; n.d = static_cast<double>(i); return WriteNumber(n); }
    bool Uint(unsigned u) { Number n; n.u.u = u; n.d = static_cast<double>(u); return WriteNumber(n); }
    bool Int64(int64_t i) { Number n; n.u.i = i; n.d = static_cast<double>(i); return WriteNumber(n); }
    bool Uint64(uint64_t u) { Number n; n.u.u = u; n.d = static_cast<double>(u); return WriteNumber(n); }
    bool Double(double d) { 
        Number n; 
        if (d < 0) n.u.i = static_cast<int64_t>(d);
        else       n.u.u = static_cast<uint64_t>(d); 
        n.d = d;
        return WriteNumber(n);
    }

    bool RawNumber(const Ch* str, SizeType len, bool) {
        WriteBuffer(kNumberType, str, len * sizeof(Ch));
        return true;
    }

    bool String(const Ch* str, SizeType len, bool) {
        WriteBuffer(kStringType, str, len * sizeof(Ch));
        return true;
    }

#ifdef RAPIDJSON_YGGDRASIL
  template <typename YggSchemaValueType>
  bool YggdrasilString(const Ch* str, SizeType len, bool copy, YggSchemaValueType&) {
    return String(str, len, copy); }
  template <typename YggSchemaValueType>
  bool YggdrasilStartObject(YggSchemaValueType&) { return StartObject(); }
  bool YggdrasilEndObject(SizeType memberCount) { return EndObject(memberCount); }
#endif // RAPIDJSON_YGGDRASIL

    bool StartObject() { return true; }
    bool Key(const Ch* str, SizeType len, bool copy) { return String(str, len, copy); }
    bool EndObject(SizeType memberCount) { 
        uint64_t h = Hash(0, kObjectType);
        uint64_t* kv = stack_.template Pop<uint64_t>(memberCount * 2);
        for (SizeType i = 0; i < memberCount; i++)
            h ^= Hash(kv[i * 2], kv[i * 2 + 1]);  // Use xor to achieve member order insensitive
        *stack_.template Push<uint64_t>() = h;
        return true;
    }
    
    bool StartArray() { return true; }
    bool EndArray(SizeType elementCount) { 
        uint64_t h = Hash(0, kArrayType);
        uint64_t* e = stack_.template Pop<uint64_t>(elementCount);
        for (SizeType i = 0; i < elementCount; i++)
            h = Hash(h, e[i]); // Use hash to achieve element order sensitive
        *stack_.template Push<uint64_t>() = h;
        return true;
    }

    bool IsValid() const { return stack_.GetSize() == sizeof(uint64_t); }

    uint64_t GetHashCode() const {
        RAPIDJSON_ASSERT(IsValid());
        return *stack_.template Top<uint64_t>();
    }

private:
    static const size_t kDefaultSize = 256;
    struct Number {
        union U {
            uint64_t u;
            int64_t i;
        }u;
        double d;
    };

    bool WriteType(Type type) { return WriteBuffer(type, 0, 0); }
    
    bool WriteNumber(const Number& n) { return WriteBuffer(kNumberType, &n, sizeof(n)); }
    
    bool WriteBuffer(Type type, const void* data, size_t len) {
        // FNV-1a from http://isthe.com/chongo/tech/comp/fnv/
        uint64_t h = Hash(RAPIDJSON_UINT64_C2(0x84222325, 0xcbf29ce4), type);
        const unsigned char* d = static_cast<const unsigned char*>(data);
        for (size_t i = 0; i < len; i++)
            h = Hash(h, d[i]);
        *stack_.template Push<uint64_t>() = h;
        return true;
    }

    static uint64_t Hash(uint64_t h, uint64_t d) {
        static const uint64_t kPrime = RAPIDJSON_UINT64_C2(0x00000100, 0x000001b3);
        h ^= d;
        h *= kPrime;
        return h;
    }

    Stack<Allocator> stack_;
};

///////////////////////////////////////////////////////////////////////////////
// SchemaValidationContext

template <typename SchemaDocumentType>
struct SchemaValidationContext {
    typedef Schema<SchemaDocumentType> SchemaType;
    typedef ISchemaStateFactory<SchemaType> SchemaValidatorFactoryType;
    typedef IValidationErrorHandler<SchemaType> ErrorHandlerType;
    typedef typename SchemaType::ValueType ValueType;
    typedef typename ValueType::Ch Ch;
#ifdef RAPIDJSON_YGGDRASIL
    typedef typename SchemaType::EncodingType EncodingType;
    typedef typename SchemaType::AllocatorType AllocatorType;
    typedef GenericNormalizedDocument<SchemaDocumentType, RAPIDJSON_DEFAULT_STACK_ALLOCATOR> NormalizedDocumentType;
#endif //RAPIDJSON_YGGDRASIL

    enum PatternValidatorType {
        kPatternValidatorOnly,
        kPatternValidatorWithProperty,
        kPatternValidatorWithAdditionalProperty
    };

    SchemaValidationContext(SchemaValidatorFactoryType& f, ErrorHandlerType& eh, const SchemaType* s) :
        factory(f),
        error_handler(eh),
        schema(s),
        valueSchema(),
        invalidKeyword(),
        invalidCode(),
        hasher(),
        arrayElementHashCodes(),
        validators(),
        validatorCount(),
        patternPropertiesValidators(),
        patternPropertiesValidatorCount(),
        patternPropertiesSchemas(),
        patternPropertiesSchemaCount(),
        valuePatternValidatorType(kPatternValidatorOnly),
        propertyExist(),
        inArray(false),
        valueUniqueness(false),
        arrayUniqueness(false)
#ifdef RAPIDJSON_YGGDRASIL
	, normalized()
#endif //RAPIDJSON_YGGDRASIL
    {
    }

    ~SchemaValidationContext() {
        if (hasher)
            factory.DestroryHasher(hasher);
        if (validators) {
            for (SizeType i = 0; i < validatorCount; i++)
                factory.DestroySchemaValidator(validators[i]);
            factory.FreeState(validators);
        }
        if (patternPropertiesValidators) {
            for (SizeType i = 0; i < patternPropertiesValidatorCount; i++)
                factory.DestroySchemaValidator(patternPropertiesValidators[i]);
            factory.FreeState(patternPropertiesValidators);
        }
        if (patternPropertiesSchemas)
            factory.FreeState(patternPropertiesSchemas);
        if (propertyExist)
            factory.FreeState(propertyExist);
    }

    SchemaValidatorFactoryType& factory;
    ErrorHandlerType& error_handler;
    const SchemaType* schema;
    const SchemaType* valueSchema;
    const Ch* invalidKeyword;
    ValidateErrorCode invalidCode;
    void* hasher; // Only validator access
    void* arrayElementHashCodes; // Only validator access this
    ISchemaValidator** validators;
    SizeType validatorCount;
    ISchemaValidator** patternPropertiesValidators;
    SizeType patternPropertiesValidatorCount;
    const SchemaType** patternPropertiesSchemas;
    SizeType patternPropertiesSchemaCount;
    PatternValidatorType valuePatternValidatorType;
    PatternValidatorType objectPatternValidatorType;
    SizeType arrayElementIndex;
    bool* propertyExist;
    bool inArray;
    bool valueUniqueness;
    bool arrayUniqueness;
#ifdef RAPIDJSON_YGGDRASIL
    NormalizedDocumentType* normalized;
#endif //RAPIDJSON_YGGDRASIL
};

#ifdef RAPIDJSON_YGGDRASIL

// template<typename ValueType>
// void printAliases_(const ValueType& aliases) {
//   RAPIDJSON_ASSERT(aliases.IsObject());
//   for (typename ValueType::ConstMemberIterator it = aliases.MemberBegin(); it != aliases.MemberEnd(); ++it) {
//     std::cerr << it->name.GetString() << ":";
//     if (it->value.IsObject()) {
//       std::cerr << std::endl;
//       for (typename ValueType::ConstMemberIterator v = it->value.MemberBegin(); v != it->value.MemberEnd(); ++v) {
// 	std::cerr << "    " << v->name.GetString() << "->" << v->value.GetString() << std::endl;
//       }
//     } else if (it->value.IsString()) {
//       std::cerr << it->value.GetString() << std::endl;
//     }
//   }
// };
template<typename ValueType, typename AllocatorType>
bool follow_aliases_(const ValueType& aliases, const ValueType& orig,
		     ValueType* dest, AllocatorType& allocator) {
  typename ValueType::ConstMemberIterator primary = aliases.FindMember(orig);
  if (primary == aliases.MemberEnd()) {
    dest->CopyFrom(orig, allocator);
    return true;
  }
  ValueType path(kArrayType);
  RAPIDJSON_ASSERT(orig.IsString());
  path.PushBack(ValueType(orig, allocator), allocator);
  RAPIDJSON_ASSERT(primary->value.IsString());
  while (aliases.HasMember(primary->value)) {
    for (typename ValueType::ConstValueIterator it = path.Begin(); it != path.End(); ++it) {
      if (primary->value == *it) {
	dest->CopyFrom(path, allocator);
	return false;
      }
    }
    path.PushBack(ValueType(primary->value, allocator), allocator);
    primary = aliases.FindMember(primary->value);
    RAPIDJSON_ASSERT(primary->value.IsString());
  }
  dest->CopyFrom(primary->value, allocator);
  return true;
};

///////////////////////////////////////////////////////////////////////////////
// GenericNormalizedDocument
  
template <typename SchemaDocumentType, typename StackAllocator = RAPIDJSON_DEFAULT_STACK_ALLOCATOR>
class GenericNormalizedDocument {
  typedef Schema<SchemaDocumentType> SchemaType;
  typedef typename SchemaType::EncodingType EncodingType;
  typedef typename SchemaType::AllocatorType AllocatorType;
  typedef typename SchemaType::SValue SValue;
  typedef typename SchemaType::PointerType PointerType;
  typedef GenericValue<EncodingType, AllocatorType> ValueType;
  typedef GenericDocument<EncodingType, AllocatorType> DocumentType;
  typedef SchemaValidationContext<SchemaDocumentType> Context;
  typedef typename EncodingType::Ch Ch;
  typedef typename ValueType::MemberIterator MemberIterator;
  typedef typename ValueType::ConstMemberIterator ConstMemberIterator;
public:
  GenericNormalizedDocument(size_t stackCapacity = kDefaultStackCapacity,
			    StackAllocator* stackAllocator = 0) :
    document_(0, stackCapacity, stackAllocator), index_(0),
    modified_(false), extending_(false), appending_(false),
    extend_context_(nullptr), extend_schema_(nullptr),
    keyStack_(stackAllocator, stackCapacity),
    valueStack_(stackAllocator, stackCapacity),
    childStack_(stackAllocator, stackCapacity),
    documentStack_(nullptr),
    aliases_(kObjectType) {}
  GenericNormalizedDocument(GenericNormalizedDocument* parent, unsigned& index,
			    size_t stackCapacity = kDefaultStackCapacity,
			    StackAllocator* stackAllocator = 0) :
    document_(0, stackCapacity, stackAllocator), index_(index),
    modified_(false), extending_(false), appending_(false),
    extend_context_(nullptr), extend_schema_(nullptr),
    keyStack_(stackAllocator, stackCapacity),
    valueStack_(stackAllocator, stackCapacity),
    childStack_(stackAllocator, stackCapacity),
    documentStack_(nullptr),
    aliases_(kObjectType) {
    parent->AddChild(this);
  }

  void AddChild(GenericNormalizedDocument* child) {
    GenericNormalizedDocument** ref = childStack_.template Push<GenericNormalizedDocument*>();
    ref[0] = child;
  }
  GenericNormalizedDocument* FindChild(unsigned index) {
    GenericNormalizedDocument** ref = childStack_.template Bottom<GenericNormalizedDocument*>();
    while (true) {
      if ((*ref)->index_ == index) return *ref;
      if (ref == childStack_.template Top<GenericNormalizedDocument*>())
	break;
      ref++;
    }
    return nullptr;
  }

  //! Get the current normalized document.
  const DocumentType& GetDocument() const { return document_; }
  //! Determine if the document was modified.
  bool WasModified() const { return modified_; }
  //! Finalize the document from elements added to the stack.
  void FinalizeFromStack() { document_.FinalizeFromStack(); }
  //! Determine if the document was finalized.
  bool WasFinalized() const { return document_.WasFinalized(); }

  bool ExtendChild(Context& context, const SchemaType& schema, unsigned index) {
    GenericNormalizedDocument* child = FindChild(index);
    RAPIDJSON_ASSERT(child);
    child->FinalizeFromStack();
    bool replaced = false;
    if (!child->ExtendAliases(context, aliases_, &replaced)) return false;
    if (!ExtendAliases(context, child->aliases_, &replaced)) return false;
    if (!(replaced || child->modified_))
      return true;
    modified_ = true;
    return Extend(context, schema, child->document_);
  }

  bool Append(Context& context, const SchemaType& schema, const ValueType& document) {
    appending_ = true;
    bool out = Extend(context, schema, document);
    appending_ = false;
    return out;
  }

  bool Extend(Context& context, const SchemaType& schema, const ValueType& document) {
    RAPIDJSON_ASSERT(!extending_);
    RAPIDJSON_ASSERT(!extend_context_);
    RAPIDJSON_ASSERT(!extend_schema_);
    RAPIDJSON_ASSERT(!document_.WasFinalized());
    PushValue(*document_.StackTop());
    extending_ = true;
    extend_context_ = &context;
    extend_schema_ = &schema;
    bool out = document.Accept(*this);
    extending_ = false;
    extend_context_ = nullptr;
    extend_schema_ = nullptr;
    PopValue();
    return out;
  }

  bool BeginValue(Context& context) {
    if (context.inArray)
      PushValue(CurrentValue()[context.arrayElementIndex]);
    if (CurrentKey())
      PushValue((*CurrentValue())[*CurrentKey()]);
    PushKey();
    return true;
  }
  bool EndValue(Context& context) {
    PopKey();
    if (context.inArray)
      PopValue();
    if (CurrentKey()) {
      PopValue();
      PopKey();
    }
    return true;
  }
  
#define NORMALIZE_(method, arg)			\
  if ((!extending_) || appending_)		\
    return document_.method arg;

#define BEGIN_NORMALIZE_(method, arg1, arg2)				\
  NORMALIZE_(method, arg1);						\
  if (CurrentKey() && !CurrentValue()->HasMember(CurrentKey()->GetString())) { \
    ValueType tmp(CurrentKey()->GetString(),				\
		  CurrentKey()->GetStringLength(),			\
		  document_.GetAllocator());				\
    CurrentValue()->AddMember(tmp, ValueType arg2,			\
			      document_.GetAllocator());		\
  }									\
  if (!BeginValue(context)) return false;				\
  
#define NORMALIZE_VALUE_(method, value)				\
  BEGIN_NORMALIZE_(method, (value), (value));			\
  if (!CurrentValue()->Is ## method()) return false;		\
  if (value != CurrentValue()->Get ## method()) return false;	\
  return EndValue(context)

  bool Null(Context& context, const SchemaType&) {
    BEGIN_NORMALIZE_(Null, (), ());
    if (!CurrentValue()->IsNull()) return false;
    return EndValue(context);
  }
  bool Bool(Context& context, const SchemaType&, bool b)       { NORMALIZE_VALUE_(Bool,   b); }
  bool Int(Context& context, const SchemaType&, int i)         { NORMALIZE_VALUE_(Int,    i); }
  bool Uint(Context& context, const SchemaType&, unsigned u)   { NORMALIZE_VALUE_(Uint,   u); }
  bool Int64(Context& context, const SchemaType&, int64_t i)   { NORMALIZE_VALUE_(Int64,  i); }
  bool Uint64(Context& context, const SchemaType&, uint64_t u) { NORMALIZE_VALUE_(Uint64, u); }
  bool Double(Context& context, const SchemaType&, double d)   {
    BEGIN_NORMALIZE_(Double, (d), (d));
    if (!CurrentValue()->IsDouble()) return false;
    double b = CurrentValue()->GetDouble();
    if (!(d >= b && d <= b)) return false;
    return EndValue(context);
  }
  bool String(Context& context, const SchemaType&, const Ch* str, SizeType length, bool copy) {
    BEGIN_NORMALIZE_(String, (str, length, copy), (str, length, document_.GetAllocator()));
    if (!CurrentValue()->IsString()) return false;
    if (internal::StrCmp(str, CurrentValue()->GetString()) != 0) return false;
    return EndValue(context);
  }
  template <typename YggSchemaValueType>
  bool YggdrasilString(Context& context, const SchemaType&, const Ch* str, SizeType length, bool copy, YggSchemaValueType& valueSchema) {
    BEGIN_NORMALIZE_(YggdrasilString, (str, length, copy, valueSchema),
		     (str, length, valueSchema));
    if (!CurrentValue()->IsYggdrasil()) return false;
    if (CurrentValue()->GetValueSchema() != valueSchema) return false;
    if (!CurrentValue()->IsString()) return false;
    if (internal::StrCmp(str, CurrentValue()->GetString()) != 0) return false;
    return EndValue(context);
  }
  template <typename YggSchemaValueType>
  bool YggdrasilStartObject(Context& context, const SchemaType&, YggSchemaValueType& valueSchema) {
    BEGIN_NORMALIZE_(YggdrasilStartObject, (valueSchema), (kObjectType, valueSchema));
    if (!CurrentValue()->IsYggdrasil()) return false;
    if (CurrentValue()->GetValueSchema() != valueSchema) return false;
    return CurrentValue()->IsObject();
  }
  bool YggdrasilEndObject(Context& context, const SchemaType&, SizeType memberCount) {
    NORMALIZE_(YggdrasilEndObject, (memberCount));
    return EndValue(context);
  }
  bool StartObject(Context& context, const SchemaType&) {
    BEGIN_NORMALIZE_(StartObject, (), (kObjectType));
    return CurrentValue()->IsObject();
  }
  bool GetFinalAlias(Context& context, const ValueType& aliases,
		     const ValueType& orig, ValueType* dest) {
    if (!follow_aliases_(aliases, orig, dest, document_.GetAllocator())) {
      context.error_handler.CircularAlias(*dest);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorCircularAlias);
    }
    return true;
  }
  bool Key(Context& context, const SchemaType& schema, const Ch*& str, SizeType& len, bool copy, bool dont_check_aliases=false) {
    if (!dont_check_aliases) {
      const ValueType& aliases = AddAliases(schema);
      ValueType orig(str, len, document_.GetAllocator());
      ConstMemberIterator match = aliases.MemberEnd();
      ValueType primary;
      if (FindAliasName(aliases, orig, match)) {
	if (!GetFinalAlias(context, aliases, orig, &primary))
	  return false;
	modified_ = true;
	len = primary.GetStringLength();
	Ch* str0 = 0;
	str0 = static_cast<Ch *>(document_.GetAllocator().Malloc((len + 1) * sizeof(Ch)));
	std::memcpy(str0, primary.GetString(), len * sizeof(Ch));
	str0[len] = '\0';
	str = str0;
      } else if (FindAliasValue(aliases, orig, match)) {
	primary.CopyFrom(orig, document_.GetAllocator());
	orig.CopyFrom(match->name, document_.GetAllocator());
      }
      // Check previous keys for alias target
      if (match != aliases.MemberEnd()) {
	if (HasMember(primary)) {
	  // TODO: Check equivalence when the value is added?
	  context.error_handler.DuplicateAlias(orig, primary);
	  RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorAliasDuplicate);
	}
      }
    }
    NORMALIZE_(Key, (str, len, copy));
    PushKey(str, len);
    return true;
  }
  bool EndObject(Context& context, const SchemaType& schema, SizeType& memberCount) {
    // Default
    // TODO: Normalize default_?
    if ((!extending_) && (schema.hasRequired_)) {
      for (SizeType index = 0; index < schema.propertyCount_; index++) {
	if (schema.properties_[index].required && !context.propertyExist[index]
	    && !schema.properties_[index].schema->default_.IsNull()) {
	  modified_ = true;
	  const Ch* str = schema.properties_[index].name.GetString();
	  SizeType len = schema.properties_[index].name.GetStringLength();
	  if (!Key(context, schema, str, len, true))
	    return false;
	  if (!Append(context, schema, schema.properties_[index].schema->default_))
	    return false;
	  if (context.propertyExist)
	    context.propertyExist[index] = true;
	  memberCount++;
	}
      }
    }
    // Do EndObject
    NORMALIZE_(EndObject, (memberCount));
    return EndValue(context);
  }
  bool StartArray(Context& context, const SchemaType&) {
    BEGIN_NORMALIZE_(StartArray, (), (kArrayType));
    return CurrentValue()->IsArray();
  }
  bool EndArray(Context& context, const SchemaType&, SizeType elementCount) {
    NORMALIZE_(EndArray, (elementCount));
    return EndValue(context);
  }

#undef NORMALIZE_
#undef BEGIN_NORMALIZE_
#undef NORMALIZE_VALUE_

#define NORMALIZE_HANDLER_(method, ...)					\
  RAPIDJSON_ASSERT(extending_);						\
  return method(*extend_context_, *extend_schema_, __VA_ARGS__);

#define NORMALIZE_HANDLER_NOARGS_(method)		\
  RAPIDJSON_ASSERT(extending_);				\
  return method(*extend_context_, *extend_schema_);

  bool Null()             { NORMALIZE_HANDLER_NOARGS_(Null); }
  bool Bool(bool b)       { NORMALIZE_HANDLER_(Bool, b); }
  bool Int(int i)         { NORMALIZE_HANDLER_(Int,  i); }
  bool Uint(unsigned u)   { NORMALIZE_HANDLER_(Uint, u); }
  bool Int64(int64_t i)   { NORMALIZE_HANDLER_(Int64, i); }
  bool Uint64(uint64_t u) { NORMALIZE_HANDLER_(Uint64, u); }
  bool Double(double d)   { NORMALIZE_HANDLER_(Double , d); }
  bool String(const Ch* str, SizeType length, bool copy)
  { NORMALIZE_HANDLER_(String, str, length, copy); }
  template <typename YggSchemaValueType>
  bool YggdrasilString(const Ch* str, SizeType length, bool copy, YggSchemaValueType& schema)
  { NORMALIZE_HANDLER_(YggdrasilString, str, length, copy, schema); }
  template <typename YggSchemaValueType>
  bool YggdrasilStartObject(YggSchemaValueType& schema)
  { NORMALIZE_HANDLER_(YggdrasilStartObject, schema); }
  bool StartObject() { NORMALIZE_HANDLER_NOARGS_(StartObject); }
  bool Key(const Ch* str, SizeType len, bool copy)
  { NORMALIZE_HANDLER_(Key, str, len, copy); }
  bool EndObject(SizeType memberCount)
  { NORMALIZE_HANDLER_(EndObject, memberCount); }
  bool StartArray() { NORMALIZE_HANDLER_NOARGS_(StartArray); }
  bool EndArray(SizeType elementCount)
  { NORMALIZE_HANDLER_(EndArray, elementCount); }

#undef NORMALIZE_HANDLER_

  void SetDocumentStack(internal::Stack<AllocatorType>* stack) {
    documentStack_ = stack;
  }
      
private:

  ValueType* CurrentValue() {
    if (extending_ && !appending_) {
      RAPIDJSON_ASSERT(!valueStack_.Empty());
      return *valueStack_.template Top<ValueType*>();
    } else {
      return document_.StackTop();
    }
  }
  void PushValue(ValueType& value) {
    ValueType** ref = valueStack_.template Push<ValueType*>();
    ref[0] = &value;
  }
  void PopValue() { valueStack_.template Pop<ValueType*>(1); }
  ValueType* CurrentKey() {
    if (keyStack_.Empty())
      return nullptr;
    else
      return *keyStack_.template Top<ValueType*>();
  }
  void PushKey(const Ch* str, SizeType len) {
    ValueType** ref = keyStack_.template Push<ValueType*>();
    ref[0] = new ValueType(str, len, document_.GetAllocator());
  }
  void PushKey() {
    ValueType** ref = keyStack_.template Push<ValueType*>();
    ref[0] = nullptr;
  }
  void PopKey() {
    ValueType** ref = keyStack_.template Pop<ValueType*>(1);
    if (ref[0])
      delete ref[0];
  }
  ValueType GetAddress(bool parent=true) {
    RAPIDJSON_ASSERT(documentStack_);
    GenericStringBuffer<EncodingType> sb;
    PointerType instancePointer;
    if (documentStack_->Empty()) {
      instancePointer = PointerType();
    } else {
      instancePointer = PointerType(documentStack_->template Bottom<Ch>(), documentStack_->GetSize() / sizeof(Ch));
    }
    ((parent && (instancePointer.GetTokenCount() > 0))
     ? PointerType(instancePointer.GetTokens(), instancePointer.GetTokenCount() - 1)
     : instancePointer).StringifyUriFragment(sb);
    // instancePointer.StringifyUriFragment(sb);
    ValueType instanceRef(sb.GetString(), static_cast<SizeType>(sb.GetSize() / sizeof(Ch)),
			  document_.GetAllocator());
    return instanceRef;
  }
  bool HasMember(ValueType& key, ValueType* val=nullptr) {
    if (extending_ && !appending_) {
      if (CurrentValue()->HasMember(key)) {
	if (val)
	  val->CopyFrom(CurrentValue()->FindMember(key)->value,
			document_.GetAllocator());
	return true;
      }
      return false;
    }
    ValueType* base = document_.StackTop();
    if (base->IsObject())
      return false;
    while ((base != document_.StackBottom()) && (!base->IsObject())) base--;
    RAPIDJSON_ASSERT(base->IsObject());
    std::cerr << "HasMember type: " << base->GetType() << std::endl;
    base++;
    while (base != document_.StackTop()) {
      std::cerr << "HasMember type: " << base->GetType() << std::endl;
      RAPIDJSON_ASSERT(base->IsString());
      if (*base == key) {
	if (val && ((base + 1) != document_.StackTop()))
	  val->CopyFrom(*(base + 1),
			document_.GetAllocator());
	return true;
      }
      base++;
      if (base == document_.StackTop()) break;
      base++;
    }
    return false;
  }
  ValueType* Address2Value(const ValueType& address, ValueType* base = nullptr, size_t unfinalized=0) {
    if (!base) base = CurrentValue();
    size_t idx = 0;
    GenericPointer<ValueType> ptr;
    if (unfinalized) {
      ValueType current = GetAddress(false);
      ptr = GenericPointer<ValueType>(address.GetString() + current.GetStringLength(),
				      address.GetStringLength() - current.GetStringLength());
    } else {
      ptr = GenericPointer<ValueType>(address.GetString(), address.GetStringLength());
    }
    return ptr.Get(*base, &idx);
  }
  //! Add new aliases and check if the document contains any of them.
  bool ExtendAliases(Context& context, ValueType& aliases, bool* replaced) {
    *replaced = false;
    for (typename ValueType::ConstMemberIterator it = aliases.MemberBegin(); it != aliases.MemberEnd(); ++it) {
      if (!aliases_.HasMember(it->name)) {
	ValueType tmp(it->name.GetString(), it->name.GetStringLength(),
		      document_.GetAllocator());
	aliases_.AddMember(tmp, kObjectType, document_.GetAllocator());
      }
      for (typename ValueType::ConstMemberIterator v = it->value.MemberBegin(); v != it->value.MemberEnd(); ++v) {
	if (aliases_[it->name].HasMember(v->name)) {
	  typename ValueType::ConstMemberIterator existing = aliases_[it->name].FindMember(v->name);
	  if (existing->value != v->value) {
	    context.error_handler.ConflictingAliases(v->name, existing->value, v->value);
	    RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorConflictingAliases);
	  }
	} else {
	  ValueType key(v->name.GetString(), v->name.GetStringLength(),
			document_.GetAllocator());
	  ValueType val(v->value.GetString(), v->value.GetStringLength(),
			document_.GetAllocator());
	  aliases_[it->name].AddMember(key, val, document_.GetAllocator());
	  ValueType primary;
	  if (!GetFinalAlias(context, aliases_[it->name], v->name, &primary))
	    return false;
	  // Check if previous parallel schema normalizaiton included any of
	  // the aliased properties
	  ValueType* root;
	  size_t unfinalized = 0;
	  if (document_.WasFinalized()) {
	    root = &document_;
	  } else {
	    unfinalized = 1;
	    root = document_.StackTop();
	  }
	  ValueType* base = Address2Value(it->name, root, unfinalized);
	  if (!base) continue;
	  RAPIDJSON_ASSERT(base->IsObject());
	  if (base->HasMember(v->name)) {
	    typename ValueType::MemberIterator old = base->FindMember(v->name);
	    if (base->HasMember(primary)) {
	      typename ValueType::MemberIterator alt = base->FindMember(primary);
	      if (alt->value != old->value) {
		context.error_handler.DuplicateAlias(v->value, v->name);
		RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorAliasDuplicate);
	      }
	    } else {
	      *replaced = true;
	      ValueType new_val;
	      new_val.CopyFrom(old->value, document_.GetAllocator());
	      base->AddMember(ValueType(v->value.GetString(),
					v->value.GetStringLength(),
					document_.GetAllocator()),
			      new_val,
			      document_.GetAllocator());
	      base->RemoveMember(v->name);
	    }
	  }
	}
      }
    }
    if (*replaced) modified_ = true;
    return true;
  }
  bool FindAliasName(const ValueType& aliases, ValueType& name,
		     ConstMemberIterator& match) {
    match = aliases.FindMember(name);
    return (match != aliases.MemberEnd());
  }
  bool FindAliasValue(const ValueType& aliases, ValueType& name,
		      ConstMemberIterator& match) {
    match = aliases.MemberBegin();
    for ( ; match != aliases.MemberEnd(); ++match)
      if (name == match->value)
	break;
    return (match != aliases.MemberEnd());
  }
  ValueType& GetAliases() {
    const ValueType address = GetAddress();
    if (!aliases_.HasMember(address)) {
      ValueType tmp(address.GetString(), address.GetStringLength(), document_.GetAllocator());
      aliases_.AddMember(tmp, kObjectType, document_.GetAllocator());
    }
    return aliases_[address];
  }
  const ValueType& AddAliases(const SchemaType& schema) {
    ValueType& aliases = GetAliases();
    if (schema.child_aliases_.MemberCount() == 0)
      return aliases;
    for (typename SValue::ConstMemberIterator it = schema.child_aliases_.MemberBegin(); it != schema.child_aliases_.MemberEnd(); ++it) {
      if (!aliases.HasMember(it->name.GetString())) {
	ValueType key1(it->name.GetString(), it->name.GetStringLength(),
		       document_.GetAllocator());
	ValueType key2(it->value.GetString(), it->value.GetStringLength(),
		       document_.GetAllocator());
	aliases.AddMember(key1, key2, document_.GetAllocator());
      }
    }
    return aliases;
  }

  static const size_t kDefaultStackCapacity = 1024;
  DocumentType document_;
  unsigned index_;
  bool modified_;
  bool extending_;
  bool appending_;
  Context* extend_context_;
  const SchemaType* extend_schema_;
  internal::Stack<AllocatorType> keyStack_;
  internal::Stack<AllocatorType> valueStack_;
  internal::Stack<AllocatorType> childStack_;
  internal::Stack<AllocatorType>* documentStack_;
  ValueType aliases_;
};

#endif // RAPIDJSON_YGGDRASIL

///////////////////////////////////////////////////////////////////////////////
// Schema

template <typename SchemaDocumentType>
class Schema {
public:
    typedef typename SchemaDocumentType::ValueType ValueType;
    typedef typename SchemaDocumentType::AllocatorType AllocatorType;
    typedef typename SchemaDocumentType::PointerType PointerType;
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;
    typedef SchemaValidationContext<SchemaDocumentType> Context;
    typedef Schema<SchemaDocumentType> SchemaType;
    typedef GenericValue<EncodingType, AllocatorType> SValue;
    typedef IValidationErrorHandler<Schema> ErrorHandler;
    typedef GenericUri<ValueType, AllocatorType> UriType;
    friend class GenericSchemaDocument<ValueType, AllocatorType>;
#ifdef RAPIDJSON_YGGDRASIL
    friend class GenericNormalizedDocument<SchemaDocumentType, RAPIDJSON_DEFAULT_STACK_ALLOCATOR>;
#endif // RAPIDJSON_YGGDRASIL

    Schema(SchemaDocumentType* schemaDocument, const PointerType& p, const ValueType& value, const ValueType& document, AllocatorType* allocator, const UriType& id = UriType()
#ifdef RAPIDJSON_YGGDRASIL
	   , const bool isMetaschema = false
#endif // RAPIDJSON_YGGDRASIL
	   ) :
        allocator_(allocator),
        uri_(schemaDocument->GetURI(), *allocator),
        id_(id),
        pointer_(p, allocator),
        typeless_(schemaDocument->GetTypeless()),
        enum_(),
        enumCount_(),
        not_(),
        type_((1 << kTotalSchemaType) - 1), // typeless
        validatorCount_(),
        notValidatorIndex_(),
        properties_(),
        additionalPropertiesSchema_(),
        patternProperties_(),
        patternPropertyCount_(),
        propertyCount_(),
        minProperties_(),
        maxProperties_(SizeType(~0)),
        additionalProperties_(true),
        hasDependencies_(),
        hasRequired_(),
        hasSchemaDependencies_(),
        additionalItemsSchema_(),
        itemsList_(),
        itemsTuple_(),
        itemsTupleCount_(),
        minItems_(),
        maxItems_(SizeType(~0)),
        additionalItems_(true),
        uniqueItems_(false),
        pattern_(),
        minLength_(0),
        maxLength_(~SizeType(0)),
        exclusiveMinimum_(false),
        exclusiveMaximum_(false),
        defaultValueLength_(0)
#ifdef RAPIDJSON_YGGDRASIL
	,
	yggtype_(kYggNullSchemaType),
	subtype_(kYggNullSubType),
	precision_(),
	shape_(),
	args_(),
	kwargs_(),
	isMetaschema_(isMetaschema),
	metaschema_(),
	metaschemaValidatorIndex_(),
	aliases_(kArrayType), child_aliases_(kObjectType), hasAliases_(false)
#endif // RAPIDJSON_YGGDRASIL
    {
        typedef typename ValueType::ConstValueIterator ConstValueIterator;
        typedef typename ValueType::ConstMemberIterator ConstMemberIterator;

        // PR #1393
        // Early add this Schema and its $ref(s) in schemaDocument's map to avoid infinite
        // recursion (with recursive schemas), since schemaDocument->getSchema() is always
        // checked before creating a new one. Don't cache typeless_, though.
        if (this != typeless_) {
          typedef typename SchemaDocumentType::SchemaEntry SchemaEntry;
          SchemaEntry *entry = schemaDocument->schemaMap_.template Push<SchemaEntry>();
          new (entry) SchemaEntry(pointer_, this, true, allocator_);
          schemaDocument->AddSchemaRefs(this);
        }

        if (!value.IsObject())
            return;

        // If we have an id property, resolve it with the in-scope id
        if (const ValueType* v = GetMember(value, GetIdString())) {
            if (v->IsString()) {
                UriType local(*v, allocator);
                id_ = local.Resolve(id_, allocator);
            }
        }

        if (const ValueType* v = GetMember(value, GetTypeString())) {
            type_ = 0;
#ifdef RAPIDJSON_YGGDRASIL
	    yggtype_ = 0;
#endif // RAPIDJSON_YGGDRASIL
            if (v->IsString())
                AddType(*v);
            else if (v->IsArray())
                for (ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr)
                    AddType(*itr);
        }

        if (const ValueType* v = GetMember(value, GetEnumString())) {
            if (v->IsArray() && v->Size() > 0) {
                enum_ = static_cast<uint64_t*>(allocator_->Malloc(sizeof(uint64_t) * v->Size()));
                for (ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr) {
                    typedef Hasher<EncodingType, MemoryPoolAllocator<> > EnumHasherType;
                    char buffer[256u + 24];
                    MemoryPoolAllocator<> hasherAllocator(buffer, sizeof(buffer));
                    EnumHasherType h(&hasherAllocator, 256);
                    itr->Accept(h);
                    enum_[enumCount_++] = h.GetHashCode();
                }
            }
        }

        if (schemaDocument) {
            AssignIfExist(allOf_, *schemaDocument, p, value, GetAllOfString(), document);
            AssignIfExist(anyOf_, *schemaDocument, p, value, GetAnyOfString(), document);
            AssignIfExist(oneOf_, *schemaDocument, p, value, GetOneOfString(), document);

            if (const ValueType* v = GetMember(value, GetNotString())) {
                schemaDocument->CreateSchema(&not_, p.Append(GetNotString(), allocator_), *v, document, id_);
                notValidatorIndex_ = validatorCount_;
                validatorCount_++;
            }

#ifdef RAPIDJSON_YGGDRASIL
	    if (yggtype_ & ((1 << kYggSchemaSchemaType) | (1 << kYggPythonInstanceSchemaType))) {
	      schemaDocument->CreateMetaSchema(&metaschema_, (yggtype_ & (1 << kYggPythonInstanceSchemaType)));
	      metaschemaValidatorIndex_ = validatorCount_;
	      validatorCount_++;
	    }
#endif // RAPIDJSON_YGGDRASIL
	}

        // Object

        const ValueType* properties = GetMember(value, GetPropertiesString());
        const ValueType* required = GetMember(value, GetRequiredString());
        const ValueType* dependencies = GetMember(value, GetDependenciesString());
        {
            // Gather properties from properties/required/dependencies
            SValue allProperties(kArrayType);

            if (properties && properties->IsObject())
                for (ConstMemberIterator itr = properties->MemberBegin(); itr != properties->MemberEnd(); ++itr)
                    AddUniqueElement(allProperties, itr->name);

            if (required && required->IsArray())
                for (ConstValueIterator itr = required->Begin(); itr != required->End(); ++itr)
                    if (itr->IsString())
                        AddUniqueElement(allProperties, *itr);

            if (dependencies && dependencies->IsObject())
                for (ConstMemberIterator itr = dependencies->MemberBegin(); itr != dependencies->MemberEnd(); ++itr) {
                    AddUniqueElement(allProperties, itr->name);
                    if (itr->value.IsArray())
                        for (ConstValueIterator i = itr->value.Begin(); i != itr->value.End(); ++i)
                            if (i->IsString())
                                AddUniqueElement(allProperties, *i);
                }

            if (allProperties.Size() > 0) {
                propertyCount_ = allProperties.Size();
                properties_ = static_cast<Property*>(allocator_->Malloc(sizeof(Property) * propertyCount_));
                for (SizeType i = 0; i < propertyCount_; i++) {
                    new (&properties_[i]) Property();
                    properties_[i].name = allProperties[i];
                    properties_[i].schema = typeless_;
                }
            }
        }

#ifdef RAPIDJSON_YGGDRASIL
	if (const ValueType* v = GetMember(value, GetAliasesString())) {
	  if (v->IsArray())
	    aliases_.CopyFrom(*v, *allocator_);
	}
#endif // RAPIDJSON_YGGDRASIL

        if (properties && properties->IsObject()) {
            PointerType q = p.Append(GetPropertiesString(), allocator_);
	    SValue child_aliases(kObjectType);
            for (ConstMemberIterator itr = properties->MemberBegin(); itr != properties->MemberEnd(); ++itr) {
                SizeType index;
                if (FindPropertyIndex(itr->name, &index))
#ifdef RAPIDJSON_YGGDRASIL
		  {
#endif // RAPIDJSON_YGGDRASIL
                    schemaDocument->CreateSchema(&properties_[index].schema, q.Append(itr->name, allocator_), itr->value, document, id_);
#ifdef RAPIDJSON_YGGDRASIL
		    if (properties_[index].schema->aliases_.Size() > 0) {
		      hasAliases_ = true;
		      for (typename SValue::ConstValueIterator itv = properties_[index].schema->aliases_.Begin(); itv != properties_[index].schema->aliases_.End(); ++itv)
			child_aliases.AddMember(SValue(itv->GetString(),
						       itv->GetStringLength(),
						       *allocator_),
						SValue(itr->name.GetString(),
						       itr->name.GetStringLength(),
						       *allocator_),
						*allocator_);
		    }
		  }
#endif // RAPIDJSON_YGGDRASIL
            }
#ifdef RAPIDJSON_YGGDRASIL
	    child_aliases_.CopyFrom(child_aliases, *allocator_);
#endif // RAPIDJSON_YGGDRASIL	    
	}

        if (const ValueType* v = GetMember(value, GetPatternPropertiesString())) {
            PointerType q = p.Append(GetPatternPropertiesString(), allocator_);
            patternProperties_ = static_cast<PatternProperty*>(allocator_->Malloc(sizeof(PatternProperty) * v->MemberCount()));
            patternPropertyCount_ = 0;

            for (ConstMemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr) {
                new (&patternProperties_[patternPropertyCount_]) PatternProperty();
                patternProperties_[patternPropertyCount_].pattern = CreatePattern(itr->name);
                schemaDocument->CreateSchema(&patternProperties_[patternPropertyCount_].schema, q.Append(itr->name, allocator_), itr->value, document, id_);
                patternPropertyCount_++;
            }
        }

        if (required && required->IsArray())
            for (ConstValueIterator itr = required->Begin(); itr != required->End(); ++itr)
                if (itr->IsString()) {
                    SizeType index;
                    if (FindPropertyIndex(*itr, &index)) {
                        properties_[index].required = true;
                        hasRequired_ = true;
                    }
                }

        if (dependencies && dependencies->IsObject()) {
            PointerType q = p.Append(GetDependenciesString(), allocator_);
            hasDependencies_ = true;
            for (ConstMemberIterator itr = dependencies->MemberBegin(); itr != dependencies->MemberEnd(); ++itr) {
                SizeType sourceIndex;
                if (FindPropertyIndex(itr->name, &sourceIndex)) {
                    if (itr->value.IsArray()) {
                        properties_[sourceIndex].dependencies = static_cast<bool*>(allocator_->Malloc(sizeof(bool) * propertyCount_));
                        std::memset(properties_[sourceIndex].dependencies, 0, sizeof(bool)* propertyCount_);
                        for (ConstValueIterator targetItr = itr->value.Begin(); targetItr != itr->value.End(); ++targetItr) {
                            SizeType targetIndex;
                            if (FindPropertyIndex(*targetItr, &targetIndex))
                                properties_[sourceIndex].dependencies[targetIndex] = true;
                        }
                    }
                    else if (itr->value.IsObject()) {
                        hasSchemaDependencies_ = true;
                        schemaDocument->CreateSchema(&properties_[sourceIndex].dependenciesSchema, q.Append(itr->name, allocator_), itr->value, document, id_);
                        properties_[sourceIndex].dependenciesValidatorIndex = validatorCount_;
                        validatorCount_++;
                    }
                }
            }
        }

        if (const ValueType* v = GetMember(value, GetAdditionalPropertiesString())) {
            if (v->IsBool())
                additionalProperties_ = v->GetBool();
            else if (v->IsObject())
                schemaDocument->CreateSchema(&additionalPropertiesSchema_, p.Append(GetAdditionalPropertiesString(), allocator_), *v, document, id_);
        }

        AssignIfExist(minProperties_, value, GetMinPropertiesString());
        AssignIfExist(maxProperties_, value, GetMaxPropertiesString());

        // Array
        if (const ValueType* v = GetMember(value, GetItemsString())) {
            PointerType q = p.Append(GetItemsString(), allocator_);
            if (v->IsObject()) // List validation
                schemaDocument->CreateSchema(&itemsList_, q, *v, document, id_);
            else if (v->IsArray()) { // Tuple validation
                itemsTuple_ = static_cast<const Schema**>(allocator_->Malloc(sizeof(const Schema*) * v->Size()));
                SizeType index = 0;
                for (ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr, index++)
                    schemaDocument->CreateSchema(&itemsTuple_[itemsTupleCount_++], q.Append(index, allocator_), *itr, document, id_);
            }
        }

        AssignIfExist(minItems_, value, GetMinItemsString());
        AssignIfExist(maxItems_, value, GetMaxItemsString());

        if (const ValueType* v = GetMember(value, GetAdditionalItemsString())) {
            if (v->IsBool())
                additionalItems_ = v->GetBool();
            else if (v->IsObject())
                schemaDocument->CreateSchema(&additionalItemsSchema_, p.Append(GetAdditionalItemsString(), allocator_), *v, document, id_);
        }

        AssignIfExist(uniqueItems_, value, GetUniqueItemsString());

        // String
        AssignIfExist(minLength_, value, GetMinLengthString());
        AssignIfExist(maxLength_, value, GetMaxLengthString());

        if (const ValueType* v = GetMember(value, GetPatternString()))
            pattern_ = CreatePattern(*v);

        // Number
        if (const ValueType* v = GetMember(value, GetMinimumString()))
            if (v->IsNumber())
                minimum_.CopyFrom(*v, *allocator_);

        if (const ValueType* v = GetMember(value, GetMaximumString()))
            if (v->IsNumber())
                maximum_.CopyFrom(*v, *allocator_);

        AssignIfExist(exclusiveMinimum_, value, GetExclusiveMinimumString());
        AssignIfExist(exclusiveMaximum_, value, GetExclusiveMaximumString());

        if (const ValueType* v = GetMember(value, GetMultipleOfString()))
            if (v->IsNumber() && v->GetDouble() > 0.0)
                multipleOf_.CopyFrom(*v, *allocator_);

        // Default
        if (const ValueType* v = GetMember(value, GetDefaultValueString()))
#ifdef RAPIDJSON_YGGDRASIL
	  {
	    default_.CopyFrom(*v, *allocator_);
#endif // RAPIDJSON_YGGDRASIL
            if (v->IsString())
                defaultValueLength_ = v->GetStringLength();
#ifdef RAPIDJSON_YGGDRASIL
	  }
#endif // RAPIDJSON_YGGDRASIL

#ifdef RAPIDJSON_YGGDRASIL
	// Yggdrasil properties
	if (const ValueType* v = GetMember(value, GetSubTypeString())) {
	  subtype_ = kYggNullSubType;
	  if (v->IsString())
	    AddSubType(*v);
	}
	if (const ValueType* v = GetMember(value, GetPrecisionString())) {
	    precision_ = 0;
	    if (v->IsNumber() && v->GetInt() >= 0)
	        precision_.CopyFrom(*v, *allocator_);
	}
	if (const ValueType* v = GetMember(value, GetUnitsString())) {
	    if (v->IsString())
	        units_.CopyFrom(*v, *allocator_);
	}
	if (const ValueType* v = GetMember(value, GetLengthString())) {
	    ValueType shp(kArrayType);
	    if (v->IsNumber() && v->GetInt() > 0) {
		shape_.CopyFrom(shp, *allocator_);
		shape_.PushBack(static_cast<SizeType>(v->GetUint()), *allocator_);
	    }
	}
	if (const ValueType* v = GetMember(value, GetShapeString())) {
	    if (v->IsArray())
	        shape_.CopyFrom(*v, *allocator_);
	}
	if (const ValueType* v = GetMember(value, GetArgsString())) {
	    if (v->IsArray())
	        args_.CopyFrom(*v, *allocator_);
	}
	if (const ValueType* v = GetMember(value, GetKwargsString())) {
	    if (v->IsObject())
	        kwargs_.CopyFrom(*v, *allocator_);
	}
#endif // RAPIDJSON_YGGDRASIL

    }

    ~Schema() {
        AllocatorType::Free(enum_);
        if (properties_) {
            for (SizeType i = 0; i < propertyCount_; i++)
                properties_[i].~Property();
            AllocatorType::Free(properties_);
        }
        if (patternProperties_) {
            for (SizeType i = 0; i < patternPropertyCount_; i++)
                patternProperties_[i].~PatternProperty();
            AllocatorType::Free(patternProperties_);
        }
        AllocatorType::Free(itemsTuple_);
#if RAPIDJSON_SCHEMA_HAS_REGEX
        if (pattern_) {
            pattern_->~RegexType();
            AllocatorType::Free(pattern_);
        }
#endif
    }

    const SValue& GetURI() const {
        return uri_;
    }

    const UriType& GetId() const {
        return id_;
    }

    const PointerType& GetPointer() const {
        return pointer_;
    }

    bool BeginValue(Context& context) const {
        if (context.inArray) {
            if (uniqueItems_)
                context.valueUniqueness = true;

            if (itemsList_)
                context.valueSchema = itemsList_;
            else if (itemsTuple_) {
                if (context.arrayElementIndex < itemsTupleCount_)
                    context.valueSchema = itemsTuple_[context.arrayElementIndex];
                else if (additionalItemsSchema_)
                    context.valueSchema = additionalItemsSchema_;
                else if (additionalItems_)
                    context.valueSchema = typeless_;
                else {
                    context.error_handler.DisallowedItem(context.arrayElementIndex);
                    // Must set valueSchema for when kValidateContinueOnErrorFlag is set, else reports spurious type error
                    context.valueSchema = typeless_;
                    // Must bump arrayElementIndex for when kValidateContinueOnErrorFlag is set
                    context.arrayElementIndex++;
                    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorAdditionalItems);
                }
            }
            else
                context.valueSchema = typeless_;

            context.arrayElementIndex++;
        }
#ifdef RAPIDJSON_YGGDRASIL
	if (hasAliases_) {
	  for (typename SValue::ConstMemberIterator a = child_aliases_.MemberBegin(); a != child_aliases_.MemberEnd(); a++) {
	    for (typename SValue::ConstMemberIterator b = a + 1; b != child_aliases_.MemberEnd(); b++) {
	      if ((a->name == b->name) && (a->value != b->value)) {
		context.error_handler.ConflictingAliases(a->name, a->value, b->value);
		RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorConflictingAliases);
	      }
	    }
	  }
	}
#endif // RAPIDJSON_YGGDRASIL
        return true;
    }

    RAPIDJSON_FORCEINLINE bool EndValue(Context& context) const {
        // Only check pattern properties if we have validators
        if (context.patternPropertiesValidatorCount > 0) {
            bool otherValid = false;
            SizeType count = context.patternPropertiesValidatorCount;
            if (context.objectPatternValidatorType != Context::kPatternValidatorOnly)
                otherValid = context.patternPropertiesValidators[--count]->IsValid();

            bool patternValid = true;
            for (SizeType i = 0; i < count; i++)
                if (!context.patternPropertiesValidators[i]->IsValid()) {
                    patternValid = false;
                    break;
                }

            if (context.objectPatternValidatorType == Context::kPatternValidatorOnly) {
                if (!patternValid) {
                    context.error_handler.PropertyViolations(context.patternPropertiesValidators, count);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorPatternProperties);
                }
            }
            else if (context.objectPatternValidatorType == Context::kPatternValidatorWithProperty) {
                if (!patternValid || !otherValid) {
                    context.error_handler.PropertyViolations(context.patternPropertiesValidators, count + 1);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorPatternProperties);
                }
            }
            else if (!patternValid && !otherValid) { // kPatternValidatorWithAdditionalProperty)
                context.error_handler.PropertyViolations(context.patternPropertiesValidators, count + 1);
                RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorPatternProperties);
            }
        }

        // For enums only check if we have a hasher
        if (enum_ && context.hasher) {
            const uint64_t h = context.factory.GetHashCode(context.hasher);
            for (SizeType i = 0; i < enumCount_; i++)
                if (enum_[i] == h)
                    goto foundEnum;
            context.error_handler.DisallowedValue(kValidateErrorEnum);
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorEnum);
            foundEnum:;
        }

        // Only check allOf etc if we have validators
        if (context.validatorCount > 0) {
            if (allOf_.schemas)
                for (SizeType i = allOf_.begin; i < allOf_.begin + allOf_.count; i++)
                    if (!context.validators[i]->IsValid()) {
                        context.error_handler.NotAllOf(&context.validators[allOf_.begin], allOf_.count);
                        RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorAllOf);
                    }

            if (anyOf_.schemas) {
                for (SizeType i = anyOf_.begin; i < anyOf_.begin + anyOf_.count; i++)
                    if (context.validators[i]->IsValid())
                        goto foundAny;
                context.error_handler.NoneOf(&context.validators[anyOf_.begin], anyOf_.count);
                RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorAnyOf);
                foundAny:;
            }

            if (oneOf_.schemas) {
                bool oneValid = false;
                for (SizeType i = oneOf_.begin; i < oneOf_.begin + oneOf_.count; i++)
                    if (context.validators[i]->IsValid()) {
                        if (oneValid) {
                            context.error_handler.NotOneOf(&context.validators[oneOf_.begin], oneOf_.count, true);
                            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorOneOfMatch);
                        } else
                            oneValid = true;
                    }
                if (!oneValid) {
                    context.error_handler.NotOneOf(&context.validators[oneOf_.begin], oneOf_.count, false);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorOneOf);
                }
            }

            if (not_ && context.validators[notValidatorIndex_]->IsValid()) {
                context.error_handler.Disallowed();
                RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorNot);
            }

#ifdef RAPIDJSON_YGGDRASIL
	    if (metaschema_ && !context.validators[metaschemaValidatorIndex_]->IsValid()) {
	      ValidateErrorCode code;
	      if ((yggtype_ & (1 << kYggSchemaSchemaType)))
		code = kValidateErrorInvalidSchema;
	      else
		code = kValidateErrorInvalidSchema;
	      context.error_handler.InvalidSchema(code, context.validators[metaschemaValidatorIndex_]);
	      RAPIDJSON_INVALID_KEYWORD_RETURN(code);
	    }

	    if (context.normalized) {
	      for (SizeType i = 0; i < context.validatorCount; i++) {
		if (context.validators[i]->IsValid()) {
		  if (!context.normalized->ExtendChild(context, *this,
						       context.validators[i]->GetValidatorID()))
		    return false;
		  break;
		}
	      }
	      
	      if (allOf_.schemas)
                for (SizeType i = allOf_.begin; i < allOf_.begin + allOf_.count; i++)
		  if (!context.normalized->ExtendChild(context, *this,
						       context.validators[i]->GetValidatorID()))
		    return false;

	      if (anyOf_.schemas) {
                for (SizeType i = anyOf_.begin; i < anyOf_.begin + anyOf_.count; i++) {
		  if (context.validators[i]->IsValid()) {
		    if (!context.normalized->ExtendChild(context, *this,
							 context.validators[i]->GetValidatorID()))
		      return false;
		    break;
		  }
		}
	      }

	      if (oneOf_.schemas) {
                for (SizeType i = oneOf_.begin; i < oneOf_.begin + oneOf_.count; i++) {
		  if (context.validators[i]->IsValid()) {
		    if (!context.normalized->ExtendChild(context, *this,
							 context.validators[i]->GetValidatorID()))
		      return false;
		    break;
		  }
		}
	      }
	      
	      if (metaschema_)
		if (!context.normalized->ExtendChild(context, *this,
						     context.validators[metaschemaValidatorIndex_]->GetValidatorID()))
		  return false;

	    }
#endif // RAPIDJSON_YGGDRASIL
	    
        }

        return true;
    }
  
#ifdef RAPIDJSON_YGGDRASIL
  // TODO: Error about normalization
#define RAPIDJSON_NORMALIZER_BASE_(method, arg)				\
  if (context.normalized) {						\
    if (!context.normalized->method arg)				\
      return false;							\
  }
#define RAPIDJSON_NORMALIZER_NOARG_(method)				\
  RAPIDJSON_NORMALIZER_BASE_(method, (context, *this))
#define RAPIDJSON_NORMALIZER_(method, ...)				\
  RAPIDJSON_NORMALIZER_BASE_(method, (context, *this, __VA_ARGS__))
#else
#define RAPIDJSON_NORMALIZER_BASE_(method, arg)	 \
  {}
#define RAPIDJSON_NORMALIZER_NOARG_(method)	\
  {}
#define RAPIDJSON_NORMALIZER_(method, ...)       \
  {}
#endif // RAPIDJSON_YGGDRASIL  

    bool Null(Context& context) const {
        RAPIDJSON_NORMALIZER_NOARG_(Null);
        if (!(type_ & (1 << kNullSchemaType))) {
            DisallowedType(context, GetNullString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }
        return CreateParallelValidator(context);
    }

    bool Bool(Context& context, bool b) const {
        (void)b;
        RAPIDJSON_NORMALIZER_(Bool, b);
        if (!(type_ & (1 << kBooleanSchemaType))) {
            DisallowedType(context, GetBooleanString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }
        return CreateParallelValidator(context);
    }

    bool Int(Context& context, int i) const {
        RAPIDJSON_NORMALIZER_(Int, i);
        if (!CheckInt(context, i))
            return false;
        return CreateParallelValidator(context);
    }

    bool Uint(Context& context, unsigned u) const {
        RAPIDJSON_NORMALIZER_(Uint, u);
        if (!CheckUint(context, u))
            return false;
        return CreateParallelValidator(context);
    }

    bool Int64(Context& context, int64_t i) const {
        RAPIDJSON_NORMALIZER_(Int64, i);
        if (!CheckInt(context, i))
            return false;
        return CreateParallelValidator(context);
    }

    bool Uint64(Context& context, uint64_t u) const {
        RAPIDJSON_NORMALIZER_(Uint64, u);
        if (!CheckUint(context, u))
            return false;
        return CreateParallelValidator(context);
    }

    bool Double(Context& context, double d) const {
        RAPIDJSON_NORMALIZER_(Double, d);
#ifdef RAPIDJSON_YGGDRASIL
      if ((yggtype_ & (1 << kYggScalarSchemaType))) {
	if (!(CheckScalar(context, GetFloatSubTypeString(), ValueType(8), ValueType())))
	  return false;
      }
#endif // RAPIDJSON_YGGDRASIL
        if (!(type_ & (1 << kNumberSchemaType))) {
            DisallowedType(context, GetNumberString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }

        if (!minimum_.IsNull() && !CheckDoubleMinimum(context, d))
            return false;

        if (!maximum_.IsNull() && !CheckDoubleMaximum(context, d))
            return false;

        if (!multipleOf_.IsNull() && !CheckDoubleMultipleOf(context, d))
            return false;

        return CreateParallelValidator(context);
    }

    bool String(Context& context, const Ch* str, SizeType length, bool copy) const {
        RAPIDJSON_NORMALIZER_(String, str, length, copy);
	(void)copy;
        if (!(type_ & (1 << kStringSchemaType))) {
            DisallowedType(context, GetStringString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }

        if (minLength_ != 0 || maxLength_ != SizeType(~0)) {
            SizeType count;
            if (internal::CountStringCodePoint<EncodingType>(str, length, &count)) {
                if (count < minLength_) {
                    context.error_handler.TooShort(str, length, minLength_);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMinLength);
                }
                if (count > maxLength_) {
                    context.error_handler.TooLong(str, length, maxLength_);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMaxLength);
                }
            }
        }

        if (pattern_ && !IsPatternMatch(pattern_, str, length)) {
            context.error_handler.DoesNotMatch(str, length);
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorPattern);
        }

#ifdef RAPIDJSON_YGGDRASIL
	if (yggtype_ & (1 << kYggPythonImportSchemaType))
	  if (!CheckPythonImport(context, str, length))
	    return false;
#endif // RAPIDJSON_YGGDRASIL

        return CreateParallelValidator(context);
    }

#ifdef RAPIDJSON_YGGDRASIL
  template <typename YggSchemaValueType>
  bool YggdrasilString(Context& context, const Ch* str, SizeType length, bool copy, YggSchemaValueType& schema) const {
    RAPIDJSON_NORMALIZER_(YggdrasilString, str, length, copy, schema);
    if (!CheckRequiredSchemaProperty(context, schema, GetTypeString()))
      return false;
    typename YggSchemaValueType::ConstMemberIterator vs = schema.FindMember(GetTypeString());
    const ValueType v(vs->value.GetString(), vs->value.GetStringLength());
    if ((v == GetScalarString()) &&
	(yggtype_ & (1 << kYggScalarSchemaType))) {
      if (!CheckSubType(context, schema))
	return false;
      if (!CheckPrecision(context, schema))
	return false;
      if (!CheckUnits(context, schema))
	return false;
    } else if (((v == Get1DArrayString()) || (v == GetNDArrayString())) &&
	       (yggtype_ & (1 << kYggNDArraySchemaType))) {
      if (!CheckSubType(context, schema))
	return false;
      if (!CheckPrecision(context, schema))
	return false;
      if (!CheckUnits(context, schema))
	return false;
      if (!CheckShape(context, schema))
	return false;
    } else if (((v == GetPythonClassString()) ||
		(v == GetPythonFunctionString())) &&
	       (yggtype_ & (1 << kYggPythonImportSchemaType))) {
      if (!CheckPythonImport(context, str, length))
	return false;
    } else {
      DisallowedType(context, v);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
    }
    return true;
  }
  template <typename YggSchemaValueType>
  bool YggdrasilStartObject(Context& context, YggSchemaValueType& schema) const {
    RAPIDJSON_NORMALIZER_(YggdrasilStartObject, schema);
    if (!CheckRequiredSchemaProperty(context, schema, GetTypeString()))
      return false;
    const typename YggSchemaValueType::ConstMemberIterator vs = schema.FindMember(GetTypeString());
    const ValueType v(vs->value.GetString(), vs->value.GetStringLength());
    if (isMetaschema_) {
      if (!((v == GetPythonInstanceString()) || (v == GetSchemaString()))) {
	DisallowedType(context, v);
	RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
      }
    } else if (!(((v == GetPythonInstanceString()) &&
		  (yggtype_ & (1 << kYggPythonInstanceSchemaType)))
		 || ((v == GetSchemaString()) &&
		     (yggtype_ & (1 << kYggSchemaSchemaType))))) {
      DisallowedType(context, v);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
    }
    return StartObject(context);
  }
  bool YggdrasilEndObject(Context& context, SizeType memberCount) const {
    RAPIDJSON_NORMALIZER_(YggdrasilEndObject, memberCount);
    return EndObject(context, memberCount);
  }
#endif // RAPIDJSON_YGGDRASIL

    bool StartObject(Context& context) const {
        RAPIDJSON_NORMALIZER_NOARG_(StartObject);
        if (!(type_ & (1 << kObjectSchemaType))) {
            DisallowedType(context, GetObjectString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }

        if (hasDependencies_ || hasRequired_) {
            context.propertyExist = static_cast<bool*>(context.factory.MallocState(sizeof(bool) * propertyCount_));
            std::memset(context.propertyExist, 0, sizeof(bool) * propertyCount_);
        }

        if (patternProperties_) { // pre-allocate schema array
            SizeType count = patternPropertyCount_ + 1; // extra for valuePatternValidatorType
            context.patternPropertiesSchemas = static_cast<const SchemaType**>(context.factory.MallocState(sizeof(const SchemaType*) * count));
            context.patternPropertiesSchemaCount = 0;
            std::memset(context.patternPropertiesSchemas, 0, sizeof(SchemaType*) * count);
        }

        return CreateParallelValidator(context);
    }

    bool Key(Context& context, const Ch* str, SizeType len, bool copy) const {
        RAPIDJSON_NORMALIZER_(Key, str, len, copy);
	(void)copy;
#ifdef RAPIDJSON_YGGDRASIL
	if (child_aliases_.HasMember(str)) {
	  SValue orig(str, len, *allocator_);
	  SValue dest;
	  if (!follow_aliases_(child_aliases_, orig, &dest, *allocator_)) {
	    context.error_handler.CircularAlias(dest);
	    RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorCircularAlias);
	    return false;
	  }
	  str = dest.GetString();
	  len = dest.GetStringLength();
	  copy = true;
	}
#endif // RAPIDJSON_YGGDRASIL
        if (patternProperties_) {
            context.patternPropertiesSchemaCount = 0;
            for (SizeType i = 0; i < patternPropertyCount_; i++)
                if (patternProperties_[i].pattern && IsPatternMatch(patternProperties_[i].pattern, str, len)) {
                    context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = patternProperties_[i].schema;
                    context.valueSchema = typeless_;
                }
        }

        SizeType index  = 0;
        if (FindPropertyIndex(ValueType(str, len).Move(), &index)) {
            if (context.patternPropertiesSchemaCount > 0) {
                context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = properties_[index].schema;
                context.valueSchema = typeless_;
                context.valuePatternValidatorType = Context::kPatternValidatorWithProperty;
            }
            else
                context.valueSchema = properties_[index].schema;

            if (context.propertyExist)
                context.propertyExist[index] = true;

            return true;
        }

        if (additionalPropertiesSchema_) {
            if (context.patternPropertiesSchemaCount > 0) {
                context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = additionalPropertiesSchema_;
                context.valueSchema = typeless_;
                context.valuePatternValidatorType = Context::kPatternValidatorWithAdditionalProperty;
            }
            else
                context.valueSchema = additionalPropertiesSchema_;
            return true;
        }
        else if (additionalProperties_) {
            context.valueSchema = typeless_;
            return true;
        }

        if (context.patternPropertiesSchemaCount == 0) { // patternProperties are not additional properties
            // Must set valueSchema for when kValidateContinueOnErrorFlag is set, else reports spurious type error
            context.valueSchema = typeless_;
            context.error_handler.DisallowedProperty(str, len);
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorAdditionalProperties);
        }

        return true;
    }

    bool EndObject(Context& context, SizeType memberCount) const {
        RAPIDJSON_NORMALIZER_(EndObject, memberCount);
        if (hasRequired_) {
            context.error_handler.StartMissingProperties();
            for (SizeType index = 0; index < propertyCount_; index++)
                if (properties_[index].required && !context.propertyExist[index])
                    if (properties_[index].schema->defaultValueLength_ == 0 )
                        context.error_handler.AddMissingProperty(properties_[index].name);
            if (context.error_handler.EndMissingProperties())
                RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorRequired);
        }

        if (memberCount < minProperties_) {
            context.error_handler.TooFewProperties(memberCount, minProperties_);
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMinProperties);
        }

        if (memberCount > maxProperties_) {
            context.error_handler.TooManyProperties(memberCount, maxProperties_);
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMaxProperties);
        }

        if (hasDependencies_) {
            context.error_handler.StartDependencyErrors();
            for (SizeType sourceIndex = 0; sourceIndex < propertyCount_; sourceIndex++) {
                const Property& source = properties_[sourceIndex];
                if (context.propertyExist[sourceIndex]) {
                    if (source.dependencies) {
                        context.error_handler.StartMissingDependentProperties();
                        for (SizeType targetIndex = 0; targetIndex < propertyCount_; targetIndex++)
                            if (source.dependencies[targetIndex] && !context.propertyExist[targetIndex])
                                context.error_handler.AddMissingDependentProperty(properties_[targetIndex].name);
                        context.error_handler.EndMissingDependentProperties(source.name);
                    }
                    else if (source.dependenciesSchema) {
                        ISchemaValidator* dependenciesValidator = context.validators[source.dependenciesValidatorIndex];
                        if (!dependenciesValidator->IsValid())
                            context.error_handler.AddDependencySchemaError(source.name, dependenciesValidator);
                    }
                }
            }
            if (context.error_handler.EndDependencyErrors())
                RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorDependencies);
        }

        return true;
    }

    bool StartArray(Context& context) const {
        RAPIDJSON_NORMALIZER_NOARG_(StartArray);
        context.arrayElementIndex = 0;
        context.inArray = true;  // Ensure we note that we are in an array

        if (!(type_ & (1 << kArraySchemaType))) {
            DisallowedType(context, GetArrayString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }

        return CreateParallelValidator(context);
    }

    bool EndArray(Context& context, SizeType elementCount) const {
        RAPIDJSON_NORMALIZER_(EndArray, elementCount);
        context.inArray = false;

        if (elementCount < minItems_) {
            context.error_handler.TooFewItems(elementCount, minItems_);
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMinItems);
        }

        if (elementCount > maxItems_) {
            context.error_handler.TooManyItems(elementCount, maxItems_);
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMaxItems);
        }

        return true;
    }

#undef RAPIDJSON_NORMALIZER_
#undef RAPIDJSON_NORMALIZER_NOARG_
#undef RAPIDJSON_NORMALIZER_BASE_

    static const ValueType& GetValidateErrorKeyword(ValidateErrorCode validateErrorCode) {
        switch (validateErrorCode) {
            case kValidateErrorMultipleOf:              return GetMultipleOfString();
            case kValidateErrorMaximum:                 return GetMaximumString();
            case kValidateErrorExclusiveMaximum:        return GetMaximumString(); // Same
            case kValidateErrorMinimum:                 return GetMinimumString();
            case kValidateErrorExclusiveMinimum:        return GetMinimumString(); // Same

            case kValidateErrorMaxLength:               return GetMaxLengthString();
            case kValidateErrorMinLength:               return GetMinLengthString();
            case kValidateErrorPattern:                 return GetPatternString();

            case kValidateErrorMaxItems:                return GetMaxItemsString();
            case kValidateErrorMinItems:                return GetMinItemsString();
            case kValidateErrorUniqueItems:             return GetUniqueItemsString();
            case kValidateErrorAdditionalItems:         return GetAdditionalItemsString();

            case kValidateErrorMaxProperties:           return GetMaxPropertiesString();
            case kValidateErrorMinProperties:           return GetMinPropertiesString();
            case kValidateErrorRequired:                return GetRequiredString();
            case kValidateErrorAdditionalProperties:    return GetAdditionalPropertiesString();
            case kValidateErrorPatternProperties:       return GetPatternPropertiesString();
            case kValidateErrorDependencies:            return GetDependenciesString();

            case kValidateErrorEnum:                    return GetEnumString();
            case kValidateErrorType:                    return GetTypeString();

            case kValidateErrorOneOf:                   return GetOneOfString();
            case kValidateErrorOneOfMatch:              return GetOneOfString(); // Same
            case kValidateErrorAllOf:                   return GetAllOfString();
            case kValidateErrorAnyOf:                   return GetAnyOfString();
            case kValidateErrorNot:                     return GetNotString();

#ifdef RAPIDJSON_YGGDRASIL
	    case kValidateErrorRequiredSchema:          return GetRequiredString();
	    case kValidateErrorSubType:                 return GetSubTypeString();
	    case kValidateErrorPrecision:               return GetPrecisionString();
	    case kValdiateErrorUnits:                   return GetUnitsString();
	    case kValidateErrorShape:                   return GetShapeString();
	    case kValidateErrorPythonImport:            return GetPythonClassString();
	    case kValidateErrorInvalidSchema:           return GetSchemaString();
	    case kNormalizeErrorAliasDuplicate:         return GetAliasesString();
	    case kNormalizeErrorCircularAlias:          return GetAliasesString();
	    case kNormalizeErrorConflictingAliases:     return GetAliasesString();
#endif // RAPIDJSON_YGGDRASIL

            default:                                    return GetNullString();
        }
    }


    // Generate functions for string literal according to Ch
#define RAPIDJSON_STRING_(name, ...) \
    static const ValueType& Get##name##String() {\
        static const Ch s[] = { __VA_ARGS__, '\0' };\
        static const ValueType v(s, static_cast<SizeType>(sizeof(s) / sizeof(Ch) - 1));\
        return v;\
    }

    RAPIDJSON_STRING_(Null, 'n', 'u', 'l', 'l')
    RAPIDJSON_STRING_(Boolean, 'b', 'o', 'o', 'l', 'e', 'a', 'n')
    RAPIDJSON_STRING_(Object, 'o', 'b', 'j', 'e', 'c', 't')
    RAPIDJSON_STRING_(Array, 'a', 'r', 'r', 'a', 'y')
    RAPIDJSON_STRING_(String, 's', 't', 'r', 'i', 'n', 'g')
    RAPIDJSON_STRING_(Number, 'n', 'u', 'm', 'b', 'e', 'r')
    RAPIDJSON_STRING_(Integer, 'i', 'n', 't', 'e', 'g', 'e', 'r')
    RAPIDJSON_STRING_(Type, 't', 'y', 'p', 'e')
    RAPIDJSON_STRING_(Enum, 'e', 'n', 'u', 'm')
    RAPIDJSON_STRING_(AllOf, 'a', 'l', 'l', 'O', 'f')
    RAPIDJSON_STRING_(AnyOf, 'a', 'n', 'y', 'O', 'f')
    RAPIDJSON_STRING_(OneOf, 'o', 'n', 'e', 'O', 'f')
    RAPIDJSON_STRING_(Not, 'n', 'o', 't')
    RAPIDJSON_STRING_(Properties, 'p', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's')
    RAPIDJSON_STRING_(Required, 'r', 'e', 'q', 'u', 'i', 'r', 'e', 'd')
    RAPIDJSON_STRING_(Dependencies, 'd', 'e', 'p', 'e', 'n', 'd', 'e', 'n', 'c', 'i', 'e', 's')
    RAPIDJSON_STRING_(PatternProperties, 'p', 'a', 't', 't', 'e', 'r', 'n', 'P', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's')
    RAPIDJSON_STRING_(AdditionalProperties, 'a', 'd', 'd', 'i', 't', 'i', 'o', 'n', 'a', 'l', 'P', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's')
    RAPIDJSON_STRING_(MinProperties, 'm', 'i', 'n', 'P', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's')
    RAPIDJSON_STRING_(MaxProperties, 'm', 'a', 'x', 'P', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's')
    RAPIDJSON_STRING_(Items, 'i', 't', 'e', 'm', 's')
    RAPIDJSON_STRING_(MinItems, 'm', 'i', 'n', 'I', 't', 'e', 'm', 's')
    RAPIDJSON_STRING_(MaxItems, 'm', 'a', 'x', 'I', 't', 'e', 'm', 's')
    RAPIDJSON_STRING_(AdditionalItems, 'a', 'd', 'd', 'i', 't', 'i', 'o', 'n', 'a', 'l', 'I', 't', 'e', 'm', 's')
    RAPIDJSON_STRING_(UniqueItems, 'u', 'n', 'i', 'q', 'u', 'e', 'I', 't', 'e', 'm', 's')
    RAPIDJSON_STRING_(MinLength, 'm', 'i', 'n', 'L', 'e', 'n', 'g', 't', 'h')
    RAPIDJSON_STRING_(MaxLength, 'm', 'a', 'x', 'L', 'e', 'n', 'g', 't', 'h')
    RAPIDJSON_STRING_(Pattern, 'p', 'a', 't', 't', 'e', 'r', 'n')
    RAPIDJSON_STRING_(Minimum, 'm', 'i', 'n', 'i', 'm', 'u', 'm')
    RAPIDJSON_STRING_(Maximum, 'm', 'a', 'x', 'i', 'm', 'u', 'm')
    RAPIDJSON_STRING_(ExclusiveMinimum, 'e', 'x', 'c', 'l', 'u', 's', 'i', 'v', 'e', 'M', 'i', 'n', 'i', 'm', 'u', 'm')
    RAPIDJSON_STRING_(ExclusiveMaximum, 'e', 'x', 'c', 'l', 'u', 's', 'i', 'v', 'e', 'M', 'a', 'x', 'i', 'm', 'u', 'm')
    RAPIDJSON_STRING_(MultipleOf, 'm', 'u', 'l', 't', 'i', 'p', 'l', 'e', 'O', 'f')
    RAPIDJSON_STRING_(DefaultValue, 'd', 'e', 'f', 'a', 'u', 'l', 't')
    RAPIDJSON_STRING_(Ref, '$', 'r', 'e', 'f')
    RAPIDJSON_STRING_(Id, 'i', 'd')

    RAPIDJSON_STRING_(SchemeEnd, ':')
    RAPIDJSON_STRING_(AuthStart, '/', '/')
    RAPIDJSON_STRING_(QueryStart, '?')
    RAPIDJSON_STRING_(FragStart, '#')
    RAPIDJSON_STRING_(Slash, '/')
    RAPIDJSON_STRING_(Dot, '.')

#ifdef RAPIDJSON_YGGDRASIL
    // yggdrasil types and properties
    RAPIDJSON_STRING_(Scalar, 's', 'c', 'a', 'l', 'a', 'r')
    RAPIDJSON_STRING_(1DArray, '1', 'd', 'a', 'r', 'r', 'a', 'y')
    RAPIDJSON_STRING_(NDArray, 'n', 'd', 'a', 'r', 'r', 'a', 'y')
    RAPIDJSON_STRING_(PythonClass, 'c', 'l', 'a', 's', 's')
    RAPIDJSON_STRING_(PythonFunction, 'f', 'u', 'n', 'c', 't', 'i', 'o', 'n')
    RAPIDJSON_STRING_(PythonInstance, 'i', 'n', 's', 't', 'a', 'n', 'c', 'e')
    RAPIDJSON_STRING_(Obj, 'o', 'b', 'j')
    RAPIDJSON_STRING_(Ply, 'p', 'l', 'y')
    RAPIDJSON_STRING_(Any, 'a', 'n', 'y')
    RAPIDJSON_STRING_(Schema, 's', 'c', 'h', 'e', 'm', 'a')
    // props
    RAPIDJSON_STRING_(SubType, 's', 'u', 'b', 't', 'y', 'p', 'e')
    RAPIDJSON_STRING_(Precision, 'p', 'r', 'e', 'c', 'i', 's', 'i', 'o', 'n')
    RAPIDJSON_STRING_(Units, 'u', 'n', 'i', 't', 's')
    RAPIDJSON_STRING_(Length, 'l', 'e', 'n', 'g', 't', 'h')
    RAPIDJSON_STRING_(Shape, 's', 'h', 'a', 'p', 'e')
    RAPIDJSON_STRING_(Args, 'a', 'r', 'g', 's')
    RAPIDJSON_STRING_(Kwargs, 'k', 'w', 'a', 'r', 'g', 's')
    RAPIDJSON_STRING_(Aliases, 'a', 'l', 'i', 'a', 's', 'e', 's')
    // Subtypes
    RAPIDJSON_STRING_(StringSubType, 's', 't', 'r', 'i', 'n', 'g')
    RAPIDJSON_STRING_(IntSubType, 'i', 'n', 't')
    RAPIDJSON_STRING_(UintSubType, 'u', 'i', 'n', 't')
    RAPIDJSON_STRING_(FloatSubType, 'f', 'l', 'o', 'a', 't')
    RAPIDJSON_STRING_(ComplexSubType, 'c', 'o', 'm', 'p', 'l', 'e', 'x')
    // Normalization
    RAPIDJSON_STRING_(Normalization, 'n', 'o', 'r', 'm', 'a', 'l', 'i', 'z', 'a', 't', 'i', 'o', 'n')
#endif // RAPIDJSON_YGGDRASIL

#undef RAPIDJSON_STRING_

protected:
    enum SchemaValueType {
        kNullSchemaType,
        kBooleanSchemaType,
        kObjectSchemaType,
        kArraySchemaType,
        kStringSchemaType,
        kNumberSchemaType,
        kIntegerSchemaType,
        kTotalSchemaType
    };

#ifdef RAPIDJSON_YGGDRASIL
    enum YggSchemaValueType {
        kYggNullSchemaType,
    	kYggScalarSchemaType,
    	kYggNDArraySchemaType,
    	kYggPythonImportSchemaType,
    	kYggPythonInstanceSchemaType,
    	kYggObjSchemaType,
    	kYggPlySchemaType,
	kYggSchemaSchemaType,
    	kYggTotalSchemaType
    };
    enum YggSchemaValueSubType {
    	kYggNullSubType,
        kYggIntSchemaSubType,
    	kYggUintSchemaSubType,
    	kYggFloatSchemaSubType,
    	kYggComplexSchemaSubType,
    	kYggStringSchemaSubType,
    	kYggTotalSchemaSubType
    };
#endif // RAPIDJSON_YGGDRASIL

#if RAPIDJSON_SCHEMA_USE_INTERNALREGEX
        typedef internal::GenericRegex<EncodingType, AllocatorType> RegexType;
#elif RAPIDJSON_SCHEMA_USE_STDREGEX
        typedef std::basic_regex<Ch> RegexType;
#else
        typedef char RegexType;
#endif

    struct SchemaArray {
        SchemaArray() : schemas(), count() {}
        ~SchemaArray() { AllocatorType::Free(schemas); }
        const SchemaType** schemas;
        SizeType begin; // begin index of context.validators
        SizeType count;
    };

    template <typename V1, typename V2>
    void AddUniqueElement(V1& a, const V2& v) {
        for (typename V1::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr)
            if (*itr == v)
                return;
        V1 c(v, *allocator_);
        a.PushBack(c, *allocator_);
    }

    static const ValueType* GetMember(const ValueType& value, const ValueType& name) {
        typename ValueType::ConstMemberIterator itr = value.FindMember(name);
        return itr != value.MemberEnd() ? &(itr->value) : 0;
    }

    static void AssignIfExist(bool& out, const ValueType& value, const ValueType& name) {
        if (const ValueType* v = GetMember(value, name))
            if (v->IsBool())
                out = v->GetBool();
    }

    static void AssignIfExist(SizeType& out, const ValueType& value, const ValueType& name) {
        if (const ValueType* v = GetMember(value, name))
            if (v->IsUint64() && v->GetUint64() <= SizeType(~0))
                out = static_cast<SizeType>(v->GetUint64());
    }

    void AssignIfExist(SchemaArray& out, SchemaDocumentType& schemaDocument, const PointerType& p, const ValueType& value, const ValueType& name, const ValueType& document) {
        if (const ValueType* v = GetMember(value, name)) {
            if (v->IsArray() && v->Size() > 0) {
                PointerType q = p.Append(name, allocator_);
                out.count = v->Size();
                out.schemas = static_cast<const Schema**>(allocator_->Malloc(out.count * sizeof(const Schema*)));
                memset(out.schemas, 0, sizeof(Schema*)* out.count);
                for (SizeType i = 0; i < out.count; i++)
                    schemaDocument.CreateSchema(&out.schemas[i], q.Append(i, allocator_), (*v)[i], document, id_);
                out.begin = validatorCount_;
                validatorCount_ += out.count;
            }
        }
    }

#if RAPIDJSON_SCHEMA_USE_INTERNALREGEX
    template <typename ValueType>
    RegexType* CreatePattern(const ValueType& value) {
        if (value.IsString()) {
            RegexType* r = new (allocator_->Malloc(sizeof(RegexType))) RegexType(value.GetString(), allocator_);
            if (!r->IsValid()) {
                r->~RegexType();
                AllocatorType::Free(r);
                r = 0;
            }
            return r;
        }
        return 0;
    }

    static bool IsPatternMatch(const RegexType* pattern, const Ch *str, SizeType) {
        GenericRegexSearch<RegexType> rs(*pattern);
        return rs.Search(str);
    }
#elif RAPIDJSON_SCHEMA_USE_STDREGEX
    template <typename ValueType>
    RegexType* CreatePattern(const ValueType& value) {
        if (value.IsString()) {
            RegexType *r = static_cast<RegexType*>(allocator_->Malloc(sizeof(RegexType)));
            try {
                return new (r) RegexType(value.GetString(), std::size_t(value.GetStringLength()), std::regex_constants::ECMAScript);
            }
            catch (const std::regex_error&) {
                AllocatorType::Free(r);
            }
        }
        return 0;
    }

    static bool IsPatternMatch(const RegexType* pattern, const Ch *str, SizeType length) {
        std::match_results<const Ch*> r;
        return std::regex_search(str, str + length, r, *pattern);
    }
#else
    template <typename ValueType>
    RegexType* CreatePattern(const ValueType&) { return 0; }

    static bool IsPatternMatch(const RegexType*, const Ch *, SizeType) { return true; }
#endif // RAPIDJSON_SCHEMA_USE_STDREGEX

    void AddType(const ValueType& type) {
        if      (type == GetNullString()   ) type_ |= 1 << kNullSchemaType;
        else if (type == GetBooleanString()) type_ |= 1 << kBooleanSchemaType;
        else if (type == GetObjectString() ) type_ |= 1 << kObjectSchemaType;
        else if (type == GetArrayString()  ) type_ |= 1 << kArraySchemaType;
        else if (type == GetStringString() ) type_ |= 1 << kStringSchemaType;
        else if (type == GetIntegerString()) type_ |= 1 << kIntegerSchemaType;
        else if (type == GetNumberString() ) type_ |= (1 << kNumberSchemaType) | (1 << kIntegerSchemaType);
#ifdef RAPIDJSON_YGGDRASIL
	// yggdrasil types
	else if (type == GetScalarString() ) yggtype_ |= 1 << kYggScalarSchemaType;
	else if (type == Get1DArrayString() ) yggtype_ |= 1 << kYggNDArraySchemaType;
	else if (type == GetNDArrayString() ) yggtype_ |= 1 << kYggNDArraySchemaType;
	else if (type == GetPythonClassString() ) {
	  type_ |= 1 << kStringSchemaType;
	  yggtype_ |= 1 << kYggPythonImportSchemaType;
	}
	else if (type == GetPythonFunctionString() ) {
	  type_ |= 1 << kStringSchemaType;
	  yggtype_ |= 1 << kYggPythonImportSchemaType;
	}
	else if (type == GetPythonInstanceString() ) {
	  type_ |= 1 << kObjectSchemaType;
	  yggtype_ |= 1 << kYggPythonInstanceSchemaType;
	}
	else if (type == GetObjString() ) yggtype_ |= 1 << kYggObjSchemaType;
	else if (type == GetPlyString() ) yggtype_ |= 1 << kYggPlySchemaType;
	else if (type == GetSchemaString() ) {
	  type_ |= 1 << kObjectSchemaType;
	  yggtype_ |= 1 << kYggSchemaSchemaType;
	}
	else if (type == GetAnyString() ) yggtype_ |= ((1 << kYggTotalSchemaType) - 1);
#endif // RAPIDJSON_YGGDRASIL
    }

#ifdef RAPIDJSON_YGGDRASIL
  YggSchemaValueSubType GetSubType(const ValueType& subtype) const {
    if      (subtype == GetIntSubTypeString()    ) return kYggIntSchemaSubType;
    else if (subtype == GetUintSubTypeString()   ) return kYggUintSchemaSubType;
    else if (subtype == GetFloatSubTypeString()  ) return kYggFloatSchemaSubType;
    else if (subtype == GetComplexSubTypeString()) return kYggComplexSchemaSubType;
    else if (subtype == GetStringSubTypeString() ) return kYggStringSchemaSubType;
    return kYggNullSubType;
  }
  const ValueType& SubType2String(const YggSchemaValueSubType subtype) const {
    switch (subtype) {
    case (kYggIntSchemaSubType): return GetIntSubTypeString();
    case (kYggUintSchemaSubType): return GetUintSubTypeString();
    case (kYggFloatSchemaSubType): return GetFloatSubTypeString();
    case (kYggComplexSchemaSubType): return GetComplexSubTypeString();
    case (kYggStringSchemaSubType): return GetStringSubTypeString();
    default:
      const ValueType& out = GetNullString();
      RAPIDJSON_ASSERT(out != GetNullString());
      return out;
    }
  }
  void AddSubType(const ValueType& subtype) { subtype_ = GetSubType(subtype); }
#endif // RAPIDJSON_YGGDRASIL

    bool CreateParallelValidator(Context& context) const {
        if (enum_ || context.arrayUniqueness)
            context.hasher = context.factory.CreateHasher();

        if (validatorCount_) {
            RAPIDJSON_ASSERT(context.validators == 0);
            context.validators = static_cast<ISchemaValidator**>(context.factory.MallocState(sizeof(ISchemaValidator*) * validatorCount_));
            context.validatorCount = validatorCount_;

            // Always return after first failure for these sub-validators
            if (allOf_.schemas)
                CreateSchemaValidators(context, allOf_, false);

            if (anyOf_.schemas)
                CreateSchemaValidators(context, anyOf_, false);

            if (oneOf_.schemas)
                CreateSchemaValidators(context, oneOf_, false);

            if (not_)
                context.validators[notValidatorIndex_] = context.factory.CreateSchemaValidator(*not_, false);

            if (hasSchemaDependencies_) {
                for (SizeType i = 0; i < propertyCount_; i++)
                    if (properties_[i].dependenciesSchema)
                        context.validators[properties_[i].dependenciesValidatorIndex] = context.factory.CreateSchemaValidator(*properties_[i].dependenciesSchema, false);
            }
	    
#ifdef RAPIDJSON_YGGDRASIL
	    if (metaschema_) {
	      context.validators[metaschemaValidatorIndex_] = context.factory.CreateSchemaValidator(*metaschema_, false);
	    }
#endif // RAPIDJSON_YGGDRASIL
	    
        }

        return true;
    }

    void CreateSchemaValidators(Context& context, const SchemaArray& schemas, const bool inheritContinueOnErrors) const {
        for (SizeType i = 0; i < schemas.count; i++)
            context.validators[schemas.begin + i] = context.factory.CreateSchemaValidator(*schemas.schemas[i], inheritContinueOnErrors);
    }

    // O(n)
    bool FindPropertyIndex(const ValueType& name, SizeType* outIndex) const {
        SizeType len = name.GetStringLength();
        const Ch* str = name.GetString();
#ifdef RAPIDJSON_YGGDRASIL
	if (child_aliases_.HasMember(str)) {
	  SValue orig(str, len, *allocator_);
	  SValue dest;
	  if (follow_aliases_(child_aliases_, orig, &dest, *allocator_)) {
	    str = dest.GetString();
	    len = dest.GetStringLength();
	  }
	}
#endif // RAPIDJSON_YGGDRASIL
        for (SizeType index = 0; index < propertyCount_; index++)
            if (properties_[index].name.GetStringLength() == len &&
                (std::memcmp(properties_[index].name.GetString(), str, sizeof(Ch) * len) == 0))
            {
                *outIndex = index;
                return true;
            }
        return false;
    }

    bool CheckInt(Context& context, int64_t i) const {
#ifdef RAPIDJSON_YGGDRASIL
      if ((yggtype_ & (1 << kYggScalarSchemaType))) {
	if (i >= 0) {
	  if (!(CheckScalar(context, GetUintSubTypeString(), ValueType(8), ValueType())))
	    return false;
	} else {
	  if (!(CheckScalar(context, GetIntSubTypeString(), ValueType(8), ValueType())))
	    return false;
	}
      }
#endif // RAPIDJSON_YGGDRASIL
        if (!(type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType)))) {
            DisallowedType(context, GetIntegerString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }

        if (!minimum_.IsNull()) {
            if (minimum_.IsInt64()) {
                if (exclusiveMinimum_ ? i <= minimum_.GetInt64() : i < minimum_.GetInt64()) {
                    context.error_handler.BelowMinimum(i, minimum_, exclusiveMinimum_);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(exclusiveMinimum_ ? kValidateErrorExclusiveMinimum : kValidateErrorMinimum);
                }
            }
            else if (minimum_.IsUint64()) {
                context.error_handler.BelowMinimum(i, minimum_, exclusiveMinimum_);
                RAPIDJSON_INVALID_KEYWORD_RETURN(exclusiveMinimum_ ? kValidateErrorExclusiveMinimum : kValidateErrorMinimum); // i <= max(int64_t) < minimum.GetUint64()
            }
            else if (!CheckDoubleMinimum(context, static_cast<double>(i)))
                return false;
        }

        if (!maximum_.IsNull()) {
            if (maximum_.IsInt64()) {
                if (exclusiveMaximum_ ? i >= maximum_.GetInt64() : i > maximum_.GetInt64()) {
                    context.error_handler.AboveMaximum(i, maximum_, exclusiveMaximum_);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(exclusiveMaximum_ ? kValidateErrorExclusiveMaximum : kValidateErrorMaximum);
                }
            }
            else if (maximum_.IsUint64()) { }
                /* do nothing */ // i <= max(int64_t) < maximum_.GetUint64()
            else if (!CheckDoubleMaximum(context, static_cast<double>(i)))
                return false;
        }

        if (!multipleOf_.IsNull()) {
            if (multipleOf_.IsUint64()) {
                if (static_cast<uint64_t>(i >= 0 ? i : -i) % multipleOf_.GetUint64() != 0) {
                    context.error_handler.NotMultipleOf(i, multipleOf_);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMultipleOf);
                }
            }
            else if (!CheckDoubleMultipleOf(context, static_cast<double>(i)))
                return false;
        }

        return true;
    }

    bool CheckUint(Context& context, uint64_t i) const {
#ifdef RAPIDJSON_YGGDRASIL
      if ((yggtype_ & (1 << kYggScalarSchemaType))) {
	if (!(CheckScalar(context, GetUintSubTypeString(), ValueType(8), ValueType())))
	  return false;
      }
#endif // RAPIDJSON_YGGDRASIL
        if (!(type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType)))) {
            DisallowedType(context, GetIntegerString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }

        if (!minimum_.IsNull()) {
            if (minimum_.IsUint64()) {
                if (exclusiveMinimum_ ? i <= minimum_.GetUint64() : i < minimum_.GetUint64()) {
                    context.error_handler.BelowMinimum(i, minimum_, exclusiveMinimum_);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(exclusiveMinimum_ ? kValidateErrorExclusiveMinimum : kValidateErrorMinimum);
                }
            }
            else if (minimum_.IsInt64())
                /* do nothing */; // i >= 0 > minimum.Getint64()
            else if (!CheckDoubleMinimum(context, static_cast<double>(i)))
                return false;
        }

        if (!maximum_.IsNull()) {
            if (maximum_.IsUint64()) {
                if (exclusiveMaximum_ ? i >= maximum_.GetUint64() : i > maximum_.GetUint64()) {
                    context.error_handler.AboveMaximum(i, maximum_, exclusiveMaximum_);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(exclusiveMaximum_ ? kValidateErrorExclusiveMaximum : kValidateErrorMaximum);
                }
            }
            else if (maximum_.IsInt64()) {
                context.error_handler.AboveMaximum(i, maximum_, exclusiveMaximum_);
                RAPIDJSON_INVALID_KEYWORD_RETURN(exclusiveMaximum_ ? kValidateErrorExclusiveMaximum : kValidateErrorMaximum); // i >= 0 > maximum_
            }
            else if (!CheckDoubleMaximum(context, static_cast<double>(i)))
                return false;
        }

        if (!multipleOf_.IsNull()) {
            if (multipleOf_.IsUint64()) {
                if (i % multipleOf_.GetUint64() != 0) {
                    context.error_handler.NotMultipleOf(i, multipleOf_);
                    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMultipleOf);
                }
            }
            else if (!CheckDoubleMultipleOf(context, static_cast<double>(i)))
                return false;
        }

        return true;
    }

    bool CheckDoubleMinimum(Context& context, double d) const {
        if (exclusiveMinimum_ ? d <= minimum_.GetDouble() : d < minimum_.GetDouble()) {
            context.error_handler.BelowMinimum(d, minimum_, exclusiveMinimum_);
            RAPIDJSON_INVALID_KEYWORD_RETURN(exclusiveMinimum_ ? kValidateErrorExclusiveMinimum : kValidateErrorMinimum);
        }
        return true;
    }

    bool CheckDoubleMaximum(Context& context, double d) const {
        if (exclusiveMaximum_ ? d >= maximum_.GetDouble() : d > maximum_.GetDouble()) {
            context.error_handler.AboveMaximum(d, maximum_, exclusiveMaximum_);
            RAPIDJSON_INVALID_KEYWORD_RETURN(exclusiveMaximum_ ? kValidateErrorExclusiveMaximum : kValidateErrorMaximum);
        }
        return true;
    }

    bool CheckDoubleMultipleOf(Context& context, double d) const {
        double a = std::abs(d), b = std::abs(multipleOf_.GetDouble());
        double q = std::floor(a / b);
        double r = a - q * b;
        if (r > 0.0) {
            context.error_handler.NotMultipleOf(d, multipleOf_);
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorMultipleOf);
        }
        return true;
    }

    void DisallowedType(Context& context, const ValueType& actualType) const {
        ErrorHandler& eh = context.error_handler;
        eh.StartDisallowedType();

        if (type_ & (1 << kNullSchemaType)) eh.AddExpectedType(GetNullString());
        if (type_ & (1 << kBooleanSchemaType)) eh.AddExpectedType(GetBooleanString());
        if (type_ & (1 << kObjectSchemaType)) eh.AddExpectedType(GetObjectString());
        if (type_ & (1 << kArraySchemaType)) eh.AddExpectedType(GetArrayString());
        if (type_ & (1 << kStringSchemaType)) eh.AddExpectedType(GetStringString());

        if (type_ & (1 << kNumberSchemaType)) eh.AddExpectedType(GetNumberString());
        else if (type_ & (1 << kIntegerSchemaType)) eh.AddExpectedType(GetIntegerString());
#ifdef RAPIDJSON_YGGDRASIL
	if (yggtype_ & (1 << kYggScalarSchemaType)) eh.AddExpectedType(GetScalarString());
	if (yggtype_ & (1 << kYggNDArraySchemaType)) eh.AddExpectedType(GetNDArrayString());
	if (yggtype_ & (1 << kYggPythonImportSchemaType)) eh.AddExpectedType(GetPythonClassString());
	if (yggtype_ & (1 << kYggPythonInstanceSchemaType)) eh.AddExpectedType(GetPythonInstanceString());
	if (yggtype_ & (1 << kYggObjSchemaType)) eh.AddExpectedType(GetObjString());
	if (yggtype_ & (1 << kYggPlySchemaType)) eh.AddExpectedType(GetPlyString());
	if (yggtype_ & (1 << kYggSchemaSchemaType)) eh.AddExpectedType(GetSchemaString());
	if (yggtype_ == ((1 << kYggTotalSchemaType) - 1))
	  eh.AddExpectedType(GetAnyString());
#endif // RAPIDJSON_YGGDRASIL
        eh.EndDisallowedType(actualType);
    }

#ifdef RAPIDJSON_YGGDRASIL
  template <typename YggSchemaValueType>
  bool CheckRequiredSchemaProperty(Context& context, const YggSchemaValueType& schema, const ValueType& name) const {
    if (!schema.HasMember(name)) {
      context.error_handler.MissingRequiredSchemaProperty(name);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorRequiredSchema);
    }
    return true;
  }
  bool CheckScalar(Context& context, const ValueType& subtype_str,
		   const ValueType& precision, const ValueType& units) const {
    if (!(CheckSubType(context, &subtype_str, true)))
      return false;
    if (!(CheckPrecision(context, &precision, true)))
      return false;
    if (!(CheckUnits(context, &units, true)))
      return false;
    return true;
  }
  bool CheckSubType(Context& context, const ValueType* subtype_str, const bool&) const {
    if (subtype_ == kYggNullSubType)
      return true;
    YggSchemaValueSubType subtype_code = GetSubType(*subtype_str);
    if ((subtype_ != subtype_code) && (!((subtype_ == kYggIntSchemaSubType) && (subtype_code == kYggUintSchemaSubType)))) {
      context.error_handler.IncorrectSubType(*subtype_str, SubType2String(subtype_));
      RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorSubType);
    }
    return true;
  }
  template <typename YggSchemaValueType>
  bool CheckSubType(Context& context, const YggSchemaValueType& schema) const {
    if (!CheckRequiredSchemaProperty(context, schema, GetSubTypeString()))
      return false;
    typename YggSchemaValueType::ConstMemberIterator vs = schema.FindMember(GetSubTypeString());
    ValueType actual(vs->value.GetString(),
		     vs->value.GetStringLength());
    return CheckSubType(context, &actual, true);
  }
  bool CheckPrecision(Context& context, const ValueType* actual, const bool&) const {
    if (precision_.IsNull())
      return true;
    if (precision_.GetUint() < actual->GetUint()) {
      context.error_handler.IncorrectPrecision(*actual, precision_);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorPrecision);
    }
    return true;
  }
  template <typename YggSchemaValueType>
  bool CheckPrecision(Context& context, const YggSchemaValueType& schema) const {
    if (!CheckRequiredSchemaProperty(context, schema, GetPrecisionString()))
      return false;
    typename YggSchemaValueType::ConstMemberIterator vs = schema.FindMember(GetPrecisionString());
    ValueType actual(vs->value.GetUint());
    return CheckPrecision(context, &actual, true);
  }
  bool CheckUnits(Context& context, const ValueType* actual, const bool&) const {
    if (units_.IsNull())
      return true;
    units::Units<EncodingType> expected_units(units_.GetString(),
					      units_.GetStringLength(),
					      false);
    units::Units<EncodingType> actual_units(actual->GetString(),
					    actual->GetStringLength(),
					    false);
    if (!(actual_units.is_compatible(expected_units))) {
      context.error_handler.IncorrectUnits(*actual, units_);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kValdiateErrorUnits);
    }
    return true;
  }
  template <typename YggSchemaValueType>
  bool CheckUnits(Context& context, const YggSchemaValueType& schema) const {
    if (!CheckRequiredSchemaProperty(context, schema, GetUnitsString()))
      return false;
    typename YggSchemaValueType::ConstMemberIterator vs = schema.FindMember(GetUnitsString());
    ValueType actual(vs->value.GetString(), vs->value.GetStringLength());
    return CheckUnits(context, &actual, true);
  }
  template <typename YggSchemaValueType>
  bool CheckShape(Context& context, const YggSchemaValueType& schema) const {
    if (!CheckRequiredSchemaProperty(context, schema, GetShapeString()))
      return false;
    if (shape_.IsNull())
      return true;
    SValue actual(kArrayType);
    typename YggSchemaValueType::ConstMemberIterator vs = schema.FindMember(GetShapeString());
    for (auto v = vs->value.Begin(); v != vs->value.End(); ++v)
      actual.PushBack(static_cast<SizeType>(v->GetUint()), *allocator_);
    if (!shape_.IsNull() && (shape_ != actual)) {
      context.error_handler.IncorrectShape(actual, shape_);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorShape);
    }
    return true;
  }
  bool CheckPythonImport(Context& context, const Ch* str, SizeType length) const {
    PyObject* pyobj = import_python_object(reinterpret_cast<const char*>(str),
					   "CheckPythonImport", true);
    if (!(pyobj)) {
      context.error_handler.InvalidPythonImport(str, length);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorPythonImport);
    }
    Py_DECREF(pyobj);
    return true;
  }
  // template <typename ObjectType>
  bool CheckSchema(Context& context, const typename ValueType::ConstObject& o) const;

#endif // RAPIDJSON_YGGDRASIL

    struct Property {
        Property() : schema(), dependenciesSchema(), dependenciesValidatorIndex(), dependencies(), required(false) {}
        ~Property() { AllocatorType::Free(dependencies); }
        SValue name;
        const SchemaType* schema;
        const SchemaType* dependenciesSchema;
        SizeType dependenciesValidatorIndex;
        bool* dependencies;
        bool required;
    };

    struct PatternProperty {
        PatternProperty() : schema(), pattern() {}
        ~PatternProperty() {
            if (pattern) {
                pattern->~RegexType();
                AllocatorType::Free(pattern);
            }
        }
        const SchemaType* schema;
        RegexType* pattern;
    };

    AllocatorType* allocator_;
    SValue uri_;
    UriType id_;
    PointerType pointer_;
    const SchemaType* typeless_;
    uint64_t* enum_;
    SizeType enumCount_;
    SchemaArray allOf_;
    SchemaArray anyOf_;
    SchemaArray oneOf_;
    const SchemaType* not_;
    unsigned type_; // bitmask of kSchemaType
    SizeType validatorCount_;
    SizeType notValidatorIndex_;

    Property* properties_;
    const SchemaType* additionalPropertiesSchema_;
    PatternProperty* patternProperties_;
    SizeType patternPropertyCount_;
    SizeType propertyCount_;
    SizeType minProperties_;
    SizeType maxProperties_;
    bool additionalProperties_;
    bool hasDependencies_;
    bool hasRequired_;
    bool hasSchemaDependencies_;

    const SchemaType* additionalItemsSchema_;
    const SchemaType* itemsList_;
    const SchemaType** itemsTuple_;
    SizeType itemsTupleCount_;
    SizeType minItems_;
    SizeType maxItems_;
    bool additionalItems_;
    bool uniqueItems_;

    RegexType* pattern_;
    SizeType minLength_;
    SizeType maxLength_;

    SValue minimum_;
    SValue maximum_;
    SValue multipleOf_;
    bool exclusiveMinimum_;
    bool exclusiveMaximum_;

    SizeType defaultValueLength_;

#ifdef RAPIDJSON_YGGDRASIL
    // Yggdrasil properties
    unsigned int yggtype_; // bitmask of kSchemaYggType
    YggSchemaValueSubType subtype_;
    SValue precision_;
    SValue units_;
    SValue shape_;
    SValue args_;
    SValue kwargs_;
    bool isMetaschema_;
    const SchemaType* metaschema_;
    SizeType metaschemaValidatorIndex_;
    SValue default_;
    SValue aliases_;
    SValue child_aliases_;
    bool hasAliases_;
#endif // RAPIDJSON_YGGDRASIL
  
};

template<typename Stack, typename Ch>
struct TokenHelper {
    RAPIDJSON_FORCEINLINE static void AppendIndexToken(Stack& documentStack, SizeType index) {
        *documentStack.template Push<Ch>() = '/';
        char buffer[21];
        size_t length = static_cast<size_t>((sizeof(SizeType) == 4 ? u32toa(index, buffer) : u64toa(index, buffer)) - buffer);
        for (size_t i = 0; i < length; i++)
            *documentStack.template Push<Ch>() = static_cast<Ch>(buffer[i]);
    }
};

// Partial specialized version for char to prevent buffer copying.
template <typename Stack>
struct TokenHelper<Stack, char> {
    RAPIDJSON_FORCEINLINE static void AppendIndexToken(Stack& documentStack, SizeType index) {
        if (sizeof(SizeType) == 4) {
            char *buffer = documentStack.template Push<char>(1 + 10); // '/' + uint
            *buffer++ = '/';
            const char* end = internal::u32toa(index, buffer);
             documentStack.template Pop<char>(static_cast<size_t>(10 - (end - buffer)));
        }
        else {
            char *buffer = documentStack.template Push<char>(1 + 20); // '/' + uint64
            *buffer++ = '/';
            const char* end = internal::u64toa(index, buffer);
            documentStack.template Pop<char>(static_cast<size_t>(20 - (end - buffer)));
        }
    }
};

} // namespace internal

///////////////////////////////////////////////////////////////////////////////
// IGenericRemoteSchemaDocumentProvider

template <typename SchemaDocumentType>
class IGenericRemoteSchemaDocumentProvider {
public:
    typedef typename SchemaDocumentType::Ch Ch;
    typedef typename SchemaDocumentType::ValueType ValueType;
    typedef typename SchemaDocumentType::AllocatorType AllocatorType;

    virtual ~IGenericRemoteSchemaDocumentProvider() {}
    virtual const SchemaDocumentType* GetRemoteDocument(const Ch* uri, SizeType length) = 0;
    virtual const SchemaDocumentType* GetRemoteDocument(GenericUri<ValueType, AllocatorType> uri) { return GetRemoteDocument(uri.GetBaseString(), uri.GetBaseStringLength()); }
};

///////////////////////////////////////////////////////////////////////////////
// GenericSchemaDocument

//! JSON schema document.
/*!
    A JSON schema document is a compiled version of a JSON schema.
    It is basically a tree of internal::Schema.

    \note This is an immutable class (i.e. its instance cannot be modified after construction).
    \tparam ValueT Type of JSON value (e.g. \c Value ), which also determine the encoding.
    \tparam Allocator Allocator type for allocating memory of this document.
*/
template <typename ValueT, typename Allocator = CrtAllocator>
class GenericSchemaDocument {
public:
    typedef ValueT ValueType;
    typedef IGenericRemoteSchemaDocumentProvider<GenericSchemaDocument> IRemoteSchemaDocumentProviderType;
    typedef Allocator AllocatorType;
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;
    typedef internal::Schema<GenericSchemaDocument> SchemaType;
    typedef GenericPointer<ValueType, Allocator> PointerType;
    typedef GenericValue<EncodingType, AllocatorType> SValue;
    typedef GenericUri<ValueType, Allocator> UriType;
    friend class internal::Schema<GenericSchemaDocument>;
    template <typename, typename, typename>
    friend class GenericSchemaValidator;

    //! Constructor.
    /*!
        Compile a JSON document into schema document.

        \param document A JSON document as source.
        \param uri The base URI of this schema document for purposes of violation reporting.
        \param uriLength Length of \c name, in code points.
        \param remoteProvider An optional remote schema document provider for resolving remote reference. Can be null.
        \param allocator An optional allocator instance for allocating memory. Can be null.
        \param pointer An optional JSON pointer to the start of the schema document
    */
    explicit GenericSchemaDocument(const ValueType& document, const Ch* uri = 0, SizeType uriLength = 0,
        IRemoteSchemaDocumentProviderType* remoteProvider = 0, Allocator* allocator = 0,
        const PointerType& pointer = PointerType()
#ifdef RAPIDJSON_YGGDRASIL
				   , bool isMetaschema = false
#endif // RAPIDJSON_YGGDRASIL
				   ) :  // PR #1393
        remoteProvider_(remoteProvider),
        allocator_(allocator),
        ownAllocator_(),
        root_(),
        typeless_(),
        schemaMap_(allocator, kInitialSchemaMapSize),
        schemaRef_(allocator, kInitialSchemaRefSize)
#ifdef RAPIDJSON_YGGDRASIL
	, metaschema_(), isMetaschema_(isMetaschema)
#endif // RAPIDJSON_YGGDRASIL
    {
        if (!allocator_)
            ownAllocator_ = allocator_ = RAPIDJSON_NEW(Allocator)();

        Ch noUri[1] = {0};
        uri_.SetString(uri ? uri : noUri, uriLength, *allocator_);
        docId_ = UriType(uri_, allocator_);

        typeless_ = static_cast<SchemaType*>(allocator_->Malloc(sizeof(SchemaType)));
        new (typeless_) SchemaType(this, PointerType(), ValueType(kObjectType).Move(), ValueType(kObjectType).Move(), allocator_, docId_);

        // Generate root schema, it will call CreateSchema() to create sub-schemas,
        // And call HandleRefSchema() if there are $ref.
        // PR #1393 use input pointer if supplied
        root_ = typeless_;
        if (pointer.GetTokenCount() == 0) {
            CreateSchemaRecursive(&root_, pointer, document, document, docId_);
        }
        else if (const ValueType* v = pointer.Get(document)) {
            CreateSchema(&root_, pointer, *v, document, docId_);
        }

        RAPIDJSON_ASSERT(root_ != 0);

        schemaRef_.ShrinkToFit(); // Deallocate all memory for ref
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move constructor in C++11
    GenericSchemaDocument(GenericSchemaDocument&& rhs) RAPIDJSON_NOEXCEPT :
        remoteProvider_(rhs.remoteProvider_),
        allocator_(rhs.allocator_),
        ownAllocator_(rhs.ownAllocator_),
        root_(rhs.root_),
        typeless_(rhs.typeless_),
        schemaMap_(std::move(rhs.schemaMap_)),
        schemaRef_(std::move(rhs.schemaRef_)),
        uri_(std::move(rhs.uri_)),
        docId_(rhs.docId_)
#ifdef RAPIDJSON_YGGDRASIL
	, metaschema_(rhs.metaschema_), isMetaschema_(rhs.isMetaschema_)
#endif // RAPIDJSON_YGGDRASIL
    {
        rhs.remoteProvider_ = 0;
        rhs.allocator_ = 0;
        rhs.ownAllocator_ = 0;
        rhs.typeless_ = 0;
    }
#endif

    //! Destructor
    ~GenericSchemaDocument() {
        while (!schemaMap_.Empty())
            schemaMap_.template Pop<SchemaEntry>(1)->~SchemaEntry();

        if (typeless_) {
            typeless_->~SchemaType();
            Allocator::Free(typeless_);
        }

        RAPIDJSON_DELETE(ownAllocator_);

#ifdef RAPIDJSON_YGGDRASIL
	if ((metaschema_) && (!isMetaschema_))
	  delete metaschema_;
#endif // RAPIDJSON_YGGDRASIL
    }

    const SValue& GetURI() const { return uri_; }

    //! Get the root schema.
    const SchemaType& GetRoot() const { return *root_; }

private:
    //! Prohibit copying
    GenericSchemaDocument(const GenericSchemaDocument&);
    //! Prohibit assignment
    GenericSchemaDocument& operator=(const GenericSchemaDocument&);

    typedef const PointerType* SchemaRefPtr; // PR #1393

    struct SchemaEntry {
        SchemaEntry(const PointerType& p, SchemaType* s, bool o, Allocator* allocator) : pointer(p, allocator), schema(s), owned(o) {}
        ~SchemaEntry() {
            if (owned) {
                schema->~SchemaType();
                Allocator::Free(schema);
            }
        }
        PointerType pointer;
        SchemaType* schema;
        bool owned;
    };

    // Changed by PR #1393
    void CreateSchemaRecursive(const SchemaType** schema, const PointerType& pointer, const ValueType& v, const ValueType& document, const UriType& id) {
        if (v.GetType() == kObjectType) {
            UriType newid = UriType(CreateSchema(schema, pointer, v, document, id), allocator_);

            for (typename ValueType::ConstMemberIterator itr = v.MemberBegin(); itr != v.MemberEnd(); ++itr)
                CreateSchemaRecursive(0, pointer.Append(itr->name, allocator_), itr->value, document, newid);
        }
        else if (v.GetType() == kArrayType)
            for (SizeType i = 0; i < v.Size(); i++)
                CreateSchemaRecursive(0, pointer.Append(i, allocator_), v[i], document, id);
    }

    // Changed by PR #1393
    const UriType& CreateSchema(const SchemaType** schema, const PointerType& pointer, const ValueType& v, const ValueType& document, const UriType& id) {
        RAPIDJSON_ASSERT(pointer.IsValid());
        if (v.IsObject()) {
            if (const SchemaType* sc = GetSchema(pointer)) {
                if (schema)
                    *schema = sc;
                AddSchemaRefs(const_cast<SchemaType*>(sc));
            }
            else if (!HandleRefSchema(pointer, schema, v, document, id)) {
                // The new schema constructor adds itself and its $ref(s) to schemaMap_
	        SchemaType* s = new (allocator_->Malloc(sizeof(SchemaType))) SchemaType(this, pointer, v, document, allocator_, id
#ifdef RAPIDJSON_YGGDRASIL
		, isMetaschema_
#endif // RAPIDJSON_YGGDRASIL
		);
                if (schema)
                    *schema = s;
                return s->GetId();
            }
        }
        else {
            if (schema)
                *schema = typeless_;
            AddSchemaRefs(typeless_);
        }
        return id;
    }

#ifdef RAPIDJSON_YGGDRASIL
  const UriType& CreateMetaSchema(const SchemaType** schema, const bool isInstance) {
      if (!metaschema_) {
	if (isMetaschema_) {
	  metaschema_ = this;
	} else {
	  GenericDocument<EncodingType, typename ValueType::AllocatorType> d;
	  d.Parse(get_metaschema<Ch>());
	  if (isInstance) {
	    ValueType v;
	    v.SetArray();
	    v.PushBack(ValueType(v.GetPythonClassString(), d.GetAllocator()), d.GetAllocator());
	    d.AddMember(SchemaType::GetRequiredString(), v, d.GetAllocator());
	  }
	  metaschema_ = new GenericSchemaDocument<ValueType, AllocatorType>(d, 0, 0, 0, 0, PointerType(), true);
	}
      }
      const SchemaType* sc = &(metaschema_->GetRoot());
      *schema = sc;
      return sc->GetId();
    }
#endif // RAPIDJSON_YGGDRASIL

    // Changed by PR #1393
    // TODO should this return a UriType& ?
    bool HandleRefSchema(const PointerType& source, const SchemaType** schema, const ValueType& v, const ValueType& document, const UriType& id) {
        typename ValueType::ConstMemberIterator itr = v.FindMember(SchemaType::GetRefString());
        if (itr == v.MemberEnd())
            return false;

        // Resolve the source pointer to the $ref'ed schema (finally)
        new (schemaRef_.template Push<SchemaRefPtr>()) SchemaRefPtr(&source);

        if (itr->value.IsString()) {
            SizeType len = itr->value.GetStringLength();
            if (len > 0) {
                // First resolve $ref against the in-scope id
                UriType scopeId = UriType(id, allocator_);
                UriType ref = UriType(itr->value, allocator_).Resolve(scopeId, allocator_);
                // See if the resolved $ref minus the fragment matches a resolved id in this document
                // Search from the root. Returns the subschema in the document and its absolute JSON pointer.
                PointerType basePointer = PointerType();
                const ValueType *base = FindId(document, ref, basePointer, docId_, false);
                if (!base) {
                    // Remote reference - call the remote document provider
                    if (remoteProvider_) {
                        if (const GenericSchemaDocument* remoteDocument = remoteProvider_->GetRemoteDocument(ref)) {
                            const Ch* s = ref.GetFragString();
                            len = ref.GetFragStringLength();
                            if (len <= 1 || s[1] == '/') {
                                // JSON pointer fragment, absolute in the remote schema
                                const PointerType pointer(s, len, allocator_);
                                if (pointer.IsValid()) {
                                    // Get the subschema
                                    if (const SchemaType *sc = remoteDocument->GetSchema(pointer)) {
                                        if (schema)
                                            *schema = sc;
                                        AddSchemaRefs(const_cast<SchemaType *>(sc));
                                        return true;
                                    }
                                }
                          } else {
                            // Plain name fragment, not allowed
                          }
                        }
                    }
                }
                else { // Local reference
                    const Ch* s = ref.GetFragString();
                    len = ref.GetFragStringLength();
                    if (len <= 1 || s[1] == '/') {
                        // JSON pointer fragment, relative to the resolved URI
                        const PointerType relPointer(s, len, allocator_);
                        if (relPointer.IsValid()) {
                            // Get the subschema
                            if (const ValueType *pv = relPointer.Get(*base)) {
                                // Now get the absolute JSON pointer by adding relative to base
                                PointerType pointer(basePointer);
                                for (SizeType i = 0; i < relPointer.GetTokenCount(); i++)
                                    pointer = pointer.Append(relPointer.GetTokens()[i], allocator_);
                                //GenericStringBuffer<EncodingType> sb;
                                //pointer.StringifyUriFragment(sb);
                                if (pointer.IsValid() && !IsCyclicRef(pointer)) {
                                    // Call CreateSchema recursively, but first compute the in-scope id for the $ref target as we have jumped there
                                    // TODO: cache pointer <-> id mapping
                                    size_t unresolvedTokenIndex;
                                    scopeId = pointer.GetUri(document, docId_, &unresolvedTokenIndex, allocator_);
                                    CreateSchema(schema, pointer, *pv, document, scopeId);
                                    return true;
                                }
                            }
                        }
                    } else {
                        // Plain name fragment, relative to the resolved URI
                        // See if the fragment matches an id in this document.
                        // Search from the base we just established. Returns the subschema in the document and its absolute JSON pointer.
                        PointerType pointer = PointerType();
                        if (const ValueType *pv = FindId(*base, ref, pointer, UriType(ref.GetBaseString(), ref.GetBaseStringLength(), allocator_), true, basePointer)) {
                            if (!IsCyclicRef(pointer)) {
                                //GenericStringBuffer<EncodingType> sb;
                                //pointer.StringifyUriFragment(sb);
                                // Call CreateSchema recursively, but first compute the in-scope id for the $ref target as we have jumped there
                                // TODO: cache pointer <-> id mapping
                                size_t unresolvedTokenIndex;
                                scopeId = pointer.GetUri(document, docId_, &unresolvedTokenIndex, allocator_);
                                CreateSchema(schema, pointer, *pv, document, scopeId);
                                return true;
                            }
                        }
                    }
                }
            }
        }

        // Invalid/Unknown $ref
        if (schema)
            *schema = typeless_;
        AddSchemaRefs(typeless_);
        return true;
    }

    //! Find the first subschema with a resolved 'id' that matches the specified URI.
    // If full specified use all URI else ignore fragment.
    // If found, return a pointer to the subschema and its JSON pointer.
    // TODO cache pointer <-> id mapping
    ValueType* FindId(const ValueType& doc, const UriType& finduri, PointerType& resptr, const UriType& baseuri, bool full, const PointerType& here = PointerType()) const {
        SizeType i = 0;
        ValueType* resval = 0;
        UriType tempuri = UriType(finduri, allocator_);
        UriType localuri = UriType(baseuri, allocator_);
        if (doc.GetType() == kObjectType) {
            // Establish the base URI of this object
            typename ValueType::ConstMemberIterator m = doc.FindMember(SchemaType::GetIdString());
            if (m != doc.MemberEnd() && m->value.GetType() == kStringType) {
                localuri = UriType(m->value, allocator_).Resolve(baseuri, allocator_);
            }
            // See if it matches
            if (localuri.Match(finduri, full)) {
                resval = const_cast<ValueType *>(&doc);
                resptr = here;
                return resval;
            }
            // No match, continue looking
            for (m = doc.MemberBegin(); m != doc.MemberEnd(); ++m) {
                if (m->value.GetType() == kObjectType || m->value.GetType() == kArrayType) {
                    resval = FindId(m->value, finduri, resptr, localuri, full, here.Append(m->name.GetString(), m->name.GetStringLength(), allocator_));
                }
                if (resval) break;
            }
        } else if (doc.GetType() == kArrayType) {
            // Continue looking
            for (typename ValueType::ConstValueIterator v = doc.Begin(); v != doc.End(); ++v) {
                if (v->GetType() == kObjectType || v->GetType() == kArrayType) {
                    resval = FindId(*v, finduri, resptr, localuri, full, here.Append(i, allocator_));
                }
                if (resval) break;
                i++;
            }
        }
        return resval;
    }

    // Added by PR #1393
    void AddSchemaRefs(SchemaType* schema) {
        while (!schemaRef_.Empty()) {
            SchemaRefPtr *ref = schemaRef_.template Pop<SchemaRefPtr>(1);
            SchemaEntry *entry = schemaMap_.template Push<SchemaEntry>();
            new (entry) SchemaEntry(**ref, schema, false, allocator_);
        }
    }

    // Added by PR #1393
    bool IsCyclicRef(const PointerType& pointer) const {
        for (const SchemaRefPtr* ref = schemaRef_.template Bottom<SchemaRefPtr>(); ref != schemaRef_.template End<SchemaRefPtr>(); ++ref)
            if (pointer == **ref)
                return true;
        return false;
    }

    const SchemaType* GetSchema(const PointerType& pointer) const {
        for (const SchemaEntry* target = schemaMap_.template Bottom<SchemaEntry>(); target != schemaMap_.template End<SchemaEntry>(); ++target)
            if (pointer == target->pointer)
                return target->schema;
        return 0;
    }

    PointerType GetPointer(const SchemaType* schema) const {
        for (const SchemaEntry* target = schemaMap_.template Bottom<SchemaEntry>(); target != schemaMap_.template End<SchemaEntry>(); ++target)
            if (schema == target->schema)
                return target->pointer;
        return PointerType();
    }

    const SchemaType* GetTypeless() const { return typeless_; }

    static const size_t kInitialSchemaMapSize = 64;
    static const size_t kInitialSchemaRefSize = 64;

    IRemoteSchemaDocumentProviderType* remoteProvider_;
    Allocator *allocator_;
    Allocator *ownAllocator_;
    const SchemaType* root_;                //!< Root schema.
    SchemaType* typeless_;
    internal::Stack<Allocator> schemaMap_;  // Stores created Pointer -> Schemas
    internal::Stack<Allocator> schemaRef_;  // Stores Pointer(s) from $ref(s) until resolved
    SValue uri_;                            // Schema document URI
    UriType docId_;
#ifdef RAPIDJSON_YGGDRASIL
    const GenericSchemaDocument<ValueT, Allocator>* metaschema_;
    bool isMetaschema_;
#endif // RAPIDJSON_YGGDRASIL
};

//! GenericSchemaDocument using Value type.
typedef GenericSchemaDocument<Value> SchemaDocument;
//! IGenericRemoteSchemaDocumentProvider using SchemaDocument.
typedef IGenericRemoteSchemaDocumentProvider<SchemaDocument> IRemoteSchemaDocumentProvider;

///////////////////////////////////////////////////////////////////////////////
// GenericSchemaValidator

//! JSON Schema Validator.
/*!
    A SAX style JSON schema validator.
    It uses a \c GenericSchemaDocument to validate SAX events.
    It delegates the incoming SAX events to an output handler.
    The default output handler does nothing.
    It can be reused multiple times by calling \c Reset().

    \tparam SchemaDocumentType Type of schema document.
    \tparam OutputHandler Type of output handler. Default handler does nothing.
    \tparam StateAllocator Allocator for storing the internal validation states.
*/
template <
    typename SchemaDocumentType,
    typename OutputHandler = BaseReaderHandler<typename SchemaDocumentType::SchemaType::EncodingType>,
    typename StateAllocator = CrtAllocator>
class GenericSchemaValidator :
    public internal::ISchemaStateFactory<typename SchemaDocumentType::SchemaType>, 
    public internal::ISchemaValidator,
    public internal::IValidationErrorHandler<typename SchemaDocumentType::SchemaType> {
public:
    typedef typename SchemaDocumentType::SchemaType SchemaType;
    typedef typename SchemaDocumentType::PointerType PointerType;
    typedef typename SchemaType::EncodingType EncodingType;
    typedef typename SchemaType::SValue SValue;
    typedef typename EncodingType::Ch Ch;
    typedef GenericStringRef<Ch> StringRefType;
    typedef GenericValue<EncodingType, StateAllocator> ValueType;
#ifdef RAPIDJSON_YGGDRASIL
    template <typename, typename, typename>
    friend class GenericSchemaNormalizer;
#endif // RAPIDJSON_YGGDRASIL

    //! Constructor without output handler.
    /*!
        \param schemaDocument The schema document to conform to.
        \param allocator Optional allocator for storing internal validation states.
        \param schemaStackCapacity Optional initial capacity of schema path stack.
        \param documentStackCapacity Optional initial capacity of document path stack.
    */
    GenericSchemaValidator(
        const SchemaDocumentType& schemaDocument,
        StateAllocator* allocator = 0, 
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity)
        :
        schemaDocument_(&schemaDocument),
        root_(schemaDocument.GetRoot()),
        stateAllocator_(allocator),
        ownStateAllocator_(0),
        schemaStack_(allocator, schemaStackCapacity),
        documentStack_(allocator, documentStackCapacity),
        outputHandler_(0),
        error_(kObjectType),
        currentError_(),
        missingDependents_(),
        valid_(true),
        flags_(kValidateDefaultFlags)
#if RAPIDJSON_SCHEMA_VERBOSE
        , depth_(0)
#endif
    {
    }

    //! Constructor with output handler.
    /*!
        \param schemaDocument The schema document to conform to.
        \param allocator Optional allocator for storing internal validation states.
        \param schemaStackCapacity Optional initial capacity of schema path stack.
        \param documentStackCapacity Optional initial capacity of document path stack.
    */
    GenericSchemaValidator(
        const SchemaDocumentType& schemaDocument,
        OutputHandler& outputHandler,
        StateAllocator* allocator = 0, 
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity)
        :
        schemaDocument_(&schemaDocument),
        root_(schemaDocument.GetRoot()),
        stateAllocator_(allocator),
        ownStateAllocator_(0),
        schemaStack_(allocator, schemaStackCapacity),
        documentStack_(allocator, documentStackCapacity),
        outputHandler_(&outputHandler),
        error_(kObjectType),
        currentError_(),
        missingDependents_(),
        valid_(true),
        flags_(kValidateDefaultFlags)
#if RAPIDJSON_SCHEMA_VERBOSE
        , depth_(0)
#endif
    {
    }

    //! Destructor.
    ~GenericSchemaValidator() {
        Reset();
        RAPIDJSON_DELETE(ownStateAllocator_);
    }

    //! Reset the internal states.
    void Reset() {
        while (!schemaStack_.Empty())
            PopSchema();
        documentStack_.Clear();
        ResetError();
    }

    //! Reset the error state.
    void ResetError() {
        error_.SetObject();
        currentError_.SetNull();
        missingDependents_.SetNull();
        valid_ = true;
    }

    //! Implementation of ISchemaValidator
    void SetValidateFlags(unsigned flags) {
        flags_ = flags;
    }
    virtual unsigned GetValidateFlags() const {
        return flags_;
    }

    //! Checks whether the current state is valid.
    // Implementation of ISchemaValidator
    virtual bool IsValid() const {
        if (!valid_) return false;
        if (GetContinueOnErrors() && !error_.ObjectEmpty()) return false;
        return true;
    }

    //! Gets the error object.
    ValueType& GetError() { return error_; }
    const ValueType& GetError() const { return error_; }

    //! Gets the JSON pointer pointed to the invalid schema.
    //  If reporting all errors, the stack will be empty.
    PointerType GetInvalidSchemaPointer() const {
        return schemaStack_.Empty() ? PointerType() : CurrentSchema().GetPointer();
    }

    //! Gets the keyword of invalid schema.
    //  If reporting all errors, the stack will be empty, so return "errors".
    const Ch* GetInvalidSchemaKeyword() const {
        if (!schemaStack_.Empty()) return CurrentContext().invalidKeyword;
        if (GetContinueOnErrors() && !error_.ObjectEmpty()) return (const Ch*)GetErrorsString();
        return 0;
    }

    //! Gets the error code of invalid schema.
    //  If reporting all errors, the stack will be empty, so return kValidateErrors.
    ValidateErrorCode GetInvalidSchemaCode() const {
        if (!schemaStack_.Empty()) return CurrentContext().invalidCode;
        if (GetContinueOnErrors() && !error_.ObjectEmpty()) return kValidateErrors;
        return kValidateErrorNone;
    }

    //! Gets the JSON pointer pointed to the invalid value.
    //  If reporting all errors, the stack will be empty.
    PointerType GetInvalidDocumentPointer() const {
        if (documentStack_.Empty()) {
            return PointerType();
        }
        else {
            return PointerType(documentStack_.template Bottom<Ch>(), documentStack_.GetSize() / sizeof(Ch));
        }
    }

    void NotMultipleOf(int64_t actual, const SValue& expected) {
        AddNumberError(kValidateErrorMultipleOf, ValueType(actual).Move(), expected);
    }
    void NotMultipleOf(uint64_t actual, const SValue& expected) {
        AddNumberError(kValidateErrorMultipleOf, ValueType(actual).Move(), expected);
    }
    void NotMultipleOf(double actual, const SValue& expected) {
        AddNumberError(kValidateErrorMultipleOf, ValueType(actual).Move(), expected);
    }
    void AboveMaximum(int64_t actual, const SValue& expected, bool exclusive) {
        AddNumberError(exclusive ? kValidateErrorExclusiveMaximum : kValidateErrorMaximum, ValueType(actual).Move(), expected,
            exclusive ? &SchemaType::GetExclusiveMaximumString : 0);
    }
    void AboveMaximum(uint64_t actual, const SValue& expected, bool exclusive) {
        AddNumberError(exclusive ? kValidateErrorExclusiveMaximum : kValidateErrorMaximum, ValueType(actual).Move(), expected,
            exclusive ? &SchemaType::GetExclusiveMaximumString : 0);
    }
    void AboveMaximum(double actual, const SValue& expected, bool exclusive) {
        AddNumberError(exclusive ? kValidateErrorExclusiveMaximum : kValidateErrorMaximum, ValueType(actual).Move(), expected,
            exclusive ? &SchemaType::GetExclusiveMaximumString : 0);
    }
    void BelowMinimum(int64_t actual, const SValue& expected, bool exclusive) {
        AddNumberError(exclusive ? kValidateErrorExclusiveMinimum : kValidateErrorMinimum, ValueType(actual).Move(), expected,
            exclusive ? &SchemaType::GetExclusiveMinimumString : 0);
    }
    void BelowMinimum(uint64_t actual, const SValue& expected, bool exclusive) {
        AddNumberError(exclusive ? kValidateErrorExclusiveMinimum : kValidateErrorMinimum, ValueType(actual).Move(), expected,
            exclusive ? &SchemaType::GetExclusiveMinimumString : 0);
    }
    void BelowMinimum(double actual, const SValue& expected, bool exclusive) {
        AddNumberError(exclusive ? kValidateErrorExclusiveMinimum : kValidateErrorMinimum, ValueType(actual).Move(), expected,
            exclusive ? &SchemaType::GetExclusiveMinimumString : 0);
    }

    void TooLong(const Ch* str, SizeType length, SizeType expected) {
        AddNumberError(kValidateErrorMaxLength,
            ValueType(str, length, GetStateAllocator()).Move(), SValue(expected).Move());
    }
    void TooShort(const Ch* str, SizeType length, SizeType expected) {
        AddNumberError(kValidateErrorMinLength,
            ValueType(str, length, GetStateAllocator()).Move(), SValue(expected).Move());
    }
    void DoesNotMatch(const Ch* str, SizeType length) {
        currentError_.SetObject();
        currentError_.AddMember(GetActualString(), ValueType(str, length, GetStateAllocator()).Move(), GetStateAllocator());
        AddCurrentError(kValidateErrorPattern);
    }

    void DisallowedItem(SizeType index) {
        currentError_.SetObject();
        currentError_.AddMember(GetDisallowedString(), ValueType(index).Move(), GetStateAllocator());
        AddCurrentError(kValidateErrorAdditionalItems, true);
    }
    void TooFewItems(SizeType actualCount, SizeType expectedCount) {
        AddNumberError(kValidateErrorMinItems,
            ValueType(actualCount).Move(), SValue(expectedCount).Move());
    }
    void TooManyItems(SizeType actualCount, SizeType expectedCount) {
        AddNumberError(kValidateErrorMaxItems,
            ValueType(actualCount).Move(), SValue(expectedCount).Move());
    }
    void DuplicateItems(SizeType index1, SizeType index2) {
        ValueType duplicates(kArrayType);
        duplicates.PushBack(index1, GetStateAllocator());
        duplicates.PushBack(index2, GetStateAllocator());
        currentError_.SetObject();
        currentError_.AddMember(GetDuplicatesString(), duplicates, GetStateAllocator());
        AddCurrentError(kValidateErrorUniqueItems, true);
    }

    void TooManyProperties(SizeType actualCount, SizeType expectedCount) {
        AddNumberError(kValidateErrorMaxProperties,
            ValueType(actualCount).Move(), SValue(expectedCount).Move());
    }
    void TooFewProperties(SizeType actualCount, SizeType expectedCount) {
        AddNumberError(kValidateErrorMinProperties,
            ValueType(actualCount).Move(), SValue(expectedCount).Move());
    }
    void StartMissingProperties() {
        currentError_.SetArray();
    }
    void AddMissingProperty(const SValue& name) {
        currentError_.PushBack(ValueType(name, GetStateAllocator()).Move(), GetStateAllocator());
    }
    bool EndMissingProperties() {
        if (currentError_.Empty())
            return false;
        ValueType error(kObjectType);
        error.AddMember(GetMissingString(), currentError_, GetStateAllocator());
        currentError_ = error;
        AddCurrentError(kValidateErrorRequired);
        return true;
    }
    void PropertyViolations(ISchemaValidator** subvalidators, SizeType count) {
        for (SizeType i = 0; i < count; ++i)
            MergeError(static_cast<GenericSchemaValidator*>(subvalidators[i])->GetError());
    }
    void DisallowedProperty(const Ch* name, SizeType length) {
        currentError_.SetObject();
        currentError_.AddMember(GetDisallowedString(), ValueType(name, length, GetStateAllocator()).Move(), GetStateAllocator());
        AddCurrentError(kValidateErrorAdditionalProperties, true);
    }

    void StartDependencyErrors() {
        currentError_.SetObject();
    }
    void StartMissingDependentProperties() {
        missingDependents_.SetArray();
    }
    void AddMissingDependentProperty(const SValue& targetName) {
        missingDependents_.PushBack(ValueType(targetName, GetStateAllocator()).Move(), GetStateAllocator());
    }
    void EndMissingDependentProperties(const SValue& sourceName) {
        if (!missingDependents_.Empty()) {
            // Create equivalent 'required' error
            ValueType error(kObjectType);
            ValidateErrorCode code = kValidateErrorRequired;
            error.AddMember(GetMissingString(), missingDependents_.Move(), GetStateAllocator());
            AddErrorCode(error, code);
            AddErrorInstanceLocation(error, false);
            // When appending to a pointer ensure its allocator is used
            PointerType schemaRef = GetInvalidSchemaPointer().Append(SchemaType::GetValidateErrorKeyword(kValidateErrorDependencies), &GetInvalidSchemaPointer().GetAllocator());
            AddErrorSchemaLocation(error, schemaRef.Append(sourceName.GetString(), sourceName.GetStringLength(), &GetInvalidSchemaPointer().GetAllocator()));
            ValueType wrapper(kObjectType);
            wrapper.AddMember(ValueType(SchemaType::GetValidateErrorKeyword(code), GetStateAllocator()).Move(), error, GetStateAllocator());
            currentError_.AddMember(ValueType(sourceName, GetStateAllocator()).Move(), wrapper, GetStateAllocator());
        }
    }
    void AddDependencySchemaError(const SValue& sourceName, ISchemaValidator* subvalidator) {
        currentError_.AddMember(ValueType(sourceName, GetStateAllocator()).Move(),
            static_cast<GenericSchemaValidator*>(subvalidator)->GetError(), GetStateAllocator());
    }
    bool EndDependencyErrors() {
        if (currentError_.ObjectEmpty())
            return false;
        ValueType error(kObjectType);
        error.AddMember(GetErrorsString(), currentError_, GetStateAllocator());
        currentError_ = error;
        AddCurrentError(kValidateErrorDependencies);
        return true;
    }

    void DisallowedValue(const ValidateErrorCode code = kValidateErrorEnum) {
        currentError_.SetObject();
        AddCurrentError(code);
    }
    void StartDisallowedType() {
        currentError_.SetArray();
    }
    void AddExpectedType(const typename SchemaType::ValueType& expectedType) {
        currentError_.PushBack(ValueType(expectedType, GetStateAllocator()).Move(), GetStateAllocator());
    }
    void EndDisallowedType(const typename SchemaType::ValueType& actualType) {
        ValueType error(kObjectType);
        error.AddMember(GetExpectedString(), currentError_, GetStateAllocator());
        error.AddMember(GetActualString(), ValueType(actualType, GetStateAllocator()).Move(), GetStateAllocator());
        currentError_ = error;
        AddCurrentError(kValidateErrorType);
    }
    void NotAllOf(ISchemaValidator** subvalidators, SizeType count) {
        // Treat allOf like oneOf and anyOf to match https://rapidjson.org/md_doc_schema.html#allOf-anyOf-oneOf
        AddErrorArray(kValidateErrorAllOf, subvalidators, count);
        //for (SizeType i = 0; i < count; ++i) {
        //    MergeError(static_cast<GenericSchemaValidator*>(subvalidators[i])->GetError());
        //}
    }
    void NoneOf(ISchemaValidator** subvalidators, SizeType count) {
        AddErrorArray(kValidateErrorAnyOf, subvalidators, count);
    }
    void NotOneOf(ISchemaValidator** subvalidators, SizeType count, bool matched = false) {
        AddErrorArray(matched ? kValidateErrorOneOfMatch : kValidateErrorOneOf, subvalidators, count);
    }
    void Disallowed() {
        currentError_.SetObject();
        AddCurrentError(kValidateErrorNot);
    }

#ifdef RAPIDJSON_YGGDRASIL
  void MissingRequiredSchemaProperty(const typename SchemaType::ValueType& name) {
    currentError_.SetObject();
    currentError_.AddMember(GetMissingString(),
			    ValueType(name, GetStateAllocator()).Move(),
			    GetStateAllocator());
    AddCurrentError(kValidateErrorPythonImport, true);
  }
  void IncorrectSubType(const typename SchemaType::ValueType& actual, const typename SchemaType::ValueType& expected) {
    currentError_.SetObject();
    currentError_.AddMember(GetExpectedString(),
			    ValueType(expected, GetStateAllocator()).Move(),
			    GetStateAllocator());
    currentError_.AddMember(GetActualString(),
			    ValueType(actual, GetStateAllocator()).Move(),
			    GetStateAllocator());
    AddCurrentError(kValidateErrorSubType, true);
  }
  void IncorrectPrecision(const typename SchemaType::ValueType& actual, const SValue& expected) {
    AddNumberError(kValidateErrorPrecision,
		   ValueType(actual, GetStateAllocator()).Move(), expected);
  }
  void IncorrectUnits(const typename SchemaType::ValueType& actual, const SValue& expected) {
    currentError_.SetObject();
    currentError_.AddMember(GetExpectedString(),
			    ValueType(expected, GetStateAllocator()).Move(),
			    GetStateAllocator());
    currentError_.AddMember(GetActualString(),
			    ValueType(actual, GetStateAllocator()).Move(),
			    GetStateAllocator());
    AddCurrentError(kValdiateErrorUnits, true);
  }
  void IncorrectShape(const SValue& actual, const SValue& expected) {
    currentError_.SetObject();
    currentError_.AddMember(GetExpectedString(),
			    ValueType(expected, GetStateAllocator()).Move(),
			    GetStateAllocator());
    currentError_.AddMember(GetActualString(),
			    ValueType(actual, GetStateAllocator()).Move(),
			    GetStateAllocator());
    AddCurrentError(kValidateErrorShape, true);
  }
  void InvalidPythonImport(const Ch* str, SizeType len) {
    currentError_.SetObject();
    currentError_.AddMember(GetDisallowedString(), ValueType(str, len, GetStateAllocator()).Move(), GetStateAllocator());
    AddCurrentError(kValidateErrorPythonImport, true);
  }
  void InvalidSchema(ValidateErrorCode code, ISchemaValidator* subvalidator) {
    currentError_.SetObject();
    currentError_.AddMember(GetErrorsString(),
			   static_cast<GenericSchemaValidator*>(subvalidator)->GetError(),
			   GetStateAllocator());
    AddCurrentError(code, true);
  }
  void DuplicateAlias(const SValue& base, const SValue& alias) {
    currentError_.SetObject();
    currentError_.AddMember(GetDuplicatesString(),
			    ValueType(kArrayType),
			    GetStateAllocator());
    ValueType dup(GetDuplicatesString(), GetStateAllocator());
    currentError_[dup].PushBack(ValueType(base, GetStateAllocator()).Move(),
				GetStateAllocator());
    currentError_[dup].PushBack(ValueType(alias, GetStateAllocator()).Move(),
				GetStateAllocator());
    AddCurrentError(kNormalizeErrorAliasDuplicate, true);
  }
  void CircularAlias(const SValue& alias) {
    currentError_.SetObject();
    currentError_.AddMember(GetCircularString(),
			    ValueType(alias, GetStateAllocator()).Move(),
			    GetStateAllocator());
    AddCurrentError(kNormalizeErrorCircularAlias, true);
  }
  void ConflictingAliases(const SValue& alias,
			  const SValue& base1,
			  const SValue& base2) {
    currentError_.SetObject();
    currentError_.AddMember(GetConflictingString(),
			    ValueType(alias, GetStateAllocator()).Move(),
			    GetStateAllocator());
    currentError_.AddMember(GetExpectedString(),
			    ValueType(base1, GetStateAllocator()).Move(),
			    GetStateAllocator());
    currentError_.AddMember(GetActualString(),
			    ValueType(base2, GetStateAllocator()).Move(),
			    GetStateAllocator());
    AddCurrentError(kNormalizeErrorConflictingAliases, true);
  }

#endif // RAPIDJSON_YGGDRASIL
  

#define RAPIDJSON_STRING_(name, ...) \
    static const StringRefType& Get##name##String() {\
        static const Ch s[] = { __VA_ARGS__, '\0' };\
        static const StringRefType v(s, static_cast<SizeType>(sizeof(s) / sizeof(Ch) - 1)); \
        return v;\
    }

    RAPIDJSON_STRING_(InstanceRef, 'i', 'n', 's', 't', 'a', 'n', 'c', 'e', 'R', 'e', 'f')
    RAPIDJSON_STRING_(SchemaRef, 's', 'c', 'h', 'e', 'm', 'a', 'R', 'e', 'f')
    RAPIDJSON_STRING_(Expected, 'e', 'x', 'p', 'e', 'c', 't', 'e', 'd')
    RAPIDJSON_STRING_(Actual, 'a', 'c', 't', 'u', 'a', 'l')
    RAPIDJSON_STRING_(Disallowed, 'd', 'i', 's', 'a', 'l', 'l', 'o', 'w', 'e', 'd')
    RAPIDJSON_STRING_(Missing, 'm', 'i', 's', 's', 'i', 'n', 'g')
    RAPIDJSON_STRING_(Errors, 'e', 'r', 'r', 'o', 'r', 's')
    RAPIDJSON_STRING_(ErrorCode, 'e', 'r', 'r', 'o', 'r', 'C', 'o', 'd', 'e')
    RAPIDJSON_STRING_(ErrorMessage, 'e', 'r', 'r', 'o', 'r', 'M', 'e', 's', 's', 'a', 'g', 'e')
    RAPIDJSON_STRING_(Duplicates, 'd', 'u', 'p', 'l', 'i', 'c', 'a', 't', 'e', 's')
#ifdef RAPIDJSON_YGGDRASIL
    RAPIDJSON_STRING_(Circular, 'c', 'i', 'r', 'c', 'u', 'l', 'a', 'r')
    RAPIDJSON_STRING_(Conflicting, 'c', 'o', 'n', 'f', 'l', 'i', 'c', 't', 'i', 'n', 'g')
#endif //RAPIDJSON_YGGDRASIL
#undef RAPIDJSON_STRING_

#if RAPIDJSON_SCHEMA_VERBOSE
#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_VERBOSE_() \
RAPIDJSON_MULTILINEMACRO_BEGIN\
    *documentStack_.template Push<Ch>() = '\0';\
    documentStack_.template Pop<Ch>(1);\
    internal::PrintInvalidDocument(documentStack_.template Bottom<Ch>());\
RAPIDJSON_MULTILINEMACRO_END
#else
#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_VERBOSE_()
#endif

#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_(method, arg1)\
    if (!valid_) return false; \
    if ((!BeginValue() && !GetContinueOnErrors()) || (!CurrentSchema().method arg1 && !GetContinueOnErrors())) {\
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_VERBOSE_();\
        return valid_ = false;\
    }

#define RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(method, arg2)\
    for (Context* context = schemaStack_.template Bottom<Context>(); context != schemaStack_.template End<Context>(); context++) {\
        if (context->hasher)\
            static_cast<HasherType*>(context->hasher)->method arg2;\
        if (context->validators)\
            for (SizeType i_ = 0; i_ < context->validatorCount; i_++)\
                static_cast<GenericSchemaValidator*>(context->validators[i_])->method arg2;\
        if (context->patternPropertiesValidators)\
            for (SizeType i_ = 0; i_ < context->patternPropertiesValidatorCount; i_++)\
                static_cast<GenericSchemaValidator*>(context->patternPropertiesValidators[i_])->method arg2;\
    }

#define RAPIDJSON_SCHEMA_HANDLE_END_(method, arg2)\
    valid_ = (EndValue() || GetContinueOnErrors()) && (!outputHandler_ || outputHandler_->method arg2);\
    return valid_;

#define RAPIDJSON_SCHEMA_HANDLE_VALUE_(method, arg1, arg2) \
    RAPIDJSON_SCHEMA_HANDLE_BEGIN_   (method, arg1);\
    RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(method, arg2);\
    RAPIDJSON_SCHEMA_HANDLE_END_     (method, arg2)

    bool Null()             { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Null,   (CurrentContext()), ( )); }
    bool Bool(bool b)       { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Bool,   (CurrentContext(), b), (b)); }
    bool Int(int i)         { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Int,    (CurrentContext(), i), (i)); }
    bool Uint(unsigned u)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Uint,   (CurrentContext(), u), (u)); }
    bool Int64(int64_t i)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Int64,  (CurrentContext(), i), (i)); }
    bool Uint64(uint64_t u) { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Uint64, (CurrentContext(), u), (u)); }
    bool Double(double d)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Double, (CurrentContext(), d), (d)); }
    bool RawNumber(const Ch* str, SizeType length, bool copy)
                                    { RAPIDJSON_SCHEMA_HANDLE_VALUE_(String, (CurrentContext(), str, length, copy), (str, length, copy)); }
    bool String(const Ch* str, SizeType length, bool copy)
                                    { RAPIDJSON_SCHEMA_HANDLE_VALUE_(String, (CurrentContext(), str, length, copy), (str, length, copy)); }

#ifdef RAPIDJSON_YGGDRASIL
#define RAPIDJSON_SCHEMA_HANDLE_END_YGG_(method, arg1, arg2)		\
    if (internal::HasYggdrasilMethodImpl<OutputHandler,ValueType>::Value) { \
      RAPIDJSON_SCHEMA_HANDLE_END_(Yggdrasil ## method, arg1);		\
    } else {								\
      RAPIDJSON_SCHEMA_HANDLE_END_(method, arg2);			\
    }
  
  template <typename YggSchemaValueType>
  bool YggdrasilString(const Ch* str, SizeType length, bool copy, YggSchemaValueType& schema)
  {
    RAPIDJSON_SCHEMA_HANDLE_BEGIN_(YggdrasilString, (CurrentContext(), str, length, copy, schema));
    RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(YggdrasilString, (str, length, copy, schema));
    RAPIDJSON_SCHEMA_HANDLE_END_YGG_(String, (str, length, copy, schema), (str, length, copy));
  }
  template <typename YggSchemaValueType>
  bool YggdrasilStartObject(YggSchemaValueType& schema) {
    RAPIDJSON_SCHEMA_HANDLE_BEGIN_(YggdrasilStartObject, (CurrentContext(), schema));
    RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(YggdrasilStartObject, (schema));
    if (internal::HasYggdrasilMethodImpl<OutputHandler,ValueType>::Value)
      return valid_ = !outputHandler_ || outputHandler_->YggdrasilStartObject(schema);
    else
      return valid_ = !outputHandler_ || outputHandler_->StartObject();
  }
  bool YggdrasilEndObject(SizeType memberCount) {
    if (!valid_) return false;
    RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(YggdrasilEndObject, (memberCount));
    if (!CurrentSchema().YggdrasilEndObject(CurrentContext(), memberCount) && !GetContinueOnErrors()) return valid_ = false;
    RAPIDJSON_SCHEMA_HANDLE_END_YGG_(EndObject, (memberCount), (memberCount));
  }

 
#undef RAPIDJSON_SCHEMA_HANDLE_END_YGG_
#endif // RAPIDJSON_YGGDRASIL

    bool StartObject() {
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_(StartObject, (CurrentContext()));
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(StartObject, ());
        return valid_ = !outputHandler_ || outputHandler_->StartObject();
    }
    
    bool Key(const Ch* str, SizeType len, bool copy) {
        if (!valid_) return false;
        AppendToken(str, len);
        if (!CurrentSchema().Key(CurrentContext(), str, len, copy) && !GetContinueOnErrors()) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(Key, (str, len, copy));
        return valid_ = !outputHandler_ || outputHandler_->Key(str, len, copy);
    }
    
    bool EndObject(SizeType memberCount) {
        if (!valid_) return false;
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(EndObject, (memberCount));
        if (!CurrentSchema().EndObject(CurrentContext(), memberCount) && !GetContinueOnErrors()) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_END_(EndObject, (memberCount));
    }

    bool StartArray() {
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_(StartArray, (CurrentContext()));
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(StartArray, ());
        return valid_ = !outputHandler_ || outputHandler_->StartArray();
    }
    
    bool EndArray(SizeType elementCount) {
        if (!valid_) return false;
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(EndArray, (elementCount));
        if (!CurrentSchema().EndArray(CurrentContext(), elementCount) && !GetContinueOnErrors()) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_END_(EndArray, (elementCount));
    }
  
#undef RAPIDJSON_SCHEMA_HANDLE_BEGIN_VERBOSE_
#undef RAPIDJSON_SCHEMA_HANDLE_BEGIN_
#undef RAPIDJSON_SCHEMA_HANDLE_PARALLEL_
#undef RAPIDJSON_SCHEMA_HANDLE_VALUE_

    // Implementation of ISchemaStateFactory<SchemaType>
    virtual ISchemaValidator* CreateSchemaValidator(const SchemaType& root, const bool inheritContinueOnErrors) {
        ISchemaValidator* sv = new (GetStateAllocator().Malloc(sizeof(GenericSchemaValidator))) GenericSchemaValidator(*schemaDocument_, root, documentStack_.template Bottom<char>(), documentStack_.GetSize(),
#if RAPIDJSON_SCHEMA_VERBOSE
        depth_ + 1,
#endif
        &GetStateAllocator());
        sv->SetValidateFlags(inheritContinueOnErrors ? GetValidateFlags() : GetValidateFlags() & ~(unsigned)kValidateContinueOnErrorFlag);
        return sv;
    }

    virtual void DestroySchemaValidator(ISchemaValidator* validator) {
        GenericSchemaValidator* v = static_cast<GenericSchemaValidator*>(validator);
        v->~GenericSchemaValidator();
        StateAllocator::Free(v);
    }

    virtual void* CreateHasher() {
        return new (GetStateAllocator().Malloc(sizeof(HasherType))) HasherType(&GetStateAllocator());
    }

    virtual uint64_t GetHashCode(void* hasher) {
        return static_cast<HasherType*>(hasher)->GetHashCode();
    }

    virtual void DestroryHasher(void* hasher) {
        HasherType* h = static_cast<HasherType*>(hasher);
        h->~HasherType();
        StateAllocator::Free(h);
    }

    virtual void* MallocState(size_t size) {
        return GetStateAllocator().Malloc(size);
    }

    virtual void FreeState(void* p) {
        StateAllocator::Free(p);
    }

private:
    typedef typename SchemaType::Context Context;
    typedef GenericValue<UTF8<>, StateAllocator> HashCodeArray;
    typedef internal::Hasher<EncodingType, StateAllocator> HasherType;

    GenericSchemaValidator( 
        const SchemaDocumentType& schemaDocument,
        const SchemaType& root,
        const char* basePath, size_t basePathSize,
#if RAPIDJSON_SCHEMA_VERBOSE
        unsigned depth,
#endif
        StateAllocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity)
        :
        schemaDocument_(&schemaDocument),
        root_(root),
        stateAllocator_(allocator),
        ownStateAllocator_(0),
        schemaStack_(allocator, schemaStackCapacity),
        documentStack_(allocator, documentStackCapacity),
        outputHandler_(0),
        error_(kObjectType),
        currentError_(),
        missingDependents_(),
        valid_(true),
        flags_(kValidateDefaultFlags)
#if RAPIDJSON_SCHEMA_VERBOSE
        , depth_(depth)
#endif
    {
        if (basePath && basePathSize)
            memcpy(documentStack_.template Push<char>(basePathSize), basePath, basePathSize);
    }

    StateAllocator& GetStateAllocator() {
        if (!stateAllocator_)
            stateAllocator_ = ownStateAllocator_ = RAPIDJSON_NEW(StateAllocator)();
        return *stateAllocator_;
    }

    bool GetContinueOnErrors() const {
        return flags_ & kValidateContinueOnErrorFlag;
    }
  
#ifdef RAPIDJSON_YGGDRASIL
    virtual
#endif // RAPIDJSON_YGGDRASIL
    bool BeginValue() {
        if (schemaStack_.Empty())
            PushSchema(root_);
        else {
            if (CurrentContext().inArray)
                internal::TokenHelper<internal::Stack<StateAllocator>, Ch>::AppendIndexToken(documentStack_, CurrentContext().arrayElementIndex);

            if (!CurrentSchema().BeginValue(CurrentContext()) && !GetContinueOnErrors())
                return false;

            SizeType count = CurrentContext().patternPropertiesSchemaCount;
            const SchemaType** sa = CurrentContext().patternPropertiesSchemas;
            typename Context::PatternValidatorType patternValidatorType = CurrentContext().valuePatternValidatorType;
            bool valueUniqueness = CurrentContext().valueUniqueness;
            RAPIDJSON_ASSERT(CurrentContext().valueSchema);
            PushSchema(*CurrentContext().valueSchema);

            if (count > 0) {
                CurrentContext().objectPatternValidatorType = patternValidatorType;
                ISchemaValidator**& va = CurrentContext().patternPropertiesValidators;
                SizeType& validatorCount = CurrentContext().patternPropertiesValidatorCount;
                va = static_cast<ISchemaValidator**>(MallocState(sizeof(ISchemaValidator*) * count));
                for (SizeType i = 0; i < count; i++)
                    va[validatorCount++] = CreateSchemaValidator(*sa[i], true);  // Inherit continueOnError
            }

            CurrentContext().arrayUniqueness = valueUniqueness;
        }
        return true;
    }

#ifdef RAPIDJSON_YGGDRASIL
    virtual
#endif // RAPIDJSON_YGGDRASIL
    bool EndValue() {
        if (!CurrentSchema().EndValue(CurrentContext()) && !GetContinueOnErrors())
            return false;

#if RAPIDJSON_SCHEMA_VERBOSE
        GenericStringBuffer<EncodingType> sb;
        schemaDocument_->GetPointer(&CurrentSchema()).Stringify(sb);

        *documentStack_.template Push<Ch>() = '\0';
        documentStack_.template Pop<Ch>(1);
        internal::PrintValidatorPointers(depth_, sb.GetString(), documentStack_.template Bottom<Ch>());
#endif
        void* hasher = CurrentContext().hasher;
        uint64_t h = hasher && CurrentContext().arrayUniqueness ? static_cast<HasherType*>(hasher)->GetHashCode() : 0;
        
        PopSchema();

        if (!schemaStack_.Empty()) {
            Context& context = CurrentContext();
            // Only check uniqueness if there is a hasher
            if (hasher && context.valueUniqueness) {
                HashCodeArray* a = static_cast<HashCodeArray*>(context.arrayElementHashCodes);
                if (!a)
                    CurrentContext().arrayElementHashCodes = a = new (GetStateAllocator().Malloc(sizeof(HashCodeArray))) HashCodeArray(kArrayType);
                for (typename HashCodeArray::ConstValueIterator itr = a->Begin(); itr != a->End(); ++itr)
                    if (itr->GetUint64() == h) {
                        DuplicateItems(static_cast<SizeType>(itr - a->Begin()), a->Size());
                        // Cleanup before returning if continuing
                        if (GetContinueOnErrors()) {
                            a->PushBack(h, GetStateAllocator());
                            while (!documentStack_.Empty() && *documentStack_.template Pop<Ch>(1) != '/');
                        }
                        RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorUniqueItems);
                    }
                a->PushBack(h, GetStateAllocator());
            }
        }

        // Remove the last token of document pointer
        while (!documentStack_.Empty() && *documentStack_.template Pop<Ch>(1) != '/')
            ;

        return true;
    }

    void AppendToken(const Ch* str, SizeType len) {
        documentStack_.template Reserve<Ch>(1 + len * 2); // worst case all characters are escaped as two characters
        *documentStack_.template PushUnsafe<Ch>() = '/';
        for (SizeType i = 0; i < len; i++) {
            if (str[i] == '~') {
                *documentStack_.template PushUnsafe<Ch>() = '~';
                *documentStack_.template PushUnsafe<Ch>() = '0';
            }
            else if (str[i] == '/') {
                *documentStack_.template PushUnsafe<Ch>() = '~';
                *documentStack_.template PushUnsafe<Ch>() = '1';
            }
            else
                *documentStack_.template PushUnsafe<Ch>() = str[i];
        }
    }

    RAPIDJSON_FORCEINLINE void PushSchema(const SchemaType& schema) { new (schemaStack_.template Push<Context>()) Context(*this, *this, &schema); }
    
    RAPIDJSON_FORCEINLINE void PopSchema() {
        Context* c = schemaStack_.template Pop<Context>(1);
        if (HashCodeArray* a = static_cast<HashCodeArray*>(c->arrayElementHashCodes)) {
            a->~HashCodeArray();
            StateAllocator::Free(a);
        }
        c->~Context();
    }

    void AddErrorInstanceLocation(ValueType& result, bool parent) {
        GenericStringBuffer<EncodingType> sb;
        PointerType instancePointer = GetInvalidDocumentPointer();
        ((parent && instancePointer.GetTokenCount() > 0)
         ? PointerType(instancePointer.GetTokens(), instancePointer.GetTokenCount() - 1)
         : instancePointer).StringifyUriFragment(sb);
        ValueType instanceRef(sb.GetString(), static_cast<SizeType>(sb.GetSize() / sizeof(Ch)),
                              GetStateAllocator());
        result.AddMember(GetInstanceRefString(), instanceRef, GetStateAllocator());
    }

    void AddErrorSchemaLocation(ValueType& result, PointerType schema = PointerType()) {
        GenericStringBuffer<EncodingType> sb;
        SizeType len = CurrentSchema().GetURI().GetStringLength();
        if (len) memcpy(sb.Push(len), CurrentSchema().GetURI().GetString(), len * sizeof(Ch));
        if (schema.GetTokenCount()) schema.StringifyUriFragment(sb);
        else GetInvalidSchemaPointer().StringifyUriFragment(sb);
        ValueType schemaRef(sb.GetString(), static_cast<SizeType>(sb.GetSize() / sizeof(Ch)),
            GetStateAllocator());
        result.AddMember(GetSchemaRefString(), schemaRef, GetStateAllocator());
    }

    void AddErrorCode(ValueType& result, const ValidateErrorCode code) {
        result.AddMember(GetErrorCodeString(), code, GetStateAllocator());
    }

    void AddError(ValueType& keyword, ValueType& error) {
        typename ValueType::MemberIterator member = error_.FindMember(keyword);
        if (member == error_.MemberEnd())
            error_.AddMember(keyword, error, GetStateAllocator());
        else {
            if (member->value.IsObject()) {
                ValueType errors(kArrayType);
                errors.PushBack(member->value, GetStateAllocator());
                member->value = errors;
            }
            member->value.PushBack(error, GetStateAllocator());
        }
    }

    void AddCurrentError(const ValidateErrorCode code, bool parent = false) {
        AddErrorCode(currentError_, code);
        AddErrorInstanceLocation(currentError_, parent);
        AddErrorSchemaLocation(currentError_);
        AddError(ValueType(SchemaType::GetValidateErrorKeyword(code), GetStateAllocator(), false).Move(), currentError_);
    }

    void MergeError(ValueType& other) {
        for (typename ValueType::MemberIterator it = other.MemberBegin(), end = other.MemberEnd(); it != end; ++it) {
            AddError(it->name, it->value);
        }
    }

    void AddNumberError(const ValidateErrorCode code, ValueType& actual, const SValue& expected,
        const typename SchemaType::ValueType& (*exclusive)() = 0) {
        currentError_.SetObject();
        currentError_.AddMember(GetActualString(), actual, GetStateAllocator());
        currentError_.AddMember(GetExpectedString(), ValueType(expected, GetStateAllocator()).Move(), GetStateAllocator());
        if (exclusive)
            currentError_.AddMember(ValueType(exclusive(), GetStateAllocator()).Move(), true, GetStateAllocator());
        AddCurrentError(code);
    }

    void AddErrorArray(const ValidateErrorCode code,
        ISchemaValidator** subvalidators, SizeType count) {
        ValueType errors(kArrayType);
        for (SizeType i = 0; i < count; ++i)
            errors.PushBack(static_cast<GenericSchemaValidator*>(subvalidators[i])->GetError(), GetStateAllocator());
        currentError_.SetObject();
        currentError_.AddMember(GetErrorsString(), errors, GetStateAllocator());
        AddCurrentError(code);
    }

    const SchemaType& CurrentSchema() const { return *schemaStack_.template Top<Context>()->schema; }
    Context& CurrentContext() { return *schemaStack_.template Top<Context>(); }
    const Context& CurrentContext() const { return *schemaStack_.template Top<Context>(); }

    static const size_t kDefaultSchemaStackCapacity = 1024;
    static const size_t kDefaultDocumentStackCapacity = 256;
    const SchemaDocumentType* schemaDocument_;
    const SchemaType& root_;
    StateAllocator* stateAllocator_;
    StateAllocator* ownStateAllocator_;
    internal::Stack<StateAllocator> schemaStack_;    //!< stack to store the current path of schema (BaseSchemaType *)
    internal::Stack<StateAllocator> documentStack_;  //!< stack to store the current path of validating document (Ch)
    OutputHandler* outputHandler_;
    ValueType error_;
    ValueType currentError_;
    ValueType missingDependents_;
    bool valid_;
    unsigned flags_;
#if RAPIDJSON_SCHEMA_VERBOSE
    unsigned depth_;
#endif
};

typedef GenericSchemaValidator<SchemaDocument> SchemaValidator;

///////////////////////////////////////////////////////////////////////////////
// SchemaValidatingReader

//! A helper class for parsing with validation.
/*!
    This helper class is a functor, designed as a parameter of \ref GenericDocument::Populate().

    \tparam parseFlags Combination of \ref ParseFlag.
    \tparam InputStream Type of input stream, implementing Stream concept.
    \tparam SourceEncoding Encoding of the input stream.
    \tparam SchemaDocumentType Type of schema document.
    \tparam StackAllocator Allocator type for stack.
*/
template <
    unsigned parseFlags,
    typename InputStream,
    typename SourceEncoding,
    typename SchemaDocumentType = SchemaDocument,
    typename StackAllocator = CrtAllocator>
class SchemaValidatingReader {
public:
    typedef typename SchemaDocumentType::PointerType PointerType;
    typedef typename InputStream::Ch Ch;
    typedef GenericValue<SourceEncoding, StackAllocator> ValueType;

    //! Constructor
    /*!
        \param is Input stream.
        \param sd Schema document.
    */
    SchemaValidatingReader(InputStream& is, const SchemaDocumentType& sd) : is_(is), sd_(sd), invalidSchemaKeyword_(), invalidSchemaCode_(kValidateErrorNone), error_(kObjectType), isValid_(true) {}

    template <typename Handler>
    bool operator()(Handler& handler) {
        GenericReader<SourceEncoding, typename SchemaDocumentType::EncodingType, StackAllocator> reader;
        GenericSchemaValidator<SchemaDocumentType, Handler> validator(sd_, handler);
        parseResult_ = reader.template Parse<parseFlags>(is_, validator);

        isValid_ = validator.IsValid();
        if (isValid_) {
            invalidSchemaPointer_ = PointerType();
            invalidSchemaKeyword_ = 0;
            invalidDocumentPointer_ = PointerType();
            error_.SetObject();
        }
        else {
            invalidSchemaPointer_ = validator.GetInvalidSchemaPointer();
            invalidSchemaKeyword_ = validator.GetInvalidSchemaKeyword();
            invalidSchemaCode_ = validator.GetInvalidSchemaCode();
            invalidDocumentPointer_ = validator.GetInvalidDocumentPointer();
            error_.CopyFrom(validator.GetError(), allocator_);
        }

        return parseResult_;
    }

    const ParseResult& GetParseResult() const { return parseResult_; }
    bool IsValid() const { return isValid_; }
    const PointerType& GetInvalidSchemaPointer() const { return invalidSchemaPointer_; }
    const Ch* GetInvalidSchemaKeyword() const { return invalidSchemaKeyword_; }
    const PointerType& GetInvalidDocumentPointer() const { return invalidDocumentPointer_; }
    const ValueType& GetError() const { return error_; }
    ValidateErrorCode GetInvalidSchemaCode() const { return invalidSchemaCode_; }

private:
    InputStream& is_;
    const SchemaDocumentType& sd_;

    ParseResult parseResult_;
    PointerType invalidSchemaPointer_;
    const Ch* invalidSchemaKeyword_;
    PointerType invalidDocumentPointer_;
    ValidateErrorCode invalidSchemaCode_;
    StackAllocator allocator_;
    ValueType error_;
    bool isValid_;
};

#ifdef RAPIDJSON_YGGDRASIL

template <
  typename SchemaDocumentType,
  typename OutputHandler = BaseReaderHandler<typename SchemaDocumentType::SchemaType::EncodingType>,
  typename StateAllocator = CrtAllocator>
class GenericSchemaNormalizer : public GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator> {
  typedef typename internal::ISchemaValidator ISchemaValidator;
public:
  typedef typename SchemaDocumentType::SchemaType SchemaType;
  typedef typename SchemaDocumentType::PointerType PointerType;
  typedef typename SchemaType::EncodingType EncodingType;
  typedef typename SchemaType::SValue SValue;
  typedef typename EncodingType::Ch Ch;
  typedef GenericStringRef<Ch> StringRefType;
  typedef GenericValue<EncodingType, StateAllocator> ValueType;
  typedef internal::GenericNormalizedDocument<SchemaDocumentType> NormalizedDocumentType;

  GenericSchemaNormalizer(
        const SchemaDocumentType& schemaDocument,
        StateAllocator* allocator = 0, 
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity) :
    GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>
    (schemaDocument,
     allocator,
     schemaStackCapacity,
     documentStackCapacity),
    normalized_(), normalization_depth_(0), validator_index_(0), child_validators_(0) {
    normalized_.SetDocumentStack(&this->documentStack_);
  }
  GenericSchemaNormalizer(
        const SchemaDocumentType& schemaDocument,
        OutputHandler& outputHandler,
        StateAllocator* allocator = 0, 
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity) :
    GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>
    (schemaDocument,
     outputHandler,
     allocator,
     schemaStackCapacity,
     documentStackCapacity),
    normalized_(), normalization_depth_(0), validator_index_(0), child_validators_(0) {
    normalized_.SetDocumentStack(&this->documentStack_);
  }

private:
  typedef typename SchemaType::Context Context;
  typedef GenericValue<UTF8<>, StateAllocator> HashCodeArray;
  typedef internal::Hasher<EncodingType, StateAllocator> HasherType;
  
  GenericSchemaNormalizer(
        const SchemaDocumentType& schemaDocument,
        const SchemaType& root,
        const char* basePath, size_t basePathSize,
#if RAPIDJSON_SCHEMA_VERBOSE
        unsigned depth,
#endif
	unsigned normalization_depth,
	NormalizedDocumentType& normalized,
	unsigned validator_index,
        StateAllocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity) :
    GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>
    (schemaDocument,
     root,
     basePath, basePathSize,
#if RAPIDJSON_SCHEMA_VERBOSE
     depth,
#endif
     allocator,
     schemaStackCapacity,
     documentStackCapacity),
    normalized_(&normalized, validator_index), normalization_depth_(normalization_depth), validator_index_(validator_index), child_validators_(0) {
    normalized_.SetDocumentStack(&this->documentStack_);
  }
  
  static const size_t kDefaultSchemaStackCapacity = 1024;
  static const size_t kDefaultDocumentStackCapacity = 256;
  NormalizedDocumentType normalized_;
  unsigned normalization_depth_;
  unsigned validator_index_;
  unsigned child_validators_;

public:

  //! Get the normalized version of the parsed document.
  const ValueType& GetNormalized() const { return normalized_.GetDocument(); }

  //! Check if the document was normalized.
  bool WasNormalized() const { return normalized_.WasModified(); }

  bool BeginValue() override {
    if (!GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>::BeginValue())
      return false;
    normalization_depth_++;
    this->CurrentContext().normalized = &normalized_;
    return true;
  }

  bool EndValue() override {
    if (!GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>::EndValue())
      return false;
    // TODO: Check parallel validators
    normalization_depth_--;
    if (normalization_depth_ == 0)
      normalized_.FinalizeFromStack();
    return true;
  }
  
  //! Implementation of ISchemaValidator
  unsigned GetValidatorID() const override { return validator_index_; }
  
  //! Implementation of ISchemaStateFactory<SchemaType>
  ISchemaValidator* CreateSchemaValidator(const SchemaType& root, const bool inheritContinueOnErrors) override {
    ISchemaValidator* sv = new (this->GetStateAllocator().Malloc(sizeof(GenericSchemaNormalizer))) GenericSchemaNormalizer(*this->schemaDocument_, root, this->documentStack_.template Bottom<char>(), this->documentStack_.GetSize(),
#if RAPIDJSON_SCHEMA_VERBOSE
        depth_ + 1,
#endif
        normalization_depth_ + 1,
        normalized_,
        child_validators_++,
        &this->GetStateAllocator());
    sv->SetValidateFlags(inheritContinueOnErrors ? this->GetValidateFlags() : this->GetValidateFlags() & ~(unsigned)kValidateContinueOnErrorFlag);
    return sv;
  }

  void DestroySchemaValidator(ISchemaValidator* validator) override {
    GenericSchemaNormalizer* v = static_cast<GenericSchemaNormalizer*>(validator);
    v->~GenericSchemaNormalizer();
    StateAllocator::Free(v);
  }

};

typedef GenericSchemaNormalizer<SchemaDocument> SchemaNormalizer;

#endif // RAPIDJSON_YGGDRASIL

RAPIDJSON_NAMESPACE_END
RAPIDJSON_DIAG_POP

#endif // RAPIDJSON_SCHEMA_H_
