#ifndef RAPIDJSON_YGGDRASIL_H_
#define RAPIDJSON_YGGDRASIL_H_

#include "base64.h"
#include "document.h"
#include "schema.h"

RAPIDJSON_NAMESPACE_BEGIN

template <typename DocumentType = Document>
class GenericGenerator {
public:
  typedef typename DocumentType::Ch Ch;
  typedef typename DocumentType::ValueType ValueType;
  typedef typename DocumentType::AllocatorType AllocatorType;

private:
  GenericGenerator(const GenericGenerator& rhs);
  GenericGenerator& operator=(const GenericGenerator& rhs);
public:
  GenericGenerator(DocumentType& source) : source_(source), current_dst_(NULL) {}

  template <typename Handler>
  bool operator()(Handler& handler) {
    current_dst_ = &handler;
    bool result = source_.Accept(*this);
    current_dst_ = NULL;
    return result;
  }
  virtual ~GenericGenerator() {}
  
  // Check that generated value is valid?
#define RAPIDJSON_GENERATE_HANDLE_CREATE_(method, arg)	       \
  ValueType val = Generate##method arg;			       \
  return val.Accept(*current_dst_);
  
  virtual bool Null()             { RAPIDJSON_GENERATE_HANDLE_CREATE_(Null, ()); }
  virtual bool Bool(bool b)       { RAPIDJSON_GENERATE_HANDLE_CREATE_(Bool, (b)); }
  virtual bool Int(int i)         { RAPIDJSON_GENERATE_HANDLE_CREATE_(Int, (i)); }
  virtual bool Uint(unsigned u)   { RAPIDJSON_GENERATE_HANDLE_CREATE_(Uint, (u)); }
  virtual bool Int64(int64_t i)   { RAPIDJSON_GENERATE_HANDLE_CREATE_(Int64, (i)); }
  virtual bool Uint64(uint64_t u) { RAPIDJSON_GENERATE_HANDLE_CREATE_(Uint64, (u)); }
  virtual bool Double(double d)   { RAPIDJSON_GENERATE_HANDLE_CREATE_(Double, (d)); }
  virtual bool RawNumber(const Ch* str, SizeType length, bool copy) {
    RAPIDJSON_GENERATE_HANDLE_CREATE_(String, (str, length, copy)); }
  virtual bool String(const Ch* str, SizeType length, bool copy) {
    RAPIDJSON_GENERATE_HANDLE_CREATE_(String, (str, length, copy)); }
#ifdef RAPIDJSON_YGGDRASIL
  virtual bool Yggdrasil(const Ch* str, SizeType length, bool copy, ValueType& schema) {
    RAPIDJSON_GENERATE_HANDLE_CREATE_(Yggdrasil, (str, length, copy, schema)); }
  virtual bool Yggdrasil(const GenericObject<false, ValueType>& o, ValueType& schema) {
    RAPIDJSON_GENERATE_HANDLE_CREATE_(Yggdrasil, (o, schema)); }
#endif // RAPIDJSON_YGGDRASIL
  virtual bool StartObject() {
    return current_dst_->StartObject(); }
  virtual bool Key(const Ch* str, SizeType len, bool copy) {
    return current_dst_->Key(str, len, copy); }
  virtual bool EndObject(SizeType memberCount) {
    return current_dst_->EndObject(memberCount); }
  virtual bool StartArray() {
    return current_dst_->StartArray(); }
  virtual bool EndArray(SizeType elementCount) {
    return current_dst_->EndArray(elementCount); }

#undef RAPIDJSON_GENERATE_HANDLE_CREATE_

#define RAPIDJSON_DIRECT_GENERATOR_(arg)				\
  return ValueType arg
  
