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

#ifndef RAPIDJSON_DOCUMENT_H_
#define RAPIDJSON_DOCUMENT_H_

/*! \file document.h */

#include "reader.h"
#include "internal/meta.h"
#include "internal/strfunc.h"
#include "memorystream.h"
#include "encodedstream.h"
#include <new>      // placement new
#include <limits>
#ifdef __cpp_lib_three_way_comparison
#include <compare>
#endif
#ifdef RAPIDJSON_YGGDRASIL
#include "stringbuffer.h"
#include "base64.h"
#include "ply.h"
#include "obj.h"
#include "pyrj.h"
#include <complex>
#include "units.h"
#include "precision.h"
#include "va_list.h"
#endif // RAPIDJSON_YGGDRASIL

RAPIDJSON_DIAG_PUSH
#ifdef __clang__
RAPIDJSON_DIAG_OFF(padded)
RAPIDJSON_DIAG_OFF(switch-enum)
RAPIDJSON_DIAG_OFF(c++98-compat)
#elif defined(_MSC_VER)
RAPIDJSON_DIAG_OFF(4127) // conditional expression is constant
RAPIDJSON_DIAG_OFF(4244) // conversion from kXxxFlags to 'uint16_t', possible loss of data
#endif

#ifdef __GNUC__
RAPIDJSON_DIAG_OFF(effc++)
#endif // __GNUC__

#ifdef GetObject
// see https://github.com/Tencent/rapidjson/issues/1448
// a former included windows.h might have defined a macro called GetObject, which affects
// GetObject defined here. This ensures the macro does not get applied
#pragma push_macro("GetObject")
#define RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#undef GetObject
#endif

#ifndef RAPIDJSON_NOMEMBERITERATORCLASS
#include <iterator> // std::random_access_iterator_tag
#endif

#if RAPIDJSON_USE_MEMBERSMAP
#include <map> // std::multimap
#endif

RAPIDJSON_NAMESPACE_BEGIN

// Forward declaration.
template <typename Encoding, typename Allocator>
class GenericValue;

template <typename Encoding, typename Allocator, typename StackAllocator>
class GenericDocument;

/*! \def RAPIDJSON_DEFAULT_ALLOCATOR
    \ingroup RAPIDJSON_CONFIG
    \brief Allows to choose default allocator.

    User can define this to use CrtAllocator or MemoryPoolAllocator.
*/
#ifndef RAPIDJSON_DEFAULT_ALLOCATOR
#define RAPIDJSON_DEFAULT_ALLOCATOR ::RAPIDJSON_NAMESPACE::MemoryPoolAllocator< ::RAPIDJSON_NAMESPACE::CrtAllocator >
#endif

/*! \def RAPIDJSON_DEFAULT_STACK_ALLOCATOR
    \ingroup RAPIDJSON_CONFIG
    \brief Allows to choose default stack allocator for Document.

    User can define this to use CrtAllocator or MemoryPoolAllocator.
*/
#ifndef RAPIDJSON_DEFAULT_STACK_ALLOCATOR
#define RAPIDJSON_DEFAULT_STACK_ALLOCATOR ::RAPIDJSON_NAMESPACE::CrtAllocator
#endif

/*! \def RAPIDJSON_VALUE_DEFAULT_OBJECT_CAPACITY
    \ingroup RAPIDJSON_CONFIG
    \brief User defined kDefaultObjectCapacity value.

    User can define this as any natural number.
*/
#ifndef RAPIDJSON_VALUE_DEFAULT_OBJECT_CAPACITY
// number of objects that rapidjson::Value allocates memory for by default
#define RAPIDJSON_VALUE_DEFAULT_OBJECT_CAPACITY 16
#endif

/*! \def RAPIDJSON_VALUE_DEFAULT_ARRAY_CAPACITY
    \ingroup RAPIDJSON_CONFIG
    \brief User defined kDefaultArrayCapacity value.

    User can define this as any natural number.
*/
#ifndef RAPIDJSON_VALUE_DEFAULT_ARRAY_CAPACITY
// number of array elements that rapidjson::Value allocates memory for by default
#define RAPIDJSON_VALUE_DEFAULT_ARRAY_CAPACITY 16
#endif

//! Name-value pair in a JSON object value.
/*!
    This class was internal to GenericValue. It used to be a inner struct.
    But a compiler (IBM XL C/C++ for AIX) have reported to have problem with that so it moved as a namespace scope struct.
    https://code.google.com/p/rapidjson/issues/detail?id=64
*/
template <typename Encoding, typename Allocator> 
class GenericMember {
public:
    GenericValue<Encoding, Allocator> name;     //!< name of member (must be a string)
    GenericValue<Encoding, Allocator> value;    //!< value of member.

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move constructor in C++11
    GenericMember(GenericMember&& rhs) RAPIDJSON_NOEXCEPT
        : name(std::move(rhs.name)),
          value(std::move(rhs.value))
    {
    }

    //! Move assignment in C++11
    GenericMember& operator=(GenericMember&& rhs) RAPIDJSON_NOEXCEPT {
        return *this = static_cast<GenericMember&>(rhs);
    }
#endif

    //! Assignment with move semantics.
    /*! \param rhs Source of the assignment. Its name and value will become a null value after assignment.
    */
    GenericMember& operator=(GenericMember& rhs) RAPIDJSON_NOEXCEPT {
        if (RAPIDJSON_LIKELY(this != &rhs)) {
            name = rhs.name;
            value = rhs.value;
        }
        return *this;
    }

    // swap() for std::sort() and other potential use in STL.
    friend inline void swap(GenericMember& a, GenericMember& b) RAPIDJSON_NOEXCEPT {
        a.name.Swap(b.name);
        a.value.Swap(b.value);
    }

private:
    //! Copy constructor is not permitted.
    GenericMember(const GenericMember& rhs);
};

///////////////////////////////////////////////////////////////////////////////
// GenericMemberIterator

#ifndef RAPIDJSON_NOMEMBERITERATORCLASS

//! (Constant) member iterator for a JSON object value
/*!
    \tparam Const Is this a constant iterator?
    \tparam Encoding    Encoding of the value. (Even non-string values need to have the same encoding in a document)
    \tparam Allocator   Allocator type for allocating memory of object, array and string.

    This class implements a Random Access Iterator for GenericMember elements
    of a GenericValue, see ISO/IEC 14882:2003(E) C++ standard, 24.1 [lib.iterator.requirements].

    \note This iterator implementation is mainly intended to avoid implicit
        conversions from iterator values to \c NULL,
        e.g. from GenericValue::FindMember.

    \note Define \c RAPIDJSON_NOMEMBERITERATORCLASS to fall back to a
        pointer-based implementation, if your platform doesn't provide
        the C++ <iterator> header.

    \see GenericMember, GenericValue::MemberIterator, GenericValue::ConstMemberIterator
 */
template <bool Const, typename Encoding, typename Allocator>
class GenericMemberIterator {

    friend class GenericValue<Encoding,Allocator>;
    template <bool, typename, typename> friend class GenericMemberIterator;

    typedef GenericMember<Encoding,Allocator> PlainType;
    typedef typename internal::MaybeAddConst<Const,PlainType>::Type ValueType;

public:
    //! Iterator type itself
    typedef GenericMemberIterator Iterator;
    //! Constant iterator type
    typedef GenericMemberIterator<true,Encoding,Allocator>  ConstIterator;
    //! Non-constant iterator type
    typedef GenericMemberIterator<false,Encoding,Allocator> NonConstIterator;

    /** \name std::iterator_traits support */
    //@{
    typedef ValueType      value_type;
    typedef ValueType *    pointer;
    typedef ValueType &    reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;
    //@}

    //! Pointer to (const) GenericMember
    typedef pointer         Pointer;
    //! Reference to (const) GenericMember
    typedef reference       Reference;
    //! Signed integer type (e.g. \c ptrdiff_t)
    typedef difference_type DifferenceType;

    //! Default constructor (singular value)
    /*! Creates an iterator pointing to no element.
        \note All operations, except for comparisons, are undefined on such values.
     */
    GenericMemberIterator() : ptr_() {}

    //! Iterator conversions to more const
    /*!
        \param it (Non-const) iterator to copy from

        Allows the creation of an iterator from another GenericMemberIterator
        that is "less const".  Especially, creating a non-constant iterator
        from a constant iterator are disabled:
        \li const -> non-const (not ok)
        \li const -> const (ok)
        \li non-const -> const (ok)
        \li non-const -> non-const (ok)

        \note If the \c Const template parameter is already \c false, this
            constructor effectively defines a regular copy-constructor.
            Otherwise, the copy constructor is implicitly defined.
    */
    GenericMemberIterator(const NonConstIterator & it) : ptr_(it.ptr_) {}
    Iterator& operator=(const NonConstIterator & it) { ptr_ = it.ptr_; return *this; }

    //! @name stepping
    //@{
    Iterator& operator++(){ ++ptr_; return *this; }
    Iterator& operator--(){ --ptr_; return *this; }
    Iterator  operator++(int){ Iterator old(*this); ++ptr_; return old; }
    Iterator  operator--(int){ Iterator old(*this); --ptr_; return old; }
    //@}

    //! @name increment/decrement
    //@{
    Iterator operator+(DifferenceType n) const { return Iterator(ptr_+n); }
    Iterator operator-(DifferenceType n) const { return Iterator(ptr_-n); }

    Iterator& operator+=(DifferenceType n) { ptr_+=n; return *this; }
    Iterator& operator-=(DifferenceType n) { ptr_-=n; return *this; }
    //@}

    //! @name relations
    //@{
    template <bool Const_> bool operator==(const GenericMemberIterator<Const_, Encoding, Allocator>& that) const { return ptr_ == that.ptr_; }
    template <bool Const_> bool operator!=(const GenericMemberIterator<Const_, Encoding, Allocator>& that) const { return ptr_ != that.ptr_; }
    template <bool Const_> bool operator<=(const GenericMemberIterator<Const_, Encoding, Allocator>& that) const { return ptr_ <= that.ptr_; }
    template <bool Const_> bool operator>=(const GenericMemberIterator<Const_, Encoding, Allocator>& that) const { return ptr_ >= that.ptr_; }
    template <bool Const_> bool operator< (const GenericMemberIterator<Const_, Encoding, Allocator>& that) const { return ptr_ < that.ptr_; }
    template <bool Const_> bool operator> (const GenericMemberIterator<Const_, Encoding, Allocator>& that) const { return ptr_ > that.ptr_; }

#ifdef __cpp_lib_three_way_comparison
    template <bool Const_> std::strong_ordering operator<=>(const GenericMemberIterator<Const_, Encoding, Allocator>& that) const { return ptr_ <=> that.ptr_; }
#endif
    //@}

    //! @name dereference
    //@{
    Reference operator*() const { return *ptr_; }
    Pointer   operator->() const { return ptr_; }
    Reference operator[](DifferenceType n) const { return ptr_[n]; }
    //@}

    //! Distance
    DifferenceType operator-(ConstIterator that) const { return ptr_-that.ptr_; }

private:
    //! Internal constructor from plain pointer
    explicit GenericMemberIterator(Pointer p) : ptr_(p) {}

    Pointer ptr_; //!< raw pointer
};

#else // RAPIDJSON_NOMEMBERITERATORCLASS

// class-based member iterator implementation disabled, use plain pointers

template <bool Const, typename Encoding, typename Allocator>
class GenericMemberIterator;

//! non-const GenericMemberIterator
template <typename Encoding, typename Allocator>
class GenericMemberIterator<false,Encoding,Allocator> {
public:
    //! use plain pointer as iterator type
    typedef GenericMember<Encoding,Allocator>* Iterator;
};
//! const GenericMemberIterator
template <typename Encoding, typename Allocator>
class GenericMemberIterator<true,Encoding,Allocator> {
public:
    //! use plain const pointer as iterator type
    typedef const GenericMember<Encoding,Allocator>* Iterator;
};

#endif // RAPIDJSON_NOMEMBERITERATORCLASS

///////////////////////////////////////////////////////////////////////////////
// GenericStringRef

//! Reference to a constant string (not taking a copy)
/*!
    \tparam CharType character type of the string

    This helper class is used to automatically infer constant string
    references for string literals, especially from \c const \b (!)
    character arrays.

    The main use is for creating JSON string values without copying the
    source string via an \ref Allocator.  This requires that the referenced
    string pointers have a sufficient lifetime, which exceeds the lifetime
    of the associated GenericValue.

    \b Example
    \code
    Value v("foo");   // ok, no need to copy & calculate length
    const char foo[] = "foo";
    v.SetString(foo); // ok

    const char* bar = foo;
    // Value x(bar); // not ok, can't rely on bar's lifetime
    Value x(StringRef(bar)); // lifetime explicitly guaranteed by user
    Value y(StringRef(bar, 3));  // ok, explicitly pass length
    \endcode

    \see StringRef, GenericValue::SetString
*/
template<typename CharType>
struct GenericStringRef {
    typedef CharType Ch; //!< character type of the string

    //! Create string reference from \c const character array
#ifndef __clang__ // -Wdocumentation
    /*!
        This constructor implicitly creates a constant string reference from
        a \c const character array.  It has better performance than
        \ref StringRef(const CharType*) by inferring the string \ref length
        from the array length, and also supports strings containing null
        characters.

        \tparam N length of the string, automatically inferred

        \param str Constant character array, lifetime assumed to be longer
            than the use of the string in e.g. a GenericValue

        \post \ref s == str

        \note Constant complexity.
        \note There is a hidden, private overload to disallow references to
            non-const character arrays to be created via this constructor.
            By this, e.g. function-scope arrays used to be filled via
            \c snprintf are excluded from consideration.
            In such cases, the referenced string should be \b copied to the
            GenericValue instead.
     */
#endif
    template<SizeType N>
    GenericStringRef(const CharType (&str)[N]) RAPIDJSON_NOEXCEPT
        : s(str), length(N-1) {}

    //! Explicitly create string reference from \c const character pointer
#ifndef __clang__ // -Wdocumentation
    /*!
        This constructor can be used to \b explicitly  create a reference to
        a constant string pointer.

        \see StringRef(const CharType*)

        \param str Constant character pointer, lifetime assumed to be longer
            than the use of the string in e.g. a GenericValue

        \post \ref s == str

        \note There is a hidden, private overload to disallow references to
            non-const character arrays to be created via this constructor.
            By this, e.g. function-scope arrays used to be filled via
            \c snprintf are excluded from consideration.
            In such cases, the referenced string should be \b copied to the
            GenericValue instead.
     */
#endif
    explicit GenericStringRef(const CharType* str)
        : s(str), length(NotNullStrLen(str)) {}

    //! Create constant string reference from pointer and length
#ifndef __clang__ // -Wdocumentation
    /*! \param str constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
        \param len length of the string, excluding the trailing NULL terminator

        \post \ref s == str && \ref length == len
        \note Constant complexity.
     */
#endif
    GenericStringRef(const CharType* str, SizeType len)
        : s(RAPIDJSON_LIKELY(str) ? str : emptyString), length(len) { RAPIDJSON_ASSERT(str != 0 || len == 0u); }

    GenericStringRef(const GenericStringRef& rhs) : s(rhs.s), length(rhs.length) {}

    //! implicit conversion to plain CharType pointer
    operator const Ch *() const { return s; }

    const Ch* const s; //!< plain CharType pointer
    const SizeType length; //!< length of the string (excluding the trailing NULL terminator)

private:
    SizeType NotNullStrLen(const CharType* str) {
        RAPIDJSON_ASSERT(str != 0);
        return internal::StrLen(str);
    }

    /// Empty string - used when passing in a NULL pointer
    static const Ch emptyString[];

    //! Disallow construction from non-const array
    template<SizeType N>
    GenericStringRef(CharType (&str)[N]) /* = delete */;
    //! Copy assignment operator not permitted - immutable type
    GenericStringRef& operator=(const GenericStringRef& rhs) /* = delete */;
};

template<typename CharType>
const CharType GenericStringRef<CharType>::emptyString[] = { CharType() };

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
    can be used to avoid copying a character string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.
    \tparam CharType Character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef

    \see GenericValue::GenericValue(StringRefType), GenericValue::operator=(StringRefType), GenericValue::SetString(StringRefType), GenericValue::PushBack(StringRefType, Allocator&), GenericValue::AddMember
*/
template<typename CharType>
inline GenericStringRef<CharType> StringRef(const CharType* str) {
    return GenericStringRef<CharType>(str);
}

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
    can be used to avoid copying a character string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.

    This version has better performance with supplied length, and also
    supports string containing null characters.

    \tparam CharType character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \param length The length of source string.
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef
*/
template<typename CharType>
inline GenericStringRef<CharType> StringRef(const CharType* str, size_t length) {
    return GenericStringRef<CharType>(str, SizeType(length));
}

#if RAPIDJSON_HAS_STDSTRING
//! Mark a string object as constant string
/*! Mark a string object (e.g. \c std::string) as a "string literal".
    This function can be used to avoid copying a string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.

    \tparam CharType character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef
    \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
*/
template<typename CharType>
inline GenericStringRef<CharType> StringRef(const std::basic_string<CharType>& str) {
    return GenericStringRef<CharType>(str.data(), SizeType(str.size()));
}
#endif

///////////////////////////////////////////////////////////////////////////////
// GenericValue type traits
namespace internal {

template <typename T, typename Encoding = void, typename Allocator = void>
struct IsGenericValueImpl : FalseType {};

// select candidates according to nested encoding and allocator types
template <typename T> struct IsGenericValueImpl<T, typename Void<typename T::EncodingType>::Type, typename Void<typename T::AllocatorType>::Type>
    : IsBaseOf<GenericValue<typename T::EncodingType, typename T::AllocatorType>, T>::Type {};

// helper to match arbitrary GenericValue instantiations, including derived classes
template <typename T> struct IsGenericValue : IsGenericValueImpl<T>::Type {};

} // namespace internal

///////////////////////////////////////////////////////////////////////////////
// TypeHelper

namespace internal {

template <typename ValueType, typename T>
struct TypeHelper {};

template<typename ValueType> 
struct TypeHelper<ValueType, bool> {
    static bool Is(const ValueType& v) { return v.IsBool(); }
    static bool Get(const ValueType& v) { return v.GetBool(); }
    static ValueType& Set(ValueType& v, bool data) { return v.SetBool(data); }
    static ValueType& Set(ValueType& v, bool data, typename ValueType::AllocatorType&) { return v.SetBool(data); }
};

template<typename ValueType> 
struct TypeHelper<ValueType, int> {
    static bool Is(const ValueType& v) { return v.IsInt(); }
    static int Get(const ValueType& v) { return v.GetInt(); }
    static ValueType& Set(ValueType& v, int data) { return v.SetInt(data); }
    static ValueType& Set(ValueType& v, int data, typename ValueType::AllocatorType&) { return v.SetInt(data); }
};

template<typename ValueType> 
struct TypeHelper<ValueType, unsigned> {
    static bool Is(const ValueType& v) { return v.IsUint(); }
    static unsigned Get(const ValueType& v) { return v.GetUint(); }
    static ValueType& Set(ValueType& v, unsigned data) { return v.SetUint(data); }
    static ValueType& Set(ValueType& v, unsigned data, typename ValueType::AllocatorType&) { return v.SetUint(data); }
};

#ifdef _MSC_VER
RAPIDJSON_STATIC_ASSERT(sizeof(long) == sizeof(int));
template<typename ValueType>
struct TypeHelper<ValueType, long> {
    static bool Is(const ValueType& v) { return v.IsInt(); }
    static long Get(const ValueType& v) { return v.GetInt(); }
    static ValueType& Set(ValueType& v, long data) { return v.SetInt(data); }
    static ValueType& Set(ValueType& v, long data, typename ValueType::AllocatorType&) { return v.SetInt(data); }
};

RAPIDJSON_STATIC_ASSERT(sizeof(unsigned long) == sizeof(unsigned));
template<typename ValueType>
struct TypeHelper<ValueType, unsigned long> {
    static bool Is(const ValueType& v) { return v.IsUint(); }
    static unsigned long Get(const ValueType& v) { return v.GetUint(); }
    static ValueType& Set(ValueType& v, unsigned long data) { return v.SetUint(data); }
    static ValueType& Set(ValueType& v, unsigned long data, typename ValueType::AllocatorType&) { return v.SetUint(data); }
};
#endif

template<typename ValueType> 
struct TypeHelper<ValueType, int64_t> {
    static bool Is(const ValueType& v) { return v.IsInt64(); }
    static int64_t Get(const ValueType& v) { return v.GetInt64(); }
    static ValueType& Set(ValueType& v, int64_t data) { return v.SetInt64(data); }
    static ValueType& Set(ValueType& v, int64_t data, typename ValueType::AllocatorType&) { return v.SetInt64(data); }
};

template<typename ValueType> 
struct TypeHelper<ValueType, uint64_t> {
    static bool Is(const ValueType& v) { return v.IsUint64(); }
    static uint64_t Get(const ValueType& v) { return v.GetUint64(); }
    static ValueType& Set(ValueType& v, uint64_t data) { return v.SetUint64(data); }
    static ValueType& Set(ValueType& v, uint64_t data, typename ValueType::AllocatorType&) { return v.SetUint64(data); }
};

template<typename ValueType> 
struct TypeHelper<ValueType, double> {
    static bool Is(const ValueType& v) { return v.IsDouble(); }
    static double Get(const ValueType& v) { return v.GetDouble(); }
    static ValueType& Set(ValueType& v, double data) { return v.SetDouble(data); }
    static ValueType& Set(ValueType& v, double data, typename ValueType::AllocatorType&) { return v.SetDouble(data); }
};

template<typename ValueType> 
struct TypeHelper<ValueType, float> {
    static bool Is(const ValueType& v) { return v.IsFloat(); }
    static float Get(const ValueType& v) { return v.GetFloat(); }
    static ValueType& Set(ValueType& v, float data) { return v.SetFloat(data); }
    static ValueType& Set(ValueType& v, float data, typename ValueType::AllocatorType&) { return v.SetFloat(data); }
};

template<typename ValueType> 
struct TypeHelper<ValueType, const typename ValueType::Ch*> {
    typedef const typename ValueType::Ch* StringType;
    static bool Is(const ValueType& v) { return v.IsString(); }
    static StringType Get(const ValueType& v) { return v.GetString(); }
    static ValueType& Set(ValueType& v, const StringType data) { return v.SetString(typename ValueType::StringRefType(data)); }
    static ValueType& Set(ValueType& v, const StringType data, typename ValueType::AllocatorType& a) { return v.SetString(data, a); }
};

#if RAPIDJSON_HAS_STDSTRING
template<typename ValueType> 
struct TypeHelper<ValueType, std::basic_string<typename ValueType::Ch> > {
    typedef std::basic_string<typename ValueType::Ch> StringType;
    static bool Is(const ValueType& v) { return v.IsString(); }
    static StringType Get(const ValueType& v) { return StringType(v.GetString(), v.GetStringLength()); }
    static ValueType& Set(ValueType& v, const StringType& data, typename ValueType::AllocatorType& a) { return v.SetString(data, a); }
};
#endif

template<typename ValueType> 
struct TypeHelper<ValueType, typename ValueType::Array> {
    typedef typename ValueType::Array ArrayType;
    static bool Is(const ValueType& v) { return v.IsArray(); }
    static ArrayType Get(ValueType& v) { return v.GetArray(); }
    static ValueType& Set(ValueType& v, ArrayType data) { return v = data; }
    static ValueType& Set(ValueType& v, ArrayType data, typename ValueType::AllocatorType&) { return v = data; }
};

template<typename ValueType> 
struct TypeHelper<ValueType, typename ValueType::ConstArray> {
    typedef typename ValueType::ConstArray ArrayType;
    static bool Is(const ValueType& v) { return v.IsArray(); }
    static ArrayType Get(const ValueType& v) { return v.GetArray(); }
};

template<typename ValueType> 
struct TypeHelper<ValueType, typename ValueType::Object> {
    typedef typename ValueType::Object ObjectType;
    static bool Is(const ValueType& v) { return v.IsObject(); }
    static ObjectType Get(ValueType& v) { return v.GetObject(); }
    static ValueType& Set(ValueType& v, ObjectType data) { return v = data; }
    static ValueType& Set(ValueType& v, ObjectType data, typename ValueType::AllocatorType&) { return v = data; }
};

template<typename ValueType> 
struct TypeHelper<ValueType, typename ValueType::ConstObject> {
    typedef typename ValueType::ConstObject ObjectType;
    static bool Is(const ValueType& v) { return v.IsObject(); }
    static ObjectType Get(const ValueType& v) { return v.GetObject(); }
};

#ifdef RAPIDJSON_YGGDRASIL

static void* __StaticNull = NULL;
static bool __StaticFalse = false;
static bool __StaticTrue = true;

// Yggdrasil TypeHelper structs
// uint
template<typename ValueType>
struct TypeHelper<ValueType, uint8_t> {
  static bool Is(const ValueType& v) { return v.template IsScalar<uint8_t>(); }
  static uint8_t Get(const ValueType& v) { return v.template GetScalar<uint8_t>(); }
  static ValueType& Set(ValueType& v, uint8_t data) { return v.SetScalar(data); }
};
template<typename ValueType>
struct TypeHelper<ValueType, uint16_t> {
  static bool Is(const ValueType& v) { return v.template IsScalar<uint16_t>(); }
  static uint16_t Get(const ValueType& v) { return v.template GetScalar<uint16_t>(); }
  static ValueType& Set(ValueType& v, uint16_t data) { return v.SetScalar(data); }
};
// int
template<typename ValueType>
struct TypeHelper<ValueType, int8_t> {
  static bool Is(const ValueType& v) { return v.template IsScalar<int8_t>(); }
  static int8_t Get(const ValueType& v) { return v.template GetScalar<int8_t>(); }
  static ValueType& Set(ValueType& v, int8_t data) { return v.SetScalar(data); }
};
template<typename ValueType>
struct TypeHelper<ValueType, int16_t> {
  static bool Is(const ValueType& v) { return v.template IsScalar<int16_t>(); }
  static int16_t Get(const ValueType& v) { return v.template GetScalar<int16_t>(); }
  static ValueType& Set(ValueType& v, int16_t data) { return v.SetScalar(data); }
};
// float
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
template<typename ValueType>
struct TypeHelper<ValueType, long double> {
  static bool Is(const ValueType& v) { return v.template IsScalar<long double>(); }
  static long double Get(const ValueType& v) { return v.template GetScalar<long double>(); }
  static ValueType& Set(ValueType& v, long double data) { return v.SetScalar(data); }
};
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
// complex
template<typename ValueType>
struct TypeHelper<ValueType, std::complex<float> > {
  static bool Is(const ValueType& v) { return v.template IsScalar<std::complex<float> >(); }
  static std::complex<float> Get(const ValueType& v) { return v.template GetScalar<std::complex<float> >(); }
  static ValueType& Set(ValueType& v, std::complex<float> data) { return v.SetScalar(data); }
};
template<typename ValueType>
struct TypeHelper<ValueType, std::complex<double> > {
  static bool Is(const ValueType& v) { return v.template IsScalar<std::complex<double> >(); }
  static std::complex<double> Get(const ValueType& v) { return v.template GetScalar<std::complex<double> >(); }
  static ValueType& Set(ValueType& v, std::complex<double> data) { return v.SetScalar(data); }
};
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
template<typename ValueType>
struct TypeHelper<ValueType, std::complex<long double> > {
  static bool Is(const ValueType& v) { return v.template IsScalar<std::complex<long double> >(); }
  static std::complex<long double> Get(const ValueType& v) { return v.template GetScalar<std::complex<long double> >(); }
  static ValueType& Set(ValueType& v, std::complex<long double> data) { return v.SetScalar(data); }
};
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
// python instance
template<typename ValueType>
struct TypeHelper<ValueType, PyObject*> {
  static bool Is(const ValueType& v) { return v.template IsPythonInstance(); }
  static PyObject* Get(const ValueType& v) { return v.template GetPythonInstance(); }
  static ValueType& Set(ValueType& v, PyObject* data) { return v.SetPythonInstance(data); }
};
#endif // YGGDRASIL_DISABLE_PYTHON_C_API
// obj & ply
template<typename ValueType>
struct TypeHelper<ValueType, ObjWavefront> {
  static bool Is(const ValueType& v) { return v.template IsObjWavefront(); }
  static ObjWavefront Get(const ValueType& v) { return v.template GetObjWavefront(); }
  static ValueType& Set(ValueType& v, ObjWavefront data) { return v.SetObj(data); }
};
template<typename ValueType>
struct TypeHelper<ValueType, Ply> {
  static bool Is(const ValueType& v) { return v.template IsPly(); }
  static Ply Get(const ValueType& v) { return v.template GetPly(); }
  static ValueType& Set(ValueType& v, Ply data) { return v.SetPly(data); }
};

#endif // RAPIDJSON_YGGDRASIL

} // namespace internal

#ifdef RAPIDJSON_YGGDRASIL
// Geometry helpers
inline
void ObjWavefront::fromPly(const Ply& p) {
  size_t nvert = 0;
  for (std::vector<std::string>::const_iterator name = p.element_order.begin(); name != p.element_order.end(); name++) {
    if ((*name) == "vertex") {
      for (std::vector<PlyElement>::const_iterator it = p.elements.find(*name)->second.elements.begin();
	   it != p.elements.find(*name)->second.elements.end(); it++) {
	this->add_element("v", it->get_double_array());
	nvert++;
      }
    } else if ((*name) == "face") {
      for (std::vector<PlyElement>::const_iterator it = p.elements.find(*name)->second.elements.begin();
	   it != p.elements.find(*name)->second.elements.end(); it++)
	this->add_element("f", it->get_int_array(nvert));
    } else if ((*name) == "edge") {
      for (std::vector<PlyElement>::const_iterator it = p.elements.find(*name)->second.elements.begin();
	   it != p.elements.find(*name)->second.elements.end(); it++)
	this->add_element("l", it->get_int_array(nvert));
    } else
      RAPIDJSON_ASSERT(((*name) == "vertex") ||
		       ((*name) == "face") ||
		       ((*name) == "edge"));
  }
}
inline
void Ply::fromObjWavefront(const ObjWavefront& o) {
  size_t nvert = 0;
  for (std::vector<ObjElement*>::const_iterator it = o.elements.begin(); it != o.elements.end(); it++) {
    if ((*it)->code == "v") {
      this->add_element("vertex", (*it)->get_double_array());
      nvert++;
    } else if ((*it)->code == "f") {
      this->add_element("face", (*it)->get_int_array(nvert));
    } else if ((*it)->code == "l") {
      this->add_element("edge", (*it)->get_int_array(nvert));
    }
    else
      RAPIDJSON_ASSERT(((*it)->code == "v") ||
		       ((*it)->code == "f") ||
		       ((*it)->code == "l"));
  }
}

// VarArgs helpers
/*!
  @brief Count the number of variable arguments that would be returned or
    set for the type described by the schema.
  @param[in] schema Schema containing type information about the
    variables.
  @param[in] set If true, the arguments are counted for the case where
    the schema would be used to set variables.
  @returns Number of variables associated with the schema.
 */
template<typename ValueType>
size_t countVarArgs(ValueType& schema, bool set) {
  GenericDocument<typename ValueType::EncodingType,
		  typename ValueType::AllocatorType,
		  RAPIDJSON_DEFAULT_STACK_ALLOCATOR> tmp;
  return tmp.CountVarArgs(schema, set);
};

template<typename ValueType>
bool VarArgList::skip(ValueType& schema, bool set) {
  GenericDocument<typename ValueType::EncodingType,
		  typename ValueType::AllocatorType,
		  RAPIDJSON_DEFAULT_STACK_ALLOCATOR> tmp;
  return tmp.SkipVarArgs(schema, *this, set);
}

#endif // RAPIDJSON_YGGDRASIL

#ifdef RAPIDJSON_YGG_DOCUMENT_DEBUG_
#define RAPIDJSON_YGG_DOCUMENT_BASE_(method, arg)			\
  std::cerr << "Document [" << #method << "]: " << StackSize() << std::endl
#define RAPIDJSON_YGG_DOCUMENT_NOARG_(method)				\
  RAPIDJSON_YGG_DOCUMENT_BASE_(method, ())
#define RAPIDJSON_YGG_DOCUMENT_(method, ...)				\
  RAPIDJSON_YGG_DOCUMENT_BASE_(method, (__VA_ARGS__))
#else
#define RAPIDJSON_YGG_DOCUMENT_BASE_(method, arg)	 \
  {}
#define RAPIDJSON_YGG_DOCUMENT_NOARG_(method)	\
  {}
#define RAPIDJSON_YGG_DOCUMENT_(method, ...)       \
  {}
#endif // RAPIDJSON_YGG_DOCUMENT_DEBUG_

// Forward declarations
template <bool, typename> class GenericArray;
template <bool, typename> class GenericObject;

///////////////////////////////////////////////////////////////////////////////
// GenericValue

//! Represents a JSON value. Use Value for UTF8 encoding and default allocator.
/*!
    A JSON value can be one of 7 types. This class is a variant type supporting
    these types.

    Use the Value if UTF8 and default allocator

    \tparam Encoding    Encoding of the value. (Even non-string values need to have the same encoding in a document)
    \tparam Allocator   Allocator type for allocating memory of object, array and string.
*/
template <typename Encoding, typename Allocator = RAPIDJSON_DEFAULT_ALLOCATOR >
class GenericValue {
public:
    //! Name-value pair in an object.
    typedef GenericMember<Encoding, Allocator> Member;
    typedef Encoding EncodingType;                  //!< Encoding type from template parameter.
    typedef Allocator AllocatorType;                //!< Allocator type from template parameter.
    typedef typename Encoding::Ch Ch;               //!< Character type derived from Encoding.
    typedef GenericStringRef<Ch> StringRefType;     //!< Reference to a constant string
    typedef typename GenericMemberIterator<false,Encoding,Allocator>::Iterator MemberIterator;  //!< Member iterator for iterating in object.
    typedef typename GenericMemberIterator<true,Encoding,Allocator>::Iterator ConstMemberIterator;  //!< Constant member iterator for iterating in object.
    typedef GenericValue* ValueIterator;            //!< Value iterator for iterating in array.
    typedef const GenericValue* ConstValueIterator; //!< Constant value iterator for iterating in array.
    typedef GenericValue<Encoding, Allocator> ValueType;    //!< Value type of itself.
    typedef GenericArray<false, ValueType> Array;
    typedef GenericArray<true, ValueType> ConstArray;
    typedef GenericObject<false, ValueType> Object;
    typedef GenericObject<true, ValueType> ConstObject;

#ifdef RAPIDJSON_YGGDRASIL
    typedef GenericDocument<Encoding, Allocator, Allocator> SchemaValueType;
    typedef units::GenericUnits<Encoding> UnitsType;
    // typedef GenericDocument<Encoding, Allocator, RAPIDJSON_DEFAULT_STACK_ALLOCATOR> SchemaValueType;
#define YGG_SCHEMA_INIT , schema_(0)  // NULL)
#define YGG_SCHEMA_INIT_ONLY : schema_(0) // NULL)
#define YGG_SCHEMA_INIT_CONSTRUCT schema_ = 0 // NULL
#else
#define YGG_SCHEMA_INIT
#define YGG_SCHEMA_INIT_ONLY
#define YGG_SCHEMA_INIT_CONSTRUCT
#endif // RAPIDJSON_YGGDRASIL
  
