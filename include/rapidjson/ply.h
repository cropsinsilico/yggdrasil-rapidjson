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

class PlyElement {
public:
  PlyElement() : property_order(), properties() {}
  PlyElement(const PlyElement &rhs) : property_order(rhs.property_order), properties(rhs.properties) {}
  PlyElement(const std::vector<std::string> &property_order0,
	     const std::map<std::string, uint16_t> &properties0,
	     std::istream &in) :
    PlyElement() {
    read(property_order0, properties0, in);
  }
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
    Number(const uint16_t &flag) : Number() { this->assign(flag, 0); }
    Number(const uint16_t &flag, std::istream &in) : Number(flag) {
      this->read(flag, in);
    }
    template <typename T>
    Number(const uint16_t &flag, const T &x) : Number(flag) {
      this->assign(flag, x);
    }
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
    Data(const uint16_t flag) : f(flag), n(flag), elements() {}
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
    template<typename T>
    Data(const uint16_t flag, const T &x) : Data(flag) {
      RAPIDJSON_ASSERT(!(flag & kListFlag));
      n.assign(flag, x);
    }
    template<typename T>
    Data(const uint16_t flag, const std::vector<T> &x) : Data(flag) {
      RAPIDJSON_ASSERT(flag & kListFlag);
      uint16_t element_flags = (uint16_t)(flag & ~kListFlag);
      for (auto it = x.begin(); it != x.end(); it++)
	elements.emplace_back(element_flags, *it);
    }
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
    uint16_t f;
    // union {
    Number n;
    std::vector<Number> elements;
    // };
  };

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
  
  std::vector<std::string> property_order;
  std::map<std::string, Data> properties;
  friend class PlyElementSet;
  template<typename T>
  friend inline uint16_t type2flag();
  
