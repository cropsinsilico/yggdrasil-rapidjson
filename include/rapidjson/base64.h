/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "rapidjson.h"

#ifndef BASE64_H_
#define BASE64_H_

#include "encodings.h"

RAPIDJSON_NAMESPACE_BEGIN

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

static const unsigned char base64_table[65] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char base64_table_last = '=';

/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
static inline unsigned char * base64_encode(const unsigned char *src, size_t len,
					    size_t *out_len)
{
  unsigned char *out, *pos;
  const unsigned char *end, *in;
  size_t olen;
  int line_len;

  olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
  olen += olen / 72; /* line feeds */
  olen++; /* nul termination */
  if (olen < len)
    return NULL; /* integer overflow */
  out = (unsigned char *)malloc(olen);
  if (out == NULL)
    return NULL;

  end = src + len;
  in = src;
  pos = out;
  line_len = 0;
  while (end - in >= 3) {
    *pos++ = base64_table[in[0] >> 2];
    *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
    *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
    *pos++ = base64_table[in[2] & 0x3f];
    in += 3;
    line_len += 4;
    if (line_len >= 72) {
      *pos++ = '\n';
      line_len = 0;
    }
  }

  if (end - in) {
    *pos++ = base64_table[in[0] >> 2];
    if (end - in == 1) {
      *pos++ = base64_table[(in[0] & 0x03) << 4];
      *pos++ = base64_table_last;
    } else {
      *pos++ = base64_table[((in[0] & 0x03) << 4) |
			    (in[1] >> 4)];
      *pos++ = base64_table[(in[1] & 0x0f) << 2];
    }
    *pos++ = base64_table_last;
    line_len += 4;
  }

  if (line_len)
    *pos++ = '\n';

  *pos = '\0';
  if (out_len)
    *out_len = (size_t)(pos - out);
  return out;
}


/**
 * base64_decode - Base64 decode
 * @src: Data to be decoded
 * @len: Length of the data to be decoded
 * @out_len: Pointer to output length variable
 * Returns: Allocated buffer of out_len bytes of decoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
static inline unsigned char * base64_decode(const unsigned char *src, size_t len,
					    size_t *out_len)
{
  unsigned char dtable[256], *out, *pos, block[4], tmp;
  size_t i, count, olen;
  int pad = 0;

  
  memset(dtable, 0x80, 256);
  for (i = 0; i < sizeof(base64_table) - 1; i++)
    dtable[base64_table[i]] = (unsigned char) i;
  dtable[base64_table_last] = 0;

  count = 0;
  for (i = 0; i < len; i++) {
    if (dtable[src[i]] != 0x80)
      count++;
  }

  if (count == 0 || count % 4)
    return NULL;

  olen = count / 4 * 3;
  pos = out = (unsigned char *)malloc(olen);
  if (out == NULL)
    return NULL;

  count = 0;
  for (i = 0; i < len; i++) {
    tmp = dtable[src[i]];
    if (tmp == 0x80)
      continue;

    if (src[i] == base64_table_last)
      pad++;
    block[count] = tmp;
    count++;
    if (count == 4) {
      *pos++ = (unsigned char)((block[0] << 2) | (block[1] >> 4));
      *pos++ = (unsigned char)((block[1] << 4) | (block[2] >> 2));
      *pos++ = (unsigned char)((block[2] << 6) | block[3]);
      count = 0;
      if (pad) {
	if (pad == 1)
	  pos--;
	else if (pad == 2)
	  pos -= 2;
	else {
	  /* Invalid padding */
	  free(out);
	  return NULL;
	}
	break;
      }
    }
  }

  *out_len = (size_t)(pos - out);
  return out;
}

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

