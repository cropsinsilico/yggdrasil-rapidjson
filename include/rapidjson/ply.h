#ifndef RAPIDJSON_PLY_H_
#define RAPIDJSON_PLY_H_

#include <iostream>

RAPIDJSON_NAMESPACE_BEGIN

#if RAPIDJSON_ENDIAN == RAPIDJSON_LITTLEENDIAN
#define NUMBER_MEMBER_(name, member, type, ptype1, ptype2, ptype3)	\
  struct name {								\
    name(const type x) : v(x), pad1(0), pad2(0), pad3(0) {}		\
    name() : name(0) {}							\
    name(std::istream &in) : name() { read(in); }			\
    std::ostream & write(std::ostream &out) const {			\
      out << v;								\
      return out;							\
    }									\
    std::istream & read(std::istream &in) {				\
      in >> v;								\
      return in;							\
    }									\
    bool is_equal(const name &y) const {				\
      return (abs((double)(v - y.v)) < 0.01); }				\
    type v;								\
    ptype1 pad1;							\
    ptype2 pad2;							\
    ptype3 pad3;							\
  } member
#else
// #define NUMBER_MEMBER_(name, member, type, padding_size)	\
//   struct name {							\
//     name(const type x) : padding(""), v(x) {}			\
//     name() : name(0) {}						\
//     char padding[padding_size];					\
//     type v;							\
//   } member
#endif

#define NUMBER_DATA_SWITCH_(x, flag, before, after)			\
  switch (flag) {							\
  case (kInt8Flag) : before x.i8.v after; break;			\
  case (kUint8Flag) : before x.u8.v after; break;			\
  case (kInt16Flag) : before x.i16.v after; break;			\
  case (kUint16Flag) : before x.u16.v after; break;			\
  case (kInt32Flag) : before x.i32.v after; break;			\
  case (kUint32Flag) : before x.u32.v after; break;			\
  case (kFloatFlag) : before x.f.v after; break;			\
  case (kDoubleFlag) : before x.d after; break;				\
  default: RAPIDJSON_ASSERT(false);					\
  }
#define NUMBER_DATA_COMPARE_(x, y, flag, var)				\
  switch (flag) {							\
  case (kInt8Flag) : var = (x.i8.v == y.i8.v); break;			\
  case (kUint8Flag) : var = (x.u8.v == y.u8.v); break;			\
  case (kInt16Flag) : var = (x.i16.v == y.i16.v); break;		\
  case (kUint16Flag) : var = (x.u16.v == y.u16.v); break;		\
  case (kInt32Flag) : var = (x.i32.v == y.i32.v); break;		\
  case (kUint32Flag) : var = (x.u32.v == y.u32.v); break;		\
  case (kFloatFlag) : var = (fabs(x.f.v - y.f.v) < 0.01f); break;	\
  case (kDoubleFlag) : var = (abs(x.d - y.d) < 0.01); break;		\
  default: RAPIDJSON_ASSERT(false);					\
  }
// Forward declarations
class PlyElement;
class PlyElementSet;
template<typename T>
inline uint16_t type2flag();
template<>
inline uint16_t type2flag<int8_t>();
template<>
inline uint16_t type2flag<uint8_t>();
template<>
inline uint16_t type2flag<int16_t>();
template<>
inline uint16_t type2flag<uint16_t>();
template<>
inline uint16_t type2flag<int32_t>();
template<>
inline uint16_t type2flag<uint32_t>();
template<>
inline uint16_t type2flag<float>();
template<>
inline uint16_t type2flag<double>();