public:
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
  std::vector<int> get_int_array() const {
    std::vector<int> out;
    for (auto name = property_order.begin(); name != property_order.end(); name++) {
      auto it = properties.find(*name);
      RAPIDJSON_ASSERT(it != properties.end());
      extend_aray_data(it->second, out);
    }
    return out;
  }
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
  PlyElementSet(const std::string& name0="", const uint32_t& count0=0) :
    name(name0), count(count0), elements(), property_order(), property_flags(), property_size_flags() {}
  template <typename T>
  PlyElementSet(const std::string& name0,
		const std::vector<const std::string> &property_names,
		const T&,
		const bool is_array = false) :
    name(name0), count(0), elements(), property_order(),
    property_flags(), property_size_flags() {
    set_flags<T>(property_names, is_array);
  }
  template <typename T, size_t M, size_t N>
  PlyElementSet(const std::string& name0,
		const T (&arr)[M][N],
		const std::vector<const std::string> &property_names) :
    PlyElementSet(name0, property_names, arr[0][0],
		  bool(N != (property_names.size()))) {
    RAPIDJSON_ASSERT((N == property_names.size())
		     || (property_names.size() == 1));
    count = M;
    for (size_t i = 0; i < M; i++)
      add_element(std::vector<T>(arr[i], arr[i] + N));
  }
  std::string name;
  uint32_t count;
  std::vector<PlyElement> elements;
  std::vector<std::string> property_order;
  std::map<std::string, uint16_t> property_flags;
  std::map<std::string, uint16_t> property_size_flags;

  template<typename T>
  void set_flags(const std::vector<const std::string> &property_names, const bool is_array) {
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
  template<typename T>
  void add_element(const std::vector<T> &arr)
  { elements.emplace_back(property_order, property_flags, arr); }
  bool is_equal(const PlyElementSet& rhs) const {
    if (this->name != rhs.name)
      return false;
    if (this->property_flags != rhs.property_flags)
      return false;
    if (this->elements != rhs.elements)
      return false;
    return true;
  }
  std::ostream & write(std::ostream &out) const {
    for (auto it = elements.begin(); it != elements.end(); it++)
      it->write(out);
    return out;
  }
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
  std::istream & read(std::istream &in) {
    for (size_t i = 0; i < count; i++)
      elements.emplace_back(property_order, property_flags, in);
    return in;
  }
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

inline
bool operator == (const PlyElementSet& lhs, const PlyElementSet& rhs)
{ return lhs.is_equal(rhs); };

class Ply {
public:
  Ply() : comments(), format("ascii 1.0"), elements(), element_order() {}
  Ply(const Ply& rhs) : comments(rhs.comments), format(rhs.format), elements(rhs.elements), element_order(rhs.element_order) {}
  template<typename Tv, SizeType Mv, SizeType Nv>
  Ply(const Tv (&vertices)[Mv][Nv]) :
    comments(), format("ascii 1.0"), elements(), element_order() {
    add_element_set_vertex(vertices);
  }
  template<typename Tv, SizeType Mv, SizeType Nv,
	   typename Tf, SizeType Mf, SizeType Nf>
  Ply(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf]) :
    Ply(vertices) {
    add_element_set("face", faces, std::vector<const std::string>({"vertex_index"}));
  }
  template<typename Tv, SizeType Mv, SizeType Nv,
	   typename Tf, SizeType Mf, SizeType Nf,
	   typename Te, SizeType Me, SizeType Ne>
  Ply(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf],
      const Te (&edges)[Me][Ne]) :
    Ply(vertices, faces) {
    add_element_set_edge(edges);
  }

  template<typename Tv, SizeType Mv, SizeType Nv>
  void add_element_set_vertex(const Tv (&vertices)[Mv][Nv]) {
    RAPIDJSON_ASSERT((Nv == 3) || (Nv == 6));
    if (Nv == 3)
      add_element_set("vertex", vertices,
		      std::vector<const std::string>({"x", "y", "z"}));
    else if (Nv == 6)
      add_element_set("vertex", vertices,
		      std::vector<const std::string>({"x", "y", "z", "red", "blue", "green"}));
  }

  template<typename Te, SizeType Me, SizeType Ne>
  void add_element_set_edge(const Te (&edges)[Me][Ne]) {
    RAPIDJSON_ASSERT((Ne == 2) || (Ne == 5));
    if (Ne == 2)
      add_element_set("edge", edges,
		      std::vector<const std::string>({"vertex1", "vertex2"}));
    else if (Ne == 5)
      add_element_set("edge", edges,
		      std::vector<const std::string>({"vertex1", "vertex2", "red", "blue", "green"}));
  }

  std::vector<std::string> comments;
  std::string format;
  std::map<std::string,PlyElementSet> elements;
  std::vector<std::string> element_order;

  template <typename T>
  void add_element(const std::string& name,
		   const std::vector<T> &arr,
		   const std::vector<const std::string> &property_names) {
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
  template <typename T, SizeType M, SizeType N>
  void add_element_set(const std::string& name,
		       const T (&arr)[M][N],
		       const std::vector<const std::string> &property_names) {
    RAPIDJSON_ASSERT(elements.find(name) == elements.end());
    element_order.push_back(name);
    elements.emplace(std::piecewise_construct,
		     std::forward_as_tuple(name),
		     std::forward_as_tuple(name, arr, property_names));
  }
  void add_element_set(const std::string& name, uint32_t count=0) {
    element_order.push_back(name);
    elements.emplace(std::piecewise_construct,
		     std::forward_as_tuple(name),
		     std::forward_as_tuple(name, count));
  }
  bool is_equal(const Ply& rhs) const {
    if (this->comments != rhs.comments)
      return false;
    if (this->format != rhs.format)
      return false;
    if (this->elements != rhs.elements)
      return false;
    return true;
  }
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

inline
bool operator == (const Ply& lhs, const Ply& rhs)
{ return lhs.is_equal(rhs); };

inline
std::ostream & operator << (std::ostream &out, const Ply &p)
{ return p.write(out); };

inline
std::istream & operator >> (std::istream &in, Ply &p)
{ return p.read(in); };


RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_PLY_H_
