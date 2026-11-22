#ifndef COD_LANG_TEXT_LITERAL_H
#define COD_LANG_TEXT_LITERAL_H

#include "../include/cod_utf8.h"

/* Decode the interior of a quoted runtime literal. Escapes never expand
   past their source width, so the original span bounds the output. */
static inline const char *cod_decode_text_literal(const uint8_t *source, size_t length,
                                                  uint8_t *output, size_t *written) {
  size_t count = 0;
  for (size_t at = 0; at < length;) {
    const uint8_t byte = source[at++];
    if (byte == '\r' || byte == '\n') return "newline in text literal";
    if (byte != '\\') {
      output[count++] = byte;
      continue;
    }
    if (at == length) return "unknown text literal escape";
    const uint8_t escaped = source[at++];
    switch (escaped) {
    case '"': case '\\': output[count++] = escaped; break;
    case 'n': output[count++] = '\n'; break;
    case 'r': output[count++] = '\r'; break;
    case 't': output[count++] = '\t'; break;
    case '0': output[count++] = 0; break;
    case 'u': {
      if (at == length || source[at++] != '{') return "expected '{' in Unicode scalar escape";
      size_t close = at;
      while (close < length && source[close] != '}') ++close;
      if (close == length) return "unterminated Unicode scalar escape";
      if (close == at) return "invalid Unicode scalar escape";
      uint32_t value = 0;
      for (; at < close; ++at) {
        const uint8_t digit = source[at];
        uint32_t part;
        if (digit >= '0' && digit <= '9') part = digit - '0';
        else if (digit >= 'a' && digit <= 'f') part = digit - 'a' + 10u;
        else if (digit >= 'A' && digit <= 'F') part = digit - 'A' + 10u;
        else return "invalid Unicode scalar escape";
        value = value * 16u + part;
        if (value > 0x10ffffu) return "invalid Unicode scalar escape";
      }
      const size_t width = cod_utf8_encode(value, output + count);
      if (!width) return "invalid Unicode scalar escape";
      count += width;
      ++at;
      break;
    }
    default: return "unknown text literal escape";
    }
  }
  if (!cod_utf8_valid(output, count)) return "invalid UTF-8 in text literal";
  *written = count;
  return NULL;
}

#endif