// template <typename StreamType>
// class Base64StreamWrapper  {
// public:
//   typedef typename StreamType::Ch Ch;
//   Base64StreamWrapper(StreamType &stream) :
//     stream_(stream), buffer_(),
//     dtable_(), // buffer_empty_(),
//     pos_(0), buffer_pos_(0) {
//     buffer_[0] = '\0';
//     buffer_[1] = '\0';
//     buffer_[2] = '\0';
//     buffer_empty_[0] = true;
//     buffer_empty_[1] = true;
//     buffer_empty_[2] = true;
//     memset(dtable_, 0x80, 256);
//     for (size_t i = 0; i < sizeof(base64_table) - 1; i++)
//       dtable_[base64_table[i]] = (unsigned char) i;
//     dtable_[base64_table_last] = 0;
//   }
// private:
//   StreamType &stream_;
//   unsigned char buffer_[3];
//   unsigned char dtable_[256];
//   bool buffer_empty_[3];
//   size_t pos_;
//   size_t buffer_pos_;
// };


//! Input stream wrapper for decoding base64.
template <typename StreamType>
class Base64InputStreamWrapper {
public:
  //! Character type read from the stream.
  typedef typename StreamType::Ch Ch;
  //! \brief Wrap a stream in a base64 decoder.
  //! \param stream Stream.
  Base64InputStreamWrapper(StreamType &stream) :
    stream_(stream), buffer_(),
    dtable_(), // buffer_empty_(),
    pos_(0), buffer_pos_(0) {
    buffer_[0] = '\0';
    buffer_[1] = '\0';
    buffer_[2] = '\0';
    buffer_empty_[0] = true;
    buffer_empty_[1] = true;
    buffer_empty_[2] = true;
    memset(dtable_, 0x80, 256);
    for (size_t i = 0; i < sizeof(base64_table) - 1; i++)
      dtable_[base64_table[i]] = (unsigned char) i;
    dtable_[base64_table_last] = 0;
    ReadNext();
  }
  //! \brief Peek at the nth byte.
  //! \param n Offset to check for byte at.
  //! \return Byte.
  unsigned char PeekByte(size_t n = 0) {
    if ((buffer_pos_ + n) < 3)
      return buffer_[buffer_pos_ + n];
    RAPIDJSON_ASSERT(buffer_pos_ == 3);  // Don't overwrite buffer
    RAPIDJSON_ASSERT(n < 3);
    ReadNext();
    return buffer_[buffer_pos_ + n];
  }
  //! \brief Peek at the next character.
  //! \return Character.
  Ch Peek() {
    Ch out = '\0';
    unsigned char *bytes = reinterpret_cast<unsigned char*>(&out);
    for (size_t i = 0; i < sizeof(Ch); i++)
      bytes[i] = PeekByte(i);
    return out;
  }
  //! \brief Peek at the next set of bytes and check if there are enough to
  //!   read a character from.
  //! \return true if there are enough bytes for a character.
  bool PeekEmpty() {
    for (size_t i = 0; i < sizeof(Ch); i++) {
      PeekByte(i);
      if (!(buffer_empty_[buffer_pos_ + i])) return false;
    }
    return true;
  }
  //! \brief Take one byte from the stream.
  //! \return Byte.
  unsigned char TakeByte() {
    if (buffer_pos_ >= 3)
      ReadNext();
    buffer_empty_[buffer_pos_] = true;
    return buffer_[buffer_pos_++];
  }
  //! \brief Take a character from the stream.
  //! \return Character.
  Ch Take() {
    Ch out = '\0';
    unsigned char *bytes = reinterpret_cast<unsigned char*>(&out);
    for (size_t i = 0; i < sizeof(Ch); i++)
      bytes[i] = TakeByte();
    pos_++;
    return out;
  }
  //! \brief Report the position in the buffer.
  //! \return Buffer position.
  size_t Tell() { return pos_; } // When is this used?
  