//! Generic ply geometry element
class PlyElement {
public:
  //! Empty constructor.
  PlyElement() : property_order(), properties() {}
  //! \brief Copy constructor.
  //! \param rhs Element to copy.
  PlyElement(const PlyElement &rhs) : property_order(rhs.property_order), properties(rhs.properties) {}
  //! \brief Create an element by reading from an input stream.
  //! \param property_order0 The names of properties of this element in the
  //!   order they should be read.
  //! \param properties0 Mapping between element properties and a flag
  //!   indicating the data type used for the property.
  //! \param in Input stream.
  PlyElement(const std::vector<std::string> &property_order0,
	     const std::map<std::string, uint16_t> &properties0,
	     std::istream &in) :
    PlyElement() {
    read(property_order0, properties0, in);
  }
  //! \brief Create an element from a vector of property values.
  //! \tparam Type of property values.
  //! \param property_order0 The names of properties of this element in the
  //!   order they are in arr.
  //! \param properties0 Mapping between element properties and a flag
  //!   indicating the data type used for the property.
  //! \param arr Property values.
  template<typename T>
  PlyElement(const std::vector<std::string> &property_order0,
	     const std::map<std::string, uint16_t> &properties0,
	     const std::vector<T> &arr) :
    PlyElement() {
    size_t N = arr.size();
    size_t i = 0;
    for (auto name = property_order0.begin(); name != property_order0.end(); name++, i++) {
      property_order.push_back(*name);
      auto it = properties0.find(*name);
      RAPIDJSON_ASSERT(it != properties0.end());
      RAPIDJSON_ASSERT(i < N);
      if ((it != properties0.end()) && (i < arr.size())) {
	if (it->second & kListFlag) {
	  properties.emplace(std::piecewise_construct,
			     std::forward_as_tuple(it->first),
			     std::forward_as_tuple(it->second, arr));
	  i = i + N;
	} else {
	  properties.emplace(std::piecewise_construct,
			     std::forward_as_tuple(it->first),
			     std::forward_as_tuple(it->second, arr[i]));
	}
      }
    }
  }

private:
  enum ElementType : uint16_t {
    kNullFlag       = 0x0000,
    kInt8Flag       = 0x0008,
    kUint8Flag      = 0x0010,
    kInt16Flag      = 0x0020,
    kUint16Flag     = 0x0040,
    kInt32Flag      = 0x0080,
    kUint32Flag     = 0x0100,
    kFloatFlag      = 0x0200,
    kDoubleFlag     = 0x0400,
    kListFlag       = 0x0800,
  };
  struct Number {
    int64_t i64;
    NUMBER_MEMBER_(F, f, float, int16_t, int8_t, int8_t);
    NUMBER_MEMBER_(I8, i8, int8_t, int8_t, int16_t, int32_t);
    NUMBER_MEMBER_(U8, u8, uint8_t, int8_t, int16_t, int32_t);
    NUMBER_MEMBER_(I16, i16, int16_t, int16_t, int16_t, int16_t);
    NUMBER_MEMBER_(U16, u16, uint16_t, int16_t, int16_t, int16_t);
    NUMBER_MEMBER_(I32, i32, int32_t, int16_t, int8_t, int8_t);
    NUMBER_MEMBER_(U32, u32, uint32_t, int16_t, int8_t, int8_t);
    double d;
    Number() : i64(0), f(0.0), i8(0), u8(0), i16(0), u16(0), i32(0), u32(0), d(0.0) {}
    // Number() : Number(0) { memset(this, 0, sizeof(Number)); }
    //! \brief Create a zeroed number instance.
    //! \param flag Flag indicating what type to store 0 as.
    Number(const uint16_t &flag) : Number() { this->assign(flag, 0); }
    //! \brief Read number data from an input stream.
    //! \param flag Flag indicating what type of data to read.
    //! \param in Input stream.
    Number(const uint16_t &flag, std::istream &in) : Number(flag) {
      this->read(flag, in);
    }
    //! \brief Create an number instance from a scalar.
    //! \tparam T Type of scalar.
    //! \param flag Flag indicating what type to store x as.
    //! \param x Scalar data to store.
    template <typename T>
    Number(const uint16_t &flag, const T &x) : Number(flag) {
      this->assign(flag, x);
    }
    //! \brief Assign a scalar value to this instance.
    //! \tparam T Type of scalar.
    //! \param flag Flag indicating what type to store x as.
    //! \param x Scalar data to store.
    template <typename T>
    void assign(const uint16_t &flag, const T &x) {
      switch (flag) {
      case (kInt8Flag) : i8 = I8((int8_t)(x)); break;
      case (kUint8Flag) : u8 = U8((uint8_t)(x)); break;
      case (kInt16Flag) : i16 = I16((int16_t)(x)); break;
      case (kUint16Flag) : u16 = U16((uint16_t)(x)); break;
      case (kInt32Flag) : i32 = I32((int32_t)(x)); break;
      case (kUint32Flag) : u32 = U32((uint32_t)(x)); break;
      case (kFloatFlag) : f = F((float)(x)); break;
      case (kDoubleFlag) : d = (double)(x); break;
      default: i64 = (int64_t)(x); break;
      }
    }
    //! \brief Read number data from an input stream.
    //! \param flag Flag indicating the type of data to read.
    //! \param in Input stream.
    //! \return Input stream.
    std::istream & read(const uint16_t &flag, std::istream &in) {
      switch (flag) {
      case (kInt8Flag) : return i8.read(in);
      case (kUint8Flag) : return u8.read(in);
      case (kInt16Flag) : return i16.read(in);
      case (kUint16Flag) : return u16.read(in);
      case (kInt32Flag) : return i32.read(in);
      case (kUint32Flag) : return u32.read(in);
      case (kFloatFlag) : return f.read(in);
      case (kDoubleFlag) : in >> d; return in;
      default: RAPIDJSON_ASSERT(false);
      }
      return in;
    }
    //! \brief Write number data to an output stream.
    //! \param flag Flag indicating the type of data in the instance.
    //! \param out Output stream.
    //! \return Output stream.
    std::ostream & write(const uint16_t &flag, std::ostream &out) const {
      switch (flag) {
      case (kInt8Flag) : return i8.write(out);
      case (kUint8Flag) : return u8.write(out);
      case (kInt16Flag) : return i16.write(out);
      case (kUint16Flag) : return u16.write(out);
      case (kInt32Flag) : return i32.write(out);
      case (kUint32Flag) : return u32.write(out);
      case (kFloatFlag) : return f.write(out);
      case (kDoubleFlag) : out << d; return out;
      default: RAPIDJSON_ASSERT(false);
      }
      return out;
    }
    //! \brief Check if this number is equivalent to another.
    //! \param flag Flag indicating the type of data in both numbers.
    //! \param y Instance for comparison.
    //! \return true if this instances is equivalent to y.
    bool is_equal(const uint16_t &flag, const Number& y) const {
      switch (flag) {
      case (kInt8Flag) : return i8.is_equal(y.i8);
      case (kUint8Flag) : return u8.is_equal(y.u8);
      case (kInt16Flag) : return i16.is_equal(y.i16);
      case (kUint16Flag) : return u16.is_equal(y.u16);
      case (kInt32Flag) : return i32.is_equal(y.i32);
      case (kUint32Flag) : return u32.is_equal(y.u32);
      case (kFloatFlag) : return f.is_equal(y.f);
      case (kDoubleFlag) : return (abs(d - y.d) < 0.01);
      default: RAPIDJSON_ASSERT(false);
      }
      return false;
    }
  }; // 8 bytes
  struct Data {
    Data() : Data(0) {}  //  memset(this, 0, sizeof(Data)); }
    //! \brief Create an empty data instance with type information.
    //! \param flag Flag indicating the type that should be used to store the
    //!    data.
    Data(const uint16_t flag) : f(flag), n(flag), elements() {}
    //! \brief Read data value(s) from an input stream.
    //! \param flag Flag indicating the type that should be used to store the
    //!    data.
    //! \param in Input stream.
    Data(const uint16_t flag, std::istream &in) : Data(flag) {
      if (flag & kListFlag) {
	uint16_t element_flags = (uint16_t)(flag & ~kListFlag);
	size_t size = 0;
	in >> size;
	for (size_t i = 0; i < size; i++)
	  elements.emplace_back(element_flags, in);
      } else {
	n.read(flag, in);
      }
    }
    //! \brief Create a data instance from a scalar value.
    //! \tparam T Type of scalar data.
    //! \param flag Flag indicating the type that should be used to store the
    //!    data.
    //! \param x Scalar data.
    template<typename T>
    Data(const uint16_t flag, const T &x) : Data(flag) {
      RAPIDJSON_ASSERT(!(flag & kListFlag));
      n.assign(flag, x);
    }
    //! \brief Create a data instance from a vector of values.
    //! \tparam T Type of data in the vector.
    //! \param flag Flag indicating the type that should be used to store the
    //!    data.
    //! \param x Vector of values.
    template<typename T>
    Data(const uint16_t flag, const std::vector<T> &x) : Data(flag) {
      RAPIDJSON_ASSERT(flag & kListFlag);
      uint16_t element_flags = (uint16_t)(flag & ~kListFlag);
      for (auto it = x.begin(); it != x.end(); it++)
	elements.emplace_back(element_flags, *it);
    }
    //! \brief Write data to an output stream.
    //! \param out Output stream.
    void write(std::ostream &out) const {
      if (f & kListFlag) {
	uint16_t element_flags = (uint16_t)(f & ~kListFlag);
	out << elements.size();
	for (auto it = elements.begin(); it != elements.end(); it++) {
	  out << " ";
	  it->write(element_flags, out);
	}
      } else {
	n.write(f, out);
      }
    }
    //! \brief Check if this data is equivalent to another Data instance.
    //! \param y Instance for comparison.
    //! \return true if this instance is equivalent to y.
    bool is_equal(const Data& y) const {
      if (f != y.f) return false;
      if (f & kListFlag) {
	if (elements.size() != y.elements.size()) return false;
	uint16_t element_flags = (uint16_t)(f & ~kListFlag);
	for (auto it1 = elements.begin(), it2 = y.elements.begin();
	     it1 != elements.end(); it1++, it2++) {
	  if (!(it1->is_equal(element_flags, *it2))) return false;
	}
	return true;
      } else {
	return n.is_equal(f, y.n);
      }
    }
    //! Flag indicating the data type.
    uint16_t f;
    // union {
    //! Numeric data component.
    Number n;
    //! Vector of numeric data components.
    std::vector<Number> elements;
    // };
  };