    //!@name Constructors and destructor.
    //@{

    //! Default constructor creates a null value.
    GenericValue() RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT { data_.f.flags = kNullFlag; }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move constructor in C++11
    GenericValue(GenericValue&& rhs) RAPIDJSON_NOEXCEPT : data_(rhs.data_) YGG_SCHEMA_INIT {
        rhs.data_.f.flags = kNullFlag; // give up contents
#ifdef RAPIDJSON_YGGDRASIL
	RawAssignSchema(rhs);
#endif // RAPIDJSON_YGGDRASIL
    }
#endif

private:
    //! Copy constructor is not permitted.
    GenericValue(const GenericValue& rhs);

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Moving from a GenericDocument is not permitted.
    template <typename StackAllocator>
    GenericValue(GenericDocument<Encoding,Allocator,StackAllocator>&& rhs);

    //! Move assignment from a GenericDocument is not permitted.
    template <typename StackAllocator>
    GenericValue& operator=(GenericDocument<Encoding,Allocator,StackAllocator>&& rhs);
#endif

public:

    //! Constructor with JSON value type.
    /*! This creates a Value of specified type with default content.
        \param type Type of the value.
        \note Default content for number is zero.
    */
    explicit GenericValue(Type type) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT {
        static const uint16_t defaultFlags[] = {
            kNullFlag, kFalseFlag, kTrueFlag, kObjectFlag, kArrayFlag, kShortStringFlag,
            kNumberAnyFlag
        };
        RAPIDJSON_NOEXCEPT_ASSERT(type >= kNullType && type <= kNumberType);
        data_.f.flags = defaultFlags[type];

        // Use ShortString to store empty string.
        if (type == kStringType)
            data_.ss.SetLength(0);
    }

    //! Explicit copy constructor (with allocator)
    /*! Creates a copy of a Value by using the given Allocator
        \tparam SourceAllocator allocator of \c rhs
        \param rhs Value to copy from (read-only)
        \param allocator Allocator for allocating copied elements and buffers. Commonly use GenericDocument::GetAllocator().
        \param copyConstStrings Force copying of constant strings (e.g. referencing an in-situ buffer)
        \see CopyFrom()
    */
    template <typename SourceAllocator>
    GenericValue(const GenericValue<Encoding,SourceAllocator>& rhs, Allocator& allocator, bool copyConstStrings = false) YGG_SCHEMA_INIT_ONLY {
#ifdef RAPIDJSON_YGGDRASIL
        YGG_SCHEMA_INIT_CONSTRUCT;
	if (rhs.HasSchema())
	  SetValueSchema(rhs.GetValueSchema(), &allocator);
#endif // RAPIDJSON_YGGDRASIL
        switch (rhs.GetType()) {
        case kObjectType:
            DoCopyMembers(rhs, allocator, copyConstStrings);
            break;
        case kArrayType: {
                SizeType count = rhs.data_.a.size;
                GenericValue* le = reinterpret_cast<GenericValue*>(allocator.Malloc(count * sizeof(GenericValue)));
                const GenericValue<Encoding,SourceAllocator>* re = rhs.GetElementsPointer();
                for (SizeType i = 0; i < count; i++)
                    new (&le[i]) GenericValue(re[i], allocator, copyConstStrings);
                data_.f.flags = kArrayFlag;
                data_.a.size = data_.a.capacity = count;
                SetElementsPointer(le);
            }
            break;
        case kStringType:
            if (rhs.data_.f.flags == kConstStringFlag && !copyConstStrings) {
                data_.f.flags = rhs.data_.f.flags;
                data_  = *reinterpret_cast<const Data*>(&rhs.data_);
            }
            else
                SetStringRaw(StringRef(rhs.GetString(), rhs.GetStringLength()), allocator);
            break;
        default:
            data_.f.flags = rhs.data_.f.flags;
            data_  = *reinterpret_cast<const Data*>(&rhs.data_);
            break;
        }
    }

    //! Constructor for boolean value.
    /*! \param b Boolean value
        \note This constructor is limited to \em real boolean values and rejects
            implicitly converted types like arbitrary pointers.  Use an explicit cast
            to \c bool, if you want to construct a boolean JSON value in such cases.
     */
#ifndef RAPIDJSON_DOXYGEN_RUNNING // hide SFINAE from Doxygen
    template <typename T>
    explicit GenericValue(T b, RAPIDJSON_ENABLEIF((internal::IsSame<bool, T>))) RAPIDJSON_NOEXCEPT  // See #472
#else
    explicit GenericValue(bool b) RAPIDJSON_NOEXCEPT
#endif
        : data_() YGG_SCHEMA_INIT {
            // safe-guard against failing SFINAE
            RAPIDJSON_STATIC_ASSERT((internal::IsSame<bool,T>::Value));
            data_.f.flags = b ? kTrueFlag : kFalseFlag;
    }

    //! Constructor for int value.
    explicit GenericValue(int i) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT {
        data_.n.i64 = i;
        data_.f.flags = (i >= 0) ? (kNumberIntFlag | kUintFlag | kUint64Flag) : kNumberIntFlag;
    }

    //! Constructor for unsigned value.
    explicit GenericValue(unsigned u) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT {
        data_.n.u64 = u; 
        data_.f.flags = (u & 0x80000000) ? kNumberUintFlag : (kNumberUintFlag | kIntFlag | kInt64Flag);
    }

    //! Constructor for int64_t value.
    explicit GenericValue(int64_t i64) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT {
        data_.n.i64 = i64;
        data_.f.flags = kNumberInt64Flag;
        if (i64 >= 0) {
            data_.f.flags |= kNumberUint64Flag;
            if (!(static_cast<uint64_t>(i64) & RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x00000000)))
                data_.f.flags |= kUintFlag;
            if (!(static_cast<uint64_t>(i64) & RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
                data_.f.flags |= kIntFlag;
        }
        else if (i64 >= static_cast<int64_t>(RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
            data_.f.flags |= kIntFlag;
    }

    //! Constructor for uint64_t value.
    explicit GenericValue(uint64_t u64) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT {
        data_.n.u64 = u64;
        data_.f.flags = kNumberUint64Flag;
        if (!(u64 & RAPIDJSON_UINT64_C2(0x80000000, 0x00000000)))
            data_.f.flags |= kInt64Flag;
        if (!(u64 & RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x00000000)))
            data_.f.flags |= kUintFlag;
        if (!(u64 & RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
            data_.f.flags |= kIntFlag;
    }

    //! Constructor for double value.
    explicit GenericValue(double d) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT { data_.n.d = d; data_.f.flags = kNumberDoubleFlag; }

    //! Constructor for float value.
    explicit GenericValue(float f) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT { data_.n.d = static_cast<double>(f); data_.f.flags = kNumberDoubleFlag; }

    //! Constructor for constant string (i.e. do not make a copy of string)
    GenericValue(const Ch* s, SizeType length) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT { SetStringRaw(StringRef(s, length)); }

    //! Constructor for constant string (i.e. do not make a copy of string)
    explicit GenericValue(StringRefType s) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT { SetStringRaw(s); }

    //! Constructor for copy-string (i.e. do make a copy of string)
    GenericValue(const Ch* s, SizeType length, Allocator& allocator) : data_() YGG_SCHEMA_INIT { SetStringRaw(StringRef(s, length), allocator); }

    //! Constructor for copy-string (i.e. do make a copy of string)
    GenericValue(const Ch*s, Allocator& allocator) : data_() YGG_SCHEMA_INIT { SetStringRaw(StringRef(s), allocator); }

#if RAPIDJSON_HAS_STDSTRING
    //! Constructor for copy-string from a string object (i.e. do make a copy of string)
    /*! \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
     */
    GenericValue(const std::basic_string<Ch>& s, Allocator& allocator) : data_() YGG_SCHEMA_INIT { SetStringRaw(StringRef(s), allocator); }
#endif

    //! Constructor for Array.
    /*!
        \param a An array obtained by \c GetArray().
        \note \c Array is always pass-by-value.
        \note the source array is moved into this value and the sourec array becomes empty.
    */
    GenericValue(Array a) RAPIDJSON_NOEXCEPT : data_(a.value_.data_) YGG_SCHEMA_INIT {
        a.value_.data_ = Data();
        a.value_.data_.f.flags = kArrayFlag;
#ifdef RAPIDJSON_YGGDRASIL
	RawAssignSchema(a.value_);
#endif // RAPIDJSON_YGGDRASIL
    }

    //! Constructor for Object.
    /*!
        \param o An object obtained by \c GetObject().
        \note \c Object is always pass-by-value.
        \note the source object is moved into this value and the sourec object becomes empty.
    */
    GenericValue(Object o) RAPIDJSON_NOEXCEPT : data_(o.value_.data_) YGG_SCHEMA_INIT {
        o.value_.data_ = Data();
        o.value_.data_.f.flags = kObjectFlag;
#ifdef RAPIDJSON_YGGDRASIL
	RawAssignSchema(o.value_);
#endif // RAPIDJSON_YGGDRASIL
    }

    //! Destructor.
    /*! Need to destruct elements of array, members of object, or copy-string.
    */
    ~GenericValue() {
#ifdef RAPIDJSON_YGGDRASIL
	DestroySchema();
#endif // RAPIDJSON_YGGDRASIL
        // With RAPIDJSON_USE_MEMBERSMAP, the maps need to be destroyed to release
        // their Allocator if it's refcounted (e.g. MemoryPoolAllocator).
        if (Allocator::kNeedFree || (RAPIDJSON_USE_MEMBERSMAP+0 &&
                                     internal::IsRefCounted<Allocator>::Value)) {
            switch(data_.f.flags) {
            case kArrayFlag:
                {
                    GenericValue* e = GetElementsPointer();
                    for (GenericValue* v = e; v != e + data_.a.size; ++v)
                        v->~GenericValue();
                    if (Allocator::kNeedFree) { // Shortcut by Allocator's trait
                        Allocator::Free(e);
                    }
                }
                break;

            case kObjectFlag:
                DoFreeMembers();
                break;

            case kCopyStringFlag:
                if (Allocator::kNeedFree) { // Shortcut by Allocator's trait
                    Allocator::Free(const_cast<Ch*>(GetStringPointer()));
                }
                break;

            default:
                break;  // Do nothing for other types.
            }
        }
    }

    //@}

    //!@name Assignment operators
    //@{

    //! Assignment with move semantics.
    /*! \param rhs Source of the assignment. It will become a null value after assignment.
    */
    GenericValue& operator=(GenericValue& rhs) RAPIDJSON_NOEXCEPT {
        if (RAPIDJSON_LIKELY(this != &rhs)) {
            // Can't destroy "this" before assigning "rhs", otherwise "rhs"
            // could be used after free if it's an sub-Value of "this",
            // hence the temporary danse.
            GenericValue temp;
            temp.RawAssign(rhs);
            this->~GenericValue();
            RawAssign(temp);
        }
        return *this;
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move assignment in C++11
    GenericValue& operator=(GenericValue&& rhs) RAPIDJSON_NOEXCEPT {
        return *this = rhs.Move();
    }
#endif

    //! Assignment of constant string reference (no copy)
    /*! \param str Constant string reference to be assigned
        \note This overload is needed to avoid clashes with the generic primitive type assignment overload below.
        \see GenericStringRef, operator=(T)
    */
    GenericValue& operator=(StringRefType str) RAPIDJSON_NOEXCEPT {
        GenericValue s(str);
        return *this = s;
    }

    //! Assignment with primitive types.
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
        \param value The value to be assigned.

        \note The source type \c T explicitly disallows all pointer types,
            especially (\c const) \ref Ch*.  This helps avoiding implicitly
            referencing character strings with insufficient lifetime, use
            \ref SetString(const Ch*, Allocator&) (for copying) or
            \ref StringRef() (to explicitly mark the pointer as constant) instead.
            All other pointer types would implicitly convert to \c bool,
            use \ref SetBool() instead.
    */
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::IsPointer<T>), (GenericValue&))
    operator=(T value) {
        GenericValue v(value);
        return *this = v;
    }

    //! Deep-copy assignment from Value
    /*! Assigns a \b copy of the Value to the current Value object
        \tparam SourceAllocator Allocator type of \c rhs
        \param rhs Value to copy from (read-only)
        \param allocator Allocator to use for copying
        \param copyConstStrings Force copying of constant strings (e.g. referencing an in-situ buffer)
     */
    template <typename SourceAllocator>
    GenericValue& CopyFrom(const GenericValue<Encoding, SourceAllocator>& rhs, Allocator& allocator, bool copyConstStrings = false) {
        RAPIDJSON_ASSERT(static_cast<void*>(this) != static_cast<void const*>(&rhs));
        this->~GenericValue();
        new (this) GenericValue(rhs, allocator, copyConstStrings);
        return *this;
    }

    //! Exchange the contents of this value with those of other.
    /*!
        \param other Another value.
        \note Constant complexity.
    */
    GenericValue& Swap(GenericValue& other) RAPIDJSON_NOEXCEPT {
        GenericValue temp;
#ifdef RAPIDJSON_YGGDRASIL
	temp.schema_ = NULL;
#endif // RAPIDJSON_YGGDRASIL
        temp.RawAssign(*this);
        RawAssign(other);
        other.RawAssign(temp);
        return *this;
    }

    //! free-standing swap function helper
    /*!
        Helper function to enable support for common swap implementation pattern based on \c std::swap:
        \code
        void swap(MyClass& a, MyClass& b) {
            using std::swap;
            swap(a.value, b.value);
            // ...
        }
        \endcode
        \see Swap()
     */
    friend inline void swap(GenericValue& a, GenericValue& b) RAPIDJSON_NOEXCEPT { a.Swap(b); }

    //! Prepare Value for move semantics
    /*! \return *this */
    GenericValue& Move() RAPIDJSON_NOEXCEPT { return *this; }
    //@}

    //!@name Equal-to and not-equal-to operators
    //@{
    //! Equal-to operator
    /*!
        \note If an object contains duplicated named member, comparing equality with any object is always \c false.
        \note Complexity is quadratic in Object's member number and linear for the rest (number of all values in the subtree and total lengths of all strings).
    */
    template <typename SourceAllocator>
    bool operator==(const GenericValue<Encoding, SourceAllocator>& rhs) const {
        typedef GenericValue<Encoding, SourceAllocator> RhsType;
        if (GetType() != rhs.GetType())
            return false;

        switch (GetType()) {
        case kObjectType: // Warning: O(n^2) inner-loop
            if (data_.o.size != rhs.data_.o.size)
                return false;           
            for (ConstMemberIterator lhsMemberItr = MemberBegin(); lhsMemberItr != MemberEnd(); ++lhsMemberItr) {
                typename RhsType::ConstMemberIterator rhsMemberItr = rhs.FindMember(lhsMemberItr->name);
                if (rhsMemberItr == rhs.MemberEnd() || lhsMemberItr->value != rhsMemberItr->value)
                    return false;
            }
            return true;
            
        case kArrayType:
            if (data_.a.size != rhs.data_.a.size)
                return false;
            for (SizeType i = 0; i < data_.a.size; i++)
                if ((*this)[i] != rhs[i])
                    return false;
            return true;

        case kStringType:
#ifdef RAPIDJSON_YGGDRASIL
	    if (HasUnits() && rhs.HasUnits() && (GetUnits() != rhs.GetUnits()))
	        return false;
#endif // RAPIDJSON_YGGDRASIL
            return StringEqual(rhs);

        case kNumberType:
            if (IsDouble() || rhs.IsDouble()) {
                double a = GetDouble();     // May convert from integer to double.
                double b = rhs.GetDouble(); // Ditto
                return a >= b && a <= b;    // Prevent -Wfloat-equal
            }
            else
                return data_.n.u64 == rhs.data_.n.u64;

        default:
            return true;
        }
    }

    //! Equal-to operator with const C-string pointer
    bool operator==(const Ch* rhs) const { return *this == GenericValue(StringRef(rhs)); }

#if RAPIDJSON_HAS_STDSTRING
    //! Equal-to operator with string object
    /*! \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
     */
    bool operator==(const std::basic_string<Ch>& rhs) const { return *this == GenericValue(StringRef(rhs)); }
#endif

    //! Equal-to operator with primitive types
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t, \c double, \c true, \c false
    */
    template <typename T> RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>,internal::IsGenericValue<T> >), (bool)) operator==(const T& rhs) const { return *this == GenericValue(rhs); }

    //! Not-equal-to operator
    /*! \return !(*this == rhs)
     */
    template <typename SourceAllocator>
    bool operator!=(const GenericValue<Encoding, SourceAllocator>& rhs) const { return !(*this == rhs); }

    //! Not-equal-to operator with const C-string pointer
    bool operator!=(const Ch* rhs) const { return !(*this == rhs); }

    //! Not-equal-to operator with arbitrary types
    /*! \return !(*this == rhs)
     */
    template <typename T> RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator!=(const T& rhs) const { return !(*this == rhs); }

#ifndef __cpp_impl_three_way_comparison
    //! Equal-to operator with arbitrary types (symmetric version)
    /*! \return (rhs == lhs)
     */
    template <typename T> friend RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator==(const T& lhs, const GenericValue& rhs) { return rhs == lhs; }

    //! Not-Equal-to operator with arbitrary types (symmetric version)
    /*! \return !(rhs == lhs)
     */
    template <typename T> friend RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator!=(const T& lhs, const GenericValue& rhs) { return !(rhs == lhs); }
    //@}
#endif

    //!@name Type
    //@{

    Type GetType()  const { return static_cast<Type>(data_.f.flags & kTypeMask); }
    bool IsNull()   const { return data_.f.flags == kNullFlag; }
    bool IsFalse()  const { return data_.f.flags == kFalseFlag; }
    bool IsTrue()   const { return data_.f.flags == kTrueFlag; }
    bool IsBool()   const { return (data_.f.flags & kBoolFlag) != 0; }
    bool IsObject() const { return data_.f.flags == kObjectFlag; }
    bool IsArray()  const { return data_.f.flags == kArrayFlag; }
    bool IsNumber() const { return (data_.f.flags & kNumberFlag) != 0; }
    bool IsInt()    const { return (data_.f.flags & kIntFlag) != 0; }
    bool IsUint()   const { return (data_.f.flags & kUintFlag) != 0; }
    bool IsInt64()  const { return (data_.f.flags & kInt64Flag) != 0; }
    bool IsUint64() const { return (data_.f.flags & kUint64Flag) != 0; }
    bool IsDouble() const { return (data_.f.flags & kDoubleFlag) != 0; }
    bool IsString() const { return (data_.f.flags & kStringFlag) != 0; }

    // Checks whether a number can be losslessly converted to a double.
    bool IsLosslessDouble() const {
        if (!IsNumber()) return false;
        if (IsUint64()) {
            uint64_t u = GetUint64();
            volatile double d = static_cast<double>(u);
            return (d >= 0.0)
                && (d < static_cast<double>((std::numeric_limits<uint64_t>::max)()))
                && (u == static_cast<uint64_t>(d));
        }
        if (IsInt64()) {
            int64_t i = GetInt64();
            volatile double d = static_cast<double>(i);
            return (d >= static_cast<double>((std::numeric_limits<int64_t>::min)()))
                && (d < static_cast<double>((std::numeric_limits<int64_t>::max)()))
                && (i == static_cast<int64_t>(d));
        }
        return true; // double, int, uint are always lossless
    }

    // Checks whether a number is a float (possible lossy).
    bool IsFloat() const  {
        if ((data_.f.flags & kDoubleFlag) == 0)
            return false;
        double d = GetDouble();
        return d >= -3.4028234e38 && d <= 3.4028234e38;
    }
    // Checks whether a number can be losslessly converted to a float.
    bool IsLosslessFloat() const {
        if (!IsNumber()) return false;
        double a = GetDouble();
        if (a < static_cast<double>(-(std::numeric_limits<float>::max)())
                || a > static_cast<double>((std::numeric_limits<float>::max)()))
            return false;
        double b = static_cast<double>(static_cast<float>(a));
        return a >= b && a <= b;    // Prevent -Wfloat-equal
    }

    //@}

    //!@name Null
    //@{

    GenericValue& SetNull() { this->~GenericValue(); new (this) GenericValue(); return *this; }

    //@}

    //!@name Bool
    //@{

    bool GetBool() const { RAPIDJSON_ASSERT(IsBool()); return data_.f.flags == kTrueFlag; }
    //!< Set boolean value
    /*! \post IsBool() == true */
    GenericValue& SetBool(bool b) { this->~GenericValue(); new (this) GenericValue(b); return *this; }

    //@}

    //!@name Object
    //@{

    //! Set this value as an empty object.
    /*! \post IsObject() == true */
    GenericValue& SetObject() { this->~GenericValue(); new (this) GenericValue(kObjectType); return *this; }

    //! Get the number of members in the object.
    SizeType MemberCount() const { RAPIDJSON_ASSERT(IsObject()); return data_.o.size; }

    //! Get the capacity of object.
    SizeType MemberCapacity() const { RAPIDJSON_ASSERT(IsObject()); return data_.o.capacity; }

    //! Check whether the object is empty.
    bool ObjectEmpty() const { RAPIDJSON_ASSERT(IsObject()); return data_.o.size == 0; }

    //! Get a value from an object associated with the name.
    /*! \pre IsObject() == true
        \tparam T Either \c Ch or \c const \c Ch (template used for disambiguation with \ref operator[](SizeType))
        \note In version 0.1x, if the member is not found, this function returns a null value. This makes issue 7.
        Since 0.2, if the name is not correct, it will assert.
        If user is unsure whether a member exists, user should use HasMember() first.
        A better approach is to use FindMember().
        \note Linear time complexity.
    */
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::NotExpr<internal::IsSame<typename internal::RemoveConst<T>::Type, Ch> >),(GenericValue&)) operator[](T* name) {
        GenericValue n(StringRef(name));
        return (*this)[n];
    }
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::NotExpr<internal::IsSame<typename internal::RemoveConst<T>::Type, Ch> >),(const GenericValue&)) operator[](T* name) const { return const_cast<GenericValue&>(*this)[name]; }

    //! Get a value from an object associated with the name.
    /*! \pre IsObject() == true
        \tparam SourceAllocator Allocator of the \c name value

        \note Compared to \ref operator[](T*), this version is faster because it does not need a StrLen().
        And it can also handle strings with embedded null characters.

        \note Linear time complexity.
    */
    template <typename SourceAllocator>
    GenericValue& operator[](const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator member = FindMember(name);
        if (member != MemberEnd())
            return member->value;
        else {
            RAPIDJSON_ASSERT(false);    // see above note

#if RAPIDJSON_HAS_CXX11
            // Use thread-local storage to prevent races between threads.
            // Use static buffer and placement-new to prevent destruction, with
            // alignas() to ensure proper alignment.
            alignas(GenericValue) thread_local static char buffer[sizeof(GenericValue)];
            return *new (buffer) GenericValue();
#elif defined(_MSC_VER) && _MSC_VER < 1900
            // There's no way to solve both thread locality and proper alignment
            // simultaneously.
            __declspec(thread) static char buffer[sizeof(GenericValue)];
            return *new (buffer) GenericValue();
// #elif defined(__GNUC__) || defined(__clang__)
//             // This will generate -Wexit-time-destructors in clang, but that's
//             // better than having under-alignment.
//             __thread static GenericValue buffer;
//             return buffer;
#else
            // Don't know what compiler this is, so don't know how to ensure
            // thread-locality.
            static GenericValue buffer;
            return buffer;
#endif
        }
    }
    template <typename SourceAllocator>
    const GenericValue& operator[](const GenericValue<Encoding, SourceAllocator>& name) const { return const_cast<GenericValue&>(*this)[name]; }

#if RAPIDJSON_HAS_STDSTRING
    //! Get a value from an object associated with name (string object).
    GenericValue& operator[](const std::basic_string<Ch>& name) { return (*this)[GenericValue(StringRef(name))]; }
    const GenericValue& operator[](const std::basic_string<Ch>& name) const { return (*this)[GenericValue(StringRef(name))]; }
#endif

    //! Const member iterator
    /*! \pre IsObject() == true */
    ConstMemberIterator MemberBegin() const { RAPIDJSON_ASSERT(IsObject()); return ConstMemberIterator(GetMembersPointer()); }
    //! Const \em past-the-end member iterator
    /*! \pre IsObject() == true */
    ConstMemberIterator MemberEnd() const   { RAPIDJSON_ASSERT(IsObject()); return ConstMemberIterator(GetMembersPointer() + data_.o.size); }
    //! Member iterator
    /*! \pre IsObject() == true */
    MemberIterator MemberBegin()            { RAPIDJSON_ASSERT(IsObject()); return MemberIterator(GetMembersPointer()); }
    //! \em Past-the-end member iterator
    /*! \pre IsObject() == true */
    MemberIterator MemberEnd()              { RAPIDJSON_ASSERT(IsObject()); return MemberIterator(GetMembersPointer() + data_.o.size); }

    //! Request the object to have enough capacity to store members.
    /*! \param newCapacity  The capacity that the object at least need to have.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \note Linear time complexity.
    */
    GenericValue& MemberReserve(SizeType newCapacity, Allocator &allocator) {
        RAPIDJSON_ASSERT(IsObject());
        DoReserveMembers(newCapacity, allocator);
        return *this;
    }

    //! Check whether a member exists in the object.
    /*!
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Whether a member with that name exists.
        \note It is better to use FindMember() directly if you need the obtain the value as well.
        \note Linear time complexity.
    */
    bool HasMember(const Ch* name) const { return FindMember(name) != MemberEnd(); }

#if RAPIDJSON_HAS_STDSTRING
    //! Check whether a member exists in the object with string object.
    /*!
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Whether a member with that name exists.
        \note It is better to use FindMember() directly if you need the obtain the value as well.
        \note Linear time complexity.
    */
    bool HasMember(const std::basic_string<Ch>& name) const { return FindMember(name) != MemberEnd(); }
#endif

    //! Check whether a member exists in the object with GenericValue name.
    /*!
        This version is faster because it does not need a StrLen(). It can also handle string with null character.
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Whether a member with that name exists.
        \note It is better to use FindMember() directly if you need the obtain the value as well.
        \note Linear time complexity.
    */
    template <typename SourceAllocator>
    bool HasMember(const GenericValue<Encoding, SourceAllocator>& name) const { return FindMember(name) != MemberEnd(); }

    //! Find member by name.
    /*!
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Iterator to member, if it exists.
            Otherwise returns \ref MemberEnd().

        \note Earlier versions of Rapidjson returned a \c NULL pointer, in case
            the requested member doesn't exist. For consistency with e.g.
            \c std::map, this has been changed to MemberEnd() now.
        \note Linear time complexity.
    */
    MemberIterator FindMember(const Ch* name) {
        GenericValue n(StringRef(name));
        return FindMember(n);
    }

    ConstMemberIterator FindMember(const Ch* name) const { return const_cast<GenericValue&>(*this).FindMember(name); }

    //! Find member by name.
    /*!
        This version is faster because it does not need a StrLen(). It can also handle string with null character.
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Iterator to member, if it exists.
            Otherwise returns \ref MemberEnd().

        \note Earlier versions of Rapidjson returned a \c NULL pointer, in case
            the requested member doesn't exist. For consistency with e.g.
            \c std::map, this has been changed to MemberEnd() now.
        \note Linear time complexity.
    */
    template <typename SourceAllocator>
    MemberIterator FindMember(const GenericValue<Encoding, SourceAllocator>& name) {
        RAPIDJSON_ASSERT(IsObject());
        RAPIDJSON_ASSERT(name.IsString());
        return DoFindMember(name);
    }
    template <typename SourceAllocator> ConstMemberIterator FindMember(const GenericValue<Encoding, SourceAllocator>& name) const { return const_cast<GenericValue&>(*this).FindMember(name); }

#if RAPIDJSON_HAS_STDSTRING
    //! Find member by string object name.
    /*!
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Iterator to member, if it exists.
            Otherwise returns \ref MemberEnd().
    */
    MemberIterator FindMember(const std::basic_string<Ch>& name) { return FindMember(GenericValue(StringRef(name))); }
    ConstMemberIterator FindMember(const std::basic_string<Ch>& name) const { return FindMember(GenericValue(StringRef(name))); }
#endif

    //! Add a member (name-value pair) to the object.
    /*! \param name A string value as name of member.
        \param value Value of any type.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \note The ownership of \c name and \c value will be transferred to this object on success.
        \pre  IsObject() && name.IsString()
        \post name.IsNull() && value.IsNull()
        \note Amortized Constant time complexity.
    */
    GenericValue& AddMember(GenericValue& name, GenericValue& value, Allocator& allocator) {
        RAPIDJSON_ASSERT(IsObject());
        RAPIDJSON_ASSERT(name.IsString());
        DoAddMember(name, value, allocator);
        return *this;
    }

#ifdef RAPIDJSON_YGGDRASIL
  GenericValue& AddMember(const GenericValue& name, GenericValue& value, Allocator& allocator) {
    RAPIDJSON_ASSERT(name.IsString());
    GenericValue name_cpy(name, allocator);  // StringRef(name.GetString()));
    return AddMember(name_cpy, value, allocator);
  }
#endif // RAPIDJSON_YGGDRASIL

    //! Add a constant string value as member (name-value pair) to the object.
    /*! \param name A string value as name of member.
        \param value constant string reference as value of member.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \pre  IsObject()
        \note This overload is needed to avoid clashes with the generic primitive type AddMember(GenericValue&,T,Allocator&) overload below.
        \note Amortized Constant time complexity.
    */
    GenericValue& AddMember(GenericValue& name, StringRefType value, Allocator& allocator) {
        GenericValue v(value);
        return AddMember(name, v, allocator);
    }

#if RAPIDJSON_HAS_STDSTRING
    //! Add a string object as member (name-value pair) to the object.
    /*! \param name A string value as name of member.
        \param value constant string reference as value of member.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \pre  IsObject()
        \note This overload is needed to avoid clashes with the generic primitive type AddMember(GenericValue&,T,Allocator&) overload below.
        \note Amortized Constant time complexity.
    */
    GenericValue& AddMember(GenericValue& name, std::basic_string<Ch>& value, Allocator& allocator) {
        GenericValue v(value, allocator);
        return AddMember(name, v, allocator);
    }
#endif

    //! Add any primitive value as member (name-value pair) to the object.
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
        \param name A string value as name of member.
        \param value Value of primitive type \c T as value of member
        \param allocator Allocator for reallocating memory. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \pre  IsObject()

        \note The source type \c T explicitly disallows all pointer types,
            especially (\c const) \ref Ch*.  This helps avoiding implicitly
            referencing character strings with insufficient lifetime, use
            \ref AddMember(StringRefType, GenericValue&, Allocator&) or \ref
            AddMember(StringRefType, StringRefType, Allocator&).
            All other pointer types would implicitly convert to \c bool,
            use an explicit cast instead, if needed.
        \note Amortized Constant time complexity.
    */
#ifdef RAPIDJSON_YGGDRASIL
    template <typename T>
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
    RAPIDJSON_ENABLEIF_RETURN((internal::OrExpr<internal::IsSame<T,uint8_t>,
			       internal::OrExpr<internal::IsSame<T,uint16_t>,
			       internal::OrExpr<internal::IsSame<T,int8_t>,
			       internal::OrExpr<internal::IsSame<T,int16_t>,
			       internal::OrExpr<internal::IsSame<T,long double>,
			       YGGDRASIL_IS_COMPLEX_TYPE(T)> > > > >),
			       (GenericValue&))
#else // YGGDRASIL_LONG_DOUBLE_AVAILABLE
    RAPIDJSON_ENABLEIF_RETURN((internal::OrExpr<internal::IsSame<T,uint8_t>,
			       internal::OrExpr<internal::IsSame<T,uint16_t>,
			       internal::OrExpr<internal::IsSame<T,int8_t>,
			       internal::OrExpr<internal::IsSame<T,int16_t>,
			       YGGDRASIL_IS_COMPLEX_TYPE(T)> > > >),
			       (GenericValue&))
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
    AddMember(GenericValue& name, T value, Allocator& allocator) {
      GenericValue v(value, allocator);
      return AddMember(name, v, allocator);
    }
    template <typename T>
    GenericValue& AddMember(GenericValue& name,
			    units::GenericQuantity<T, EncodingType> value,
			    Allocator& allocator) {
      GenericValue v(value);
      return AddMember(name, v, allocator);
    }
    template <typename T>
    GenericValue& AddMember(GenericValue& name,
			    units::GenericQuantity<T, EncodingType>* value,
			    SizeType nelements, Allocator& allocator) {
      GenericValue v(value, nelements, allocator);
      return AddMember(name, v, allocator);
    }
    template <typename T>
    GenericValue& AddMember(GenericValue& name,
			    units::GenericQuantity<T, EncodingType>* value,
			    SizeType shape[], SizeType ndim,
			    Allocator& allocator) {
      GenericValue v(value, shape, ndim, allocator);
      return AddMember(name, v, allocator);
    }
  
    template <typename T>
    GenericValue& AddMember(GenericValue& name, T value, const Ch* units_str, SizeType units_len, Allocator& allocator) {
      GenericValue v(value, units_str, units_len, allocator);
      return AddMember(name, v, allocator);
    }
    template <typename T>
    GenericValue& AddMember(GenericValue& name, T* value, SizeType nelements, const Ch* units_str, SizeType units_len, Allocator& allocator) {
      GenericValue v(value, nelements, units_str, units_len, allocator);
      return AddMember(name, v, allocator);
    }
    template <typename T>
    GenericValue& AddMember(GenericValue& name, T* value, SizeType nelements, Allocator& allocator) {
      GenericValue v(value, nelements, allocator);
      return AddMember(name, v, allocator);
    }
    template <typename T>
    GenericValue& AddMember(GenericValue& name, T* value, SizeType shape[], SizeType ndim, const Ch* units_str, SizeType units_len, Allocator& allocator) {
      GenericValue v(value, shape, ndim, units_str, units_len, allocator);
      return AddMember(name, v, allocator);
    }
    template <typename T>
    GenericValue& AddMember(GenericValue& name, T* value, SizeType shape[], SizeType ndim, Allocator& allocator) {
      GenericValue v(value, shape, ndim, allocator);
      return AddMember(name, v, allocator);
    }
			       
  
    template <typename T>
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>,
				internal::OrExpr<internal::IsSame<T,uint8_t>,
				internal::OrExpr<internal::IsSame<T,uint16_t>,
				internal::OrExpr<internal::IsSame<T,int8_t>,
				internal::OrExpr<internal::IsSame<T,int16_t>,
				internal::OrExpr<internal::IsSame<T,long double>,
				internal::OrExpr<YGGDRASIL_IS_COMPLEX_TYPE(T),
				internal::IsGenericValue<T> > > > > > > >),
			       (GenericValue&))
