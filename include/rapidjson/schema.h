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
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath> // abs, floor
#ifdef RAPIDJSON_YGGDRASIL
#include "units.h"
#include "writer.h"
#include "metaschema.h"
#if RAPIDJSON_HAS_CXX11
#define OVERRIDE_CXX11 override
#else // RAPIDJSON_HAS_CXX11
#define OVERRIDE_CXX11
#endif // RAPIDJSON_HAS_CXX11
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
#include "prettywriter.h"
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
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

#ifdef RAPIDJSON_YGGDRASIL
#define RAPIDJSON_INVALID_KEYWORD_WARNING(code)\
RAPIDJSON_MULTILINEMACRO_BEGIN\
    context.invalidCode = code;\
    context.invalidKeyword = SchemaType::GetValidateErrorKeyword(code).GetString();\
    RAPIDJSON_INVALID_KEYWORD_VERBOSE(context.invalidKeyword);\
RAPIDJSON_MULTILINEMACRO_END
#endif // RAPIDJSON_YGGDRASIL

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

#ifdef RAPIDJSON_YGGDRASIL
template <
  typename SchemaDocumentType,
  typename OutputHandler,
  typename StateAllocator>
class GenericSchemaNormalizer;
#endif // RAPIDJSON_YGGDRASIL

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
#ifdef RAPIDJSON_YGGDRASIL
    virtual ISchemaValidator* CreateSchemaValidator(const SchemaType&, const bool inheritContinueOnErrors, typename SchemaType::PointerType) = 0;
#else // RAPIDJSON_YGGDRASIL
    virtual ISchemaValidator* CreateSchemaValidator(const SchemaType&, const bool inheritContinueOnErrors) = 0;
#endif // RAPIDJSON_YGGDRASIL
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
  virtual void InvalidPythonClass(const Ch* str, SizeType len, const SValue& expected) = 0;
  virtual void InvalidSchema(ValidateErrorCode code, ISchemaValidator* subvalidator) = 0;
  // Normalization errors
  virtual void DuplicateAlias(const SValue& base, const SValue& alias) = 0;
  virtual void CircularAlias(const SValue& alias) = 0;
  virtual void ConflictingAliases(const SValue& alias, const SValue& base1, const SValue& base2) = 0;
  virtual ValidateErrorCode NotSingularItem(ISchemaValidator** subvalidator) = 0;
  virtual void NormalizationMergeConflict(const typename SchemaType::ValueType& cond, const SValue& expected, const SValue& actual) = 0;
  virtual void AddWarnings(ISchemaValidator** subvalidators, SizeType count) = 0;
  virtual void DeprecationWarning(const SValue* warning=nullptr) = 0;
  virtual bool EndMissingPropertiesShared(const SValue& instanceRef, const SValue& schemaRef) = 0;
  virtual void DisallowedValueEnum(const typename SchemaType::ValueType& expected) = 0;
  virtual ValidateErrorCode SharedNormalizationError(ISchemaValidator* subvalidator) = 0;
  virtual void GenericError(const char* str) = 0;
#endif // RAPIDJSON_YGGDRASIL
  
};

#ifdef RAPIDJSON_YGGDRASIL
#define RAPIDJSON_YGGDRASIL_GENERIC_SET_ERROR(...)			\
  {									\
    std::string error_msg;						\
    int N = snprintf(&(error_msg.front()), 0, __VA_ARGS__);		\
    if (N < 0) {							\
      std::cerr << "Error in snprintf while setting generic error" << std::endl;	\
      return false;							\
    }									\
    error_msg.reserve((size_t)(N + 1));					\
    snprintf(&(error_msg.front()), (size_t)(N + 1), __VA_ARGS__);	\
    context.error_handler.GenericError(error_msg.c_str());		\
  }
#define RAPIDJSON_YGGDRASIL_GENERIC_ERROR(...)				\
  {									\
    RAPIDJSON_YGGDRASIL_GENERIC_SET_ERROR(__VA_ARGS__);			\
    RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorGeneric);			\
  }
#else // RAPIDJSON_YGGDRASIL
#define RAPIDJSON_YGGDRASIL_GENERIC_ERROR(...) {}
#endif // RAPIDJSON_YGGDRASIL

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

#ifdef RAPIDJSON_YGGDRASIL
///////////////////////////////////////////////////////////////////////////////
// SharedProperties

template <typename SchemaDocumentType>
class SharedProperties {
public:
  typedef Schema<SchemaDocumentType> SchemaType;
  typedef typename SchemaType::SValue SValue;
  typedef typename SchemaType::ValueType ValueType;
  typedef typename SchemaType::AllocatorType AllocatorType;
  typedef typename SchemaType::SharedProperty SharedPropertyType;
  typedef typename SchemaType::PointerType PointerType;
  typedef typename SchemaType::Ch Ch;
  typedef GenericNormalizedDocument<SchemaDocumentType, RAPIDJSON_DEFAULT_STACK_ALLOCATOR> NormalizedDocumentType;
  typedef typename NormalizedDocumentType::ValueType NormValueType;
  SharedProperties() :
    localPropertyCount(0), otherPropertyCount(0), propertyCount(0),
    localProperties(), otherProperties() {}
  ~SharedProperties() {
    if (localProperties) {
      for (SizeType i = 0; i < localPropertyCount; i++)
	localProperties[i].~SharedPropertyType();
      AllocatorType::Free(localProperties);
      localProperties = 0;
    }
    if (otherProperties) {
      AllocatorType::Free(otherProperties);
      otherProperties = 0;
    }
  }
  void AddLocalProperty(SchemaType* schema, const SValue& key,
			const ValueType* v, bool push=false) {
    localPropertyCount++;
    propertyCount++;
    localProperties = static_cast<SharedPropertyType*>(schema->allocator_->Realloc(localProperties, sizeof(SharedPropertyType) * (localPropertyCount - 1), sizeof(SharedPropertyType) * localPropertyCount));
    SizeType index = localPropertyCount - 1;
    new (&localProperties[index]) SharedPropertyType(key, schema, index, push);
    localProperties[index].Update(v);
  }
  void AddOtherProperty(SchemaType* schema, SharedPropertyType* link,
			const PointerType& path) {
    otherPropertyCount++;
    propertyCount++;
    otherProperties = static_cast<SharedPropertyType**>(schema->allocator_->Realloc(otherProperties, sizeof(SharedPropertyType*) * (otherPropertyCount - 1), sizeof(SharedPropertyType*) * otherPropertyCount));
    otherProperties[otherPropertyCount - 1] = link;
    link->currentInstance->AddLink(schema, otherPropertyCount - 1, path);
  }
  void SortSources(const SchemaType* root, const PointerType path) {
    if (localProperties) {
      for (SizeType i = 0; i < localPropertyCount; i++)
	localProperties[i].SortSources(root, path);
    }
  }
  bool isLocal(const SizeType& i) const {
    return (i < localPropertyCount);
  }
  SizeType relativeIndex(const SizeType& i) const {
    if (isLocal(i))
      return i;
    return i - localPropertyCount;
  }
  SizeType siblingCount(bool local, size_t index) const {
    SizeType N = (SizeType)index;
    if (localProperties && !local)
      N += localPropertyCount;
    return N;
  }
  SharedPropertyType* GetProperty(const SizeType& i) {
    if (i < localPropertyCount)
      return localProperties + i;
    else
      return otherProperties[i - localPropertyCount];
  }
  const SharedPropertyType* GetProperty(const SizeType& i) const {
    if (i < localPropertyCount)
      return localProperties + i;
    else
      return otherProperties[i - localPropertyCount];
  }
  void Display(bool* mask=nullptr) {
    for (SizeType i = 0; i < propertyCount; i++) {
      std::cerr << "    ";
      GetProperty(i)->Display();
      if (mask && !mask[i])
	std::cerr << " [masked]";
      std::cerr << std::endl;
    }
  }
  bool isSrc(const PointerType& schemaPtr,
	      const Ch* str, SizeType length) const {
    for (SizeType i = 0; i < propertyCount; i++)  {
      if (GetProperty(i)->isSrc(isLocal(i)) &&
	  GetProperty(i)->HasProperty(str, length) &&
	  GetProperty(i)->Matches(schemaPtr, true, false))
	return true;
    }
    return false;
  }
  bool isDst(const PointerType& schemaPtr,
	     const Ch* str, SizeType length) const {
    for (SizeType i = 0; i < propertyCount; i++)  {
      if (GetProperty(i)->isDst(isLocal(i)) &&
	  GetProperty(i)->HasProperty(str, length) &&
	  GetProperty(i)->Matches(schemaPtr, false, false))
	return true;
    }
    return false;
  }
  void SetCurrentPtr(const PointerType& x, bool checkInstance = false) {
    for (SizeType i = 0; i < propertyCount; i++)
      GetProperty(i)->SetCurrentPtr(x, checkInstance);
    for (SizeType i = 0; i < propertyCount; i++)
      GetProperty(i)->SetSiblingCurrentPtr(x, checkInstance);
  }
  void SetSiblingCurrentPtr(const PointerType& x, bool source,
			    bool local, size_t index,
			    bool checkInstance = false) {
    SizeType N = siblingCount(local, index);
    for (SizeType i = 0; i < N; i++) {
      SharedPropertyType* prop = GetProperty(i);
      if (prop->isSrc(isLocal(i)) == source) continue;
      prop->SetCurrentPtr(x, checkInstance);
    }
  }
  void AddObject(const PointerType& instancePtr,
		 const PointerType& schemaPtr,
		 const SValue& present,
		 NormalizedDocumentType& normalized) {
    SetCurrentPtr(instancePtr, true);
    for (SizeType i = 0; i < propertyCount; i++)
      GetProperty(i)->AddObject(instancePtr, schemaPtr,
				normalized, present, isLocal(i));
  }
  void AddMissingObject(const SValue& name,
			const PointerType& instancePtr,
			NormalizedDocumentType& normalized) {
    PointerType iP = instancePtr.Append(name.GetString(),
					name.GetStringLength());
    for (SizeType i = 0; i < propertyCount; i++)
      GetProperty(i)->AddMissingObject(iP, normalized, isLocal(i));
  }
  SizeType localPropertyCount;
  SizeType otherPropertyCount;
  SizeType propertyCount;
  SharedPropertyType* localProperties;
  SharedPropertyType** otherProperties;
};

#endif // RAPIDJSON_YGGDRASIL

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
    typedef typename SchemaType::PointerType PointerType;
    typedef SharedProperties<SchemaDocumentType> SharedPropertiesType;
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
	, normalized(),
	schemaPointerAbs(),
	valuePointer(),
	patternPropertiesPointers(0)
#endif //RAPIDJSON_YGGDRASIL
    {}

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
#ifdef RAPIDJSON_YGGDRASIL
	if (patternPropertiesPointers) {
	    for (SizeType i = 0; i < patternPropertiesSchemaCount; i++)
	        (patternPropertiesPointers + i)->~PointerType();
	    factory.FreeState(patternPropertiesPointers);
	}
#endif // RAPIDJSON_YGGDRASIL
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
    PointerType schemaPointerAbs;
    PointerType valuePointer;
    PointerType* patternPropertiesPointers;
#endif //RAPIDJSON_YGGDRASIL
};

#ifdef RAPIDJSON_YGGDRASIL

enum SingularFlag {
  kSingularNoFlags = 0,
  kSingularArray = 1,
  kSingularItem = 2,
  kSingularObject = 3,
  kSingularValue = 4
};

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
// }
template<typename ValueType, typename AllocatorType>
bool follow_aliases_(const ValueType& aliases, const ValueType& orig,
		     ValueType* dest, AllocatorType& allocator) {
  typename ValueType::ConstMemberIterator primary = aliases.FindMember(orig);
  if (primary == aliases.MemberEnd()) {
    dest->CopyFrom(orig, allocator, true);
    return true;
  }
  ValueType path(kArrayType);
  RAPIDJSON_ASSERT(orig.IsString());
  path.PushBack(ValueType(orig, allocator, true).Move(), allocator);
  RAPIDJSON_ASSERT(primary->value.IsString());
  while (aliases.HasMember(primary->value)) {
    for (typename ValueType::ConstValueIterator it = path.Begin(); it != path.End(); ++it) {
      if (primary->value == *it) {
	dest->CopyFrom(path, allocator, true);
	return false;
      }
    }
    path.PushBack(ValueType(primary->value, allocator, true).Move(), allocator);
    primary = aliases.FindMember(primary->value);
    RAPIDJSON_ASSERT(primary->value.IsString());
  }
  dest->CopyFrom(primary->value, allocator, true);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Temporary memory
template <typename NormalizedDocument>
class TemporaryMemory {
public:
  TemporaryMemory(NormalizedDocument* doc=0) : doc_(doc), mem_() {}
  ~TemporaryMemory() {
    if (doc_ && mem_)
      doc_->GetAllocator().Free(mem_);
  }
  void stealMemory() {
    RAPIDJSON_ASSERT(!mem_);
    mem_ = doc_->temporary_memory_;
    doc_->temporary_memory_ = nullptr;
  }
  NormalizedDocument* doc_;
  void* mem_;
};
  
///////////////////////////////////////////////////////////////////////////////
// GenericNormalizedDocument
  
template <typename SchemaDocumentType, typename StackAllocator = RAPIDJSON_DEFAULT_STACK_ALLOCATOR>
class GenericNormalizedDocument {
public:
  typedef Schema<SchemaDocumentType> SchemaType;
  typedef typename SchemaType::EncodingType EncodingType;
  typedef typename SchemaType::AllocatorType AllocatorType;
  typedef typename SchemaType::SValue SValue;
  typedef typename SchemaType::PointerType PointerType;
  typedef typename SchemaType::SharedProperty SharedPropertyType;
  typedef GenericValue<EncodingType, AllocatorType> ValueType;
  typedef GenericDocument<EncodingType, AllocatorType, StackAllocator> DocumentType;
  typedef SchemaValidationContext<SchemaDocumentType> Context;
  typedef typename EncodingType::Ch Ch;
  typedef typename ValueType::MemberIterator MemberIterator;
  typedef typename ValueType::ConstMemberIterator ConstMemberIterator;
  typedef typename ValueType::ValueIterator ValueIterator;
  typedef typename ValueType::ConstValueIterator ConstValueIterator;
  typedef StackAllocator StackAllocatorType;
  friend class TemporaryMemory<GenericNormalizedDocument<SchemaDocumentType, StackAllocator> >;
  typedef units::GenericUnits<EncodingType> UnitsType;
  GenericNormalizedDocument(AllocatorType* allocator = 0,
			    StackAllocator* stackAllocator = 0,
			    size_t stackCapacity = kDefaultStackCapacity) :
    document_(allocator, stackCapacity, stackAllocator), index_(0),
    extending_(false), appending_(false),
    extend_context_(nullptr), extend_schema_(nullptr),
    keyStack_(stackAllocator, stackCapacity),
    valueStack_(stackAllocator, stackCapacity),
    childStack_(stackAllocator, stackCapacity),
    modifiedStack_(stackAllocator, stackCapacity),
    singularStack_(stackAllocator, stackCapacity),
    sharedStack_(stackAllocator, stackCapacity),
    documentStack_(nullptr), tempSharedCount_(0), tempSharedStack_(0),
    aliases_(kObjectType), inSingular_(false),
    temporary_memory_(nullptr),
    extend_child_(nullptr), basePointer_(allocator), core_(0) {}
  GenericNormalizedDocument(GenericNormalizedDocument* parent,
			    unsigned& index, StackAllocator* stackAllocator = 0,
			    size_t stackCapacity = kDefaultStackCapacity) :
    document_(&parent->GetAllocator(), stackCapacity, stackAllocator),
    index_(index), extending_(false), appending_(false),
    extend_context_(nullptr), extend_schema_(nullptr),
    keyStack_(stackAllocator, stackCapacity),
    valueStack_(stackAllocator, stackCapacity),
    childStack_(stackAllocator, stackCapacity),
    modifiedStack_(stackAllocator, stackCapacity),
    singularStack_(stackAllocator, stackCapacity),
    sharedStack_(stackAllocator, stackCapacity),
    documentStack_(nullptr), tempSharedCount_(0), tempSharedStack_(0),
    aliases_(kObjectType), inSingular_(false),
    temporary_memory_(nullptr),
    extend_child_(nullptr),
    basePointer_(&parent->GetAllocator()),
    core_(parent->core_) {
    parent->AddChild(this);
    SizeType N = (SizeType)(parent->singularStack_.GetSize() / sizeof(ModificationEntry));
    ModificationEntry* it = parent->singularStack_.template Bottom<ModificationEntry>();
    for (SizeType i = 0; i < N; i++, it++)
      RecordSingular(it->before, it->after);
  }

  //! Destructor.
  ~GenericNormalizedDocument() {
    while (!keyStack_.Empty())
      PopKey();
    while (!valueStack_.Empty())
      PopValue();
    while (!childStack_.Empty())
      PopChild();
    while (!modifiedStack_.Empty())
      PopModified();
    while (!singularStack_.Empty())
      PopSingular();
    while (!sharedStack_.Empty())
      sharedStack_.template Pop<PairEntry>(1)->~PairEntry();
    document_.ClearStack();
    if (temporary_memory_) {
      GetAllocator().Free(temporary_memory_);
      temporary_memory_ = nullptr;
    }
  }

  struct KeyEntry {
    ValueType* key;
    SizeType* idx;
    ValueType* aliased;
  };
  struct ValueEntry {
    ValueEntry(ValueType& value) :
      val(&value), ptr(), modified(false), child_modified(false),
      singular(false), child_singular(false) {}
    ValueEntry(ValueType& value, PointerType& p, AllocatorType* allocator = 0) :
      val(&value), ptr(p, allocator), modified(false), child_modified(false),
      singular(false), child_singular(false) {}
    ValueType* val;
    PointerType ptr;
    bool modified;
    bool child_modified;
    bool singular;
    bool child_singular;
  };
  struct ModificationEntry {
    ModificationEntry(const PointerType& before0, const PointerType& after0,
		      AllocatorType& allocator,
		      bool no_before0=false, bool no_after0=false) :
      before(before0, &allocator), after(after0, &allocator),
      no_before(no_before0), no_after(no_after0) {}
    PointerType before;
    PointerType after;
    bool no_before;
    bool no_after;
  };
  enum ModificationFlag {
    kCheckModifiedNull,
    kCheckModifiedBefore,
    kCheckModifiedBoth,
    kCheckModifiedAfter
  };
  struct SharedValueEntry {
    SharedValueEntry(AllocatorType* allocator = 0) :
      instancePtr(allocator), schemaPtr(allocator),
      shared(kArrayType),
      properties(kArrayType),
      present(kArrayType),
      set(false), local(false), missing(false), multiple(false),
      parent(0) {}
    SharedValueEntry(PointerType pInstance,
		     PointerType pSchema,
		     const SValue& properties0,
		     AllocatorType& allocator) :
      instancePtr(pInstance, &allocator),
      schemaPtr(pSchema, &allocator),
      shared(kArrayType),
      properties(properties0, allocator),
      present(kArrayType),
      set(false), local(false), missing(false), multiple(false),
      parent(0) {}
    void Display(bool useInstance = false) const {
      if (!set) {
	std::cerr << "NULL (";
      }
      if (useInstance)
	DisplayPointer(instancePtr);
      else
	DisplayPointer(schemaPtr);
      std::cerr << " ";
      DisplayValue(properties);
      if (!set)
	std::cerr << ")";
    }
    void CopyFrom(const SharedValueEntry& rhs, AllocatorType& allocator) {
      instancePtr = rhs.instancePtr;
      schemaPtr = rhs.schemaPtr;
      properties.CopyFrom(rhs.properties, allocator);
      present.CopyFrom(rhs.present, allocator);
      set = rhs.set;
      local = rhs.local;
      missing = rhs.missing;
      multiple = rhs.multiple;
      parent = rhs.parent;
    }
    PointerType instancePtr;
    PointerType schemaPtr;
    SValue shared;
    // Missing properties for dsts & available properties for srcs
    SValue properties;
    SValue present;
    bool set;
    bool local;
    bool missing;
    bool multiple;
    typename SchemaType::SharedPropertyBase* parent;
  };
  struct PairEntry {
    PairEntry() :
      complete(false), properties(kArrayType),
      prefix(), src(), dst() {}
    PairEntry(const PointerType& prefix0,
	      AllocatorType* allocator) :
      complete(false), properties(kArrayType),
      prefix(prefix0, allocator),
      src(allocator), dst(allocator) {}
    PairEntry(const PairEntry& other,
	      AllocatorType* allocator) :
      complete(other.complete),
      properties(other.properties, *allocator),
      prefix(other.prefix, allocator),
      src(allocator), dst(allocator) {
      src.CopyFrom(other.src, *allocator);
      dst.CopyFrom(other.dst, *allocator);
    }
    void Display(bool useInstance = false) const {
      src.Display(useInstance);
      std::cerr << " -> ";
      dst.Display(useInstance);
    }
    bool missing() const { return dst.missing || src.missing; }
    bool set() const { return dst.set && src.set; }
    bool Completes(const PairEntry& other,
		   bool verbose = false) const {
      bool source = src.set;
      const SharedValueEntry* iVi = GetValue(source);
      const SharedValueEntry* iVo = GetValue(!source);
      const SharedValueEntry* oVi = other.GetValue(source);
      const SharedValueEntry* oVo = other.GetValue(!source);
      if (verbose) {
	std::cerr << "Completes: " << Matches(oVi->schemaPtr, source, false) << ", " << Matches(oVo->schemaPtr, !source, false) << ", " << Matches(oVi->instancePtr, source, true) << ", " << other.Matches(iVo->instancePtr, !source, true) << std::endl;
	std::cerr << "    First: ";
	DisplayPointer(prefix);
	std::cerr << std::endl;
	std::cerr << "      - " << iVi->local << ", " << iVi->multiple << std::endl;
	std::cerr << "        ";
	iVi->Display();
	std::cerr << std::endl;
	std::cerr << "        ";
	iVi->Display(true);
	std::cerr << std::endl;
	std::cerr << "      - " << iVo->local << ", " << iVo->multiple << std::endl;
	std::cerr << "        ";
	iVo->Display();
	std::cerr << std::endl;
	std::cerr << "        ";
	iVo->Display(true);
	std::cerr << std::endl;
	std::cerr << "    Second: ";
	DisplayPointer(other.prefix);
	std::cerr << std::endl;
	std::cerr << "      - " << oVi->local << ", " << oVi->multiple << std::endl;
	std::cerr << "        ";
	oVi->Display();
	std::cerr << std::endl;
	std::cerr << "        ";
	oVi->Display(true);
	std::cerr << std::endl;
	std::cerr << "      - " << oVo->local << ", " << oVo->multiple << std::endl;
	std::cerr << "        ";
	oVo->Display();
	std::cerr << std::endl;
	std::cerr << "        ";
	oVo->Display(true);
	std::cerr << std::endl;
      }
      if (!iVi->set || iVo->set || !oVo->set || oVi->set ||
	  prefix != other.prefix ||
	  ((iVi->local || !oVi->multiple)
	   && !Matches(oVi->schemaPtr, source, false)) ||
	  ((iVo->local || !iVo->multiple)
	   && !Matches(oVo->schemaPtr, !source, false)) ||
	  !Matches(oVi->instancePtr, source, true) ||
	  !other.Matches(iVo->instancePtr, !source, true))
	// TODO: when multiple, check schema prefix?
	// (iVi->multiple && !oVi->schemaPtr.PartialCompare(iVi->schemaPtr)) ||
	// (iVo->multiple && !iVo->schemaPtr.PartialCompare(oVo->schemaPtr)))
	return false;
      return true;
    }
    bool HasMember(const SValue& name) const {
      if (!src.set) return false;
      return src.properties.Contains(name);
    }
    bool HasMissing(const SValue& name) const {
      if (!dst.set) return false;
      return dst.properties.Contains(name);
    }
    const SharedValueEntry* GetValue(bool source) const {
      if (source)
	return &src;
      else
	return &dst;
    }
    SharedValueEntry* GetValue(bool source) {
      if (source)
	return &src;
      else
	return &dst;
    }
    bool Matches(const PointerType& ptr, bool source,
		 bool checkInstance = false) const {
      const SharedValueEntry* it = GetValue(source);
      if (!it) return false;
      if (checkInstance) {
	if (!it->set) {
	  typename SchemaType::SharedPropertyBase* parent = it->parent;
	  if (!parent)
	    parent = GetValue(!source)->parent;
	  if (!(parent && parent->schema)) {
	    std::cerr << "Matches: ";
	    Display();
	    std::cerr << std::endl;
	  }
	  RAPIDJSON_ASSERT(parent && parent->schema);
	  return parent->schema->PointerMatches(it->instancePtr, ptr,
						parent->hasRegex);
	}
	return it->instancePtr == ptr;
      }
      return it->schemaPtr == ptr;
    }
    void SetProperties(typename SchemaType::SharedPropertyBase* property,
		       AllocatorType* allocator) {
      if (property && properties.Size() == 0) {
	for (SizeType i = 0; i < property->propertyCount; i++)
	  properties.PushBack(SValue(property->properties[i].name,
				     *allocator, true).Move(),
			      *allocator);
      }
      bool setSrc = src.set && src.properties.Size() == 0;
      bool setDst = dst.set && dst.properties.Size() == 0;
      if (!(setSrc || setDst)) return;
      for (typename SValue::ConstValueIterator name = properties.Begin();
	   name != properties.End(); ++name) {
	if (setSrc && src.present.Contains(*name))
	  src.properties.PushBack(SValue(name->GetString(),
					 name->GetStringLength(),
					 *allocator).Move(),
				  *allocator);
	if (setDst && !dst.present.Contains(*name))
	  dst.properties.PushBack(SValue(name->GetString(),
					 name->GetStringLength(),
					 *allocator).Move(),
				  *allocator);
	
      }
    }
    void Template(bool source,
		  typename SchemaType::SharedPropertyBase* property,
		  AllocatorType* allocator,
		  bool multiple = false) {
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      std::cerr << "Template (";
      if (!source)
	std::cerr << "dst";
      else
	std::cerr << "src";
      std::cerr << "): ";
      DisplayPointer(property->instancePtr);
      std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      SharedValueEntry* it = 0;
      SharedValueEntry* ot = 0;
      if (source) {
	it = &src;
	ot = &dst;
      } else {
	it = &dst;
	ot = &src;
      }
      if (!ot->missing) {
	RAPIDJSON_ASSERT(!it->set && ot->set);
	if (it->set || !ot->set) return;
      }
      it->instancePtr = prefix;
      for (size_t i = prefix.GetTokenCount();
	   i < property->instancePtr.GetTokenCount(); i++)
	it->instancePtr = it->instancePtr.Append(property->instancePtr.GetTokens()[i],
						 allocator);
      if (!multiple) {
	it->schemaPtr = property->schemaPtr;
	it->parent = property;
      }
      it->local = !ot->local;
      it->multiple = multiple;
      if (!source)
	SetProperties(it->parent, allocator);
    }
    void Missing(bool source, const PointerType& instancePtr,
		 typename SchemaType::SharedPropertyBase* property,
		 AllocatorType* allocator,
		 bool multiple = false) {
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      std::cerr << "Missing (";
      if (!source)
	std::cerr << "dst";
      else
	std::cerr << "src";
      std::cerr << "): ";
      DisplayPointer(property->instancePtr);
      std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      SharedValueEntry* it = 0;
      SharedValueEntry* ot = 0;
      if (source) {
	it = &src;
	ot = &dst;
      } else {
	it = &dst;
	ot = &src;
      }
      RAPIDJSON_ASSERT(!it->set && !ot->missing && !ot->set);
      if (it->set || ot->missing || ot->set) return;
      it->missing = true;
      it->instancePtr = instancePtr;
      if (!multiple) {
	it->schemaPtr = property->schemaPtr;
	it->parent = property;
      }
      it->local = property->local;
      it->multiple = multiple;
      // Push: wait for destination to set the properties for the pair
      if (source && it->local && !dst.set)
	return;
      SetProperties(it->parent, allocator);
    }
    void Complete(bool source, const PairEntry& other,
		  AllocatorType* allocator) {
      GetValue(source)->CopyFrom(*other.GetValue(source), *allocator);
      if (!source)
	SetProperties(dst.parent, allocator);
    }
    void Complete(bool source,
		  const PointerType& instancePtr,
		  const PointerType& schemaPtr,
		  const SValue& present,
		  AllocatorType* allocator,
		  typename SchemaType::SharedPropertyBase* property = 0) {
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      std::cerr << "AddObject (";
      if (!source)
	std::cerr << "dst";
      else
	std::cerr << "src";
      std::cerr << "): ";
      DisplayPointer(instancePtr);
      std::cerr << " ";
      DisplayValue(present);
      std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      SharedValueEntry* it = 0;
      SharedValueEntry* ot = 0;
      if (source) {
	it = &src;
	ot = &dst;
      } else {
	it = &dst;
	ot = &src;
      }
      if (it->set) {
	if (it->instancePtr == instancePtr && it->schemaPtr == schemaPtr)
	  return;
	std::cerr << "PAIR ENTRY CONFLICT: [";
	DisplayPointer(it->instancePtr);
	std::cerr << ", ";
	DisplayPointer(it->schemaPtr);
	std::cerr << "] vs [";
	DisplayPointer(instancePtr);
	std::cerr << ", ";
	DisplayPointer(schemaPtr);
	std::cerr << "], prefix: ";
	DisplayPointer(prefix);
	std::cerr << std::endl;
	return;
      }
      it->instancePtr = instancePtr;
      it->schemaPtr = schemaPtr;
      if (property)
	it->parent = property;
      else
	RAPIDJSON_ASSERT(it->parent);
      it->present.CopyFrom(present, *allocator, true);
      it->set = true;
      if (ot->set)
	it->local = !ot->local;
      else {
	it->local = it->parent->local;
	ot->local = !it->local;
      }
      // Push: wait for destination to set the properties for the pair
      if (source && it->local && !dst.set)
	return;
      SetProperties(it->parent, allocator);
      return;
    }
    bool HasUnvisitedProperty(const SValue& name, bool source) const {
      if ((source && !src.set && dst.set && dst.properties.Contains(name)) ||
	  (!source && !dst.set && src.set && src.properties.Contains(name)) ||
	  (!src.set && !dst.set && properties.Contains(name)))
	return true;
      return false;
    }
    bool IsFinalized(const SValue& name, bool source,
		     const GenericNormalizedDocument& normalized) const {
      if (!source) {
	// Sibling srcs/dsts that assign to the same destination
	if (!dst.set) return false;
	if (!normalized.IsFinalizedShared(this, name, source))
	  return false;
      }
      // sibling srcs/dsts that assign to the same source
      if (!src.set) return !properties.Contains(name);
      return normalized.IsFinalizedShared(this, name, true);
    }
    ValueType* GetParent(GenericNormalizedDocument& normalized,
			 bool source) {
      SharedValueEntry* it = GetValue(source);
      if (!it) return 0;
      return normalized.Get(it->instancePtr);
    }
    ValueType* GetMember(const SValue& name,
			 GenericNormalizedDocument& normalized) {
      ValueType* parent = 0;
      return GetMember(name, normalized, parent);
    }
    ValueType* GetMember(const SValue& name,
			 GenericNormalizedDocument& normalized,
			 ValueType*& parent) {
      RAPIDJSON_ASSERT(src.set);
      if (!src.set) return 0;
      RAPIDJSON_ASSERT(dst.parent);
      if (!dst.parent) return 0;
      typename SchemaType::SharedProperty::PropertyEntry* prop = dst.parent->FindProperty(name);
      RAPIDJSON_ASSERT(prop);
      if (!prop) return 0;
      if (!parent)
	parent = GetParent(normalized, true);
      RAPIDJSON_ASSERT(parent);
      if (!(parent && parent->HasMember(name))) {
	std::cerr << "GetMember [" << name.GetString() << "]: ";
	Display();
	std::cerr << std::endl;
      }
      RAPIDJSON_ASSERT(parent && parent->HasMember(name));
      if (parent && parent->HasMember(name)) {
	if (!prop->inSource)
	  src.shared.PushBack(SValue(name.GetString(),
				     name.GetStringLength(),
				     normalized.GetAllocator()).Move(),
			      normalized.GetAllocator());
	return &(parent->FindMember(name)->value);
      }
      return 0;
    }
    bool SetMember(Context& context, const SValue& name,
		   const ValueType* value,
		   GenericNormalizedDocument& normalized) {
      ValueType* parent = 0;
      return SetMember(context, name, value, normalized, parent);
    }
    bool SetMember(Context& context,
		   const SValue& name,
		   const ValueType* value,
		   GenericNormalizedDocument& normalized,
		   ValueType*& parent) {
      ValueType value0(kNullType);
      RAPIDJSON_ASSERT(dst.set);
      if (!dst.set) return false;
      if (value) {
	if (!parent)
	  parent = GetParent(normalized, false);
	RAPIDJSON_ASSERT(parent && dst.parent);
	if (parent && !parent->HasMember(name) && dst.parent) {
	  typename SchemaType::SharedProperty::PropertyEntry* prop = dst.parent->FindProperty(name);
	  RAPIDJSON_ASSERT(prop);
	  if (prop) {
	    if (normalized.NormalizeShared(context, *value, value0,
					   dst.instancePtr.Append(name.GetString(), name.GetStringLength(), &normalized.GetAllocator()),
					   dst.schemaPtr.Append(SchemaType::GetPropertiesString(), &normalized.GetAllocator()).Append(name.GetString(), name.GetStringLength(), &normalized.GetAllocator()),
					   *prop->base->schema))
	      value = &value0;
	    else {
	      std::cerr << "SetMember: Error in normalizing new value for \"" << name.GetString() << "\"" << std::endl;
	      return false;
	    }
	  }
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	  std::cerr << "SETTING [" << name.GetString() << "]: ";
	  DisplayPointer(dst.instancePtr);
	  std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	  parent->AddMember(ValueType(name.GetString(),
				      name.GetStringLength(),
				      normalized.GetAllocator()).Move(),
			    ValueType(*value,
				      normalized.GetAllocator(), true).Move(),
			    normalized.GetAllocator());
	  normalized.RecordModified(dst.instancePtr,
				    dst.instancePtr.Append(name.GetString(),
							   name.GetStringLength(),
							   &normalized.GetAllocator()),
				    true, false, "shared");
	}
      }
      if (!normalized.SetSharedSiblings(context, this, name, value))
	return false;
      RemoveMember(name);
      return true;
    }
    void RemoveMember(const SValue& name) {
      typename SValue::ConstValueIterator it = dst.properties.Index(name);
      if (it != dst.properties.End())
	dst.properties.Erase(it);
    }
    bool AssignMissing(Context& context,
		       GenericNormalizedDocument& normalized) {
      if (!src.set || !dst.set) return true;
      ValueType* srcParent = 0;
      ValueType* dstParent = 0;
      SValue copy(dst.properties, normalized.GetAllocator());
      for (SizeType i = 0; i < copy.Size(); i++) {
	if (normalized.HasUnvisitedSharedSiblings(this, copy[i], true))
	  continue;
	if (!HasMember(copy[i])) {
	  if (!normalized.SetSharedSiblings(context, this, copy[i], 0))
	    return false;
	  continue;
	}
	ValueType* val = GetMember(copy[i], normalized, srcParent);
	RAPIDJSON_ASSERT(val);
	if (!val) continue;
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	std::cerr << "AssignMissing [";
	DisplayPointer(src.instancePtr);
	std::cerr << " -> ";
	DisplayPointer(dst.instancePtr);
	std::cerr << "]: " << copy[i].GetString() << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	if (!SetMember(context, copy[i], val, normalized, dstParent)) {
	  typename SValue::ConstValueIterator sh = src.shared.Index(copy[i]);
	  if (sh != src.shared.End())
	    src.shared.Erase(sh);
	  return false;
	}
      }
      return true;
    }
    bool AssignDefaults(Context& context,
			GenericNormalizedDocument& normalized) {
      if (!src.set || !dst.set) return true;
      ValueType* dstParent = 0;
      SValue copy(dst.properties, normalized.GetAllocator());
      for (SizeType i = 0; i < copy.Size(); i++) {
	if (!IsFinalized(copy[i], false, normalized)) continue;
	RAPIDJSON_ASSERT(dst.parent);
	if (!dst.parent) continue;
	typename SchemaType::SharedProperty::PropertyEntry* prop = dst.parent->FindProperty(copy[i]);
	RAPIDJSON_ASSERT(prop);
	if (!prop) continue;
	if (!prop->base->required) {
	  RemoveMember(copy[i]);
	  continue;
	}
	const ValueType* val = prop->GetDefault();
	if (!val) continue;
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	std::cerr << "AssignDefaults [";
	DisplayPointer(dst.instancePtr);
	std::cerr << "]: " << copy[i].GetString() << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	if (!SetMember(context, copy[i], val, normalized, dstParent))
	  return false;
      }
      return true;
    }
    void RecordMissing(GenericNormalizedDocument& normalized,
		       ValueType& missing) {
      if (!src.set || !dst.set || dst.properties.Size() == 0) return;
      ValueType* parent = 0;
      GenericStringBuffer<EncodingType> sb;
      SValue copy(dst.properties, normalized.GetAllocator());
      for (SizeType i = 0; i < copy.Size(); i++) {
	if (!IsFinalized(copy[i], false, normalized)) continue;
	if (!parent)
	  parent = GetParent(normalized, false);
	RAPIDJSON_ASSERT(parent && dst.parent);
	if (!parent || parent->HasMember(copy[i]) || !dst.parent) continue;
	typename SchemaType::SharedProperty::PropertyEntry* prop = dst.parent->FindProperty(copy[i]);
	RAPIDJSON_ASSERT(prop);
	if (!prop) continue;
	if (!prop->base->required) {
	  RemoveMember(copy[i]);
	  continue;
	}
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	std::cerr << "RecordMissing: " << copy[i].GetString() << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	dst.instancePtr.StringifyUriFragment(sb);
	ValueType instanceRef(sb.GetString(),
				  (SizeType)sb.GetLength(),
				  normalized.GetAllocator());
	sb.Clear();
	dst.schemaPtr.StringifyUriFragment(sb);
	const ValueType schemaRef(sb.GetString(),
				      (SizeType)sb.GetLength(),
				      normalized.GetAllocator());
	sb.Clear();
	if (!missing.HasMember(schemaRef))
	  missing.AddMember(ValueType(schemaRef,
					  normalized.GetAllocator(),
					  true).Move(),
			    ValueType(kArrayType).Move(),
			    normalized.GetAllocator());
	ValueType val(kArrayType);
	val.PushBack(instanceRef, normalized.GetAllocator());
	val.PushBack(ValueType(copy[i].GetString(),
				   copy[i].GetStringLength(),
				   normalized.GetAllocator()).Move(),
		     normalized.GetAllocator());
	missing[schemaRef].PushBack(val, normalized.GetAllocator());
      }
    }
    void RemoveShared(GenericNormalizedDocument& normalized) {
      if (!src.set || !dst.set || src.shared.Size() == 0) return;
      ValueType* parent = 0;
      SValue copy(src.shared, normalized.GetAllocator());
      for (SizeType i = 0; i < copy.Size(); i++) {
	if (!IsFinalized(copy[i], false, normalized) ||
	    normalized.HasUnvisitedSharedSiblings(this, copy[i], false))
	  continue;
	if (!parent)
	  parent = GetParent(normalized, true);
	RAPIDJSON_ASSERT(parent);
	if (!parent || !parent->HasMember(copy[i])) continue;
	parent->RemoveMember(copy[i]);
	typename SValue::ConstValueIterator it = src.properties.Index(copy[i]);
	if (it != src.properties.End())
	  src.properties.Erase(it);
	src.shared.Erase(src.shared.Index(copy[i]));
      }
    }
    bool complete;
    SValue properties;
    PointerType prefix;
    SharedValueEntry src;
    SharedValueEntry dst;
  };
  
  //! Get the allocator of this document.
  AllocatorType& GetAllocator() {
    return document_.GetAllocator();
  }
  
  void AddChild(GenericNormalizedDocument* child) {
    GenericNormalizedDocument** ref = childStack_.template Push<GenericNormalizedDocument*>();
    ref[0] = child;
  }
  void PopChild() {
    childStack_.template Pop<GenericNormalizedDocument*>(1);
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
  bool WasModified() const { return (modifiedStack_.GetSize() > 0); }
  //! Finalize the document from elements added to the stack.
  void FinalizeFromStack(bool afterError = false) {
    if (afterError)
      document_.ConsolidateStack();
    document_.FinalizeFromStack();
  }
  //! Determine if the document was finalized.
  bool WasFinalized() const { return document_.WasFinalized(); }

  bool ExtendChild(Context& context, const SchemaType& schema, unsigned index) {
    GenericNormalizedDocument* child = FindChild(index);
    RAPIDJSON_ASSERT(child);
    if (!child)
      RAPIDJSON_YGGDRASIL_GENERIC_ERROR("Could not find child: %d",
					(int)index);
    child->FinalizeFromStack();
    bool replaced = false;
    if (!child->ExtendAliases(context, aliases_, &replaced)) return false;
    if (!ExtendAliases(context, child->aliases_, &replaced)) return false;
    if (!ExtendShared(context, schema, child->sharedStack_)) return false;
    if (!(replaced || child->WasModified()))
      return true;
    extend_child_ = child;
    return Extend(context, schema, child->document_);
  }
  void AddSharedTemp(const internal::Stack<StackAllocatorType>& childShared) {
    // Pairs created in a normalization object for a set value need to be
    //   cached and added later to avoid modifiying the pair stack during
    //   iteration.
    tempSharedStack_ = static_cast<PairEntry*>(GetAllocator().Realloc(tempSharedStack_, sizeof(PairEntry) * tempSharedCount_, sizeof(PairEntry) * tempSharedCount_ + childShared.GetSize()));
    memcpy((char*)(tempSharedStack_ + tempSharedCount_),
	   (char*)(childShared.template Bottom<PairEntry>()),
	   childShared.GetSize());
    tempSharedCount_ += childShared.GetSize() / sizeof(PairEntry);
  }
  bool ExtendSharedTemp(Context& context, const SchemaType& schema,
			bool runDefaults = false) {
    if (!tempSharedStack_) return true;
    bool out = ExtendShared(context, schema,
			    tempSharedStack_, tempSharedCount_,
			    true);
    for (SizeType i = 0; i < tempSharedCount_; i++)
      (tempSharedStack_ + i)->~PairEntry();
    AllocatorType::Free(tempSharedStack_);
    tempSharedStack_ = 0;
    tempSharedCount_ = 0;
    if (!out) return out;
    for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++)
      if (!it->AssignMissing(context, *this)) return false;
    if (!ExtendSharedTemp(context, schema)) return false;
    if (runDefaults) {
      for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	   it != sharedStack_.template End<PairEntry>(); it++)
	if (!it->AssignDefaults(context, *this)) return false;
      if (!ExtendSharedTemp(context, schema, true)) return false;
    }
    return out;
  }
  bool ExtendShared(Context& context, const SchemaType& schema,
		    const internal::Stack<StackAllocatorType>& childShared) {
    return ExtendShared(context, schema,
			childShared.template Bottom<PairEntry>(),
			childShared.GetSize() / sizeof(PairEntry));
  }
  bool ExtendShared(Context& context, const SchemaType& schema,
		    const PairEntry* childShared, size_t childSharedCount,
		    bool skipCheck = false, bool verbose = false) {
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
    std::cerr << "ExtendShared Child: " << std::endl;
    for (SizeType i = 0; i < childSharedCount; i++) {
      std::cerr << "    ";
      (childShared + i)->Display(true);
      std::cerr << std::endl;
    }
    std::cerr << std::endl;
    std::cerr << "ExtendShared Before: " << std::endl;
    DisplayShared(false);
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
    size_t NStack = sharedStack_.GetSize() / sizeof(PairEntry);
    for (size_t i = 0; i < childSharedCount; i++) {
      const PairEntry* ot = childShared + i;
      bool match = false;
      if ((ot->src.set && !ot->dst.set) ||
	  (!ot->src.set && ot->dst.set)) {
	bool source = ot->src.set;
	size_t iStack = 0;
	for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	     it != sharedStack_.template End<PairEntry>(); it++, iStack++) {
	  if (iStack >= NStack) break;
	  if (!ot->Completes(*it, verbose)) continue;
	  if (it->GetValue(source)->multiple) {
	    PairEntry* pair = sharedStack_.template Push<PairEntry>();
	    new (pair) PairEntry(*it, &GetAllocator());
	    pair->Complete(source, *ot, &GetAllocator());
	  } else {
	    it->Complete(source, *ot, &GetAllocator());
	  }
	  match = true;
	}
      }
      if (!match) {
	PairEntry* pair = sharedStack_.template Push<PairEntry>();
	new (pair) PairEntry(*ot, &GetAllocator());
      }
    }
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
    std::cerr << "ExtendShared After: " << std::endl;
    DisplayShared(false);
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
    if (!skipCheck)
      return CheckSharedPairs(context, schema);
    return true;
  }
  bool Append(Context& context, const SchemaType& schema, const ValueType& document) {
    appending_ = true;
    bool out = Extend(context, schema, document);
    appending_ = false;
    return out;
  }

  bool Extend(Context& context, const SchemaType& schema,
	      const ValueType& document) {
    RAPIDJSON_ASSERT(!extending_);
    RAPIDJSON_ASSERT(!extend_context_);
    RAPIDJSON_ASSERT(!extend_schema_);
    RAPIDJSON_ASSERT(!document_.WasFinalized());
    RAPIDJSON_ASSERT(document_.StackSize() > 0);
    if (extending_ || extend_context_ || extend_schema_ ||
	document_.WasFinalized() || (document_.StackSize() == 0))
      RAPIDJSON_YGGDRASIL_GENERIC_ERROR("Something is wrong with the state of"
					" the normalized document at the "
					" start of an extend call.");
    PushValue(*document_.StackTop());
    extending_ = true;
    extend_context_ = &context;
    extend_schema_ = &schema;
    bool out = document.Accept(*this);
    if (!out)
      return false;
    extending_ = false;
    extend_context_ = nullptr;
    extend_schema_ = nullptr;
    PopValue();
    return out;
  }