  //! \brief Check if two Data instances are equivalent.
  //! \param d1 First data instance for comparison.
  //! \param d2 Second data instance for comparison.
  //! \return true if the two instances are equivalent.
  bool is_equal_data(const Data &d1, const Data &d2) const {
    bool out = false;
    if (d1.f != d2.f) return false;
    if (d1.f & kListFlag) {
      uint16_t element_flags = (uint16_t)(d1.f & ~kListFlag);
      if (d1.elements.size() != d2.elements.size()) return false;
      for (auto it1 = d1.elements.begin(), it2 = d2.elements.begin(); it1 != d1.elements.end(); it1++, it2++) {
	NUMBER_DATA_COMPARE_((*it1), (*it2), element_flags, out);
	if (!out) return false;
      }
    } else {
      NUMBER_DATA_COMPARE_(d1.n, d2.n, d1.f, out);
    }
    return out;
  }

  //! \brief Add a property value from a Number instance to a vector.
  //! \param x Number instance.
  //! \param flag Type flag indicating the type of data stored in x.
  //! \param[in,out] out Vector to add property value to.
  template <typename T>
  void extend_aray_data_number(const Number &x, const uint16_t &flag,
			       std::vector<T> &out) const {
    switch (flag) {
    case (kInt8Flag) : out.push_back((T)(x.i8.v)); break;
    case (kUint8Flag) : out.push_back((T)(x.u8.v)); break;
    case (kInt16Flag) : out.push_back((T)(x.i16.v)); break;
    case (kUint16Flag) : out.push_back((T)(x.u16.v)); break;
    case (kInt32Flag) : out.push_back((T)(x.i32.v)); break;
    case (kUint32Flag) : out.push_back((T)(x.u32.v)); break;
    case (kFloatFlag) : out.push_back((T)(x.f.v)); break;
    case (kDoubleFlag) : out.push_back((T)(x.d)); break;
    default: RAPIDJSON_ASSERT(false);
    }
  }

  //! \brief Add property values from a Data instance to a vector.
  //! \param d Data instance.
  //! \param[in,out] out Vector to add property values to.
  template <typename T>
  void extend_aray_data(const Data &d, std::vector<T> &out) const {
    if (d.f & kListFlag) {
      uint16_t element_flags = (uint16_t)(d.f & ~kListFlag);
      for (auto it = d.elements.begin(); it != d.elements.end(); it++)
	extend_aray_data_number(*it, element_flags, out);
    } else {
      extend_aray_data_number(d.n, d.f, out);
    }
  }

  //! Names of properties defining the element in the order they are read/
  //!   written when serialized.
  std::vector<std::string> property_order;
  //! Mapping between property names and the property values.
  std::map<std::string, Data> properties;
  
  friend class PlyElementSet;
  template<typename T>
  friend inline uint16_t type2flag();
  
public:
  //! \brief Convert a ply serialized type name to a type flag.
  //! \param type Ply serialized type name.
  //! \return Type flag.
  static uint16_t typename2flag(const std::string& type) {
    if (type == "list")
      return kListFlag;
    else if (type == "char")
      return kInt8Flag;
    else if (type == "uchar")
      return kUint8Flag;
    else if (type == "short")
      return kInt16Flag;
    else if (type == "ushort")
      return kUint16Flag;
    else if (type == "int")
      return kInt32Flag;
    else if (type == "uint")
      return kUint32Flag;
    else if (type == "float")
      return kFloatFlag;
    else if (type == "double")
      return kDoubleFlag;
    RAPIDJSON_ASSERT(!sizeof(std::string("Unsupported type: ") + type));
    return 0;
  }
  //! \brief Convert a type flag into the ply serialized type name.
  //! \param type Type flag.
  //! \return Ply serialized type name.
  static std::string flag2typename(const uint16_t& type) {
    if (type & kListFlag) {
      return "list";
    }
    switch (type) {
    case kInt8Flag:
      return "char";
    case kUint8Flag:
      return "uchar";
    case kInt16Flag:
      return "short";
    case kUint16Flag:
      return "ushort";
    case kInt32Flag:
      return "int";
    case kUint32Flag:
      return "uint";
    case kFloatFlag:
      return "float";
    case kDoubleFlag:
      return "double";
    default:
      RAPIDJSON_ASSERT(!sizeof(std::string("Unsupported type: ") + std::to_string(type)));
    }
    return "";
  }
  //! \brief Check if this element is equivalent to another.
  //! \param rhs Element for comparison.
  //! \return true if this element is equivalent to rhs.
  bool is_equal(const PlyElement& rhs) const {
    if (this->property_order.size() != rhs.property_order.size())
      return false;
    if (this->properties.size() != rhs.properties.size())
      return false;
    for (size_t i = 0; i < this->property_order.size(); i++) {
      if (this->property_order[i] != rhs.property_order[i])
	return false;
    }
    for (auto lit = this->properties.begin(); lit != this->properties.end(); lit++) {
      auto rit = rhs.properties.find(lit->first);
      RAPIDJSON_ASSERT(rit != rhs.properties.end());
      if (rit == rhs.properties.end())
	return false;
      if (!(is_equal_data(lit->second, rit->second)))
	return false;
    }
    return true;
  }
  //! \brief Write element properties to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    for (auto name = property_order.begin(); name != property_order.end(); name++) {
      auto it = properties.find(*name);
      RAPIDJSON_ASSERT(it != properties.end());
      if (it != properties.end()) {
	if (it != properties.begin())
	  out << " ";
	it->second.write(out);
      }
    }
    out << std::endl;
    return out;
  }
  //! \brief Read in element properties from an input stream.
  //! \param property_order0 Element property names in the order they should
  //!   be read.
  //! \param properties0 Map between property names and a flag indicating
  //!   the data type for the property values.
  //! \param in Input stream.
  //! \return Input stream.
  std::istream & read(const std::vector<std::string> &property_order0,
		      const std::map<std::string, uint16_t> &properties0,
		      std::istream &in) {
    for (auto name = property_order0.begin(); name != property_order0.end(); name++) {
      property_order.push_back(*name);
      auto it = properties0.find(*name);
      RAPIDJSON_ASSERT(it != properties0.end());
      properties.emplace(std::piecewise_construct,
			 std::forward_as_tuple(it->first),
			 std::forward_as_tuple(it->second, in));
    }
    return in;
  }
  //! \brief Retrieve a value of a specific type from a Data instance.
  //! \tparam T Type of values to retrieve.
  //! \param x Data instance.
  //! \return Data value.
  template <typename T>
  static T get_scalar(const Data &x) {
    RAPIDJSON_ASSERT(!(x.f & kListFlag));
    if (x.f & kInt8Flag)
      return (T)(x.n.i8.v);
    else if (x.f & kUint8Flag)
      return (T)(x.n.u8.v);
    else if (x.f & kInt16Flag)
      return (T)(x.n.i16.v);
    else if (x.f & kUint16Flag)
      return (T)(x.n.u16.v);
    else if (x.f & kInt32Flag)
      return (T)(x.n.i32.v);
    else if (x.f & kUint32Flag)
      return (T)(x.n.u32.v);
    else if (x.f & kFloatFlag)
      return (T)(x.n.f.v);
    else if (x.f & kDoubleFlag)
      return (T)(x.n.d);
    RAPIDJSON_ASSERT(!sizeof("Cannot get scalar for type"));
    return T(0);
  }
  //! \brief Get element values as an array of ints.
  //! \return Array of int values.
  std::vector<int> get_int_array() const {
    std::vector<int> out;
    for (auto name = property_order.begin(); name != property_order.end(); name++) {
      auto it = properties.find(*name);
      RAPIDJSON_ASSERT(it != properties.end());
      extend_aray_data(it->second, out);
    }
    return out;
  }
  //! \brief Get element values as an array of doubles.
  //! \return Array of double values.
  std::vector<double> get_double_array() const {
    std::vector<double> out;
    for (auto name = property_order.begin(); name != property_order.end(); name++) {
      auto it = properties.find(*name);
      RAPIDJSON_ASSERT(it != properties.end());
      extend_aray_data(it->second, out);
    }
    return out;
  }
  
  friend bool operator == (const PlyElement& lhs, const PlyElement& rhs);
};