  // unsigned char* PeekNext() {
  //   // std::cerr << "PeekNext" << std::endl;
  //   // RAPIDJSON_ASSERT(false);
  //   // return NULL;
  //   unsigned char *out = (unsigned char*)malloc(3 * sizeof(unsigned char));
  //   unsigned char *pos = out;
  //   unsigned char encoded[4] = {0x80, 0x80, 0x80, 0x80};
  //   unsigned char src = '\0';
  //   size_t pad = 0;
  //   const Ch* next = stream_.Peek4();
  //   for (size_t i = 0; i < 4; i++) {
  //     src = (unsigned char)(next[i]);
  //     encoded[i] = dtable_[src];
  //     if (encoded[i] == 0x80)
  // 	continue;
  //     if (src == base64_table_last)
  // 	pad++;
  //   }
  //   unsigned char *block = encoded;
  //   *pos++ = (unsigned char)((block[0] << 2) | (block[1] >> 4));
  //   *pos++ = (unsigned char)((block[1] << 4) | (block[2] >> 2));
  //   *pos++ = (unsigned char)((block[2] << 6) | block[3]);
  //   for (size_t i = 0; i < pad; i++)
  //     out[2 - i] = '\0';
  //   return out;
  // }

  //! \brief Read the next set of bytes from the stream.
  void ReadNext() {
    // Decode
    unsigned char *pos = &(buffer_[0]);
    unsigned char encoded[4] = {0x80, 0x80, 0x80, 0x80};
    unsigned char src = '\0';
    size_t pad = 0;
    buffer_pos_ = 0;
    buffer_[0] = '\0';
    buffer_[1] = '\0';
    buffer_[2] = '\0';
    buffer_empty_[0] = false;
    buffer_empty_[1] = false;
    buffer_empty_[2] = false;
    if (stream_.Peek() == '\0') {
      buffer_empty_[0] = true;
      buffer_empty_[1] = true;
      buffer_empty_[2] = true;
      return;
    }
    // std::cerr << "Reading from " << stream_.Tell() << std::endl;
    // std::cerr << "  encoded: ";
    for (size_t i = 0; i < 4; i++) {
      while (encoded[i] == 0x80) {
	RAPIDJSON_ASSERT(stream_.Peek() != '\0');
	// std::cerr << stream_.Peek();
	src = (unsigned char)(stream_.Take());
	encoded[i] = dtable_[src];
      }
      if (src == base64_table_last)
	pad++;
    }
    // std::cerr << std::endl;
    unsigned char *block = encoded;
    *pos++ = (unsigned char)((block[0] << 2) | (block[1] >> 4));
    *pos++ = (unsigned char)((block[1] << 4) | (block[2] >> 2));
    *pos++ = (unsigned char)((block[2] << 6) | block[3]);
    for (size_t i = 0; i < pad; i++) {
      buffer_[2 - i] = '\0';
      buffer_empty_[2 - i] = true;
    }
    // std::cerr << "  decoded: ";
    // for (size_t i = 0; i < 3; i++)
    //   std::cerr << buffer_[i];
    // std::cerr << std::endl;
  }
  
  //! \brief Wrapper for stream
  UTFType GetType() const { return stream_.GetType(); }
  
private:
  StreamType &stream_;
  unsigned char buffer_[3];
  unsigned char dtable_[256];
  bool buffer_empty_[3];
  size_t pos_;
  size_t buffer_pos_;
  
};