#else // YGGDRASIL_LONG_DOUBLE_AVAILABLE
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>,
				internal::OrExpr<internal::IsSame<T,uint8_t>,
				internal::OrExpr<internal::IsSame<T,uint16_t>,
				internal::OrExpr<internal::IsSame<T,int8_t>,
				internal::OrExpr<internal::IsSame<T,int16_t>,
				internal::OrExpr<YGGDRASIL_IS_COMPLEX_TYPE(T),
				internal::IsGenericValue<T> > > > > > >),
			       (GenericValue&))
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
#else // RAPIDJSON_YGGDRASIL
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericValue&))
#endif // RAPIDJSON_YGGDRASIL
    AddMember(GenericValue& name, T value, Allocator& allocator) {
        GenericValue v(value);
        return AddMember(name, v, allocator);
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericValue& AddMember(GenericValue&& name, GenericValue&& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(GenericValue&& name, GenericValue& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(GenericValue& name, GenericValue&& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(StringRefType name, GenericValue&& value, Allocator& allocator) {
        GenericValue n(name);
        return AddMember(n, value, allocator);
    }
#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS


    //! Add a member (name-value pair) to the object.
    /*! \param name A constant string reference as name of member.
        \param value Value of any type.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \note The ownership of \c value will be transferred to this object on success.
        \pre  IsObject()
        \post value.IsNull()
        \note Amortized Constant time complexity.
    */
    GenericValue& AddMember(StringRefType name, GenericValue& value, Allocator& allocator) {
        GenericValue n(name);
        return AddMember(n, value, allocator);
    }

    //! Add a constant string value as member (name-value pair) to the object.
    /*! \param name A constant string reference as name of member.
        \param value constant string reference as value of member.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \pre  IsObject()
        \note This overload is needed to avoid clashes with the generic primitive type AddMember(StringRefType,T,Allocator&) overload below.
        \note Amortized Constant time complexity.
    */
    GenericValue& AddMember(StringRefType name, StringRefType value, Allocator& allocator) {
        GenericValue v(value);
        return AddMember(name, v, allocator);
    }

    //! Add any primitive value as member (name-value pair) to the object.
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
        \param name A constant string reference as name of member.
        \param value Value of primitive type \c T as value of member
        \param allocator Allocator for reallocating memory. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \pre  IsObject()

        \note The source type \c T explicitly disallows all pointer types,
            especially (\c const) \ref Ch*.  This helps avoiding implicitly
            referencing character strings with insufficient lifetime, use
            \ref AddMember(StringRefType, GenericValue&, Allocator&) or \ref
            AddMember(StringRefType, StringRefType, Allocator&).
            All other pointer types would implicitly convert to \c bool,
            use an explicit cast instead, if needed.
        \note Amortized Constant time complexity.
    */
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericValue&))
    AddMember(StringRefType name, T value, Allocator& allocator) {
        GenericValue n(name);
        return AddMember(n, value, allocator);
    }

    //! Remove all members in the object.
    /*! This function do not deallocate memory in the object, i.e. the capacity is unchanged.
        \note Linear time complexity.
    */
    void RemoveAllMembers() {
        RAPIDJSON_ASSERT(IsObject()); 
        DoClearMembers();
    }

    //! Remove a member in object by its name.
    /*! \param name Name of member to be removed.
        \return Whether the member existed.
        \note This function may reorder the object members. Use \ref
            EraseMember(ConstMemberIterator) if you need to preserve the
            relative order of the remaining members.
        \note Linear time complexity.
    */
    bool RemoveMember(const Ch* name) {
        GenericValue n(StringRef(name));
        return RemoveMember(n);
    }

#if RAPIDJSON_HAS_STDSTRING
    bool RemoveMember(const std::basic_string<Ch>& name) { return RemoveMember(GenericValue(StringRef(name))); }
#endif

    template <typename SourceAllocator>
    bool RemoveMember(const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator m = FindMember(name);
        if (m != MemberEnd()) {
            RemoveMember(m);
            return true;
        }
        else
            return false;
    }

    //! Remove a member in object by iterator.
    /*! \param m member iterator (obtained by FindMember() or MemberBegin()).
        \return the new iterator after removal.
        \note This function may reorder the object members. Use \ref
            EraseMember(ConstMemberIterator) if you need to preserve the
            relative order of the remaining members.
        \note Constant time complexity.
    */
    MemberIterator RemoveMember(MemberIterator m) {
        RAPIDJSON_ASSERT(IsObject());
        RAPIDJSON_ASSERT(data_.o.size > 0);
        RAPIDJSON_ASSERT(GetMembersPointer() != 0);
        RAPIDJSON_ASSERT(m >= MemberBegin() && m < MemberEnd());
        return DoRemoveMember(m);
    }

    //! Remove a member from an object by iterator.
    /*! \param pos iterator to the member to remove
        \pre IsObject() == true && \ref MemberBegin() <= \c pos < \ref MemberEnd()
        \return Iterator following the removed element.
            If the iterator \c pos refers to the last element, the \ref MemberEnd() iterator is returned.
        \note This function preserves the relative order of the remaining object
            members. If you do not need this, use the more efficient \ref RemoveMember(MemberIterator).
        \note Linear time complexity.
    */
    MemberIterator EraseMember(ConstMemberIterator pos) {
        return EraseMember(pos, pos +1);
    }

    //! Remove members in the range [first, last) from an object.
    /*! \param first iterator to the first member to remove
        \param last  iterator following the last member to remove
        \pre IsObject() == true && \ref MemberBegin() <= \c first <= \c last <= \ref MemberEnd()
        \return Iterator following the last removed element.
        \note This function preserves the relative order of the remaining object
            members.
        \note Linear time complexity.
    */
    MemberIterator EraseMember(ConstMemberIterator first, ConstMemberIterator last) {
        RAPIDJSON_ASSERT(IsObject());
        RAPIDJSON_ASSERT(data_.o.size > 0);
        RAPIDJSON_ASSERT(GetMembersPointer() != 0);
        RAPIDJSON_ASSERT(first >= MemberBegin());
        RAPIDJSON_ASSERT(first <= last);
        RAPIDJSON_ASSERT(last <= MemberEnd());
        return DoEraseMembers(first, last);
    }

    //! Erase a member in object by its name.
    /*! \param name Name of member to be removed.
        \return Whether the member existed.
        \note Linear time complexity.
    */
    bool EraseMember(const Ch* name) {
        GenericValue n(StringRef(name));
        return EraseMember(n);
    }

#if RAPIDJSON_HAS_STDSTRING
    bool EraseMember(const std::basic_string<Ch>& name) { return EraseMember(GenericValue(StringRef(name))); }
#endif

    template <typename SourceAllocator>
    bool EraseMember(const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator m = FindMember(name);
        if (m != MemberEnd()) {
            EraseMember(m);
            return true;
        }
        else
            return false;
    }

    Object GetObject() { RAPIDJSON_ASSERT(IsObject()); return Object(*this); }
    Object GetObj() { RAPIDJSON_ASSERT(IsObject()); return Object(*this); }
    ConstObject GetObject() const { RAPIDJSON_ASSERT(IsObject()); return ConstObject(*this); }
    ConstObject GetObj() const { RAPIDJSON_ASSERT(IsObject()); return ConstObject(*this); }

    //@}

    //!@name Array
    //@{

    //! Set this value as an empty array.
    /*! \post IsArray == true */
    GenericValue& SetArray() { this->~GenericValue(); new (this) GenericValue(kArrayType); return *this; }

    //! Get the number of elements in array.
    SizeType Size() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.size; }

    //! Get the capacity of array.
    SizeType Capacity() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.capacity; }

    //! Check whether the array is empty.
    bool Empty() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.size == 0; }

    //! Remove all elements in the array.
    /*! This function do not deallocate memory in the array, i.e. the capacity is unchanged.
        \note Linear time complexity.
    */
    void Clear() {
        RAPIDJSON_ASSERT(IsArray()); 
        GenericValue* e = GetElementsPointer();
        for (GenericValue* v = e; v != e + data_.a.size; ++v)
            v->~GenericValue();
        data_.a.size = 0;
    }

    //! Get an element from array by index.
    /*! \pre IsArray() == true
        \param index Zero-based index of element.
        \see operator[](T*)
    */
    GenericValue& operator[](SizeType index) {
        RAPIDJSON_ASSERT(IsArray());
        RAPIDJSON_ASSERT(index < data_.a.size);
        return GetElementsPointer()[index];
    }
    const GenericValue& operator[](SizeType index) const { return const_cast<GenericValue&>(*this)[index]; }

    //! Element iterator
    /*! \pre IsArray() == true */
    ValueIterator Begin() { RAPIDJSON_ASSERT(IsArray()); return GetElementsPointer(); }
    //! \em Past-the-end element iterator
    /*! \pre IsArray() == true */
    ValueIterator End() { RAPIDJSON_ASSERT(IsArray()); return GetElementsPointer() + data_.a.size; }
    //! Constant element iterator
    /*! \pre IsArray() == true */
    ConstValueIterator Begin() const { return const_cast<GenericValue&>(*this).Begin(); }
    //! Constant \em past-the-end element iterator
    /*! \pre IsArray() == true */
    ConstValueIterator End() const { return const_cast<GenericValue&>(*this).End(); }

    //! Request the array to have enough capacity to store elements.
    /*! \param newCapacity  The capacity that the array at least need to have.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \note Linear time complexity.
    */
    GenericValue& Reserve(SizeType newCapacity, Allocator &allocator) {
        RAPIDJSON_ASSERT(IsArray());
        if (newCapacity > data_.a.capacity) {
            SetElementsPointer(reinterpret_cast<GenericValue*>(allocator.Realloc(GetElementsPointer(), data_.a.capacity * sizeof(GenericValue), newCapacity * sizeof(GenericValue))));
#ifdef RAPIDJSON_YGGDRASIL
	    for (SizeType i = data_.a.capacity; i < newCapacity; i++) {
	      GetElementsPointer()[i].schema_ = NULL;
	    }
#endif // RAPIDJSON_YGGDRASIL
            data_.a.capacity = newCapacity;
        }
        return *this;
    }

    //! Append a GenericValue at the end of the array.
    /*! \param value        Value to be appended.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \pre IsArray() == true
        \post value.IsNull() == true
        \return The value itself for fluent API.
        \note The ownership of \c value will be transferred to this array on success.
        \note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.
        \note Amortized constant time complexity.
    */
    GenericValue& PushBack(GenericValue& value, Allocator& allocator) {
        RAPIDJSON_ASSERT(IsArray());
        if (data_.a.size >= data_.a.capacity)
            Reserve(data_.a.capacity == 0 ? kDefaultArrayCapacity : (data_.a.capacity + (data_.a.capacity + 1) / 2), allocator);
        GetElementsPointer()[data_.a.size++].RawAssign(value);
        return *this;
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericValue& PushBack(GenericValue&& value, Allocator& allocator) {
        return PushBack(value, allocator);
    }
#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS

    //! Append a constant string reference at the end of the array.
    /*! \param value        Constant string reference to be appended.
        \param allocator    Allocator for reallocating memory. It must be the same one used previously. Commonly use GenericDocument::GetAllocator().
        \pre IsArray() == true
        \return The value itself for fluent API.
        \note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.
        \note Amortized constant time complexity.
        \see GenericStringRef
    */
    GenericValue& PushBack(StringRefType value, Allocator& allocator) {
        return (*this).template PushBack<StringRefType>(value, allocator);
    }

    //! Append a primitive value at the end of the array.
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
        \param value Value of primitive type T to be appended.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \pre IsArray() == true
        \return The value itself for fluent API.
        \note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.

        \note The source type \c T explicitly disallows all pointer types,
            especially (\c const) \ref Ch*.  This helps avoiding implicitly
            referencing character strings with insufficient lifetime, use
            \ref PushBack(GenericValue&, Allocator&) or \ref
            PushBack(StringRefType, Allocator&).
            All other pointer types would implicitly convert to \c bool,
            use an explicit cast instead, if needed.
        \note Amortized constant time complexity.
    */
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericValue&))
    PushBack(T value, Allocator& allocator) {
        GenericValue v(value);
        return PushBack(v, allocator);
    }

    //! Remove the last element in the array.
    /*!
        \note Constant time complexity.
    */
    GenericValue& PopBack() {
        RAPIDJSON_ASSERT(IsArray());
        RAPIDJSON_ASSERT(!Empty());
        GetElementsPointer()[--data_.a.size].~GenericValue();
        return *this;
    }

    //! Remove an element of array by iterator.
    /*!
        \param pos iterator to the element to remove
        \pre IsArray() == true && \ref Begin() <= \c pos < \ref End()
        \return Iterator following the removed element. If the iterator pos refers to the last element, the End() iterator is returned.
        \note Linear time complexity.
    */
    ValueIterator Erase(ConstValueIterator pos) {
        return Erase(pos, pos + 1);
    }

    //! Remove elements in the range [first, last) of the array.
    /*!
        \param first iterator to the first element to remove
        \param last  iterator following the last element to remove
        \pre IsArray() == true && \ref Begin() <= \c first <= \c last <= \ref End()
        \return Iterator following the last removed element.
        \note Linear time complexity.
    */
    ValueIterator Erase(ConstValueIterator first, ConstValueIterator last) {
        RAPIDJSON_ASSERT(IsArray());
        RAPIDJSON_ASSERT(data_.a.size > 0);
        RAPIDJSON_ASSERT(GetElementsPointer() != 0);
        RAPIDJSON_ASSERT(first >= Begin());
        RAPIDJSON_ASSERT(first <= last);
        RAPIDJSON_ASSERT(last <= End());
        ValueIterator pos = Begin() + (first - Begin());
        for (ValueIterator itr = pos; itr != last; ++itr)
            itr->~GenericValue();
        std::memmove(static_cast<void*>(pos), last, static_cast<size_t>(End() - last) * sizeof(GenericValue));
        data_.a.size -= static_cast<SizeType>(last - first);
        return pos;
    }

    Array GetArray() { RAPIDJSON_ASSERT(IsArray()); return Array(*this); }
    ConstArray GetArray() const { RAPIDJSON_ASSERT(IsArray()); return ConstArray(*this); }

    //@}

    //!@name Number
    //@{

#ifdef RAPIDJSON_YGGDRASIL
    //! Locate an array element.
    ConstValueIterator Index(const ValueType& x) const {
      RAPIDJSON_ASSERT(IsArray());
      if (IsArray()) {
	for (ConstValueIterator it = Begin(); it != End(); ++it) {
	  if (x == *it)
	    return it;
	}
      }
      return End();
    }
    //! Check if an array contains an element.
    bool Contains(const ValueType& x) const {
      RAPIDJSON_ASSERT(IsArray());
      return (Index(x) != End());
    }
    int GetInt() const          {
      if (IsScalar()) {
	if (GetSubType() == GetIntSubTypeString())
	  return GetScalar<int>();
	else if (GetSubType() == GetUintSubTypeString())
	  return static_cast<int>(GetScalar<unsigned>());
      }
      RAPIDJSON_ASSERT(data_.f.flags & kIntFlag);   return data_.n.i.i;  }
    unsigned GetUint() const    {
      if (IsScalar()) {
	if (GetSubType() == GetUintSubTypeString())
	  return GetScalar<unsigned>();
	else if (GetSubType() == GetIntSubTypeString())
	  return static_cast<unsigned>(GetScalar<int>());
      }
      RAPIDJSON_ASSERT(data_.f.flags & kUintFlag);  return data_.n.u.u;  }
    int64_t GetInt64() const    {
      if (IsScalar()) {
	if (GetSubType() == GetIntSubTypeString())
	  return GetScalar<int64_t>();
	else if (GetSubType() == GetUintSubTypeString())
	  return static_cast<int64_t>(GetScalar<uint64_t>());
      }
      RAPIDJSON_ASSERT(data_.f.flags & kInt64Flag); return data_.n.i64;  }
    uint64_t GetUint64() const  {
      if (IsScalar()) {
	if (GetSubType() == GetUintSubTypeString())
	  return GetScalar<uint64_t>();
	else if (GetSubType() == GetIntSubTypeString())
	  return static_cast<uint64_t>(GetScalar<int64_t>());
      }
      RAPIDJSON_ASSERT(data_.f.flags & kUint64Flag); return data_.n.u64; }
  
    double GetDouble() const {
      if (IsScalar() && (GetSubType() == GetFloatSubTypeString()))
	return GetScalar<double>();
      RAPIDJSON_ASSERT(IsNumber());
      if ((data_.f.flags & kDoubleFlag) != 0)                return data_.n.d;   // exact type, no conversion.
      if ((data_.f.flags & kIntFlag) != 0)                   return data_.n.i.i; // int -> double
      if ((data_.f.flags & kUintFlag) != 0)                  return data_.n.u.u; // unsigned -> double
      if ((data_.f.flags & kInt64Flag) != 0)                 return static_cast<double>(data_.n.i64); // int64_t -> double (may lose precision)
      RAPIDJSON_ASSERT((data_.f.flags & kUint64Flag) != 0);  return static_cast<double>(data_.n.u64); // uint64_t -> double (may lose precision)
    }
  
#else // RAPIDJSON_YGGDRASIL
    int GetInt() const          { RAPIDJSON_ASSERT(data_.f.flags & kIntFlag);   return data_.n.i.i;   }
    unsigned GetUint() const    { RAPIDJSON_ASSERT(data_.f.flags & kUintFlag);  return data_.n.u.u;   }
    int64_t GetInt64() const    { RAPIDJSON_ASSERT(data_.f.flags & kInt64Flag); return data_.n.i64; }
    uint64_t GetUint64() const  { RAPIDJSON_ASSERT(data_.f.flags & kUint64Flag); return data_.n.u64; }

    //! Get the value as double type.
    /*! \note If the value is 64-bit integer type, it may lose precision. Use \c IsLosslessDouble() to check whether the converison is lossless.
    */
    double GetDouble() const {
        RAPIDJSON_ASSERT(IsNumber());
        if ((data_.f.flags & kDoubleFlag) != 0)                return data_.n.d;   // exact type, no conversion.
        if ((data_.f.flags & kIntFlag) != 0)                   return data_.n.i.i; // int -> double
        if ((data_.f.flags & kUintFlag) != 0)                  return data_.n.u.u; // unsigned -> double
        if ((data_.f.flags & kInt64Flag) != 0)                 return static_cast<double>(data_.n.i64); // int64_t -> double (may lose precision)
        RAPIDJSON_ASSERT((data_.f.flags & kUint64Flag) != 0);  return static_cast<double>(data_.n.u64); // uint64_t -> double (may lose precision)
    }
#endif // RAPIDJSON_YGGDRASIL

    //! Get the value as float type.
    /*! \note If the value is 64-bit integer type, it may lose precision. Use \c IsLosslessFloat() to check whether the converison is lossless.
    */
    float GetFloat() const {
        return static_cast<float>(GetDouble());
    }

    GenericValue& SetInt(int i)             { this->~GenericValue(); new (this) GenericValue(i);    return *this; }
    GenericValue& SetUint(unsigned u)       { this->~GenericValue(); new (this) GenericValue(u);    return *this; }
    GenericValue& SetInt64(int64_t i64)     { this->~GenericValue(); new (this) GenericValue(i64);  return *this; }
    GenericValue& SetUint64(uint64_t u64)   { this->~GenericValue(); new (this) GenericValue(u64);  return *this; }
    GenericValue& SetDouble(double d)       { this->~GenericValue(); new (this) GenericValue(d);    return *this; }
    GenericValue& SetFloat(float f)         { this->~GenericValue(); new (this) GenericValue(static_cast<double>(f)); return *this; }

    //@}

    //!@name String
    //@{

    const Ch* GetString() const { RAPIDJSON_ASSERT(IsString()); return DataString(data_); }

    //! Get the length of string.
    /*! Since rapidjson permits "\\u0000" in the json string, strlen(v.GetString()) may not equal to v.GetStringLength().
    */
    SizeType GetStringLength() const { RAPIDJSON_ASSERT(IsString()); return DataStringLength(data_); }

    //! Set this value as a string without copying source string.
    /*! This version has better performance with supplied length, and also support string containing null character.
        \param s source string pointer. 
        \param length The length of source string, excluding the trailing null terminator.
        \return The value itself for fluent API.
        \post IsString() == true && GetString() == s && GetStringLength() == length
        \see SetString(StringRefType)
    */
    GenericValue& SetString(const Ch* s, SizeType length) { return SetString(StringRef(s, length)); }

    //! Set this value as a string without copying source string.
    /*! \param s source string reference
        \return The value itself for fluent API.
        \post IsString() == true && GetString() == s && GetStringLength() == s.length
    */
    GenericValue& SetString(StringRefType s) { this->~GenericValue(); SetStringRaw(s); YGG_SCHEMA_INIT_CONSTRUCT; return *this; }

    //! Set this value as a string by copying from source string.
    /*! This version has better performance with supplied length, and also support string containing null character.
        \param s source string. 
        \param length The length of source string, excluding the trailing null terminator.
        \param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \post IsString() == true && GetString() != s && strcmp(GetString(),s) == 0 && GetStringLength() == length
    */
    GenericValue& SetString(const Ch* s, SizeType length, Allocator& allocator) { return SetString(StringRef(s, length), allocator); }

    //! Set this value as a string by copying from source string.
    /*! \param s source string. 
        \param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \post IsString() == true && GetString() != s && strcmp(GetString(),s) == 0 && GetStringLength() == length
    */
    GenericValue& SetString(const Ch* s, Allocator& allocator) { return SetString(StringRef(s), allocator); }

    //! Set this value as a string by copying from source string.
    /*! \param s source string reference
        \param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \post IsString() == true && GetString() != s.s && strcmp(GetString(),s) == 0 && GetStringLength() == length
    */
    GenericValue& SetString(StringRefType s, Allocator& allocator) { this->~GenericValue(); SetStringRaw(s, allocator); YGG_SCHEMA_INIT_CONSTRUCT; return *this; }

#if RAPIDJSON_HAS_STDSTRING
    //! Set this value as a string by copying from source string.
    /*! \param s source string.
        \param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \post IsString() == true && GetString() != s.data() && strcmp(GetString(),s.data() == 0 && GetStringLength() == s.size()
        \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
    */
    GenericValue& SetString(const std::basic_string<Ch>& s, Allocator& allocator) { return SetString(StringRef(s), allocator); }
#endif

    //@}

    //!@name Array
    //@{

    //! Templated version for checking whether this value is type T.
    /*!
        \tparam T Either \c bool, \c int, \c unsigned, \c int64_t, \c uint64_t, \c double, \c float, \c const \c char*, \c std::basic_string<Ch>
    */
    template <typename T>
    bool Is() const { return internal::TypeHelper<ValueType, T>::Is(*this); }

    template <typename T>
    T Get() const { return internal::TypeHelper<ValueType, T>::Get(*this); }

    template <typename T>
    T Get() { return internal::TypeHelper<ValueType, T>::Get(*this); }

    template<typename T>
    ValueType& Set(const T& data) { return internal::TypeHelper<ValueType, T>::Set(*this, data); }

    template<typename T>
    ValueType& Set(const T& data, AllocatorType& allocator) { return internal::TypeHelper<ValueType, T>::Set(*this, data, allocator); }

    //@}

    //! Generate events of this value to a Handler.
    /*! This function adopts the GoF visitor pattern.
        Typical usage is to output this JSON value as JSON text via Writer, which is a Handler.
        It can also be used to deep clone this value via GenericDocument, which is also a Handler.
        \tparam Handler type of handler.
        \param handler An object implementing concept Handler.
    */
    template <typename Handler>
#ifdef RAPIDJSON_YGGDRASIL
    bool Accept(Handler& handler, bool skip_yggdrasil=false) const {
        if (IsYggdrasil() && (!skip_yggdrasil))
	  return AcceptYggdrasil(handler);
#else // RAPIDJSON_YGGDRASIL
    bool Accept(Handler& handler) const {
#endif // RAPIDJSON_YGGDRASIL
        switch(GetType()) {
        case kNullType:     return handler.Null();
        case kFalseType:    return handler.Bool(false);
        case kTrueType:     return handler.Bool(true);

        case kObjectType:
            if (RAPIDJSON_UNLIKELY(!handler.StartObject()))
                return false;
            for (ConstMemberIterator m = MemberBegin(); m != MemberEnd(); ++m) {
                RAPIDJSON_ASSERT(m->name.IsString()); // User may change the type of name by MemberIterator.
                if (RAPIDJSON_UNLIKELY(!handler.Key(m->name.GetString(), m->name.GetStringLength(), (m->name.data_.f.flags & kCopyFlag) != 0)))
                    return false;
                if (RAPIDJSON_UNLIKELY(!m->value.Accept(handler)))
                    return false;
            }
            return handler.EndObject(data_.o.size);

        case kArrayType:
            if (RAPIDJSON_UNLIKELY(!handler.StartArray()))
                return false;
            for (ConstValueIterator v = Begin(); v != End(); ++v)
                if (RAPIDJSON_UNLIKELY(!v->Accept(handler)))
                    return false;
            return handler.EndArray(data_.a.size);
    
        case kStringType:
	    return handler.String(GetString(), GetStringLength(), (data_.f.flags & kCopyFlag) != 0);
        default:
            RAPIDJSON_ASSERT(GetType() == kNumberType);
            if (IsDouble())         return handler.Double(data_.n.d);
            else if (IsInt())       return handler.Int(data_.n.i.i);
            else if (IsUint())      return handler.Uint(data_.n.u.u);
            else if (IsInt64())     return handler.Int64(data_.n.i64);
            else                    return handler.Uint64(data_.n.u64);
        }
    }

private:
    template <typename, typename> friend class GenericValue;
    template <typename, typename, typename> friend class GenericDocument;

    enum {
        kBoolFlag       = 0x0008,
        kNumberFlag     = 0x0010,
        kIntFlag        = 0x0020,
        kUintFlag       = 0x0040,
        kInt64Flag      = 0x0080,
        kUint64Flag     = 0x0100,
        kDoubleFlag     = 0x0200,
        kStringFlag     = 0x0400,
        kCopyFlag       = 0x0800,
        kInlineStrFlag  = 0x1000,

        // Initial flags of different types.
        kNullFlag = kNullType,
        // These casts are added to suppress the warning on MSVC about bitwise operations between enums of different types.
        kTrueFlag = static_cast<int>(kTrueType) | static_cast<int>(kBoolFlag),
        kFalseFlag = static_cast<int>(kFalseType) | static_cast<int>(kBoolFlag),
        kNumberIntFlag = static_cast<int>(kNumberType) | static_cast<int>(kNumberFlag | kIntFlag | kInt64Flag),
        kNumberUintFlag = static_cast<int>(kNumberType) | static_cast<int>(kNumberFlag | kUintFlag | kUint64Flag | kInt64Flag),
        kNumberInt64Flag = static_cast<int>(kNumberType) | static_cast<int>(kNumberFlag | kInt64Flag),
        kNumberUint64Flag = static_cast<int>(kNumberType) | static_cast<int>(kNumberFlag | kUint64Flag),
        kNumberDoubleFlag = static_cast<int>(kNumberType) | static_cast<int>(kNumberFlag | kDoubleFlag),
        kNumberAnyFlag = static_cast<int>(kNumberType) | static_cast<int>(kNumberFlag | kIntFlag | kInt64Flag | kUintFlag | kUint64Flag | kDoubleFlag),
        kConstStringFlag = static_cast<int>(kStringType) | static_cast<int>(kStringFlag),
        kCopyStringFlag = static_cast<int>(kStringType) | static_cast<int>(kStringFlag | kCopyFlag),
        kShortStringFlag = static_cast<int>(kStringType) | static_cast<int>(kStringFlag | kCopyFlag | kInlineStrFlag),
        kObjectFlag = kObjectType,
        kArrayFlag = kArrayType,

        kTypeMask = 0x07
    };

    static const SizeType kDefaultArrayCapacity = RAPIDJSON_VALUE_DEFAULT_ARRAY_CAPACITY;
    static const SizeType kDefaultObjectCapacity = RAPIDJSON_VALUE_DEFAULT_OBJECT_CAPACITY;

    struct Flag {
#if RAPIDJSON_48BITPOINTER_OPTIMIZATION
        char payload[sizeof(SizeType) * 2 + 6];     // 2 x SizeType + lower 48-bit pointer
#elif RAPIDJSON_64BIT
        char payload[sizeof(SizeType) * 2 + sizeof(void*) + 6]; // 6 padding bytes
#else
        char payload[sizeof(SizeType) * 2 + sizeof(void*) + 2]; // 2 padding bytes
#endif
        uint16_t flags;
    };

    struct String {
        SizeType length;
        SizeType hashcode;  //!< reserved
        const Ch* str;
    };  // 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    // implementation detail: ShortString can represent zero-terminated strings up to MaxSize chars
    // (excluding the terminating zero) and store a value to determine the length of the contained
    // string in the last character str[LenPos] by storing "MaxSize - length" there. If the string
    // to store has the maximal length of MaxSize then str[LenPos] will be 0 and therefore act as
    // the string terminator as well. For getting the string length back from that value just use
    // "MaxSize - str[LenPos]".
    // This allows to store 13-chars strings in 32-bit mode, 21-chars strings in 64-bit mode,
    // 13-chars strings for RAPIDJSON_48BITPOINTER_OPTIMIZATION=1 inline (for `UTF8`-encoded strings).
    struct ShortString {
        enum { MaxChars = sizeof(static_cast<Flag*>(0)->payload) / sizeof(Ch), MaxSize = MaxChars - 1, LenPos = MaxSize };
        Ch str[MaxChars];

        inline static bool Usable(SizeType len) { return                       (MaxSize >= len); }
        inline void     SetLength(SizeType len) { str[LenPos] = static_cast<Ch>(MaxSize -  len); }
        inline SizeType GetLength() const       { return  static_cast<SizeType>(MaxSize -  str[LenPos]); }
    };  // at most as many bytes as "String" above => 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    // By using proper binary layout, retrieval of different integer types do not need conversions.
    union Number {
#if RAPIDJSON_ENDIAN == RAPIDJSON_LITTLEENDIAN
        struct I {
            int i;
            char padding[4];
        }i;
        struct U {
            unsigned u;
            char padding2[4];
        }u;
#else
        struct I {
            char padding[4];
            int i;
        }i;
        struct U {
            char padding2[4];
            unsigned u;
        }u;
#endif
        int64_t i64;
        uint64_t u64;
        double d;
    };  // 8 bytes

    struct ObjectData {
        SizeType size;
        SizeType capacity;
        Member* members;
    };  // 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    struct ArrayData {
        SizeType size;
        SizeType capacity;
        GenericValue* elements;
    };  // 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    union Data {
        String s;
        ShortString ss;
        Number n;
        ObjectData o;
        ArrayData a;
        Flag f;
    };  // 16 bytes in 32-bit mode, 24 bytes in 64-bit mode, 16 bytes in 64-bit with RAPIDJSON_48BITPOINTER_OPTIMIZATION

    static RAPIDJSON_FORCEINLINE const Ch* DataString(const Data& data) {
        return (data.f.flags & kInlineStrFlag) ? data.ss.str : RAPIDJSON_GETPOINTER(Ch, data.s.str);
    }
    static RAPIDJSON_FORCEINLINE SizeType DataStringLength(const Data& data) {
        return (data.f.flags & kInlineStrFlag) ? data.ss.GetLength() : data.s.length;
    }

    RAPIDJSON_FORCEINLINE const Ch* GetStringPointer() const { return RAPIDJSON_GETPOINTER(Ch, data_.s.str); }
    RAPIDJSON_FORCEINLINE const Ch* SetStringPointer(const Ch* str) { return RAPIDJSON_SETPOINTER(Ch, data_.s.str, str); }
    RAPIDJSON_FORCEINLINE GenericValue* GetElementsPointer() const { return RAPIDJSON_GETPOINTER(GenericValue, data_.a.elements); }
    RAPIDJSON_FORCEINLINE GenericValue* SetElementsPointer(GenericValue* elements) { return RAPIDJSON_SETPOINTER(GenericValue, data_.a.elements, elements); }
    RAPIDJSON_FORCEINLINE Member* GetMembersPointer() const { return RAPIDJSON_GETPOINTER(Member, data_.o.members); }
    RAPIDJSON_FORCEINLINE Member* SetMembersPointer(Member* members) { return RAPIDJSON_SETPOINTER(Member, data_.o.members, members); }