#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
#define DEBUG_STEP							\
  std::cerr << __debug_section << " " << __debug_section_mark <<	\
    "[" << __debug_section_idx << "]: " <<				\
    __debug_section_value << " = " << out << " (";			\
  DisplayCurrentPointer();						\
  if (__debug_section_schema) {						\
    std::cerr << ", ";							\
    DisplayPointer(GetSchemaPointer(*__debug_section_schema));		\
    std::cerr << ", " << __debug_section_schema->isSingular_;		\
  }									\
  std::cerr << ")" << std::endl;					\
  __debug_section_idx++
#define DEBUG_INIT(section, mark, value, schema)	\
  int __debug_section_idx = 0;				\
  std::string __debug_section = #section;		\
  std::string __debug_section_mark = #mark;		\
  std::string __debug_section_value = value;		\
  const SchemaType* __debug_section_schema = schema;	\
  DEBUG_STEP
#define DEBUG_MOD(section, mark, value)		\
  __debug_section_idx = 0;			\
  __debug_section = #section;			\
  __debug_section_mark = #mark;			\
  __debug_section_value = value;		\
  DEBUG_STEP
#else // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
#define DEBUG_STEP
#define DEBUG_INIT(section, mark, value, schema)
#define DEBUG_MOD(section, mark, value)
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
#define INIT_CHECK(section, mark, value, schema)	\
  bool out = true;					\
  DEBUG_INIT(section, mark, value, schema)
#define MOD_CHECK(section, mark, value)		\
  DEBUG_MOD(section, mark, value)
#define CHECK_RESULT				\
  DEBUG_STEP;					\
  if (!out) return false

  // Methods for normalizing values
  bool BeginNorm(Context& context, const SchemaType& schema) {
    if ((schema.isSingular_ == kSingularItem) && (ToggleSingular())) {
      RecordModifiedSingular((SizeType)0);
      return NormStartArray(context, schema);
    } else if ((schema.isSingular_ == kSingularValue) && (ToggleSingular())) {
      RecordModifiedSingular(schema.parentKey_);
      if (!NormStartObject(context, schema)) return false;
      RAPIDJSON_ASSERT(schema.parentKey_.IsString());
      return NormKey(context, schema, schema.parentKey_.GetString(),
		     schema.parentKey_.GetStringLength(), true);
    }
    return true;
  }
  bool EndNorm(Context& context, const SchemaType& schema) {
    if ((schema.isSingular_ == kSingularItem) && (ToggleSingular())) {
      return NormEndArray(context, schema, 1);
    } else if ((schema.isSingular_ == kSingularValue) && (ToggleSingular())) {
      // TODO: Missing after defaults from parentSchema set?
      SizeType memberCount = 1;
      RAPIDJSON_ASSERT(schema.parentSchema_ &&
		       schema.parentSchema_->allowSingularSchema_.schemas);
      if (!(schema.parentSchema_ &&
	    schema.parentSchema_->allowSingularSchema_.schemas))
	return false;
      return NormEndObject(context, schema, memberCount,
			   schema.parentSchema_->allowSingularSchema_.schemas[0]);
    }
    return true;
  }

#define NORM_BEGIN_STUB_(method)		\
  INIT_CHECK(Norm, stub, #method, &schema)
#define NORM_BEGIN_(method)			\
  INIT_CHECK(Norm, begin, #method, &schema);	\
  out = BeginNorm(context, schema);		\
  CHECK_RESULT
#define NORM_BODY_(method, args)		\
  MOD_CHECK(Norm, body, #method);		\
  out = document_.method args;			\
  CHECK_RESULT
#define NORM_END_(method)			\
  MOD_CHECK(Norm, end, #method);		\
  out = EndNorm(context, schema);		\
  CHECK_RESULT;					\
  return out
#define NORM_VALUE_(method, args)		\
  NORM_BEGIN_(method);				\
  NORM_BODY_(method, args);			\
  NORM_END_(method)

  bool NormNull(Context& context, const SchemaType& schema)
  { NORM_VALUE_(Null, ()); }
  bool NormBool(Context& context, const SchemaType& schema, bool b)
  { NORM_VALUE_(Bool, (b)); }
  bool NormInt(Context& context, const SchemaType& schema, int i)
  { NORM_VALUE_(Int, (i)); }
  bool NormUint(Context& context, const SchemaType& schema, unsigned u)
  { NORM_VALUE_(Uint, (u)); }
  bool NormInt64(Context& context, const SchemaType& schema, int64_t i)
  { NORM_VALUE_(Int64, (i)); }
  bool NormUint64(Context& context, const SchemaType& schema, uint64_t u)
  { NORM_VALUE_(Uint64, (u)); }
  bool NormDouble(Context& context, const SchemaType& schema, double d)
  { NORM_VALUE_(Double, (d)); }
  bool NormString(Context& context, const SchemaType& schema, const Ch* str, SizeType length, bool)
  { NORM_VALUE_(String, (str, length, true)); }
  template <typename YggSchemaValueType>
  bool NormYggdrasilString(Context& context, const SchemaType& schema, const Ch* str, SizeType length, bool copy, YggSchemaValueType& valueSchema) {
    NORM_BEGIN_(YggdrasilString);
    // Units
    typename YggSchemaValueType::ConstMemberIterator typeV = valueSchema.FindMember(SchemaType::GetTypeString());
    typename YggSchemaValueType::ConstMemberIterator subtypeV = valueSchema.FindMember(SchemaType::GetSubTypeString());
    typename YggSchemaValueType::ConstMemberIterator precisionV = valueSchema.FindMember(SchemaType::GetPrecisionString());
    typename YggSchemaValueType::ConstMemberIterator unitsV = valueSchema.FindMember(SchemaType::GetUnitsString());
    typename YggSchemaValueType::ConstMemberIterator shapeV = valueSchema.FindMember(SchemaType::GetShapeString());
    typename YggSchemaValueType::ConstMemberIterator lengthV = valueSchema.FindMember(SchemaType::GetLengthString());
    if ((typeV != valueSchema.MemberEnd()) &&
	(subtypeV != valueSchema.MemberEnd()) &&
	(precisionV != valueSchema.MemberEnd())) {
      typename SchemaType::YggSchemaValueSubType subtype = schema.GetSubType(typename SchemaType::ValueType(subtypeV->value.GetString(), subtypeV->value.GetStringLength()).Move());
      SizeType precision = (SizeType)(precisionV->value.GetUint64());
      SizeType nelements = 0;
      if (typeV->value == YggSchemaValueType::GetScalarString())
	nelements = 1;
      else if (lengthV != valueSchema.MemberEnd())
	nelements = (SizeType)(lengthV->value.GetUint64());
      else if (shapeV != valueSchema.MemberEnd()) {
	nelements = 1;
	for (typename YggSchemaValueType::ConstValueIterator v = shapeV->value.Begin(); v != shapeV->value.End(); ++v)
	  nelements *= static_cast<SizeType>(v->GetUint64());
      }
      // Subtype & precision
      YggSubType src_subtype = (YggSubType)subtype;
      YggSubType dst_subtype = (YggSubType)schema.subtype_;
      SizeType src_precision = precision;
      SizeType dst_precision = schema.precision_.GetUint();
      if (((src_subtype == dst_subtype) &&
	   (src_precision < dst_precision)) ||
	  ((src_subtype != dst_subtype) &&
	   canCast(src_subtype, src_precision, dst_subtype, dst_precision, false))) {
	SizeType src_size = sizeOfSubtype(src_subtype, src_precision);
	SizeType dst_size = sizeOfSubtype(dst_subtype, dst_precision);
	SizeType src_nbytes = length * (SizeType)sizeof(Ch);
	SizeType dst_nbytes = src_nbytes;
	RAPIDJSON_ASSERT(src_nbytes == (nelements * src_size));
	unsigned char* dst = (unsigned char*)(&(str[0]));
	if (dst_size > src_size) {
	  dst_nbytes = nelements * dst_size;
	  dst = (unsigned char*)SetTemporary(dst_nbytes);
	}
	changePrecision(src_subtype, src_precision,
			(const unsigned char*)str, src_nbytes,
			dst_subtype, dst_precision,
			dst, dst_nbytes, nelements);
	if (dst_size > src_size) {
	  str = (Ch*)dst;
	  length = dst_nbytes / (SizeType)sizeof(Ch);
	}
	subtype = schema.subtype_;
	precision = schema.precision_.GetUint();
	const typename SchemaType::ValueType& subtype_str = schema.SubType2String(schema.subtype_);
	valueSchema[SchemaType::GetSubTypeString()].SetString(subtype_str.GetString(), subtype_str.GetStringLength(), valueSchema.GetAllocator());
      }
      // Units
      if (unitsV != valueSchema.MemberEnd()) {
	if (schema.units_.IsString() && (unitsV->value != schema.units_.IsString())) {
	  UnitsType src_units(unitsV->value.GetString(),
			      unitsV->value.GetStringLength(),
			      false);
	  UnitsType dst_units(schema.units_.GetString(),
			      schema.units_.GetStringLength(),
			      false);
	  if ((src_units != dst_units) && (src_units.is_compatible(dst_units))) {
	    RecordModified(false, "units");
	    changeUnits((YggSubType)subtype, precision,
			(unsigned char*)str, src_units,
			(unsigned char*)(&(str[0])), dst_units,
			length * (SizeType)sizeof(Ch), nelements);
	    valueSchema[SchemaType::GetUnitsString()].SetString(schema.units_.GetString(),
								schema.units_.GetStringLength(),
								valueSchema.GetAllocator());
								
	  }
	}
      }
    }
    NORM_BODY_(YggdrasilString, (str, length, copy, valueSchema));
    NORM_END_(YggdrasilString);
  }
  template <typename YggSchemaValueType>
  bool NormYggdrasilStartObject(Context& context, const SchemaType& schema, YggSchemaValueType& valueSchema) {
    NORM_BEGIN_(YggdrasilStartObject);
    NORM_BODY_(YggdrasilStartObject, (valueSchema));
    return true;
  }
  bool NormYggdrasilEndObject(Context& context, const SchemaType& schema, SizeType memberCount) {
    NORM_BEGIN_STUB_(YggdrasilEndObject);
    NORM_BODY_(YggdrasilEndObject, (memberCount));
    NORM_END_(YggdrasilEndObject);
  }
  bool NormStartObject(Context& context, const SchemaType& schema) {
    NORM_BEGIN_(StartObject);
    NORM_BODY_(StartObject, ());
    return true;
  }
  bool AliasKey(Context& context, const Ch* str, SizeType len, bool,
		bool dont_check_aliases,
		ValueType& orig, ValueType& primary,
		const SchemaType* schema=nullptr) {
    bool exists = false;
    bool aliased = false;
    if (!dont_check_aliases) {
      const ValueType& aliases = AddAliases(schema);
      orig.SetString(str, len, GetAllocator());
      ConstMemberIterator match = aliases.MemberEnd();
      if (FindAliasName(aliases, orig, match)) {
	if (!GetFinalAlias(context, aliases, orig, &primary))
	  return false;
	RecordModifiedAlias(primary, orig);
	len = primary.GetStringLength();
	str = primary.GetString();
	aliased = true;
      } else if (FindAliasValue(aliases, orig, match)) {
	primary.CopyFrom(orig, GetAllocator(), true);
	orig.CopyFrom(match->name, GetAllocator(), true);
	if (extending_ && !appending_ && HasMember(orig))
	  RecordModifiedAlias(primary, orig);
      }
      // Check previous keys for alias target
      if (match != aliases.MemberEnd()) {
	if (HasMember(primary)) {
	  exists = true;
	  if ((!extending_) || appending_) {
	    // TODO: Check equivalence when the value is added?
	    context.error_handler.DuplicateAlias(orig, primary);
	    RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorAliasDuplicate);
	  }
	}
      }
    }
    if (!exists)
      orig.SetNull();
    if (!aliased)
      primary.SetNull();
    return true;
  }
  bool NormKey(Context& context, const SchemaType& schema, const Ch* str, SizeType len, bool copy, bool dont_check_aliases=false) {
    ValueType orig;
    ValueType primary;
    INIT_CHECK(Norm, key, (const char*)str, &schema);
    out = AliasKey(context, str, len, copy, dont_check_aliases,
		   orig, primary, &schema);
    CHECK_RESULT;
    if (primary.IsString()) {
      len = primary.GetStringLength();
      str = primary.GetString();
      copy = true;
    }
    NORM_BODY_(Key, (str, len, copy));
    return true;
  }
  bool NormEndObject(Context& context, const SchemaType& schema, SizeType& memberCount,
		     const SchemaType* baseSchema = nullptr) {
    NORM_BEGIN_STUB_(EndObject);
    // Default
    bool baseSchemaSet = (baseSchema != nullptr);
    if (!baseSchemaSet) baseSchema = &schema;
    PointerType iP = GetInstancePointer(false, true);
    PointerType iS = context.schemaPointerAbs;
    if (baseSchemaSet) {
      iP = iP.Pop(1, &GetAllocator());
      iS = iS.Pop((SizeType)(schema.pointer_.GetTokenCount() - baseSchema->pointer_.GetTokenCount()), &GetAllocator());
    }
    if ((!extending_) && (baseSchema->hasRequired_)) {
      for (SizeType index = 0; index < baseSchema->propertyCount_; index++) {
	if (!baseSchema->properties_[index].required ||
	    (!baseSchemaSet && context.propertyExist[index]) ||
	    (baseSchemaSet && schema.parentKey_ == baseSchema->properties_[index].name))
	  continue;
	if (const SValue* defV = baseSchema->properties_[index].schema->GetDefaultValue()) {
	  if (CheckSharedMissing(*baseSchema, iP, iS, index)) {
	    if (!baseSchemaSet)
	      context.propertyExist[index] = true;
	  } else {
	    RecordModifiedAdded(baseSchema->properties_[index].name, *defV);
	    const Ch* str = baseSchema->properties_[index].name.GetString();
	    SizeType len = baseSchema->properties_[index].name.GetStringLength();
	    if (!NormKey(context, *baseSchema, str, len, true))
	      return false;
	    if (!Append(context, *baseSchema, *defV))
	      return false;
	    if (!baseSchemaSet)
	      context.propertyExist[index] = true;
	    memberCount++;
	  }
	}
      }
    }
    NORM_BODY_(EndObject, (memberCount));
    out = FinalizeShared(context, *baseSchema, iP, iS);
    CHECK_RESULT;
    // Do EndObject
    NORM_END_(EndObject);
  }
  bool FinalizeShared(Context& context, const SchemaType& schema,
		      const PointerType& iP, const PointerType& iS) {
    if (!schema.sharedProperties_) return true;
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
    std::cerr << "FinalizedShared: ";
    DisplayPointer(iS);
    std::cerr << ", ";
    DisplayPointer(iP);
    std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
    ValueType* v = CurrentValue();
    SValue present(kArrayType);
    for (MemberIterator it = v->MemberBegin(); it != v->MemberEnd(); it++)
      present.PushBack(SValue(it->name, GetAllocator(), true).Move(),
		       GetAllocator());
    AddSharedObject(iP, iS, present);
    schema.sharedProperties_->AddObject(iP, iS, present, *this);
    return CheckSharedPairs(context, schema);
  }
  bool CheckSharedMissing(const SchemaType& schema,
			  const PointerType& instancePtr,
			  const PointerType& schemaPtr,
			  SizeType index) {
    // The property itself is shared
    if (schema.properties_[index].schema->sharedProperties_)
      schema.properties_[index].schema->sharedProperties_->AddMissingObject(schema.properties_[index].name, instancePtr, *this);
    return (schema.sharedProperties_ &&
	    schema.sharedProperties_->isDst(schemaPtr,
					    schema.properties_[index].name.GetString(),
					    schema.properties_[index].name.GetStringLength()));
  }
  bool CheckSharedPairs(Context& context, const SchemaType& schema) {
    for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++)
      if (!it->AssignMissing(context, *this)) return false;
    if (!ExtendSharedTemp(context, schema)) return false;
    for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++)
      if (!it->AssignDefaults(context, *this)) return false;
    if (!ExtendSharedTemp(context, schema, true)) return false;
    ValueType missing(kObjectType);
    for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++) {
      it->RecordMissing(*this, missing);
      it->RemoveShared(*this);
    }
    for (ConstMemberIterator it = missing.MemberBegin();
	 it != missing.MemberEnd(); ++it) {
      context.error_handler.StartMissingProperties();
      for (ConstValueIterator v = it->value.Begin(); v != it->value.End(); ++v)
	context.error_handler.AddMissingProperty((*v)[1]);
      if (context.error_handler.EndMissingPropertiesShared((*it->value.Begin())[0], it->name))
	RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorRequired);
    }
    return true;
  }
  PairEntry* FindSharedPair(const PointerType& p, bool source,
			    bool checkInstance = false) {
    for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++) {
      if (it->Matches(p, source, checkInstance))
	return it;
    }
    return 0;
  }
  void DisplayShared(bool useInstance = false) const {
    for (const PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++) {
      std::cerr << "    ";
      it->Display(useInstance);
      std::cerr << std::endl;
    }
    std::cerr << std::endl;
  }
  void AddSharedObject(const PointerType& instancePtr,
		       const PointerType& schemaPtr,
		       const SValue& present) {
    for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++) {
      bool source = !it->src.set;
      if (it->missing() && !it->src.set && !it->dst.set)
	source = !it->src.missing;
      if (it->set() ||
	  !it->prefix.PartialCompare(instancePtr) ||
	  !it->Matches(instancePtr, source, true)) continue;
      it->Complete(source, instancePtr, schemaPtr, present, &GetAllocator());
    }
  }
  void AddSharedObject(const PointerType& prefix0,
		       const PointerType& instancePtr,
		       const PointerType& schemaPtr,
		       const SValue& present,
		       typename SchemaType::SharedPropertyBase* parent,
		       typename SchemaType::SharedPropertyBase* partner,
		       bool multiplePartners = false) {
    PointerType prefix = instancePtr.PartialFront(prefix0.GetTokenCount(),
						  &GetAllocator());
    PairEntry* pair = sharedStack_.template Push<PairEntry>();
    new (pair) PairEntry(prefix, &GetAllocator());
    pair->Complete(parent->source, instancePtr, schemaPtr, present,
		   &GetAllocator(), parent);
    pair->Template(partner->source, partner,
		   &GetAllocator(), multiplePartners);
  }
  void AddMissingObject(const PointerType& prefix0,
			const PointerType& instancePtr,
			typename SchemaType::SharedPropertyBase* parent,
			bool multiplePairs,
			typename SchemaType::SharedPropertyBase* partner,
			bool multiplePartners) {
    PointerType prefix = instancePtr.PartialFront(prefix0.GetTokenCount(),
						  &GetAllocator());
    PairEntry* pair = sharedStack_.template Push<PairEntry>();
    new (pair) PairEntry(prefix, &GetAllocator());
    RAPIDJSON_ASSERT(parent && partner);
    pair->Missing(parent->source, instancePtr, parent,
		  &GetAllocator(), multiplePairs);
    pair->Template(partner->source, partner,
		   &GetAllocator(), multiplePartners);
  }
  bool IsFinalizedShared(const PairEntry* skip, const SValue& name,
			 bool source) const {
    const SharedValueEntry* skipV = skip->GetValue(source);
    RAPIDJSON_ASSERT(skipV->set);
    if (!skipV->set) return false;
    for (const PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++) {
      if (it == skip) continue;
      if (!it->Matches(skipV->instancePtr, false, true)) continue;
      if (!it->IsFinalized(name, true, *this))
	return false;
    }
    return true;
  }
  bool HasUnvisitedSharedSiblings(PairEntry* skip, const SValue& name,
				  bool source) const {
    SharedValueEntry* skipV = skip->GetValue(!source);
    RAPIDJSON_ASSERT(skipV->set);
    if (!skipV->set) return true;
    for (const PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++) {
      if (it == skip ||
	  it->GetValue(source)->set || !it->GetValue(!source)->set)
	continue;
      if (it->HasUnvisitedProperty(name, source) &&
	  it->Matches(skipV->instancePtr, !source, true))
	return true;
    }
    return false;
  }
  bool SetSharedSiblings(Context& context,
			 PairEntry* skip, const SValue& name,
			 const ValueType* value) {
    RAPIDJSON_ASSERT(skip);
    if (!skip) return false;
    SharedValueEntry* skipV = skip->GetValue(false);
    RAPIDJSON_ASSERT(skipV->set);
    if (!skipV->set) return false;
    for (PairEntry* it = sharedStack_.template Bottom<PairEntry>();
	 it != sharedStack_.template End<PairEntry>(); it++) {
      if (it == skip || !it->dst.set || !it->src.set ||
	  !it->HasMissing(name) ||
	  !it->Matches(skipV->instancePtr, true, true)) continue;
      if (!it->SetMember(context, name, value, *this))
	return false;
    }
    return true;
  }
  bool NormStartArray(Context& context, const SchemaType& schema) {
    NORM_BEGIN_(StartArray);
    NORM_BODY_(StartArray, ());
    return true;
  }
  bool NormalizeShared(Context& context,
		       const SValue& x, SValue& dest,
		       const PointerType instancePtr,
		       const PointerType schemaPtr,
		       const SchemaType& schema) {
    typename SchemaType::ValueType sdv(kNullType);
    SchemaDocumentType sd(sdv);
    GenericStringBuffer<EncodingType> sb;
    instancePtr.StringifyUriFragment(sb);
    // TODO: Move intializer to factory?
    typedef GenericSchemaNormalizer<SchemaDocumentType, BaseReaderHandler<EncodingType>, RAPIDJSON_DEFAULT_STACK_ALLOCATOR> NormalizerType;
    NormalizerType n(sd, schema, sb.GetString(), sb.GetLength(), schemaPtr, *this);
    x.Accept(n);
    if (n.IsValid()) {
      if (n.WasNormalized())
	dest.CopyFrom(n.GetNormalized(), GetAllocator(), true);
      else
	dest.CopyFrom(x, GetAllocator(), true);
      if (!n.GetNormalizedDoc().sharedStack_.Empty()) {
	AddSharedTemp(n.GetNormalizedDoc().sharedStack_);
	// Don't call the PairEntry destructor, it will be called after the
	// temporary stack is processed.
	while (!n.GetNormalizedDoc().sharedStack_.Empty())
	  n.GetNormalizedDoc().sharedStack_.template Pop<PairEntry>(1);
      }
      return true;
    }
    ValidateErrorCode code = context.error_handler.SharedNormalizationError(static_cast<ISchemaValidator*>(&n));
    RAPIDJSON_INVALID_KEYWORD_RETURN(code);
    return false;
  }
  bool NormEndArray(Context& context, const SchemaType& schema, SizeType elementCount) {
    NORM_BEGIN_STUB_(EndArray);
    NORM_BODY_(EndArray, (elementCount));
    NORM_END_(EndArray);
  }