//! Output stream wrapper that will encode character bytes as base64.
template <typename StreamType>
class Base64OutputStreamWrapper {
public:
  //! Character type writen to the stream.
  typedef typename StreamType::Ch Ch;
  //! Wrap a stream in base64 encoding.
  Base64OutputStreamWrapper(StreamType &stream) :
    stream_(stream), buffer_(),
    dtable_(), // buffer_empty_(),
    pos_(0), buffer_pos_(0) {
    buffer_[0] = '\0';
    buffer_[1] = '\0';
    buffer_[2] = '\0';
    buffer_empty_[0] = true;
    buffer_empty_[1] = true;
    buffer_empty_[2] = true;
    memset(dtable_, 0x80, 256);
    for (size_t i = 0; i < sizeof(base64_table) - 1; i++)
      dtable_[base64_table[i]] = (unsigned char) i;
    dtable_[base64_table_last] = 0;
  }
  //! \brief Reserve enough space in the stream for a certain number of characters.
  //! \tparam Ch2 Type of character to reserve space for.
  //! \param count Number of characters to reserve space for.
  template<typename Ch2>
  void Reserve(size_t count) {
    stream_.Reserve(count * sizeof(Ch2) * 4 / 3);
  }
  //! \brief Begin a stream.
  //! \returns Character inserted.
  Ch* PutBegin() { return stream_.PutBegin(); }
  //! \brief Insert a byte.
  //! \param ch Byte.
  void PutByte(unsigned char ch) {
    RAPIDJSON_ASSERT(buffer_pos_ < 3);
    buffer_empty_[buffer_pos_] = false;
    buffer_[buffer_pos_++] = ch;
    if (buffer_pos_ == 3)
      WriteNext();
  }
  //! \brief Insert a character.
  //! \tparam Ch2 Character type.
  //! \param ch Character.
  template<typename Ch2>
  void Put(Ch2 ch) {
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&ch);
    for (size_t i = 0; i < sizeof(Ch2); i++) {
      PutByte(bytes[i]);
    }
  }
  //! \brief Flush the stream.
  void Flush() { stream_.Flush(); }
  //! \brief Finalize an partial byte sets and insert a character.
  //! \tparam Ch2 Character type.
  //! \param ch Character to insert at the end.
  //! \return Stream size.
  template<typename Ch2>
  size_t PutEnd(Ch2* ch) {
    if (buffer_pos_ > 0)
      WriteNext();
    return stream_.PutEnd(ch);
  }

  //! \brief Write the bytes in the buffer to the stream as base64 encoded characters.
  void WriteNext() {
    if (buffer_pos_ == 0) return;
    // Encode
    unsigned char encoded[4] = {'\0', '\0', '\0', '\0'};
    unsigned char *in = &(buffer_[0]);
    unsigned char *pos = &(encoded[0]);
    if ((!buffer_empty_[0]) && (!buffer_empty_[1]) && (!buffer_empty_[2])) {
      *pos++ = base64_table[in[0] >> 2];
      *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
      *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
      *pos++ = base64_table[in[2] & 0x3f];
    } else {
      *pos++ = base64_table[in[0] >> 2];
      if (buffer_empty_[1]) {
	*pos++ = base64_table[(in[0] & 0x03) << 4];
	*pos++ = base64_table_last;
      } else {
	*pos++ = base64_table[((in[0] & 0x03) << 4) |
			      (in[1] >> 4)];
	*pos++ = base64_table[(in[1] & 0x0f) << 2];
      }
      *pos++ = base64_table_last;
    }
    // Output
    for (size_t i = 0; i < 4; i++) {
      if (encoded[i] == '\0')
	break;
      stream_.Put((Ch)(encoded[i]));
    }
    // Reset
    buffer_pos_ = 0;
    buffer_[0] = '\0';
    buffer_[1] = '\0';
    buffer_[2] = '\0';
    buffer_empty_[0] = true;
    buffer_empty_[1] = true;
    buffer_empty_[2] = true;
  }
  //! \brief Dummy yggdrasil method.
  template <typename SchemaValueType>
  bool YggdrasilString() { return false; }

  //! \brief Wrapper for stream
  UTFType GetType() const { return stream_.GetType(); }
  
private:
  StreamType &stream_;
  unsigned char buffer_[3];
  unsigned char dtable_[256];
  bool buffer_empty_[3];
  size_t pos_;
  size_t buffer_pos_;
  
};

RAPIDJSON_NAMESPACE_END

#endif // BASE64_H_