  virtual ValueType GenerateNull()             { RAPIDJSON_DIRECT_GENERATOR_(()); }
  virtual ValueType GenerateBool(bool b)       { RAPIDJSON_DIRECT_GENERATOR_((b)); }
  virtual ValueType GenerateInt(int i)         { RAPIDJSON_DIRECT_GENERATOR_((i)); }
  virtual ValueType GenerateUint(unsigned u)   { RAPIDJSON_DIRECT_GENERATOR_((u)); }
  virtual ValueType GenerateInt64(int64_t i)   { RAPIDJSON_DIRECT_GENERATOR_((i)); }
  virtual ValueType GenerateUint64(uint64_t u) { RAPIDJSON_DIRECT_GENERATOR_((u)); }
  virtual ValueType GenerateDouble(double d)   { RAPIDJSON_DIRECT_GENERATOR_((d)); }
  virtual ValueType GenerateRawNumber(const Ch* str, SizeType length, bool copy) {
    return GenerateString(str, length, copy); }
  virtual ValueType GenerateString(const Ch* str, SizeType length, bool copy) {
    if (copy) {
      AllocatorType& allocator = source_.GetAllocator();
      RAPIDJSON_DIRECT_GENERATOR_((str, length, allocator));
    } else {
      RAPIDJSON_DIRECT_GENERATOR_((str, length));
    }
  }
#ifdef RAPIDJSON_YGGDRASIL
  virtual ValueType GenerateYggdrasil(const Ch* str, SizeType length, bool copy, ValueType& schema) {
    if (copy) {
      AllocatorType& allocator = source_.GetAllocator();
      RAPIDJSON_DIRECT_GENERATOR_((str, length, allocator, schema));
    } else {
      RAPIDJSON_DIRECT_GENERATOR_((str, length, schema));
    }
  }
  virtual ValueType GenerateYggdrasil(const GenericObject<false, ValueType>& o, ValueType& schema) {
    AllocatorType& allocator = source_.GetAllocator();
    RAPIDJSON_DIRECT_GENERATOR_((o, allocator, schema));
  }
#endif // RAPIDJSON_YGGDRASIL
  
protected:
  DocumentType& source_;
  DocumentType* current_dst_;
};