#undef NORM_VALUE_
#undef NORM_END_
#undef NORM_BEGIN_
  
  // Methods for extending a document, checking and/or merging parallel
  //   entries in the process.
  bool BeginExtend(Context& context, bool dont_recurse=false) {
    ValueType* current = CurrentValue();
    if (!current)
      RAPIDJSON_YGGDRASIL_GENERIC_ERROR("No current value set");
    if (CurrentIdx()) {
      bool childMod = false;
      if (*CurrentIdx() >= current->Size()) {
	if ((!CurrentModified()) && CurrentChildModified()) {
	  current->PushBack(ValueType(kNullType).Move(), GetAllocator());
	  childMod = true;
	} else {
	  RAPIDJSON_YGGDRASIL_GENERIC_ERROR("Current index %d outside of"
					    " the current array (size = %d)",
					    (int)(*CurrentIdx()),
					    (int)(current->Size()));
	}
      }
      PushValue((*current)[*CurrentIdx()], *CurrentIdx(), false, childMod);
      CurrentIdx()[0]++;
      current = CurrentValue();
    } else if (CurrentKey()) {
      if (!current->IsObject())
	RAPIDJSON_YGGDRASIL_GENERIC_ERROR("Current value is not an object,"
					  " but a key \"%s\" is set (type = %d)",
					  (const char*)(CurrentKey()->GetString()),
					  (int)(current->GetType()));
      if (!current->HasMember(CurrentKey()->GetString()))
	RAPIDJSON_YGGDRASIL_GENERIC_ERROR("Current value does not have the "
					  "expected key: %s",
					  (const char*)(CurrentKey()->GetString()));
      if (InAliasedKey())
	PushValue((*current)[*CurrentKey()], *(AliasedKey()->aliased));
      else
	PushValue((*current)[*CurrentKey()], *CurrentKey());
      current = CurrentValue();
    }
    PushKey();
    if ((!dont_recurse) && CurrentSingular() && !CurrentChildSingular() &&
	(current->IsArray() || current->IsObject())) {
      if (current->IsArray()) {
	PushKey((SizeType)0);
      } else if (current->IsObject()) {
	ModificationEntry* key_mod = nullptr;
	if (!isValueSingular(GetInstancePointer(), false, &key_mod))
	  return false;
	typename PointerType::Token key_token = key_mod->after.GetTokens()[key_mod->after.GetTokenCount() - 1];
	PushKey(key_token.name, key_token.length);
      }
      return BeginExtend(context, true);
    }
    return true;
  }
  bool EndExtend(Context& context, bool dont_recurse=false) {
    ValueType* current = CurrentValue();
    if ((!dont_recurse) && CurrentSingular() && !CurrentChildSingular() &&
	(current->IsArray() || current->IsObject()))
      return EndExtend(context, true);
    PopKey();
    if (CurrentIdx()) {
      PopValue();
      current = CurrentValue();
    } else if (CurrentKey()) {
      PopValue();
      PopKey();
      current = CurrentValue();
    } else if (CurrentChildModified() && !CurrentModified()) {
      StealChildModified();
    }
    return true;
  }
  bool GetParentKey(PointerType p, ValueType* out) {
    ModificationEntry* key_mod = nullptr;
    if (!isValueSingular(p, false, &key_mod))
      return false;
    typename PointerType::Token key_token = key_mod->after.GetTokens()[key_mod->after.GetTokenCount() - 1];
    if (key_token.index != kPointerInvalidIndex)
      out->SetUint(key_token.index);
    else
      out->SetString(key_token.name, key_token.length, GetAllocator());
    return true;
  }

#define REQUIRED_PROPERTY_(method, cond, args)				\
  if (CurrentChildModified() && !(cond)) {				\
    if (InAliasedKey()) {						\
      KeyEntry* aliased = AliasedKey();					\
      context.error_handler.DuplicateAlias(*(aliased->aliased),		\
					   *(aliased->key));		\
      out = false;							\
      DEBUG_STEP;							\
      RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorAliasDuplicate);	\
    } else {								\
      RAPIDJSON_ASSERT(current);					\
      if (!current)							\
	RAPIDJSON_YGGDRASIL_GENERIC_ERROR("No current value set when raising NormalizationMergeConflict error"); \
      context.error_handler.NormalizationMergeConflict(SchemaType::Get ## method ## String(), \
						       *current, ValueType args.Move()); \
      out = false;							\
      DEBUG_STEP;							\
      RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorMergeConflict);	\
    }									\
  }
#define EXTEND_BEGIN_STUB_(method)					\
  INIT_CHECK(Extend, stub, #method, extend_schema_)
#define EXTEND_BEGIN_(method, args)					\
  INIT_CHECK(Extend, begin, #method, extend_schema_);			\
  ValueType* current = CurrentValue();					\
  if (current && current->IsObject() && CurrentKey() &&			\
      (!current->HasMember(CurrentKey()->GetString()))) {		\
    current->AddMember(ValueType(CurrentKey()->GetString(),		\
				 CurrentKey()->GetStringLength(),	\
				 GetAllocator()).Move(),		\
		       ValueType args.Move(),				\
		       GetAllocator());					\
  }									\
  /* Delay marking collections as modified until after it is complete */\
  bool in_collection = (current->IsArray() || current->IsObject());	\
  out = BeginExtend(context, dont_recurse);				\
  CHECK_RESULT;								\
  if ((!CurrentModified()) && CurrentChildModified()) {			\
    ValueType child_swap args;						\
    CurrentValue()->Swap(child_swap);					\
    /* CurrentValue()->CopyFrom(child_swap, GetAllocator(), true); */	\
    if (!(CurrentValue()->IsArray() || CurrentValue()->IsObject() ||	\
	  in_collection)) {						\
      StealChildModified();						\
    }									\
  } else if (!CurrentSingular() && CurrentChildSingular() && extend_child_) { \
    /* ValueType tmp args; */						\
    /* ValueType child_swap(*CurrentValue(), GetAllocator(), true); */	\
    /* CurrentValue()->CopyFrom(tmp, GetAllocator(), true); */		\
    ValueType child_swap args;						\
    CurrentValue()->Swap(child_swap);					\
    if (CurrentValue()->IsArray()) {					\
      RecordModifiedSingular((SizeType)0);				\
      CurrentValue()->PushBack(child_swap, GetAllocator());		\
    } else if (CurrentValue()->IsObject()) {				\
      ValueType key;							\
      out = extend_child_->GetParentKey(GetInstancePointer(), &key);	\
      CHECK_RESULT;							\
      RecordModifiedSingular(key);					\
      CurrentValue()->AddMember(key, child_swap, GetAllocator());	\
    }									\
  }									\
  current = CurrentValue();						\
  REQUIRED_PROPERTY_(method, (current), args);				\
  CHECK_RESULT
#define EXTEND_END_(method)						\
  MOD_CHECK(Extend, end, #method);					\
  out = EndExtend(context, dont_recurse);				\
  CHECK_RESULT;								\
  return out
#define EXTEND_VALUE_(method, value, args)				\
  bool dont_recurse = false;						\
  EXTEND_BEGIN_(method, args);						\
  REQUIRED_PROPERTY_(method, (current->Is ## method()), args);		\
  REQUIRED_PROPERTY_(method, (value == current->Get ## method()), args); \
  EXTEND_END_(method)

  bool ExtendNull(Context& context) {
    bool dont_recurse = false;
    EXTEND_BEGIN_(Null, (kNullType));
    REQUIRED_PROPERTY_(Null, (current->IsNull()), (kNullType));
    EXTEND_END_(Null);
  }
  bool ExtendBool(Context& context, bool b) { EXTEND_VALUE_(Bool, b, (b)); }
  bool ExtendInt(Context& context, int i) { EXTEND_VALUE_(Int, i, (i)); }
  bool ExtendUint(Context& context, unsigned u) { EXTEND_VALUE_(Uint, u, (u)); }
  bool ExtendInt64(Context& context, int64_t i) { EXTEND_VALUE_(Int64, i, (i)); }
  bool ExtendUint64(Context& context, uint64_t u) { EXTEND_VALUE_(Uint64, u, (u)); }
  bool ExtendDouble(Context& context, double d) {
    bool dont_recurse = false;
    EXTEND_BEGIN_(Double, (d));
    REQUIRED_PROPERTY_(Double, (current->IsDouble()), (d));
    double b = current->GetDouble();
    REQUIRED_PROPERTY_(Double, (d >= b && d <= b), (d));
    EXTEND_END_(Double);
  }
  bool ExtendString(Context& context, const Ch* str, SizeType length, bool) {
    bool dont_recurse = false;
    EXTEND_BEGIN_(String, (str, length, GetAllocator()));
    REQUIRED_PROPERTY_(String, (current->IsString()), (str, length));
    REQUIRED_PROPERTY_(String,
		       (internal::StrCmp(str, current->GetString()) == 0),
		       (str, length));
    EXTEND_END_(String);
  }
  template <typename YggSchemaValueType>
  bool ExtendYggdrasilString(Context& context, const Ch* str, SizeType length, bool, YggSchemaValueType& valueSchema) {
    bool dont_recurse = false;
    EXTEND_BEGIN_(YggdrasilString, (str, length, valueSchema));
    REQUIRED_PROPERTY_(YggdrasilString,
		       (current && current->IsYggdrasil()),
		       (str, length, valueSchema));
    REQUIRED_PROPERTY_(YggdrasilString,
		       (current->GetValueSchema() == valueSchema),
		       (str, length, valueSchema));
    REQUIRED_PROPERTY_(YggdrasilString,
		       current->IsString(),
		       (str, length, valueSchema));
    REQUIRED_PROPERTY_(YggdrasilString,
		       (internal::StrCmp(str, current->GetString()) == 0),
		       (str, length, valueSchema));
    EXTEND_END_(YggdrasilString);
  }
  template <typename YggSchemaValueType>
  bool ExtendYggdrasilStartObject(Context& context, YggSchemaValueType& valueSchema, bool dont_recurse=false) {
    EXTEND_BEGIN_(YggdrasilObject, (kObjectType, valueSchema));
    REQUIRED_PROPERTY_(YggdrasilObject,
		       (current && current->IsYggdrasil()),
		       (kObjectType, valueSchema));
    REQUIRED_PROPERTY_(YggdrasilObject,
		       (current->GetValueSchema() == valueSchema),
		       (kObjectType, valueSchema));
    REQUIRED_PROPERTY_(YggdrasilObject,
		       current->IsObject(),
		       (kObjectType, valueSchema));
    return true;
  }
  bool ExtendYggdrasilEndObject(Context& context, SizeType, bool dont_recurse=false) {
    EXTEND_BEGIN_STUB_(YggdrasilObject);
    EXTEND_END_(YggdrasilEndObject);
  }
  bool ExtendStartObject(Context& context, bool dont_recurse=false) {
    EXTEND_BEGIN_(Object, (kObjectType));
    if (extend_child_ && CurrentModified() && CurrentChildModified()) {
      PointerType p = GetInstancePointer();
      ModificationEntry* key_mod = nullptr;
      if (!isValueSingular(p) &&
	  extend_child_->isValueSingular(p, false, &key_mod)) {
	typename PointerType::Token key_token = key_mod->after.GetTokens()[key_mod->after.GetTokenCount() - 1];
	ValueType key(key_token.name, key_token.length, GetAllocator());
	ValueType tmp(kObjectType);
	tmp.AddMember(ValueType(key, GetAllocator(), true).Move(),
		      ValueType(kNullType).Move(), GetAllocator());
	// tmp[key].CopyFrom(*current, GetAllocator(), true);
	// current->CopyFrom(tmp, GetAllocator(), true);
	current->Swap(tmp[key]);
	current->Swap(tmp);
	StealChildModified();
      }
    }
    REQUIRED_PROPERTY_(Object, current->IsObject(), (kObjectType));
    return true;
  }
  bool ExtendKey(Context& context, const Ch* str, SizeType len, bool copy) {
    ValueType orig;
    ValueType primary;
    INIT_CHECK(Extend, key, (const char*)str, extend_schema_);
    out = AliasKey(context, str, len, copy, false, orig, primary);
    CHECK_RESULT;
    if (primary.IsString()) {
      len = primary.GetStringLength();
      str = primary.GetString();
      copy = true;
    }
    if (orig.IsString())
      PushKey(str, len, &orig);
    else
      PushKey(str, len);
    CHECK_RESULT;
    return true;
  }
  bool ExtendEndObject(Context& context, SizeType, bool dont_recurse=false) {
    EXTEND_BEGIN_STUB_(Object);
    ValueType* current = CurrentValue();
    REQUIRED_PROPERTY_(Object, current->IsObject(), (*current, GetAllocator()));
    // Don't check memberCount, as members may be missing from either the
    //   child or the parent
    // REQUIRED_PROPERTY_(Object, current->MemberCount() == memberCount,
    // 		       (*current, GetAllocator()));
    if ((!CurrentModified()) && CurrentChildModified())
      StealChildModified();
    EXTEND_END_(EndObject);
  }
  bool ExtendStartArray(Context& context, bool dont_recurse=false) {
    EXTEND_BEGIN_(Array, (kArrayType));
    if (extend_child_ && CurrentModified() && CurrentChildModified()) {
      PointerType p = GetInstancePointer();
      if (!isValueSingular(p) && extend_child_->isValueSingular(p)) {
	ValueType tmp(kArrayType);
	tmp.PushBack(*current, GetAllocator());
	// current->CopyFrom(tmp, GetAllocator(), true);
	current->Swap(tmp);
	StealChildModified();
      }
    }
    REQUIRED_PROPERTY_(Array, current->IsArray(), (kArrayType));
    PushKey(0);
    return true;
  }
  bool ExtendEndArray(Context& context, SizeType elementCount, bool dont_recurse=false) {
    EXTEND_BEGIN_STUB_(Array);
    ValueType* current = CurrentValue();
    REQUIRED_PROPERTY_(Array, CurrentIdx(), (*current, GetAllocator()));
    REQUIRED_PROPERTY_(Array, current->IsArray(), (*current, GetAllocator()));
    REQUIRED_PROPERTY_(Array, current->Size() == elementCount,
		       (*current, GetAllocator()));
    out = CurrentIdx();
    CHECK_RESULT;
    PopKey();
    if ((!CurrentModified()) && CurrentChildModified())
      StealChildModified();
    EXTEND_END_(EndArray);
  }

#undef EXTEND_VALUE_
#undef EXTEND_BEGIN_
#undef EXTEND_END_
#undef REQUIRED_PROPERTY_

  void SetDocumentStack(internal::Stack<StackAllocatorType>* stack) {
    documentStack_ = stack;
  }

  void SetBasePointer(const PointerType& p) {
    basePointer_ = p;
  }

  void* SetTemporary(SizeType size) {
    RAPIDJSON_ASSERT(!temporary_memory_);
    temporary_memory_ = GetAllocator().Malloc(size);
    RAPIDJSON_ASSERT(temporary_memory_);
    return temporary_memory_;
  }
  
  PointerType GetSchemaPointer(const SchemaType& schema) const {
    if (extend_schema_)
      return extend_schema_->GetPointer();
    return schema.GetPointer();
  }

  template <typename VType>
  PointerType GetInstancePointer(const VType& key, bool parent,
				 bool modified=false,
				 RAPIDJSON_DISABLEIF((
       internal::OrExpr<YGGDRASIL_IS_INT_TYPE(VType),
       internal::OrExpr<YGGDRASIL_IS_UINT_TYPE(VType),
       internal::OrExpr<internal::IsSame<VType, bool>,
       internal::IsPointer<VType> > > >))) {
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    std::cerr << "GetInstancePointer (key = " << key.GetString() << ", " << key.GetStringLength() << ") -> ";
    if (parent) {
      DisplayPointer(GetInstancePointer(false, modified));
      std::cerr << " -> ";
    }
    DisplayPointer(GetInstancePointer(parent, modified));
    std::cerr << " -> ";
    DisplayPointer(GetInstancePointer(parent, modified).Append(key.GetString(),
							       key.GetStringLength(),
							       &GetAllocator()));
    std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    return GetInstancePointer(parent, modified).Append(key.GetString(),
						       key.GetStringLength(),
						       &GetAllocator());
  }
  PointerType GetInstancePointer(const SizeType& index, bool parent,
				 bool modified=false) {
    return GetInstancePointer(parent, modified).Append(index, &GetAllocator());
  }
  PointerType GetInstancePointer(bool parent=false, bool modified=false) {
    PointerType instancePointer = GetInstancePointerBase(&GetAllocator());
    if (modified)
      instancePointer = ReplaceSingular(instancePointer);
    if (parent && (instancePointer.GetTokenCount() > 0))
      return instancePointer.Pop(1, &GetAllocator());
    return instancePointer;
  }
  PointerType GetInstancePointerSingular(bool parent=false,
					 bool modified=false) {
    return GetInstancePointer(SchemaType::GetAllowSingularString(), parent,
			      modified);
  }
  template<typename VType>
  PointerType GetInstancePointerSingular(const VType& key, bool parent,
					 bool modified = false,
					 RAPIDJSON_DISABLEIF((
       internal::OrExpr<YGGDRASIL_IS_INT_TYPE(VType),
       internal::OrExpr<YGGDRASIL_IS_UINT_TYPE(VType),
       internal::OrExpr<internal::IsSame<VType, bool>,
       internal::IsPointer<VType> > > >))) {
    return GetInstancePointerSingular(parent, modified).Append(key.GetString(),
							       key.GetStringLength(),
							       &GetAllocator());
  }
  PointerType GetInstancePointerSingular(const SizeType& index, bool parent,
					 bool modified = false) {
    return GetInstancePointerSingular(parent, modified).Append(index, &GetAllocator());
  }
  PointerType GetInstancePointerBase(AllocatorType* allocator = 0) const {
    RAPIDJSON_ASSERT(documentStack_);
    if (extending_ && !appending_ && !valueStack_.Empty()) {
      return PointerType(CurrentPointer(), allocator);
    } else {
      if (!documentStack_->Empty()) {
	return PointerType(documentStack_->template Bottom<Ch>(), documentStack_->GetSize() / sizeof(Ch), allocator);
      }
    }
    return PointerType(allocator);
  }
  ValueType* Get(const PointerType& p, size_t* unresolvedTokenIndex = 0) {
    // Replace other modifications like alias?
    PointerType pSing = ReplaceSingular(p);
    if (core_ && !pSing.StartsWith(basePointer_))
      return core_->Get(pSing, unresolvedTokenIndex);
    RAPIDJSON_ASSERT(pSing.StartsWith(basePointer_));
    return pSing.PartialBack(basePointer_.GetTokenCount()).GetFromUnfinalized(document_, unresolvedTokenIndex);
  }
  void RecordModified(const PointerType before, const PointerType after,
		      bool no_before=false, bool no_after=false,
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
		      const char* debugMessage=0
#else // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
		      const char* = 0
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
		      ) {
    ModificationEntry* match = nullptr;
    PointerType after_mod = ReplaceSingular(after);
    if (!(isValueModified(before, true, kCheckModifiedBefore, no_before, &match) &&
          (match->no_after == no_after) &&
          PointerStartsWith(match->after, after_mod, true))) {
      ModificationEntry* ref = modifiedStack_.template Push<ModificationEntry>();
      new (ref) ModificationEntry(before, after_mod, GetAllocator(),
				  no_before, no_after);
    }
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    std::cerr << "RecordModified";
    if (debugMessage)
      std::cerr << "[" << debugMessage << "]";
    std::cerr << " (";
    std::cerr << isValueModified(before, true, kCheckModifiedBefore, no_before, &match) << ", " <<
      (match->no_after == no_after) << ", " <<
      PointerStartsWith(match->after, after_mod, true) << ", ";
    DisplayPointer(before);
    std::cerr << " -> ";
    DisplayPointer(after);
    std::cerr << "::";
    DisplayPointer(after_mod);
    std::cerr << ") ";
    DisplayModifications();
    std::cerr << "RecordModified [Singular] ";
    DisplayModifications(&singularStack_);
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
  }
  
private:

  void RecordModifiedAlias(const ValueType& primary, const ValueType& alias,
			   PointerType* ptr_base=nullptr) {
    bool in_extend = (ptr_base || (extending_ && !appending_));
    RecordModified(alias, primary, !in_extend, "alias");
    if (in_extend && HasMember(alias)) {
      PointerType p_primary;
      PointerType p_alias;
      if (ptr_base) {
	p_primary = ptr_base->Append(primary.GetString(),
				     primary.GetStringLength(),
				     &GetAllocator());
	p_alias = ptr_base->Append(alias.GetString(),
				   alias.GetStringLength(),
				   &GetAllocator());
      } else {
	p_primary = GetInstancePointer(primary, false);
	p_alias = GetInstancePointer(alias, false);
      }
      // Replace all labels of modification with alias
      SizeType N = (SizeType)(modifiedStack_.GetSize() / sizeof(ModificationEntry));
      ModificationEntry* it = modifiedStack_.template Bottom<ModificationEntry>();
      for (SizeType i = 0; i < N; i++, it++) {
	if (PointerStartsWith(it->after, p_alias, false)) {
	  // TODO: Replace old modification?
	  // *it = it->Replace(p_alias.GetTokenCount() - 1,
	  // 		    primary.GetString(), primary.GetStringLength(),
	  // 		    &GetAllocator());
	  RecordModified(it->after.Replace((SizeType)(p_alias.GetTokenCount() - 1),
					   primary.GetString(),
					   primary.GetStringLength(),
					   &GetAllocator()),
			 false, false, "existingAlias");
	}
      }
      // Swap the aliased value
      ValueType tmp(kNullType);
      tmp.Swap(*GetMember(alias));
      // tmp.CopyFrom(*GetMember(alias), GetAllocator(), true);
      RemoveMember(alias);
      CurrentValue()->AddMember(ValueType(primary, GetAllocator(), true).Move(),
				tmp, GetAllocator());
    }
  }

  template<typename VType>
  void RecordModifiedSingular(const VType& key, bool parent=false) {
    RecordModified(GetInstancePointer(parent, true),
		   GetInstancePointerSingular(key, parent, true),
		   false, false, "singular");
    RecordSingular(GetInstancePointer(parent, true),
		   GetInstancePointer(key, parent, true));
  }
  PointerType ReplacePrefix(const PointerType p,
			    const PointerType& before,
			    const PointerType& after) {
    if ((!PointerStartsWith(p, before, false)) ||
	PointerStartsWith(p, after, false))
      return p;
    PointerType q = after;
    for (size_t i = before.GetTokenCount(); i < p.GetTokenCount(); i++)
      q = q.Append(p.GetTokens()[i], &GetAllocator());
    return q;
  }
  PointerType ReplaceSingular(const PointerType p) {
    if (singularStack_.Empty())
      return p;
    PointerType q(p, &GetAllocator());
    SizeType N = (SizeType)(singularStack_.GetSize() / sizeof(ModificationEntry));
    ModificationEntry* it = singularStack_.template Bottom<ModificationEntry>();
    for (SizeType i = 0; i < N; i++, it++)
      q = ReplacePrefix(q, it->before, it->after);
    return q;
  }
  PointerType ReplaceModified(const PointerType& p) {
    if (modifiedStack_.Empty()) return p;
    PointerType q(p, &GetAllocator());
    SizeType N = (SizeType)(modifiedStack_.GetSize() / sizeof(ModificationEntry));
    ModificationEntry* it = modifiedStack_.template Bottom<ModificationEntry>();
    for (SizeType i = 0; i < N; i++, it++)
      q = ReplacePrefix(q, it->before, it->after);
    return q;
  }
  void RecordSingular(const PointerType before, const PointerType after) {
    SizeType N = (SizeType)(singularStack_.GetSize() / sizeof(ModificationEntry));
    ModificationEntry* it = singularStack_.template Bottom<ModificationEntry>();
    for (SizeType i = 0; i < N; i++, it++) {
      if (PointerStartsWith(it->before, before, true) &&
	  PointerStartsWith(it->after, after, true))
	return;
    }
    PointerType after_mod = ReplaceSingular(after);
    it = singularStack_.template Bottom<ModificationEntry>();
    for (SizeType i = 0; i < N; i++, it++)
      it->after = ReplacePrefix(it->after, before, after_mod);
    ModificationEntry* ref = singularStack_.template Push<ModificationEntry>();
    new (ref) ModificationEntry(before, after_mod, GetAllocator());
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    std::cerr << "RecordSingular ";
    DisplayModifications(&singularStack_);
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
  }
  void PopSingular() {
    ModificationEntry* ref = singularStack_.template Pop<ModificationEntry>(1);
    ref->~ModificationEntry();
  }
  void RecordModified(const PointerType p,
		      bool no_before=false, bool no_after=false,
		      const char* debugMessage=0) {
    RecordModified(p, p, no_before, no_after, debugMessage);
  }
  void RecordModified(bool parent=false,
		      const char* debugMessage=0) {
    RecordModified(GetInstancePointer(parent), false, false, debugMessage);
    if (extending_ && !appending_ && !valueStack_.Empty()) {
      if (parent)
	(valueStack_.template Top<ValueEntry>() - 1)->modified = true;
      else
	valueStack_.template Top<ValueEntry>()->modified = true;
    }
  }
  template <typename VType>
  void RecordModified(const VType& key, bool parent=false,
		      const char* debugMessage=0) {
    RecordModified(GetInstancePointer(key, parent), false, false,
		   debugMessage);
  }
  template <typename VType>
  void RecordModified(const VType& key_before, const VType& key_after,
		      bool parent=false, const char* debugMessage=0) {
    RecordModified(GetInstancePointer(key_before, parent),
		   GetInstancePointer(key_after, parent),
		   false, false, debugMessage);
  }
  void RecordModified(ModificationEntry& entry,
		      const char* debugMessage=0) {
    RecordModified(entry.before, entry.after,
		   entry.no_before, entry.no_after,
		   debugMessage);
  }
  template <typename VType>
  void RecordModifiedRemoved(const VType& key, bool parent=false) {
    RecordModified(GetInstancePointer(key, parent),
		   GetInstancePointer(parent),
		   false, true, "removed");
  }
  template <typename VType>
  void RecordModifiedAdded(const VType& key, bool parent=false) {
    RecordModified(GetInstancePointer(parent),
		   GetInstancePointer(key, parent),
		   true, false, "added");
  }
  template <typename VType>
  void RecordModifiedAdded(const VType& key, const VType& val,
			   bool parent=false) {
    return RecordModifiedAdded(key, val, GetInstancePointer(parent));
  }
  template <typename VType>
  void RecordModifiedAdded(const VType& key, const VType& val,
			   const PointerType pOld) {
    PointerType pNew;
    if (key.IsString())
      pNew = pOld.Append(key.GetString(),
			 key.GetStringLength(),
			 &GetAllocator());
    else
      pNew = pOld.Append((SizeType)(key.GetUint()), &GetAllocator());
    RecordModified(pOld, pNew, true, false, "added");
    if (val.IsArray()) {
      SizeType i = 0;
      for (typename VType::ConstValueIterator it = val.Begin();
	   it != val.End(); ++it, ++i)
	RecordModifiedAdded(VType(i).Move(), *it, pNew);
    } else if (val.IsObject()) {
      for (typename VType::ConstMemberIterator it = val.MemberBegin();
	   it != val.MemberEnd(); ++it)
	RecordModifiedAdded(it->name, it->value, pNew);
    }
  }
  void StealChildModified(bool parent=false) {
    StealChildModified(GetInstancePointer(parent));
  }
  void StealChildModified(const PointerType& p) {
    if ((!extend_child_) || extend_child_->modifiedStack_.Empty())
      return;
    SizeType N = (SizeType)(extend_child_->modifiedStack_.GetSize() / sizeof(ModificationEntry));
    ModificationEntry* it = extend_child_->modifiedStack_.template Bottom<ModificationEntry>();
    for (SizeType i = 0; i < N; i++, it++) {
      if (PointerStartsWith(it->before, p, false))
	RecordModified(*it, "steal");
    }
  }
  void PopModified() {
    ModificationEntry* ref = modifiedStack_.template Pop<ModificationEntry>(1);
    ref->~ModificationEntry();
  }
  bool isValueSingular(const PointerType& p, bool exact=false,
		       ModificationEntry** match=nullptr,
		       bool appended=false) {
    PointerType q;
    if (appended) {
      q = p.Pop(1).Append(
	   SchemaType::GetAllowSingularString().GetString(),
	   SchemaType::GetAllowSingularString().GetStringLength()).Append(
	       p.GetTokens()[p.GetTokenCount() - 1]);
    } else {
      q = p.Append(SchemaType::GetAllowSingularString().GetString(),
		   SchemaType::GetAllowSingularString().GetStringLength());
    }
    return isValueModified(q, exact, kCheckModifiedAfter, false, match);
  }
  bool isValueModified(const PointerType& p, bool exact=false,
		       ModificationFlag checkFlag=kCheckModifiedNull,
		       bool is_empty=false, ModificationEntry** match=nullptr) {
    if (modifiedStack_.Empty())
      return false;
    if (checkFlag == kCheckModifiedNull) {
      if (extending_ && !appending_)
	checkFlag = kCheckModifiedBoth;
      else
	checkFlag = kCheckModifiedBefore;
    }
    SizeType N = (SizeType)(modifiedStack_.GetSize() / sizeof(ModificationEntry));
    ModificationEntry* it = modifiedStack_.template Bottom<ModificationEntry>();
    for (SizeType i = 0; i < N; i++, it++) {
      if (((checkFlag <= kCheckModifiedBoth) &&
	   PointerStartsWith(it->before, p, exact) &&
	   ((it->no_before == is_empty) || (!exact))) ||
	  ((checkFlag >= kCheckModifiedBoth) &&
	   PointerStartsWith(it->after, p, exact) &&
	   ((it->no_after == is_empty) || (!exact)))) {
	if (match)
	  *match = it;
	return true;
      }
    }
    return false;
  }
  bool PointerStartsWith(const PointerType& a, const PointerType& b,
			 bool exact=false) {
    if (exact)
      return (a == b);
    return a.StartsWith(b);
  }

  bool ToggleSingular() {
    inSingular_ = (!inSingular_);
    return inSingular_;
  }

  const ValueType* CurrentValue() const {
    if (extending_ && !appending_) {
      RAPIDJSON_ASSERT(!valueStack_.Empty());
      if (valueStack_.Empty())
	return nullptr;
      return valueStack_.template Top<ValueEntry>()->val;
    } else {
      RAPIDJSON_ASSERT(document_.StackSize() > 0);
      if (document_.StackSize() == 0)
	return nullptr;
      return document_.StackTop();
    }
  }
  ValueType* CurrentValue() {
    if (extending_ && !appending_) {
      RAPIDJSON_ASSERT(!valueStack_.Empty());
      if (valueStack_.Empty())
	return nullptr;
      return valueStack_.template Top<ValueEntry>()->val;
    } else {
      RAPIDJSON_ASSERT(document_.StackSize() > 0);
      if (document_.StackSize() == 0)
	return nullptr;
      return document_.StackTop();
    }
  }
  bool CurrentModified() {
    if (extending_ && !appending_ && !valueStack_.Empty())
      return valueStack_.template Top<ValueEntry>()->modified;
    return false;
  }
  bool CurrentChildModified() {
    if (extending_ && !appending_ && !valueStack_.Empty())
      return valueStack_.template Top<ValueEntry>()->child_modified;
    return false;
  }
  bool CurrentSingular() {
    if (extending_ && !appending_ && !valueStack_.Empty())
      return valueStack_.template Top<ValueEntry>()->singular;
    return false;
  }
  bool CurrentChildSingular() {
    if (extending_ && !appending_ && !valueStack_.Empty())
      return valueStack_.template Top<ValueEntry>()->child_singular;
    return false;
  }
  template<typename VType>
  bool CurrentChildSingular(const VType& key) {
    if (CurrentChildSingular())
      return true;
    PointerType p;
    if (valueStack_.Empty())
      p = GetInstancePointer(key, false);
    else
      p = CurrentPointer(key);
    return isValueSingular(p, false, nullptr, true);
  }
  void PushValue(ValueType& value, PointerType& p, bool appended=false,
		 bool modified = false, bool child_modified = false) {
    ValueEntry* ref = valueStack_.template Push<ValueEntry>();
    new (ref) ValueEntry(value, p, &GetAllocator());
    ref->singular = isValueSingular(p, false, nullptr, appended);
    if (ref->singular || modified)
      ref->modified = true;
    else
      ref->modified = isValueModified(p, false, kCheckModifiedBoth);
    if (extend_child_) {
      ref->child_singular = extend_child_->isValueSingular(p, false, nullptr,
							   appended);
      if (ref->child_singular || child_modified)
	ref->child_modified = true;
      else
	ref->child_modified = extend_child_->isValueModified(p, false,
							     kCheckModifiedBoth);
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
      std::cerr << "Push value: ";
      DisplayPointer(p);
      std::cerr << ", " << ref->modified << ", " << ref->child_modified << ", " << ref->singular << ", " << ref->child_singular << std::endl;
      std::cerr << "Parent ";
      DisplayModifications();
      std::cerr << "Child ";
      extend_child_->DisplayModifications();
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    }
  }
  void PushValue(ValueType& value,
		 bool modified = false, bool child_modified = false) {
    PointerType p;
    if (valueStack_.Empty())
      p = GetInstancePointer();
    else
      p = CurrentPointer();
    PushValue(value, p, false, modified, child_modified);
  }
  void PushValue(ValueType& value, ValueType& key,
		 bool modified = false, bool child_modified = false) {
    PointerType p = CurrentPointer(key);
    PushValue(value, p, true, modified, child_modified);
  }
  void PushValue(ValueType& value, SizeType index,
		 bool modified = false, bool child_modified = false) {
    PointerType p = CurrentPointer(index);
    PushValue(value, p, true, modified, child_modified);
  }
  void PopValue() {
    ValueEntry* ref = valueStack_.template Pop<ValueEntry>(1);
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    std::cerr << "PopValue: ";
    DisplayValue(*(ref->val));
    std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    ref->~ValueEntry();
  }
  size_t ValueCount() const {
    return valueStack_.GetSize() / sizeof(ValueEntry);
  }
  const PointerType& CurrentPointer() const {
    RAPIDJSON_ASSERT(!valueStack_.Empty());
    return valueStack_.template Top<ValueEntry>()->ptr;
  }
  PointerType CurrentPointer(const SizeType& index) {
    return CurrentPointer().Append(index, &GetAllocator());
  }
  template<typename VType>
  PointerType CurrentPointer(const VType& key) {
    return CurrentPointer().Append(key.GetString(), key.GetStringLength(),
				   &GetAllocator());
  }
  KeyEntry* AliasedKey() {
    if (KeyCount() < 2)
      return nullptr;
    KeyEntry* ref = (keyStack_.template Top<KeyEntry>() - 1);
    return ref;
  }
  bool InAliasedKey() {
    KeyEntry* ref = AliasedKey();
    return ((ref != nullptr) &&
	    ((ref->key != nullptr) && (ref->aliased != nullptr)));
  }
  ValueType* CurrentKey() {
    if (keyStack_.Empty())
      return nullptr;
    else
      return keyStack_.template Top<KeyEntry>()->key;
  }
  SizeType* CurrentIdx() {
    if (keyStack_.Empty())
      return nullptr;
    else
      return keyStack_.template Top<KeyEntry>()->idx;
  }
  void PushKey(SizeType idx) {
    KeyEntry* ref = keyStack_.template Push<KeyEntry>();
    ref->idx = new SizeType(idx);
    ref->key = nullptr;
    ref->aliased = nullptr;
  }
  void PushKey(const Ch* str, SizeType len, ValueType* aliased=nullptr) {
    KeyEntry* ref = keyStack_.template Push<KeyEntry>();
    ref->key = new ValueType(str, len, GetAllocator());
    ref->idx = nullptr;
    ref->aliased = nullptr;
    if (aliased)
      ref->aliased = new ValueType(aliased->GetString(),
				   aliased->GetStringLength(),
				   GetAllocator());
  }
  void PushKey() {
    KeyEntry* ref = keyStack_.template Push<KeyEntry>();
    ref->key = nullptr;
    ref->idx = nullptr;
    ref->aliased = nullptr;
  }
  void PopKey() {
    KeyEntry* ref = keyStack_.template Pop<KeyEntry>(1);
    if (ref->key)
      delete ref->key;
    if (ref->idx)
      delete ref->idx;
    if (ref->aliased)
      delete ref->aliased;
  }
  size_t KeyCount() const {
    return keyStack_.GetSize() / sizeof(KeyEntry);
  }
  bool GetFinalAlias(Context& context, const ValueType& aliases,
		     const ValueType& orig, ValueType* dest) {
    if (!follow_aliases_(aliases, orig, dest, GetAllocator())) {
      context.error_handler.CircularAlias(*dest);
      RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorCircularAlias);
    }
    return true;
  }
  void GetSchemaRef(GenericStringBuffer<EncodingType>& sb, const SchemaType& schema) {
    SizeType len = schema.GetURI().GetStringLength();
    if (len) memcpy(sb.Push(len), schema.GetURI().GetString(), len * sizeof(Ch));
    PointerType schemaPointer = GetSchemaPointer(schema);
    schemaPointer.StringifyUriFragment(sb);
  }
  void GetInstanceRef(GenericStringBuffer<EncodingType>& sb, bool parent=true) {
    PointerType instancePointer = GetInstancePointer();
    ((parent && (instancePointer.GetTokenCount() > 0))
     ? PointerType(instancePointer.GetTokens(), instancePointer.GetTokenCount() - 1)
     : instancePointer).StringifyUriFragment(sb);
  }
  void RemoveMember(const ValueType& key, SizeType* memberCount=nullptr,
		    const SchemaType* schema=nullptr) {
    if (schema) {
      for (SizeType index = 0; index < schema->propertyCount_; index++)
	if (schema->properties_[index].name.GetStringLength() == key.GetStringLength() &&
	    std::memcmp(schema->properties_[index].name.GetString(),
			key.GetString(), sizeof(Ch) * key.GetStringLength()))
	  return;
    }
    if (extending_ && !appending_) {
      RAPIDJSON_ASSERT(CurrentValue()->IsObject());
      CurrentValue()->RemoveMember(key);
      return;
    }
    if (document_.StackSize() == 0)
      return;
    ValueType* base = document_.StackTop();
    if (memberCount) {
      SizeType i = 0;
      while ((base != document_.StackBottom()) && (i < (*memberCount * 2))) {
	base--;
	i++;
      }
    } else {
      if (base->IsObject())
	return;
      while ((base != document_.StackBottom()) && (!base->IsObject())) base--;
    }
    RAPIDJSON_ASSERT(base->IsObject());
    base++;
    while (base != document_.StackTop()) {
      RAPIDJSON_ASSERT(base->IsString());
      if ((*base == key) && (base != document_.StackTop())) {
	if (base + 1 == document_.StackTop()) {
	  document_.StackPop()->~ValueType();
	  document_.StackPop()->~ValueType();
	} else {
	  base->~ValueType();
	  (base + 1)->~ValueType();
	  memmove((void*)base, (void*)(base + 2), (document_.StackSize() - 2) * sizeof(ValueType));
	  // Don't call destructor as the memory should be empty
	  document_.StackPop();
	  document_.StackPop();
	}
	if (memberCount)
	  (*memberCount)--;
	return;
      }
      base++;
      if (base == document_.StackTop()) break;
      base++;
    }
  }
  bool HasMember(const ValueType& key, SizeType* memberCount=nullptr) const {
    if (extending_ && !appending_) {
      RAPIDJSON_ASSERT(CurrentValue()->IsObject());
      if (CurrentValue()->HasMember(key))
	return true;
      return false;
    }
    if (document_.StackSize() == 0)
      return false;
    const ValueType* base = document_.StackTop();
    if (memberCount) {
      SizeType i = 0;
      while ((base != document_.StackBottom()) && (i < (*memberCount * 2))) {
	base--;
	i++;
      }
    } else {
      if (base->IsObject())
	  return false;
      while ((base != document_.StackBottom()) && (!base->IsObject())) base--;
    }
    RAPIDJSON_ASSERT(base->IsObject());
    base++;
    while (base != document_.StackTop()) {
      RAPIDJSON_ASSERT(base->IsString());
      if (*base == key)
	return true;
      base++;
      if (base == document_.StackTop()) break;
      base++;
    }
    return false;
  }
  ValueType* GetMember(const ValueType& key, SizeType* memberCount=nullptr) {
    if (extending_ && !appending_) {
      RAPIDJSON_ASSERT(CurrentValue()->IsObject());
      if (CurrentValue()->HasMember(key))
	return &(CurrentValue()->FindMember(key)->value);
      return nullptr;
    }
    if (document_.StackSize() == 0)
      return nullptr;
    ValueType* base = document_.StackTop();
    if (memberCount) {
      SizeType i = 0;
      while ((base != document_.StackBottom()) && (i < (*memberCount * 2))) {
	base--;
	i++;
      }
    } else {
      if (base->IsObject())
	return nullptr;
      while ((base != document_.StackBottom()) && (!base->IsObject())) base--;
    }
    RAPIDJSON_ASSERT(base->IsObject());
    base++;
    while (base != document_.StackTop()) {
      RAPIDJSON_ASSERT(base->IsString());
      if ((*base == key) && (base != document_.StackTop()))
	return (base + 1);
      base++;
      if (base == document_.StackTop()) break;
      base++;
    }
    return nullptr;
  }
  bool Address2Pointer(const ValueType& address, PointerType& ptr,
		       size_t unfinalized=0) {
    if (unfinalized) {
      GenericStringBuffer<EncodingType> instanceRef;
      GetInstanceRef(instanceRef, false);
      SizeType currentLength = static_cast<SizeType>(instanceRef.GetSize() / sizeof(Ch));
      if (currentLength > address.GetStringLength()) {
	// std::cerr << "In Address2Value, the current address is longer than the requested address: " << instanceRef.GetString() << " vs. " << address.GetString() << std::endl;
	return false;
      } else {
	ptr = PointerType(address.GetString() + currentLength,
			  address.GetStringLength() - currentLength,
			  &GetAllocator());
      }
    } else {
      ptr = PointerType(address.GetString(), address.GetStringLength(),
			&GetAllocator());
    }
    return true;
  }
  ValueType* Address2Value(const ValueType& address, ValueType* base = nullptr,
			   PointerType* ptr=nullptr, size_t unfinalized=0) {
    if (!base) base = CurrentValue();
    size_t idx = 0;
    PointerType ptr_target;
    if (!ptr)
      ptr = &ptr_target;
    if (!Address2Pointer(address, *ptr, unfinalized))
      return nullptr;
    typedef GenericPointer<ValueType, AllocatorType> ValuePointerType;
    ValuePointerType schema_ptr((typename ValuePointerType::Token*)(ptr->GetTokens()),
				ptr->GetTokenCount());
    return schema_ptr.Get(*base, &idx);
  }
  //! Add new aliases and check if the document contains any of them.
  bool ExtendAliases(Context& context, ValueType& aliases, bool* replaced) {
    *replaced = false;
    for (typename ValueType::ConstMemberIterator it = aliases.MemberBegin(); it != aliases.MemberEnd(); ++it) {
      if (!aliases_.HasMember(it->name)) {
	aliases_.AddMember(ValueType(it->name.GetString(),
				     it->name.GetStringLength(),
				     GetAllocator()).Move(),
			   kObjectType, GetAllocator());
      }
      for (typename ValueType::ConstMemberIterator v = it->value.MemberBegin(); v != it->value.MemberEnd(); ++v) {
	if (aliases_[it->name].HasMember(v->name)) {
	  typename ValueType::ConstMemberIterator existing = aliases_[it->name].FindMember(v->name);
	  if (existing->value != v->value) {
	    context.error_handler.ConflictingAliases(v->name, existing->value, v->value);
	    RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorConflictingAliases);
	  }
	} else {
	  aliases_[it->name].AddMember(ValueType(v->name.GetString(),
						 v->name.GetStringLength(),
						 GetAllocator()).Move(),
				       ValueType(v->value.GetString(),
						 v->value.GetStringLength(),
						 GetAllocator()).Move(),
				       GetAllocator());
	  ValueType primary;
	  if (!GetFinalAlias(context, aliases_[it->name], v->name, &primary))
	    return false;
	  // Check if previous parallel schema normalization included any of
	  // the aliased properties
	  ValueType* root = nullptr;
	  size_t unfinalized = 0;
	  if (document_.WasFinalized()) {
	    root = &document_;
	  } else {
	    unfinalized = 1;
	    if (document_.StackSize() == 0) {
	      std::cerr << "Address2Value: Empty document stack" << std::endl;
	      continue;
	    }
	    root = document_.StackTop();
	  }
	  PointerType base_ptr;
	  ValueType* base = Address2Value(it->name, root, &base_ptr,
					  unfinalized);
	  if (base && base->IsObject() && (base->HasMember(v->name))) {
	    typename ValueType::MemberIterator old = base->FindMember(v->name);
	    if (base->HasMember(primary)) {
	      typename ValueType::MemberIterator alt = base->FindMember(primary);
	      if (alt->value != old->value) {
		context.error_handler.DuplicateAlias(v->value, v->name);
		RAPIDJSON_INVALID_KEYWORD_RETURN(kNormalizeErrorAliasDuplicate);
	      }
	    } else {
	      *replaced = true;
	      RecordModifiedAlias(primary, v->name, &base_ptr);
	    }
	  }
	}
      }
    }
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
    GenericStringBuffer<EncodingType> address;
    GetInstanceRef(address, ((!extending_) || appending_));
    if (!aliases_.HasMember(address.GetString())) {
      aliases_.AddMember(ValueType(address.GetString(),
				   static_cast<SizeType>(address.GetSize() / sizeof(Ch)),
				   GetAllocator()).Move(),
			 kObjectType, GetAllocator());
    }
    return aliases_[address.GetString()];
  }
  const ValueType& AddAliases(const SchemaType* schema) {
    ValueType& aliases = GetAliases();
    if ((schema == nullptr) || (schema->child_aliases_.MemberCount() == 0))
      return aliases;
    for (typename SValue::ConstMemberIterator it = schema->child_aliases_.MemberBegin(); it != schema->child_aliases_.MemberEnd(); ++it) {
      if (!aliases.HasMember(it->name.GetString())) {
	aliases.AddMember(ValueType(it->name.GetString(),
				    it->name.GetStringLength(),
				    GetAllocator()).Move(),
			  ValueType(it->value.GetString(),
				    it->value.GetStringLength(),
				    GetAllocator()).Move(),
			  GetAllocator());
      }
    }
    return aliases;
  }

public:
  static void DisplayPointer(const PointerType& p) {
    GenericStringBuffer<EncodingType> sb;
    if (!p.IsValid())
      std::cerr << "DisplayPointer: invalid pointer" << std::endl;
    p.StringifyUriFragment(sb);
    std::cerr << sb.GetString();
  }
  void DisplayCurrentPointer() {
    DisplayPointer(GetInstancePointer());
  }
  template<typename VType>
  static void DisplayValue(VType& value
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
			   , bool pretty=false
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
			   ) {
    StringBuffer sb;
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    if (pretty) {
      PrettyWriter<StringBuffer, typename VType::EncodingType, UTF8<> > w(sb);
      value.Accept(w);
    } else {
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
      Writer<StringBuffer, typename VType::EncodingType, UTF8<> > w(sb);
      value.Accept(w);
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    }
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION
    std::cerr << (const char*)(sb.GetString());
  }
  void DisplayAliases() {
    DisplayValue(aliases_, true);
  }
  void DisplayModifications(internal::Stack<StackAllocatorType>* stack=nullptr) {
    if (!stack)
      stack = &modifiedStack_;
    std::cerr << "Modifications: " << std::endl;
    if (stack->Empty())
      return;
    GenericStringBuffer<EncodingType> sb;
    SizeType N = (SizeType)(stack->GetSize() / sizeof(ModificationEntry));
    ModificationEntry* it = stack->template Bottom<ModificationEntry>();
    for (SizeType i = 0; i < N; i++, it++) {
      std::cerr << "    ";
      // if (!it->no_before) {
      it->before.StringifyUriFragment(sb);
      std::cerr << sb.GetString();
      sb.Clear();
      // }
      if (it->no_before) {
	std::cerr << " (added)";
      }
      std::cerr << " -> ";
      // if (!it->no_after) {
      it->after.StringifyUriFragment(sb);
      std::cerr << sb.GetString();
      sb.Clear();
      // }
      if (it->no_after) {
	std::cerr << " (removed)";
      }
      std::cerr << std::endl;
    }
  }

  static const size_t kDefaultStackCapacity = 1024;
  DocumentType document_;
  unsigned index_;
  bool extending_;
  bool appending_;
  Context* extend_context_;
  const SchemaType* extend_schema_;
  internal::Stack<StackAllocatorType> keyStack_;
  internal::Stack<StackAllocatorType> valueStack_;
  internal::Stack<StackAllocatorType> childStack_;
  internal::Stack<StackAllocatorType> modifiedStack_;
  internal::Stack<StackAllocatorType> singularStack_;
  internal::Stack<StackAllocatorType> sharedStack_;
  internal::Stack<StackAllocatorType>* documentStack_;
  size_t tempSharedCount_;
  PairEntry* tempSharedStack_;
  ValueType aliases_;
  bool inSingular_;
  void* temporary_memory_;
  GenericNormalizedDocument* extend_child_;
  PointerType basePointer_;
  GenericNormalizedDocument* core_;

public:
#define PARAM_WITH_CONTEXT_(...)				\
  (Context& context, const SchemaType& schema, __VA_ARGS__)
#define ADD_NORMALIZE_HANDLER_(method, param, ...)			\
  bool method param {							\
    RAPIDJSON_ASSERT(extending_);					\
    return method(*extend_context_, *extend_schema_, __VA_ARGS__);	\
  }									\
  bool method PARAM_WITH_CONTEXT_ param {				\
    if ((!extending_) || appending_) {					\
      return Norm ## method(context, schema, __VA_ARGS__);		\
    } else {								\
      return Extend ## method(context, __VA_ARGS__);			\
    }									\
  }
#define ADD_NORMALIZE_HANDLER_NOARGS_(method)				\
  bool method() {							\
    RAPIDJSON_ASSERT(extending_);					\
    return method(*extend_context_, *extend_schema_);			\
  }									\
  bool method(Context& context, const SchemaType& schema) {		\
    if ((!extending_) || appending_) {					\
      return Norm ## method(context, schema);				\
    } else {								\
      return Extend ## method(context);					\
    }									\
  }
#define ADD_NORMALIZE_HANDLER_TEMPLATE_(method, param, ...)		\
  template <typename YggSchemaValueType>				\
  bool method param {							\
    RAPIDJSON_ASSERT(extending_);					\
    return method(*extend_context_, *extend_schema_, __VA_ARGS__);	\
  }									\
  template <typename YggSchemaValueType>				\
  bool method PARAM_WITH_CONTEXT_ param {				\
    if ((!extending_) || appending_) {					\
      return Norm ## method(context, schema, __VA_ARGS__);		\
    } else {								\
      return Extend ## method(context, __VA_ARGS__);			\
    }									\
  }

  ADD_NORMALIZE_HANDLER_NOARGS_(Null)
  ADD_NORMALIZE_HANDLER_(Bool, (bool b), b)
  ADD_NORMALIZE_HANDLER_(Int, (int i), i)
  ADD_NORMALIZE_HANDLER_(Uint, (unsigned u), u)
  ADD_NORMALIZE_HANDLER_(Int64, (int64_t i), i)
  ADD_NORMALIZE_HANDLER_(Uint64, (uint64_t u), u)
  ADD_NORMALIZE_HANDLER_(Double, (double d), d)
  ADD_NORMALIZE_HANDLER_(String, (const Ch* str, SizeType length, bool copy),
			 str, length, copy)
  ADD_NORMALIZE_HANDLER_TEMPLATE_(YggdrasilString,
				  (const Ch* str, SizeType length, bool copy,
				   YggSchemaValueType& valueSchema),
				  str, length, copy, valueSchema)
  ADD_NORMALIZE_HANDLER_TEMPLATE_(YggdrasilStartObject,
				  (YggSchemaValueType& valueSchema),
				  valueSchema)
  ADD_NORMALIZE_HANDLER_(YggdrasilEndObject, (SizeType memberCount),
			 memberCount)
  ADD_NORMALIZE_HANDLER_NOARGS_(StartObject)
  ADD_NORMALIZE_HANDLER_(Key, (const Ch* str, SizeType length, bool copy),
			 str, length, copy)
  ADD_NORMALIZE_HANDLER_(EndObject, (SizeType memberCount), memberCount)
  ADD_NORMALIZE_HANDLER_NOARGS_(StartArray)
  ADD_NORMALIZE_HANDLER_(EndArray, (SizeType elementCount), elementCount)

#undef ADD_NORMALIZE_HANDLER_
#undef ADD_NORMALIZE_HANDLER_NOARGS_
#undef ADD_NORMALIZE_HANDLER_TEMPLATE_
#undef PARAM_WITH_CONTEXT_

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
    friend class SharedProperties<SchemaDocumentType>;
    typedef GenericNormalizedDocument<SchemaDocumentType, RAPIDJSON_DEFAULT_STACK_ALLOCATOR> NormalizedDocumentType;
    typedef SharedProperties<SchemaDocumentType> SharedPropertiesType;
    typedef units::GenericUnits<EncodingType> UnitsType;
#endif // RAPIDJSON_YGGDRASIL

    Schema(SchemaDocumentType* schemaDocument, const PointerType& p, const ValueType& value, const ValueType& document, AllocatorType* allocator, const UriType& id = UriType()
#ifdef RAPIDJSON_YGGDRASIL
	   , const bool isMetaschema = false, const SingularFlag isSingular = kSingularNoFlags, const SchemaType* parentSchema = nullptr, const ValueType* parentKey = nullptr
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
	yggtype_((1 << kYggTotalSchemaType) - 1),
	subtype_(kYggNullSubType),
	precision_(),
	units_(),
	shape_(),
	class_(),
	args_(),
	kwargs_(),
	isMetaschema_(isMetaschema),
	inSort_(false),
	metaschema_(),
	metaschemaValidatorIndex_(),
	instance_(),
	instanceValidatorIndex_(),
	defaultSet_(false), default_(),
	aliases_(kArrayType), child_aliases_(kObjectType), hasAliases_(false),
	allowSingular_(false), isSingular_(isSingular), singularPtr_(),
	parentSchema_(parentSchema), parentKey_(kNullType),
	deprecated_(false), enumValues_(),
	sharedProperties_()
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

#ifdef RAPIDJSON_YGGDRASIL
	if (parentKey)
	  parentKey_.CopyFrom(*parentKey, *allocator_, true);
	if (const ValueType* v = GetMember(value, GetAliasesString())) {
	  if (v->IsArray())
	    aliases_.CopyFrom(*v, *allocator_, true);
	}
	if (schemaDocument && (isSingular_ != kSingularArray) && (isSingular_ != kSingularObject) && (!isMetaschema_)) {
	  SingularFlag kSingular = kSingularNoFlags;
	  if (type_ & (1 << kArraySchemaType))
	    kSingular = kSingularArray;
	  else if ((type_ & (1 << kObjectSchemaType)))
	    kSingular = kSingularObject;
	  AssignSingularIfExist(*schemaDocument, p, value, GetAllowSingularString(), document, kSingular, true);
	  if (allowSingularSchema_.schemas) {
	    schemaDocument->CreateSchema(&allowSingularSchema_.schemas[0], p, value, document, id_, kSingular);
	    RAPIDJSON_ASSERT(!allowSingularSchema_.schemas[0]->allowSingularSchema_.schemas);
	    // Reset types so that they are only evaluated within the nested
	    // schema
	    type_ = (1 << kTotalSchemaType) - 1;
	    yggtype_ = (1 << kYggTotalSchemaType) - 1;
	    return;
	  }
	}
	if (const ValueType* v = GetMember(value, GetDeprecatedString())) {
	  if (v->IsBool())
	    deprecated_.SetBool(v->GetBool());
	  else if (v->IsString())
	    deprecated_.SetString(v->GetString(), v->GetStringLength(), *allocator_);
	}
	// Initialize class before instance
	if (const ValueType* v = GetMember(value, GetPythonClassString())) {
	    if (v->IsPythonClass() || v->IsString())
	      class_.CopyFrom(*v, *allocator_, true);
	}
#endif // RAPIDJSON_YGGDRASIL
	
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
#ifdef RAPIDJSON_YGGDRASIL
		enumValues_ = v;
#endif // RAPIDJSON_YGGDRASIL
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
	    if ((yggtype_ & ((1 << kYggSchemaSchemaType) |
			     (1 << kYggPythonInstanceSchemaType))) &&
		!(type_ & (1 << kObjectSchemaType))) {
	      if (yggtype_ & (1 << kYggSchemaSchemaType)) {
		schemaDocument->CreateMetaSchema(&metaschema_);
		metaschemaValidatorIndex_ = validatorCount_;
		validatorCount_++;
	      } else {
		schemaDocument->CreateInstanceSchema(&instance_, class_, p);
		instanceValidatorIndex_ = validatorCount_;
		validatorCount_++;
	      }
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
						       *allocator_).Move(),
						SValue(itr->name.GetString(),
						       itr->name.GetStringLength(),
						       *allocator_).Move(),
						*allocator_);
		    }
		  }
#endif // RAPIDJSON_YGGDRASIL
            }
#ifdef RAPIDJSON_YGGDRASIL
	    child_aliases_.CopyFrom(child_aliases, *allocator_, true);
#endif // RAPIDJSON_YGGDRASIL	    
	}

        if (const ValueType* v = GetMember(value, GetPatternPropertiesString())) {
            PointerType q = p.Append(GetPatternPropertiesString(), allocator_);
            patternProperties_ = static_cast<PatternProperty*>(allocator_->Malloc(sizeof(PatternProperty) * v->MemberCount()));
            patternPropertyCount_ = 0;

            for (ConstMemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr) {
                new (&patternProperties_[patternPropertyCount_]) PatternProperty();
                patternProperties_[patternPropertyCount_].pattern = CreatePattern(itr->name);
#ifdef RAPIDJSON_YGGDRASIL
		patternProperties_[patternPropertyCount_].patternStr.SetString(itr->name.GetString(), itr->name.GetStringLength(), *allocator_);
#endif // RAPIDJSON_YGGDRASIL
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
	    defaultSet_ = true;
	    default_.CopyFrom(*v, *allocator_, true);
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
	        units_.CopyFrom(*v, *allocator_, true);
	}
	if (const ValueType* v = GetMember(value, GetLengthString())) {
	    if (v->IsNumber() && v->GetInt() > 0) {
	        shape_.SetArray();
		shape_.PushBack(static_cast<SizeType>(v->GetUint()), *allocator_);
	    }
	}
	if (const ValueType* v = GetMember(value, GetShapeString())) {
	    if (v->IsArray())
	        shape_.CopyFrom(*v, *allocator_);
	}
	if (const ValueType* v = GetMember(value, GetArgsString())) {
	    if (v->IsArray())
	        args_.CopyFrom(*v, *allocator_, true);
	}
	if (const ValueType* v = GetMember(value, GetKwargsString())) {
	    if (v->IsObject())
	        kwargs_.CopyFrom(*v, *allocator_, true);
	}
	// Push/pull shared properties
	const ValueType* pushProperties = GetMember(value, GetPushPropertiesString());
	const ValueType* pullProperties = GetMember(value, GetPullPropertiesString());
	if (pushProperties || pullProperties) {
	  if (pushProperties)
	    AssignSharedProperties(pushProperties, true);
	  if (pullProperties)
	    AssignSharedProperties(pullProperties);
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
#ifdef RAPIDJSON_YGGDRASIL
	if (sharedProperties_)
	  delete sharedProperties_;
#endif // RAPIDJSON_YGGDRASIL
    }

#ifdef RAPIDJSON_YGGDRASIL
    const SValue* GetDefaultValue() const {
      if (allowSingularSchema_.schemas)
	return allowSingularSchema_.schemas[0]->GetDefaultValue();
      if (defaultSet_) return &default_;
      return 0;
    }
#endif // RAPIDJSON_YGGDRASIL

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
        if (context.inArray) {
	    if (itemsList_) {
	        context.valuePointer = context.schemaPointerAbs.Append(GetItemsString(), allocator_);
            } else if (itemsTuple_) {
	        if ((context.arrayElementIndex - 1) < itemsTupleCount_) {
		    context.valuePointer = context.schemaPointerAbs.Append(GetItemsString(), allocator_).Append(context.arrayElementIndex - 1, allocator_);
                } else if (additionalItemsSchema_) {
		    context.valuePointer = context.schemaPointerAbs.Append(GetAdditionalItemsString(), allocator_);
		}
	    }
        }
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
#ifdef RAPIDJSON_YGGDRASIL
            context.error_handler.DisallowedValueEnum(*enumValues_);
#else // RAPIDJSON_YGGDRASIL
            context.error_handler.DisallowedValue(kValidateErrorEnum);
#endif // RAPIDJSON_YGGDRASIL
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
	      ValidateErrorCode code = kValidateErrorInvalidSchema;
	      context.error_handler.InvalidSchema(code, context.validators[metaschemaValidatorIndex_]);
	      RAPIDJSON_INVALID_KEYWORD_RETURN(code);
	    }

	    if (instance_ && !context.validators[instanceValidatorIndex_]->IsValid()) {
	      ValidateErrorCode code = kValidateErrorInvalidSchema;
	      context.error_handler.InvalidSchema(code, context.validators[instanceValidatorIndex_]);
	      RAPIDJSON_INVALID_KEYWORD_RETURN(code);
	    }

	    if (allowSingularSchema_.schemas) {
	      if (!((context.validators[allowSingularSchema_.begin]->IsValid()) ||
		    (context.validators[allowSingularSchema_.begin + 1]->IsValid()))) {
		ValidateErrorCode code = context.error_handler.NotSingularItem(&context.validators[allowSingularSchema_.begin]);
		RAPIDJSON_INVALID_KEYWORD_RETURN(code);
	      }
	    }

	    if (context.normalized) {

	      if (allOf_.schemas)
                for (SizeType i = 0; i < allOf_.count; i++)
		  if (!context.normalized->ExtendChild(context,
						       *allOf_.schemas[i],
						       context.validators[i + allOf_.begin]->GetValidatorID()))
		    return false;

	      if (anyOf_.schemas) {
                for (SizeType i = 0; i < anyOf_.count; i++) {
		  if (context.validators[i + anyOf_.begin]->IsValid()) {
		    if (!context.normalized->ExtendChild(context,
							 *anyOf_.schemas[i],
							 context.validators[i + anyOf_.begin]->GetValidatorID()))
		      return false;
		    break;
		  }
		}
	      }

	      if (oneOf_.schemas) {
                for (SizeType i = 0; i < oneOf_.count; i++) {
		  if (context.validators[i + oneOf_.begin]->IsValid()) {
		    if (!context.normalized->ExtendChild(context,
							 *oneOf_.schemas[i],
							 context.validators[i + oneOf_.begin]->GetValidatorID()))
		      return false;
		    break;
		  }
		}
	      }
	      
	      if (metaschema_)
		if (!context.normalized->ExtendChild(context,
						     *metaschema_,
						     context.validators[metaschemaValidatorIndex_]->GetValidatorID()))
		  return false;

	      if (instance_)
		if (!context.normalized->ExtendChild(context,
						     *instance_,
						     context.validators[instanceValidatorIndex_]->GetValidatorID()))
		  return false;

	      if (allowSingularSchema_.schemas) {
                for (SizeType i = 0; i < allowSingularSchema_.count; i++) {
		  if (context.validators[i + allowSingularSchema_.begin]->IsValid()) {
		    if (!context.normalized->ExtendChild(context,
							 *allowSingularSchema_.schemas[i],
							 context.validators[i + allowSingularSchema_.begin]->GetValidatorID()))
		      return false;
		    break;
		  }
		}
	      }
	      
	    }

	    // Warnings
	    if (allOf_.schemas)
	      context.error_handler.AddWarnings(&context.validators[allOf_.begin], allOf_.count);
	    
	    if (anyOf_.schemas)
	      for (SizeType i = anyOf_.begin; i < anyOf_.begin + anyOf_.count; i++)
		if (context.validators[i]->IsValid()) {
		  context.error_handler.AddWarnings(&context.validators[i], 1);
		  break;
		}

            if (oneOf_.schemas)
	      for (SizeType i = oneOf_.begin; i < oneOf_.begin + oneOf_.count; i++)
		if (context.validators[i]->IsValid()) {
		  context.error_handler.AddWarnings(&context.validators[i], 1);
		  break;
		}

	    if (metaschema_)
	      context.error_handler.AddWarnings(&context.validators[metaschemaValidatorIndex_], 1);

	    if (instance_)
	      context.error_handler.AddWarnings(&context.validators[instanceValidatorIndex_], 1);
	    
	    if (allowSingularSchema_.schemas)
	      for (SizeType i = allowSingularSchema_.begin; i < allowSingularSchema_.begin + allowSingularSchema_.count; i++)
		if (context.validators[i]->IsValid()) {
		  context.error_handler.AddWarnings(&context.validators[i], 1);
		  break;
		}
#endif // RAPIDJSON_YGGDRASIL
	    
        }

#ifdef RAPIDJSON_YGGDRASIL
	if (deprecated_.IsBool() && deprecated_.GetBool()) {
	  context.error_handler.DeprecationWarning();
	  RAPIDJSON_INVALID_KEYWORD_WARNING(kDeprecatedWarning);
	} else if (deprecated_.IsString()) {
	  context.error_handler.DeprecationWarning(&deprecated_);
	  RAPIDJSON_INVALID_KEYWORD_WARNING(kDeprecatedWarning);
	}
#endif // RAPIDJSON_YGGDRASIL
	
        return true;
    }
  
#ifdef RAPIDJSON_YGGDRASIL

  bool RequiresPython() const {
    if (!class_.IsNull() || isMetaschema_ ||
	(yggtype_ & ((1 << kYggPythonImportSchemaType) |
		     (1 << kYggSchemaSchemaType))))
      return true;
    if (properties_) {
      for (SizeType i = 0; i < propertyCount_; i++)
	if (properties_[i].schema->RequiresPython())
	  return true;
    }
    if (patternProperties_) {
      for (SizeType i = 0; i < patternPropertyCount_; i++)
	if (patternProperties_[i].schema->RequiresPython())
	  return true;
    }
    if ((additionalPropertiesSchema_) && (additionalPropertiesSchema_->RequiresPython()))
      return true;
    if ((not_) && (not_->RequiresPython()))
      return true;
    if ((additionalItemsSchema_) && (additionalItemsSchema_->RequiresPython()))
      return true;
    if ((itemsList_) && (itemsList_->RequiresPython()))
      return true;
    if (itemsTuple_) {
      for (SizeType i = 0; i < itemsTupleCount_; i++)
	if (itemsTuple_[i]->RequiresPython())
	  return true;
    }
    return false;
  }
  