#if RAPIDJSON_USE_MEMBERSMAP

    struct MapTraits {
        struct Less {
            bool operator()(const Data& s1, const Data& s2) const {
                SizeType n1 = DataStringLength(s1), n2 = DataStringLength(s2);
                int cmp = std::memcmp(DataString(s1), DataString(s2), sizeof(Ch) * (n1 < n2 ? n1 : n2));
                return cmp < 0 || (cmp == 0 && n1 < n2);
            }
        };
        typedef std::pair<const Data, SizeType> Pair;
        typedef std::multimap<Data, SizeType, Less, StdAllocator<Pair, Allocator> > Map;
        typedef typename Map::iterator Iterator;
    };
    typedef typename MapTraits::Map         Map;
    typedef typename MapTraits::Less        MapLess;
    typedef typename MapTraits::Pair        MapPair;
    typedef typename MapTraits::Iterator    MapIterator;

    //
    // Layout of the members' map/array, re(al)located according to the needed capacity:
    //
    //    {Map*}<>{capacity}<>{Member[capacity]}<>{MapIterator[capacity]}
    //
    // (where <> stands for the RAPIDJSON_ALIGN-ment, if needed)
    //

    static RAPIDJSON_FORCEINLINE size_t GetMapLayoutSize(SizeType capacity) {
        return RAPIDJSON_ALIGN(sizeof(Map*)) +
               RAPIDJSON_ALIGN(sizeof(SizeType)) +
               RAPIDJSON_ALIGN(capacity * sizeof(Member)) +
               capacity * sizeof(MapIterator);
    }

    static RAPIDJSON_FORCEINLINE SizeType &GetMapCapacity(Map* &map) {
        return *reinterpret_cast<SizeType*>(reinterpret_cast<uintptr_t>(&map) +
                                            RAPIDJSON_ALIGN(sizeof(Map*)));
    }

    static RAPIDJSON_FORCEINLINE Member* GetMapMembers(Map* &map) {
        return reinterpret_cast<Member*>(reinterpret_cast<uintptr_t>(&map) +
                                         RAPIDJSON_ALIGN(sizeof(Map*)) +
                                         RAPIDJSON_ALIGN(sizeof(SizeType)));
    }

    static RAPIDJSON_FORCEINLINE MapIterator* GetMapIterators(Map* &map) {
        return reinterpret_cast<MapIterator*>(reinterpret_cast<uintptr_t>(&map) +
                                              RAPIDJSON_ALIGN(sizeof(Map*)) +
                                              RAPIDJSON_ALIGN(sizeof(SizeType)) +
                                              RAPIDJSON_ALIGN(GetMapCapacity(map) * sizeof(Member)));
    }

    static RAPIDJSON_FORCEINLINE Map* &GetMap(Member* members) {
        RAPIDJSON_ASSERT(members != 0);
        return *reinterpret_cast<Map**>(reinterpret_cast<uintptr_t>(members) -
                                        RAPIDJSON_ALIGN(sizeof(SizeType)) -
                                        RAPIDJSON_ALIGN(sizeof(Map*)));
    }

    // Some compilers' debug mechanisms want all iterators to be destroyed, for their accounting..
    RAPIDJSON_FORCEINLINE MapIterator DropMapIterator(MapIterator& rhs) {
#if RAPIDJSON_HAS_CXX11
        MapIterator ret = std::move(rhs);
#else
        MapIterator ret = rhs;
#endif
        rhs.~MapIterator();
        return ret;
    }

    Map* &DoReallocMap(Map** oldMap, SizeType newCapacity, Allocator& allocator) {
        Map **newMap = static_cast<Map**>(allocator.Malloc(GetMapLayoutSize(newCapacity)));
        GetMapCapacity(*newMap) = newCapacity;
        if (!oldMap) {
            *newMap = new (allocator.Malloc(sizeof(Map))) Map(MapLess(), allocator);
        }
        else {
            *newMap = *oldMap;
            size_t count = (*oldMap)->size();
            std::memcpy(static_cast<void*>(GetMapMembers(*newMap)),
                        static_cast<void*>(GetMapMembers(*oldMap)),
                        count * sizeof(Member));
            MapIterator *oldIt = GetMapIterators(*oldMap),
                        *newIt = GetMapIterators(*newMap);
            while (count--) {
                new (&newIt[count]) MapIterator(DropMapIterator(oldIt[count]));
            }
            Allocator::Free(oldMap);
        }
        return *newMap;
    }

    RAPIDJSON_FORCEINLINE Member* DoAllocMembers(SizeType capacity, Allocator& allocator) {
        return GetMapMembers(DoReallocMap(0, capacity, allocator));
    }

    void DoReserveMembers(SizeType newCapacity, Allocator& allocator) {
        ObjectData& o = data_.o;
        if (newCapacity > o.capacity) {
            Member* oldMembers = GetMembersPointer();
            Map **oldMap = oldMembers ? &GetMap(oldMembers) : 0,
                *&newMap = DoReallocMap(oldMap, newCapacity, allocator);
            RAPIDJSON_SETPOINTER(Member, o.members, GetMapMembers(newMap));
#ifdef RAPIDJSON_YGGDRASIL
	    for (SizeType i = o.capacity; i < newCapacity; i++) {
	      GetMembersPointer()[i].name.schema_ = NULL;
	      GetMembersPointer()[i].value.schema_ = NULL;
	    }
#endif // RAPIDJSON_YGGDRASIL
            o.capacity = newCapacity;
        }
    }

    template <typename SourceAllocator>
    MemberIterator DoFindMember(const GenericValue<Encoding, SourceAllocator>& name) {
        if (Member* members = GetMembersPointer()) {
            Map* &map = GetMap(members);
            MapIterator mit = map->find(reinterpret_cast<const Data&>(name.data_));
            if (mit != map->end()) {
                return MemberIterator(&members[mit->second]);
            }
        }
        return MemberEnd();
    }

    void DoClearMembers() {
        if (Member* members = GetMembersPointer()) {
            Map* &map = GetMap(members);
            MapIterator* mit = GetMapIterators(map);
            for (SizeType i = 0; i < data_.o.size; i++) {
                map->erase(DropMapIterator(mit[i]));
                members[i].~Member();
            }
            data_.o.size = 0;
        }
    }

    void DoFreeMembers() {
        if (Member* members = GetMembersPointer()) {
            GetMap(members)->~Map();
            for (SizeType i = 0; i < data_.o.size; i++) {
                members[i].~Member();
            }
            if (Allocator::kNeedFree) { // Shortcut by Allocator's trait
                Map** map = &GetMap(members);
                Allocator::Free(*map);
                Allocator::Free(map);
            }
        }
    }

#else // !RAPIDJSON_USE_MEMBERSMAP

    RAPIDJSON_FORCEINLINE Member* DoAllocMembers(SizeType capacity, Allocator& allocator) {
        return Malloc<Member>(allocator, capacity);
    }

    void DoReserveMembers(SizeType newCapacity, Allocator& allocator) {
        ObjectData& o = data_.o;
        if (newCapacity > o.capacity) {
            Member* newMembers = Realloc<Member>(allocator, GetMembersPointer(), o.capacity, newCapacity);
            RAPIDJSON_SETPOINTER(Member, o.members, newMembers);
#ifdef RAPIDJSON_YGGDRASIL
	    for (SizeType i = o.capacity; i < newCapacity; i++) {
	      GetMembersPointer()[i].name.schema_ = NULL;
	      GetMembersPointer()[i].value.schema_ = NULL;
	    }
#endif // RAPIDJSON_YGGDRASIL
            o.capacity = newCapacity;
        }
    }

    template <typename SourceAllocator>
    MemberIterator DoFindMember(const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator member = MemberBegin();
        for ( ; member != MemberEnd(); ++member)
            if (name.StringEqual(member->name))
                break;
        return member;
    }

    void DoClearMembers() {
        for (MemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
            m->~Member();
        data_.o.size = 0;
    }

    void DoFreeMembers() {
        for (MemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
            m->~Member();
        Allocator::Free(GetMembersPointer());
    }

#endif // !RAPIDJSON_USE_MEMBERSMAP

    void DoAddMember(GenericValue& name, GenericValue& value, Allocator& allocator) {
        ObjectData& o = data_.o;
        if (o.size >= o.capacity)
            DoReserveMembers(o.capacity ? (o.capacity + (o.capacity + 1) / 2) : kDefaultObjectCapacity, allocator);
        Member* members = GetMembersPointer();
        Member* m = members + o.size;
        m->name.RawAssign(name);
        m->value.RawAssign(value);
#if RAPIDJSON_USE_MEMBERSMAP
        Map* &map = GetMap(members);
        MapIterator* mit = GetMapIterators(map);
        new (&mit[o.size]) MapIterator(map->insert(MapPair(m->name.data_, o.size)));
#endif
        ++o.size;
    }

    MemberIterator DoRemoveMember(MemberIterator m) {
        ObjectData& o = data_.o;
        Member* members = GetMembersPointer();
#if RAPIDJSON_USE_MEMBERSMAP
        Map* &map = GetMap(members);
        MapIterator* mit = GetMapIterators(map);
        SizeType mpos = static_cast<SizeType>(&*m - members);
        map->erase(DropMapIterator(mit[mpos]));
#endif
        MemberIterator last(members + (o.size - 1));
        if (o.size > 1 && m != last) {
#if RAPIDJSON_USE_MEMBERSMAP
            new (&mit[mpos]) MapIterator(DropMapIterator(mit[&*last - members]));
            mit[mpos]->second = mpos;
#endif
            *m = *last; // Move the last one to this place
        }
        else {
            m->~Member(); // Only one left, just destroy
        }
        --o.size;
        return m;
    }

    MemberIterator DoEraseMembers(ConstMemberIterator first, ConstMemberIterator last) {
        ObjectData& o = data_.o;
        MemberIterator beg = MemberBegin(),
                       pos = beg + (first - beg),
                       end = MemberEnd();
#if RAPIDJSON_USE_MEMBERSMAP
        Map* &map = GetMap(GetMembersPointer());
        MapIterator* mit = GetMapIterators(map);
#endif
        for (MemberIterator itr = pos; itr != last; ++itr) {
#if RAPIDJSON_USE_MEMBERSMAP
            map->erase(DropMapIterator(mit[itr - beg]));
#endif
            itr->~Member();
        }
#if RAPIDJSON_USE_MEMBERSMAP
        if (first != last) {
            // Move remaining members/iterators
            MemberIterator next = pos + (last - first);
            for (MemberIterator itr = pos; next != end; ++itr, ++next) {
                std::memcpy(static_cast<void*>(&*itr), &*next, sizeof(Member));
                SizeType mpos = static_cast<SizeType>(itr - beg);
                new (&mit[mpos]) MapIterator(DropMapIterator(mit[next - beg]));
                mit[mpos]->second = mpos;
            }
        }
#else
        std::memmove(static_cast<void*>(&*pos), &*last,
                     static_cast<size_t>(end - last) * sizeof(Member));
#endif
        o.size -= static_cast<SizeType>(last - first);
        return pos;
    }

    template <typename SourceAllocator>
    void DoCopyMembers(const GenericValue<Encoding,SourceAllocator>& rhs, Allocator& allocator, bool copyConstStrings) {
        RAPIDJSON_ASSERT(rhs.GetType() == kObjectType);

        data_.f.flags = kObjectFlag;
        SizeType count = rhs.data_.o.size;
        Member* lm = DoAllocMembers(count, allocator);
        const typename GenericValue<Encoding,SourceAllocator>::Member* rm = rhs.GetMembersPointer();
#if RAPIDJSON_USE_MEMBERSMAP
        Map* &map = GetMap(lm);
        MapIterator* mit = GetMapIterators(map);
#endif
        for (SizeType i = 0; i < count; i++) {
            new (&lm[i].name) GenericValue(rm[i].name, allocator, copyConstStrings);
            new (&lm[i].value) GenericValue(rm[i].value, allocator, copyConstStrings);
#if RAPIDJSON_USE_MEMBERSMAP
            new (&mit[i]) MapIterator(map->insert(MapPair(lm[i].name.data_, i)));
#endif
        }
        data_.o.size = data_.o.capacity = count;
        SetMembersPointer(lm);
    }

    // Initialize this value as array with initial data, without calling destructor.
    void SetArrayRaw(GenericValue* values, SizeType count, Allocator& allocator) {
        data_.f.flags = kArrayFlag;
        if (count) {
            GenericValue* e = static_cast<GenericValue*>(allocator.Malloc(count * sizeof(GenericValue)));
            SetElementsPointer(e);
            std::memcpy(static_cast<void*>(e), values, count * sizeof(GenericValue));
        }
        else
            SetElementsPointer(0);
        data_.a.size = data_.a.capacity = count;
    }

    //! Initialize this value as object with initial data, without calling destructor.
    void SetObjectRaw(Member* members, SizeType count, Allocator& allocator) {
        data_.f.flags = kObjectFlag;
        if (count) {
            Member* m = DoAllocMembers(count, allocator);
            SetMembersPointer(m);
            std::memcpy(static_cast<void*>(m), members, count * sizeof(Member));
#if RAPIDJSON_USE_MEMBERSMAP
            Map* &map = GetMap(m);
            MapIterator* mit = GetMapIterators(map);
            for (SizeType i = 0; i < count; i++) {
                new (&mit[i]) MapIterator(map->insert(MapPair(m[i].name.data_, i)));
            }
#endif
        }
        else
            SetMembersPointer(0);
        data_.o.size = data_.o.capacity = count;
    }

    //! Initialize this value as constant string, without calling destructor.
    void SetStringRaw(StringRefType s) RAPIDJSON_NOEXCEPT {
        data_.f.flags = kConstStringFlag;
        SetStringPointer(s);
        data_.s.length = s.length;
    }

    //! Initialize this value as copy string with initial data, without calling destructor.
    void SetStringRaw(StringRefType s, Allocator& allocator) {
        Ch* str = 0;
        if (ShortString::Usable(s.length)) {
            data_.f.flags = kShortStringFlag;
            data_.ss.SetLength(s.length);
            str = data_.ss.str;
        } else {
            data_.f.flags = kCopyStringFlag;
            data_.s.length = s.length;
            str = static_cast<Ch *>(allocator.Malloc((s.length + 1) * sizeof(Ch)));
            SetStringPointer(str);
        }
        std::memcpy(str, s, s.length * sizeof(Ch));
        str[s.length] = '\0';
    }

    //! Assignment without calling destructor
    void RawAssign(GenericValue& rhs) RAPIDJSON_NOEXCEPT {
        data_ = rhs.data_;
        // data_.f.flags = rhs.data_.f.flags;
        rhs.data_.f.flags = kNullFlag;
#ifdef RAPIDJSON_YGGDRASIL
	RawAssignSchema(rhs);
#endif // RAPIDJSON_YGGDRASIL
    }

    template <typename SourceAllocator>
    bool StringEqual(const GenericValue<Encoding, SourceAllocator>& rhs) const {
        RAPIDJSON_ASSERT(IsString());
        RAPIDJSON_ASSERT(rhs.IsString());

        const SizeType len1 = GetStringLength();
        const SizeType len2 = rhs.GetStringLength();
        if(len1 != len2) { return false; }

        const Ch* const str1 = GetString();
        const Ch* const str2 = rhs.GetString();
        if(str1 == str2) { return true; } // fast path for constant string

        return (std::memcmp(str1, str2, sizeof(Ch) * len1) == 0);
    }

    Data data_;


#ifdef RAPIDJSON_YGGDRASIL
public:

#define RAPIDJSON_STRING_(name, ...)					\
  static const ValueType& Get##name##String() {				\
    static const Ch s[] = { __VA_ARGS__, '\0' };			\
    static const ValueType v(s, static_cast<SizeType>(sizeof(s) / sizeof(Ch) - 1)); \
    return v;								\
  }

  RAPIDJSON_STRING_(Type, 't', 'y', 'p', 'e')
  RAPIDJSON_STRING_(Object, 'o', 'b', 'j', 'e', 'c', 't')
  RAPIDJSON_STRING_(Array, 'a', 'r', 'r', 'a', 'y')
  RAPIDJSON_STRING_(Properties, 'p', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's')
  RAPIDJSON_STRING_(Items, 'i', 't', 'e', 'm', 's')
  RAPIDJSON_STRING_(Scalar, 's', 'c', 'a', 'l', 'a', 'r')
  RAPIDJSON_STRING_(1DArray, '1', 'd', 'a', 'r', 'r', 'a', 'y')
  RAPIDJSON_STRING_(NDArray, 'n', 'd', 'a', 'r', 'r', 'a', 'y')
  RAPIDJSON_STRING_(PythonClass, 'c', 'l', 'a', 's', 's')
  RAPIDJSON_STRING_(PythonFunction, 'f', 'u', 'n', 'c', 't', 'i', 'o', 'n')
  RAPIDJSON_STRING_(PythonInstance, 'i', 'n', 's', 't', 'a', 'n', 'c', 'e')
  RAPIDJSON_STRING_(Obj, 'o', 'b', 'j')
  RAPIDJSON_STRING_(ObjWavefront, 'o', 'b', 'j')
  RAPIDJSON_STRING_(Ply, 'p', 'l', 'y')
  RAPIDJSON_STRING_(Schema, 's', 'c', 'h', 'e', 'm', 'a')
  RAPIDJSON_STRING_(Any, 'a', 'n', 'y')
  // props
  RAPIDJSON_STRING_(Title, 't', 'i', 't', 'l', 'e')
  RAPIDJSON_STRING_(SubType, 's', 'u', 'b', 't', 'y', 'p', 'e')
  RAPIDJSON_STRING_(Precision, 'p', 'r', 'e', 'c', 'i', 's', 'i', 'o', 'n')
  RAPIDJSON_STRING_(Units, 'u', 'n', 'i', 't', 's')
  RAPIDJSON_STRING_(Length, 'l', 'e', 'n', 'g', 't', 'h')
  RAPIDJSON_STRING_(Shape, 's', 'h', 'a', 'p', 'e')
  RAPIDJSON_STRING_(Args, 'a', 'r', 'g', 's')
  RAPIDJSON_STRING_(Kwargs, 'k', 'w', 'a', 'r', 'g', 's')
  RAPIDJSON_STRING_(Encoding, 'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g')
  RAPIDJSON_STRING_(ASCIIEncoding, 'A', 'S', 'C', 'I', 'I')
  RAPIDJSON_STRING_(UCS4Encoding, 'U', 'C', 'S', '4')
  RAPIDJSON_STRING_(UTF8Encoding, 'U', 'T', 'F', '8')
  RAPIDJSON_STRING_(UTF16Encoding, 'U', 'T', 'F', '1', '6')
  RAPIDJSON_STRING_(UTF32Encoding, 'U', 'T', 'F', '3', '2')
    
  // Subtypes
  RAPIDJSON_STRING_(IntSubType, 'i', 'n', 't')
  RAPIDJSON_STRING_(UintSubType, 'u', 'i', 'n', 't')
  RAPIDJSON_STRING_(FloatSubType, 'f', 'l', 'o', 'a', 't')
  RAPIDJSON_STRING_(ComplexSubType, 'c', 'o', 'm', 'p', 'l', 'e', 'x')
  RAPIDJSON_STRING_(StringSubType, 's', 't', 'r', 'i', 'n', 'g')
  RAPIDJSON_STRING_(NullSubType, 'n', 'u', 'l', 'l')

#undef RAPIDJSON_STRING_

  // Specialization checking if handler has Yggdrasil method
  template <typename Handler>
  bool AcceptYggdrasil(Handler& handler, RAPIDJSON_ENABLEIF((internal::HasYggdrasilMethod<Handler,SchemaValueType>))) const {
    switch(GetType()) {
    case kObjectType: {
      if (RAPIDJSON_UNLIKELY(!handler.YggdrasilStartObject(*schema_)))
	return false;
      for (ConstMemberIterator m = MemberBegin(); m != MemberEnd(); ++m) {
	RAPIDJSON_ASSERT(m->name.IsString()); // User may change the type of name by MemberIterator.
	if (RAPIDJSON_UNLIKELY(!handler.Key(m->name.GetString(), m->name.GetStringLength(), (m->name.data_.f.flags & kCopyFlag) != 0)))
	  return false;
	if (RAPIDJSON_UNLIKELY(!m->value.Accept(handler)))
	  return false;
      }
      return handler.YggdrasilEndObject(data_.o.size);
    }
    case kStringType:
      return handler.YggdrasilString(GetString(), GetStringLength(), (data_.f.flags & kCopyFlag) != 0, *schema_);
    default:
      return false;
    }
  }
  // Fallback
  template <typename Handler>
  bool AcceptYggdrasil(Handler& handler, RAPIDJSON_DISABLEIF((internal::HasYggdrasilMethod<Handler,SchemaValueType>))) const {
    return Accept(handler, true); }

  static const ValueType& YggSubTypeString(enum YggSubType subtype) {
    switch (subtype) {
    case kYggUintSubType:
      return GetUintSubTypeString();
    case kYggIntSubType:
      return GetIntSubTypeString();
    case kYggFloatSubType:
      return GetFloatSubTypeString();
    case kYggComplexSubType:
      return GetComplexSubTypeString();
    case kYggStringSubType:
      return GetStringSubTypeString();
    default:
      RAPIDJSON_ASSERT(subtype == kYggNullSubType);
      return GetNullSubTypeString();
    }
  }
  
  template <typename T>
  static const ValueType& YggSubTypeString() {
    return YggSubTypeString(GetYggSubType<T>());
  }

  //! Constructors for strings w/ schema
  GenericValue(const Ch* s, SizeType length,
	       const Ch* schema, SizeType schema_length) RAPIDJSON_NOEXCEPT :
    data_() YGG_SCHEMA_INIT {
    SetStringRaw(StringRef(s, length));
    SetValueSchemaRaw(schema, schema_length);
  }
  explicit GenericValue(StringRefType s,
			const Ch* schema, SizeType schema_length) RAPIDJSON_NOEXCEPT :
    data_() YGG_SCHEMA_INIT {
    SetStringRaw(s);
    SetValueSchemaRaw(schema, schema_length);
  }
  GenericValue(const Ch* s, SizeType length, Allocator& allocator,
	       const Ch* schema, SizeType schema_length) RAPIDJSON_NOEXCEPT :
    data_() YGG_SCHEMA_INIT {
    SetStringRaw(StringRef(s, length), allocator);
    SetValueSchemaRaw(schema, schema_length, &allocator);
  }
  GenericValue(const Ch* s, Allocator& allocator,
	       const Ch* schema, SizeType schema_length) RAPIDJSON_NOEXCEPT :
    data_() YGG_SCHEMA_INIT {
    SetStringRaw(StringRef(s), allocator);
    SetValueSchemaRaw(schema, schema_length, &allocator);
  }
#if RAPIDJSON_HAS_STDSTRING
  GenericValue(const std::basic_string<Ch>& s, Allocator& allocator,
	       const Ch* schema, SizeType schema_length) RAPIDJSON_NOEXCEPT :
    data_() YGG_SCHEMA_INIT {
    SetStringRaw(StringRef(s), allocator);
    SetValueSchemaRaw(schema, schema_length, &allocator);
  }
#endif
  // Units
  template <typename T>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType> &x,
			Allocator& allocator,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType nelements = 1;
    SetNDArrayRaw(&x, &nelements, 1, &allocator);
  }
  template <typename T>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType> &x,
			Allocator* allocator = 0,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType nelements = 1;
    SetNDArrayRaw(&x, &nelements, 1, allocator);
  }
  template <typename T, SizeType N>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType> (&x)[N],
			Allocator& allocator,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType nelements = N;
    SetNDArrayRaw(&(x[0]), &nelements, 1, &allocator);
  }
  template <typename T, SizeType N>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType> (&x)[N],
			Allocator* allocator = 0,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType nelements = N;
    SetNDArrayRaw(&(x[0]), &nelements, 1, allocator);
  }
  template <typename T>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType>* x,
			SizeType nelements,
			Allocator& allocator,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, &nelements, 1, &allocator); }
  template <typename T>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType>* x,
			SizeType nelements,
			Allocator* allocator = 0,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, &nelements, 1, allocator); }
  template <typename T, SizeType M, SizeType N>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType> (&x)[M][N],
			Allocator& allocator,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[2] = {M, N};
    SetNDArrayRaw(&(x[0][0]), &(shape[0]), 2, &allocator);
  }
  template <typename T, SizeType M, SizeType N>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType> (&x)[M][N],
			Allocator* allocator = 0,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[2] = {M, N};
    SetNDArrayRaw(&(x[0][0]), &(shape[0]), 2, allocator);
  }
  template <typename T, SizeType N>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType>* x,
			SizeType shape[N],
			Allocator& allocator,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, &(shape[0]), N, &allocator); }
  template <typename T, SizeType N>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType>* x,
			SizeType shape[N],
			Allocator* allocator = 0,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, &(shape[0]), N, allocator); }
  template <typename T>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType>* x,
			SizeType* shape,
			SizeType ndim, Allocator& allocator,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, &allocator); }
  template <typename T>
  explicit GenericValue(const units::GenericQuantityArray<T, EncodingType>& x,
			Allocator* allocator = 0,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(&x, allocator); }
  template <typename T>
  explicit GenericValue(const units::GenericQuantity<T, EncodingType>* x,
			const SizeType* shape, const SizeType ndim,
			Allocator* allocator = 0,
			RAPIDJSON_DISABLEIF((internal::IsPointer<T>)))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, allocator); }

  // Scalars not covered by rapidjson & units
  template <typename T>
  explicit GenericValue(const T x,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_SCALAR_TYPE(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(&x, nullptr, 0, nullptr, 0); }
  template <typename T>
  explicit GenericValue(const T x,
			const Ch* units_str, const SizeType units_len=0,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(&x, nullptr, 0, units_str, units_len); }
  template <typename T>
  explicit GenericValue(const T x, Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_SCALAR_TYPE(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(&x, nullptr, 0, nullptr, 0, &allocator); }
  template <typename T>
  explicit GenericValue(const T x,
			const Ch* units_str, const SizeType units_len,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(&x, nullptr, 0, units_str, units_len, &allocator); }
  // Scalar string
  explicit GenericValue(const Ch* x, const SizeType precision,
			const Ch* encoding, const SizeType encoding_len,
			Allocator& allocator)
  { SetNDArrayRaw(x, nullptr, 0, 0, 0, &allocator, kYggStringSubType, precision, encoding, encoding_len); }
  // Explicit 1D Array
  template <typename T, SizeType N>
  explicit GenericValue(const T (&x)[N],
			const Ch* units_str=nullptr, const SizeType units_len=0,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType nelements = N;
    SetNDArrayRaw(&(x[0]), &nelements, 1, units_str, units_len);
  }
  template <typename T, SizeType N>
  explicit GenericValue(const T (&x)[N],
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType nelements = N;
    SetNDArrayRaw(&(x[0]), &nelements, 1, nullptr, 0, &allocator);
  }
  template <typename T, SizeType N>
  explicit GenericValue(const T (&x)[N],
			const Ch* units_str,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType nelements = N;
    SetNDArrayRaw(&(x[0]), &nelements, 1, units_str, 0, &allocator);
  }
  template <typename T, SizeType N>
  explicit GenericValue(const T (&x)[N],
			const Ch* units_str, const SizeType units_len,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType nelements = N;
    SetNDArrayRaw(&(x[0]), &nelements, 1, units_str, units_len, &allocator);
  }
  // 1D Array as pointer
  template <typename T>
  explicit GenericValue(const T* x, SizeType nelements,
			const Ch* units_str=nullptr, const SizeType units_len=0,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, &nelements, 1, units_str, units_len); }
  template <typename T>
  explicit GenericValue(const T* x, SizeType nelements, Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, &nelements, 1, nullptr, 0, &allocator); }
  template <typename T>
  explicit GenericValue(const T* x, const SizeType nelements,
			const Ch* units_str,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, &nelements, 1, units_str, 0, &allocator); }
  template <typename T>
  explicit GenericValue(const T* x, const SizeType nelements,
			const Ch* units_str, const SizeType units_len,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, &nelements, 1, units_str, units_len, &allocator); }
  // 1D array of strings
  explicit GenericValue(const Ch* x, const SizeType precision,
			const SizeType nelements, Allocator& allocator,
			const Ch* encoding=0, const SizeType encoding_len=0)
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT {
    SizeType shape[] = {nelements};
    SetNDArrayRaw(x, shape, 1, nullptr, 0, &allocator, kYggStringSubType, precision, encoding, encoding_len);
  }
  template <SizeType M, SizeType N>
  explicit GenericValue(const Ch (&x)[M][N],
			const Ch* encoding=0, const SizeType encoding_len=0)
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[] = {M};
    SetNDArrayRaw(&(x[0][0]), shape, 1, nullptr, 0, 0, kYggStringSubType, N, encoding, encoding_len);
  }
  template <SizeType M, SizeType N>
  explicit GenericValue(const Ch (&x)[M][N], Allocator& allocator,
			const Ch* encoding=0, const SizeType encoding_len=0)
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[] = {M};
    SetNDArrayRaw(&(x[0][0]), shape, 1, nullptr, 0, &allocator, kYggStringSubType, N, encoding, encoding_len);
  }
  // Explicit 2D array
  template <typename T, SizeType M, SizeType N>
  explicit GenericValue(const T (&x)[M][N],
			const Ch* units_str=nullptr, const SizeType units_len=0,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[] = {M, N};
    SetNDArrayRaw(&(x[0][0]), shape, 2, units_str, units_len);
  }
  template <typename T, SizeType M, SizeType N>
  explicit GenericValue(const T (&x)[M][N],
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[] = {M, N};
    SetNDArrayRaw(&(x[0][0]), shape, 2, nullptr, 0, allocator);
  }
  template <typename T, SizeType M, SizeType N>
  explicit GenericValue(const T (&x)[M][N],
			const Ch* units_str,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[] = {M, N};
    SetNDArrayRaw(&(x[0][0]), shape, 2, units_str, 0, allocator);
  }
  template <typename T, SizeType M, SizeType N>
  explicit GenericValue(const T (&x)[M][N],
			const Ch* units_str, const SizeType units_len,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[] = {M, N};
    SetNDArrayRaw(&(x[0][0]), shape, 2, units_str, units_len, allocator);
  }
  // Pointer to ND array
  template <typename T>
  explicit GenericValue(const T* x,
			const SizeType shape[], const SizeType ndim,
			const Ch* units_str=nullptr, const SizeType units_len=0,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, units_str, units_len); }
  template <typename T>
  explicit GenericValue(const T* x,
			const SizeType shape[], const SizeType ndim,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, nullptr, 0, &allocator); }
  template <typename T>
  explicit GenericValue(const T* x,
			const SizeType shape[], const SizeType ndim,
			const Ch* units_str,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, units_str, 0, &allocator); }
  template <typename T>
  explicit GenericValue(const T* x,
			const SizeType shape[], const SizeType ndim,
			const Ch* units_str, const SizeType units_len,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, units_str, units_len, &allocator); }
  // Get ndim from template
  template <typename T, SizeType N>
  explicit GenericValue(const T* x,
			const SizeType shape[N], const SizeType ndim,
			const Ch* units_str=nullptr, const SizeType units_len=0,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, units_str, units_len); }
  template <typename T, SizeType N>
  explicit GenericValue(const T* x,
			const SizeType shape[N], const SizeType ndim,
			const Ch* units_str, const SizeType units_len,
			Allocator& allocator,
			RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T))))
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, units_str, units_len, &allocator); }
  // ND array of strings
  explicit GenericValue(const Ch* x, const SizeType precision,
			const SizeType shape[], const SizeType ndim,
			Allocator& allocator,
			const Ch* encoding=0, const SizeType encoding_len=0)
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetNDArrayRaw(x, shape, ndim, nullptr, 0, &allocator, kYggStringSubType, precision, encoding, encoding_len); }
  template <SizeType L, SizeType M, SizeType N>
  explicit GenericValue(const Ch (&x)[L][M][N],
			const Ch* encoding=0, const SizeType encoding_len=0)
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[] = {L, M};
    SetNDArrayRaw(&(x[0][0][0]), shape, 2, nullptr, 0, 0, kYggStringSubType, N, encoding, encoding_len);
  }
  template <SizeType L, SizeType M, SizeType N>
  explicit GenericValue(const Ch (&x)[L][M][N], Allocator& allocator,
			const Ch* encoding=0, const SizeType encoding_len=0)
    RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  {
    SizeType shape[] = {L, M};
    SetNDArrayRaw(&(x[0][0][0]), shape, 2, nullptr, 0, &allocator, kYggStringSubType, N, encoding, encoding_len);
  }
  // Other types with dedicated classes
  explicit GenericValue(PyObject* x) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetPythonObjectRaw(x); }
  explicit GenericValue(PyObject* x, Allocator& allocator) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetPythonObjectRaw(x, &allocator); }
  explicit GenericValue(const ObjWavefront& x, Allocator* allocator = 0) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetObjRaw(x, allocator); }
  explicit GenericValue(const Ply& x, Allocator* allocator = 0) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT
  { SetPlyRaw(x, allocator); }
  // Explicit schema provided
  template <typename SourceAllocator>
  explicit GenericValue(const Ch* s, SizeType length, Allocator& allocator,
			const GenericValue<Encoding,SourceAllocator>& schema) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT { SetStringRaw(StringRef(s, length), allocator); SetValueSchema(schema, &allocator); }
  template <typename SourceAllocator>
  explicit GenericValue(const Ch* s, SizeType length,
			const GenericValue<Encoding,SourceAllocator>& schema) RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT { SetStringRaw(StringRef(s, length)); SetValueSchema(schema); }
  template <typename SourceAllocator>
  explicit GenericValue(Type type,
			const GenericValue<Encoding,SourceAllocator>& schema)
  RAPIDJSON_NOEXCEPT : data_() YGG_SCHEMA_INIT {
    static const uint16_t defaultFlags[] = {
      kNullFlag, kFalseFlag, kTrueFlag, kObjectFlag, kArrayFlag, kShortStringFlag,
      kNumberAnyFlag
    };
    RAPIDJSON_NOEXCEPT_ASSERT(type >= kNullType && type <= kNumberType);
    data_.f.flags = defaultFlags[type];
    
    // Use ShortString to store empty string.
    if (type == kStringType)
      data_.ss.SetLength(0);
    SetValueSchema(schema);
  }

  // TODO: Pass stack allocator?
  void InitSchema(Allocator* allocator = 0) {
    bool allocator_created = false;
    if (!allocator) {
      allocator = RAPIDJSON_NEW(Allocator)();
      allocator_created = true;
    }
    schema_ = reinterpret_cast<SchemaValueType*>(allocator->Malloc(sizeof(SchemaValueType)));
    new (schema_) SchemaValueType(kObjectType, allocator,
				  1024, allocator);
    // Force document to clean up allocator (and thereby it's
    // own memory for the MemoryPoolAllocator)
    if (allocator_created)
      schema_->ownAllocator_ = schema_->allocator_;
  }
  void DestroySchema() {
    if (schema_ != NULL) {
      Allocator* schema_allocator = schema_->ownAllocator_;
      schema_->ownAllocator_ = NULL;
      schema_->ClearStack();
      schema_->~GenericDocument();
      Allocator::Free(schema_);
      RAPIDJSON_DELETE(schema_allocator);
      schema_ = NULL;
      schema_allocator = NULL;
    }
  }
  void ResetSchema(Allocator* allocator = 0) {
    DestroySchema();
    InitSchema(allocator);
  }
  // TODO: additional location where setschema called without allocator
  template <typename SourceStackAllocator>
  void SetValueSchema(GenericDocument<Encoding, Allocator, SourceStackAllocator>& schema) {
    return SetValueSchema(schema, schema.GetAllocator());
  }
  // Non-copy version
  // template <typename SourceAllocator>
  // void SetValueSchema(GenericValue<Encoding,SourceAllocator>& schema,
  // 		 SourceAllocator* allocator = 0) {
  //   RAPIDJSON_ASSERT(schema.IsObject());
  //   if (schema_ == NULL)
  //     InitSchema(allocator);
  //   schema_ = schema;
  // }
  // Copy version
  template <typename SourceAllocator>
  void SetValueSchema(const GenericValue<Encoding,SourceAllocator>& schema,
		      Allocator* allocator = 0) {
    RAPIDJSON_ASSERT(schema.IsObject());
    if (schema_ == NULL)
      InitSchema(allocator);
    schema_->CopyFrom(schema, schema_->GetAllocator(), true);
  }
  bool HasSchema() const {
    if (schema_ == NULL) return false;
    if (!(schema_->IsObject())) return false;
    return schema_->HasMember(GetTypeString());
  }
  bool HasUnits() const {
    if (!HasSchema()) return false;
    return schema_->HasMember(GetUnitsString());
  }