template <typename DocumentType = Document>
class GenericSchemaEncoder :
  public GenericGenerator<DocumentType> {
public:
  typedef GenericGenerator<DocumentType> GenericType;
  typedef typename DocumentType::ValueType ValueType;
  typedef typename DocumentType::AllocatorType AllocatorType;
  typedef typename DocumentType::Ch Ch;
  typedef typename GenericSchemaDocument<ValueType>::SchemaType SchemaType;
  GenericSchemaEncoder(DocumentType& source) : GenericGenerator<DocumentType>(source) {}

  bool StartObject() override {
    const ValueType& type_str = SchemaType::GetTypeString();
    const ValueType& object_str = SchemaType::GetObjectString();
    const ValueType& properties_str = SchemaType::GetPropertiesString();
    if (!GenericType::current_dst_->StartObject()) return false;
    if (!GenericType::current_dst_->Key(type_str.GetString(),
					type_str.GetStringLength(),
					true)) return false;
    if (!GenericType::current_dst_->String(object_str.GetString(),
					   object_str.GetStringLength(),
					   true)) return false;
    if (!GenericType::current_dst_->Key(properties_str.GetString(),
					properties_str.GetStringLength(),
					true)) return false;
    return GenericType::current_dst_->StartObject();
  }
  bool EndObject(SizeType memberCount) override {
    // End properties property then end the object's schema
    GenericType::current_dst_->EndObject(memberCount);
    // Object's schema should have elements for type & properties
    return GenericType::current_dst_->EndObject(2); }
  bool StartArray() override {
    const ValueType& type_str = SchemaType::GetTypeString();
    const ValueType& array_str = SchemaType::GetArrayString();
    const ValueType& items_str = SchemaType::GetItemsString();
    if (!GenericType::current_dst_->StartObject()) return false;
    if (!GenericType::current_dst_->Key(type_str.GetString(),
					type_str.GetStringLength(),
					true)) return false;
    if (!GenericType::current_dst_->String(array_str.GetString(),
					   array_str.GetStringLength(),
					   true)) return false;
    if (!GenericType::current_dst_->Key(items_str.GetString(),
					items_str.GetStringLength(),
					true)) return false;
    return GenericType::current_dst_->StartArray(); }
  bool EndArray(SizeType elementCount) override {
    // End items property then end the array's schema
    if (!GenericType::current_dst_->EndArray(elementCount))
      return false;
    // Array's schema should have elements for type & items
    return GenericType::current_dst_->EndObject(2); }
  
#define RAPIDJSON_INIT_SCHEMA_(name)				\
  ValueType out(kObjectType);					\
  const ValueType& type_prop_str = SchemaType::GetTypeString();	\
  const ValueType& type_name_str = SchemaType::Get##name##String();\
  out.AddMember(ValueType(type_prop_str.GetString(), type_prop_str.GetStringLength()),\
		ValueType(type_name_str.GetString(), type_name_str.GetStringLength()), \
		GenericType::current_dst_->GetAllocator());		\
  return out
  
  ValueType GenerateNull() override            {
    RAPIDJSON_INIT_SCHEMA_(Null); }
  ValueType GenerateBool(bool) override        {
    RAPIDJSON_INIT_SCHEMA_(Boolean); }
  ValueType GenerateInt(int) override          {
    RAPIDJSON_INIT_SCHEMA_(Integer); }
  ValueType GenerateUint(unsigned) override    {
    RAPIDJSON_INIT_SCHEMA_(Integer); }
  ValueType GenerateInt64(int64_t) override    {
    RAPIDJSON_INIT_SCHEMA_(Integer); }
  ValueType GenerateUint64(uint64_t) override  {
    RAPIDJSON_INIT_SCHEMA_(Integer); }
  ValueType GenerateDouble(double) override    {
    RAPIDJSON_INIT_SCHEMA_(Number); }
  ValueType GenerateRawNumber(const Ch*, SizeType, bool) override {
    RAPIDJSON_INIT_SCHEMA_(Number); }
  ValueType GenerateString(const Ch*, SizeType, bool) override {
    RAPIDJSON_INIT_SCHEMA_(String); }
#ifdef RAPIDJSON_YGGDRASIL
  ValueType GenerateYggdrasil(const Ch*, SizeType, bool, ValueType& schema) override {
    ValueType out;
    out.CopyFrom(schema, GenericType::current_dst_->GetAllocator());
    return out;
  }
  bool Yggdrasil(const GenericObject<false, ValueType>& o, ValueType& schema) override {
    const ValueType& type_str = SchemaType::GetTypeString();
    const ValueType& inst_str = SchemaType::GetPythonInstanceString();
    const ValueType& args_str = SchemaType::GetArgsString();
    const ValueType& kwargs_str = SchemaType::GetKwargsString();
    if (!(schema.IsObject())) return false;
    if (!(schema.HasMember(type_str))) return false;
    if (schema.FindMember(type_str)->value != inst_str) return false;
    if (!GenericType::current_dst_->StartObject()) return false;
    if (!GenericType::current_dst_->Key(type_str.GetString(),
					type_str.GetStringLength(),
					true)) return false;
    if (!GenericType::current_dst_->String(inst_str.GetString(),
					   inst_str.GetStringLength(),
					   true)) return false;
    SizeType memberCount = 1;
    if (o.HasMember(args_str)) {
      memberCount++;
      if (!GenericType::current_dst_->Key(args_str.GetString(),
					  args_str.GetStringLength(),
					  true)) return false;
      if (!(o.FindMember(args_str)->value.Accept(*this)))
	return false;
    }
    if (o.HasMember(kwargs_str)) {
      memberCount++;
      if (!GenericType::current_dst_->Key(kwargs_str.GetString(),
					  kwargs_str.GetStringLength(),
					  true)) return false;
      if (!(o.FindMember(kwargs_str)->value.Accept(*this)))
	return false;
    }
    return GenericType::current_dst_->EndObject(memberCount);
  }
#endif // RAPIDJSON_YGGDRASIL
#undef RAPIDJSON_INIT_SCHEMA_

};

typedef GenericSchemaEncoder<Document> SchemaEncoder;


RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_YGGDRASIL_H_
// Local Variables:
// mode: c++
// End:
