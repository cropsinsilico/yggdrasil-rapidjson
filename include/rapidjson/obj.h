#ifndef RAPIDJSON_OBJ_H_
#define RAPIDJSON_OBJ_H_

#include <iostream>

RAPIDJSON_NAMESPACE_BEGIN

#define IS_EQUAL_FLT(a, b) (fabs(a - b) < 0.01)
#define IS_EQUAL_DBL(a, b) (abs(a - b) < 0.01)
#define OBJ_ELEMENT_INIT(word, lhs, args) {				\
    if      (word == "v"     ) lhs = new ObjVertex args;		\
    else if (word == "vp"    ) lhs = new ObjVParameter args;		\
    else if (word == "vn"    ) lhs = new ObjVNormal args;		\
    else if (word == "vt"    ) lhs = new ObjVTexture args;		\
    else if (word == "p"     ) lhs = new ObjPoint args;			\
    else if (word == "l"     ) lhs = new ObjLine args;			\
    else if (word == "f"     ) lhs = new ObjFace args;			\
    else if (word == "curv"  ) lhs = new ObjCurve args;			\
    else if (word == "curv2" ) lhs = new ObjCurve2D args;		\
    else if (word == "surf"  ) lhs = new ObjSurface args;		\
    else if (word == "cstype") lhs = new ObjFreeFormType args;		\
    else if (word == "deg"   ) lhs = new ObjDegree args;		\
    else if (word == "bmat"  ) lhs = new ObjBasisMatrix args;		\
    else if (word == "step"  ) lhs = new ObjStep args;			\
    else if (word == "parm"  ) lhs = new ObjParameter args;		\
    else if (word == "trim"  ) lhs = new ObjTrim args;			\
    else if (word == "hole"  ) lhs = new ObjHole args;			\
    else if (word == "scrv"  ) lhs = new ObjScrv args;			\
    else if (word == "sp"    ) lhs = new ObjSpecialPoints args;		\
    else if (word == "con"   ) lhs = new ObjConnect args;		\
    else if (word == "g"     ) lhs = new ObjGroup args;			\
    else if (word == "s"     ) lhs = new ObjSmoothingGroup args;	\
    else if (word == "mg"    ) lhs = new ObjMergingGroup args;		\
    else if (word == "o"     ) lhs = new ObjObjectName args;		\
    else if (word == "end"   ) lhs = NULL;				\
    else RAPIDJSON_ASSERT(!sizeof(std::string("Unsupported element signifier: ") + word)); \
  }

template <typename T>
inline bool is_equal(const T &a, const T &b) {
  return (a == b);
};
template <>
inline bool is_equal(const float &a, const float &b) {
  return IS_EQUAL_FLT(a, b);
};
template <>
inline bool is_equal(const double &a, const double &b) {
  return IS_EQUAL_DBL(a, b);
};

template <typename T>
inline bool is_equal_vectors(const std::vector<T>& a, const std::vector<T>& b) {
  if (a.size() != b.size()) return false;
  for (auto ait = a.begin(), bit = b.begin(); ait != a.end(); ait++, bit++)
    if (!is_equal(*ait, *bit)) return false;
  return true;
};

// Forward declaration
class ObjElement;
inline void read_obj_elements(std::istream &in, std::vector<ObjElement*> &elements,
			      const char break_at = 0);

class ObjColor {
public:
  ObjColor() :
    r(0), g(0), b(0), is_set(false) {}
  ObjColor(const ObjColor& rhs) :
    r(rhs.r), g(rhs.g), b(rhs.b), is_set(rhs.is_set) {}
  ObjColor(uint8_t red, uint8_t green, uint8_t blue) :
    r(red), g(green), b(blue), is_set(true) {}
  uint8_t r;
  uint8_t g;
  uint8_t b;
  bool is_set;
  bool is_equal(const ObjColor& rhs) const {
    const ObjColor& lhs = *this;
    if (lhs.is_set != rhs.is_set) return false;
    if (lhs.r != rhs.r) return false;
    if (lhs.g != rhs.g) return false;
    if (lhs.b != rhs.b) return false;
    return true;
  }
  friend bool operator == (const ObjColor& lhs, const ObjColor& rhs);
};

inline
bool operator == (const ObjColor& lhs, const ObjColor& rhs)
{ return lhs.is_equal(rhs); };

typedef int64_t ObjRef;