#if RAPIDJSON_HAS_STDSTRING
  bool SetUnits(const std::basic_string<Ch> units) {
    return SetUnits(units.c_str(), units.length());
  }
#endif // RAPIDJSON_HAS_STDSTRING
  bool SetUnits(const Ch* units_str, const SizeType units_len=0) {
    if (units_len == 0)
      AddSchemaMember(GetUnitsString(), units_str,
		      internal::StrLen(units_str));
    else
      AddSchemaMember(GetUnitsString(), units_str, units_len);
    return true;
  }
  const ValueType& GetUnits() const {
    RAPIDJSON_ASSERT(HasUnits());
    return schema_->FindMember(GetUnitsString())->value;
  }
  const SchemaValueType& GetValueSchema() const {
    RAPIDJSON_ASSERT(schema_ != NULL);
    return *schema_;
  }
  bool HasSchemaNested() const {
    if (HasSchema()) return true;
    switch(GetType()) {
    case kObjectType:
      {
	for (ConstMemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
	  if (m->value.HasSchemaNested()) return true;
	break;
      }
    case kArrayType:
      {
	for (ConstValueIterator v = Begin(); v != End(); ++v)
	  if (v->HasSchemaNested()) return true;
	break;
      }
    default:
      break;
    }
    return false;
  }
  ValueType GetSchemaNested(Allocator& allocator) const {
    ValueType out(kObjectType);
    if (HasSchema()) {
      for (typename SchemaValueType::ConstMemberIterator m = schema_->MemberBegin();
	   m != schema_->MemberEnd(); ++m)
	out.AddMember(ValueType(m->name, allocator, true),
		      ValueType(m->value, allocator, true).Move(),
		      allocator);
    } else {
      switch(GetType()) {
      case kObjectType:
	{
	  ValueType v(StringRef(GetObjectString().GetString()));
	  out.AddMember(GetTypeString(), v, allocator);
	  ValueType props(kObjectType);
	  for (ConstMemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
	    if (m->value.HasSchemaNested())
	      props.AddMember(StringRef(m->name.GetString(),
					m->name.GetStringLength()),
			      m->value.GetSchemaNested(allocator).Move(),
			      allocator);
	  out.AddMember(GetPropertiesString(), props, allocator);
	  break;
	}
      case kArrayType:
	{
	  ValueType v(StringRef(GetArrayString().GetString()));
	  out.AddMember(GetTypeString(), v, allocator);
	  ValueType items(kArrayType);
	  for (ConstValueIterator v2 = Begin(); v2 != End(); ++v2)
	    items.PushBack(v2->GetSchemaNested(allocator).Move(), allocator);
	  out.AddMember(GetItemsString(), items, allocator);
	  break;
	}
      default:
	break;
      }
    }
    return out;
  }
  void RawAssignSchema(GenericValue& rhs) RAPIDJSON_NOEXCEPT {
    schema_ = rhs.schema_;
    rhs.schema_ = NULL;
  }
  void AddSchemaMember(const ValueType& key, const ValueType& value) {
    RAPIDJSON_ASSERT(schema_ != NULL);
    Allocator& allocator = schema_->GetAllocator();
    schema_->AddMember(typename SchemaValueType::ValueType(key, allocator, true),
		       typename SchemaValueType::ValueType(value, allocator, true).Move(),
		       allocator);
  }
  void AddSchemaMember(const ValueType& key, unsigned int value) {
    RAPIDJSON_ASSERT(schema_ != NULL);
    Allocator& allocator = schema_->GetAllocator();
    schema_->AddMember(typename SchemaValueType::ValueType(key, allocator, true),
		       typename SchemaValueType::ValueType(value).Move(),
		       allocator);
  }
  void AddSchemaMember(const ValueType& key, const Ch* str, SizeType str_len) {
    RAPIDJSON_ASSERT(schema_ != NULL);
    Allocator& allocator = schema_->GetAllocator();
    schema_->AddMember(typename SchemaValueType::ValueType(key, allocator, true),
		       typename SchemaValueType::ValueType(str, str_len, allocator).Move(),
		       allocator);
  }
		       

  template <typename SourceEncoding, typename SourceAllocator,
	    typename SourceStackAllocator>
  bool AddSchema(GenericDocument<SourceEncoding, SourceAllocator, SourceStackAllocator>& schema) {
    return AddSchema(schema, schema.allocator_);
  }
  template <typename SourceEncoding, typename SourceAllocator>
  bool AddSchema(GenericValue<SourceEncoding, SourceAllocator>& schema,
		 SourceAllocator* allocator = 0) {
    switch(GetType()) {
    case kObjectType:
      if (schema.IsObject() &&
	  schema.HasMember(GetPropertiesString())) {
	for (MemberIterator m = MemberBegin(); m != MemberEnd(); ++m) {
	  RAPIDJSON_ASSERT(m->name.IsString()); // User may change the type of name by MemberIterator.
	  if (schema[GetPropertiesString()].HasMember(m->name))
	    if (RAPIDJSON_UNLIKELY(!m->value.AddSchema(schema[GetPropertiesString()][m->name],
						       allocator)))
	      return false;
	}
      }
      if ((schema[GetTypeString()] == GetPythonInstanceString()) ||
	  (schema[GetTypeString()] == GetSchemaString()))
	SetValueSchema(schema, allocator);
      return true;
    case kArrayType:
      if (schema.IsObject() &&
	  schema.HasMember(GetItemsString())) {
	SizeType i = 0;
	for (ValueType* v = Begin(); v != End(); ++v, ++i)
	  if (i < schema[GetItemsString()].Size())
	    if (RAPIDJSON_UNLIKELY(!v->AddSchema(schema[GetItemsString()][i],
						 allocator)))
	      return false;
      }
      return true;
    case kStringType:
      if (schema.IsObject() &&
	  schema.HasMember(GetTypeString())) {
	if ((schema[GetTypeString()] == GetScalarString()) ||
	    (schema[GetTypeString()] == Get1DArrayString()) ||
	    (schema[GetTypeString()] == GetNDArrayString()) ||
	    (schema[GetTypeString()] == GetPythonClassString()) ||
	    (schema[GetTypeString()] == GetPythonFunctionString()) ||
	    (schema[GetTypeString()] == GetPythonInstanceString()) ||
	    (schema[GetTypeString()] == GetObjString()) ||
	    (schema[GetTypeString()] == GetPlyString())) {
	  SetValueSchema(schema, allocator);
	}
      }
      break;
    default:
      if (IsYggdrasil())
	SetValueSchema(schema, allocator);
    }
    return true;
  }

  void SetValueSchemaRaw(const Ch* s, SizeType,
		    Allocator* allocator = 0) {
    ResetSchema(allocator);
    schema_ = &(schema_->Parse(s));
  }

  //! Variable argument setting/getting
  template <typename DocumentType>
  bool ApplyVarArgs(ValueType& schema, VarArgList &ap,
		    const uint16_t flag, DocumentType* parent,
		    size_t table_nelements=0) {
    if (!(schema.IsObject() && schema.HasMember("type") &&
	  schema["type"].IsString())) {
      // ygglog_throw_error("ApplyVarArgs: Schema must be an object "
      //                    "containing a 'type' string property.");
      return false;
    }
    RAPIDJSON_ASSERT((flag != kGetVarArgsFlag) || parent);
    if (flag == kGetVarArgsFlag and !parent)
      return false;
    std::string schema_type(schema["type"].GetString());
#define BASE_(method_set, method_chk, method_get, type)		\
    type tmp;							\
    if (flag == kSetVarArgsFlag) {				\
      if (!method_chk)						\
	return false;						\
      tmp = method_set;						\
    }								\
    if (!ap.apply(&tmp, flag)) {				\
      return false;						\
    }								\
    if (flag == kGetVarArgsFlag) {				\
      method_get;						\
    }
#define BASE_STD_(method, type)			\
    BASE_(Get ## method(), Is ## method(), Set ## method(tmp), type)
#define CASE_STRING_							\
    const char* tmp = NULL;						\
    size_t tmp_len = 0;							\
    char* mem = NULL;							\
    char** mem_ref = NULL;						\
    size_t mem_len = 0;							\
    if (flag == kSetVarArgsFlag) {					\
      tmp_len = (size_t)GetStringLength();				\
      tmp = GetString();						\
      mem_len = tmp_len;						\
    }									\
    if (!ap.apply_mem(mem, mem_ref, flag))				\
      return false;							\
    if (!ap.apply(&mem_len, flag, true))				\
      return false;							\
    if (flag == kSetVarArgsFlag) {					\
      if (!ap.set_mem_term(mem, mem_ref, mem_len, tmp, tmp_len))	\
	return false;							\
    } else if (flag == kGetVarArgsFlag) {				\
      SetString(mem, (SizeType)mem_len, parent->GetAllocator());	\
    }
    // TODO: Move this to yggdrasil side
    bool use_generic = false;
    if (schema.HasMember("use_generic") &&
	schema["use_generic"].IsBool() &&
	schema["use_generic"].GetBool()) {
      use_generic = true;
    }
    if (use_generic ||
	schema_type == std::string("any") ||
	schema_type == std::string("schema")) {
      DocumentType* tmp = NULL;
      DocumentType** tmp_ref = NULL;
      if (!ap.apply_mem(tmp, tmp_ref, flag))
	return false;
      if (flag == kSetVarArgsFlag) {
	if (tmp == NULL) {
	  // if (!ap.allow_realloc)
	  //   return false;
	  tmp = new DocumentType();
	  tmp_ref[0] = tmp;
	}
	tmp->CopyFrom(*this, tmp->GetAllocator());
      } else if (flag == kGetVarArgsFlag) {
	CopyFrom(*tmp, parent->GetAllocator());
      }
      return true;
    }
    else if (schema_type == "null") {
      BASE_(NULL, IsNull(), SetNull(), void*)
    }
    else if (schema_type == "boolean") {
      BASE_STD_(Bool, bool)
    }
    else if (schema_type == std::string("integer")) {
      BASE_STD_(Int, int);
    }
    else if (schema_type == std::string("number")) {
      BASE_STD_(Double, double)
    }
    // else if (schema_type == std::string("integer") ||
    // 	     schema_type == std::string("number")) {
    //   if (flag & kSetVarArgsFlag) {
    //     if (IsInt()) {
    // 	  BASE_STD_(Int, int)
    //     } else if (IsUint()) {
    // 	  BASE_STD_(Uint, unsigned)
    //     } else if (IsInt64()) {
    // 	  BASE_STD_(Int64, int64_t)
    //     } else if (IsUint64()) {
    // 	  BASE_STD_(Uint64, uint64_t)
    // 	} else if (schema_type == std::string("number") && IsDouble()) {
    // 	  BASE_STD_(Double, double)
    //     }
    //   } else if (schema_type == std::string("number")) {
    // 	BASE_STD_(Double, double)
    //   } else {
    // 	BASE_STD_(Int, int)
    //   }
    // }
    else if (schema_type == std::string("string")) {
      CASE_STRING_;
    }
    else if (schema_type == std::string("array")) {
      if (!(schema.HasMember("items") && (schema["items"].IsArray() ||
					  schema["items"].IsObject()))) {
	// ygglog_throw_error("ApplyVarArgs: schema for array must contain an items member");
	return false;
      }
      size_t nelements = 0;
      if (flag & kSetVarArgsFlag) {
	nelements = is_document_format_array(*this, true);
      } else {
	nelements = (size_t)is_schema_format_array(schema);
      }
      if (nelements) {
	if (!ap.apply(&nelements, flag))
	  return false;
      }
      SizeType total = 0;
      bool advance = false;
      if (flag == kSetVarArgsFlag) {
	if (!IsArray())
	  return false;
	total = Size();
	advance = true;
      } else {
	if (!schema["items"].IsArray())
	  return false;
	total = schema["items"].Size();
	if (flag == kGetVarArgsFlag) {
	  SetArray();
	  Reserve(total, parent->GetAllocator());
	  for (SizeType i = 0; i < total; i++) {
	    ValueType item;
	    PushBack(item, parent->GetAllocator());
	  }
	  advance = true;
	}
      }

      ValueIterator it;
      if (advance)
	it = Begin();
      else
	it = this;
      for (SizeType i = 0; i < total; i++) {
	if (schema["items"].IsArray()) {
	  if (!it->ApplyVarArgs(schema["items"][i], ap, flag,
				parent, nelements))
	    return false;
	} else {
	  if (!it->ApplyVarArgs(schema["items"], ap, flag,
				parent, nelements))
	    return false;
	}
	if (advance)
	  it++;
      }
    }
    else if (schema_type == std::string("object")) {
      if (!(schema.HasMember("properties") && schema["properties"].IsObject())) {
	// ygglog_throw_error("ApplyVarArgs: schema for object must contain a properties member");
	return false;
      }
      SizeType total = 0;
      bool advance = false;
      if (flag == kSetVarArgsFlag) {
	if (!IsObject())
	  return false;
	total = MemberCount();
	advance = true;
      } else {
	total = schema["properties"].MemberCount();
	if (flag == kGetVarArgsFlag) {
	  SetObject();
	  MemberReserve(total, parent->GetAllocator());
	  for (MemberIterator it = schema["properties"].MemberBegin();
	       it != schema["properties"].MemberEnd(); it++) {
	    ValueType item;
	    AddMember(ValueType(it->name, parent->GetAllocator()).Move(),
		      item, parent->GetAllocator());
	  }
	  advance = true;
	}
      }

      MemberIterator it;
      ValueType* name = NULL;
      ValueType* value = NULL;
      if (advance)
	it = MemberBegin();
      else
	it = schema["properties"].MemberBegin();
      for (SizeType i = 0; i < total; i++) {
	if (advance) {
	  name = &(it->name);
	  value = &(it->value);
	} else {
	  name = &(it->name);
	  value = this;
	}
	if (!value->ApplyVarArgs(schema["properties"][*name], ap,
				 flag, parent))
	  return false;
	// if (!it->value.ApplyVarArgs(schema["properties"][it->name], ap,
	// 			    flag, parent))
	//   return false;
	// if (advance)
	it++;
      }
    }
    else if (schema_type == std::string("scalar")) {
      if (!(schema.HasMember("subtype") && schema["subtype"].IsString()))  {
	// ygglog_throw_error("ApplyVarArgs: Scalar schema must contain a string subtype member");
	return false;
      }
      std::string schema_subtype(schema["subtype"].GetString());
      bool is_string = (schema_subtype == std::string("string") ||
			schema_subtype == std::string("bytes") ||
			schema_subtype == std::string("unicode"));
      int schema_precision = 0;
      if (schema.HasMember("precision") && schema["precision"].IsInt()) {
	if (!is_string)
	  schema_precision = schema["precision"].GetInt();
      } else if (!is_string) {
	// ygglog_throw_error("ApplyVarArgs: Scalar %s schema must contain an integer precision member", schema_subtype.c_str());
	return false;
      }
      int precision = schema_precision;
      std::string subtype = schema_subtype;
      if (flag == kSetVarArgsFlag) {
	precision = (int)(GetPrecision());
	subtype = std::string(GetSubType().GetString());
      }
#define CASE_SCALAR_(subT, prec, type)					\
      if (subtype == std::string(#subT) && prec == precision) {		\
	type tmp;							\
	if (flag == kSetVarArgsFlag) {					\
	  tmp = GetScalar<type>();					\
	}								\
	if (!ap.apply(&tmp, flag)) {					\
	  return false;							\
	}								\
	if (flag == kGetVarArgsFlag) {					\
	  ValueType schema_cpy(schema, parent->GetAllocator());		\
	  if (is_string && !schema_precision) {				\
	    schema_cpy.AddMember(GetPrecisionString(),			\
				 ValueType(precision).Move(),		\
				 parent->GetAllocator());		\
	  }								\
	  SetYggdrasilString((const char*)(&tmp),			\
			     static_cast<SizeType>(precision),		\
			     parent->GetAllocator(), schema_cpy);	\
	}								\
      }
      CASE_SCALAR_(int, 1, int8_t)
      else CASE_SCALAR_(int, 2, int16_t)
      else CASE_SCALAR_(int, 4, int32_t)
      else CASE_SCALAR_(int, 8, int64_t)
      else CASE_SCALAR_(uint, 1, uint8_t)
      else CASE_SCALAR_(uint, 2, uint16_t)
      else CASE_SCALAR_(uint, 4, uint32_t)
      else CASE_SCALAR_(uint, 8, uint64_t)
      else CASE_SCALAR_(float, 4, float)
      else CASE_SCALAR_(float, 8, double)
      else CASE_SCALAR_(complex, 8, std::complex<float>)
      else CASE_SCALAR_(complex, 16, std::complex<double>)
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
      else CASE_SCALAR_(float, 16, long double)
      else CASE_SCALAR_(complex, 32, std::complex<long double>)
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
      else if (is_string) {
	CASE_STRING_;
	// TODO: Encoding
	ValueType schema_cpy(schema, parent->GetAllocator());
	if (flag == kGetVarArgsFlag) {
	  if (!schema_precision) {
	    schema_cpy.AddMember(GetPrecisionString(),
				 ValueType(0).Move(),
				 parent->GetAllocator());
	  }
	  schema_cpy["precision"].SetUint64(mem_len);
	  SetValueSchema(schema_cpy, &(parent->GetAllocator()));
	}
      }
      else {
	// ygglog_throw_error("ApplyVarArgs: Unsupported subtype and precision combination for scalar: subtype = %s, precision = %d", schema_subtype.c_str(), schema_precision);
	return false;
      }
#undef CASE_SCALAR_
#undef CASE_STRING_
    }
    else if (schema_type == std::string("ndarray") ||
	     schema_type == std::string("1darray")) {
      if (!(schema.HasMember("subtype") && schema["subtype"].IsString())) {
	// ygglog_throw_error("ApplyVarArgs: ndarray schema must contain a string subtype member");
	return false;
      }
      size_t schema_ndim = 0;
      bool has_shape = false;
      size_t nelements = 1;
      std::string schema_subtype(schema["subtype"].GetString());
      size_t schema_precision = 0;
      bool is_string = (schema_subtype == std::string("string") ||
			schema_subtype == std::string("bytes") ||
			schema_subtype == std::string("unicode"));
      if (schema.HasMember("precision") && schema["precision"].IsUint()) {
	schema_precision = static_cast<size_t>(schema["precision"].GetUint());
      } else if (!is_string) {
	// ygglog_throw_error("ApplyVarArgs: ndarray schema must contain an integer precision member");
	return false;
      }
      if (schema_type == std::string("1darray")) {
	schema_ndim = 1;
      }
      // if (!ap.for_fortran) {
      if (schema.HasMember("length") && schema["length"].IsUint()) {
	schema_ndim = 1;
	has_shape = true;
	nelements *= static_cast<size_t>(schema["length"].GetUint());
      } else if (schema.HasMember("shape") && schema["shape"].IsArray()) {
	schema_ndim = static_cast<size_t>(schema["shape"].Size());
	has_shape = true;
	for (ValueIterator it = schema["shape"].Begin();
	     it != schema["shape"].End(); it++) {
	  nelements *= static_cast<size_t>(it->GetUint());
	}
      }
      // }
      if (schema_ndim == 0 && schema.HasMember("ndim") && schema["ndim"].IsInt())
	schema_ndim = static_cast<size_t>(schema["ndim"].GetUint());
      if (table_nelements) {
	nelements = table_nelements;
      }
      // if (schema_ndim == 0)
      //   ygglog_throw_error("ApplyVarArgs: Could not determine the number of dimension in the array from the schema");
      std::string subtype = schema_subtype;
      size_t precision = schema_precision;
      size_t ndim = schema_ndim;
      if (flag == kSetVarArgsFlag) {
	subtype = std::string(GetSubType().GetString());
	precision = static_cast<size_t>(GetPrecision());
	if (Is1DArray()) {
	  ndim = 1;
	} else {
	  ndim = static_cast<size_t>(GetShape().Size());
	}
	nelements = static_cast<size_t>(GetNElements());
      }
      char* mem = NULL;
      char** mem_ref = NULL;
      size_t mem_len = 0;
      size_t mem_ndim = 0;
      size_t mem_prec = 0;
      size_t* mem_shape = NULL;
      size_t** mem_shape_ref = NULL;
      bool exchange_size = ((!table_nelements) &&
			    (ap.for_fortran || !has_shape));
      // bool exchange_size = (!((has_shape && !ap.for_fortran) ||
      // 			      table_nelements));
      bool exchange_len = (exchange_size && (ndim == 1));
      bool exchange_shape = (exchange_size && (ndim > 1));
      bool exchange_prec = (is_string && (ap.for_fortran ||
					  !table_nelements));
      mem_len = nelements;
      mem_ndim = ndim;
      mem_prec = precision;
      if (!ap.apply_mem(mem, mem_ref, flag))
	return false;
      if (exchange_len) {
	if (!ap.apply(&mem_len, flag, true))
	  return false;
	mem_shape = &mem_len;
      } else if (exchange_shape) {
	if (!ap.apply(&mem_ndim, flag, true))
	  return false;
	if (!ap.apply_mem(mem_shape, mem_shape_ref, flag))
	  return false;
      }
      if (exchange_prec) {
	if (!ap.apply(&mem_prec, flag, true))
	  return false;
      }
      if (flag == kSetVarArgsFlag) {
	// TODO: Check that shape matches in case of not allow_realloc
	if (exchange_shape) {
	  size_t i = 0;
	  size_t src_shape_len = ndim;
	  size_t* src_shape = (size_t*)malloc(src_shape_len * sizeof(size_t));
	  if (Is1DArray()) {
	    src_shape[0] = static_cast<size_t>(GetNElements());
	  } else {
	    for (ConstValueIterator it = GetShape().Begin();
		 it != GetShape().End(); it++, i++) {
	      src_shape[i] = static_cast<size_t>(it->GetUint());
	    }
	  }
	  if (!ap.set_mem(mem_shape, mem_shape_ref, mem_ndim,
			  src_shape, src_shape_len))
	    return false;
	  free(src_shape);
	}
	const char* tmp = GetString();
	// TODO: Handle element by element terminating characters
	if (is_string) {
	  size_t tmp_len = nelements;
	  size_t tmp_prec = precision;
	  if (!ap.set_mem_strided(mem, mem_ref, mem_len, mem_prec,
				  tmp, tmp_len, tmp_prec))
	    return false;
	} else {
	  size_t tmp_nbytes = static_cast<size_t>(GetNBytes());
	  size_t mem_nbytes = mem_len * mem_prec;
	  if (!ap.set_mem(mem, mem_ref, mem_nbytes, tmp, tmp_nbytes))
	    return false;
	}
      } else if (flag == kGetVarArgsFlag) {
	if (exchange_shape) {
	  if (ap.for_fortran && !mem_shape) {
	    mem_len = 0;
	  } else {
	    mem_len = 1;
	    for (size_t i = 0; i < mem_ndim; i++)
	      mem_len *= mem_shape[i];
	  }
	}
	ValueType schema_cpy(schema, parent->GetAllocator());
	if (exchange_prec) {
	  if (!schema_precision)
	    schema_cpy.AddMember(GetPrecisionString(),
				 ValueType(0).Move(),
				 parent->GetAllocator());
	  schema_cpy["precision"].SetUint64(mem_prec);
	}
	if (exchange_size) {
	  if (!has_shape) {
	    schema_cpy.AddMember(GetShapeString(),
				 ValueType(kArrayType).Move(),
				 parent->GetAllocator());
	    schema_cpy["shape"].Reserve((SizeType)mem_ndim,
					parent->GetAllocator());
	    for (size_t i = 0; i < mem_ndim; i++)
	      schema_cpy["shape"].PushBack(0, parent->GetAllocator());
	  } else {
	    RAPIDJSON_ASSERT(schema_ndim == mem_ndim);
	    if (schema_ndim != mem_ndim)
	      return false;
	  }
	  RAPIDJSON_ASSERT(mem_shape);
	  if (!mem_shape)
	    return false;
	  for (size_t i = 0; i < mem_ndim; i++)
	    schema_cpy["shape"][static_cast<SizeType>(i)].SetUint64(mem_shape[i]);
	}
	SetYggdrasilString(mem,
			   static_cast<SizeType>(mem_prec * mem_len),
			   parent->GetAllocator(), schema_cpy);
      }

      /*
#define CASE_NDARRAY_(subT, prec, type)					\
      if (subtype == std::string(subT) && prec == precision) {		\
	type* dst = (type*)mem;						\
	type** dst_ref = (type**)mem_ref;				\
	if (flag == kSetVarArgsFlag) {					\
	  type* src = (type*)GetString();				\
	  size_t src_nbytes = static_cast<size_t>(GetNBytes());		\
	  if (!ap.set_mem(dst, dst_ref, mem_len,			\
			  src, src_nbytes / sizeof(type))) {		\
	    return false;						\
	  }								\
	} else if (flag == kGetVarArgsFlag) {				\
	  SetYggdrasilString(mem, mem_prec * mem_len,			\
			     parent->GetAllocator(), schema_cpy);	\
	}								\
      }
      CASE_NDARRAY_(int, 1, int8_t)
      else CASE_NDARRAY_(int, 2, int16_t)
      else CASE_NDARRAY_(int, 4, int32_t)
      else CASE_NDARRAY_(int, 8, int64_t)
      else CASE_NDARRAY_(uint, 1, uint8_t)
      else CASE_NDARRAY_(uint, 2, uint16_t)
      else CASE_NDARRAY_(uint, 4, uint32_t)
      else CASE_NDARRAY_(uint, 8, uint64_t)
      else CASE_NDARRAY_(float, 4, float)
      else CASE_NDARRAY_(float, 8, double)
      else CASE_NDARRAY_(complex, 8, complex_float_t)
      else CASE_NDARRAY_(complex, 16, complex_double_t)
      else CASE_NDARRAY_(string, precision, char)
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
      else CASE_NDARRAY_(float, 16, long double)
      else CASE_NDARRAY_(complex, 32, complex_long_double_t)
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
      else {
	// ygglog_throw_error("ApplyVarArgs: Unsupported subtype and precision combination for ndarray: subtype = %s, precision = %d", schema_subtype.c_str(), schema_precision);
	return false;
      }
#undef CASE_NDARRAY_
      */
    }
    // TODO: Handle wrapping on C side as python_t
#define CASE_PYTHON_(str)						\
    if (schema_type == Get ## str ## String()) {			\
      PyObject* tmp = NULL;						\
      if (flag == kSetVarArgsFlag) {					\
	tmp = GetPythonObjectRaw();					\
      }									\
      if (!ap.apply(&tmp, flag)) {					\
	return false;							\
      }									\
      if (flag == kGetVarArgsFlag) {					\
	SetPythonObjectRaw(tmp);					\
      }									\
    }
    else CASE_PYTHON_(PythonClass)
    else CASE_PYTHON_(PythonFunction)
    else CASE_PYTHON_(PythonInstance)
#undef CASE_PYTHON_
    // TODO: Handle wrapping on C side as obj_t & ply_t
#define CASE_GEOMETRY_(str)					\
    if (schema_type == Get ## str ## String()) {		\
      str* tmp = NULL;						\
      str** tmp_ref = NULL;					\
      if (!ap.apply_mem(tmp, tmp_ref, flag)) {			\
	return false;						\
      }								\
      if (flag == kSetVarArgsFlag) {				\
	if (tmp == NULL) {					\
	  if (!ap.allow_realloc)				\
	    return false;					\
	  tmp = new str();					\
	  tmp_ref[0] = tmp;					\
	}							\
	Get ## str(*tmp);					\
      } else if (flag == kGetVarArgsFlag) {			\
	Set ## str(*tmp);					\
      }								\
    }
    else CASE_GEOMETRY_(ObjWavefront)
    else CASE_GEOMETRY_(Ply)
#undef CASE_GEOMETRY_
    else {
      // ygglog_throw_error("ApplyVarArgs: Unsupported type %s, stored as a string", schema_type.GetString());
      return false;
    }
#undef BASE_STD_
#undef BASE_
    return true;
  }
  
  template <typename T>
  bool SetNDArrayRaw(const units::GenericQuantityArray<T, EncodingType>* x,
		     Allocator* allocator = 0,
		     RAPIDJSON_DISABLEIF((internal::IsPointer<T>))) {
    std::basic_string<Ch> units_str = x->unitsStr();
    bool out = false;
    if (units_str.size() > 0)
      out = SetNDArrayRaw(x->value(), x->shape(), x->ndim(),
			  units_str.c_str(),
			  static_cast<SizeType>(units_str.size()),
			  allocator);
    else
      out = SetNDArrayRaw(x->value(), x->shape(), x->ndim(),
			  nullptr, 0, allocator);
    return out;
  }
  template <typename T>
  bool SetNDArrayRaw(const units::GenericQuantity<T, EncodingType>* x,
		     const SizeType* shape, const SizeType ndim,
		     Allocator* allocator = 0,
		     RAPIDJSON_DISABLEIF((internal::IsPointer<T>))) {
    SizeType nelements = 1;
    for (SizeType i = 0; i < ndim; i++)
      nelements = nelements * shape[i];
    RAPIDJSON_ASSERT(nelements > 0);
    if (nelements == 0)
      return false;
    std::basic_string<Ch> units_str = x[0].unitsStr();
    T* temp = (T*)malloc(nelements * sizeof(T));
    RAPIDJSON_ASSERT(temp);
    if (!temp)
      return false;
    for (SizeType i = 0; i < nelements; i++)
      temp[i] = x[i].value();
    bool out = SetNDArrayRaw(temp, shape, ndim,
			     units_str.c_str(),
			     static_cast<SizeType>(units_str.size()),
			     allocator);
    free(temp);
    temp = NULL;
    return out;
  }
  template <typename T>
  bool SetNDArrayRaw(const T* data,
		     const SizeType* shape, const SizeType& ndim,
		     const Ch* units_str=nullptr, const SizeType units_len=0,
		     Allocator* allocator = 0,
		     enum YggSubType subtype = kYggNullSubType,
		     SizeType precision = 0,
		     const Ch* encoding_str = nullptr,
		     const SizeType encoding_len = 0,
		     RAPIDJSON_DISABLEIF((internal::IsPointer<T>))) {
    ResetSchema(allocator);
    SizeType nbytes = 0;
    if (precision == 0)
      precision = (SizeType)sizeof(T);
    nbytes = precision;
    ValueType shape_array(kArrayType);
    for (SizeType i = 0; i < ndim; i++) {
      nbytes = nbytes * shape[i];
      shape_array.PushBack(shape[i], schema_->GetAllocator());
    }
    SetStringRaw(StringRef(reinterpret_cast<const Ch*>(data),
			   nbytes / sizeof(Ch)),
		 schema_->GetAllocator());
    // Update schema
    schema_->MemberReserve(5, schema_->GetAllocator());
    if (ndim == 0) {
      AddSchemaMember(GetTypeString(), GetScalarString());
    } else {
      AddSchemaMember(GetTypeString(), GetNDArrayString());
    }
    if (subtype == kYggNullSubType) {
      AddSchemaMember(GetSubTypeString(), YggSubTypeString<T>());
    } else {
      AddSchemaMember(GetSubTypeString(), YggSubTypeString(subtype));
    }
    AddSchemaMember(GetPrecisionString(), static_cast<unsigned int>(precision));
    if (units_str) {
      if (!SetUnits(units_str, units_len)) return false;
    }
    if (ndim > 0) {
      AddSchemaMember(GetShapeString(), shape_array);
    }
    if (encoding_len > 0) {
      AddSchemaMember(GetEncodingString(), ValueType(encoding_str, encoding_len, schema_->GetAllocator()).Move());
    }
    return true;
  }
#ifdef YGGDRASIL_DISABLE_PYTHON_C_API
  PyObject* GetPythonObjectRaw() const {
    RAPIDJSON_ASSERT(isPythonInitialized());
    return NULL;
  }
  bool SetPythonObjectRaw(PyObject* x, Allocator* allocator = 0,
#ifdef RAPIDJSON_DONT_IMPORT_NUMPY
			  bool=false,
#else // RAPIDJSON_DONT_IMPORT_NUMPY
			  bool skipTitle=false,
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
			  bool allowPickle=true) {
    RAPIDJSON_ASSERT(isPythonInitialized());
    return false;
  }
#else // YGGDRASIL_DISABLE_PYTHON_C_API
  bool GetPythonObjectClassName(PyObject* x, Ch*& mod_cls, SizeType& mod_cls_siz,
				Allocator& allocator) {
    RAPIDJSON_ASSERT(PyObject_HasAttrString(x, "__module__"));
    RAPIDJSON_ASSERT(PyObject_HasAttrString(x, "__name__"));
    if (!(PyObject_HasAttrString(x, "__module__") && PyObject_HasAttrString(x, "__name__")))
      return false;
    PyObject *mod_py = PyObject_GetAttrString(x, "__module__");
    // This version uses the full path, but it will not be portable
    // PyObject* inspect = PyImport_ImportModule("inspect");
    // if (inspect == NULL)
    //   return false;
    // PyObject* inspect_getfile = PyObject_GetAttrString(inspect, "getfile");
    // Py_DECREF(inspect);
    // if (inspect_getfile == NULL)
    //   return false;
    // PyObject* mod_py = PyObject_CallFunction(inspect_getfile, "(O)", x);
    // Py_DECREF(inspect_getfile);
    // if (mod_py == NULL) {
    //   PyErr_Clear();
    //   mod_py = PyObject_GetAttrString(x, "__module__");
    // }
    RAPIDJSON_ASSERT(mod_py != NULL);
    if (mod_py == NULL)
      return false;
    PyObject *cls_py = PyObject_GetAttrString(x, "__name__");
    RAPIDJSON_ASSERT(cls_py != NULL);
    if (cls_py == NULL) {
      Py_DECREF(mod_py);
      return false;
    }
    SizeType mod_len = 0;
    SizeType cls_len = 0;
    Ch* mod = PyUnicode_AsEncoding<Encoding,Allocator>(mod_py, mod_len, allocator);
    Ch* cls = PyUnicode_AsEncoding<Encoding,Allocator>(cls_py, cls_len, allocator);
    Py_DECREF(mod_py);
    Py_DECREF(cls_py);
    RAPIDJSON_ASSERT((mod != NULL) && (cls != NULL));
    if ((mod == NULL) || (cls == NULL))
      return false;
    mod_cls_siz = mod_len + cls_len + 1;
    mod_cls = static_cast<Ch*>(allocator.Malloc((mod_cls_siz + 1) * sizeof(Ch)));
    RAPIDJSON_ASSERT(mod_cls);
    if (!mod_cls)
      return false;
    memcpy(mod_cls, mod, mod_len * sizeof(Ch));
    mod_cls[mod_len] = (Ch)(':');
    memcpy(mod_cls + mod_len + 1, cls, cls_len);
    mod_cls[mod_len + cls_len + 1] = '\0';
    allocator.Free(mod);
    allocator.Free(cls);
    mod = NULL;
    cls = NULL;
    return true;
  }
  bool GetPythonObjectClassAttr(PyObject* x, const char* attr,
                                Allocator& allocator, ValueType& out,
				bool call_function = false) {
    if (!attr || !PyObject_HasAttrString(x, attr))
      return false;
    PyObject* x_attr = PyObject_GetAttrString(x, attr);
    RAPIDJSON_ASSERT(x_attr != NULL);
    if (x_attr == NULL)
      return false;
    if (call_function && PyCallable_Check(x_attr)) {
      PyObject* py_args = PyTuple_New(0);
      PyObject* x_attr_res = PyObject_Call(x_attr, py_args, NULL);
      Py_DECREF(py_args);
      Py_DECREF(x_attr);
      if (x_attr_res == NULL) {
	return false;
      }
      x_attr = x_attr_res;
    }
    bool ret = out.SetPythonObjectRaw(x_attr, &allocator);
    Py_DECREF(x_attr);
    return ret;
  }
  PyObject* GetPythonObjectRaw() const {
    RAPIDJSON_ASSERT(isPythonInitialized());
    if (!isPythonInitialized())
      return NULL;
    PyObject* out = NULL;
    switch (GetType()) {
    case kNullType: {
      Py_INCREF(Py_None);
      return Py_None;
    }
    case kFalseType: {
      Py_INCREF(Py_False);
      return Py_False;
    }
    case kTrueType: {
      Py_INCREF(Py_True);
      return Py_True;
    }
    case kObjectType: {
      if (IsPythonInstance())
	return GetPythonInstance();
      out = PyDict_New();
      RAPIDJSON_ASSERT(out);
      ConstMemberIterator item;
      int result = 0;
      for (item = MemberBegin(); item != MemberEnd(); item++) {
	const char* ikey = item->name.GetString();
	PyObject* ival = item->value.GetPythonObjectRaw();
	result = PyDict_SetItemString(out, ikey, ival);
	RAPIDJSON_ASSERT(result == 0);
	Py_DECREF(ival);
      }
      (void)result;
      return out;
    }
    case kArrayType: {
      Py_ssize_t len = 0;
      out = PyList_New(len);
      if (out == NULL)
	return NULL;
      ConstValueIterator item;
      for (item = Begin(); item != End(); item++) {
	PyObject* ival = item->GetPythonObjectRaw();
	if (ival == NULL) {
	  Py_DECREF(out);
	  return NULL;
	}
	if (PyList_Append(out, ival) < 0) {
	  Py_DECREF(out);
	  Py_DECREF(ival);
	  return NULL;
	}
	Py_DECREF(ival);
      }
      if (IsStructuredArray(out)) {
	PyObject* tmp = GetStructuredArray(out);
	Py_DECREF(out);
	return tmp;
      }
      return out;
    }
    case kStringType: {
      if (IsPythonClass())
	return GetPythonClass();
      else if (IsPythonFunction())
	return GetPythonFunction();
      else if (IsPythonInstance())
	return GetPythonInstance();
#ifndef RAPIDJSON_DONT_IMPORT_NUMPY
      else if (IsScalar()) {
	ValueType enc;
	int typenum = GetSubTypeNumpyType(enc);
	if (typenum < 0)
	  return NULL;
	if (typenum == NPY_STRING) {
	  return PyBytes_FromStringAndSize(GetString(), GetStringLength());
	} else if (typenum == NPY_UNICODE) {
	  PyObject* pyBytes = PyBytes_FromStringAndSize(GetString(), GetStringLength());
	  out = PyUnicode_FromEncodedObject(pyBytes, enc.GetString(), NULL);
	  Py_DECREF(pyBytes);
	  return out;
	}
	PyArray_Descr* desc = PyArray_DescrNewFromType(typenum);
	if (desc == NULL) return NULL;
	if (PyTypeNum_ISFLEXIBLE(typenum))
	  desc->elsize = static_cast<int>(GetPrecision());
	out = PyArray_Scalar((void*)GetString(), desc, NULL);
	// if (out != NULL) {
	//   Py_INCREF(desc);
	//   out = PyArray_FromScalar(out, desc);
	// }
	return out;
      } else if (IsNDArray()) {
	ValueType enc;
	int typenum = GetSubTypeNumpyType(enc);
	if (typenum < 0) return NULL;
	PyArray_Descr* desc = PyArray_DescrNewFromType(typenum);
	if (desc == NULL) return NULL;
	if (PyTypeNum_ISFLEXIBLE(typenum))
	  desc->elsize = static_cast<int>(GetPrecision());
	if (HasTitle()) {
	  const ValueType& title = GetTitle();
	  PyObject* titlePy = PyUnicode_FromStringAndSize(title.GetString(),
							  title.GetStringLength());
	  PyArray_Descr* sub_desc = desc;
	  desc = PyArray_DescrNewFromType(NPY_VOID);
	  if (desc == NULL) return NULL;
	  desc->names = PyTuple_Pack(1, titlePy);
	  desc->fields = PyDict_New();
	  desc->elsize = sub_desc->elsize;
	  PyObject* offset = PyLong_FromSsize_t(0);
	  PyObject* sub_dtype = PyTuple_Pack(2, sub_desc, offset);
	  Py_DECREF(sub_desc);
	  Py_DECREF(offset);
	  PyDict_SetItem(desc->fields, titlePy, sub_dtype);
	  Py_DECREF(titlePy);
	  Py_DECREF(sub_dtype);
	}
	SizeType ndim = 0;
	SizeType* shape = GetShape(ndim, schema_->GetAllocator());
	if (!shape) {
	  Py_DECREF(desc);
	  return NULL;
	}
	npy_intp* np_shape = (npy_intp*)(schema_->GetAllocator().Malloc(sizeof(npy_intp) * ndim));
	if (!np_shape) {
	  schema_->GetAllocator().Free(shape);
	  shape = NULL;
	  Py_DECREF(desc);
	  return NULL;
	}
	for (SizeType i = 0; i < ndim; i++)
	  np_shape[i] = (npy_intp)shape[i];
	schema_->GetAllocator().Free(shape);
	shape = NULL;
	// don't use allocator so that python array is responsible for freeing
	bool free_data = false;
	void* data = (void*)GetString();
	if (typenum == NPY_UNICODE && (!(enc == GetUCS4EncodingString() ||
					 enc == GetUTF32EncodingString()))) {
	  void* tmp = 0;
	  SizeType tmp_nbytes = 0;
	  if (!TranslateEncoding(GetString(), GetStringLength() * (SizeType)sizeof(Ch), enc.GetString(),
				 tmp, tmp_nbytes, GetUTF32EncodingString().GetString(),
				 schema_->GetAllocator(), true)) {
	    Py_DECREF(desc);
	    return NULL;
	  }
	  std::cerr << "CHANGING THE ENCODING" << std::endl;
	  free_data = true;
	  data = tmp;
	  desc->elsize = static_cast<int>(tmp_nbytes / GetNElements());
	}
	PyObject* tmp = PyArray_NewFromDescr(&PyArray_Type, desc,
					     (int)ndim, np_shape,
					     NULL, data, 0, NULL);
	if (tmp) {
	  out = PyArray_NewCopy((PyArrayObject*)tmp, NPY_CORDER);
	  Py_DECREF(tmp);
	}
	if (free_data) {
	  schema_->GetAllocator().Free(data);
	  data = NULL;
	}
	schema_->GetAllocator().Free(np_shape);
	np_shape = NULL;
	return out;
      }
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
      return PyUnicode_FromStringAndSize(GetString(), GetStringLength());
    }
    case kNumberType: {
      if (IsDouble()) {
	out = PyFloat_FromDouble(GetDouble());
      } else if (IsUint()) {
	out = PyLong_FromUnsignedLongLong(GetUint64());
      } else {
	out = PyLong_FromLongLong(GetInt64());
      }
      return out;
    }
    default:
      RAPIDJSON_ASSERT(!GetType());
    }
    return out;
  }
  bool SetPythonObjectRaw(PyObject* x, Allocator* allocator = 0,
#ifdef RAPIDJSON_DONT_IMPORT_NUMPY
			  bool=false,
#else // RAPIDJSON_DONT_IMPORT_NUMPY
			  bool skipTitle=false,
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
			  bool allowPickle=true) {
    RAPIDJSON_ASSERT(isPythonInitialized());
    if (!isPythonInitialized())
      return false;
    bool out = true;
    if (x == NULL)
      return false;
    else if (PyList_CheckExact(x)) {
      RAPIDJSON_ASSERT(allocator);
      if (!allocator)
	return false;
      SetArray();
      for (SizeType i = 0; i < PyList_Size(x); i++) {
        PushBack(ValueType(PyList_GetItem(x, i), *allocator).Move(), *allocator);
      }
    } else if (PyTuple_CheckExact(x)) {
      RAPIDJSON_ASSERT(allocator);
      if (!allocator)
	return false;
      SetArray();
      for (SizeType i = 0; i < PyTuple_Size(x); i++) {
        PushBack(ValueType(PyTuple_GetItem(x, i), *allocator).Move(), *allocator);
      }
    } else if (PyDict_CheckExact(x)) {
      RAPIDJSON_ASSERT(allocator);
      if (!allocator)
	return false;
      SetObject();
      PyObject* keys = PyDict_Keys(x);
      PyObject* ikey;
      for (SizeType i = 0; i < PyDict_Size(x); i++) {
        ikey = PyList_GetItem(keys, i);
        AddMember(ValueType(ikey, *allocator),
                  ValueType(PyDict_GetItem(x, ikey), *allocator).Move(),
                  *allocator);
      }
      Py_DECREF(keys);
    } else if (PyUnicode_Check(x)) {
      RAPIDJSON_ASSERT(allocator);
      if (!allocator)
	return false;
      Py_ssize_t x_size = 0;
      const char* x_bytes = PyUnicode_AsUTF8AndSize(x, &x_size);
      SetStringRaw(StringRef(x_bytes, (size_t)x_size), *allocator);
    } else if (PyBytes_Check(x) || PyByteArray_Check(x)) {
      RAPIDJSON_ASSERT(allocator);
      if (!allocator)
	return false;
      ResetSchema(allocator);
      Py_ssize_t x_size = 0;
      if (PyBytes_Check(x)) {
	x_size = PyBytes_Size(x);
	SetStringRaw(StringRef(PyBytes_AsString(x), (size_t)x_size),
		     *allocator);
      } else {
	x_size = PyByteArray_Size(x);
	SetStringRaw(StringRef(PyByteArray_AsString(x), (size_t)x_size),
		     *allocator);
      }
      schema_->MemberReserve(5, schema_->GetAllocator());
      AddSchemaMember(GetTypeString(), GetScalarString());
      AddSchemaMember(GetSubTypeString(), GetStringSubTypeString());
      AddSchemaMember(GetPrecisionString(), GetStringLength());
    } else if (PyLong_Check(x)) {
      int overflow = 0;
      Set(static_cast<int64_t>(PyLong_AsLongLongAndOverflow(x, &overflow)));
      RAPIDJSON_ASSERT(overflow == 0);
      RAPIDJSON_ASSERT(PyErr_Occurred() == NULL);
    } else if (PyFloat_Check(x)) {
      Set(PyFloat_AsDouble(x));
      RAPIDJSON_ASSERT(PyErr_Occurred() == NULL);
    } else if (PyComplex_Check(x)) {
      Set(std::complex<double>(PyComplex_RealAsDouble(x),
                               PyComplex_ImagAsDouble(x)));
      RAPIDJSON_ASSERT(PyErr_Occurred() == NULL);
    } else if (x == Py_False) {
      SetBool(false);
    } else if (x == Py_True) {
      SetBool(true);
    } else if (x == Py_None) {
      SetNull();
    } else if (PyType_Check(x) || PyFunction_Check(x)) {
      SetString("");
      ResetSchema(allocator);
      Ch* mod_cls = NULL;
      SizeType mod_cls_siz = 0;
      out = GetPythonObjectClassName(x, mod_cls, mod_cls_siz,
				     schema_->GetAllocator());
      RAPIDJSON_ASSERT(out && (mod_cls != NULL));
      if (!out || (mod_cls == NULL))
	return out;
      if (PyType_Check(x))
	AddSchemaMember(GetTypeString(), GetPythonClassString());
      else
	AddSchemaMember(GetTypeString(), GetPythonFunctionString());
      SetStringRaw(StringRefType(mod_cls, mod_cls_siz), schema_->GetAllocator());
      schema_->GetAllocator().Free(mod_cls);
      mod_cls = NULL;
#ifndef RAPIDJSON_DONT_IMPORT_NUMPY
    } else if (PyArray_CheckScalar(x)) {
      ResetSchema(allocator);
      PyArray_Descr* desc = NULL;
      PyObject* scalar = NULL;
      if (PyObject_IsInstance(x, (PyObject*)&PyArray_Type)) {
	desc = PyArray_DESCR((PyArrayObject*)x);
	Py_INCREF(desc);
	scalar = PyArray_Scalar(PyArray_DATA((PyArrayObject*)x), desc, NULL);
      } else {
	desc = PyArray_DescrFromScalar(x);
	Py_INCREF(x);
	scalar = x;
      }
      if (desc == NULL || scalar == NULL) {
	Py_XDECREF(desc);
	Py_XDECREF(scalar);
	return false;
      }
      SizeType precision;
      ValueType subtype;
      ValueType encoding;
      if (!NumpyType2SubType(desc, subtype, precision, encoding, 0,
			     schema_->GetAllocator())) {
	Py_DECREF(desc);
	Py_DECREF(scalar);
	return false;
      }
      void* data = schema_->GetAllocator().Malloc(precision);
      if (!data) {
	Py_DECREF(desc);
	return false;
      }
      PyArray_CastScalarToCtype(scalar, data, desc);
      Py_DECREF(scalar);
      Py_INCREF(desc);
      SetStringRaw(StringRef(static_cast<Ch*>(data), precision / sizeof(Ch)),
		   schema_->GetAllocator());
      schema_->GetAllocator().Free(data);
      data = NULL;
      if (desc->type_num == NPY_UNICODE && encoding == GetUTF8EncodingString()) {
	Py_DECREF(desc);
	return true;
      }
      Py_DECREF(desc);
      schema_->MemberReserve(5, schema_->GetAllocator());
      AddSchemaMember(GetTypeString(), GetScalarString());
      AddSchemaMember(GetSubTypeString(), subtype);
      AddSchemaMember(GetPrecisionString(), precision);
      if (!encoding.IsNull())
	AddSchemaMember(GetEncodingString(), encoding);
      return true;
    } else if (PyArray_Check(x)) {
      PyArray_Descr* desc = PyArray_DESCR((PyArrayObject*)x);
      if (desc == NULL)
	return false;
      if (PyDataType_HASFIELDS(desc)) {
	bool single = (PyDict_Size(desc->fields) == 1);
	if (!single) {
	  SetArray();
	  ResetSchema(allocator);
	  Reserve((SizeType)PyDict_Size(desc->fields), schema_->GetAllocator());
	}
	PyObject *kw_key, *kw_val;
	Py_ssize_t kw_pos = 0;
	while (PyDict_Next(desc->fields, &kw_pos, &kw_key, &kw_val)) {
	  PyObject* dtype = PyTuple_GetItem(kw_val, 0);
	  if (dtype == NULL)
	    return false;
	  PyObject* offsetObj = PyTuple_GetItem(kw_val, 1);
	  if (offsetObj == NULL)
	    return false;
	  Py_ssize_t offset = PyNumber_AsSsize_t(offsetObj, NULL);
	  if (offset < 0)
	    return false;
	  Ch* kw_keyS = NULL;
	  Py_ssize_t kw_keyS_len = 0;
	  if (sizeof(Ch) == sizeof(wchar_t))
	    kw_keyS = (Ch*)PyUnicode_AsWideCharString(kw_key, &kw_keyS_len);
	  else
	    kw_keyS = (Ch*)PyUnicode_AsUTF8AndSize(kw_key, &kw_keyS_len);
	  if (kw_keyS == NULL)
	    return false;
	  Py_INCREF(dtype);
	  PyObject* field = PyArray_GetField((PyArrayObject*)x,
					     (PyArray_Descr*)dtype,
					     (int)offset);
	  if (field == NULL)
	    return false;
	  if (single) {
	    if (!SetPythonObjectRaw(field))
	      return false;
	    if (!skipTitle) {
	      ValueType field_name(kw_keyS,
				   static_cast<SizeType>(kw_keyS_len),
				   schema_->GetAllocator());
	      AddSchemaMember(GetTitleString(), field_name);
	    }
	  } else {
	    ValueType pyField(field);
	    Py_DECREF(field);
	    if (!skipTitle) {
	      ValueType field_name(kw_keyS,
				   static_cast<SizeType>(kw_keyS_len),
				   schema_->GetAllocator());
	      pyField.AddSchemaMember(GetTitleString(), field_name);
	    }
	    PushBack(pyField, schema_->GetAllocator());
	  }
	}
	return true;
      }
      ResetSchema(allocator);
      SizeType precision;
      ValueType subtype;
      ValueType encoding;
      if (!NumpyType2SubType(desc, subtype, precision, encoding,
			     (SizeType)PyArray_ITEMSIZE((PyArrayObject*)x),
			     schema_->GetAllocator()))
	return false;
      SizeType nelements = (SizeType)PyArray_SIZE((PyArrayObject*)x);
      ValueType shape(kArrayType);
      int ndim = PyArray_NDIM((PyArrayObject*)x);
      npy_intp* np_shape = PyArray_SHAPE((PyArrayObject*)x);
      if (np_shape == NULL)
	return false;
      for (int i = 0; i < ndim; i++)
	shape.PushBack((SizeType)np_shape[i], schema_->GetAllocator());
      PyArrayObject* cpy = PyArray_GETCONTIGUOUS((PyArrayObject*)x);
      if (cpy == NULL)
	return false;
      void* data = (void*)PyArray_BYTES(cpy);
      if (data == NULL) {
	if (!PyArray_IS_C_CONTIGUOUS((PyArrayObject*)x))
	  Py_DECREF(cpy);
	return false;
      }
      SetStringRaw(StringRef(static_cast<Ch*>(data),
			     precision * nelements / sizeof(Ch)),
		   schema_->GetAllocator());
      if (!PyArray_IS_C_CONTIGUOUS((PyArrayObject*)x))
	Py_DECREF(cpy);
      schema_->MemberReserve(5, schema_->GetAllocator());
      AddSchemaMember(GetTypeString(), GetNDArrayString());
      AddSchemaMember(GetSubTypeString(), subtype);
      AddSchemaMember(GetPrecisionString(), precision);
      AddSchemaMember(GetShapeString(), shape);
      if (!encoding.IsNull())
	AddSchemaMember(GetEncodingString(), encoding);
      return true;
    } else if (PyObject_IsInstanceString(x, "pandas.core.frame.DataFrame")) {
      bool error = false;
      PyObject *column_dtypes = NULL, *columns = NULL, *dtypes = NULL,
	*key = NULL, *itype = NULL, *val = NULL, *skipTitleObject = NULL,
	*vtype = NULL, *vv = NULL;
      PyObject *method = NULL, *args = NULL, *kwargs = NULL, *arr = NULL;
      PyObject* new_itype = NULL;
      std::string new_itype_str;
      column_dtypes = PyDict_New();
      if (column_dtypes == NULL) {
	error = true;
	goto cleanup;
      }
      columns = PyObject_GetAttrString(x, "columns");
      if (columns == NULL || !PySequence_Check(columns)) {
	error = true;
	goto cleanup;
      }
      skipTitleObject = PyObject_CallMethod(columns, "is_integer", NULL);
      if (skipTitleObject == NULL) {
	error = true;
	goto cleanup;
      }
      if (skipTitleObject == Py_True)
	skipTitle = true;
      Py_DECREF(skipTitleObject);
      dtypes = PyObject_GetAttrString(x, "dtypes");
      if (dtypes == NULL || !PyMapping_Check(dtypes)) {
	error = true;
	goto cleanup;
      }
      for (Py_ssize_t i = 0; i < PySequence_Size(columns); i++) {
	key = PySequence_GetItem(columns, i);
	if (key == NULL) {
	  error = true;
	  goto cleanup;
	}
	itype = PyObject_GetItem(dtypes, key);
	if (itype == NULL) {
	  error = true;
	  goto cleanup;
	}
	if (((PyArray_Descr*)itype)->type_num == NPY_OBJECT) {
	  val = PyObject_GetItem(x, key);
	  if (val == NULL) {
	    error = true;
	    goto cleanup;
	  }
	  Py_ssize_t max_len = 0;
	  for (Py_ssize_t j = 0; j < PyObject_Size(val); j++) {
	    vv = PySequence_GetItem(val, j);
	    if (vv == NULL) {
	      error = true;
	      goto cleanup;
	    }
	    if (vtype == NULL) {
	      vtype = PyObject_Type(vv);
	    } else if (!PyObject_IsInstance(vv, vtype)) {
	      error = true;
	      goto cleanup;
	    }
	    Py_ssize_t new_len = 0;
	    if (vtype == (PyObject*)(&PyUnicode_Type)) {
	      new_len = PyUnicode_GET_LENGTH(vv);
	    } else {
	      new_len = PyObject_Size(vv);
	    }
	    Py_DECREF(vv);
	    vv = NULL;
	    if (new_len > max_len)
	      max_len = new_len;
	  }
	  new_itype_str = "";
	  if (vtype == (PyObject*)(&PyUnicode_Type)) {
	    new_itype_str += "<U";
	  } else if (vtype == (PyObject*)(&PyBytes_Type)) {
	    new_itype_str += "<S";
	  } else {
	    error = true;
	    goto cleanup;
	  }
	  new_itype_str += std::to_string((int)max_len);
	  new_itype = PyUnicode_FromString(new_itype_str.c_str());
	  if (new_itype == NULL) {
	    error = true;
	    goto cleanup;
	  }
	  if (PyDict_SetItem(column_dtypes, key, (PyObject*)new_itype) < 0) {
	    error = true;
	    goto cleanup;
	  }
	  Py_DECREF(new_itype);
	  Py_DECREF(vtype);
	  Py_DECREF(val);
	  new_itype = NULL;
	  vtype = NULL;
	  val = NULL;
	}
	Py_DECREF(key);
	Py_DECREF(itype);
	key = NULL;
	itype = NULL;
      }
      method = PyObject_GetAttrString(x, "to_records");
      if (method == NULL) {
	error = true;
	goto cleanup;
      }
      args = PyTuple_New(0);
      if (args == NULL) {
	error = true;
	goto cleanup;
      }
      kwargs = PyDict_New();
      if (kwargs == NULL) {
	error = true;
	goto cleanup;
      }
      if (PyDict_SetItemString(kwargs, "index", Py_False) < 0) {
	error = true;
	goto cleanup;
      }
      if (PyDict_SetItemString(kwargs, "column_dtypes", column_dtypes) < 0) {
	error = true;
	goto cleanup;
      }
      arr = PyObject_Call(method, args, kwargs);
      if (arr == NULL) {
	error = true;
	goto cleanup;
      }
      error = !SetPythonObjectRaw(arr, allocator, skipTitle);
    cleanup:
      Py_XDECREF(column_dtypes);
      Py_XDECREF(columns);
      Py_XDECREF(dtypes);
      Py_XDECREF(key);
      Py_XDECREF(itype);
      Py_XDECREF(new_itype);
      Py_XDECREF(val);
      Py_XDECREF(vtype);
      Py_XDECREF(vv);
      Py_XDECREF(method);
      Py_XDECREF(args);
      Py_XDECREF(kwargs);
      Py_XDECREF(arr);
      return (!error);
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
    } else {
      SetObject();
      ResetSchema(allocator);
      AddSchemaMember(GetTypeString(), GetPythonInstanceString());
      RAPIDJSON_ASSERT(PyObject_HasAttrString(x, "__class__"));
      if (!PyObject_HasAttrString(x, "__class__"))
	return false;
      PyObject* inst_class = PyObject_GetAttrString(x, "__class__");
      RAPIDJSON_ASSERT(inst_class);
      if (inst_class == NULL)
 	return false;
      Ch* mod_cls_ref = NULL;
      SizeType mod_cls_siz = 0;
      out = GetPythonObjectClassName(inst_class, mod_cls_ref, mod_cls_siz,
				     schema_->GetAllocator());
      Py_DECREF(inst_class);
      RAPIDJSON_ASSERT(out && (mod_cls_ref != NULL));
      if (!out && (mod_cls_ref == NULL))
	return false;
      ValueType mod_cls(mod_cls_ref, mod_cls_siz, schema_->GetAllocator());
      schema_->GetAllocator().Free(mod_cls_ref);
      mod_cls_ref = NULL;
      ValueType args;
      ValueType kwargs;
      char args_keys[6][50] = {"input_arguments",
	"input_args",
	"_input_arguments",
	"_input_args",
	"get_input_arguments",
	"get_input_args"};
      char kwargs_keys[6][50] = {"input_keyword_arguments",
	"input_kwargs",
	"_input_keyword_arguments",
	"_input_kwargs",
	"get_input_keyword_arguments",
	"get_input_kwargs"};
      for (SizeType i = 0; i < 6; i++) {
	if (GetPythonObjectClassAttr(x, args_keys[i], schema_->GetAllocator(), args, true))
	  break;
      }
      for (SizeType i = 0; i < 6; i++) {
	if (GetPythonObjectClassAttr(x, kwargs_keys[i], schema_->GetAllocator(), kwargs, true))
	  break;
      }
      if (args.IsNull() && kwargs.IsNull()) {
	goto pickle;
      }
      AddMember(GetPythonClassString(), mod_cls, schema_->GetAllocator());
      if (!(args.IsNull())) {
	AddMember(GetArgsString(), args, schema_->GetAllocator());
	if (args.HasSchemaNested())
	  AddSchemaMember(GetArgsString(),
			  args.GetSchemaNested(schema_->GetAllocator()));
      }
      if (!(kwargs.IsNull())) {
	AddMember(GetKwargsString(), kwargs, schema_->GetAllocator());
	if (kwargs.HasSchemaNested())
	  AddSchemaMember(GetKwargsString(),
			  kwargs.GetSchemaNested(schema_->GetAllocator()));
      }
    }
    return out;
  pickle:
    if (!allowPickle)
      return false;
    PyObject* py_str = pickle_python_object(x, "SetPythonObjectRaw", true);
    if (py_str == NULL)
      return false;
    char* buffer = NULL;
    Py_ssize_t buffer_len = 0;
    if (PyBytes_AsStringAndSize(py_str, &buffer, &buffer_len) < 0) {
      Py_DECREF(py_str);
      return false;
    }
    SetStringRaw(StringRef((Ch*)buffer, (SizeType)((size_t)buffer_len / sizeof(Ch))),
		 schema_->GetAllocator());
    Py_DECREF(py_str);
    return true;
  }
#endif // YGGDRASIL_DISABLE_PYTHON_C_API
  void SetObjRaw(const ObjWavefront& x, Allocator* allocator = 0) {
    std::stringstream ss;
    ss << x;
    std::string s = ss.str();
    ResetSchema(allocator);
    SetStringRaw(StringRef(s.c_str(), s.size()),
                 schema_->GetAllocator());
    AddSchemaMember(GetTypeString(), GetObjString());
  }
  void SetPlyRaw(const Ply& x, Allocator* allocator = 0) {
    std::stringstream ss;
    ss << x;
    std::string s = ss.str();
    ResetSchema(allocator);
    SetStringRaw(StringRef(s.c_str(), s.size()),
		 schema_->GetAllocator());
    AddSchemaMember(GetTypeString(), GetPlyString());
  }

  const ValueType& GetYggType() const {
    RAPIDJSON_ASSERT(IsYggdrasil());
    ConstMemberIterator type = schema_->FindMember(GetTypeString());
    return type->value;
  }

  bool RequiresPython() const {
    switch(GetType()) {
    case kStringType: {
      if (IsYggdrasil()) {
	const ValueType& type = GetYggType();
	if ((type == GetPythonClassString()) ||
	    (type == GetPythonFunctionString()))
	  return true;
      }
      return false;
    }
    case kObjectType: {
      if (IsYggdrasil()) {
	const ValueType& type = GetYggType();
	if (type == GetPythonInstanceString())
	  return true;
      }
      for (ConstMemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
	if (m->value.RequiresPython())
	  return true;
      return false;
    }
    case kArrayType: {
      for (ConstValueIterator v = Begin(); v != End(); ++v)
	if (v->RequiresPython())
	  return true;
      return false;
    }
    default:
      return false;
    }
  }
  bool IsYggdrasil() const {
    if (!(IsString() || IsObject())) return false;
    return HasSchema();
  }
  bool HasTitle() const {
    if (!IsYggdrasil()) return false;
    ConstMemberIterator title = schema_->FindMember(GetTitleString());
    return (title != schema_->MemberEnd());
  }
  const ValueType& GetTitle() const {
    RAPIDJSON_ASSERT(HasTitle());
    ConstMemberIterator title = schema_->FindMember(GetTitleString());
    return title->value;
  }
  bool IsScalar() const {
    if (!IsYggdrasil()) return false;
    if (GetYggType() == GetScalarString())
      return true;
    if (IsNDArray()) {
      ConstMemberIterator shape = schema_->FindMember(GetShapeString());
      if (shape == schema_->MemberEnd())
	return true;
    }
    return false;
  }
  bool IsScalar(const Ch* subT) const {
    if (!IsScalar())
      return false;
    return (ValueType(subT, internal::StrLen(subT)) == GetSubType());
  }
  template <typename T>
  bool IsScalar(RAPIDJSON_ENABLEIF((YGGDRASIL_IS_SCALAR_TYPE(T)))) const {
    if (!IsScalar())
      return false;
    return (YggSubTypeString<T>() == GetSubType());
  }
#define IS_BUILTIN_SCALAR_(type, check)					\
  template <typename T>							\
  bool IsScalar(RAPIDJSON_ENABLEIF((internal::IsSame<T, type>))) const { \
    if (!IsScalar()) {							\
      return Is ## check();						\
    }									\
    return (YggSubTypeString<T>() == GetSubType());			\
  }
  IS_BUILTIN_SCALAR_(float, Float)
  IS_BUILTIN_SCALAR_(double, Double)
  IS_BUILTIN_SCALAR_(int, Int)
  IS_BUILTIN_SCALAR_(unsigned, Uint)
  IS_BUILTIN_SCALAR_(int64_t, Int64)
  IS_BUILTIN_SCALAR_(uint64_t, Uint64)
#undef IS_BUILTIN_SCALAR_
  bool Is1DArray() const {
    if (!IsYggdrasil()) return false;
    if (GetYggType() == Get1DArrayString())
      return true;
    if (IsNDArray()) {
      ConstMemberIterator shape = schema_->FindMember(GetShapeString());
      if ((shape != schema_->MemberEnd()) && shape->value.IsArray() && (shape->value.Size() == 1))
	return true;
    }
    return false;
  }
  template <typename T>
  bool Is1DArray() const {
    if (!Is1DArray())
      return false;
    return (YggSubTypeString<T>() == GetSubType());
  }
  bool IsNDArray() const {
    if (!IsYggdrasil()) return false;
    return (GetYggType() == GetNDArrayString());
  }
  template <typename T>
  bool IsNDArray() const {
    if (!IsNDArray())
      return false;
    return (YggSubTypeString<T>() == GetSubType());
  }
  bool IsPythonClass() const {
    if (!IsYggdrasil()) return false;
    return (GetYggType() == GetPythonClassString());
  }
  bool IsPythonFunction() const {
    if (!IsYggdrasil()) return false;
    return (GetYggType() == GetPythonFunctionString());
  }
  bool IsPythonInstance() const {
    if (!IsYggdrasil()) return false;
    return (GetYggType() == GetPythonInstanceString());
  }
  bool IsObjWavefront() const {
    if (!IsYggdrasil()) return false;
    return (GetYggType() == GetObjString());
  }
  bool IsPly() const {
    if (!IsYggdrasil()) return false;
    return (GetYggType() == GetPlyString());
  }
  bool IsSchema() const {
    if (!IsYggdrasil()) return false;
    return (GetYggType() == GetSchemaString());
  }
  enum YggSubType GetSubTypeCode() const {
    const ValueType& subtype = GetSubType();
    if      (subtype == GetIntSubTypeString()) return kYggIntSubType;
    else if (subtype == GetUintSubTypeString()) return kYggUintSubType;
    else if (subtype == GetFloatSubTypeString()) return kYggFloatSubType;
    else if (subtype == GetComplexSubTypeString()) return kYggComplexSubType;
    else if (subtype == GetStringSubTypeString()) return kYggStringSubType;
    RAPIDJSON_ASSERT(false);
    return kYggNullSubType;
  }

#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
#ifndef RAPIDJSON_DONT_IMPORT_NUMPY
  static bool NumpyType2SubType(PyArray_Descr* desc,
				ValueType& subtype,
				SizeType& precision,
				ValueType& encoding,
				SizeType itemsize,
				Allocator& allocator) {
    if (desc->type_num == NPY_STRING || desc->type_num == NPY_UNICODE) {
      if (itemsize == 0) return false;
      precision = itemsize;
      if (desc->type_num == NPY_UNICODE)
	encoding.CopyFrom(GetUCS4EncodingString(), allocator);
      subtype.CopyFrom(GetStringSubTypeString(), allocator);
      return true;
    }
    if (!(PyDataType_ISNUMBER(desc))) {
      std::cerr << "NumpyType2SubType: Non-number numpy element (itemsize = " << itemsize << ")" << std::endl;
      return false;
    }
    precision = (SizeType)(desc->elsize);
    if (PyDataType_ISUNSIGNED(desc))
      subtype.CopyFrom(GetUintSubTypeString(), allocator);
    else if (PyDataType_ISSIGNED(desc))
      subtype.CopyFrom(GetIntSubTypeString(), allocator);
    else if (PyDataType_ISFLOAT(desc))
      subtype.CopyFrom(GetFloatSubTypeString(), allocator);
    else if (PyDataType_ISCOMPLEX(desc))
      subtype.CopyFrom(GetComplexSubTypeString(), allocator);
    else
      return false;
    return true;
  }

  int GetSubTypeNumpyType(ValueType& enc) const {
    switch (GetSubTypeCode()) {
    case (kYggIntSubType): {
      switch (GetPrecision()) {
      case (1):
	return NPY_INT8;
      case (2):
	return NPY_INT16;
      case (4):
	return NPY_INT32;
      case (8):
	return NPY_INT64;
      default:
	return -1;
      }
    }
    case (kYggUintSubType): {
      switch (GetPrecision()) {
      case (1):
	return NPY_UINT8;
      case (2):
	return NPY_UINT16;
      case (4):
	return NPY_UINT32;
      case (8):
	return NPY_UINT64;
      default:
	return -1;
      }
    }
    case (kYggFloatSubType): {
      switch (GetPrecision()) {
      case (2):
	return NPY_FLOAT16;
      case (4):
	return NPY_FLOAT32;
      case (8):
	return NPY_FLOAT64;
      default:
	return -1;
      }
    }
    case (kYggComplexSubType): {
      switch (GetPrecision()) {
      case (8):
	return NPY_COMPLEX64;
      case (16):
	return NPY_COMPLEX128;
      default:
	return -1;
      }
    }
    case (kYggStringSubType): {
      if (!HasEncoding())
	return NPY_STRING;
      enc.CopyFrom(GetEncoding(), schema_->GetAllocator());
      if (enc == GetASCIIEncodingString())
	return NPY_STRING;
      return NPY_UNICODE;
    }
    default: 
      return -1;
    }
  }
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
#endif // YGGDRASIL_DISABLE_PYTHON_C_API

  const ValueType& GetSubType() const {
    RAPIDJSON_ASSERT(IsYggdrasil());
    RAPIDJSON_ASSERT(schema_->HasMember(GetSubTypeString()));
    ConstMemberIterator subtype = schema_->FindMember(GetSubTypeString());
    RAPIDJSON_ASSERT(subtype->value.IsString());
    return subtype->value;
  }

  const Ch* GetSubType(SizeType &length) const {
    const ValueType& subtype = GetSubType();
    length = subtype.GetStringLength();
    return subtype.GetString();
  }

  bool HasPrecision() const {
    if (!IsYggdrasil()) return false;
    ConstMemberIterator x = schema_->FindMember(GetPrecisionString());
    return (x != schema_->MemberEnd());
  }
  SizeType GetPrecision() const {
    RAPIDJSON_ASSERT(HasPrecision());
    return static_cast<SizeType>(schema_->FindMember(GetPrecisionString())->value.GetUint());
  }

  bool HasEncoding() const {
    if (!IsYggdrasil()) return false;
    ConstMemberIterator x = schema_->FindMember(GetEncodingString());
    return (x != schema_->MemberEnd());
  }
  const ValueType& GetEncoding() const {
    RAPIDJSON_ASSERT(HasEncoding());
    ConstMemberIterator encoding = schema_->FindMember(GetEncodingString());
    RAPIDJSON_ASSERT(encoding != schema_->MemberEnd());
    return encoding->value;
  }
  
  const ValueType& GetShape() const {
    RAPIDJSON_ASSERT(IsNDArray());
    ConstMemberIterator shape = schema_->FindMember(GetShapeString());
    RAPIDJSON_ASSERT(shape != schema_->MemberEnd());
    return shape->value;
  }
  SizeType* GetShape(SizeType &ndim, Allocator& allocator) const {
    RAPIDJSON_ASSERT(IsYggdrasil());
    ndim = 0;
    SizeType* out = NULL;
    ConstMemberIterator shape = schema_->FindMember(GetShapeString());
    if (shape != schema_->MemberEnd()) {
      RAPIDJSON_ASSERT(shape->value.IsArray());
      ndim = static_cast<SizeType>(shape->value.Size());
      out = static_cast<SizeType*>(allocator.Malloc(ndim * sizeof(SizeType)));
      for (SizeType i = 0; i < shape->value.Size(); i++) {
	out[i] = static_cast<SizeType>(shape->value[i].GetUint());
      }
    }
    return out;
  }

  SizeType GetNElements() const {
    RAPIDJSON_ASSERT(IsYggdrasil());
    SizeType out = 1;
    ConstMemberIterator shape = schema_->FindMember(GetShapeString());
    if (shape != schema_->MemberEnd()) {
      RAPIDJSON_ASSERT(shape->value.IsArray());
      for (SizeType i = 0; i < shape->value.Size(); i++) {
	out *= static_cast<SizeType>(shape->value[i].GetUint());
      }
    } else {
      ConstMemberIterator length = schema_->FindMember(GetLengthString());
      if (length != schema_->MemberEnd()) {
	out *= static_cast<SizeType>(length->value.GetUint());
      }
    }
    return out;
  }

  void* GetDataPtr(bool& requires_freeing) const {
    switch (GetType()) {
    case kNullType:     return (void*)(&internal::__StaticNull);
    case kFalseType:    return (void*)(&internal::__StaticFalse);
    case kTrueType:     return (void*)(&internal::__StaticTrue);
    case kStringType: {
      if (IsObjWavefront()) {
	requires_freeing = true;
	ObjWavefront* tmp = new ObjWavefront();
	GetObjWavefront(*tmp);
	return (void*)tmp;
      } else if (IsPly()) {
	requires_freeing = true;
	Ply* tmp = new Ply();
	GetPly(*tmp);
	return (void*)tmp;
      } else if (IsPythonFunction()) {
	requires_freeing = true;
	return (void*)GetPythonObjectRaw();
      } else if (IsPythonInstance()) {
	requires_freeing = true;
	return (void*)GetPythonObjectRaw();
      } else {
	return (void*)(GetString());
      }
    }
    case kNumberType: {
      if (IsDouble())         return (void*)(&data_.n.d);
      else if (IsInt())       return (void*)(&data_.n.i.i);
      else if (IsUint())      return (void*)(&data_.n.u.u);
      else if (IsInt64())     return (void*)(&data_.n.i64);
      else                    return (void*)(&data_.n.u64);
    }
    // case kObjectType: {
    // 	if (IsPythonInstance()) {
    // 	  requires_freeing = true;
    // 	  return (void*)GetPythonObjectRaw();
    // 	} else {
    // 	  return (void*)this;
    // 	}
    // }
    default:
      return (void*)this;
    }
    return NULL;
  }
  
  SizeType GetNBytes() const {
    switch (GetType()) {
    case kNullType:     return (SizeType)(sizeof(void*));
    case kFalseType:    return (SizeType)(sizeof(bool));
    case kTrueType:     return (SizeType)(sizeof(bool));
    case kStringType: {
      if (IsScalar() || IsNDArray()) {
	return GetPrecision() * GetNElements();
      } else if (IsObjWavefront()) {
	return (SizeType)(sizeof(ObjWavefront));
      } else if (IsPly()) {
	return (SizeType)(sizeof(Ply));
      } else if (IsPythonFunction()) {
	return (SizeType)(sizeof(PyObject));
      } else if (IsPythonInstance()) {
	return (SizeType)(sizeof(PyObject));
      } else {
	return GetStringLength();
      }
    }
    case kNumberType: {
      if (IsDouble())         return (SizeType)(sizeof(double));
      else if (IsInt())       return (SizeType)(sizeof(int));
      else if (IsUint())      return (SizeType)(sizeof(unsigned));
      else if (IsInt64())     return (SizeType)(sizeof(int64_t));
      else                    return (SizeType)(sizeof(uint64_t));
    }
    // case kObjectType: {
    //   if (IsPythonInstance()) {
    // 	return (SizeType)(sizeof(PyObject));
    //   } else {
    //  return (SizeType)(sizeof(GenericDocument<EncodingType, AllocatorType>));
    //   }
    // }
    default:
      return (SizeType)(sizeof(GenericDocument<EncodingType, AllocatorType, AllocatorType>));
    }
  }

  template <typename T>
  void ChangePrecision(const unsigned char* bytes, T* dst,
		       SizeType nelements) const {
    changePrecision(GetSubTypeCode(), GetPrecision(),
		    bytes, dst, nelements);
  }
  
  template <typename T>
  T* ChangePrecision(const unsigned char* bytes, SizeType nelements,
		     Allocator& allocator) const {
    T* v2 = (T*)allocator.Malloc(nelements * sizeof(T));
    RAPIDJSON_ASSERT(v2);
    ChangePrecision(bytes, v2, nelements);
    return v2;
  }
  
  template <typename T>
  void GetScalarValueBase(T& data) const {
    RAPIDJSON_ASSERT(YggSubTypeString<T>() == GetSubType());
    RAPIDJSON_ASSERT((GetStringLength() * sizeof(Ch)) == GetPrecision());
    ChangePrecision(reinterpret_cast<const unsigned char*>(GetString()),
		    &data, 1);
  }
  template <typename T>
  void GetScalarValue(T& data,
		      RAPIDJSON_DISABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T)))) const {
    if (!IsYggdrasil()) {
      if (IsInt() && (GetYggSubType<T>() == kYggIntSubType)) {
	data = static_cast<T>(GetInt());
	return;
      } else if (IsUint() && (GetYggSubType<T>() == kYggUintSubType)) {
	data = static_cast<T>(GetUint());
	return;
      } else if (IsInt64() && (GetYggSubType<T>() == kYggIntSubType)) {
	data = static_cast<T>(GetInt64());
	return;
      } else if (IsUint64() && (GetYggSubType<T>() == kYggUintSubType)) {
	data = static_cast<T>(GetUint64());
	return;
      } else if (IsDouble() && (GetYggSubType<T>() == kYggFloatSubType)) {
	data = static_cast<T>(GetDouble());
	return;
      }
    }
    GetScalarValueBase(data);
  }
  template <typename T>
  void GetScalarValue(std::complex<T>& data) const {
    GetScalarValueBase(data);
  }
  template <typename T>
  void GetScalarQuantity(units::GenericQuantity<T, EncodingType>& data,
			 const UnitsType data_units) const {
    T value = data.value();
    GetScalarValue(value);
    UnitsType new_units = data_units;
    if ((new_units.is_empty()) && (!(data.units().is_empty())))
	new_units = data.units();
    data.set_value(value);
    if (HasUnits()) {
      data.set_units(GetUnits().GetString());
      if (!new_units.is_empty())
	data.convert_to(new_units);
    } else {
      data.set_units(new_units);
    }
  }
  template <typename T>
  void GetScalarQuantity(units::GenericQuantity<T, EncodingType>& data,
			 const Ch* units_str) const {
    UnitsType new_units(units_str);
    return GetScalarQuantity(data, new_units); }
  template <typename T>
  void GetScalarQuantity(units::GenericQuantity<T, EncodingType>& data) const {
    UnitsType new_units;
    return GetScalarQuantity(data, new_units); }
  template <typename T>
  units::GenericQuantity<T, EncodingType> GetScalarQuantity(const UnitsType data_units) const {
    units::GenericQuantity<T, EncodingType> data;
    GetScalarQuantity(data, data_units);
    return data; }
  template <typename T>
  units::GenericQuantity<T, EncodingType> GetScalarQuantity(const Ch* units_str) const {
    units::GenericQuantity<T, EncodingType> data;
    GetScalarQuantity(data, units_str);
    return data; }
  template <typename T>
  units::GenericQuantity<T, EncodingType> GetScalarQuantity() const {
    units::GenericQuantity<T, EncodingType> data;
    GetScalarQuantity(data);
    return data; }
  
  template <typename T>
  void GetScalar(T& data) const {
    GetScalarValue(data);
  }
  template <typename T>
  void GetScalar(T& data, const UnitsType data_units) const {
    units::GenericQuantity<T, EncodingType> x(data, data_units);
    GetScalarQuantity(x);
    data = x.value();
  }
  template <typename T>
  void GetScalar(T& data, const Ch* units_str) const {
    UnitsType data_units(units_str);
    GetScalar(data, data_units);
  }
  template <typename T>
  T GetScalar(RAPIDJSON_DISABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T)))) const {
    T data = 0;
    GetScalarValue(data);
    return data;
  }
  template <typename T>
  T GetScalar(RAPIDJSON_ENABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T)))) const {
    T data(0, 0);
    GetScalarValue(data);
    return data;
  }
  template <typename T>
  T GetScalar(const UnitsType data_units,
	      RAPIDJSON_DISABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T)))) const {
    T data = 0;
    GetScalar(data, data_units);
    return data;
  }
  template <typename T>
  T GetScalar(const UnitsType data_units,
	      RAPIDJSON_ENABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T)))) const {
    T data(0, 0);
    GetScalar(data, data_units);
    return data;
  }
  template <typename T>
  T GetScalar(const Ch* units_str) const {
    UnitsType data_units(units_str);
    return GetScalar<T>(data_units);
  }

#define GET_SCALAR_METHOD(type, name)				\
  type Get ## name(const Ch* units_str) const {			\
    return GetScalar<type>(units_str); }			\
  type Get ## name() const {					\
    return GetScalar<type>(); }

  GET_SCALAR_METHOD(uint8_t, Uint8);
  GET_SCALAR_METHOD(uint16_t, Uint16);
  GET_SCALAR_METHOD(uint32_t, Uint32);
  GET_SCALAR_METHOD(int8_t, Int8);
  GET_SCALAR_METHOD(int16_t, Int16);
  GET_SCALAR_METHOD(int32_t, Int32);
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
  GET_SCALAR_METHOD(long double, LongDouble);
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
  GET_SCALAR_METHOD(std::complex<float>, ComplexFloat);
  GET_SCALAR_METHOD(std::complex<double>, ComplexDouble);
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
  GET_SCALAR_METHOD(std::complex<long double>, ComplexLongDouble);
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE

#undef GET_SCALAR_METHOD

  template <typename T>
  T* Get(SizeType& nelements) const {
    T* out = NULL;
    Get1DArray(&out, nelements);
    return out;
  }

  template <typename T>
  void GetArrayValueBase(T*& data, SizeType& ndim, SizeType*& shape,
			 Allocator& allocator) const {
    RAPIDJSON_ASSERT(GetSubType() == GetStringSubTypeString() ||
		     YggSubTypeString<T>() == GetSubType());
    size_t length = 0;
    unsigned char* decoded_bytes = GetDecodedString(length, allocator);
    RAPIDJSON_ASSERT((SizeType)length == GetNBytes());
    shape = GetShape(ndim, allocator);
    RAPIDJSON_ASSERT(shape);
    SizeType nelements = GetNElements();
    if (sizeof(T) != GetPrecision() && GetSubType() != GetStringSubTypeString()) {
      unsigned char* old_decoded_bytes = decoded_bytes;
      decoded_bytes = (unsigned char*)ChangePrecision<T>(decoded_bytes,
							 nelements,
							 allocator);
      allocator.Free(old_decoded_bytes);
      old_decoded_bytes = NULL;
      length = sizeof(T) * nelements;
    }
    if (data != NULL) {
      free(data);
      data = NULL;
    }
    data = reinterpret_cast<T*>(decoded_bytes);
  }
  template <typename T>
  void GetArrayQuantity(units::GenericQuantityArray<T, EncodingType>* data,
			Allocator& allocator,
			const UnitsType data_units = UnitsType()) const {
    T* value = nullptr;
    SizeType ndim = 0;
    SizeType* shape = nullptr;
    GetArrayValueBase(value, ndim, shape, allocator);
    UnitsType new_units = data_units;
    if ((new_units.is_empty()) && (!(data->units().is_empty())))
	new_units = data->units();
    data->set_value(value, ndim, shape);
    // TODO: Pass allocator to quantity
    if (HasUnits()) {
      data->set_units(GetUnits().GetString());
      if (!new_units.is_empty())
	data->convert_to(new_units);
    } else {
      data->set_units(new_units);
    }
    allocator.Free(value);
    allocator.Free(shape);
    value = NULL;
    shape = NULL;
  }
  template <typename T>
  void GetArrayQuantity(units::GenericQuantityArray<T, EncodingType>* data,
			Allocator& allocator, const Ch* units_str) const {
    return GetArrayQuantity(data, allocator, UnitsType(units_str)); }
  template <typename T>
  units::GenericQuantityArray<T, EncodingType> GetArrayQuantity(Allocator& allocator,
								const UnitsType data_units = UnitsType()) const {
    units::GenericQuantityArray<T, EncodingType> data;
    GetArrayQuantity(&data, allocator, data_units);
    return data;
  }
  template <typename T>
  units::GenericQuantityArray<T, EncodingType> GetArrayQuantity(Allocator& allocator,
								const Ch* units_str) const {
    return GetArrayQuantity<T>(allocator, UnitsType(units_str)); }

  // 1D array access
  template <typename T>
  void Get1DArray(T*& data, SizeType& nelements, Allocator& allocator,
		  const UnitsType data_units = UnitsType(),
		  RAPIDJSON_DISABLEIF((internal::IsSame<T, Ch>))) const {
    units::GenericQuantityArray<T, EncodingType> x;
    GetArrayQuantity(&x, allocator, data_units);
    nelements = x.nelements();
    data = (T*)allocator.Malloc(nelements * sizeof(T));
    RAPIDJSON_ASSERT(data);
    for (SizeType i = 0; i < nelements; i++)
      data[i] = x.value()[i];
    // data = x.pop_value();
  }
  template <typename T>
  void Get1DArray(T*& data, SizeType& nelements, Allocator& allocator,
		  const Ch* units_str) const {
    return Get1DArray(data, nelements, allocator, UnitsType(units_str)); }
      
  template <typename T>
  T* Get1DArray(SizeType& nelements, Allocator& allocator,
		const UnitsType data_units = UnitsType()) const {
    T* data = nullptr;
    Get1DArray(data, nelements, allocator, data_units);
    return data;
  }
  template <typename T>
  T* Get1DArray(SizeType& nelements, Allocator& allocator,
		const Ch* units_str) const {
    return Get1DArray<T>(nelements, allocator, UnitsType(units_str)); }
  // 1D string array access
  void Get1DArray(Ch*& data, SizeType& nelements, SizeType& precision,
		  Allocator& allocator) const {
    SizeType ndim = 0;
    SizeType* shape = nullptr;
    GetNDArray(data, shape, ndim, precision, allocator);
    nelements = GetNElements();
    allocator.Free(shape);
    shape = NULL;
  }
  Ch* Get1DArray(SizeType& nelements, SizeType& precision,
		 Allocator& allocator) const {
    Ch* data = nullptr;
    Get1DArray(data, nelements, precision, allocator);
    return data;
  }
  
  // ND array access
  template <typename T>
  void GetNDArray(T*& data, SizeType*& shape, SizeType& ndim,
		  Allocator& allocator,
		  const UnitsType data_units = UnitsType(),
		  RAPIDJSON_DISABLEIF((internal::IsSame<T, Ch>))) const {
    SizeType nelements = 1;
    units::GenericQuantityArray<T, EncodingType> x;
    GetArrayQuantity<T>(&x, allocator, data_units);
    ndim = x.ndim();
    if (shape != nullptr) {
      free(shape);
      shape = NULL;
    }
    shape = (SizeType*)allocator.Malloc(ndim * sizeof(SizeType));
    RAPIDJSON_ASSERT(shape);
    for (SizeType i = 0; i < ndim; i++) {
      shape[i] = x.shape()[i];
      nelements = nelements * shape[i];
    }
    if (data != nullptr) {
      free(data);
      data = NULL;
    }
    data = (T*)allocator.Malloc(nelements * sizeof(T));
    RAPIDJSON_ASSERT(data);
    for (SizeType i = 0; i < nelements; i++)
      data[i] = x.value()[i];
    // shape = x.pop_shape();
    // data = x.pop_value();
  }
  template <typename T>
  void GetNDArray(T*& data, SizeType*& shape, SizeType& ndim,
		  Allocator& allocator, const Ch* units_str) const {
    return GetNDArray(data, shape, ndim, allocator, UnitsType(units_str)); }
      
  template <typename T>
  T* GetNDArray(SizeType*& shape, SizeType& ndim, Allocator& allocator,
		const UnitsType data_units = UnitsType()) const {
    T* data = nullptr;
    GetNDArray(data, shape, ndim, allocator, data_units);
    return data;
  }
  template <typename T>
  T* GetNDArray(SizeType*& shape, SizeType& ndim, Allocator& allocator,
		const Ch* units_str) const {
    return GetNDArray<T>(shape, ndim, allocator, UnitsType(units_str)); }
  // ND string array access
  void GetNDArray(Ch*& data, SizeType*& shape, SizeType& ndim,
		  SizeType& precision, Allocator& allocator) const {
    GetArrayValueBase(data, ndim, shape, allocator);
    precision = GetPrecision();
  }
  Ch* GetNDArray(SizeType*& shape, SizeType& ndim, SizeType& precision,
		 Allocator& allocator) const {
    Ch* data = nullptr;
    GetNDArray(data, shape, ndim, precision, allocator);
    return data;
  }
  
  template <typename T>
  T* Get(SizeType** shape, SizeType& ndim, Allocator& allocator) const {
    return GetNDArray<T>(shape, ndim, allocator); }

  unsigned char* GetDecodedString(size_t &length, Allocator &allocator) const {
    length = (size_t)(GetStringLength() * sizeof(Ch));
    unsigned char* decoded_bytes = (unsigned char*)allocator.Malloc(length);
    RAPIDJSON_ASSERT(decoded_bytes != NULL);
    memcpy(decoded_bytes, GetString(), length);
    return decoded_bytes;
  }

#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
  PyObject* GetPythonClass(bool allowFunc = false) const {
    const char *mod_class;
    if (IsPythonInstance()) {
      if (IsObject()) {
	ConstMemberIterator m = FindMember(GetPythonClassString());
	RAPIDJSON_ASSERT(m != MemberEnd());
	mod_class = reinterpret_cast<const char*>(m->value.GetString());
      } else if (IsString()) {
	PyObject* py_inst = GetPythonInstance();
	if (py_inst == NULL)
	  return NULL;
	PyObject* py_cls = PyObject_Type(py_inst);
	Py_DECREF(py_inst);
	return py_cls;
      } else {
	return NULL;
      }
    } else if (IsPythonClass() || (allowFunc && IsPythonFunction())) {
      mod_class = reinterpret_cast<const char*>(GetString());
    } else {
      return NULL;
    }
    PyObject* py_class = import_python_object(mod_class, "GetPythonClass: ", true);
    return py_class;
  }
  PyObject* GetPythonFunction() const { return GetPythonClass(true); }
  PyObject* GetPythonInstance() const {
    if (IsObject()) {
      PyObject* py_class = GetPythonClass();
      PyObject* py_args = NULL;
      PyObject* py_kwargs = NULL;
      // Args
      ConstMemberIterator ma = FindMember(GetArgsString());
      if (ma != MemberEnd()) {
	PyObject* py_args_list = ma->value.GetPythonObjectRaw();
	py_args = PyList_AsTuple(py_args_list);
	Py_DECREF(py_args_list);
      } else {
	py_args = PyTuple_New(0);
      }
      // Kwargs
      ConstMemberIterator mk = FindMember(GetKwargsString());
      if (mk != MemberEnd()) {
	py_kwargs = mk->value.GetPythonObjectRaw();
      } else {
	py_kwargs = PyDict_New();
      }
      RAPIDJSON_ASSERT(py_class);
      RAPIDJSON_ASSERT(py_args);
      RAPIDJSON_ASSERT(py_kwargs);
      PyObject* py_inst = NULL;
      if (py_class && py_args && py_kwargs)
	py_inst = PyObject_Call(py_class, py_args, py_kwargs);
      Py_XDECREF(py_class);
      Py_XDECREF(py_args);
      Py_XDECREF(py_kwargs);
      return py_inst;
    } else if (IsString()) {
      return unpickle_python_object((char*)GetString(),
				    (size_t)(GetStringLength() * sizeof(Ch)),
				    "GetPythonInstance", true);
    } else {
      return NULL;
    }
  }
#endif // YGGDRASIL_DISABLE_PYTHON_C_API
  void GetObjWavefront(ObjWavefront &o) const {
    if (IsObjWavefront()) {
      std::stringstream ss;
      ss.str(GetString());
      ss >> o;
    } else if (IsPly()) {
      Ply p = GetPly();
      o.fromPly(p);
    } else {
      RAPIDJSON_ASSERT(IsPly() || IsObjWavefront());
    }
  }
  ObjWavefront GetObjWavefront() const {
    ObjWavefront o;
    GetObjWavefront(o);
    return o;
  }
  void GetPly(Ply &p) const {
    if (IsPly()) {
      std::stringstream ss;
      ss.str(GetString());
      ss >> p;
    } else if (IsObjWavefront()) {
      ObjWavefront o = GetObjWavefront();
      p.fromObjWavefront(o);
    } else {
      RAPIDJSON_ASSERT(IsPly() || IsObjWavefront());
    }
  }
  Ply GetPly() const {
    Ply p = Ply();
    GetPly(p);
    return p;
  }
  Object GetSchema() { RAPIDJSON_ASSERT(IsSchema()); return GetObject(); }

  // Methods from provided schema
  template <typename SourceAllocator>
  GenericValue& SetYggdrasilString(const Ch* s, SizeType length, Allocator& allocator,
				   const GenericValue<Encoding,SourceAllocator>& schema) {
    this->~GenericValue();
    new (this) GenericValue(s, length, allocator, schema);
    return *this; }

  template<typename T>
  GenericValue& SetScalar(const T x, const Ch* units_str=nullptr,
			  const SizeType units_len=0,
			  Allocator* allocator = 0) {
    this->~GenericValue();
    if (allocator)
      new (this) GenericValue(x, units_str, units_len, *allocator);
    else
      new (this) GenericValue(x, units_str, units_len);
    return *this; }
  GenericValue& SetScalar(const Ch* x, SizeType precision, Allocator& allocator,
			  const Ch* encoding=nullptr, SizeType encoding_len=0) {
    this->~GenericValue();
    new (this) GenericValue(x, precision, encoding, encoding_len, allocator);
    return *this; }
  template<typename T>
  GenericValue& Set1DArray(const T* x, SizeType nelements,
			   const Ch* units_str=nullptr,
			   const SizeType units_len=0,
			   Allocator* allocator = 0) {
    this->~GenericValue();
    if (allocator)
      new (this) GenericValue(x, nelements, units_str, units_len, *allocator);
    else
      new (this) GenericValue(x, nelements, units_str, units_len);
    return *this; }
  template<typename T>
  GenericValue& SetNDArray(const T* x, SizeType shape[], SizeType ndim,
			   const Ch* units_str=nullptr,
			   const SizeType units_len=0,
			   Allocator* allocator = 0) {
    this->~GenericValue();
    if (allocator)
      new (this) GenericValue(x, shape, ndim, units_str, units_len, *allocator);
    else
      new (this) GenericValue(x, shape, ndim, units_str, units_len);
    return *this; }
  GenericValue& SetNDArray(const Ch* x, SizeType precision,
			   SizeType shape[], SizeType ndim,
			   Allocator& allocator,
			   const Ch* encoding=nullptr,
			   SizeType encoding_len=0) {
    this->~GenericValue();
    new (this) GenericValue(x, precision, shape, ndim, allocator, encoding, encoding_len);
    return *this; }
#ifndef YGGDRASIL_DISABLE_PYTHON_C_API
  GenericValue& SetPythonInstance(PyObject* x, Allocator* allocator = 0) {
    this->~GenericValue();
    if (allocator)
      new (this) GenericValue(x, *allocator);
    else
      new (this) GenericValue(x);
    return *this; }
#endif // YGGDRASIL_DISABLE_PYTHON_C_API
  GenericValue& SetObjWavefront(ObjWavefront x, Allocator* allocator = 0) {
    this->~GenericValue();
    new (this) GenericValue(x, allocator);
    return *this; }
  GenericValue& SetObj(ObjWavefront x, Allocator* allocator = 0) {
    return SetObjWavefront(x, allocator); }
  GenericValue& SetPly(Ply x, Allocator* allocator = 0) {
    this->~GenericValue();
    new (this) GenericValue(x, allocator);
    return *this; }
  GenericValue& SetSchema(Allocator* allocator = 0) {
    this->~GenericValue();
    new (this) GenericValue(kObjectType);
    ResetSchema(allocator);
    AddSchemaMember(GetTypeString(), GetSchemaString());
    return *this; }

// protected:
  SchemaValueType* schema_;

#endif // RAPIDJSON_YGGDRASIL
  
};

//! GenericValue with UTF8 encoding
typedef GenericValue<UTF8<> > Value;

///////////////////////////////////////////////////////////////////////////////
// GenericDocument 

//! A document for parsing JSON text as DOM.
/*!
    \note implements Handler concept
    \tparam Encoding Encoding for both parsing and string storage.
    \tparam Allocator Allocator for allocating memory for the DOM
    \tparam StackAllocator Allocator for allocating memory for stack during parsing.
    \warning Although GenericDocument inherits from GenericValue, the API does \b not provide any virtual functions, especially no virtual destructor.  To avoid memory leaks, do not \c delete a GenericDocument object via a pointer to a GenericValue.
*/
template <typename Encoding, typename Allocator = RAPIDJSON_DEFAULT_ALLOCATOR, typename StackAllocator = RAPIDJSON_DEFAULT_STACK_ALLOCATOR >
class GenericDocument : public GenericValue<Encoding, Allocator> {
public:
    typedef typename Encoding::Ch Ch;                       //!< Character type derived from Encoding.
    typedef GenericValue<Encoding, Allocator> ValueType;    //!< Value type of the document.
    typedef Allocator AllocatorType;                        //!< Allocator type from template parameter.
    typedef StackAllocator StackAllocatorType;              //!< StackAllocator type from template parameter.

    //! Constructor
    /*! Creates an empty document of specified type.
        \param type             Mandatory type of object to create.
        \param allocator        Optional allocator for allocating memory.
        \param stackCapacity    Optional initial capacity of stack in bytes.
        \param stackAllocator   Optional allocator for allocating memory for stack.
    */
    explicit GenericDocument(Type type, Allocator* allocator = 0, size_t stackCapacity = kDefaultStackCapacity, StackAllocator* stackAllocator = 0) :
        GenericValue<Encoding, Allocator>(type),  allocator_(allocator), ownAllocator_(0), stack_(stackAllocator, stackCapacity), parseResult_()
    {
        if (!allocator_)
            ownAllocator_ = allocator_ = RAPIDJSON_NEW(Allocator)();
    }

    //! Constructor
    /*! Creates an empty document which type is Null. 
        \param allocator        Optional allocator for allocating memory.
        \param stackCapacity    Optional initial capacity of stack in bytes.
        \param stackAllocator   Optional allocator for allocating memory for stack.
    */
    GenericDocument(Allocator* allocator = 0, size_t stackCapacity = kDefaultStackCapacity, StackAllocator* stackAllocator = 0) : 
        allocator_(allocator), ownAllocator_(0), stack_(stackAllocator, stackCapacity), parseResult_()
    {
#ifdef RAPIDJSON_YGGDRASIL
        this->schema_ = NULL;
#endif // RAPIDJSON_YGGDRASIL
        if (!allocator_)
            ownAllocator_ = allocator_ = RAPIDJSON_NEW(Allocator)();
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move constructor in C++11
    GenericDocument(GenericDocument&& rhs) RAPIDJSON_NOEXCEPT
        : ValueType(std::forward<ValueType>(rhs)), // explicit cast to avoid prohibited move from Document
          allocator_(rhs.allocator_),
          ownAllocator_(rhs.ownAllocator_),
          stack_(std::move(rhs.stack_)),
          parseResult_(rhs.parseResult_)
    {
        rhs.allocator_ = 0;
        rhs.ownAllocator_ = 0;
        rhs.parseResult_ = ParseResult();
    }
#endif

    ~GenericDocument() {
        // Clear the ::ValueType before ownAllocator is destroyed, ~ValueType()
        // runs last and may access its elements or members which would be freed
        // with an allocator like MemoryPoolAllocator (CrtAllocator does not
        // free its data when destroyed, but MemoryPoolAllocator does).
        if (ownAllocator_) {
            ValueType::SetNull();
        }
        Destroy();
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move assignment in C++11
    GenericDocument& operator=(GenericDocument&& rhs) RAPIDJSON_NOEXCEPT
    {
        // The cast to ValueType is necessary here, because otherwise it would
        // attempt to call GenericValue's templated assignment operator.
        ValueType::operator=(std::forward<ValueType>(rhs));

        // Calling the destructor here would prematurely call stack_'s destructor
        Destroy();

        allocator_ = rhs.allocator_;
        ownAllocator_ = rhs.ownAllocator_;
        stack_ = std::move(rhs.stack_);
        parseResult_ = rhs.parseResult_;

        rhs.allocator_ = 0;
        rhs.ownAllocator_ = 0;
        rhs.parseResult_ = ParseResult();

        return *this;
    }
#endif

    //! Exchange the contents of this document with those of another.
    /*!
        \param rhs Another document.
        \note Constant complexity.
        \see GenericValue::Swap
    */
    GenericDocument& Swap(GenericDocument& rhs) RAPIDJSON_NOEXCEPT {
        ValueType::Swap(rhs);
        stack_.Swap(rhs.stack_);
        internal::Swap(allocator_, rhs.allocator_);
        internal::Swap(ownAllocator_, rhs.ownAllocator_);
        internal::Swap(parseResult_, rhs.parseResult_);
        return *this;
    }

    // Allow Swap with ValueType.
    // Refer to Effective C++ 3rd Edition/Item 33: Avoid hiding inherited names.
    using ValueType::Swap;

    //! free-standing swap function helper
    /*!
        Helper function to enable support for common swap implementation pattern based on \c std::swap:
        \code
        void swap(MyClass& a, MyClass& b) {
            using std::swap;
            swap(a.doc, b.doc);
            // ...
        }
        \endcode
        \see Swap()
     */
    friend inline void swap(GenericDocument& a, GenericDocument& b) RAPIDJSON_NOEXCEPT { a.Swap(b); }

    //! Populate this document by a generator which produces SAX events.
    /*! \tparam Generator A functor with <tt>bool f(Handler)</tt> prototype.
        \param g Generator functor which sends SAX events to the parameter.
        \return The document itself for fluent API.
    */
    template <typename Generator>
    GenericDocument& Populate(Generator& g) {
        ClearStackOnExit scope(*this);
        if (g(*this)) {
            RAPIDJSON_ASSERT(stack_.GetSize() == sizeof(ValueType)); // Got one and only one root object
            ValueType::operator=(*stack_.template Pop<ValueType>(1));// Move value from stack to document
        }
        return *this;
    }

    //!@name Parse from stream
    //!@{

    //! Parse JSON text from an input stream (with Encoding conversion)
    /*! \tparam parseFlags Combination of \ref ParseFlag.
        \tparam SourceEncoding Encoding of input stream
        \tparam InputStream Type of input stream, implementing Stream concept
        \param is Input stream to be parsed.
        \return The document itself for fluent API.
    */
    template <unsigned parseFlags, typename SourceEncoding, typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        GenericReader<SourceEncoding, Encoding, StackAllocator> reader(
            stack_.HasAllocator() ? &stack_.GetAllocator() : 0);
        ClearStackOnExit scope(*this);
        parseResult_ = reader.template Parse<parseFlags>(is, *this);
        if (parseResult_) {
            RAPIDJSON_ASSERT(stack_.GetSize() == sizeof(ValueType)); // Got one and only one root object
            ValueType::operator=(*stack_.template Pop<ValueType>(1));// Move value from stack to document
        }
        return *this;
    }

    //! Parse JSON text from an input stream
    /*! \tparam parseFlags Combination of \ref ParseFlag.
        \tparam InputStream Type of input stream, implementing Stream concept
        \param is Input stream to be parsed.
        \return The document itself for fluent API.
    */
    template <unsigned parseFlags, typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        return ParseStream<parseFlags, Encoding, InputStream>(is);
    }

    //! Parse JSON text from an input stream (with \ref kParseDefaultFlags)
    /*! \tparam InputStream Type of input stream, implementing Stream concept
        \param is Input stream to be parsed.
        \return The document itself for fluent API.
    */
    template <typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        return ParseStream<kParseDefaultFlags, Encoding, InputStream>(is);
    }
    //!@}

    //!@name Parse in-place from mutable string
    //!@{

    //! Parse JSON text from a mutable string
    /*! \tparam parseFlags Combination of \ref ParseFlag.
        \param str Mutable zero-terminated string to be parsed.
        \return The document itself for fluent API.
    */
    template <unsigned parseFlags>
    GenericDocument& ParseInsitu(Ch* str) {
        GenericInsituStringStream<Encoding> s(str);
        return ParseStream<parseFlags | kParseInsituFlag>(s);
    }

    //! Parse JSON text from a mutable string (with \ref kParseDefaultFlags)
    /*! \param str Mutable zero-terminated string to be parsed.
        \return The document itself for fluent API.
    */
    GenericDocument& ParseInsitu(Ch* str) {
        return ParseInsitu<kParseDefaultFlags>(str);
    }
    //!@}

    //!@name Parse from read-only string
    //!@{

    //! Parse JSON text from a read-only string (with Encoding conversion)
    /*! \tparam parseFlags Combination of \ref ParseFlag (must not contain \ref kParseInsituFlag).
        \tparam SourceEncoding Transcoding from input Encoding
        \param str Read-only zero-terminated string to be parsed.
    */
    template <unsigned parseFlags, typename SourceEncoding>
    GenericDocument& Parse(const typename SourceEncoding::Ch* str) {
        RAPIDJSON_ASSERT(!(parseFlags & kParseInsituFlag));
        GenericStringStream<SourceEncoding> s(str);
        return ParseStream<parseFlags, SourceEncoding>(s);
    }

    //! Parse JSON text from a read-only string
    /*! \tparam parseFlags Combination of \ref ParseFlag (must not contain \ref kParseInsituFlag).
        \param str Read-only zero-terminated string to be parsed.
    */
    template <unsigned parseFlags>
    GenericDocument& Parse(const Ch* str) {
        return Parse<parseFlags, Encoding>(str);
    }

    //! Parse JSON text from a read-only string (with \ref kParseDefaultFlags)
    /*! \param str Read-only zero-terminated string to be parsed.
    */
    GenericDocument& Parse(const Ch* str) {
        return Parse<kParseDefaultFlags>(str);
    }

    template <unsigned parseFlags, typename SourceEncoding>
    GenericDocument& Parse(const typename SourceEncoding::Ch* str, size_t length) {
        RAPIDJSON_ASSERT(!(parseFlags & kParseInsituFlag));
        MemoryStream ms(reinterpret_cast<const char*>(str), length * sizeof(typename SourceEncoding::Ch));
        EncodedInputStream<SourceEncoding, MemoryStream> is(ms);
        ParseStream<parseFlags, SourceEncoding>(is);
        return *this;
    }

    template <unsigned parseFlags>
    GenericDocument& Parse(const Ch* str, size_t length) {
        return Parse<parseFlags, Encoding>(str, length);
    }
    
    GenericDocument& Parse(const Ch* str, size_t length) {
        return Parse<kParseDefaultFlags>(str, length);
    }

#if RAPIDJSON_HAS_STDSTRING
    template <unsigned parseFlags, typename SourceEncoding>
    GenericDocument& Parse(const std::basic_string<typename SourceEncoding::Ch>& str) {
        // c_str() is constant complexity according to standard. Should be faster than Parse(const char*, size_t)
        return Parse<parseFlags, SourceEncoding>(str.c_str());
    }

    template <unsigned parseFlags>
    GenericDocument& Parse(const std::basic_string<Ch>& str) {
        return Parse<parseFlags, Encoding>(str.c_str());
    }

    GenericDocument& Parse(const std::basic_string<Ch>& str) {
        return Parse<kParseDefaultFlags>(str);
    }
#endif // RAPIDJSON_HAS_STDSTRING    

    //!@}

    //!@name Handling parse errors
    //!@{

    //! Whether a parse error has occurred in the last parsing.
    bool HasParseError() const { return parseResult_.IsError(); }

    //! Get the \ref ParseErrorCode of last parsing.
    ParseErrorCode GetParseError() const { return parseResult_.Code(); }

    //! Get the position of last parsing error in input, 0 otherwise.
    size_t GetErrorOffset() const { return parseResult_.Offset(); }

    //! Implicit conversion to get the last parse result
#ifndef __clang // -Wdocumentation
    /*! \return \ref ParseResult of the last parse operation

        \code
          Document doc;
          ParseResult ok = doc.Parse(json);
          if (!ok)
            printf( "JSON parse error: %s (%u)\n", GetParseError_En(ok.Code()), ok.Offset());
        \endcode
     */
#endif
    operator ParseResult() const { return parseResult_; }
    //!@}

    //! Get the allocator of this document.
    Allocator& GetAllocator() {
        RAPIDJSON_ASSERT(allocator_);
        return *allocator_;
    }

    //! Get the capacity of stack in bytes.
    size_t GetStackCapacity() const { return stack_.GetCapacity(); }

private:
    // clear stack on any exit from ParseStream, e.g. due to exception
    struct ClearStackOnExit {
        explicit ClearStackOnExit(GenericDocument& d) : d_(d) {}
        ~ClearStackOnExit() { d_.ClearStack(); }
    private:
        ClearStackOnExit(const ClearStackOnExit&);
        ClearStackOnExit& operator=(const ClearStackOnExit&);
        GenericDocument& d_;
    };

    // callers of the following private Handler functions
    // template <typename,typename,typename> friend class GenericReader; // for parsing
    template <typename, typename> friend class GenericValue; // for deep copying

public:
    // Implementation of Handler
    bool Null() { RAPIDJSON_YGG_DOCUMENT_NOARG_(Null); new (stack_.template Push<ValueType>()) ValueType(); return true; }
    bool Bool(bool b) { RAPIDJSON_YGG_DOCUMENT_(Bool, b); new (stack_.template Push<ValueType>()) ValueType(b); return true; }
    bool Int(int i) { RAPIDJSON_YGG_DOCUMENT_(Int, i); new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Uint(unsigned i) { RAPIDJSON_YGG_DOCUMENT_(Uint, i); new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Int64(int64_t i) { RAPIDJSON_YGG_DOCUMENT_(Int64, i); new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Uint64(uint64_t i) { RAPIDJSON_YGG_DOCUMENT_(Uint64, i); new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Double(double d) { RAPIDJSON_YGG_DOCUMENT_(Double, d); new (stack_.template Push<ValueType>()) ValueType(d); return true; }

    bool RawNumber(const Ch* str, SizeType length, bool copy) { 
        if (copy) 
            new (stack_.template Push<ValueType>()) ValueType(str, length, GetAllocator());
        else
            new (stack_.template Push<ValueType>()) ValueType(str, length);
        return true;
    }
#ifdef RAPIDJSON_YGGDRASIL
  bool FromYggdrasilString(const Ch* str, SizeType length, bool copy) {
    if (!isYggdrasilString(str, length, copy))
      return false;
    GenericStringBuffer<Encoding, Allocator> os_body(&GetAllocator());
    GenericStringBuffer<Encoding, Allocator> os_schema(&GetAllocator());
    if (!parseYggdrasilString<Encoding, GenericStringBuffer<Encoding, Allocator> >(str, length, copy, os_body, os_schema))
      return false;
    ValueType* x = new (stack_.template Push<ValueType>()) ValueType(
	os_body.GetString(), (SizeType)(os_body.GetLength()),
	GetAllocator(),
	os_schema.GetString(), (SizeType)(os_schema.GetLength()));
    if ((x->IsPythonInstance() || x->IsSchema()) &&
	(os_body.GetLength() > 0) &&
	(os_body.GetString()[0] == '{')) {
      GenericDocument x_obj(&GetAllocator());
      x_obj.Parse(os_body.GetString());
      x->SetObjectRaw(x_obj.GetMembersPointer(), x_obj.MemberCount(), GetAllocator());
    }
    return true;
  }
  bool WasFinalized() const { return (stack_.GetSize() == 0); }
  void ConsolidateStack() {
    // This is not perfect as it dosn't account for the presence of empty
    // arrays or objects
    if (stack_.GetSize() > sizeof(ValueType)) {
      ValueType* v = stack_.template Top<ValueType>();
      while (true) {
	if (v->IsArray() && v->Size() == 0) {
	  SizeType N = (SizeType)(stack_.template Top<ValueType>() - v);
	  EndArray(N);
	} else if (v->IsObject() && v->MemberCount() == 0) {
	  SizeType N = (SizeType)(stack_.template Top<ValueType>() - v);
	  size_t i = 0;
	  bool allStrings = true;
	  for (ValueType* k = v + 1; k != stack_.template End<ValueType>();
	       k++, i++) {
	    if ((i % 2) == 0 && !k->IsString()) {
	      allStrings = false;
	      break;
	    }
	  }
	  if (allStrings) {
	    if (N % 2) {
	      Null(); // Add a buffer null
	      N++;
	    }
	    EndObject(N / 2);
	  }
	}
	if (v == stack_.template Bottom<ValueType>())
	  break;
	v--;
      }
    }
  }
  void FinalizeFromStack() {
    if (stack_.GetSize() > 0) {
      RAPIDJSON_ASSERT(stack_.GetSize() == sizeof(ValueType)); // Got one and only one root object
      ValueType::operator=(*stack_.template Pop<ValueType>(1));// Move value from stack to document
    }
  }
  template <typename StackWriter>
  void RecordStack(StackWriter& writer) const {
    writer.StartArray();
    for (const ValueType* v = stack_.template Bottom<ValueType>();
	 v != stack_.template End<ValueType>(); v++)
      v->Accept(writer);
    writer.EndArray(static_cast<SizeType>(StackSize()));
  }
  ValueType* StackPop() {
    RAPIDJSON_ASSERT(!stack_.Empty());
    return stack_.template Pop<ValueType>(1);
  }
  const ValueType* StackTop() const {
    RAPIDJSON_ASSERT(!stack_.Empty());
    return stack_.template Top<ValueType>();
  }
  ValueType* StackTop() {
    RAPIDJSON_ASSERT(!stack_.Empty());
    return stack_.template Top<ValueType>();
  }
  const ValueType* StackBottom() const {
    RAPIDJSON_ASSERT(!stack_.Empty());
    return stack_.template Bottom<ValueType>();
  }
  ValueType* StackBottom() {
    RAPIDJSON_ASSERT(!stack_.Empty());
    return stack_.template Bottom<ValueType>();
  }
  size_t StackSize() const {
    return stack_.GetSize() / sizeof(ValueType);
  }
  // TODO: Version where schema not provided when setting from existing
  //   populated document.
  size_t CountVarArgs(ValueType& schema, bool set) {
    VarArgList ap;
    ap.is_empty = true;
    uint16_t flag = kCountVarArgsFlag;
    if (set)
      flag |= kSetVarArgsFlag;
    else
      flag |= kGetVarArgsFlag;
    if (ApplyVarArgs(schema, ap, flag))
      return ap.get_nargs();
    return 0;
  }
  bool SkipVarArgs(ValueType& schema, VarArgList& ap, bool set) {
    uint16_t flag = kSkipVarArgsFlag;
    if (set)
      flag |= kSetVarArgsFlag;
    else
      flag |= kGetVarArgsFlag;
    return ApplyVarArgs(schema, ap, flag);
  }
  bool SetVarArgs(ValueType& schema, VarArgList& ap) {
    return ApplyVarArgs(schema, ap, kSetVarArgsFlag);
  }
  bool SetVarArgs(ValueType* schema, ...) {
    size_t nargs = CountVarArgs(*schema, true);
    RAPIDJSON_BEGIN_VAR_ARGS(ap, schema, &nargs, false);
    bool out = SetVarArgs(*schema, ap);
    RAPIDJSON_END_VAR_ARGS(ap);
    return out;
  }
  bool SetVarArgsRealloc(ValueType& schema, VarArgList& ap) {
    return ApplyVarArgs(schema, ap, kSetVarArgsFlag);
  }
  bool SetVarArgsRealloc(ValueType* schema, ...) {
    size_t nargs = CountVarArgs(*schema, true);
    RAPIDJSON_BEGIN_VAR_ARGS(ap, schema, &nargs, true);
    bool out = SetVarArgsRealloc(*schema, ap);
    RAPIDJSON_END_VAR_ARGS(ap);
    return out;
  }
  bool GetVarArgs(ValueType& schema, VarArgList& ap) {
    return ApplyVarArgs(schema, ap, kGetVarArgsFlag);
  }
  bool GetVarArgs(ValueType* schema, ...) {
    size_t nargs = CountVarArgs(*schema, false);
    RAPIDJSON_BEGIN_VAR_ARGS(ap, schema, &nargs, false);
    bool out = GetVarArgs(*schema, ap);
    RAPIDJSON_END_VAR_ARGS(ap);
    return out;
  }
  bool ApplyVarArgs(ValueType& schema, VarArgList &ap,
		    const uint16_t flag) {
    return ValueType::ApplyVarArgs(schema, ap, flag, this);
  }
		    
#endif // RAPIDJSON_YGGDRASIL

    bool String(const Ch* str, SizeType length, bool copy) {
        RAPIDJSON_YGG_DOCUMENT_(String, str, length, copy);
#ifdef RAPIDJSON_YGGDRASIL
	if (FromYggdrasilString(str, length, copy)) return true;
#endif // RAPIDJSON_YGGDRASIL
        if (copy) 
            new (stack_.template Push<ValueType>()) ValueType(str, length, GetAllocator());
        else
            new (stack_.template Push<ValueType>()) ValueType(str, length);
        return true;
    }

    bool StartObject() { RAPIDJSON_YGG_DOCUMENT_NOARG_(StartObject); new (stack_.template Push<ValueType>()) ValueType(kObjectType); return true; }
    
    bool Key(const Ch* str, SizeType length, bool copy) { RAPIDJSON_YGG_DOCUMENT_(Key, str, length, copy); return String(str, length, copy); }

    bool EndObject(SizeType memberCount) {
        RAPIDJSON_YGG_DOCUMENT_(EndObject, memberCount);
        typename ValueType::Member* members = stack_.template Pop<typename ValueType::Member>(memberCount);
        stack_.template Top<ValueType>()->SetObjectRaw(members, memberCount, GetAllocator());
        return true;
    }

    bool StartArray() { RAPIDJSON_YGG_DOCUMENT_NOARG_(StartArray); new (stack_.template Push<ValueType>()) ValueType(kArrayType); return true; }
    
    bool EndArray(SizeType elementCount) {
        RAPIDJSON_YGG_DOCUMENT_(EndArray, elementCount);
        ValueType* elements = stack_.template Pop<ValueType>(elementCount);
        stack_.template Top<ValueType>()->SetArrayRaw(elements, elementCount, GetAllocator());
        return true;
    }

private:
    //! Prohibit copying
    GenericDocument(const GenericDocument&);
    //! Prohibit assignment
    GenericDocument& operator=(const GenericDocument&);

#ifdef RAPIDJSON_YGGDRASIL
public:
#endif // RAPIDJSON_YGGDRASIL
    void ClearStack() {
        if (Allocator::kNeedFree)
            while (stack_.GetSize() > 0)    // Here assumes all elements in stack array are GenericValue (Member is actually 2 GenericValue objects)
                (stack_.template Pop<ValueType>(1))->~ValueType();
        else
            stack_.Clear();
        stack_.ShrinkToFit();
    }

#ifdef RAPIDJSON_YGGDRASIL
private:
#endif // RAPIDJSON_YGGDRASIL
    void Destroy() {
        RAPIDJSON_DELETE(ownAllocator_);
    }

    static const size_t kDefaultStackCapacity = 1024;
    Allocator* allocator_;
    Allocator* ownAllocator_;
    internal::Stack<StackAllocator> stack_;
    ParseResult parseResult_;

#ifdef RAPIDJSON_YGGDRASIL
public:
  GenericDocument& Parse(const Ch* str, GenericDocument& schema) {
    GenericDocument& out = Parse(str);
    this->AddSchema(schema);
    return out;
  }
  template <typename YggSchemaValueType>
  bool YggdrasilString(const Ch* str, SizeType length, bool copy, YggSchemaValueType& schema) {
    RAPIDJSON_YGG_DOCUMENT_(YggdrasilString, str, length, copy, schema);
    if (copy) 
      new (stack_.template Push<ValueType>()) ValueType(str, length, GetAllocator(), schema);
    else
      new (stack_.template Push<ValueType>()) ValueType(str, length, schema);
    return true;
  }
  template <typename YggSchemaValueType>
  bool YggdrasilStartObject(YggSchemaValueType& schema) {
    RAPIDJSON_YGG_DOCUMENT_(YggdrasilStartObject, schema);
    new (stack_.template Push<ValueType>()) ValueType(kObjectType, schema);
    return true;
  }
  bool YggdrasilEndObject(SizeType memberCount) {
    RAPIDJSON_YGG_DOCUMENT_(YggdrasilEndObject, memberCount);
    return EndObject(memberCount);
  }

#endif // RAPIDJSON_YGGDRASIL

#undef RAPIDJSON_YGG_DOCUMENT_
#undef RAPIDJSON_YGG_DOCUMENT_NOARG_
#undef RAPIDJSON_YGG_DOCUMENT_BASE_

};

//! GenericDocument with UTF8 encoding
typedef GenericDocument<UTF8<> > Document;


//! Helper class for accessing Value of array type.
/*!
    Instance of this helper class is obtained by \c GenericValue::GetArray().
    In addition to all APIs for array type, it provides range-based for loop if \c RAPIDJSON_HAS_CXX11_RANGE_FOR=1.
*/
template <bool Const, typename ValueT>
class GenericArray {
public:
    typedef GenericArray<true, ValueT> ConstArray;
    typedef GenericArray<false, ValueT> Array;
    typedef ValueT PlainType;
    typedef typename internal::MaybeAddConst<Const,PlainType>::Type ValueType;
    typedef ValueType* ValueIterator;  // This may be const or non-const iterator
    typedef const ValueT* ConstValueIterator;
    typedef typename ValueType::AllocatorType AllocatorType;
    typedef typename ValueType::StringRefType StringRefType;

    template <typename, typename>
    friend class GenericValue;

    GenericArray(const GenericArray& rhs) : value_(rhs.value_) {}
    GenericArray& operator=(const GenericArray& rhs) { value_ = rhs.value_; return *this; }
    ~GenericArray() {}

    operator ValueType&() const { return value_; }
    SizeType Size() const { return value_.Size(); }
    SizeType Capacity() const { return value_.Capacity(); }
    bool Empty() const { return value_.Empty(); }
    void Clear() const { value_.Clear(); }
    ValueType& operator[](SizeType index) const {  return value_[index]; }
    ValueIterator Begin() const { return value_.Begin(); }
    ValueIterator End() const { return value_.End(); }
    GenericArray Reserve(SizeType newCapacity, AllocatorType &allocator) const { value_.Reserve(newCapacity, allocator); return *this; }
    GenericArray PushBack(ValueType& value, AllocatorType& allocator) const { value_.PushBack(value, allocator); return *this; }
#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericArray PushBack(ValueType&& value, AllocatorType& allocator) const { value_.PushBack(value, allocator); return *this; }
#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericArray PushBack(StringRefType value, AllocatorType& allocator) const { value_.PushBack(value, allocator); return *this; }
    template <typename T> RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (const GenericArray&)) PushBack(T value, AllocatorType& allocator) const { value_.PushBack(value, allocator); return *this; }
    GenericArray PopBack() const { value_.PopBack(); return *this; }
    ValueIterator Erase(ConstValueIterator pos) const { return value_.Erase(pos); }
    ValueIterator Erase(ConstValueIterator first, ConstValueIterator last) const { return value_.Erase(first, last); }

#if RAPIDJSON_HAS_CXX11_RANGE_FOR
    ValueIterator begin() const { return value_.Begin(); }
    ValueIterator end() const { return value_.End(); }
#endif

private:
    GenericArray();
    GenericArray(ValueType& value) : value_(value) {}
    ValueType& value_;
};

//! Helper class for accessing Value of object type.
/*!
    Instance of this helper class is obtained by \c GenericValue::GetObject().
    In addition to all APIs for array type, it provides range-based for loop if \c RAPIDJSON_HAS_CXX11_RANGE_FOR=1.
*/
template <bool Const, typename ValueT>
class GenericObject {
public:
    typedef GenericObject<true, ValueT> ConstObject;
    typedef GenericObject<false, ValueT> Object;
    typedef ValueT PlainType;
    typedef typename internal::MaybeAddConst<Const,PlainType>::Type ValueType;
    typedef GenericMemberIterator<Const, typename ValueT::EncodingType, typename ValueT::AllocatorType> MemberIterator;  // This may be const or non-const iterator
    typedef GenericMemberIterator<true, typename ValueT::EncodingType, typename ValueT::AllocatorType> ConstMemberIterator;
    typedef typename ValueType::AllocatorType AllocatorType;
    typedef typename ValueType::StringRefType StringRefType;
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename ValueType::Ch Ch;

    template <typename, typename>
    friend class GenericValue;

    GenericObject(const GenericObject& rhs) : value_(rhs.value_) {}
    GenericObject& operator=(const GenericObject& rhs) { value_ = rhs.value_; return *this; }
    ~GenericObject() {}

    operator ValueType&() const { return value_; }
    SizeType MemberCount() const { return value_.MemberCount(); }
    SizeType MemberCapacity() const { return value_.MemberCapacity(); }
    bool ObjectEmpty() const { return value_.ObjectEmpty(); }
    template <typename T> ValueType& operator[](T* name) const { return value_[name]; }
    template <typename SourceAllocator> ValueType& operator[](const GenericValue<EncodingType, SourceAllocator>& name) const { return value_[name]; }
#if RAPIDJSON_HAS_STDSTRING
    ValueType& operator[](const std::basic_string<Ch>& name) const { return value_[name]; }
#endif
    MemberIterator MemberBegin() const { return value_.MemberBegin(); }
    MemberIterator MemberEnd() const { return value_.MemberEnd(); }
    GenericObject MemberReserve(SizeType newCapacity, AllocatorType &allocator) const { value_.MemberReserve(newCapacity, allocator); return *this; }
    bool HasMember(const Ch* name) const { return value_.HasMember(name); }
#if RAPIDJSON_HAS_STDSTRING
    bool HasMember(const std::basic_string<Ch>& name) const { return value_.HasMember(name); }
#endif
    template <typename SourceAllocator> bool HasMember(const GenericValue<EncodingType, SourceAllocator>& name) const { return value_.HasMember(name); }
    MemberIterator FindMember(const Ch* name) const { return value_.FindMember(name); }
    template <typename SourceAllocator> MemberIterator FindMember(const GenericValue<EncodingType, SourceAllocator>& name) const { return value_.FindMember(name); }
#if RAPIDJSON_HAS_STDSTRING
    MemberIterator FindMember(const std::basic_string<Ch>& name) const { return value_.FindMember(name); }
#endif
    GenericObject AddMember(ValueType& name, ValueType& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(ValueType& name, StringRefType value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
#if RAPIDJSON_HAS_STDSTRING
    GenericObject AddMember(ValueType& name, std::basic_string<Ch>& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
#endif
    template <typename T> RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&)) AddMember(ValueType& name, T value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericObject AddMember(ValueType&& name, ValueType&& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(ValueType&& name, ValueType& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(ValueType& name, ValueType&& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(StringRefType name, ValueType&& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericObject AddMember(StringRefType name, ValueType& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(StringRefType name, StringRefType value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    template <typename T> RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericObject)) AddMember(StringRefType name, T value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    void RemoveAllMembers() { value_.RemoveAllMembers(); }
    bool RemoveMember(const Ch* name) const { return value_.RemoveMember(name); }
#if RAPIDJSON_HAS_STDSTRING
    bool RemoveMember(const std::basic_string<Ch>& name) const { return value_.RemoveMember(name); }
#endif
    template <typename SourceAllocator> bool RemoveMember(const GenericValue<EncodingType, SourceAllocator>& name) const { return value_.RemoveMember(name); }
    MemberIterator RemoveMember(MemberIterator m) const { return value_.RemoveMember(m); }
    MemberIterator EraseMember(ConstMemberIterator pos) const { return value_.EraseMember(pos); }
    MemberIterator EraseMember(ConstMemberIterator first, ConstMemberIterator last) const { return value_.EraseMember(first, last); }
    bool EraseMember(const Ch* name) const { return value_.EraseMember(name); }
#if RAPIDJSON_HAS_STDSTRING
    bool EraseMember(const std::basic_string<Ch>& name) const { return EraseMember(ValueType(StringRef(name))); }
#endif
    template <typename SourceAllocator> bool EraseMember(const GenericValue<EncodingType, SourceAllocator>& name) const { return value_.EraseMember(name); }

#if RAPIDJSON_HAS_CXX11_RANGE_FOR
    MemberIterator begin() const { return value_.MemberBegin(); }
    MemberIterator end() const { return value_.MemberEnd(); }
#endif

#ifdef RAPIDJSON_YGGDRASIL
  template <typename Handler>
  bool Accept(Handler& handler, bool skip_yggdrasil=false) const {
    return value_.Accept(handler, skip_yggdrasil);
  }
#endif // RAPIDJSON_YGGDRASIL

private:
    GenericObject();
    GenericObject(ValueType& value) : value_(value) {}
    ValueType& value_;
};

#ifdef RAPIDJSON_YGGDRASIL
  
#define ENCODING_STRING_(name, ...)			\
  const Ch name ## String[] = { __VA_ARGS__, '\0' }
#define COMPARE_(x, dec)						\
  (std::basic_string<Ch>(x) == std::basic_string<Ch>(dec ## String))
#define SWITCH_(x, CASE_)						\
  CASE_(x, ASCII, ASCII)						\
  else CASE_(x, ASCII, Null)						\
  else CASE_(x, UTF8, UTF8)						\
  else CASE_(x, UTF16, UTF16)						\
  else CASE_(x, UTF32, UTF32)						\
  else CASE_(x, UTF32, UCS4)						\
  else {								\
    return false;							\
  }
template <typename SourceEncoding, typename DestEncoding, typename AllocatorType>
inline bool TranslateEncoding_inner(const void* src, SizeType srcNbytes,
				    void*& dst, SizeType& dstNbytes,
				    AllocatorType& allocator,
				    bool requireFixedWidth = false) {
  if (requireFixedWidth && (!DestEncoding::fixedWidth || !SourceEncoding::fixedWidth))
    return false;
  SizeType srcLength = srcNbytes / ((SizeType)sizeof(typename SourceEncoding::Ch));
  GenericStringStream<SourceEncoding> is((const typename SourceEncoding::Ch*)src);
  GenericStringBuffer<DestEncoding> os;
  while (RAPIDJSON_LIKELY(is.Tell() < srcLength)) {
    Transcoder<SourceEncoding, DestEncoding>::Transcode(is, os);
  }
  dstNbytes = (SizeType)os.GetLength() * (SizeType)sizeof(typename DestEncoding::Ch);
  dst = allocator.Malloc(dstNbytes);
  if (!dst) return false;
  memcpy(dst, os.GetString(), dstNbytes);
  return true;
}
template <typename SourceEncoding, typename Ch, typename AllocatorType>
inline bool TranslateEncoding_outer(const void* src, SizeType srcNbytes,
				    void*& dst, SizeType& dstNbytes, const Ch* dstEncoding,
				    AllocatorType& allocator,
				    bool requireFixedWidth = false) {
  ENCODING_STRING_(ASCII, 'A', 'S', 'C', 'I', 'I');
  ENCODING_STRING_(UTF8, 'U', 'T', 'F', '8');
  ENCODING_STRING_(UTF16, 'U', 'T', 'F', '1', '6');
  ENCODING_STRING_(UTF32, 'U', 'T', 'F', '3', '2');
  ENCODING_STRING_(UCS4, 'U', 'C', 'S', '4');
  ENCODING_STRING_(Null, 'n', 'u', 'l', 'l');
#define INNER_CASE_(x, dec, decS)					\
  if (COMPARE_(x, decS)) {						\
    return TranslateEncoding_inner<SourceEncoding, dec<> >(src, srcNbytes, dst, dstNbytes, allocator, requireFixedWidth); \
  }
  SWITCH_(dstEncoding, INNER_CASE_)
}
template <typename Ch, typename AllocatorType>
inline bool TranslateEncoding(const void* src, SizeType srcNbytes, const Ch* srcEncoding,
			      void*& dst, SizeType& dstNbytes, const Ch* dstEncoding,
			      AllocatorType& allocator,
			      bool requireFixedWidth = false) {
  ENCODING_STRING_(ASCII, 'A', 'S', 'C', 'I', 'I');
  ENCODING_STRING_(UTF8, 'U', 'T', 'F', '8');
  ENCODING_STRING_(UTF16, 'U', 'T', 'F', '1', '6');
  ENCODING_STRING_(UTF32, 'U', 'T', 'F', '3', '2');
  ENCODING_STRING_(UCS4, 'U', 'C', 'S', '4');
  ENCODING_STRING_(Null, 'n', 'u', 'l', 'l');
#define OUTER_CASE_(x, dec, decS)					\
  if (COMPARE_(x, decS)) {						\
    return TranslateEncoding_outer<dec<> >(src, srcNbytes, dst, dstNbytes, dstEncoding, allocator, requireFixedWidth); \
  }
  SWITCH_(srcEncoding, OUTER_CASE_)
}
#undef SWITCH_
#undef OUTER_CASE_
#undef INNER_CASE_
#undef COMPARE_
#undef ENCODING_STRING_


#endif // RAPIDJSON_YGGDRASIL
  
RAPIDJSON_NAMESPACE_END
RAPIDJSON_DIAG_POP

#ifdef RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#pragma pop_macro("GetObject")
#undef RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#endif

#endif // RAPIDJSON_DOCUMENT_H_