#define RAPIDJSON_NORMALIZER_BASE_(method, arg)				\
  TemporaryMemory<typename Context::NormalizedDocumentType> __temporary_normalized_memory(context.normalized); \
  if (context.normalized) {						\
    if (!context.normalized->method arg)				\
      return false;							\
    __temporary_normalized_memory.stealMemory();			\
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
#ifdef RAPIDJSON_YGGDRASIL
        if (!((type_ & (1 << kStringSchemaType)) ||
	      (yggtype_ & (1 << kYggPythonImportSchemaType)))) {
#else // RAPIDJSON_YGGDRASIL
        if (!(type_ & (1 << kStringSchemaType))) {
#endif // RAPIDJSON_YGGDRASIL
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
	if ((yggtype_ & (1 << kYggPythonImportSchemaType)) &&
	    !(type_ & (1 << kStringSchemaType)))
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
#ifdef RAPIDJSON_YGGDRASIL
        if (!((type_ & (1 << kObjectSchemaType)) ||
	      (yggtype_ & ((1 << kYggPythonInstanceSchemaType) |
			   (1 << kYggSchemaSchemaType))))) {
#else // RAPIDJSON_YGGDRASIL
        if (!(type_ & (1 << kObjectSchemaType))) {
#endif // RAPIDJSON_YGGDRASIL
            DisallowedType(context, GetObjectString());
            RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorType);
        }

#ifdef RAPIDJSON_YGGDRASIL
        if (hasDependencies_ || hasRequired_ || sharedProperties_) {
#else // RAPIDJSON_YGGDRASIL
	if (hasDependencies_ || hasRequired_) {
#endif // RAPIDJSON_YGGDRASIL
            context.propertyExist = static_cast<bool*>(context.factory.MallocState(sizeof(bool) * propertyCount_));
            std::memset(context.propertyExist, 0, sizeof(bool) * propertyCount_);
        }

        if (patternProperties_) { // pre-allocate schema array
            SizeType count = patternPropertyCount_ + 1; // extra for valuePatternValidatorType
            context.patternPropertiesSchemas = static_cast<const SchemaType**>(context.factory.MallocState(sizeof(const SchemaType*) * count));
            context.patternPropertiesSchemaCount = 0;
            std::memset(context.patternPropertiesSchemas, 0, sizeof(SchemaType*) * count);
#ifdef RAPIDJSON_YGGDRASIL
	    context.patternPropertiesPointers = static_cast<PointerType*>(context.factory.MallocState(sizeof(PointerType) * count));
	    for (SizeType i = 0; i < count; i++)
	      new (&context.patternPropertiesPointers[i]) PointerType(allocator_);
#endif // RAPIDJSON_YGGDRASIL
        }

        return CreateParallelValidator(context);
    }

    bool Key(Context& context, const Ch* str, SizeType len, bool copy) const {
        RAPIDJSON_NORMALIZER_(Key, str, len, copy);
	(void)copy;
#ifdef RAPIDJSON_YGGDRASIL
	SValue dest;
	if (child_aliases_.HasMember(str)) {
	  SValue orig(str, len, *allocator_);
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
#ifdef RAPIDJSON_YGGDRASIL
	  PointerType pPattern = context.schemaPointerAbs.Append(SchemaType::GetPatternPropertiesString(),
								 allocator_);
#endif // RAPIDJSON_YGGDRASIL
            context.patternPropertiesSchemaCount = 0;
            for (SizeType i = 0; i < patternPropertyCount_; i++)
                if (patternProperties_[i].pattern && IsPatternMatch(patternProperties_[i].pattern, str, len)) {
                    context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = patternProperties_[i].schema;
                    context.valueSchema = typeless_;
#ifdef RAPIDJSON_YGGDRASIL
		    context.patternPropertiesPointers[context.patternPropertiesSchemaCount - 1] = pPattern.Append(i, allocator_);
#endif // RAPIDJSON_YGGDRASIL
                }
        }

        SizeType index  = 0;
        if (FindPropertyIndex(ValueType(str, len).Move(), &index)) {
#ifdef RAPIDJSON_YGGDRASIL
	    PointerType pProp = context.schemaPointerAbs.Append(SchemaType::GetPropertiesString(), allocator_).Append(str, len, allocator_);
#endif // RAPIDJSON_YGGDRASIL
            if (context.patternPropertiesSchemaCount > 0) {
                context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = properties_[index].schema;
                context.valueSchema = typeless_;
                context.valuePatternValidatorType = Context::kPatternValidatorWithProperty;
#ifdef RAPIDJSON_YGGDRASIL
		context.patternPropertiesPointers[context.patternPropertiesSchemaCount - 1] = pProp;
#endif // RAPIDJSON_YGGDRASIL
            }
            else
#ifdef RAPIDJSON_YGGDRASIL
	      {
#endif // RAPIDJSON_YGGDRASIL
                context.valueSchema = properties_[index].schema;
#ifdef RAPIDJSON_YGGDRASIL
		context.valuePointer = pProp;
	      }
#endif // RAPIDJSON_YGGDRASIL

            if (context.propertyExist)
                context.propertyExist[index] = true;

            return true;
        }

        if (additionalPropertiesSchema_) {
#ifdef RAPIDJSON_YGGDRASIL
	    PointerType pProp = context.schemaPointerAbs.Append(SchemaType::GetAdditionalPropertiesString(), allocator_);
#endif // RAPIDJSON_YGGDRASIL
            if (context.patternPropertiesSchemaCount > 0) {
                context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = additionalPropertiesSchema_;
                context.valueSchema = typeless_;
                context.valuePatternValidatorType = Context::kPatternValidatorWithAdditionalProperty;
#ifdef RAPIDJSON_YGGDRASIL
		context.patternPropertiesPointers[context.patternPropertiesSchemaCount - 1] = pProp;
#endif // RAPIDJSON_YGGDRASIL
            }
            else
#ifdef RAPIDJSON_YGGDRASIL
	      {
#endif // RAPIDJSON_YGGDRASIL
                context.valueSchema = additionalPropertiesSchema_;
#ifdef RAPIDJSON_YGGDRASIL
		context.valuePointer = pProp;
	      }
#endif // RAPIDJSON_YGGDRASIL
            return true;
        }
        else if (additionalProperties_) {
            context.valueSchema = typeless_;
            return true;
        }

        if (context.patternPropertiesSchemaCount == 0) { // patternProperties are not additional properties
#ifdef RAPIDJSON_YGGDRASIL
	    if (context.normalized &&
		sharedProperties_ &&
		sharedProperties_->isSrc(context.schemaPointerAbs, str, len)) {
	        context.valueSchema = typeless_;
		return true;
	    }
#endif // RAPIDJSON_YGGDRASIL
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
	    case kValidateErrorPythonClass:             return GetPythonClassString();
	    case kValidateErrorInvalidSchema:           return GetSchemaString();
	    case kNormalizeErrorAliasDuplicate:         return GetAliasesString();
	    case kNormalizeErrorCircularAlias:          return GetAliasesString();
	    case kNormalizeErrorConflictingAliases:     return GetAliasesString();
	    case kNormalizeErrorMergeConflict:          return GetNormalizationString();
	    case kDeprecatedWarning:                    return GetDeprecatedString();
	    case kValidateErrorGeneric:                 return GetGenericString();
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
    RAPIDJSON_STRING_(AllowSingular, 'a', 'l', 'l', 'o', 'w', 'S', 'i', 'n', 'g', 'u', 'l', 'a', 'r')
    RAPIDJSON_STRING_(Deprecated, 'd', 'e', 'p', 'r', 'e', 'c', 'a', 't', 'e', 'd')
    RAPIDJSON_STRING_(PushProperties, 'p', 'u', 's', 'h', 'P', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's')
    RAPIDJSON_STRING_(PullProperties, 'p', 'u', 'l', 'l', 'P', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's')
    // Subtypes
    RAPIDJSON_STRING_(StringSubType, 's', 't', 'r', 'i', 'n', 'g')
    RAPIDJSON_STRING_(IntSubType, 'i', 'n', 't')
    RAPIDJSON_STRING_(UintSubType, 'u', 'i', 'n', 't')
    RAPIDJSON_STRING_(FloatSubType, 'f', 'l', 'o', 'a', 't')
    RAPIDJSON_STRING_(ComplexSubType, 'c', 'o', 'm', 'p', 'l', 'e', 'x')
    // Normalization
    RAPIDJSON_STRING_(Normalization, 'n', 'o', 'r', 'm', 'a', 'l', 'i', 'z', 'a', 't', 'i', 'o', 'n')
    RAPIDJSON_STRING_(Bool, 'b', 'o', 'o', 'l')
    RAPIDJSON_STRING_(Double, 'd', 'o', 'u', 'b', 'l', 'e')
    RAPIDJSON_STRING_(Int, 'i', 'n', 't')
    RAPIDJSON_STRING_(Uint, 'u', 'i', 'n', 't')
    RAPIDJSON_STRING_(Int64, 'i', 'n', 't', '6', '4')
    RAPIDJSON_STRING_(Uint64, 'u', 'i', 'n', 't', '6', '4')
    RAPIDJSON_STRING_(YggdrasilObject, 'y', 'g', 'g', 'O', 'b', 'j', 'e', 'c', 't')
    RAPIDJSON_STRING_(YggdrasilString, 'y', 'g', 'g', 'S', 't', 'r', 'i', 'n', 'g')
    RAPIDJSON_STRING_(Generic, 'g', 'e', 'n', 'e', 'r', 'i', 'c')
    RAPIDJSON_STRING_(RelativeUp, '.', '.')
    RAPIDJSON_STRING_(Wildcard, '.', '*')
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

#ifdef RAPIDJSON_YGGDRASIL
  void AssignSingularIfExist(SchemaDocumentType& schemaDocument, const PointerType& p, const ValueType& value, const ValueType& name, const ValueType& document, const SingularFlag containerFlag, bool first_pass=false, bool force=false) {
    const ValueType* v = GetMember(value, name);
    bool deleteV = false;
    if (!v && force) {
      v = new ValueType(kObjectType); // Empty schema
      deleteV = true;
    }
    if (v) {
      if (first_pass) {
	if (v->IsBool() && v->GetBool()) {
	  allowSingular_ = true;
	  if (containerFlag == kSingularArray)
	    AssignSingularIfExist(schemaDocument, p, value, GetItemsString(), document, containerFlag);
	  else if (containerFlag == kSingularObject)
	    AssignSingularIfExistObject(schemaDocument, p, value, document);
	} else if (v->IsString() && (containerFlag == kSingularObject)) {
	  AssignSingularIfExistObject(schemaDocument, p, value, document, v);
	}
	return;
      }
      const ValueType* v0 = nullptr;
      PointerType q = p.Append(name, allocator_);
      if (v->IsObject()) {
	v0 = v;
      } else if (v->IsArray() && (v->Size() == 1)) {
	v0 = &((*v)[0]);
	q = q.Append(0, allocator_);
      }
      if (v0) {
	allowSingularSchema_.count = 2;
	allowSingularSchema_.schemas = static_cast<const Schema**>(allocator_->Malloc(allowSingularSchema_.count * sizeof(const Schema*)));
	memset(allowSingularSchema_.schemas, 0, sizeof(Schema*)* allowSingularSchema_.count);
	if (containerFlag == kSingularObject) {
	  schemaDocument.CreateSchema(&allowSingularSchema_.schemas[1], q, *v0, document, id_, kSingularValue, this, &name);
	} else {
	  schemaDocument.CreateSchema(&allowSingularSchema_.schemas[1], q, *v0, document, id_, kSingularItem, this);
	}
	allowSingularSchema_.begin = validatorCount_;
	validatorCount_ += allowSingularSchema_.count;
	singularPtr_ = q;
      }
    }
    if (deleteV)
      delete v;
  }
  void AssignSingularIfExistObject(SchemaDocumentType& schemaDocument, const PointerType& p, const ValueType& value, const ValueType& document,
				   const ValueType* prop0=nullptr) {
    const ValueType* properties = GetMember(value, GetPropertiesString());
    const ValueType* required = GetMember(value, GetRequiredString());
    bool force = (prop0 != nullptr);
    if (!prop0) {
      if (required && required->IsArray()) // && (required->Size() == 1))
	prop0 = &((*required)[0]);
      else if (properties && properties->IsObject() && (properties->MemberCount() == 1))
	prop0 = &(properties->MemberBegin()->name);
    }
    if (properties && prop0)
      AssignSingularIfExist(schemaDocument, p.Append(GetPropertiesString(), allocator_), *properties, *prop0, document, kSingularObject, false, force);
  }
#endif // RAPIDJSON_YGGDRASIL

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
    RegexType* CreatePattern(const ValueType& value) const {
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
    RegexType* CreatePattern(const ValueType& value) const {
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
    RegexType* CreatePattern(const ValueType&) const { return 0; }

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
	else if (type == GetScalarString()  ) yggtype_ |= 1 << kYggScalarSchemaType;
	else if (type == Get1DArrayString() ) yggtype_ |= 1 << kYggNDArraySchemaType;
	else if (type == GetNDArrayString() ) yggtype_ |= 1 << kYggNDArraySchemaType;
	else if (type == GetPythonClassString()    ) yggtype_ |= 1 << kYggPythonImportSchemaType;
	else if (type == GetPythonFunctionString() ) yggtype_ |= 1 << kYggPythonImportSchemaType;
	else if (type == GetPythonInstanceString() ) yggtype_ |= 1 << kYggPythonInstanceSchemaType;
	else if (type == GetObjString()    ) yggtype_ |= 1 << kYggObjSchemaType;
	else if (type == GetPlyString()    ) yggtype_ |= 1 << kYggPlySchemaType;
	else if (type == GetSchemaString() ) yggtype_ |= 1 << kYggSchemaSchemaType;
	else if (type == GetAnyString()    ) {
	  type_ |= ((1 << kTotalSchemaType) - 1);
	  yggtype_ |= ((1 << kYggTotalSchemaType) - 1);
	}
#endif // RAPIDJSON_YGGDRASIL
    }

#ifdef RAPIDJSON_YGGDRASIL
    bool PointerMatches(const PointerType pattern,
			const PointerType x,
			bool patternHasRegex,
			bool patternIsPrefix = false,
			bool verbose = false) const {
      if (!patternHasRegex) {
	if (patternIsPrefix)
	  return pattern.PartialCompare(x);
	return (pattern == x);
      }
      if (patternIsPrefix) {
	if (x.GetTokenCount() < pattern.GetTokenCount()) return false;
      } else {
	if (x.GetTokenCount() != pattern.GetTokenCount()) return false;
      }
      bool out = true;
      for (size_t i = 0; i < pattern.GetTokenCount(); i++) {
	if (x.GetTokens()[i].length == pattern.GetTokens()[i].length &&
	    (std::memcmp(x.GetTokens()[i].name,
			 pattern.GetTokens()[i].name,
			 x.GetTokens()[i].length * sizeof(Ch)) == 0))
	  continue;
	RegexType* patternReg = CreatePattern(SValue(pattern.GetTokens()[i].name, pattern.GetTokens()[i].length).Move());
	if (!patternReg) {
	  std::cerr << "PointerMatches: Invalid pattern \"" << pattern.GetTokens()[i].name << "\"" << std::endl;
	  out = false;
	  break;
	} else {
	  bool match = IsPatternMatch(patternReg,
				      x.GetTokens()[i].name,
				      x.GetTokens()[i].length);
#if RAPIDJSON_SCHEMA_HAS_REGEX
	  patternReg->~RegexType();
	  AllocatorType::Free(patternReg);
#endif
	  if (!match) {
	    out = false;
	    break;
	  }
	}
      }
      if (verbose) {
	std::cerr << "PointerMatches(";
	NormalizedDocumentType::DisplayPointer(pattern);
	std::cerr << ", ";
	NormalizedDocumentType::DisplayPointer(x);
	std::cerr << ", " << out << ")" << std::endl;
      }
      return out;
    }
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

#ifdef RAPIDJSON_YGGDRASIL
            // Always return after first failure for these sub-validators
            if (allOf_.schemas)
	        CreateSchemaValidators(context, allOf_, false,
				       GetAllOfString());

            if (anyOf_.schemas)
	        CreateSchemaValidators(context, anyOf_, false,
				       GetAnyOfString());

            if (oneOf_.schemas)
	        CreateSchemaValidators(context, oneOf_, false,
				       GetOneOfString());

            if (not_)
	        context.validators[notValidatorIndex_] = context.factory.CreateSchemaValidator(*not_, false, context.schemaPointerAbs.Append(GetNotString(), allocator_));

            if (hasSchemaDependencies_) {
	        PointerType pDepend = context.schemaPointerAbs.Append(GetDependenciesString(), allocator_);
                for (SizeType i = 0; i < propertyCount_; i++)
		    if (properties_[i].dependenciesSchema)
		        context.validators[properties_[i].dependenciesValidatorIndex] = context.factory.CreateSchemaValidator(*properties_[i].dependenciesSchema, false, pDepend.Append(properties_[i].name.GetString(), properties_[i].name.GetStringLength(), allocator_));
	    }
#else // RAPIDJSON_YGGDRASIL
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
#endif // RAPIDJSON_YGGDRASIL
	    
#ifdef RAPIDJSON_YGGDRASIL
	    if (metaschema_)
	      context.validators[metaschemaValidatorIndex_] = context.factory.CreateSchemaValidator(*metaschema_, false, context.schemaPointerAbs);
	    
	    if (instance_)
	      context.validators[instanceValidatorIndex_] = context.factory.CreateSchemaValidator(*instance_, false, context.schemaPointerAbs);
	    
	    if (allowSingularSchema_.schemas) {
	      context.validators[allowSingularSchema_.begin] = context.factory.CreateSchemaValidator(*allowSingularSchema_.schemas[0], false, context.schemaPointerAbs);
	      PointerType pSing(context.schemaPointerAbs, allocator_);
	      if (singularPtr_.StartsWith(pointer_)) {
		for (size_t i = pointer_.GetTokenCount();
		     i < singularPtr_.GetTokenCount(); i++)
		  pSing = pSing.Append(singularPtr_.GetTokens()[i], allocator_);
	      }
	      context.validators[allowSingularSchema_.begin + 1] = context.factory.CreateSchemaValidator(*allowSingularSchema_.schemas[1], false, pSing);
	    }
#endif // RAPIDJSON_YGGDRASIL
	    
        }

        return true;
    }

#ifdef RAPIDJSON_YGGDRASIL
    void CreateSchemaValidators(Context& context, const SchemaArray& schemas, const bool inheritContinueOnErrors, const ValueType& str) const {
        PointerType pGroup = context.schemaPointerAbs.Append(str, allocator_);
        for (SizeType i = 0; i < schemas.count; i++)
	    context.validators[schemas.begin + i] = context.factory.CreateSchemaValidator(*schemas.schemas[i], inheritContinueOnErrors, pGroup.Append(i, allocator_));
    }
#else // RAPIDJSON_YGGDRASIL
    void CreateSchemaValidators(Context& context, const SchemaArray& schemas, const bool inheritContinueOnErrors) const {
        for (SizeType i = 0; i < schemas.count; i++)
            context.validators[schemas.begin + i] = context.factory.CreateSchemaValidator(*schemas.schemas[i], inheritContinueOnErrors);
    }
#endif // RAPIDJSON_YGGDRASIL

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
    UnitsType expected_units(units_.GetString(),
			     units_.GetStringLength(),
			     false);
    UnitsType actual_units(actual->GetString(),
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
    for (typename YggSchemaValueType::ConstValueIterator v = vs->value.Begin(); v != vs->value.End(); ++v)
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
    if (class_.IsPythonClass() || class_.IsString()) {
      PyObject* pycls = import_python_object(reinterpret_cast<const char*>(class_.GetString()),
					     "CheckPythonImport", true);
      if (pycls && (PyObject_IsSubclass(pyobj, pycls) <= 0)) {
	Py_DECREF(pyobj);
	Py_DECREF(pycls);
	context.error_handler.InvalidPythonClass(str, length, class_);
	RAPIDJSON_INVALID_KEYWORD_RETURN(kValidateErrorPythonClass);
      }
      Py_XDECREF(pycls);
    }
    Py_DECREF(pyobj);
    return true;
  }

    class SharedProperty; //!< Forward declaration.
#endif // RAPIDJSON_YGGDRASIL
    struct Property {
        Property() : schema(), dependenciesSchema(), dependenciesValidatorIndex(), dependencies(), required(false)
#ifdef RAPIDJSON_YGGDRASIL
		   , sharedProperty()
#endif // RAPIDJSON_YGGDRASIL
      {}
        ~Property() { AllocatorType::Free(dependencies); }
        SValue name;
        const SchemaType* schema;
        const SchemaType* dependenciesSchema;
        SizeType dependenciesValidatorIndex;
        bool* dependencies;
        bool required;
#ifdef RAPIDJSON_YGGDRASIL
        SharedProperty* sharedProperty;
#endif // RAPIDJSON_YGGDRASIL
    };
    
    struct PatternProperty {
#ifdef RAPIDJSON_YGGDRASIL
        PatternProperty() : schema(), pattern(), patternStr() {}
#else // RAPIDJSON_YGGDRASIL
        PatternProperty() : schema(), pattern() {}
#endif // RAPIDJSON_YGGDRASIL
        ~PatternProperty() {
            if (pattern) {
                pattern->~RegexType();
                AllocatorType::Free(pattern);
            }
        }
        const SchemaType* schema;
        RegexType* pattern;
#ifdef RAPIDJSON_YGGDRASIL
        SValue patternStr;
#endif // RAPIDJSON_YGGDRASIL
    };

#ifdef RAPIDJSON_YGGDRASIL
    enum PointerOrderFlag {
      kPointerOrderFalse,
      kPointerOrderTrue,
      kPointerOrderNull
    };
    // Pull
    //   - Destination is local
    //   - Source is in links
    //   - Set destinations after all links visited
    // Push
    //   - Destinations are in links
    //   - Source is local
    //   - Set destinations after sources for that destination are set
    // Class organization:
    //   SharedProperties
    //       Contain all SharedProperty objects that originate
    //       or link to the owning schema.
    //   SharedProperty
    //       One for each rule in the schema for sharing properties. Contains
    //       links and instances.
    //   LinkEntry
    //       Provides pointer to linked schema for a rule
    //   InstanceEntry
    //       Information for one instance of a particular rule with a prefix
    //       and PairEntry objects for different links
    //   PairEntry
    //       Source & destination info for completing links during eval for
    //       a particular JSON document.
    class SharedPropertyBase {
    public:
      SharedPropertyBase(AllocatorType* allocator = 0) :
	schema(), index(0),
	local(false), source(false), hasRegex(false),
	ownProperties(false), propertyCount(0), properties(),
	instancePtr(allocator), schemaPtr(allocator) {}
      SharedPropertyBase(bool local0, bool source0,
			 SchemaType* schema0, size_t index0) :
	schema(schema0), index(index0),
	local(local0), source(source0), hasRegex(false),
	ownProperties(false), propertyCount(0), properties(),
	instancePtr(schema0->allocator_), schemaPtr(schema0->allocator_) {}
      SharedPropertyBase(bool local0, bool source0,
			 SchemaType* schema0, size_t index0,
			 const PointerType& path) :
	schema(schema0), index(index0),
	local(local0), source(source0), hasRegex(false),
	ownProperties(false), propertyCount(0), properties(),
	instancePtr(schema0->allocator_),
	schemaPtr(path, schema0->allocator_) {
	SetInstancePtr(schema0->allocator_);
      }
      void SetInstancePtr(AllocatorType* allocator = 0) {
	size_t invalidToken = schemaPtr.GetTokenCount() + 1;
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	  std::cerr << "SharedPropertyBase: ";
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	instancePtr = SchemaPointer2InstancePointer(schemaPtr,
						    hasRegex,
						    &invalidToken,
						    allocator);
	if (invalidToken != (schemaPtr.GetTokenCount() + 1)) {
	  std::cerr << "SharedPropertyBase: Error converting schema pointer to instance, failed at token " << invalidToken << " of \"";
	  NormalizedDocumentType::DisplayPointer(schemaPtr);
	  std::cerr << "\"" << std::endl;
	}
      }
      ~SharedPropertyBase() {
	if (ownProperties && properties) {
	  for (SizeType i = 0; i < propertyCount; i++)
	    (properties + i)->~PropertyEntry();
	  AllocatorType::Free(properties);
	}
      }
      struct PropertyEntry {
	PropertyEntry() :
	  name(), base(), inSource(false) {}
	PropertyEntry(const Ch* str,
		      SizeType length,
		      AllocatorType& allocator,
		      bool inSource0 = false) :
	  name(str, length, allocator),
	  base(), inSource(inSource0) {}
	void Display() const {
	  std::cerr << name.GetString();
	}
	const SValue* GetDefault() const {
	  if (base && base->schema)
	    return base->schema->GetDefaultValue();
	  return 0;
	}
	SValue name;
	Property* base;
	bool inSource;
      };
      void Display() const {
	NormalizedDocumentType::DisplayPointer(schemaPtr);
      }
      void AddPropertiesRef(SharedPropertyBase* parent) {
	RAPIDJSON_ASSERT(parent->properties && !properties);
	ownProperties = false;
	propertyCount = parent->propertyCount;
	properties = parent->properties;
      }
      void AddPropertiesCpy(bool allProperties, const SValue& propertyNames,
			    const SchemaType* src = 0) {
	if (!src) src = schema;
	ownProperties = true;
	if (allProperties)
	  propertyCount = src->propertyCount_;
	else
	  propertyCount = propertyNames.Size();
	RAPIDJSON_ASSERT(!properties);
	properties = static_cast<PropertyEntry*>(src->allocator_->Malloc(propertyCount * sizeof(PropertyEntry)));
	SizeType j = 0;
	for (SizeType i = 0; i < src->propertyCount_; i++) {
	  if (allProperties ||
	      propertyNames.Contains(src->properties_[i].name)) {
	    new (&properties[j]) PropertyEntry(src->properties_[i].name.GetString(),
					       src->properties_[i].name.GetStringLength(),
					       *src->allocator_);
	    properties[j].base = &(src->properties_[i]);
	    j++;
	  }
	}
	propertyCount = j;
      }
      void SetInSource(const SchemaType* src) {
	for (SizeType i = 0; i < src->propertyCount_; i++) {
	  PropertyEntry* match = FindProperty(src->properties_[i].name);
	  if (match)
	    match->inSource = true;
	}
      }
      template<typename VT>
      const PropertyEntry* FindProperty(const VT& name) const {
	return FindProperty(name.GetString(), name.GetStringLength());
      }
      const PropertyEntry* FindProperty(const Ch* str, SizeType length) const {
	for (SizeType i = 0; i < propertyCount; i++) {
	  if (length == properties[i].name.GetStringLength() &&
	      std::memcmp(str, properties[i].name.GetString(),
			  length * sizeof(Ch)) == 0)
	    return &properties[i];
	}
	return nullptr;
      }
      template<typename VT>
      PropertyEntry* FindProperty(const VT& name) {
	return FindProperty(name.GetString(), name.GetStringLength());
      }
      PropertyEntry* FindProperty(const Ch* str, SizeType length) {
	for (SizeType i = 0; i < propertyCount; i++) {
	  if (length == properties[i].name.GetStringLength() &&
	      std::memcmp(str, properties[i].name.GetString(),
			  length * sizeof(Ch)) == 0)
	    return &properties[i];
	}
	return nullptr;
      }
      bool Matches(const PointerType x, bool checkInstance = false,
		   bool verbose = false) const {
	if (checkInstance)
	  return schema->PointerMatches(instancePtr, x, hasRegex,
					false, verbose);
	return schema->PointerMatches(schemaPtr, x, false, false, verbose);
      }
      bool PointerMatches(const PointerType pattern,
			  const PointerType x,
			  bool patternHasRegex,
			  bool patternIsPrefix = false,
			  bool verbose = false) const {
	return schema->PointerMatches(pattern, x, patternHasRegex,
				      patternIsPrefix, verbose);
      }
      SchemaType* schema;
      size_t index;
      bool local;
      bool source;
      bool hasRegex;
      bool ownProperties;
      SizeType propertyCount;
      PropertyEntry* properties;
      PointerType instancePtr;
      PointerType schemaPtr;
    };
    class SharedProperty : public SharedPropertyBase {
    public:
      typedef typename NormalizedDocumentType::ValueType NormValueType;
      typedef typename SharedPropertyBase::PropertyEntry PropertyEntry;
      SharedProperty(AllocatorType* allocator = 0,
		     size_t stackCapacity = kDefaultStackCapacity) :
	SharedPropertyBase(allocator),
	push(false), allProperties(false), relative(false), inSchema(false),
	path(),
	propertyNames(kArrayType),
	ptr(allocator),
	currentInstance(0),
	instances(allocator, stackCapacity) {}
      SharedProperty(const SValue& path0, SchemaType* schema0,
		     size_t index0, bool push0 = false,
		     size_t stackCapacity = kDefaultStackCapacity) :
	SharedPropertyBase(true, push0, schema0, index0),
	push(push0), allProperties(false), relative(false), inSchema(false),
	path(path0.GetString(), path0.GetStringLength(),
	     *schema0->allocator_),
	propertyNames(kArrayType),
	ptr(schema0->allocator_),
	currentInstance(0),
	instances(schema0->allocator_, stackCapacity) {
	inSchema = (path.GetStringLength() > 0 &&
		    path.GetString()[0] == '!');
	relative = !((path.GetStringLength() > 0 &&
		      path.GetString()[0] == '/') ||
		     (inSchema &&
		      path.GetStringLength() > 1 &&
		      path.GetString()[1] == '/'));
	ptr = PointerType::FromRelative(path.GetString(),
					path.GetStringLength(),
					*schema0->allocator_);
      }
      ~SharedProperty() {
	while (!instances.Empty())
	  instances.template Pop<InstanceEntry>(1)->~InstanceEntry();
      }
      struct InstanceEntry; // forward declaration
      void Update(const ValueType* v) {
	if (v->IsBool()) {
	  if (v->GetBool()) {
	    allProperties = true;
	    if (!push)
	      AddPropertiesPull();
	  }
	} else if (v->IsArray() && v->Size() > 0) {
	  propertyNames.CopyFrom(*v, *this->schema->allocator_);
	  if (!push)
	    AddPropertiesPull();
	}
      }
      struct LinkEntry : public SharedPropertyBase {
	LinkEntry(AllocatorType* allocator = 0) :
	  SharedPropertyBase(allocator) {}
	LinkEntry(SchemaType* schema0, size_t index0,
		  const PointerType& path,
		  SharedProperty* parentProperty) :
	  SharedPropertyBase(false, !parentProperty->push,
			     schema0, index0, path) {
	  if (parentProperty->push)
	    AddPropertiesPush(parentProperty);
	  else
	    AddPropertiesPull(parentProperty);
	}
	void AddPropertiesPull(SharedProperty* parent) {
	  RAPIDJSON_ASSERT(!parent->push && parent->properties);
	  this->AddPropertiesRef(parent);
	  this->SetInSource(this->schema);
	}
	void AddPropertiesPush(SharedProperty* parent) {
	  // Should only be run for links in push
	  RAPIDJSON_ASSERT(parent->push);
	  this->AddPropertiesCpy(parent->allProperties,
				 parent->propertyNames,
				 this->schema);
	  this->SetInSource(parent->schema);
	}
      };
      struct InstanceEntry : public SharedPropertyBase {
	InstanceEntry(AllocatorType* allocator = 0,
		      size_t stackCapacity = 0) :
	  SharedPropertyBase(allocator),
	  first(kPointerOrderNull),
	  currentFlag(false),
	  currentMatchLocal(false), currentMatchLinks(false),
	  parentProperty(0),
	  prefix(allocator),
	  propertyNames(kArrayType), // HERE INSTANCE
	  links(allocator, stackCapacity) {}
	InstanceEntry(SharedProperty* parentProperty0,
		      const PointerType& rootPath,
		      AllocatorType* allocator = 0,
		      size_t stackCapacity = kDefaultStackCapacity) :
	  SharedPropertyBase(true, parentProperty0->push,
			     parentProperty0->schema,
			     parentProperty0->index, rootPath),
	  first(kPointerOrderNull),
	  currentFlag(false),
	  currentMatchLocal(false), currentMatchLinks(false),
	  parentProperty(parentProperty0),
	  prefix(allocator),
	  propertyNames(kArrayType), // HERE INSTANCE
	  links(allocator, stackCapacity) {
	  if (parentProperty->properties)
	    this->AddPropertiesRef(parentProperty);
	}
	~InstanceEntry() {
	  while (!links.Empty())
	    links.template Pop<LinkEntry>(1)->~LinkEntry();
	}
        void DisplayLinks() const {
	  std::cerr << "[";
	  for (const LinkEntry* it = LinksBegin(); it != LinksEnd(); it++) {
	    std::cerr << std::endl << "        ";
	    it->Display();
	  }
	  std::cerr << "]";
	}
	void Display() const {
	  std::cerr << "InstanceEntry(" << std::endl << "      links = ";
	  DisplayLinks();
	  std::cerr << ")";
	}
        size_t NLinks() const {
	  return (links.GetSize() / sizeof(LinkEntry));
	}
        LinkEntry* LinksBegin() {
	  return links.template Bottom<LinkEntry>();
        }
        LinkEntry* LinksEnd() {
	  return LinksBegin() + NLinks();
        }
        const LinkEntry* LinksBegin() const {
	  return links.template Bottom<LinkEntry>();
        }
        const LinkEntry* LinksEnd() const {
	  return LinksBegin() + NLinks();
        }
        void AddMissingPtr(const PointerType& p, SValue* missing,
			   AllocatorType* allocator) const {
	  GenericStringBuffer<EncodingType> sb;
	  if (!p.IsValid())
	    std::cerr << "AddMissingPtr: invalid pointer" << std::endl;
	  RAPIDJSON_ASSERT(allocator);
	  if (!allocator) return;
	  p.StringifyUriFragment(sb);
	  missing->PushBack(SValue(sb.GetString(), (SizeType)sb.GetLength(),
				   *allocator).Move(),
			    *allocator);
	}
	bool HasProperty(const SValue& name) const {
	  return propertyNames.Contains(name); // HERE INSTANCE
	}
	bool Matches(const PointerType& x, bool local0,
		     bool checkInstance = false) const {
	  if (local0) {
	    return SharedPropertyBase::Matches(x, checkInstance);
	  } else {
	    for (const LinkEntry* it = LinksBegin(); it != LinksEnd(); it++)
	      if (it->Matches(x, checkInstance))
		return true;
	  }
	  return false;
	}
	void SetCurrentPtr(const PointerType& x, bool checkInstance = false) {
	  currentMatchLocal = Matches(x, true, checkInstance);
	  currentMatchLinks = Matches(x, false, checkInstance);
	}
	void SetSiblingCurrentPtr(const PointerType& x, bool checkInstance = false) {
	  // Links only
	  for (const LinkEntry* it = LinksBegin(); it != LinksEnd(); it++)
	    it->schema->sharedProperties_->SetSiblingCurrentPtr(x,
								parentProperty->isSrc(true),
								false,
								it->index, checkInstance);
	}
	void SetSiblingMembers(const SValue& name,
			       const NormValueType* value,
			       NormalizedDocumentType& normalized) {
	  if (!((!parentProperty->push && currentMatchLinks) ||
		(parentProperty->push && currentMatchLocal)))
	    return;
	  if (parentProperty->isLocal(false))
	    parentProperty->schema->sharedProperties_->SetSiblingMembers(name, value, normalized, this->instancePtr, true, parentProperty->index);
	  else {
	    for (const LinkEntry* it = LinksBegin(); it != LinksEnd(); it++)
	      it->schema->sharedProperties_->SetSiblingMembers(name, value, normalized, it->instancePtr, false, it->index);
	  }
	}
	LinkEntry* AddLink(SchemaType* iSchema, size_t index0,
			   const PointerType& path) {
	  bool firstLink = links.Empty();
	  LinkEntry* ref = links.template Push<LinkEntry>();
	  new (ref) LinkEntry(iSchema, index0, path, parentProperty);
	  if (firstLink) {
	    size_t nMatch = this->instancePtr.CountMatchingTokens(ref->instancePtr);
	    prefix = this->instancePtr.PartialFront(nMatch, iSchema->allocator_);
	    first = iSchema->PointersOrdered(this->schemaPtr, ref->schemaPtr);
	  } else {
	    RAPIDJSON_ASSERT(prefix.PartialCompare(ref->instancePtr));
	    RAPIDJSON_ASSERT(LinksBegin()->instancePtr == ref->instancePtr);
	    RAPIDJSON_ASSERT(first == iSchema->PointersOrdered(this->schemaPtr, ref->schemaPtr));
	  }
	  // HERE INSTANCE
	  for (SizeType i = 0; i < ref->propertyCount; i++) {
	    if (firstLink || !propertyNames.Contains(ref->properties[i].name))
	      propertyNames.PushBack(SValue(ref->properties[i].name,
					    *iSchema->allocator_, true).Move(),
				     *iSchema->allocator_);
	  }
	  return ref;
	}
	void AddMissingObject(PointerType instancePtr0,
			      NormalizedDocumentType& normalized,
			      bool local0) {
	  if ((first == kPointerOrderTrue && !local0) ||
	      (first == kPointerOrderFalse && local0) ||
	      (first == kPointerOrderNull))
	    return;
	  SharedPropertyBase* pairProperty = 0;
	  SharedPropertyBase* partnerProperty = 0;
	  bool multiplePairs = false;
	  bool multiplePartners = false;
	  if (local0) {
	    RAPIDJSON_ASSERT(NLinks() > 0);
	    multiplePartners = NLinks() > 1;
	    pairProperty = this;
	    partnerProperty = LinksBegin();
	  } else {
	    multiplePairs = NLinks() > 1;
	    pairProperty = LinksBegin();
	    partnerProperty = this;
	  }
	  normalized.AddMissingObject(prefix, instancePtr0,
				      pairProperty, multiplePairs,
				      partnerProperty, multiplePartners);
	}
	void AddObject(PointerType instancePtr0, PointerType schemaPtr0,
		       NormalizedDocumentType& normalized,
		       const SValue& props, bool local0, bool source0) {
	  // Don't add object if a pair has already been created
	  if ((first == kPointerOrderTrue && !local0) ||
	      (first == kPointerOrderFalse && local0))
	    return;
	  else if (first == kPointerOrderNull) {
	    // TODO: Additional checks?
	    typename NormalizedDocumentType::PairEntry* pair = 0;
	    if (local0)
	      pair = normalized.FindSharedPair(schemaPtr0, source0);
	    else
	      pair = normalized.FindSharedPair(this->schemaPtr, !source0);
	    if (pair) return;
	  }
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	  std::cerr << "AddObject [";
	  NormalizedDocumentType::DisplayPointer(instancePtr0);
	  std::cerr << ", ";
	  NormalizedDocumentType::DisplayPointer(schemaPtr0);
	  std::cerr << "]: expected = ";
	  NormalizedDocumentType::DisplayValue(propertyNames); // HERE INSTANCE
	  std::cerr << ", existing = ";
	  NormalizedDocumentType::DisplayValue(props);
	  std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	  SharedPropertyBase* pairProperty = 0;
	  SharedPropertyBase* partnerProperty = 0;
	  bool multiplePartners = false;
	  if (local0) {
	    RAPIDJSON_ASSERT(NLinks() > 0);
	    multiplePartners = NLinks() > 2;
	    pairProperty = this;
	    partnerProperty = LinksBegin();
	  } else {
	    size_t nLinkMatched = 0;
	    for (LinkEntry* it = LinksBegin(); it != LinksEnd(); it++) {
	      if (parentProperty->PointerMatches(it->schemaPtr,
						 schemaPtr0,
						 false)) {
		pairProperty = it;
		partnerProperty = this;
		nLinkMatched++;
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
		std::cerr << "AddObject (LINK MATCH): ";
		NormalizedDocumentType::DisplayPointer(schemaPtr0);
		std::cerr << " vs ";
		NormalizedDocumentType::DisplayPointer(it->schemaPtr);
		std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	      }
	    }
	    if (nLinkMatched != 1) {
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	      std::cerr << "AddObject (NO LINK MATCHED): ";
	      NormalizedDocumentType::DisplayPointer(schemaPtr0);
	      std::cerr << " vs [";
	      for (LinkEntry* it = LinksBegin(); it != LinksEnd(); it++) {
		if (it != LinksBegin())
		  std::cerr << ", ";
		NormalizedDocumentType::DisplayPointer(it->schemaPtr);
	      }
	      std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
	      return; // Don't create/add to a pair
	    }
	  }
	  RAPIDJSON_ASSERT(source0 == pairProperty->source &&
			   !source0 == partnerProperty->source);
	  normalized.AddSharedObject(prefix, instancePtr0, schemaPtr0,
				     props, pairProperty, partnerProperty,
				     multiplePartners);
	}
	PointerOrderFlag first;
	bool currentFlag;
	bool currentMatchLocal;
	bool currentMatchLinks;
	SharedProperty* parentProperty;
	PointerType prefix;
	SValue propertyNames; // HERE INSTANCE
        internal::Stack<AllocatorType> links;
      };
      void DisplayInstances() const {
	std::cerr << "[";
	for (const InstanceEntry* it = InstsBegin(); it != InstsEnd(); it++) {
	  if (it != InstsBegin())
	    std::cerr << ", ";
	  it->Display();
	}
	std::cerr << "]";
      }
      void Display() const {
	std::cerr << "SharedProperty(\"" << path.GetString() << "\", ";
	if (push)
	  std::cerr << "push";
	else
	  std::cerr << "pull";
	std::cerr << ", properties = [";
	for (SizeType i = 0; i < this->propertyCount; i++) {
	  const PropertyEntry* it = &this->properties[i];
	  if (i > 0)
	    std::cerr << ", ";
	  it->Display();
	}
	std::cerr << "], instances = ";
	DisplayInstances();
	std::cerr << ")";
      }
      size_t NInsts() const {
	return (instances.GetSize() / sizeof(InstanceEntry));
      }
      InstanceEntry* InstsBegin() {
	return instances.template Bottom<InstanceEntry>();
      }
      InstanceEntry* InstsEnd() {
	return InstsBegin() + NInsts();
      }
      const InstanceEntry* InstsBegin() const {
	return instances.template Bottom<InstanceEntry>();
      }
      const InstanceEntry* InstsEnd() const {
	return InstsBegin() + NInsts();
      }
      bool isDst(bool local0) const {
	return ((local0 && !push) || (!local0 && push));
      }
      bool isSrc(bool local0) const {
	return (!isDst(local0));
      }
      bool isLocal(bool source0) const {
	return ((source0 && push) || (!source0 && !push));
      }
      bool Matches(const PointerType& x, bool source0,
		   bool checkInstance = false) const {
	bool local0 = isLocal(source0);
	for (const InstanceEntry* it = InstsBegin(); it != InstsEnd(); it++)
	  if (it->Matches(x, local0, checkInstance))
	    return true;
	return false;
      }
      bool HasProperty(const SValue& name) const {
	if (!allProperties)
	  return propertyNames.Contains(name);
	if (!push)
	  return (bool)(this->FindProperty(name));
	for (const InstanceEntry* it = InstsBegin(); it != InstsEnd(); it++)
	  if (it->HasProperty(name))
	    return true;
	return false;
      }
      bool HasProperty(const Ch* str, SizeType length) const {
	return HasProperty(SValue(str, length).Move());
      }
      void AddPropertiesPull() {
	RAPIDJSON_ASSERT(!push && this->schema && !this->properties);
	this->AddPropertiesCpy(allProperties, propertyNames);
      }
      InstanceEntry* AddInstance(const PointerType& schemaPtr0) {
	InstanceEntry* ref = instances.template Push<InstanceEntry>();
	new (ref) InstanceEntry(this, schemaPtr0, this->schema->allocator_);
	return ref;
      }
      void SetCurrentPtr(const PointerType& x, bool checkInstance = false) {
	for (InstanceEntry* it = InstsBegin(); it != InstsEnd(); it++)
	  it->SetCurrentPtr(x, checkInstance);
      }
      void SetSiblingCurrentPtr(const PointerType& x, bool checkInstance = false) {
	this->schema->sharedProperties_->SetSiblingCurrentPtr(x, isSrc(false),
							      true,
							      this->index,
							      checkInstance);
	for (InstanceEntry* it = InstsBegin(); it != InstsEnd(); it++)
	  it->SetSiblingCurrentPtr(x, checkInstance);
      }
      void SortSources(const SchemaType* root,
		       const PointerType& rootPath) {
	currentInstance = AddInstance(rootPath);
	PointerType localPtr(ptr, this->schema->allocator_);
	if (relative) {
	  if (inSchema)
	    localPtr = ptr.RelativeTo(currentInstance->schemaPtr,
				      this->schema->allocator_);
	  else
	    localPtr = ptr.RelativeTo(currentInstance->instancePtr,
				      this->schema->allocator_);
	}
	if (inSchema) {
	  const SchemaType* s = SchemaType::Get(root, localPtr);
	  if (!s) {
	    std::cerr << "SortSources: Could not find schema document" << std::endl;
	    instances.template Pop<InstanceEntry>(1)->~InstanceEntry();
	  } else {
	    const_cast<SchemaType*>(s)->AddSharedPropertyLink(PointerType(),
							      localPtr,
							      this);
	  }
	} else {
	  const_cast<SchemaType*>(root)->AddSharedPropertyLink(localPtr,
							       PointerType(),
							       this);
	}
	if (NInsts() == 0) {
	  std::cerr << "SortSources: No instances found ";
	  Display();
	  std::cerr << std::endl;
	}
      }
      void AddObject(PointerType instancePtr0, PointerType schemaPtr0,
		     NormalizedDocumentType& normalized,
		     const SValue& props, bool local0) {
	bool source0 = isSrc(local0);
	for (InstanceEntry* it = InstsBegin(); it != InstsEnd(); it++) {
	  it->currentFlag = (it->NLinks() > 0);
	  if (it->currentFlag && ((local0 && it->currentMatchLocal) ||
				  (!local0 && it->currentMatchLinks)))
	    it->AddObject(instancePtr0, schemaPtr0, normalized,
			  props, local0, source0);
	  it->currentFlag = (it->currentFlag &&
			     (it->currentMatchLocal || it->currentMatchLinks));
	}
      }
      void AddMissingObject(PointerType instancePtr0,
			    NormalizedDocumentType& normalized,
			    bool local0) {
	for (InstanceEntry* it = InstsBegin(); it != InstsEnd(); it++) {
	  if (it->NLinks() == 0 ||
	      (local0 && !it->Matches(instancePtr0, true, true)) ||
	      (!local0 && !it->Matches(instancePtr0, false, true)))
	    continue;
	  it->AddMissingObject(instancePtr0, normalized, local0);
	}
      }
      static const size_t kDefaultStackCapacity = 128;
      bool push;
      bool allProperties;
      bool relative;
      bool inSchema;
      SValue path;
      SValue propertyNames;
      PointerType ptr;
      InstanceEntry* currentInstance;
      internal::Stack<AllocatorType> instances;
    };
#define NESTED_FUNCTION_CALL_APPEND(method, ptr, path, var, name, ...)	\
    if (var.schemas) {							\
      PointerType varPath = path.Append(Get ## name ## String().GetString(), \
					Get ## name ## String().GetStringLength(), \
					allocator_);			\
      for (SizeType i = 0; i < var.count; i++)				\
	method(ptr, varPath.Append(i, allocator_), __VA_ARGS__,		\
	       var.schemas[i], true);					\
    }
#define NESTED_FUNCTION_CALL(method, ptr, path, ...)			\
    if (ptr.GetTokenCount() > 0) {					\
      RAPIDJSON_ASSERT(!(ptr.GetTokens()[0].length == 2 &&		\
			 ptr.GetTokens()[0].name[0] == '.' &&		\
			 ptr.GetTokens()[0].name[1] == '.'));		\
      if (ptr.GetTokens()[0].length == 2 &&				\
	  ptr.GetTokens()[0].name[0] == '.' &&				\
	  ptr.GetTokens()[0].name[1] == '.') {				\
	std::cerr << #method << ": parent disallowed" << std::endl;	\
	return;								\
      }									\
    }									\
    NESTED_FUNCTION_CALL_APPEND(method, ptr, path, allOf_, AllOf,	\
				__VA_ARGS__);				\
    NESTED_FUNCTION_CALL_APPEND(method, ptr, path, anyOf_, AnyOf,	\
				__VA_ARGS__);				\
    NESTED_FUNCTION_CALL_APPEND(method, ptr, path, oneOf_, OneOf,	\
				__VA_ARGS__);				\
    if (allowSingularSchema_.schemas) {					\
      method(ptr, path, __VA_ARGS__, allowSingularSchema_.schemas[0], true); \
      if (false) {							\
      if (allowSingular_) {						\
	if (type_ & (1 << kArraySchemaType)) {				\
	  PointerType itemsPath = path.Append(GetItemsString().GetString(), \
					      GetItemsString().GetStringLength(), \
					      allocator_);		\
	  if (itemsList_) {						\
	    method(ptr, itemsPath,					\
		   __VA_ARGS__, allowSingularSchema_.schemas[1], true);	\
	  } else if (itemsTuple_) {					\
	    method(ptr, itemsPath.Append(0, allocator_),		\
		   __VA_ARGS__, allowSingularSchema_.schemas[1], true);	\
	  }								\
	}								\
	if (type_ & (1 << kObjectSchemaType)) {				\
	  PointerType propertiesPath = path.Append(GetPropertiesString().GetString(), \
						   GetPropertiesString().GetStringLength(), \
						   allocator_);		\
	  method(ptr, propertiesPath.Append(allowSingularSchema_.schemas[1]->parentKey_.GetString(), \
					    allowSingularSchema_.schemas[1]->parentKey_.GetStringLength(), \
					    allocator_),		\
		 __VA_ARGS__, allowSingularSchema_.schemas[1], true);	\
	}								\
      } else {								\
	method(ptr, path.Append(GetAllowSingularString().GetString(),	\
				GetAllowSingularString().GetStringLength(), \
				allocator_),				\
	       __VA_ARGS__, allowSingularSchema_.schemas[1], true);	\
      }									\
      }									\
    }									\
    if (ptr.GetTokenCount() > 0) {					\
      SizeType index;							\
      ValueType name;							\
      if (ptr.GetTokens()[0].index == kPointerInvalidIndex)		\
	name.SetString(ptr.GetTokens()[0].name,				\
		       ptr.GetTokens()[0].length);			\
      else								\
	name.SetUint(ptr.GetTokens()[0].index);				\
      if (name.IsString()) {						\
	bool patternMatched = false;					\
	if (patternProperties_) {					\
	  PointerType patternPath = path.Append(GetPatternPropertiesString().GetString(), \
						GetPatternPropertiesString().GetStringLength(),	\
						allocator_);		\
	  for (SizeType i = 0; i < patternPropertyCount_; i++) {	\
	    if (patternProperties_[i].pattern &&			\
		IsPatternMatch(patternProperties_[i].pattern,		\
			       name.GetString(),			\
			       name.GetStringLength())) {		\
	      method(ptr, patternPath.Append(name.GetString(),		\
					     name.GetStringLength(),	\
					     allocator_),		\
		     __VA_ARGS__, patternProperties_[i].schema);	\
	      patternMatched = true;					\
	    }								\
	  }								\
	}								\
	PointerType propertiesPath = path.Append(GetPropertiesString().GetString(), \
						 GetPropertiesString().GetStringLength(), \
						 allocator_);		\
	if (properties_ && FindPropertyIndex(name, &index))		\
	  method(ptr, propertiesPath.Append(name.GetString(),		\
					    name.GetStringLength(),	\
					    allocator_),		\
		 __VA_ARGS__, properties_[index].schema);		\
	else if (additionalPropertiesSchema_)				\
	  method(ptr, path.Append(GetAdditionalPropertiesString().GetString(), \
				  GetAdditionalPropertiesString().GetStringLength(), \
				  allocator_),				\
		 __VA_ARGS__, additionalPropertiesSchema_);		\
	else if (itemsList_ && (name == GetWildcardString()))		\
	  method(ptr, path.Append(GetItemsString().GetString(),		\
				  GetItemsString().GetStringLength(),	\
				  allocator_),				\
		 __VA_ARGS__, itemsList_);				\
	else if (additionalItemsSchema_ && (name == GetWildcardString())) \
	  method(ptr, path.Append(GetAdditionalItemsString().GetString(), \
				  GetAdditionalItemsString().GetStringLength(),	\
				  allocator_),				\
		 __VA_ARGS__, additionalItemsSchema_);			\
	else if (!patternMatched) {					\
	  std::cerr << #method << ": Failed to find child property: " << name.GetString() << ", "; \
	  NormalizedDocumentType::DisplayPointer(ptr);			\
	  std::cerr << std::endl;					\
	  return;							\
	}								\
      } else {								\
	PointerType itemsPath = path.Append(GetItemsString().GetString(), \
					    GetItemsString().GetStringLength(), \
					    allocator_);		\
	if (itemsList_)							\
	  method(ptr, itemsPath, __VA_ARGS__, itemsList_);		\
	else if (itemsTuple_ && name.GetUint() < itemsTupleCount_)	\
	  method(ptr, itemsPath.Append(name.GetUint(), allocator_),	\
		 __VA_ARGS__, itemsTuple_[name.GetUint()]);		\
	else if (additionalItemsSchema_)				\
	  method(ptr, path.Append(GetAdditionalItemsString().GetString(), \
				  GetAdditionalItemsString().GetStringLength(),	\
				  allocator_),				\
		 __VA_ARGS__, additionalItemsSchema_);			\
	else {								\
	  std::cerr << #method << ": Failed to find child item: " << name.GetUint() << ", "; \
	  NormalizedDocumentType::DisplayPointer(ptr);			\
	  std::cerr << std::endl;					\
	  return;							\
	}								\
      }									\
      return;								\
    }
#define NESTED_FUNCTION_CALL_VERBOSE(method, ptr, path, ...)		\
    std::cerr << #method << " [";					\
    NormalizedDocumentType::DisplayPointer(ptr);			\
    std::cerr << "]: ";							\
    NormalizedDocumentType::DisplayPointer(path);			\
    std::cerr << std::endl;						\
    NESTED_FUNCTION_CALL(method, ptr, path, __VA_ARGS__)
    
    void GetUniqueSharedProperties(const ValueType* v, SValue& dest,
				   bool push=false,
				   const ValueType* key0=nullptr) {
      if (v->IsObject()) {
	if (v->MemberCount() > 0 && !key0) {
	  for (typename ValueType::ConstMemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr)
	    GetUniqueSharedProperties(&itr->value, dest, push, &(itr->name));
	}
      } else if (v->IsBool() || v->IsArray()) {
	if (v->IsArray()) {
	  for (typename ValueType::ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr) {
	    if (!itr->IsString())
	      return;
	  }
	}
	if (key0)
	  AddUniqueElement(dest, *key0);
	else
	  AddUniqueElement(dest, GetRelativeUpString());
      }
    }
    PointerOrderFlag PointersOrdered(const PointerType& a, const PointerType& b) const {
      size_t nPrefix = a.CountMatchingTokens(b);
      if ((a.GetTokenCount() == nPrefix) || (b.GetTokenCount() == nPrefix)) {
	if (a.GetTokenCount() > b.GetTokenCount())
	  return kPointerOrderTrue;
	return kPointerOrderFalse;
      }
      SValue last(a.GetTokens()[nPrefix - 1].name, a.GetTokens()[nPrefix - 1].length);
      if (last == GetAllOfString() ||
	  last == GetAnyOfString() ||
	  last == GetOneOfString() ||
	  last == GetItemsString()) {
	if (a.GetTokens()[nPrefix].index < b.GetTokens()[nPrefix].index)
	  return kPointerOrderTrue;
	return kPointerOrderFalse;
      }
      if (last == GetPropertiesString())
	return kPointerOrderNull;
      std::cerr << "PointersOrdered: Could not order pointers ";
      NormalizedDocumentType::DisplayPointer(a);
      std::cerr << " -> ";
      NormalizedDocumentType::DisplayPointer(b);
      std::cerr << std::endl;
      return kPointerOrderNull;
    }
    void AddSharedPropertyLink(const PointerType p,
			       const PointerType path,
			       SharedProperty* sharedProp,
			       const SchemaType* childSchema=nullptr,
			       bool parallelSchema=false) {
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      std::cerr << "AddSharedPropertyLink: ";
      NormalizedDocumentType::DisplayPointer(p);
      std::cerr << ", ";
      NormalizedDocumentType::DisplayPointer(path);
      std::cerr << " [";
      NormalizedDocumentType::DisplayPointer(pointer_);
      std::cerr << "]" << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      if (childSchema) {
	if (parallelSchema) {
	  const_cast<SchemaType*>(childSchema)->AddSharedPropertyLink(p,
								      path,
								      sharedProp,
								      nullptr,
								      true);
	} else {
	  PointerType p_up = p.Remove(0, allocator_);
	  const_cast<SchemaType*>(childSchema)->AddSharedPropertyLink(p_up,
								      path,
								      sharedProp);
	}
	return;
      }
      NESTED_FUNCTION_CALL(AddSharedPropertyLink, p, path, sharedProp);
      if (allowSingularSchema_.schemas)
	return;
      if (!sharedProperties_)
	sharedProperties_ = new SharedPropertiesType();
      sharedProperties_->AddOtherProperty(this, sharedProp, path);
    }
    static PointerType SchemaPointer2InstancePointer(const PointerType& p,
						     bool& hasRegex,
						     size_t* unresolvedTokenIndex = 0,
						     AllocatorType* allocator = 0) {
      // Wildcards used for items/additionalProperties/additionalItems when
      //   index is not explicit
      PointerType out(allocator);
      for (const typename PointerType::Token *t = p.GetTokens(); t != p.GetTokens() + p.GetTokenCount(); ++t) {
	if (t->index == kPointerInvalidIndex) {
	  ValueType token(t->name, t->length);
	  if (token == GetPropertiesString()) {
	    t++;
	    if (t != p.GetTokens() + p.GetTokenCount() &&
		t->index == kPointerInvalidIndex) {
	      out = out.Append(t->name, t->length, allocator);
	      continue;
	    }
	  } else if (token == GetPatternPropertiesString()) {
	    t++;
	    if (t != p.GetTokens() + p.GetTokenCount() &&
		t->index == kPointerInvalidIndex) {
	      out = out.Append(t->name, t->length, allocator);
	      hasRegex = true;
	      continue;
	    }
	  } else if (token == GetAdditionalPropertiesString()) {
	    // TODO: Indicate it applies to only additional properties?
	    out = out.Append(GetWildcardString().GetString(),
			     GetWildcardString().GetStringLength(),
			     allocator);
	    hasRegex = true;
	    continue;
	  } else if (token == GetItemsString()) {
	    if ((t + 1) != p.GetTokens() + p.GetTokenCount() &&
		(t + 1)->index != kPointerInvalidIndex) {
	      t++;
	      out = out.Append(t->index, allocator);
	    } else {
	      out = out.Append(GetWildcardString().GetString(),
			       GetWildcardString().GetStringLength(),
			       allocator);
	      hasRegex = true;
	    }
	    continue;
	  } else if (token == GetAdditionalItemsString()) {
	    // TODO: Indicate it applies to only additional items?
	    out = out.Append(GetWildcardString().GetString(),
			     GetWildcardString().GetStringLength(),
			     allocator);
	    hasRegex = true;
	    continue;
	  } else if (token == GetAllOfString()) {
	    t++;
	    if (t != p.GetTokens() + p.GetTokenCount() &&
		t->index != kPointerInvalidIndex) {
	      continue;
	    }
	  } else if (token == GetAnyOfString()) {
	    t++;
	    if (t != p.GetTokens() + p.GetTokenCount() &&
		t->index != kPointerInvalidIndex) {
	      continue;
	    }
	  } else if (token == GetOneOfString()) {
	    t++;
	    if (t != p.GetTokens() + p.GetTokenCount() &&
		t->index != kPointerInvalidIndex) {
	      continue;
	    }
	  } else if (token == GetAllowSingularString()) {
	    continue;
	  }
	}
	// Error: unresolved token
	if (unresolvedTokenIndex)
	  *unresolvedTokenIndex = static_cast<size_t>(t - p.GetTokens());
	if (t == p.GetTokens() + p.GetTokenCount()) {
	  std::cerr << "SchemaPointer2InstancePointer: Missing final token \"";
	  NormalizedDocumentType::DisplayPointer(p);
	  std::cerr << "\"" << std::endl;
	} else {
	  std::cerr << "SchemaPointer2InstancePointer: Failing token is \"" << t->name << "\" from ";
	  NormalizedDocumentType::DisplayPointer(p);
	  std::cerr << std::endl;
	}
	break;
      }
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      std::cerr << "SchemaPointer2InstancePointer: ";
      NormalizedDocumentType::DisplayPointer(p);
      std::cerr << " -> ";
      NormalizedDocumentType::DisplayPointer(out);
      std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      return out;
    }
						     
    static const SchemaType* Get(const SchemaType* root,
				 const PointerType& p,
				 size_t* unresolvedTokenIndex = 0) {
      RAPIDJSON_ASSERT(p.IsValid());
      bool allowFailure = false;
      const SchemaType* v = root;
      for (const typename PointerType::Token *t = p.GetTokens(); t != p.GetTokens() + p.GetTokenCount(); ++t) {
	if (t->index == kPointerInvalidIndex) {
	  ValueType token(t->name, t->length);
	  if (v->allowSingularSchema_.schemas) {
	    if (token == GetAllowSingularString()) {
	      v = v->allowSingularSchema_.schemas[1];
	      continue;
	    }
	    v = v->allowSingularSchema_.schemas[0];
	  }
	  if (token == GetPropertiesString()) {
	    t++;
	    SizeType index = 0;
	    if (t != p.GetTokens() + p.GetTokenCount() &&
		t->index == kPointerInvalidIndex &&
		v->FindPropertyIndex(ValueType(t->name, t->length).Move(), &index)) {
	      v = v->properties_[index].schema;
	      continue;
	    }
	  } else if (token == GetPatternPropertiesString()) {
	    // TODO: Currently, only the first match will be followed so
	    //   if there are multiple matches, those schemas will not
	    //   be linked.
	    if (v->patternProperties_) {
	      t++;
	      if (t != p.GetTokens() + p.GetTokenCount() &&
		  t->index == kPointerInvalidIndex) {
		bool match = false;
		for (SizeType i = 0; i < v->patternPropertyCount_; i++) {
		  if (v->patternProperties_[i].pattern &&
		      (IsPatternMatch(v->patternProperties_[i].pattern,
				      t->name, t->length) ||
		       ((t->length == v->patternProperties_[i].patternStr.GetStringLength()) &&
			(std::memcmp(t->name, v->patternProperties_[i].patternStr.GetString(), t->length * sizeof(Ch)) == 0)))) {
		    v = v->patternProperties_[i].schema;
		    match = true;
		    break;
		  }
		}
		if (match)
		  continue;
	      }
	    }
	  } else if (token == GetAdditionalPropertiesString()) {
	    if (v->additionalPropertiesSchema_) {
	      v = v->additionalPropertiesSchema_;
	      continue;
	    }
	  } else if (token == GetItemsString()) {
	    if (v->itemsList_) {
	      v = v->itemsList_;
	      continue;
	    } else if (v->itemsTuple_) {
	      t++;
	      if (t != p.GetTokens() + p.GetTokenCount() &&
		  t->index != kPointerInvalidIndex &&
		  t->index < v->itemsTupleCount_) {
		v = v->itemsTuple_[t->index];
		continue;
	      }
	    } else {
	      std::cerr << "No items in schema" << std::endl;
	    }
	  } else if (token == GetAdditionalItemsString()) {
	    if (v->additionalItemsSchema_) {
	      v = v->additionalItemsSchema_;
	      continue;
	    }
	  } else if (token == GetAllOfString()) {
	    if (v->allOf_.schemas) {
	      t++;
	      if (t != p.GetTokens() + p.GetTokenCount() &&
		  t->index != kPointerInvalidIndex &&
		  t->index < v->allOf_.count) {
		v = v->allOf_.schemas[t->index];
		continue;
	      }
	    }
	  } else if (token == GetAnyOfString()) {
	    if (v->anyOf_.schemas) {
	      t++;
	      if (t != p.GetTokens() + p.GetTokenCount() &&
		  t->index != kPointerInvalidIndex &&
		  t->index < v->anyOf_.count) {
		v = v->anyOf_.schemas[t->index];
		allowFailure = true;
		continue;
	      }
	    }
	  } else if (token == GetOneOfString()) {
	    if (v->oneOf_.schemas) {
	      t++;
	      if (t != p.GetTokens() + p.GetTokenCount() &&
		  t->index != kPointerInvalidIndex &&
		  t->index < v->oneOf_.count) {
		v = v->oneOf_.schemas[t->index];
		allowFailure = true;
		continue;
	      }
	    }
	  }
	}
	// Error: unresolved token
	if (unresolvedTokenIndex)
	  *unresolvedTokenIndex = static_cast<size_t>(t - p.GetTokens());
	allowFailure = false;
	if (!allowFailure) {
	  std::cerr << "Get: Error in ";
	  NormalizedDocumentType::DisplayPointer(v->pointer_);
	  std::cerr << std::endl;
	  if (t == p.GetTokens() + p.GetTokenCount()) {
	    std::cerr << "Get: Missing final token \"";
	    NormalizedDocumentType::DisplayPointer(p);
	    std::cerr << "\"" << std::endl;
	  } else {
	    std::cerr << "Get: Failing token is \"" << t->name << "\" from ";
	    NormalizedDocumentType::DisplayPointer(p);
	    std::cerr << std::endl;
	    std::cerr << "Properties = " << std::endl;
	    for (SizeType index = 0; index < v->propertyCount_; index++)
	      std::cerr << "    " << v->properties_[index].name.GetString() << std::endl;
	  }
	}
	return 0;
      }
#ifdef RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      std::cerr << "Get: ";
      NormalizedDocumentType::DisplayPointer(p);
      std::cerr << " -> ";
      NormalizedDocumentType::DisplayPointer(v->pointer_);
      std::cerr << std::endl;
#endif // RAPIDJSON_YGGDRASIL_DEBUG_NORMALIZATION_SHARED
      return v;
    }
#define NESTED_CONST_CALL(method, schema, ...)			\
      const_cast<SchemaType*>(schema)->method(__VA_ARGS__)
#define NESTED_CONST_CALL_ARRAY(method, var, name, ...)			\
    {									\
      if (var.schemas) {						\
	PointerType arrayPath = path.Append(Get ## name ## String().GetString(), \
					    Get ## name ## String().GetStringLength(), \
					    allocator_);		\
	for (SizeType i = 0; i < var.count; i++)			\
	  NESTED_CONST_CALL(SortSharedProperties, var.schemas[i], root,	\
			    arrayPath.Append(i, allocator_));		\
      }									\
    }
    void SortSharedProperties(const SchemaType* root, const PointerType path) {
      if (sharedProperties_)
	sharedProperties_->SortSources(root, path);
      if (inSort_)
	return;
      inSort_ = true;
      if (properties_) {
	PointerType propertiesPath = path.Append(GetPropertiesString().GetString(),
						 GetPropertiesString().GetStringLength(),
						 allocator_);
	for (SizeType i = 0; i < propertyCount_; i++)
	  NESTED_CONST_CALL(SortSharedProperties, properties_[i].schema,
			    root,
			    propertiesPath.Append(properties_[i].name.GetString(),
						  properties_[i].name.GetStringLength(),
						  allocator_));
      }
      if (additionalPropertiesSchema_)
	NESTED_CONST_CALL(SortSharedProperties, additionalPropertiesSchema_,
			  root,
			  path.Append(GetAdditionalPropertiesString().GetString(),
				      GetAdditionalPropertiesString().GetStringLength(),
				      allocator_));
      if (patternProperties_) {
	PointerType patternPath = path.Append(GetPatternPropertiesString().GetString(),
					      GetPatternPropertiesString().GetStringLength(),
					      allocator_);
	for (SizeType i = 0; i < patternPropertyCount_; i++)
	  NESTED_CONST_CALL(SortSharedProperties,
			    patternProperties_[i].schema,
			    root,
			    patternPath.Append(patternProperties_[i].patternStr.GetString(),
					       patternProperties_[i].patternStr.GetStringLength(),
					       allocator_));
      }
      PointerType itemsPath = path.Append(GetItemsString().GetString(),
					  GetItemsString().GetStringLength(),
					  allocator_);
      if (itemsList_)
	NESTED_CONST_CALL(SortSharedProperties, itemsList_,
			  root, itemsPath);
      else if (itemsTuple_) {
	for (SizeType i = 0; i < itemsTupleCount_; i++)
	  NESTED_CONST_CALL(SortSharedProperties, itemsTuple_[i], root,
			    itemsPath.Append(i, allocator_));
      }
      if (additionalItemsSchema_)
	NESTED_CONST_CALL(SortSharedProperties, additionalItemsSchema_, root,
			  path.Append(GetAdditionalItemsString().GetString(),
				      GetAdditionalItemsString().GetStringLength(),
				      allocator_));
      // if (hasSchemaDependencies_) {
      // dependenciesSchema?
      // }
      NESTED_CONST_CALL_ARRAY(SortSharedProperties, allOf_, AllOf, root, path);
      NESTED_CONST_CALL_ARRAY(SortSharedProperties, anyOf_, AnyOf, root, path);
      NESTED_CONST_CALL_ARRAY(SortSharedProperties, oneOf_, OneOf, root, path);
      if (allowSingularSchema_.schemas) {
	for (SizeType i = 0; i < 1; i++)
	  NESTED_CONST_CALL(SortSharedProperties, allowSingularSchema_.schemas[i],
			    root, path);
      }
    }
#undef NESTED_CONST_CALL_ARRAY
#undef NESTED_CONST_CALL
    void AssignSharedProperties(const ValueType* v, bool push=false,
				const ValueType* key0=nullptr) {
      if (v->IsObject()) {
	if (v->MemberCount() > 0 && !key0) {
	  for (typename ValueType::ConstMemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr)
	    AssignSharedProperties(&(itr->value), push, &(itr->name));
	}
      } else if (v->IsBool() || v->IsArray()) {
	if (v->IsArray()) {
	  for (typename ValueType::ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr) {
	    if (!itr->IsString())
	      return;
	  }
	}
	SValue key;
	if (key0) {
	  RAPIDJSON_ASSERT(key0->IsString());
	  key.SetString(key0->GetString(),
			key0->GetStringLength(),
			*allocator_);
	} else {
	  key.SetString(GetRelativeUpString().GetString(),
			GetRelativeUpString().GetStringLength(),
			*allocator_);
	}
	RAPIDJSON_ASSERT(key.IsString());
	if (!sharedProperties_)
	  sharedProperties_ = new SharedPropertiesType();
	sharedProperties_->AddLocalProperty(this, key, v, push);
      }
    }
#undef NESTED_FUNCTION_CALL_VERBOSE
#undef NESTED_FUNCTION_CALL
#undef NESTED_FUNCTION_CALL_APPEND
#endif // RAPIDJSON_YGGDRASIL

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
    SValue class_;
    SValue args_;
    SValue kwargs_;
    bool isMetaschema_;
    bool inSort_;
    const SchemaType* metaschema_;
    SizeType metaschemaValidatorIndex_;
    const SchemaType* instance_;
    SizeType instanceValidatorIndex_;
    bool defaultSet_;
    SValue default_;
    SValue aliases_;
    SValue child_aliases_;
    bool hasAliases_;
    SchemaArray allowSingularSchema_;
    bool allowSingular_;
    SingularFlag isSingular_;
    PointerType singularPtr_;
    const SchemaType* parentSchema_;
    SValue parentKey_;
    SValue deprecated_;
    const ValueType* enumValues_;
    SharedPropertiesType* sharedProperties_;
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
	, metaschema_doc_(), metaschema_(), isMetaschema_(isMetaschema),
	instanceMap_(allocator, kInitialInstanceMapSize)
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

#ifdef RAPIDJSON_YGGDRASIL
	const_cast<SchemaType*>(root_)->SortSharedProperties(root_, PointerType());
#endif // RAPIDJSON_YGGDRASIL

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
	, metaschema_doc_(rhs.metaschema_doc_), metaschema_(rhs.metaschema_), isMetaschema_(rhs.isMetaschema_), instanceMap_(std::move(rhs.instanceMap_))
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
	if ((metaschema_) && (!isMetaschema_)) {
	  delete metaschema_;
	  delete metaschema_doc_;
	}
	while (!instanceMap_.Empty())
	  instanceMap_.template Pop<InstanceSchemaEntry>(1)->~InstanceSchemaEntry();
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

#ifdef RAPIDJSON_YGGDRASIL
    typedef GenericSchemaDocument<ValueT, Allocator> InstanceSchemaDoc;
  
    struct InstanceSchemaEntry {
        InstanceSchemaEntry(const PointerType& p, InstanceSchemaDoc* d, bool o, Allocator* allocator) : pointer(p, allocator), document(d), owned(o) {}
        ~InstanceSchemaEntry() {
            if (owned) {
                document->~InstanceSchemaDoc();
                Allocator::Free(document);
            }
        }
        PointerType pointer;
        InstanceSchemaDoc* document;
        bool owned;
    };
  
    const InstanceSchemaDoc* GetInstanceSchema(const PointerType& pointer) const {
        for (const InstanceSchemaEntry* target = instanceMap_.template Bottom<InstanceSchemaEntry>(); target != instanceMap_.template End<InstanceSchemaEntry>(); ++target)
            if (pointer == target->pointer)
                return target->document;
        return 0;
    }
#endif // RAPIDJSON_YGGDRASIL

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
    const UriType& CreateSchema(const SchemaType** schema, const PointerType& pointer, const ValueType& v, const ValueType& document, const UriType& id
#ifdef RAPIDJSON_YGGDRASIL
		,
		internal::SingularFlag singular=internal::kSingularNoFlags,
		const SchemaType* parentSchema=nullptr,
		const ValueType* parentKey=nullptr
#endif // RAPIDJSON_YGGDRASIL
				) {
        RAPIDJSON_ASSERT(pointer.IsValid());
        if (v.IsObject()) {
#ifdef RAPIDJSON_YGGDRASIL
	    if (singular) {
	        if (const SchemaType* sc = GetSchema(pointer, singular)) {
		    if (schema)
		        *schema = sc;
		    AddSchemaRefs(const_cast<SchemaType*>(sc));
		    return id;
		} else if (!HandleRefSchema(pointer, schema, v, document, id)) {
		    SchemaType* s = new (allocator_->Malloc(sizeof(SchemaType))) SchemaType(this, pointer, v, document, allocator_, id, isMetaschema_, singular, parentSchema, parentKey);
		    if (schema)
		        *schema = s;
		    return s->GetId();
		}
	    }
#endif // RAPIDJSON_YGGDRASIL
            if (const SchemaType* sc = GetSchema(pointer)) {
                if (schema)
                    *schema = sc;
                AddSchemaRefs(const_cast<SchemaType*>(sc));
            }
            else if (!HandleRefSchema(pointer, schema, v, document, id)) {
                // The new schema constructor adds itself and its $ref(s) to schemaMap_
	        SchemaType* s = new (allocator_->Malloc(sizeof(SchemaType))) SchemaType(this, pointer, v, document, allocator_, id
#ifdef RAPIDJSON_YGGDRASIL
		, isMetaschema_, singular, parentSchema, parentKey
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
  const UriType& CreateMetaSchema(const SchemaType** schema) {
      if (!metaschema_) {
	if (isMetaschema_) {
	  metaschema_ = this;
	} else {
	  GenericDocument<EncodingType, typename ValueType::AllocatorType>* new_doc;
	  new_doc = new GenericDocument<EncodingType, typename ValueType::AllocatorType>();
	  new_doc->Parse(get_metaschema<Ch>());
	  metaschema_doc_ = new_doc;
	  metaschema_ = new GenericSchemaDocument<ValueType, AllocatorType>(*metaschema_doc_, 0, 0, 0, 0, PointerType(), true);
	}
      }
      const SchemaType* sc = &(metaschema_->GetRoot());
      *schema = sc;
      return sc->GetId();
    }
  
    const UriType& CreateInstanceSchema(const SchemaType** schema, const typename SchemaType::SValue& cls, const PointerType& pointer) {
      const InstanceSchemaDoc* sd = GetInstanceSchema(pointer);
      if (!sd) {
	const SchemaType* meta_sc;
	CreateMetaSchema(&meta_sc);
	GenericDocument<EncodingType, typename ValueType::AllocatorType> d;
	metaschema_doc_->Accept(d);
	d.FinalizeFromStack();
	ValueType v;
	v.SetArray();
	v.PushBack(ValueType(v.GetPythonClassString(), d.GetAllocator()).Move(), d.GetAllocator());
	d.AddMember(SchemaType::GetRequiredString(), v, d.GetAllocator());
	if (cls.IsPythonClass()) {
	  d[v.GetPropertiesString()][v.GetPythonClassString()][SchemaType::GetAnyOfString()][0].AddMember(
  	      v.GetPythonClassString(),
	      ValueType(cls.GetString(),
			cls.GetStringLength(),
			d.GetAllocator()).Move(),
	      d.GetAllocator());
	  d[v.GetPropertiesString()][v.GetPythonClassString()][SchemaType::GetAnyOfString()][1][v.GetItemsString()].AddMember(
	      v.GetPythonClassString(),
	      ValueType(cls.GetString(),
			cls.GetStringLength(),
			d.GetAllocator()).Move(),
	      d.GetAllocator());
	}
	InstanceSchemaDoc* new_doc = new (allocator_->Malloc(sizeof(InstanceSchemaDoc))) InstanceSchemaDoc(d, 0, 0, 0, 0, PointerType(), true);
	sd = new_doc;
	InstanceSchemaEntry *entry = instanceMap_.template Push<InstanceSchemaEntry>();
	new (entry) InstanceSchemaEntry(pointer, new_doc, true, allocator_);
      }
      const SchemaType* sc = &(sd->GetRoot());
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

#ifdef RAPIDJSON_YGGDRASIL
    const SchemaType* GetSchema(const PointerType& pointer,
				internal::SingularFlag singular) const {
        for (const SchemaEntry* target = schemaMap_.template Bottom<SchemaEntry>(); target != schemaMap_.template End<SchemaEntry>(); ++target)
            if (pointer == target->pointer && singular == target->schema->isSingular_)
                return target->schema;
        return 0;
    }
#endif // RAPIDJSON_YGGDRASIL
    const SchemaType* GetSchema(const PointerType& pointer) const {
        for (const SchemaEntry* target = schemaMap_.template Bottom<SchemaEntry>(); target != schemaMap_.template End<SchemaEntry>(); ++target)
#ifdef RAPIDJSON_YGGDRASIL
	  {
	    if (target->schema->isSingular_) continue;
#endif // RAPIDJSON_YGGDRASIL
            if (pointer == target->pointer)
                return target->schema;
#ifdef RAPIDJSON_YGGDRASIL
	  }
#endif // RAPIDJSON_YGGDRASIL
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
    static const size_t kInitialInstanceMapSize = 4;
    const GenericDocument<EncodingType, typename ValueType::AllocatorType>* metaschema_doc_;
    const GenericSchemaDocument<ValueT, Allocator>* metaschema_;
    bool isMetaschema_;
    internal::Stack<Allocator> instanceMap_;
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
    typedef internal::SharedProperties<SchemaDocumentType> SharedPropertiesType;
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
#ifdef RAPIDJSON_YGGDRASIL
	, warning_(kObjectType),
	currentWarning_()
#endif // RAPIDJSON_YGGDRASIL
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
#ifdef RAPIDJSON_YGGDRASIL
	, warning_(kObjectType),
	currentWarning_()
#endif // RAPIDJSON_YGGDRASIL
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
#ifdef RAPIDJSON_YGGDRASIL
	ResetWarning();
#endif // RAPIDJSON_YGGDRASIL
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

#ifdef RAPIDJSON_YGGDRASIL

    template<typename ErrorType, typename OtherType>
    void CheckErrorReplace_(ErrorType* curr, OtherType* other,
			    typename ErrorType::AllocatorType& allocator) const {
      if (other->IsNull())
	return;
      if (curr->IsNull() ||
	  (curr->IsString() &&
	   (other->GetStringLength() > curr->GetStringLength())))
	curr->SetString(other->GetString(),
			other->GetStringLength(),
			allocator);
    }
    template<typename ErrorType>
    SizeType getMaxLen_(ErrorType* a, ErrorType* b) const {
      if (a->IsNull()) {
	if (b->IsNull())
	  return 0;
	else
	  return b->GetStringLength();
      } else if (b->IsNull())
	return a->GetStringLength();
      else if (a->GetStringLength() > b->GetStringLength())
	return a->GetStringLength();
      else
	return b->GetStringLength();
    }
    
    template<typename ErrorType>
    bool GetErrorMsg(ErrorType& out,
		     typename ErrorType::AllocatorType& allocator,
		     const ValueType* err = nullptr,
		     ErrorType* nonTypeError = nullptr,
		     ErrorType* typeError = nullptr,
		     bool isSingular = false) const {
      typedef typename ValueType::ConstMemberIterator MemberIter;
      if (!err)
	err = &error_;
      if ((!isSingular) && (err->MemberCount() > 1))
	out.SetArray();
      bool hasSingular = false;
      ErrorType nonTypeError_target;
      ErrorType typeError_target;
      ErrorType nonTypeError_singular;
      ErrorType typeError_singular;
      SizeType idx_singular = 0;
      if (!nonTypeError)
	nonTypeError = &nonTypeError_target;
      if (!typeError)
	typeError = &typeError_target;
      for (MemberIter ierrTyp = err->MemberBegin(); ierrTyp != err->MemberEnd(); ierrTyp++) {
	if (!ierrTyp->value.IsObject())
	  continue;
	ErrorType iout;
	if (ierrTyp->name == GetSingularString()) {
	  hasSingular = true;
	  if (out.IsArray())
	    idx_singular = out.Size();
	  if (!GetErrorMsg(iout, allocator, &(ierrTyp->value),
			   &nonTypeError_singular, &typeError_singular,
			   true))
	    return false;
	} else {
	  if (!GetErrorMsg_(iout, allocator, &(ierrTyp->value),
			    nonTypeError, typeError, isSingular))
	    return false;
	}
	if ((!isSingular) && (err->MemberCount() > 1))
	  out.PushBack(iout, allocator);
	else
	  out.Swap(iout);
      }
      if (hasSingular) {
	SizeType maxOther = getMaxLen_(typeError, nonTypeError);
	SizeType maxSingu = getMaxLen_(&typeError_singular,
				       &nonTypeError_singular);
	if (nonTypeError_singular.IsNull() &&
	    ((maxSingu < maxOther) || (*typeError == typeError_singular))) {
	  if (out.IsArray())
	    out.Erase(out.Begin() + idx_singular);
	} else if (nonTypeError->IsNull() && (maxOther < maxSingu)) {
	  if (out.IsArray()) {
	    ErrorType tmp(out[idx_singular], allocator);
	    out.Swap(tmp);
	  }
	}
	CheckErrorReplace_(nonTypeError, &nonTypeError_singular, allocator);
	CheckErrorReplace_(typeError, &typeError_singular, allocator);
      }
      return true;
    }

    template<typename ErrorType>
    bool GetErrorMsg_(ErrorType& out,
		      typename ErrorType::AllocatorType& allocator,
		      const ValueType* err,
		      ErrorType* nonTypeError,
		      ErrorType* typeError,
		      bool isSingular = false) const {
      typedef typename ValueType::ConstMemberIterator MemberIter;
      typedef typename ValueType::ConstValueIterator ValueIter;
      out.SetObject();
      internal::Stack<StateAllocator> msg_stack(stateAllocator_, kDefaultDocumentStackCapacity);
      MemberIter code = err->FindMember(GetErrorCodeString());
      if (code == err->MemberEnd())
	return false;
      MemberIter iRef = err->FindMember(GetInstanceRefString());
      MemberIter sRef = err->FindMember(GetSchemaRefString());
      if (iRef != err->MemberEnd()) {
	switch ((ValidateErrorCode)(code->value.GetInt())) {
	case kValidateErrorType:
	  CheckErrorReplace_(typeError, &(iRef->value), allocator);
	  break;
	case kValidateErrorOneOf:
	case kValidateErrorAllOf:
	case kValidateErrorAnyOf:
	  break;
	default:
	  CheckErrorReplace_(nonTypeError, &(iRef->value), allocator);
	}
      }
      const RAPIDJSON_ERROR_CHARTYPE* msg = GetValidateError_En((ValidateErrorCode)(code->value.GetInt()));
      SizeType msg_len = internal::StrLen(msg);
      SizeType start = 0;
      SizeType len = 0;
      StringBuffer sb;
      Writer<StringBuffer> w(sb);
      for (SizeType i = 0; i < msg_len; i++) {
	*msg_stack.template Push<Ch>() = msg[i];
	if (msg[i] == (RAPIDJSON_ERROR_CHARTYPE)'%') {
	  i++;
	  start = i;
	  while ((i < msg_len) && !((msg[i] == ' ') ||
				    (msg[i] == '\'') ||
				    (msg[i] == '\"') ||
				    (msg[i] == '.') ||
				    (msg[i] == ','))) {
	    *msg_stack.template Push<Ch>() = msg[i];
	    i++;
	  }
	  len = i - start;
	  i--; // So that the space/quote is added in next iteration
	  *msg_stack.template Push<Ch>() = '\0';
	  ValueType key(msg_stack.template Top<Ch>() - len, len,
			*stateAllocator_);
	  msg_stack.template Pop<Ch>(1);
	  MemberIter val = err->FindMember(key);
	  if (val != err->MemberEnd()) {
	    msg_stack.template Pop<Ch>(len + 1);
	    if (!val->value.Accept(w))
	      return false;
	    SizeType val_start = 0;
	    SizeType val_end = (SizeType)sb.GetLength();
	    if (val->value.IsString()) {
	      val_start++;
	      val_end--;
	    }
	    for (SizeType j = val_start; j < val_end; j++)
	      *msg_stack.template Push<Ch>() = sb.GetString()[j];
	    sb.Clear();
	    w.Reset(sb);
	  } else {
	    std::cerr << "Missing key in error message generation: " << key.GetString() << std::endl;
	  }
	}
      }
      if (msg_stack.GetSize() > 0) {
	out.AddMember(ErrorType(GetMessageString(),
				GetMessageString().length,
				allocator).Move(),
		      ErrorType(msg_stack.template Bottom<Ch>(),
				static_cast<SizeType>(msg_stack.GetSize() / sizeof(Ch)),
				allocator).Move(),
		      allocator);
	msg_stack.Clear();
      }
      if (iRef != err->MemberEnd()) {
	out.AddMember(ErrorType(iRef->name.GetString(),
				iRef->name.GetStringLength(),
				allocator).Move(),
		      ErrorType(iRef->value.GetString(),
				iRef->value.GetStringLength(),
				allocator).Move(),
		      allocator);
	
      }
      if (sRef != err->MemberEnd()) {
	out.AddMember(ErrorType(sRef->name.GetString(),
				sRef->name.GetStringLength(),
				allocator).Move(),
		      ErrorType(sRef->value.GetString(),
				sRef->value.GetStringLength(),
				allocator).Move(),
		      allocator);
      }
      if (isSingular) {
	out.AddMember(ErrorType(GetSingularString(),
				GetSingularString().length,
				allocator).Move(),
		      ErrorType(isSingular).Move(),
		      allocator);
      }
      MemberIter errArray = err->FindMember(GetErrorsString());
      if ((errArray != err->MemberEnd()) && errArray->value.IsArray()) {
	ErrorType errs(kArrayType);
	
	for (ValueIter ierr = errArray->value.Begin(); ierr != errArray->value.End(); ierr++) {
	  ErrorType iout;
	  ErrorType inonTypeError;
	  ErrorType itypeError;
	  if (!GetErrorMsg(iout, allocator, ierr,
			   &inonTypeError, &itypeError))
	    return false;
	  CheckErrorReplace_(nonTypeError, &inonTypeError, allocator);
	  CheckErrorReplace_(typeError, &itypeError, allocator);
	  errs.PushBack(iout, allocator);
	}
	out.AddMember(ErrorType(GetErrorsString(),
				GetErrorsString().length,
				allocator).Move(),
		      errs, allocator);
      }
      return true;
    }
  
    //! Check if the validator will check for a Python object.
    bool RequiresPython() const { return root_.RequiresPython(); }
  
    //! Reset the warning state.
    void ResetWarning() {
        warning_.SetObject();
        currentWarning_.SetNull();
    }
  
    //! Gets the warning object.
    ValueType& GetWarning() { return warning_; }
    const ValueType& GetWarning() const { return warning_; }
#endif // RAPIDJSON_YGGDRASIL

    //! Gets the JSON pointer pointed to the invalid schema.
    //  If reporting all errors, the stack will be empty.
#ifdef RAPIDJSON_YGGDRASIL
    virtual
#endif // RAPIDJSON_YGGDRASIL
    PointerType GetInvalidSchemaPointer() const {
        return schemaStack_.Empty() ? PointerType() : CurrentSchema().GetPointer();
    }

    //! Gets the keyword of invalid schema.
    //  If reporting all errors, the stack will be empty, so return "errors".
    const Ch* GetInvalidSchemaKeyword() const {
        if (!schemaStack_.Empty()) return CurrentContext().invalidKeyword;
        if (GetContinueOnErrors() && !error_.ObjectEmpty()) return (const Ch*)GetErrorsString();
#ifdef RAPIDJSON_YGGDRASIL
	if (!warning_.ObjectEmpty()) return (const Ch*)GetWarningsString();
#endif // RAPIDJSON_YGGDRASIL
        return 0;
    }

    //! Gets the error code of invalid schema.
    //  If reporting all errors, the stack will be empty, so return kValidateErrors.
    ValidateErrorCode GetInvalidSchemaCode() const {
        if (!schemaStack_.Empty()) return CurrentContext().invalidCode;
        if (GetContinueOnErrors() && !error_.ObjectEmpty()) return kValidateErrors;
#ifdef RAPIDJSON_YGGDRASIL
	if (!warning_.ObjectEmpty()) return kValidateWarnings;
#endif // RAPIDJSON_YGGDRASIL
        return kValidateErrorNone;
    }

    //! Gets the JSON pointer pointed to the invalid value.
    //  If reporting all errors, the stack will be empty.
#ifdef RAPIDJSON_YGGDRASIL
    virtual
#endif // RAPIDJSON_YGGDRASIL
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
#ifdef RAPIDJSON_YGGDRASIL
    virtual bool EndMissingPropertiesShared(const SValue&, const SValue&) {
        return EndMissingProperties();
    }
    void DisallowedValueEnum(const typename SchemaType::ValueType& expected) {
      currentError_.SetObject();
      currentError_.AddMember(GetExpectedString(),
			      ValueType(expected, GetStateAllocator()).Move(),
			      GetStateAllocator());
      AddCurrentError(kValidateErrorEnum);
    }
    ValidateErrorCode SharedNormalizationError(ISchemaValidator* subvalidator) {
      ValueType sharedError(static_cast<GenericSchemaValidator*>(subvalidator)->GetError(), GetStateAllocator(), true);
      RAPIDJSON_ASSERT(sharedError.IsObject() && (sharedError.MemberCount() == 1));
      typename ValueType::ConstMemberIterator m = sharedError.MemberBegin();
      typename ValueType::ConstMemberIterator vcode = m->value.FindMember(GetErrorCodeString());
      RAPIDJSON_ASSERT(vcode != m->value.MemberEnd());
      MergeError(sharedError);
      return static_cast<ValidateErrorCode>(vcode->value.GetUint());
    }
    void GenericError(const char* str) {
      // std::cerr << "GenericError: " << str << std::endl;
      currentError_.SetObject();
      std::basic_string<Ch> msg = units::convert_chars<UTF8<char>, EncodingType>(std::basic_string<char>(str));
      currentError_.AddMember(GetMessageString(),
			      ValueType(msg.c_str(), (SizeType)(msg.size()),
					GetStateAllocator()).Move(),
			      GetStateAllocator());
      AddCurrentError(kValidateErrorGeneric);
    }
#endif // RAPIDJSON_YGGDRASIL
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
  void InvalidPythonClass(const Ch* str, SizeType len, const SValue& expected) {
    currentError_.SetObject();
    currentError_.AddMember(GetExpectedString(),
			    ValueType(expected, GetStateAllocator()).Move(),
			    GetStateAllocator());
    currentError_.AddMember(GetActualString(),
			    ValueType(str, len, GetStateAllocator()).Move(),
			    GetStateAllocator());
    AddCurrentError(kValidateErrorPythonClass, true);
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
			    ValueType(kArrayType).Move(),
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
  ValidateErrorCode NotSingularItem(ISchemaValidator** subvalidator) {
    error_.CopyFrom(static_cast<GenericSchemaValidator*>(subvalidator[0])->GetError(), GetStateAllocator(), true);
    RAPIDJSON_ASSERT(error_.IsObject() && (error_.MemberCount() == 1));
    typename ValueType::ConstMemberIterator m = error_.MemberBegin();
    typename ValueType::ConstMemberIterator vcode = m->value.FindMember(GetErrorCodeString());
    RAPIDJSON_ASSERT(vcode != m->value.MemberEnd());
    error_.AddMember(GetSingularString(),
		     ValueType(static_cast<GenericSchemaValidator*>(subvalidator[1])->GetError(),
			       GetStateAllocator()).Move(),
		     GetStateAllocator());
    return static_cast<ValidateErrorCode>(vcode->value.GetUint());
  }
  void NormalizationMergeConflict(const typename SchemaType::ValueType& cond,
				  const SValue& expected, const SValue& actual) {
    currentError_.SetObject();
    currentError_.AddMember(GetConflictingString(),
			    ValueType(cond, GetStateAllocator()).Move(),
			    GetStateAllocator());
    currentError_.AddMember(GetExpectedString(),
			    ValueType(expected, GetStateAllocator()).Move(),
			    GetStateAllocator());
    currentError_.AddMember(GetActualString(),
			    ValueType(actual, GetStateAllocator()).Move(),
			    GetStateAllocator());
    AddCurrentError(kNormalizeErrorMergeConflict, false);
  }
  void AddWarnings(ISchemaValidator** subvalidators, SizeType count) {
    AddWarningArray(subvalidators, count);
  }
  void DeprecationWarning(const SValue* warning=nullptr) {
    currentWarning_.SetObject();
    if (warning)
      currentWarning_.AddMember(GetWarningString(),
			      ValueType(*warning, GetStateAllocator()).Move(),
			      GetStateAllocator());
    AddCurrentWarning(kDeprecatedWarning, false);
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
    RAPIDJSON_STRING_(Type, 't', 'y', 'p', 'e')
    RAPIDJSON_STRING_(Warning, 'w', 'a', 'r', 'n', 'i', 'n', 'g')
    RAPIDJSON_STRING_(Warnings, 'w', 'a', 'r', 'n', 'i', 'n', 'g', 's')
    RAPIDJSON_STRING_(Singular, 's', 'i', 'n', 'g', 'u', 'l', 'a', 'r')
    RAPIDJSON_STRING_(Message, 'm', 'e', 's', 's', 'a', 'g', 'e')
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
        valid_ = false;\
        return valid_;\
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
        valid_ = !outputHandler_ || outputHandler_->StartObject();
        return valid_;
    }
    
    bool Key(const Ch* str, SizeType len, bool copy) {
        if (!valid_) return false;
        AppendToken(str, len);
        if (!CurrentSchema().Key(CurrentContext(), str, len, copy) && !GetContinueOnErrors()) {
            valid_ = false;
            return valid_;
        }
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(Key, (str, len, copy));
        valid_ = !outputHandler_ || outputHandler_->Key(str, len, copy);
        return valid_;
    }
    
    bool EndObject(SizeType memberCount) {
        if (!valid_) return false;
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(EndObject, (memberCount));
        if (!CurrentSchema().EndObject(CurrentContext(), memberCount) && !GetContinueOnErrors()) { 
            valid_ = false; 
            return valid_; 
        }
        RAPIDJSON_SCHEMA_HANDLE_END_(EndObject, (memberCount));
    }

    bool StartArray() {
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_(StartArray, (CurrentContext()));
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(StartArray, ());
        valid_ = !outputHandler_ || outputHandler_->StartArray();
        return valid_;
    }
    
    bool EndArray(SizeType elementCount) {
        if (!valid_) return false;
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(EndArray, (elementCount));
        if (!CurrentSchema().EndArray(CurrentContext(), elementCount) && !GetContinueOnErrors()) {
            valid_ = false;
            return valid_;
        }
        RAPIDJSON_SCHEMA_HANDLE_END_(EndArray, (elementCount));
    }
  
#undef RAPIDJSON_SCHEMA_HANDLE_BEGIN_VERBOSE_
#undef RAPIDJSON_SCHEMA_HANDLE_BEGIN_
#undef RAPIDJSON_SCHEMA_HANDLE_PARALLEL_
#undef RAPIDJSON_SCHEMA_HANDLE_VALUE_

    // Implementation of ISchemaStateFactory<SchemaType>
    virtual ISchemaValidator* CreateSchemaValidator(const SchemaType& root, const bool inheritContinueOnErrors
#ifdef RAPIDJSON_YGGDRASIL
						    , PointerType
#endif // RAPIDJSON_YGGDRASIL
						    ) {
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
#ifdef RAPIDJSON_YGGDRASIL
	, warning_(kObjectType),
	currentWarning_()
#endif // RAPIDJSON_YGGDRASIL
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
#ifdef RAPIDJSON_YGGDRASIL
	    PointerType prevValuePointer = CurrentContext().valuePointer;
	    PointerType* pa = CurrentContext().patternPropertiesPointers;
#endif // RAPIDJSON_YGGDRASIL
            typename Context::PatternValidatorType patternValidatorType = CurrentContext().valuePatternValidatorType;
            bool valueUniqueness = CurrentContext().valueUniqueness;
            RAPIDJSON_ASSERT(CurrentContext().valueSchema);
            PushSchema(*CurrentContext().valueSchema);

            if (count > 0) {
                CurrentContext().objectPatternValidatorType = patternValidatorType;
                ISchemaValidator**& va = CurrentContext().patternPropertiesValidators;
                SizeType& validatorCount = CurrentContext().patternPropertiesValidatorCount;
                va = static_cast<ISchemaValidator**>(MallocState(sizeof(ISchemaValidator*) * count));
#ifdef RAPIDJSON_YGGDRASIL
		RAPIDJSON_ASSERT(pa);
                for (SizeType i = 0; i < count; i++)
		    va[validatorCount++] = CreateSchemaValidator(*sa[i], true, pa[i]);  // Inherit continueOnError
#else // RAPIDJSON_YGGDRASIL
                for (SizeType i = 0; i < count; i++)
		    va[validatorCount++] = CreateSchemaValidator(*sa[i], true);  // Inherit continueOnError
#endif // RAPIDJSON_YGGDRASIL
            }

            CurrentContext().arrayUniqueness = valueUniqueness;
#ifdef RAPIDJSON_YGGDRASIL
	    CurrentContext().valuePointer = prevValuePointer;
	    CurrentContext().schemaPointerAbs = prevValuePointer;
#endif // RAPIDJSON_YGGDRASIL
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

#ifdef RAPIDJSON_YGGDRASIL
    //! Support for warnings
    void AddWarning(const ValueType& keyword, const ValueType& warning) {
      typename ValueType::MemberIterator member = warning_.FindMember(keyword);
      if (member == warning_.MemberEnd())
	warning_.AddMember(keyword, ValueType(warning, GetStateAllocator()).Move(), GetStateAllocator());
      else {
	if (member->value.IsObject()) {
	  ValueType warnings(kArrayType);
	  warnings.PushBack(member->value, GetStateAllocator());
	  member->value = warnings;
	}
	member->value.PushBack(ValueType(warning, GetStateAllocator()).Move(), GetStateAllocator());
      }
    }

    void AddCurrentWarning(const ValidateErrorCode code, bool parent = false) {
        AddErrorCode(currentWarning_, code);
        AddErrorInstanceLocation(currentWarning_, parent);
        AddErrorSchemaLocation(currentWarning_);
        AddWarning(ValueType(SchemaType::GetValidateErrorKeyword(code), GetStateAllocator(), false).Move(), currentWarning_);
    }

    void AddWarningArray(ISchemaValidator** subvalidators, SizeType count) {
      for (SizeType i = 0; i < count; ++i) {
	const ValueType& iwarnings = static_cast<GenericSchemaValidator*>(subvalidators[i])->GetWarning();
	for (typename ValueType::ConstMemberIterator it = iwarnings.MemberBegin(); it != iwarnings.MemberEnd(); ++it) {
	  if (it->value.IsArray()) {
	    for (typename ValueType::ConstValueIterator iit = it->value.Begin(); iit != it->value.End(); ++iit)
	      AddWarning(it->name, *iit);
	  } else {
	    AddWarning(it->name, it->value);
	  }
	}
      }
    }

#endif // RAPIDJSON_YGGDRASIL

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
#ifdef RAPIDJSON_YGGDRASIL
    ValueType warning_;
    ValueType currentWarning_;
#endif // RAPIDJSON_YGGDRASIL
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
  typedef internal::GenericNormalizedDocument<SchemaDocumentType, StateAllocator> NormalizedDocumentType;

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
    normalized_(0, &this->GetStateAllocator()), normalization_depth_(0), validator_index_(0), child_validators_(0), temp_instanceRef_(nullptr), temp_schemaRef_(nullptr), schemaPointerAbs_(allocator) {
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
    normalized_(0, &this->GetStateAllocator()), normalization_depth_(0), validator_index_(0), child_validators_(0),
    temp_instanceRef_(nullptr), temp_schemaRef_(nullptr),
    schemaPointerAbs_(allocator) {
    normalized_.SetDocumentStack(&this->documentStack_);
  }
  GenericSchemaNormalizer(
        const SchemaDocumentType& schemaDocument,
        const SchemaType& root,
        const Ch* basePath, size_t basePathSize,
	const PointerType& schemaPointerAbs,
        NormalizedDocumentType& core,
        StateAllocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity) :
    GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>
    (schemaDocument,
     root,
     (char*)basePath, basePathSize * sizeof(Ch),
#if RAPIDJSON_SCHEMA_VERBOSE
     0, // depth
#endif
     allocator,
     schemaStackCapacity,
     documentStackCapacity),
    normalized_(0, &this->GetStateAllocator()),
    normalization_depth_(0),
    validator_index_(0), child_validators_(0),
    temp_instanceRef_(nullptr), temp_schemaRef_(nullptr),
    schemaPointerAbs_(schemaPointerAbs, &normalized_.GetAllocator()) {
    normalized_.core_ = &core;
    normalized_.SetDocumentStack(&this->documentStack_);
    if (basePath) {
      PointerType basePointer(basePath, basePathSize,
			      &normalized_.GetAllocator());
      normalized_.SetBasePointer(basePointer);
    }
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
	const PointerType& schemaPointerAbs,
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
    normalized_(&normalized, validator_index),
    normalization_depth_(normalization_depth), validator_index_(validator_index), child_validators_(0),
    temp_instanceRef_(nullptr), temp_schemaRef_(nullptr),
    schemaPointerAbs_(schemaPointerAbs, &normalized_.GetAllocator()) {
    normalized_.SetDocumentStack(&this->documentStack_);
    if (basePath) {
      PointerType basePointer((Ch*)basePath, basePathSize / sizeof(Ch),
			      &normalized_.GetAllocator());
      normalized_.SetBasePointer(basePointer);
    }
  }
  
  static const size_t kDefaultSchemaStackCapacity = 1024;
  static const size_t kDefaultDocumentStackCapacity = 256;
  NormalizedDocumentType normalized_;
  unsigned normalization_depth_;
  unsigned validator_index_;
  unsigned child_validators_;
  const SValue* temp_instanceRef_;
  const SValue* temp_schemaRef_;
  const PointerType schemaPointerAbs_;

public:

  //! Get the normalized document instance.
  const NormalizedDocumentType& GetNormalizedDoc() const
  { return normalized_; }
  
  //! Get the normalized document instance.
  NormalizedDocumentType& GetNormalizedDoc()
  { return normalized_; }
  
  //! Get the normalized version of the parsed document.
  const typename NormalizedDocumentType::ValueType& GetNormalized() const
  { return normalized_.GetDocument(); }

  //! Check if the document was normalized.
  bool WasNormalized() const { return normalized_.WasModified(); }

  bool BeginValue() OVERRIDE_CXX11 {
    bool emptyInit = this->schemaStack_.Empty();
    if (!GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>::BeginValue())
      return false;
    normalization_depth_++;
    this->CurrentContext().normalized = &normalized_;
    if (emptyInit)
      this->CurrentContext().schemaPointerAbs = schemaPointerAbs_;
    return true;
  }

  bool EndValue() OVERRIDE_CXX11 {
    if (!GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>::EndValue())
      return false;
    normalization_depth_--;
    if (normalization_depth_ == 0)
      normalized_.FinalizeFromStack();
    return true;
  }
  
  //! Implementation of ISchemaValidator
  unsigned GetValidatorID() const OVERRIDE_CXX11 { return validator_index_; }
  
  //! Implementation of ISchemaStateFactory<SchemaType>
  ISchemaValidator* CreateSchemaValidator(const SchemaType& root, const bool inheritContinueOnErrors, PointerType p) OVERRIDE_CXX11 {
    ISchemaValidator* sv = new (this->GetStateAllocator().Malloc(sizeof(GenericSchemaNormalizer))) GenericSchemaNormalizer(*this->schemaDocument_, root, this->documentStack_.template Bottom<char>(), this->documentStack_.GetSize(),
#if RAPIDJSON_SCHEMA_VERBOSE
        depth_ + 1,
#endif
        normalization_depth_ + 1,
        normalized_,
        child_validators_++,
        p,
        &this->GetStateAllocator());
    sv->SetValidateFlags(inheritContinueOnErrors ? this->GetValidateFlags() : this->GetValidateFlags() & ~(unsigned)kValidateContinueOnErrorFlag);
    return sv;
  }

  void DestroySchemaValidator(ISchemaValidator* validator) OVERRIDE_CXX11 {
    GenericSchemaNormalizer* v = static_cast<GenericSchemaNormalizer*>(validator);
    v->~GenericSchemaNormalizer();
    StateAllocator::Free(v);
  }

  //! Gets the JSON pointer pointed to the invalid schema.
  //  If reporting all errors, the stack will be empty.
  PointerType GetInvalidSchemaPointer() const OVERRIDE_CXX11 {
    if (temp_schemaRef_)
      return PointerType(temp_schemaRef_->GetString(),
			 temp_schemaRef_->GetStringLength());
    PointerType out;
    if (!this->schemaStack_.Empty())
      out = normalized_.GetSchemaPointer(this->CurrentSchema());
    else
      out = GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>::GetInvalidSchemaPointer();
    return out;
  }

  //! Gets the JSON pointer pointed to the invalid value.
  //  If reporting all errors, the stack will be empty.
  PointerType GetInvalidDocumentPointer() const OVERRIDE_CXX11 {
    if (temp_instanceRef_)
      return PointerType(temp_instanceRef_->GetString(),
			 temp_instanceRef_->GetStringLength());
    return normalized_.GetInstancePointerBase();
  }

  bool EndMissingPropertiesShared(const SValue& instanceRef,
				  const SValue& schemaRef) OVERRIDE_CXX11 {
    temp_instanceRef_ = &instanceRef;
    temp_schemaRef_ = &schemaRef;
    bool out = GenericSchemaValidator<SchemaDocumentType, OutputHandler, StateAllocator>::EndMissingPropertiesShared(instanceRef, schemaRef);
    temp_instanceRef_ = nullptr;
    temp_schemaRef_ = nullptr;
    return out;
  }
};

typedef GenericSchemaNormalizer<SchemaDocument> SchemaNormalizer;

///////////////////////////////////////////////////////////////////////////////
// SchemaEncoder
//
//! Handler to encode schema from an object
template<typename Encoding, typename Allocator = RAPIDJSON_DEFAULT_ALLOCATOR, typename StackAllocator = RAPIDJSON_DEFAULT_STACK_ALLOCATOR>
class GenericSchemaEncoder {
public:
  typedef typename Encoding::Ch Ch;
  typedef GenericValue<Encoding, Allocator> ValueType;
  typedef GenericDocument<Encoding, Allocator> DocumentType;
  typedef internal::Schema<GenericSchemaDocument<ValueType> > SchemaType;
  GenericSchemaEncoder(Allocator* allocator = 0,
		       StackAllocator* stackAllocator = 0,
		       size_t stackCapacity = kDefaultStackCapacity) :
    document_(allocator, stackCapacity, stackAllocator) {}
#define ADD_TYPE_(method)						\
  if (!AddKey(SchemaType::GetTypeString()))				\
    return false;							\
  if (!AddString(SchemaType::Get ## method ## String()))		\
    return false
#define BASIC_TYPE_(method, name, args)					\
  bool method args {							\
    if (!BeginValue()) return false;					\
    ADD_TYPE_(name);							\
    return EndValue(1);							\
  }
  BASIC_TYPE_(Null, Null, ())
  BASIC_TYPE_(Bool, Boolean, (bool))
  BASIC_TYPE_(Int, Integer, (int))
  BASIC_TYPE_(Uint, Integer, (unsigned))
  BASIC_TYPE_(Int64, Integer, (int64_t))
  BASIC_TYPE_(Uint64, Integer, (uint64_t))
  BASIC_TYPE_(Double, Number, (double))
  BASIC_TYPE_(String, String, (const Ch*, SizeType, bool))
#undef BASIC_TYPE_
  bool StartObject() {
    if (!BeginValue()) return false;
    ADD_TYPE_(Object);
    if (!AddKey(ValueType::GetPropertiesString())) return false;
    return document_.StartObject();
  }
  bool Key(const Ch* str, SizeType len, bool copy) {
    return document_.Key(str, len, copy);
  }
  bool EndObject(SizeType memberCount) {
    if (!document_.EndObject(memberCount)) return false;
    return EndValue(2); // type & properties
  }
  bool StartArray() {
    if (!BeginValue()) return false;
    ADD_TYPE_(Array);
    if (!AddKey(ValueType::GetItemsString())) return false;
    return document_.StartArray();
  }
  bool EndArray(SizeType elementCount) {
    if (!document_.EndArray(elementCount)) return false;
    return EndValue(2); // type & items
  }
  template <typename YggSchemaValueType>
  bool YggdrasilString(const Ch*, SizeType, bool, YggSchemaValueType& schema) {
    return schema.Accept(document_);
  }
  template <typename YggSchemaValueType>
  bool YggdrasilStartObject(YggSchemaValueType& schema) {
    if (!schema.Accept(document_)) return false;
    return document_.StartObject();
  }
  bool YggdrasilEndObject(SizeType memberCount) {
    if (!EndValue(memberCount)) return false;
    document_.StackPop();
    // ValueType* last = document_.StackPop();
    // ValueType* top = document_.StackTop();
    // if (!(top->IsObject() && last->IsObject())) return false;
    // for (typename ValueType::MemberIterator it = last->MemberBegin(); it != last->MemberEnd(); it++) {
    //   top->AddMember(it->name, it->value, document_.GetAllocator());
    // }
    return true;
  }
  template <typename Handler>
  bool Accept(Handler& handler) {
    document_.FinalizeFromStack();
    return document_.Accept(handler);
  }
  DocumentType& GetSchema() {
    document_.FinalizeFromStack();
    return document_;
  }
  
private:
  bool AddKey(const ValueType& key) {
    return document_.Key(key.GetString(), key.GetStringLength(), true);
  }
  bool AddString(const ValueType& str) {
    return document_.String(str.GetString(), str.GetStringLength(), true);
  }
  bool BeginValue() {
    return document_.StartObject();
  }
  bool EndValue(SizeType memberCount) {
    return document_.EndObject(memberCount);
  }
  
  static const size_t kDefaultStackCapacity = 1024;
  DocumentType document_;
};

typedef GenericSchemaEncoder<UTF8<char> > SchemaEncoder;

#endif // RAPIDJSON_YGGDRASIL

RAPIDJSON_NAMESPACE_END
RAPIDJSON_DIAG_POP

#endif // RAPIDJSON_SCHEMA_H_