class ObjRefVertex {
public:
  ObjRefVertex() :
    v(-1), vt(-1), vn(-1), Nparam(-1) {}
  ObjRefVertex(const ObjRefVertex& rhs) :
    v(rhs.v), vt(rhs.vt), vn(rhs.vn), Nparam(rhs.Nparam) {}
  ObjRefVertex(ObjRef v0, ObjRef vt0=-1, ObjRef vn0=-1,
	       int8_t Nparam0=-1) :
    v(v0), vt(vt0), vn(vn0), Nparam(Nparam0) {
    if (Nparam < 0) {
      if (vn >= 0)
	Nparam = 3;
      else if (vt >= 0)
	Nparam = 2;
      else
	Nparam = 1;
    }
  }
  template <typename T>
  ObjRefVertex(const T& v0,
	       RAPIDJSON_ENABLEIF((
      internal::OrExpr<internal::IsSame<T,int>,
      internal::OrExpr<internal::IsSame<T,int8_t>,
      internal::OrExpr<internal::IsSame<T,uint8_t>,
      internal::OrExpr<internal::IsSame<T,int16_t>,
      internal::OrExpr<internal::IsSame<T,uint16_t>,
      internal::OrExpr<internal::IsSame<T,int32_t>,
      internal::OrExpr<internal::IsSame<T,uint32_t>,
      internal::OrExpr<internal::IsSame<T,int64_t>,
      internal::OrExpr<internal::IsSame<T,ObjRef>,
      internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjRefVertex(v0, -1, -1) {}
  std::ostream & write(std::ostream &out) const {
    out << v;
    if (Nparam > 1) {
      out << "/";
      if (vt >= 0)
	out << vt;
    }
    if (Nparam > 2) {
      out << "/";
      if (vn >= 0)
	out << vn;
    }
    return out;
  }
  std::istream & read(std::istream &in) {
    std::string word;
    in >> word;
    std::istringstream ss_word(word);
    std::string token;
    std::istringstream ss_token;
    v = -1;
    vt = -1;
    vn = -1;
    Nparam = 0;
    // v
    RAPIDJSON_ASSERT(std::getline(ss_word, token, '/'));
    ss_token.str(token);
    ss_token >> v;
    // vt
    if (!std::getline(ss_word, token, '/')) {
      Nparam = 1;
      return in;
    }
    ss_token.str(token);
    ss_token >> vt;
    // vn
    if (!std::getline(ss_word, token, '/')) {
      Nparam = 1;
      return in;
    }
    ss_token.str(token);
    ss_token >> vt;
    Nparam = 3;
    return in;
  }
  ObjRef v;
  ObjRef vt;
  ObjRef vn;
  int8_t Nparam;
  bool is_equal(const ObjRefVertex& rhs) const {
    const ObjRefVertex& lhs = *this;
    if (lhs.Nparam != rhs.Nparam) return false;
    if (lhs.v != rhs.v) return false;
    if (lhs.vt != rhs.vt) return false;
    if (lhs.vn != rhs.vn) return false;
    return true;
  }
  friend bool operator == (const ObjRefVertex& lhs, const ObjRefVertex& rhs);
  friend std::ostream & operator << (std::ostream &out, const ObjRefVertex &p);
  friend std::istream & operator >> (std::istream &in, ObjRefVertex &p);
};

inline
bool operator == (const ObjRefVertex& lhs, const ObjRefVertex& rhs)
{ return lhs.is_equal(rhs); };

inline
std::ostream & operator << (std::ostream &out, const ObjRefVertex &p)
{ return p.write(out); };

inline
std::istream & operator >> (std::istream &in, ObjRefVertex &p)
{ return p.read(in); };


class ObjRefCurve {
public:
  ObjRefCurve() :
    u0(0.0), u1(0.0), curv2d(-1) {}
  ObjRefCurve(const ObjRefCurve& rhs) :
    u0(rhs.u0), u1(rhs.u1), curv2d(rhs.curv2d) {}
  ObjRefCurve(double u00, double u10=0.0, ObjRef curv2d0=-1) :
    u0(u00), u1(u10), curv2d(curv2d0) {}
  std::ostream & write(std::ostream &out) const {
    out << u0 << " " << u1 << " " << curv2d;
    return out;
  }
  std::istream & read(std::istream &in) {
    in >> u0;
    in >> u1;
    in >> curv2d;
    return in;
  }
  double u0;
  double u1;
  ObjRef curv2d;
  bool is_equal(const ObjRefCurve& rhs) const {
    const ObjRefCurve& lhs = *this;
    if (!IS_EQUAL_DBL(lhs.u0, rhs.u0)) return false;
    if (!IS_EQUAL_DBL(lhs.u1, rhs.u1)) return false;
    if (lhs.curv2d != rhs.curv2d) return false;
    return true;
  }
  friend bool operator == (const ObjRefCurve& lhs, const ObjRefCurve& rhs);
  friend std::ostream & operator << (std::ostream &out, const ObjRefCurve &p);
  friend std::istream & operator >> (std::istream &in, ObjRefCurve &p);
};

inline
bool operator == (const ObjRefCurve& lhs, const ObjRefCurve& rhs)
{ return lhs.is_equal(rhs); };

inline
std::ostream & operator << (std::ostream &out, const ObjRefCurve &p)
{ return p.write(out); };

inline
std::istream & operator >> (std::istream &in, ObjRefCurve &p)
{ return p.read(in); };


class ObjRefSurface {
public:
  ObjRefSurface() :
    surf(-1), q0(0), q1(0), curv2d(-1) {}
  ObjRefSurface(const ObjRefSurface& rhs) :
    surf(rhs.surf), q0(rhs.q0), q1(rhs.q1), curv2d(rhs.curv2d) {}
  ObjRefSurface(ObjRef surf0, double q00=0.0, double q10=0.0, ObjRef curv2d0=-1) :
    surf(surf0), q0(q00), q1(q10), curv2d(curv2d0) {}
  std::ostream & write(std::ostream &out) const {
    out << surf << " " << q0 << " " << q1 << " " << curv2d;
    return out;
  }
  std::istream & read(std::istream &in) {
    in >> surf;
    in >> q0;
    in >> q1;
    in >> curv2d;
    return in;
  }
  ObjRef surf;
  double q0;
  double q1;
  ObjRef curv2d;
  bool is_equal(const ObjRefSurface& rhs) const {
    const ObjRefSurface& lhs = *this;
    if (lhs.surf != rhs.surf) return false;
    if (!IS_EQUAL_DBL(lhs.q0, rhs.q0)) return false;
    if (!IS_EQUAL_DBL(lhs.q1, rhs.q1)) return false;
    if (lhs.curv2d != rhs.curv2d) return false;
    return true;
  }
  friend bool operator == (const ObjRefSurface& lhs, const ObjRefSurface& rhs);
  friend std::ostream & operator << (std::ostream &out, const ObjRefSurface &p);
  friend std::istream & operator >> (std::istream &in, ObjRefSurface &p);
};

inline
bool operator == (const ObjRefSurface& lhs, const ObjRefSurface& rhs)
{ return lhs.is_equal(rhs); };

inline
std::ostream & operator << (std::ostream &out, const ObjRefSurface &p)
{ return p.write(out); };

inline
std::istream & operator >> (std::istream &in, ObjRefSurface &p)
{ return p.read(in); };


class ObjElement {
public:
  ObjElement() : code("") {}
  ObjElement(const std::string& code0) : code(code0) {}
  ObjElement(const ObjElement& rhs) : code(rhs.code) {}
  virtual ~ObjElement() {}
  virtual ObjElement* copy() const { return new ObjElement(*this); }
  template <typename T1, typename T2>
  void assign_values(std::vector<T1>& dst, const T2* src, const size_t &N)
  { assign_values(dst, std::vector<T2>(src, src+N)); }
  template <typename T1, typename T2, size_t N>
  void assign_values(std::vector<T1>& dst, const T2 (&src)[N])
  { assign_values(dst, std::vector<T2>(src, src+N)); }
  template <typename T1, typename T2>
  void assign_values(std::vector<T1>& dst, const std::vector<T2> &src) {
    for (auto it = src.begin(); it != src.end(); it++)
      dst.emplace_back(*it);
  }
  template <typename T, size_t N>
  void assign_values(const T (&)[N]) {}
  virtual void from_values() {}
  virtual void read_values(std::istream&) {
    std::cerr << "Child class must overrride read_values" << std::endl;
  }
  virtual void write_values(std::ostream&) const {
    std::cerr << "Child class must overrride write_values" << std::endl;
  }
  virtual bool is_equal(const ObjElement*) const {
    std::cerr << "Child class must overrride is_equal" << std::endl;
    return false;
  }
  virtual std::vector<std::string> get_string_array() const {
    std::vector<std::string> out;
    std::cerr << "Child class must overrride get_string_array" << std::endl;
    return out;
  }
  virtual std::vector<int> get_int_array() const {
    std::vector<int> out;
    std::cerr << "Child class must overrride get_int_array" << std::endl;
    return out;
  }
  virtual std::vector<double> get_double_array() const {
    std::vector<double> out;
    std::cerr << "Child class must overrride get_double_array" << std::endl;
    return out;
  }
  template <typename T>
  void read_values(std::istream &in, std::vector<T> &values) {
    T x = 0;
    while ((in.peek() != '\n') && (in >> x))
      values.push_back(x);
  }
  std::ostream & write(std::ostream &out) const {
    out << code << " ";
    write_values(out);
    out << std::endl;
    return out;
  }
  template <typename T>
  void write_values(std::ostream &out, const std::vector<T> &values) const {
    for (auto it = values.begin(); it != values.end(); it++) {
      if (it != values.begin())
	out << " ";
      out << *it;
    }
  }
  std::string as_string() const {
    std::string out;
    std::stringstream ss(out);
    write(ss);
    return out;
  }
  std::string code;
  friend std::ostream & operator << (std::ostream &out, const ObjElement &p);
};

inline
std::ostream & operator << (std::ostream &out, const ObjElement &p)
{ return p.write(out); };

// Vertex data
class ObjVertex : public ObjElement {
public:
  ObjVertex(const ObjVertex& rhs) :
    ObjElement(rhs), values(rhs.values), x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w), color(rhs.color) {}
  ObjVertex(std::istream &in) :
    ObjElement("v"), values(), x(0), y(0), z(0), w(-1), color() {
    ObjElement::read_values(in, values);
    // read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjVertex(const T (&src)[N]) : ObjVertex(std::vector<T>(src, src+N)) {}
  template <typename T>
  ObjVertex(const std::vector<T> &values0,
	    RAPIDJSON_ENABLEIF((
     internal::OrExpr<internal::IsSame<T,double>,
     internal::OrExpr<internal::IsSame<T,float>,
     internal::OrExpr<internal::IsSame<T,int8_t>,
     internal::OrExpr<internal::IsSame<T,uint8_t>,
     internal::OrExpr<internal::IsSame<T,int16_t>,
     internal::OrExpr<internal::IsSame<T,uint16_t>,
     internal::OrExpr<internal::IsSame<T,int32_t>,
     internal::OrExpr<internal::IsSame<T,uint32_t>,
     internal::OrExpr<internal::IsSame<T,int64_t>,
     internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjElement("v"), values(), x(0), y(0), z(0), w(-1), color() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjVertex(const std::vector<T> &,
	    RAPIDJSON_DISABLEIF((
     internal::OrExpr<internal::IsSame<T,double>,
     internal::OrExpr<internal::IsSame<T,float>,
     internal::OrExpr<internal::IsSame<T,int8_t>,
     internal::OrExpr<internal::IsSame<T,uint8_t>,
     internal::OrExpr<internal::IsSame<T,int16_t>,
     internal::OrExpr<internal::IsSame<T,uint16_t>,
     internal::OrExpr<internal::IsSame<T,int32_t>,
     internal::OrExpr<internal::IsSame<T,uint32_t>,
     internal::OrExpr<internal::IsSame<T,int64_t>,
     internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjElement("v"), values(), x(0), y(0), z(0), w(-1), color() {
    RAPIDJSON_ASSERT(!sizeof("ObjVertex type is double"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 3)
		     || (values.size() == 4)
		     || (values.size() == 6)
		     || (values.size() == 7));
    x = values[0];
    y = values[1];
    z = values[2];
    if ((values.size() == 6) || (values.size() == 7)) {
      color.r = (uint8_t)values[3];
      color.g = (uint8_t)values[4];
      color.b = (uint8_t)values[5];
    }
    if ((values.size() == 4) || (values.size() == 7))
      w = values[values.size() - 1];
    else
      w = -1;
  }
  ObjVertex* copy() const override { return new ObjVertex(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override {
    out << x << " " << y << " " << z;
    if (color.is_set)
      out << " " << color.r << " " << color.g << " " << color.b;
    if (w >= 0)
      out << " " << w;
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjVertex* lhs = this;
    const ObjVertex* rhs = static_cast<const ObjVertex*>(rhs0);
    if (!IS_EQUAL_DBL(lhs->x, rhs->x)) return false;
    if (!IS_EQUAL_DBL(lhs->y, rhs->y)) return false;
    if (!IS_EQUAL_DBL(lhs->z, rhs->z)) return false;
    if (!IS_EQUAL_DBL(lhs->w, rhs->w)) return false;
    return (lhs->color == rhs->color);
  }
  std::vector<double> get_double_array() const override {
    std::vector<double> out({x, y, z});
    return out;
  }
  std::vector<double> values;
  double x;
  double y;
  double z;
  double w; // Negative indicates default weight of 1
  ObjColor color;
};

class ObjVParameter : public ObjElement {
public:
  ObjVParameter(const ObjVParameter& rhs) :
    ObjElement(rhs), values(), u(rhs.u), v(rhs.v), w(rhs.w) {}
  ObjVParameter(std::istream &in) :
    ObjElement("vp"), values(), u(0), v(0), w(-1) {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjVParameter(const T (&src)[N]) : ObjVParameter(std::vector<T>(src, src+N)) {}
  ObjVParameter(const std::vector<double> &values0) :
    ObjElement("vp"), values(), u(0), v(0), w(-1) {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjVParameter(const std::vector<T> &) :
    ObjElement("vp"), values(), u(0), v(0), w(-1) {
    RAPIDJSON_ASSERT(!sizeof("ObjVParameter type is double"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 2)
		     || (values.size() == 3));
    u = values[0];
    v = values[1];
    if (values.size() == 3)
      w = values[values.size() - 1];
    else
      w = -1;
  }
  ObjVParameter* copy() const override { return new ObjVParameter(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override {
    out << u << " " << v;
    if (w >= 0)
      out << " " << w;
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjVParameter* lhs = this;
    const ObjVParameter* rhs = static_cast<const ObjVParameter*>(rhs0);
    if (!IS_EQUAL_DBL(lhs->u, rhs->u)) return false;
    if (!IS_EQUAL_DBL(lhs->v, rhs->v)) return false;
    if (!IS_EQUAL_DBL(lhs->w, rhs->w)) return false;
    return true;
  }
  std::vector<double> get_double_array() const override {
    std::vector<double> out({u, v, w});
    return out;
  }
  std::vector<double> values;
  double u;
  double v;
  double w; // Negative indicates default weight of 1
};

class ObjVNormal : public ObjElement {
public:
  ObjVNormal(const ObjVNormal& rhs) :
    ObjElement(rhs), values(), i(rhs.i), j(rhs.j), k(rhs.k) {}
  ObjVNormal(std::istream &in) :
    ObjElement("vn"), values(), i(0), j(0), k(0) {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjVNormal(const T (&src)[N]) : ObjVNormal(std::vector<T>(src, src+N)) {}
  ObjVNormal(const std::vector<double> &values0) :
    ObjElement("vn"), values(), i(0), j(0), k(0) {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjVNormal(const std::vector<T> &) :
    ObjElement("vn"), values(), i(0), j(0), k(0) {
    RAPIDJSON_ASSERT(!sizeof("ObjVNormal type is double"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT(values.size() == 3);
    i = values[0];
    j = values[1];
    k = values[2];
  }
  ObjVNormal* copy() const override { return new ObjVNormal(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override {
    out << i << " " << j << " " << k;
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjVNormal* lhs = this;
    const ObjVNormal* rhs = static_cast<const ObjVNormal*>(rhs0);
    if (!IS_EQUAL_DBL(lhs->i, rhs->i)) return false;
    if (!IS_EQUAL_DBL(lhs->j, rhs->j)) return false;
    if (!IS_EQUAL_DBL(lhs->k, rhs->k)) return false;
    return true;
  }
  std::vector<double> get_double_array() const override {
    std::vector<double> out({i, j, k});
    return out;
  }
  std::vector<double> values;
  double i;
  double j;
  double k;
};

class ObjVTexture : public ObjElement {
public:
  ObjVTexture(const ObjVTexture& rhs) :
    ObjElement(rhs), values(), u(rhs.u), v(rhs.v), w(rhs.w) {}
  ObjVTexture(std::istream &in) :
    ObjElement("vt"), values(), u(0), v(0), w(0) {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjVTexture(const T (&src)[N]) : ObjVTexture(std::vector<T>(src, src+N)) {}
  ObjVTexture(const std::vector<double> &values0) :
    ObjElement("vt"), values(), u(0), v(0), w(0) {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjVTexture(const std::vector<T> &) :
    ObjElement("vt"), values(), u(0), v(0), w(0) {
    RAPIDJSON_ASSERT(!sizeof("ObjVTexture type is double"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 1)
		     || (values.size() == 2)
		     || (values.size() == 3));
    u = values[0];
    v = -1.0;
    w = -1.0;
    if (values.size() == 2)
      v = values[1];
    if (values.size() == 3)
      w = values[2];
  }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override {
    out << u;
    if (v >= 0)
      out << " " << v;
    if (w >= 0)
      out << " " << w;
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjVTexture* lhs = this;
    const ObjVTexture* rhs = static_cast<const ObjVTexture*>(rhs0);
    if (!IS_EQUAL_DBL(lhs->u, rhs->u)) return false;
    if (!IS_EQUAL_DBL(lhs->v, rhs->v)) return false;
    if (!IS_EQUAL_DBL(lhs->w, rhs->w)) return false;
    return true;
  }
  std::vector<double> get_double_array() const override {
    std::vector<double> out({u, v, w});
    return out;
  }
  std::vector<double> values;
  double u;
  double v; // Negative indicates default of 0
  double w; // Negative indicates default of 0
};

// Elements
class ObjPoint : public ObjElement {
public:
  ObjPoint(const ObjPoint& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjPoint(std::istream &in) :
    ObjElement("p"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjPoint(const T (&src)[N]) : ObjPoint(std::vector<T>(src, src+N)) {}
  template <typename T>
  ObjPoint(const std::vector<T> &values0,
	      RAPIDJSON_ENABLEIF((
      internal::OrExpr<internal::IsSame<T,int>,
      internal::OrExpr<internal::IsSame<T,int8_t>,
      internal::OrExpr<internal::IsSame<T,uint8_t>,
      internal::OrExpr<internal::IsSame<T,int16_t>,
      internal::OrExpr<internal::IsSame<T,uint16_t>,
      internal::OrExpr<internal::IsSame<T,int32_t>,
      internal::OrExpr<internal::IsSame<T,uint32_t>,
      internal::OrExpr<internal::IsSame<T,int64_t>,
      internal::OrExpr<internal::IsSame<T,ObjRef>,
      internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjElement("p"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjPoint(const std::vector<T> &,
	      RAPIDJSON_DISABLEIF((
      internal::OrExpr<internal::IsSame<T,int>,
      internal::OrExpr<internal::IsSame<T,int8_t>,
      internal::OrExpr<internal::IsSame<T,uint8_t>,
      internal::OrExpr<internal::IsSame<T,int16_t>,
      internal::OrExpr<internal::IsSame<T,uint16_t>,
      internal::OrExpr<internal::IsSame<T,int32_t>,
      internal::OrExpr<internal::IsSame<T,uint32_t>,
      internal::OrExpr<internal::IsSame<T,int64_t>,
      internal::OrExpr<internal::IsSame<T,ObjRef>,
      internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjElement("p"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjPoint type is ObjRef"));
  }
  ObjPoint* copy() const override { return new ObjPoint(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjPoint* lhs = this;
    const ObjPoint* rhs = static_cast<const ObjPoint*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((int)(*it));
    return out;
  }
  std::vector<ObjRef> values;
};

class ObjLine : public ObjElement {
public:
  ObjLine(const ObjLine& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjLine(std::istream &in) :
    ObjElement("l"), values() {
    read_values(in);
    // ObjElement::read_values(in, values);
    from_values();
  }
  // template <typename T, size_t N>
  // ObjLine(const T (&src)[N]) : ObjLine(std::vector<T>(src, src+N)) {}
  template <typename T>
  ObjLine(const std::vector<T> &values0,
	  RAPIDJSON_ENABLEIF((
      internal::OrExpr<internal::IsSame<T,ObjRefVertex>,
      internal::OrExpr<internal::IsSame<T,int>,
      internal::OrExpr<internal::IsSame<T,int8_t>,
      internal::OrExpr<internal::IsSame<T,uint8_t>,
      internal::OrExpr<internal::IsSame<T,int16_t>,
      internal::OrExpr<internal::IsSame<T,uint16_t>,
      internal::OrExpr<internal::IsSame<T,int32_t>,
      internal::OrExpr<internal::IsSame<T,uint32_t>,
      internal::OrExpr<internal::IsSame<T,int64_t>,
      internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjElement("l"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjLine(const std::vector<T> &,
	  RAPIDJSON_DISABLEIF((
      internal::OrExpr<internal::IsSame<T,ObjRefVertex>,
      internal::OrExpr<internal::IsSame<T,int>,
      internal::OrExpr<internal::IsSame<T,int8_t>,
      internal::OrExpr<internal::IsSame<T,uint8_t>,
      internal::OrExpr<internal::IsSame<T,int16_t>,
      internal::OrExpr<internal::IsSame<T,uint16_t>,
      internal::OrExpr<internal::IsSame<T,int32_t>,
      internal::OrExpr<internal::IsSame<T,uint32_t>,
      internal::OrExpr<internal::IsSame<T,int64_t>,
      internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjElement("l"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjLine type is ObjRefVertex"));
  }
  void from_values() override
  { RAPIDJSON_ASSERT(values.size() >= 2); }
  ObjLine* copy() const override { return new ObjLine(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjLine* rhs = static_cast<const ObjLine*>(rhs0);
    if (!(is_equal_vectors(this->values, rhs->values))) return false;
    return true;
  }
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((int)(it->v));
    return out;
  }
  std::vector<ObjRefVertex> values;
};

class ObjFace : public ObjElement {
public:
  ObjFace(const ObjFace& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjFace(std::istream &in) :
    ObjElement("f"), values() {
    ObjElement::read_values(in, values);
    // read_values(in);
    from_values();
  }
  // template <typename T, size_t N>
  // ObjFace(const T (&src)[N]) : ObjFace(std::vector<T>(src, src+N)) {}
  template <typename T>
  ObjFace(const std::vector<T> &values0,
	  RAPIDJSON_ENABLEIF((
      internal::OrExpr<internal::IsSame<T,ObjRefVertex>,
      internal::OrExpr<internal::IsSame<T,int>,
      internal::OrExpr<internal::IsSame<T,int8_t>,
      internal::OrExpr<internal::IsSame<T,uint8_t>,
      internal::OrExpr<internal::IsSame<T,int16_t>,
      internal::OrExpr<internal::IsSame<T,uint16_t>,
      internal::OrExpr<internal::IsSame<T,int32_t>,
      internal::OrExpr<internal::IsSame<T,uint32_t>,
      internal::OrExpr<internal::IsSame<T,int64_t>,
      internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjElement("f"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjFace(const std::vector<T> &,
	  RAPIDJSON_DISABLEIF((
      internal::OrExpr<internal::IsSame<T,ObjRefVertex>,
      internal::OrExpr<internal::IsSame<T,int>,
      internal::OrExpr<internal::IsSame<T,int8_t>,
      internal::OrExpr<internal::IsSame<T,uint8_t>,
      internal::OrExpr<internal::IsSame<T,int16_t>,
      internal::OrExpr<internal::IsSame<T,uint16_t>,
      internal::OrExpr<internal::IsSame<T,int32_t>,
      internal::OrExpr<internal::IsSame<T,uint32_t>,
      internal::OrExpr<internal::IsSame<T,int64_t>,
      internal::IsSame<T,uint64_t>>>>>>>>>>))) :
    ObjElement("f"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjFace type is ObjRefVertex"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT(values.size() >= 3);
  }
  ObjFace* copy() const override { return new ObjFace(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjFace* lhs = this;
    const ObjFace* rhs = static_cast<const ObjFace*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((int)(it->v));
    return out;
  }
  std::vector<ObjRefVertex> values;
};

class ObjCurve : public ObjElement {
public:
  ObjCurve(const ObjCurve& rhs) :
    ObjElement(rhs), values(rhs.values), u0(rhs.u0), u1(rhs.u1) {}
  ObjCurve(std::istream &in) :
    ObjElement("curv"), values(), u0(0), u1(0) {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjCurve(const T (&src)[N]) : ObjCurve(std::vector<T>(src, src+N)) {}
  ObjCurve(const std::vector<ObjRef> &values0) :
    ObjElement("curv"), values(), u0(0), u1(0) {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjCurve(const std::vector<T> &) :
    ObjElement("curv"), values(), u0(0), u1(0) {
    RAPIDJSON_ASSERT(!sizeof("ObjCurve type is ObjRef"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT(values.size() >= 2);
  }
  ObjCurve* copy() const override { return new ObjCurve(*this); }
  void read_values(std::istream &in) override {
    in >> u0;
    in >> u1;
    ObjElement::read_values(in, values);
  }
  void write_values(std::ostream &out) const override {
    out << u0 << " " << u1 << " ";
    ObjElement::write_values(out, values);
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjCurve* lhs = this;
    const ObjCurve* rhs = static_cast<const ObjCurve*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (!IS_EQUAL_DBL(lhs->u0, rhs->u0)) return false;
    if (!IS_EQUAL_DBL(lhs->u1, rhs->u1)) return false;
    return true;
  }
  std::vector<double> get_double_array() const override {
    std::vector<double> out({u0, u1});
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((double)(*it));
    return out;
  }
  std::vector<ObjRef> values;
  double u0;
  double u1;
};

class ObjCurve2D : public ObjElement {
public:
  ObjCurve2D(const ObjCurve2D& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjCurve2D(std::istream &in) :
    ObjElement("curv2"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjCurve2D(const T (&src)[N]) : ObjCurve2D(std::vector<T>(src, src+N)) {}
  ObjCurve2D(const std::vector<ObjRef> &values0) :
    ObjElement("curv2"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjCurve2D(const std::vector<T> &) :
    ObjElement("curv2"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjCurve2D type is ObjRef"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT(values.size() >= 2);
  }
  ObjCurve2D* copy() const override { return new ObjCurve2D(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjCurve2D* lhs = this;
    const ObjCurve2D* rhs = static_cast<const ObjCurve2D*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((int)(*it));
    return out;
  }
  std::vector<ObjRef> values;
};

class ObjSurface : public ObjElement {
public:
  ObjSurface(const ObjSurface& rhs) :
    ObjElement(rhs), values(rhs.values), s0(rhs.s0), s1(rhs.s1), t0(rhs.t0), t1(rhs.t1) {}
  ObjSurface(std::istream &in) :
    ObjElement("surf"), values(), s0(0), s1(0), t0(0), t1(0) {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjSurface(const T (&src)[N]) : ObjSurface(std::vector<T>(src, src+N)) {}
  ObjSurface(const std::vector<ObjRefVertex> &values0) :
    ObjElement("surf"), values(), s0(0), s1(0), t0(0), t1(0) {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjSurface(const std::vector<T> &) :
    ObjElement("surf"), values(), s0(0), s1(0), t0(0), t1(0) {
    RAPIDJSON_ASSERT(!sizeof("ObjSurface type is ObjRefVertex"));
  }
  ObjSurface* copy() const override { return new ObjSurface(*this); }
  void read_values(std::istream &in) override {
    in >> s0;
    in >> s1;
    in >> t0;
    in >> t1;
    ObjElement::read_values(in, values);
  }
  void write_values(std::ostream &out) const override {
    out << s0 << " " << s1 << " " << t0 << " " << t1 << " ";
    ObjElement::write_values(out, values);
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjSurface* lhs = this;
    const ObjSurface* rhs = static_cast<const ObjSurface*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (!IS_EQUAL_DBL(lhs->s0, rhs->s0)) return false;
    if (!IS_EQUAL_DBL(lhs->s1, rhs->s1)) return false;
    if (!IS_EQUAL_DBL(lhs->t0, rhs->t0)) return false;
    if (!IS_EQUAL_DBL(lhs->t1, rhs->t1)) return false;
    return true;
  }
  std::vector<double> get_double_array() const override {
    std::vector<double> out({s0, s1, t0, t1});
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((double)(it->v));
    return out;
  }
  std::vector<ObjRefVertex> values;
  double s0;
  double s1;
  double t0;
  double t1;
};

// Free-form elements
class ObjFreeFormType : public ObjElement {
public:
  ObjFreeFormType(const ObjFreeFormType& rhs) :
    ObjElement(rhs), values(rhs.values), elements() {
    for (auto it = rhs.elements.begin(); it != rhs.elements.end(); it++)
      elements.push_back((*it)->copy());
  }
  ObjFreeFormType(std::istream &in) :
    ObjElement("cstype"), values(), elements() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjFreeFormType(const T (&src)[N]) : ObjFreeFormType(std::vector<T>(src, src+N)) {}
  ObjFreeFormType(const std::vector<std::string> &values0) :
    ObjElement("cstype"), values(), elements() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjFreeFormType(const std::vector<T> &) :
    ObjElement("cstype"), values(), elements() {
    RAPIDJSON_ASSERT(!sizeof("ObjFreeFormType type is string"));
  }
  ~ObjFreeFormType() {
    for (auto it = elements.begin(); it != elements.end(); it++)
      delete *it;
    elements.resize(0);
  }
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 1)
		     || (values.size() == 2));
  }
  ObjFreeFormType* copy() const override { return new ObjFreeFormType(*this); }
  void read_values(std::istream &in) override {
    ObjElement::read_values(in, values);
    read_obj_elements(in, elements);
  }
  void write_values(std::ostream &out) const override {
    ObjElement::write_values(out, values);
    out << std::endl;
    for (auto it = elements.begin(); it != elements.end(); it++)
      (*it)->write(out);
    out << "end";
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjFreeFormType* lhs = this;
    const ObjFreeFormType* rhs = static_cast<const ObjFreeFormType*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (lhs->elements.size() != rhs->elements.size()) return false;
    for (auto lit = lhs->elements.begin(), rit = rhs->elements.begin();
	 lit != lhs->elements.end(); lit++, rit++) {
      if (!((*lit)->is_equal(*rit)))
	return false;
    }
    return true;
  }
  std::vector<std::string> get_string_array() const override {
    std::vector<std::string> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back(*it);
    for (auto it = elements.begin(); it != elements.end(); it++)
      out.push_back((*it)->as_string());
    return out;
  }
  std::vector<std::string> values;
  std::vector<ObjElement*> elements;
};

class ObjDegree : public ObjElement {
public:
  ObjDegree(const ObjDegree& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjDegree(std::istream &in) :
    ObjElement("deg"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjDegree(const T (&src)[N]) : ObjDegree(std::vector<T>(src, src+N)) {}
  ObjDegree(const std::vector<uint16_t> &values0) :
    ObjElement("deg"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjDegree(const std::vector<T> &) :
    ObjElement("deg"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjDegree type is uint16_t"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 1) || (values.size() == 2));
  }
  ObjDegree* copy() const override { return new ObjDegree(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjDegree* lhs = this;
    const ObjDegree* rhs = static_cast<const ObjDegree*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back(*it);
    return out;
  }
  std::vector<uint16_t> values;
};

class ObjBasisMatrix : public ObjElement {
public:
  ObjBasisMatrix(const ObjBasisMatrix& rhs) :
    ObjElement(rhs), values(rhs.values), direction(rhs.direction) {}
  ObjBasisMatrix(std::istream &in) :
    ObjElement("bmat"), values(), direction("") {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjBasisMatrix(const T (&src)[N]) : ObjBasisMatrix(std::vector<T>(src, src+N)) {}
  ObjBasisMatrix(const std::vector<double> &values0) :
    ObjElement("bmat"), values(), direction("") {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjBasisMatrix(const std::vector<T> &) :
    ObjElement("bmat"), values(), direction("") {
    RAPIDJSON_ASSERT(!sizeof("ObjBasisMatrix type is double"));
  }
  ObjBasisMatrix* copy() const override { return new ObjBasisMatrix(*this); }
  void read_values(std::istream &in) override {
    in >> direction;
    RAPIDJSON_ASSERT((direction == "u") || (direction == "v"));
    ObjElement::read_values(in, values);
  }
  void write_values(std::ostream &out) const override {
    out << direction << " ";
    ObjElement::write_values(out, values);
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjBasisMatrix* lhs = this;
    const ObjBasisMatrix* rhs = static_cast<const ObjBasisMatrix*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (lhs->direction != rhs->direction) return false;
    return true;
  }
  std::vector<double> values;
  std::string direction;
};

class ObjStep : public ObjElement {
public:
  ObjStep(const ObjStep& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjStep(std::istream &in) :
    ObjElement("step"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjStep(const T (&src)[N]) : ObjStep(std::vector<T>(src, src+N)) {}
  ObjStep(const std::vector<double> &values0) :
    ObjElement("step"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjStep(const std::vector<T> &) :
    ObjElement("step"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjStep type is double"));
  }
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 1) || (values.size() == 2));
  }
  ObjStep* copy() const override { return new ObjStep(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjStep* lhs = this;
    const ObjStep* rhs = static_cast<const ObjStep*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<double> get_double_array() const override {
    std::vector<double> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back(*it);
    return out;
  }
  std::vector<double> values;
};

class ObjParameter : public ObjElement {
public:
  ObjParameter(const ObjParameter& rhs) :
    ObjElement(rhs), values(rhs.values), direction(rhs.direction) {}
  ObjParameter(std::istream &in) :
    ObjElement("parm"), values(), direction("") {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjParameter(const T (&src)[N]) : ObjParameter(std::vector<T>(src, src+N)) {}
  ObjParameter(const std::vector<double> &values0) :
    ObjElement("parm"), values(), direction("") {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjParameter(const std::vector<T> &) :
    ObjElement("parm"), values(), direction("") {
    RAPIDJSON_ASSERT(sizeof("ObjParameter type is double"));
  }
  ObjParameter* copy() const override { return new ObjParameter(*this); }
  void read_values(std::istream &in) override {
    in >> direction;
    RAPIDJSON_ASSERT((direction == "u") || (direction == "v"));
    ObjElement::read_values(in, values);
  }
  void write_values(std::ostream &out) const override {
    out << direction << " ";
    ObjElement::write_values(out, values);
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjParameter* lhs = this;
    const ObjParameter* rhs = static_cast<const ObjParameter*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (lhs->direction != rhs->direction) return false;
    return true;
  }
  std::vector<double> values;
  std::string direction;
};

class ObjTrim : public ObjElement {
public:
  ObjTrim(const ObjTrim& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjTrim(std::istream &in) :
    ObjElement("trim"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjTrim(const T (&src)[N]) : ObjTrim(std::vector<T>(src, src+N)) {}
  ObjTrim(const std::vector<ObjRefCurve> &values0) :
    ObjElement("trim"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjTrim(const std::vector<T> &) :
    ObjElement("trim"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjTrim type is ObjRefCurve"));
  }
  ObjTrim* copy() const override { return new ObjTrim(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjTrim* lhs = this;
    const ObjTrim* rhs = static_cast<const ObjTrim*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<ObjRefCurve> values;
};

class ObjHole : public ObjElement {
public:
  ObjHole(const ObjHole& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjHole(std::istream &in) :
    ObjElement("hole"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjHole(const T (&src)[N]) : ObjHole(std::vector<T>(src, src+N)) {}
  ObjHole(const std::vector<ObjRefCurve> &values0) :
    ObjElement("hole"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjHole(const std::vector<T> &) :
    ObjElement("hole"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjHole type is ObjRefCurve"));
  }
  ObjHole* copy() const override { return new ObjHole(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjHole* lhs = this;
    const ObjHole* rhs = static_cast<const ObjHole*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<ObjRefCurve> values;
};

class ObjScrv : public ObjElement {
public:
  ObjScrv(const ObjScrv& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjScrv(std::istream &in) :
    ObjElement("scrv"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjScrv(const T (&src)[N]) : ObjScrv(std::vector<T>(src, src+N)) {}
  ObjScrv(const std::vector<ObjRefCurve> &values0) :
    ObjElement("scrv"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjScrv(const std::vector<T> &) :
    ObjElement("scrv"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjScrv type is ObjRefCurve"));
  }
  ObjScrv* copy() const override { return new ObjScrv(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjScrv* lhs = this;
    const ObjScrv* rhs = static_cast<const ObjScrv*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<ObjRefCurve> values;
};

class ObjSpecialPoints : public ObjElement {
public:
  ObjSpecialPoints(const ObjSpecialPoints& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjSpecialPoints(std::istream &in) :
    ObjElement("sp"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjSpecialPoints(const T (&src)[N]) : ObjSpecialPoints(std::vector<T>(src, src+N)) {}
  ObjSpecialPoints(const std::vector<ObjRef> &values0) :
    ObjElement("sp"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjSpecialPoints(const std::vector<T> &) :
    ObjElement("sp"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjSpecialPoints type is ObjRef"));
  }
  ObjSpecialPoints* copy() const override { return new ObjSpecialPoints(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjSpecialPoints* lhs = this;
    const ObjSpecialPoints* rhs = static_cast<const ObjSpecialPoints*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<ObjRef> values;
};

class ObjConnect : public ObjElement {
public:
  ObjConnect(const ObjConnect& rhs) :
    ObjElement(rhs), values(rhs.values) {}
  ObjConnect(std::istream &in) :
    ObjElement("con"), values() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjConnect(const T (&src)[N]) : ObjConnect(std::vector<T>(src, src+N)) {}
  ObjConnect(const std::vector<ObjRefSurface> &values0) :
    ObjElement("con"), values() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjConnect(const std::vector<T> &) :
    ObjElement("con"), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjConnect type is ObjRefSurface"));
  }
  ObjConnect* copy() const override { return new ObjConnect(*this); }
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjConnect* lhs = this;
    const ObjConnect* rhs = static_cast<const ObjConnect*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  std::vector<ObjRefSurface> values;
};

// Grouping
class ObjGroup : public ObjElement {
public:
  ObjGroup(const ObjGroup& rhs) :
    ObjElement(rhs), values(rhs.values), elements() {
    for (auto it = rhs.elements.begin(); it != rhs.elements.end(); it++)
      elements.push_back((*it)->copy());
  }
  ObjGroup(std::istream &in) :
    ObjElement("g"), values(), elements() {
    read_values(in);
    from_values();
  }
  template <typename T, size_t N>
  ObjGroup(const T (&src)[N]) : ObjGroup(std::vector<T>(src, src+N)) {}
  ObjGroup(const std::vector<std::string> &values0) :
    ObjElement("g"), values(), elements() {
    assign_values(values, values0);
    from_values();
  }
  template <typename T>
  ObjGroup(const std::vector<T> &) :
    ObjElement("g"), values(), elements() {
    RAPIDJSON_ASSERT(!sizeof("ObjGroup type is string"));
  }
  ~ObjGroup() {
    for (auto it = elements.begin(); it != elements.end(); it++)
      delete *it;
    elements.resize(0);
  }
  ObjGroup* copy() const override { return new ObjGroup(*this); }
  void read_values(std::istream &in) override {
    ObjElement::read_values(in, values);
    read_obj_elements(in, elements, 'g');
  }
  void write_values(std::ostream &out) const override {
    ObjElement::write_values(out, values);
    out << std::endl;
    for (auto it = elements.begin(); it != elements.end(); it++)
      (*it)->write(out);
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjGroup* lhs = this;
    const ObjGroup* rhs = static_cast<const ObjGroup*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (lhs->elements.size() != rhs->elements.size()) return false;
    for (auto lit = lhs->elements.begin(), rit = rhs->elements.begin();
	 lit != lhs->elements.end(); lit++, rit++) {
      if (!((*lit)->is_equal(*rit)))
	return false;
    }
    return true;
  }
  std::vector<std::string> values;
  std::vector<ObjElement*> elements;
};

class ObjSmoothingGroup : public ObjElement {
public:
  ObjSmoothingGroup(const ObjSmoothingGroup& rhs) :
    ObjElement(rhs), group_number(rhs.group_number) {}
  ObjSmoothingGroup(std::istream &in) :
    ObjElement("s"), group_number(-1) {
    read_values(in);
  }
  template <typename T, size_t N>
  ObjSmoothingGroup(const T (&src)[N]) : ObjSmoothingGroup(std::vector<T>(src, src+N)) {}
  template <typename T>
  ObjSmoothingGroup(const std::vector<T> &) :
    ObjElement("s"), group_number(-1) {
    RAPIDJSON_ASSERT(!sizeof("ObjSmoothingGroup cannot be initialized from an array"));
  }
  ObjSmoothingGroup* copy() const override { return new ObjSmoothingGroup(*this); }
  void read_values(std::istream &in) override {
    std::string word;
    in >> word;
    if (word == "off")
      group_number = -1;
    else
      group_number = stoi(word);
  }
  void write_values(std::ostream &out) const override {
    if (group_number < 0)
      out << "off";
    else
      out << group_number;
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjSmoothingGroup* lhs = this;
    const ObjSmoothingGroup* rhs = static_cast<const ObjSmoothingGroup*>(rhs0);
    if (lhs->group_number != rhs->group_number) return false;
    return true;
  }
  ObjRef group_number;
};

class ObjMergingGroup : public ObjElement {
public:
  ObjMergingGroup(const ObjMergingGroup& rhs) :
    ObjElement(rhs), group_number(rhs.group_number), resolution(rhs.resolution) {}
  ObjMergingGroup(std::istream &in) :
    ObjElement("mg"), group_number(-1), resolution(0) {
    read_values(in);
  }
  template <typename T, size_t N>
  ObjMergingGroup(const T (&src)[N]) : ObjMergingGroup(std::vector<T>(src, src+N)) {}
  template <typename T>
  ObjMergingGroup(const std::vector<T> &) :
    ObjElement("mg"), group_number(-1), resolution(0) {
    RAPIDJSON_ASSERT(!sizeof("ObjMergingGroup type cannot be initialized from an array"));
  }
  ObjMergingGroup* copy() const override { return new ObjMergingGroup(*this); }
  void read_values(std::istream &in) override {
    std::string word;
    in >> word;
    if (word == "off") {
      group_number = -1;
      resolution = 0.0;
    } else {
      group_number = stoi(word);
      in >> resolution;
    }
  }
  void write_values(std::ostream &out) const override {
    if (group_number < 0)
      out << "off";
    else {
      out << group_number << " " << resolution;
    }
  }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjMergingGroup* lhs = this;
    const ObjMergingGroup* rhs = static_cast<const ObjMergingGroup*>(rhs0);
    if (lhs->group_number != rhs->group_number) return false;
    if (!(IS_EQUAL_DBL(lhs->resolution, rhs->resolution))) return false;
    return true;
  }
  ObjRef group_number;
  double resolution;
};

class ObjObjectName : public ObjElement {
public:
  ObjObjectName(const ObjObjectName& rhs) :
    ObjElement(rhs), name(rhs.name) {}
  ObjObjectName(std::istream &in) :
    ObjElement("o"), name("") {
    read_values(in);
  }
  template <typename T, size_t N>
  ObjObjectName(const T (&src)[N]) : ObjObjectName(std::vector<T>(src, src+N)) {}
  template <typename T>
  ObjObjectName(const std::vector<T> &) :
    ObjElement("o"), name("") {
    RAPIDJSON_ASSERT(!sizeof("ObjObjectName cannot be initialized from an array"));
  }
  ObjObjectName* copy() const override { return new ObjObjectName(*this); }
  void read_values(std::istream &in) override { in >> name; }
  void write_values(std::ostream &out) const override { out << name; }
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjObjectName* rhs = static_cast<const ObjObjectName*>(rhs0);
    if (this->name != rhs->name) return false;
    return true;
  }
  std::string name;
};

inline ObjElement* read_obj_element(std::istream &in) {
  std::string word = "";
  ObjElement* out = NULL;
  if (in >> word)
    OBJ_ELEMENT_INIT(word, out, (in));
  return out;
};

inline void read_obj_elements(std::istream &in, std::vector<ObjElement*> &elements,
			      const char break_at) {
  while (true) {
    if (break_at) {
      in >> std::skipws;
      if (in.peek() == break_at)
	return;
    }
    ObjElement* x = read_obj_element(in);
    if (x == NULL)
      break;
    elements.push_back(x);
  }
};

// Actual Obj wavefront class
class ObjWavefront {
public:
  ObjWavefront() : elements() {}
  ObjWavefront(const ObjWavefront& rhs) : elements() {
    for (auto it = rhs.elements.begin(); it != rhs.elements.end(); it++)
      elements.push_back((*it)->copy());
  }
  template<typename Tv, size_t Mv, size_t Nv>
  ObjWavefront(const Tv (&vertices)[Mv][Nv]) :
    elements() {
    add_element_set("v", vertices);
  }
  template<typename Tv, size_t Mv, size_t Nv,
	   typename Tf, size_t Mf, size_t Nf>
  ObjWavefront(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf]={}) :
    ObjWavefront(vertices) {
    add_element_set("f", faces);
  }
  template<typename Tv, size_t Mv, size_t Nv,
	   typename Tf, size_t Mf, size_t Nf,
	   typename Te, size_t Me, size_t Ne>
  ObjWavefront(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf]={},
	       const Te (&edges)[Me][Ne]={}) :
    ObjWavefront(vertices, faces) {
    add_element_set("l", edges);
  }
  ~ObjWavefront() {
    for (auto it = elements.begin(); it != elements.end(); it++)
      delete *it;
    // elements.resize(0);
  }
  std::vector<ObjElement*> elements;

  template <typename T, size_t M, size_t N>
  void add_element_set(std::string name, const T (&values)[M][N]) {
    for (SizeType i = 0; i < M; i++)
      add_element(name, values[i]);
  }
  template <typename T, size_t N>
  void add_element(std::string name, const T (&values)[N]) {
    ObjElement* x = NULL;
    OBJ_ELEMENT_INIT(name, x, (std::vector<T>(values, values+N)));
    elements.push_back(x);
  }
  template <typename T>
  void add_element(std::string name, const std::vector<T> &values) {
    ObjElement* x = NULL;
    OBJ_ELEMENT_INIT(name, x, (values));
    elements.push_back(x);
  }

  bool is_equal(const ObjWavefront& rhs) const {
    const ObjWavefront& lhs = *this;
    if (lhs.elements.size() != rhs.elements.size())
      return false;
    for (auto lit = lhs.elements.begin(), rit = rhs.elements.begin();
	 lit != lhs.elements.end(); lit++, rit++) {
      if (!((*lit)->is_equal(*rit)))
	return false;
    }
    return true;
  }
  std::ostream & write(std::ostream &out) const {
    for (auto it = elements.begin(); it != elements.end(); it++)
      (*it)->write(out);
    return out;
  }
  std::istream & read(std::istream &in) {
    read_obj_elements(in, elements);
    return in;
  }
  friend bool operator == (const ObjWavefront& lhs, const ObjWavefront& rhs);
  friend std::ostream & operator << (std::ostream &out, const ObjWavefront &p);
  friend std::istream & operator >> (std::istream &in,  ObjWavefront &p);
};

inline
bool operator == (const ObjWavefront& lhs, const ObjWavefront& rhs)
{ return lhs.is_equal(rhs); };

inline
std::ostream & operator << (std::ostream &out, const ObjWavefront &p)
{ return p.write(out); };

inline
std::istream & operator >> (std::istream &in, ObjWavefront &p)
{ return p.read(in); };


RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_OBJ_H_