//! \brief Check if two PlyElement instances are equivalent.
//! \param lhs First element for comparison.
//! \param rhs Second element for comparison.
//! \return true if the two elements are equivalent.
inline
bool operator == (const PlyElement& lhs, const PlyElement& rhs)
{ return lhs.is_equal(rhs); };

template<typename T>
inline uint16_t type2flag() { return PlyElement::kNullFlag; }
template<>
inline uint16_t type2flag<int8_t>() { return PlyElement::kInt8Flag; }
template<>
inline uint16_t type2flag<uint8_t>() { return PlyElement::kUint8Flag; }
template<>
inline uint16_t type2flag<int16_t>() { return PlyElement::kInt16Flag; }
template<>
inline uint16_t type2flag<uint16_t>() { return PlyElement::kUint16Flag; }
template<>
inline uint16_t type2flag<int32_t>() { return PlyElement::kInt32Flag; }
template<>
inline uint16_t type2flag<uint32_t>() { return PlyElement::kUint32Flag; }
template<>
inline uint16_t type2flag<float>() { return PlyElement::kFloatFlag; }
template<>
inline uint16_t type2flag<double>() { return PlyElement::kDoubleFlag; }

class PlyElementSet {
public:
  //! \brief Create an empty element set.
  //! \param name0 Name of the element type in the set.
  //! \param count0 Number of elements in the set.
  PlyElementSet(const std::string& name0="", const uint32_t& count0=0) :
    name(name0), count(count0), elements(), property_order(), property_flags(), property_size_flags() {}
  //! \brief Create an empty element set with property information.
  //! \tparam T Data type that will be expected for element property values.
  //!   This is determined by the unused third parameter.
  //! \param name0 Name of the element type in the set.
  //! \param property_names Names of properties defining each element in the
  //!   order they were read or will be written.
  //! \param is_array If true, the provided property is recorded as an array.
  template <typename T>
  PlyElementSet(const std::string& name0,
		const std::vector<std::string> &property_names,
		const T&,
		const bool is_array = false) :
    name(name0), count(0), elements(), property_order(),
    property_flags(), property_size_flags() {
    set_flags<T>(property_names, is_array);
  }
  //! \brief Create an element set from an array of property values.
  //! \tparam T Type of property values.
  //! \tparam M Number of elements in the set.
  //! \tparam N Number of property values for each element.
  //! \param name0 Name of the element type in the set.
  //! \param arr Array of property values for each element in the set.
  //! \param property_names Names of properties defining each element in the
  //!   order they were read or will be written.
  template <typename T, size_t M, size_t N>
  PlyElementSet(const std::string& name0,
		const T (&arr)[M][N],
		const std::vector<std::string> &property_names) :
    PlyElementSet(name0, property_names, arr[0][0],
		  bool(N != (property_names.size()))) {
    RAPIDJSON_ASSERT((N == property_names.size())
		     || (property_names.size() == 1));
    count = M;
    for (size_t i = 0; i < M; i++)
      add_element(std::vector<T>(arr[i], arr[i] + N));
  }
  //! Name of the type of element in the set.
  std::string name;
  //! Number of elements in the set.
  uint32_t count;
  //! Elements in the set.
  std::vector<PlyElement> elements;
  //! The names of properties defining each element in the order that they
  //!   were read or will be written.
  std::vector<std::string> property_order;
  //! Mapping between properties and flags defining the types used for the
  //!   property values.
  std::map<std::string, uint16_t> property_flags;
  //! Mapping between properties and the sizes of the properties if they are
  //!   stored/serialized as arrays.
  std::map<std::string, uint16_t> property_size_flags;

  //! \brief Set the type flags for the element set.
  //! \tparam T Data type that will be used to store element properties.
  //! \param property_names Names of properties defining each element.
  //! \param is_array If true, the provided property is recorded as an array.
  template<typename T>
  void set_flags(const std::vector<std::string> &property_names, const bool is_array) {
    uint16_t flags = type2flag<T>();
    uint16_t size_flags = 0;
    if (is_array) {
      RAPIDJSON_ASSERT(property_names.size() == 1);
      flags = (flags | PlyElement::kListFlag);
      size_flags = PlyElement::kUint8Flag;
    }
    for (auto it = property_names.begin(); it != property_names.end(); it++) {
      property_order.push_back(*it);
      property_flags.emplace(std::piecewise_construct,
			     std::forward_as_tuple(*it),
			     std::forward_as_tuple(flags));
      property_size_flags.emplace(std::piecewise_construct,
				  std::forward_as_tuple(*it),
				  std::forward_as_tuple(size_flags));
    }
  }
  //! \brief Add an element to the set.
  //! \tparam Type of property values.
  //! \param arr Property values for the new element.
  template<typename T>
  void add_element(const std::vector<T> &arr)
  { elements.emplace_back(property_order, property_flags, arr); }
  //! \brief Check if this element set is equivalent to another.
  //! \param rhs Element set to compare against.
  //! \return true if this element set is equivalent to rhs.
  bool is_equal(const PlyElementSet& rhs) const {
    if (this->name != rhs.name)
      return false;
    if (this->property_flags != rhs.property_flags)
      return false;
    if (this->elements != rhs.elements)
      return false;
    return true;
  }
  //! \brief Write all elements in the set to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    for (auto it = elements.begin(); it != elements.end(); it++)
      it->write(out);
    return out;
  }
  //! \brief Write the header entry defining an element set to an output
  //!   stream.
  //! \param out Output stream.
  void write_header(std::ostream &out) const {
    out << "element " << name << " " << elements.size() << std::endl;
    for (auto name = property_order.begin(); name != property_order.end(); name++) {
      auto it = property_flags.find(*name);
      RAPIDJSON_ASSERT(it != property_flags.end());
      
      out << "property " << PlyElement::flag2typename(it->second) << " ";
      if (it->second & PlyElement::kListFlag) {
	uint16_t element_flag = PlyElement::kUint8Flag;
	auto it_size = property_size_flags.find(*name);
	if (it_size != property_size_flags.end())
	  element_flag = it_size->second;
	out << PlyElement::flag2typename(element_flag) << " "
	    << PlyElement::flag2typename((uint16_t)(it->second & ~PlyElement::kListFlag)) << " ";
      }
      out << it->first << std::endl;
    }
  }
  //! \brief Read all expected elements from an input stream.
  //! \param in Input stream.
  //! \return Input stream.
  std::istream & read(std::istream &in) {
    for (size_t i = 0; i < count; i++)
      elements.emplace_back(property_order, property_flags, in);
    return in;
  }
  //! \brief Read the next property from an input stream.
  //! \param in Input stream.
  void read_property(std::istream &in) {
    std::string property_name;
    std::string property_type;
    in >> property_type;
    uint16_t flags = PlyElement::typename2flag(property_type);
    uint16_t size_flags = 0;
    if (flags & PlyElement::kListFlag) {
      std::string word;
      in >> word;
      size_flags = PlyElement::typename2flag(word);
      in >> word;
      flags = flags | PlyElement::typename2flag(word);
    }
    in >> property_name;
    property_order.push_back(property_name);
    property_flags[property_name] = flags;
    property_size_flags[property_name] = size_flags;
  }
};

//! \brief Check for equivalence between two PlyElementSet instances.
//! \param lhs First element set for comparison.
//! \param rhs Second element set for comparison.
//! \return true if the two instances are equivalent.
inline
bool operator == (const PlyElementSet& lhs, const PlyElementSet& rhs)
{ return lhs.is_equal(rhs); };

//! Ply 3D geometry container class.
class Ply {
public:
  Ply() : comments(), format("ascii 1.0"), elements(), element_order() {}
  //! \brief Copy constructor.
  //! \param rhs Instance to copy.
  Ply(const Ply& rhs) : comments(rhs.comments), format(rhs.format), elements(rhs.elements), element_order(rhs.element_order) {}
  //! \brief Create an Ply instance from a C array of vertices.
  //! \tparam Tv Type of value in vertex value arrays.
  //! \tparam Mv Number of vertex elements.
  //! \tparam Nv Number of values in the array for each vertex element.
  //! \param vertices Array of vertex element value arrays.
  template<typename Tv, SizeType Mv, SizeType Nv>
  Ply(const Tv (&vertices)[Mv][Nv]) :
    comments(), format("ascii 1.0"), elements(), element_order() {
    add_element_set_vertex(vertices);
  }
  //! \brief Create an Ply instance from C arrays of vertices and faces.
  //! \tparam Tv Type of value in vertex value arrays.
  //! \tparam Mv Number of vertex elements.
  //! \tparam Nv Number of values in the array for each vertex element.
  //! \tparam Tf Type of value in face value arrays.
  //! \tparam Mf Number of face elements.
  //! \tparam Nf Number of values in the array for each face element.
  //! \param vertices Array of vertex element value arrays.
  //! \param faces Array of face element value arrays.
  template<typename Tv, SizeType Mv, SizeType Nv,
	   typename Tf, SizeType Mf, SizeType Nf>
  Ply(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf]) :
    Ply(vertices) {
    add_element_set("face", faces, std::vector<std::string>({"vertex_index"}));
  }
  //! \brief Create an Ply instance from C arrays of vertices, faces, and
  //!   edges.
  //! \tparam Tv Type of value in vertex value arrays.
  //! \tparam Mv Number of vertex elements.
  //! \tparam Nv Number of values in the array for each vertex element.
  //! \tparam Tf Type of value in face value arrays.
  //! \tparam Mf Number of face elements.
  //! \tparam Nf Number of values in the array for each face element.
  //! \tparam Te Type of value in edge value arrays.
  //! \tparam Me Number of edge elements.
  //! \tparam Ne Number of values in the array for each edge element.
  //! \param vertices Array of vertex element value arrays.
  //! \param faces Array of face element value arrays.
  //! \param edges Array of edge element value arrays.
  template<typename Tv, SizeType Mv, SizeType Nv,
	   typename Tf, SizeType Mf, SizeType Nf,
	   typename Te, SizeType Me, SizeType Ne>
  Ply(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf],
      const Te (&edges)[Me][Ne]) :
    Ply(vertices, faces) {
    add_element_set_edge(edges);
  }

  //! \brief Add a set of vertex elements.
  //! \tparam Tv Type of element properties.
  //! \tparam Mv Number of elements.
  //! \tparam Nv Number of properties defining each element.
  //! \param vertices Property values for each element in the set.
  template<typename Tv, SizeType Mv, SizeType Nv>
  void add_element_set_vertex(const Tv (&vertices)[Mv][Nv]) {
    RAPIDJSON_ASSERT((Nv == 3) || (Nv == 6));
    if (Nv == 3)
      add_element_set("vertex", vertices,
		      std::vector<std::string>({"x", "y", "z"}));
    else if (Nv == 6)
      add_element_set("vertex", vertices,
		      std::vector<std::string>({"x", "y", "z", "red", "blue", "green"}));
  }

  //! \brief Add a set of edge elements.
  //! \tparam Te Type of element properties.
  //! \tparam Me Number of elements.
  //! \tparam Ne Number of properties defining each element.
  //! \param edges Property values for each element in the set.
  template<typename Te, SizeType Me, SizeType Ne>
  void add_element_set_edge(const Te (&edges)[Me][Ne]) {
    RAPIDJSON_ASSERT((Ne == 2) || (Ne == 5));
    if (Ne == 2) {
      std::vector<std::string> property_names {"vertex1", "vertex2"};
      add_element_set("edge", edges, property_names);
    } else if (Ne == 5) {
      std::vector<std::string> property_names {"vertex1", "vertex2", "red", "blue", "green"};
      add_element_set("edge", edges, property_names);
    }
  }

  //! Comments at the beginning of the serialized geometry.
  std::vector<std::string> comments;
  //! Version string specifying the format of the serialized geometry.
  std::string format;
  //! Map between element type names and sets of elements.
  std::map<std::string,PlyElementSet> elements;
  //! Element type names in the order that they were read or should be written.
  std::vector<std::string> element_order;

  //! \brief Add a single element to the geometry.
  //! \param name Name of the type of element being added.
  //! \param arr Vector of element properties.
  //! \param property_names Vector of element property names.
  template <typename T>
  void add_element(const std::string& name,
		   const std::vector<T> &arr,
		   const std::vector<std::string> &property_names) {
    bool is_array = bool(arr.size() != property_names.size());
    RAPIDJSON_ASSERT((!is_array) || (property_names.size() == 1));
    if (elements.find(name) == elements.end()) {
      element_order.push_back(name);
      elements.emplace(std::piecewise_construct,
		       std::forward_as_tuple(name),
		       std::forward_as_tuple(name));
      elements[name].set_flags<T>(property_names, is_array);
    }
    elements[name].count++;
    elements[name].add_element(arr);
  }
  //! \brief Add a new element set to the geometry.
  //! \tparam T Type of property values.
  //! \tparam M Number of elements in the set.
  //! \tparam N Number of properties for each element.
  //! \param name Name fo the type of element in the set.
  //! \param arr Property values for each of the elements in the set.
  //! \param property_names The names of properties defining each element in
  //!   the set in the order they were read or will be written.
  template <typename T, SizeType M, SizeType N>
  void add_element_set(const std::string& name,
		       const T (&arr)[M][N],
		       const std::vector<std::string> &property_names) {
    RAPIDJSON_ASSERT(elements.find(name) == elements.end());
    element_order.push_back(name);
    elements.emplace(std::piecewise_construct,
		     std::forward_as_tuple(name),
		     std::forward_as_tuple(name, arr, property_names));
  }
  //! \brief Add a new element set to the geometry and allocates for elements.
  //! \param name Name of the type of element in the set.
  //! \param count Number of elements that should be allocated for in the set.
  void add_element_set(const std::string& name, uint32_t count=0) {
    element_order.push_back(name);
    elements.emplace(std::piecewise_construct,
		     std::forward_as_tuple(name),
		     std::forward_as_tuple(name, count));
  }
  //! \brief Check if this geometry is equivalent to another.
  //! \param rhs Geometry to compare against.
  //! \return true If this geometry is equivalent to rhs.
  bool is_equal(const Ply& rhs) const {
    if (this->comments != rhs.comments)
      return false;
    if (this->format != rhs.format)
      return false;
    if (this->elements != rhs.elements)
      return false;
    return true;
  }
  //! \brief Write geometry elements to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    // Write header
    out << "ply" << std::endl
	<< "format " << format << std::endl;
    for (auto it = comments.begin(); it != comments.end(); it++)
      out << "comment " << *it << std::endl;
    for (auto it = element_order.begin(); it != element_order.end(); it++) {
      auto eit = elements.find(*it);
      RAPIDJSON_ASSERT(eit != elements.end());
      eit->second.write_header(out);
    }
    out << "end_header" << std::endl;
    // Write body
    for (auto it = element_order.begin(); it != element_order.end(); it++) {
      auto eit = elements.find(*it);
      RAPIDJSON_ASSERT(eit != elements.end());
      eit->second.write(out);
    }
    return out;
  }
  //! \brief Read geometry elements from an input stream.
  //! \param in Input stream.
  //! \return Input stream.
  std::istream & read(std::istream &in) {
    std::string word;
    in >> word;
    if (word != "ply")
      RAPIDJSON_ASSERT(!sizeof("Input does not appear to be in ply format"));
    // Read header
    std::string current_element;
    while (in >> word) {
      if (word == "end_header")
	break;
      else if (word == "format") {
	bool first = true;
	format = "";
	while ((in.peek()!='\n') && (in >> word)) {
	  if (!first)
	    format += " ";
	  format += word;
	  first = false;
	}
      } else if (word == "comment") {
	std::string comment;
	bool first = true;
	while ((in.peek()!='\n') && (in >> word)) {
	  if (!first)
	    comment += " ";
	  comment += word;
	  first = false;
	}
	comments.push_back(comment);
      } else if (word == "element") {
	uint32_t count = 0;
	in >> current_element;
	in >> count;
	add_element_set(current_element, count);
      } else if (word == "property") {
	elements[current_element].read_property(in);
      } else {
	RAPIDJSON_ASSERT(!sizeof(std::string("Unrecognized input beginning w/ ") + word));
      }
    }
    // Read body
    for (auto it = element_order.begin(); it != element_order.end(); it++) {
      auto eit = elements.find(*it);
      RAPIDJSON_ASSERT(eit != elements.end());
      eit->second.read(in);
    }
    return in;
  }
  
  friend bool operator == (const Ply& lhs, const Ply& rhs);
  friend std::ostream & operator << (std::ostream &out, const Ply &p);
  friend std::istream & operator >> (std::istream &in,  Ply &p);
};

//! \brief Check for equality between two ply geometries.
//! \param lhs First geometry for comparison.
//! \param rhs Second geometry for comparison.
//! \return true if the two geometries are equivalent.
inline
bool operator == (const Ply& lhs, const Ply& rhs)
{ return lhs.is_equal(rhs); };

//! \brief Write a Ply geometry to an output stream.
//! \param out Output stream.
//! \param p Geometry.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const Ply &p)
{ return p.write(out); };

//! \brief Read a Ply geometry from an input stream.
//! \param in Input stream.
//! \param p Destination geometry.
//! \return Input sream.
inline
std::istream & operator >> (std::istream &in, Ply &p)
{ return p.read(in); };


RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_PLY_H_
