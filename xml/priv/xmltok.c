/*
Copyright (c) 1998, 1999 Thai Open Source Software Center Ltd
See the file COPYING for copying permission.
*/


#include "xmltok.h"
#include "nametab.h"


#define IGNORE_SECTION_TOK_VTABLE , PREFIX(ignoreSectionTok)


#define VTABLE1 \
  { PREFIX(prologTok), PREFIX(contentTok), \
    PREFIX(cdataSectionTok) IGNORE_SECTION_TOK_VTABLE }, \
  { PREFIX(attributeValueTok), PREFIX(entityValueTok) }, \
  PREFIX(sameName), \
  PREFIX(nameMatchesAscii), \
  PREFIX(nameLength), \
  PREFIX(skipS), \
  PREFIX(getAtts), \
  PREFIX(charRefNumber), \
  PREFIX(predefinedEntityName), \
  PREFIX(updatePosition), \
  PREFIX(isPublicId)

#define VTABLE VTABLE1, PREFIX(toUtf8), PREFIX(toUtf16)

#define UCS2_GET_NAMING(pages, hi, lo) \
   (namingBitmap[(pages[hi] << 3) + ((lo) >> 5)] & (1 << ((lo) & 0x1F)))

/* A 2 byte UTF-8 representation splits the characters 11 bits
between the bottom 5 and 6 bits of the bytes.
We need 8 bits to index into pages, 3 bits to add to that index and
5 bits to generate the mask. */
#define UTF8_GET_NAMING2(pages, byte) \
    (namingBitmap[((pages)[(((byte)[0]) >> 2) & 7] << 3) \
                      + ((((byte)[0]) & 3) << 1) \
                      + ((((byte)[1]) >> 5) & 1)] \
         & (1 << (((byte)[1]) & 0x1F)))

/* A 3 byte UTF-8 representation splits the characters 16 bits
between the bottom 4, 6 and 6 bits of the bytes.
We need 8 bits to index into pages, 3 bits to add to that index and
5 bits to generate the mask. */
#define UTF8_GET_NAMING3(pages, byte) \
  (namingBitmap[((pages)[((((byte)[0]) & 0xF) << 4) \
                             + ((((byte)[1]) >> 2) & 0xF)] \
		       << 3) \
                      + ((((byte)[1]) & 3) << 1) \
                      + ((((byte)[2]) >> 5) & 1)] \
         & (1 << (((byte)[2]) & 0x1F)))

#define UTF8_GET_NAMING(pages, p, n) \
  ((n) == 2 \
  ? UTF8_GET_NAMING2(pages, (const unsigned char *)(p)) \
  : ((n) == 3 \
     ? UTF8_GET_NAMING3(pages, (const unsigned char *)(p)) \
     : 0))

#define UTF8_INVALID3(p) \
  ((*p) == 0xED \
  ? (((p)[1] & 0x20) != 0) \
  : ((*p) == 0xEF \
     ? ((p)[1] == 0xBF && ((p)[2] == 0xBF || (p)[2] == 0xBE)) \
     : 0))

#define UTF8_INVALID4(p) ((*p) == 0xF4 && ((p)[1] & 0x30) != 0)

static
int isNever(const ENCODING *enc, const char *p)
{
  return 0;
}

static
int utf8_isName2(const ENCODING *enc, const char *p)
{
  return UTF8_GET_NAMING2(namePages, (const unsigned char *)p);
}

static
int utf8_isName3(const ENCODING *enc, const char *p)
{
  return UTF8_GET_NAMING3(namePages, (const unsigned char *)p);
}

#define utf8_isName4 isNever

static
int utf8_isNmstrt2(const ENCODING *enc, const char *p)
{
  return UTF8_GET_NAMING2(nmstrtPages, (const unsigned char *)p);
}

static
int utf8_isNmstrt3(const ENCODING *enc, const char *p)
{
  return UTF8_GET_NAMING3(nmstrtPages, (const unsigned char *)p);
}

#define utf8_isNmstrt4 isNever

#define utf8_isInvalid2 isNever

static
int utf8_isInvalid3(const ENCODING *enc, const char *p)
{
  return UTF8_INVALID3((const unsigned char *)p);
}

static
int utf8_isInvalid4(const ENCODING *enc, const char *p)
{
  return UTF8_INVALID4((const unsigned char *)p);
}

struct normal_encoding {
  ENCODING enc;
  unsigned char type[256];
#ifdef XML_MIN_SIZE
  int (*byteType)(const ENCODING *, const char *);
  int (*isNameMin)(const ENCODING *, const char *);
  int (*isNmstrtMin)(const ENCODING *, const char *);
  int (*byteToAscii)(const ENCODING *, const char *);
  int (*charMatches)(const ENCODING *, const char *, int);
#endif /* XML_MIN_SIZE */
  int (*isName2)(const ENCODING *, const char *);
  int (*isName3)(const ENCODING *, const char *);
  int (*isName4)(const ENCODING *, const char *);
  int (*isNmstrt2)(const ENCODING *, const char *);
  int (*isNmstrt3)(const ENCODING *, const char *);
  int (*isNmstrt4)(const ENCODING *, const char *);
  int (*isInvalid2)(const ENCODING *, const char *);
  int (*isInvalid3)(const ENCODING *, const char *);
  int (*isInvalid4)(const ENCODING *, const char *);
};

#ifdef XML_MIN_SIZE

#define STANDARD_VTABLE(E) \
 E ## byteType, \
 E ## isNameMin, \
 E ## isNmstrtMin, \
 E ## byteToAscii, \
 E ## charMatches,

#else

#define STANDARD_VTABLE(E) /* as nothing */

#endif

#define NORMAL_VTABLE(E) \
 E ## isName2, \
 E ## isName3, \
 E ## isName4, \
 E ## isNmstrt2, \
 E ## isNmstrt3, \
 E ## isNmstrt4, \
 E ## isInvalid2, \
 E ## isInvalid3, \
 E ## isInvalid4

static int checkCharRefNumber(int);

#include "xmltok_impl.h"
#include "ascii.h"

#ifdef XML_MIN_SIZE
#define sb_isNameMin isNever
#define sb_isNmstrtMin isNever
#endif

#ifdef XML_MIN_SIZE
#define MINBPC(enc) ((enc)->minBytesPerChar)
#else
/* minimum bytes per character */
#define MINBPC(enc) 1
#endif

#define SB_BYTE_TYPE(enc, p) \
  (((struct normal_encoding *)(enc))->type[(unsigned char)*(p)])

#ifdef XML_MIN_SIZE
static
int sb_byteType(const ENCODING *enc, const char *p)
{
  return SB_BYTE_TYPE(enc, p);
}
#define BYTE_TYPE(enc, p) \
 (((const struct normal_encoding *)(enc))->byteType(enc, p))
#else
#define BYTE_TYPE(enc, p) SB_BYTE_TYPE(enc, p)
#endif

#ifdef XML_MIN_SIZE
#define BYTE_TO_ASCII(enc, p) \
 (((const struct normal_encoding *)(enc))->byteToAscii(enc, p))
static
int sb_byteToAscii(const ENCODING *enc, const char *p)
{
  return *p;
}
#else
#define BYTE_TO_ASCII(enc, p) (*(p))
#endif

#define IS_NAME_CHAR(enc, p, n) \
 (((const struct normal_encoding *)(enc))->isName ## n(enc, p))
#define IS_NMSTRT_CHAR(enc, p, n) \
 (((const struct normal_encoding *)(enc))->isNmstrt ## n(enc, p))
#define IS_INVALID_CHAR(enc, p, n) \
 (((const struct normal_encoding *)(enc))->isInvalid ## n(enc, p))

#ifdef XML_MIN_SIZE
#define IS_NAME_CHAR_MINBPC(enc, p) \
 (((const struct normal_encoding *)(enc))->isNameMin(enc, p))
#define IS_NMSTRT_CHAR_MINBPC(enc, p) \
 (((const struct normal_encoding *)(enc))->isNmstrtMin(enc, p))
#else
#define IS_NAME_CHAR_MINBPC(enc, p) (0)
#define IS_NMSTRT_CHAR_MINBPC(enc, p) (0)
#endif

#ifdef XML_MIN_SIZE
#define CHAR_MATCHES(enc, p, c) \
 (((const struct normal_encoding *)(enc))->charMatches(enc, p, c))
static
int sb_charMatches(const ENCODING *enc, const char *p, int c)
{
  return *p == c;
}
#else
/* c is an ASCII character */
#define CHAR_MATCHES(enc, p, c) (*(p) == c)
#endif

#define PREFIX(ident) normal_ ## ident


/**************** xmltok impl add *************************************/

#ifndef IS_INVALID_CHAR
#define IS_INVALID_CHAR(enc, ptr, n) (0)
#endif



#define INVALID_LEAD_CASE(n, ptr, nextTokPtr) \
    case BT_LEAD ## n: \
      if (end - ptr < n) \
	return XML_TOK_PARTIAL_CHAR; \
      if (IS_INVALID_CHAR(enc, ptr, n)) { \
        *(nextTokPtr) = (ptr); \
        return XML_TOK_INVALID; \
      } \
      ptr += n; \
      break;

#define INVALID_CASES(ptr, nextTokPtr) \
  INVALID_LEAD_CASE(2, ptr, nextTokPtr) \
  INVALID_LEAD_CASE(3, ptr, nextTokPtr) \
  INVALID_LEAD_CASE(4, ptr, nextTokPtr) \
  case BT_NONXML: \
  case BT_MALFORM: \
  case BT_TRAIL: \
    *(nextTokPtr) = (ptr); \
    return XML_TOK_INVALID;

#define CHECK_NAME_CASE(n, enc, ptr, end, nextTokPtr) \
   case BT_LEAD ## n: \
     if (end - ptr < n) \
       return XML_TOK_PARTIAL_CHAR; \
     if (!IS_NAME_CHAR(enc, ptr, n)) { \
       *nextTokPtr = ptr; \
       return XML_TOK_INVALID; \
     } \
     ptr += n; \
     break;

#define CHECK_NAME_CASES(enc, ptr, end, nextTokPtr) \
  case BT_NONASCII: \
    if (!IS_NAME_CHAR_MINBPC(enc, ptr)) { \
      *nextTokPtr = ptr; \
      return XML_TOK_INVALID; \
    } \
  case BT_NMSTRT: \
  case BT_HEX: \
  case BT_DIGIT: \
  case BT_NAME: \
  case BT_MINUS: \
    ptr += MINBPC(enc); \
    break; \
  CHECK_NAME_CASE(2, enc, ptr, end, nextTokPtr) \
  CHECK_NAME_CASE(3, enc, ptr, end, nextTokPtr) \
  CHECK_NAME_CASE(4, enc, ptr, end, nextTokPtr)

#define CHECK_NMSTRT_CASE(n, enc, ptr, end, nextTokPtr) \
   case BT_LEAD ## n: \
     if (end - ptr < n) \
       return XML_TOK_PARTIAL_CHAR; \
     if (!IS_NMSTRT_CHAR(enc, ptr, n)) { \
       *nextTokPtr = ptr; \
       return XML_TOK_INVALID; \
     } \
     ptr += n; \
     break;

#define CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr) \
  case BT_NONASCII: \
    if (!IS_NMSTRT_CHAR_MINBPC(enc, ptr)) { \
      *nextTokPtr = ptr; \
      return XML_TOK_INVALID; \
    } \
  case BT_NMSTRT: \
  case BT_HEX: \
    ptr += MINBPC(enc); \
    break; \
  CHECK_NMSTRT_CASE(2, enc, ptr, end, nextTokPtr) \
  CHECK_NMSTRT_CASE(3, enc, ptr, end, nextTokPtr) \
  CHECK_NMSTRT_CASE(4, enc, ptr, end, nextTokPtr)

#ifndef PREFIX
#define PREFIX(ident) ident
#endif

/* ptr points to character following "<!-" */

static
int PREFIX(scanComment)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr != end) {
    if (!CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    ptr += MINBPC(enc);
    while (ptr != end) {
      switch (BYTE_TYPE(enc, ptr)) {
      INVALID_CASES(ptr, nextTokPtr)
      case BT_MINUS:
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
	  if ((ptr += MINBPC(enc)) == end)
	    return XML_TOK_PARTIAL;
	  if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  *nextTokPtr = ptr + MINBPC(enc);
	  return XML_TOK_COMMENT;
	}
	break;
      default:
	ptr += MINBPC(enc);
	break;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "<!" */

static
int PREFIX(scanDecl)(const ENCODING *enc, const char *ptr, const char *end,
		     const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_MINUS:
    return PREFIX(scanComment)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_LSQB:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_COND_SECT_OPEN;
  case BT_NMSTRT:
  case BT_HEX:
    ptr += MINBPC(enc);
    break;
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_PERCNT:
      if (ptr + MINBPC(enc) == end)
	return XML_TOK_PARTIAL;
      /* don't allow <!ENTITY% foo "whatever"> */
      switch (BYTE_TYPE(enc, ptr + MINBPC(enc))) {
      case BT_S: case BT_CR: case BT_LF: case BT_PERCNT:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      /* fall through */
    case BT_S: case BT_CR: case BT_LF:
      *nextTokPtr = ptr;
      return XML_TOK_DECL_OPEN;
    case BT_NMSTRT:
    case BT_HEX:
      ptr += MINBPC(enc);
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(checkPiTarget)(const ENCODING *enc, const char *ptr, const char *end, int *tokPtr)
{
  int upper = 0;
  *tokPtr = XML_TOK_PI;
  if (end - ptr != MINBPC(enc)*3)
    return 1;
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_x:
    break;
  case ASCII_X:
    upper = 1;
    break;
  default:
    return 1;
  }
  ptr += MINBPC(enc);
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_m:
    break;
  case ASCII_M:
    upper = 1;
    break;
  default:
    return 1;
  }
  ptr += MINBPC(enc);
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_l:
    break;
  case ASCII_L:
    upper = 1;
    break;
  default:
    return 1;
  }
  if (upper)
    return 0;
  *tokPtr = XML_TOK_XML_DECL;
  return 1;
}

/* ptr points to character following "<?" */

static
int PREFIX(scanPi)(const ENCODING *enc, const char *ptr, const char *end,
		   const char **nextTokPtr)
{
  int tok;
  const char *target = ptr;
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_S: case BT_CR: case BT_LF:
      if (!PREFIX(checkPiTarget)(enc, target, ptr, &tok)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      ptr += MINBPC(enc);
      while (ptr != end) {
        switch (BYTE_TYPE(enc, ptr)) {
        INVALID_CASES(ptr, nextTokPtr)
	case BT_QUEST:
	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	    *nextTokPtr = ptr + MINBPC(enc);
	    return tok;
	  }
	  break;
	default:
	  ptr += MINBPC(enc);
	  break;
	}
      }
      return XML_TOK_PARTIAL;
    case BT_QUEST:
      if (!PREFIX(checkPiTarget)(enc, target, ptr, &tok)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	*nextTokPtr = ptr + MINBPC(enc);
	return tok;
      }
      /* fall through */
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}


static
int PREFIX(scanCdataSection)(const ENCODING *enc, const char *ptr, const char *end,
			     const char **nextTokPtr)
{
  static const char CDATA_LSQB[] = { ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, ASCII_LSQB };
  int i;
  /* CDATA[ */
  if (end - ptr < 6 * MINBPC(enc))
    return XML_TOK_PARTIAL;
  for (i = 0; i < 6; i++, ptr += MINBPC(enc)) {
    if (!CHAR_MATCHES(enc, ptr, CDATA_LSQB[i])) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_CDATA_SECT_OPEN;
}

static
int PREFIX(cdataSectionTok)(const ENCODING *enc, const char *ptr, const char *end,
			    const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (!CHAR_MATCHES(enc, ptr, ASCII_RSQB))
      break;
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
      ptr -= MINBPC(enc);
      break;
    }
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_CDATA_SECT_CLOSE;
  case BT_CR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (BYTE_TYPE(enc, ptr) == BT_LF)
      ptr += MINBPC(enc);
    *nextTokPtr = ptr;
    return XML_TOK_DATA_NEWLINE;
  case BT_LF:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DATA_NEWLINE;
  INVALID_CASES(ptr, nextTokPtr)
  default:
    ptr += MINBPC(enc);
    break;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (end - ptr < n || IS_INVALID_CHAR(enc, ptr, n)) { \
	*nextTokPtr = ptr; \
	return XML_TOK_DATA_CHARS; \
      } \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONXML:
    case BT_MALFORM:
    case BT_TRAIL:
    case BT_CR:
    case BT_LF:
    case BT_RSQB:
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

/* ptr points to character following "</" */

static
int PREFIX(scanEndTag)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_S: case BT_CR: case BT_LF:
      for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
	switch (BYTE_TYPE(enc, ptr)) {
	case BT_S: case BT_CR: case BT_LF:
	  break;
	case BT_GT:
	  *nextTokPtr = ptr + MINBPC(enc);
          return XML_TOK_END_TAG;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
      }
      return XML_TOK_PARTIAL;

    case BT_COLON:
      /* no need to check qname syntax here, since end-tag must match exactly */
      ptr += MINBPC(enc);
      break;

    case BT_GT:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_END_TAG;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&#X" */

static
int PREFIX(scanHexCharRef)(const ENCODING *enc, const char *ptr, const char *end,
			   const char **nextTokPtr)
{
  if (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
    case BT_HEX:
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_DIGIT:
      case BT_HEX:
	break;
      case BT_SEMI:
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_CHAR_REF;
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&#" */

static
int PREFIX(scanCharRef)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr != end) {
    if (CHAR_MATCHES(enc, ptr, ASCII_x))
      return PREFIX(scanHexCharRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_DIGIT:
	break;
      case BT_SEMI:
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_CHAR_REF;
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&" */

static
int PREFIX(scanRef)(const ENCODING *enc, const char *ptr, const char *end,
		    const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_NUM:
    return PREFIX(scanCharRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_SEMI:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_ENTITY_REF;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following first character of attribute name */

static
int PREFIX(scanAtts)(const ENCODING *enc, const char *ptr, const char *end,
		     const char **nextTokPtr)
{

  int hadColon = 0;

  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)

    case BT_COLON:
      if (hadColon) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      hadColon = 1;
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      break;

    case BT_S: case BT_CR: case BT_LF:
      for (;;) {
	int t;

	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	t = BYTE_TYPE(enc, ptr);
	if (t == BT_EQUALS)
	  break;
	switch (t) {
	case BT_S:
	case BT_LF:
	case BT_CR:
	  break;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
      }
    /* fall through */
    case BT_EQUALS:
      {
	int open;

	hadColon = 0;

	for (;;) {

	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  open = BYTE_TYPE(enc, ptr);
	  if (open == BT_QUOT || open == BT_APOS)
	    break;
	  switch (open) {
	  case BT_S:
	  case BT_LF:
	  case BT_CR:
	    break;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	}
	ptr += MINBPC(enc);
	/* in attribute value */
	for (;;) {
	  int t;
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  t = BYTE_TYPE(enc, ptr);
	  if (t == open)
	    break;
	  switch (t) {
	  INVALID_CASES(ptr, nextTokPtr)
	  case BT_AMP:
	    {
	      int tok = PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, &ptr);
	      if (tok <= 0) {
		if (tok == XML_TOK_INVALID)
		  *nextTokPtr = ptr;
		return tok;
	      }
	      break;
	    }
	  case BT_LT:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  default:
	    ptr += MINBPC(enc);
	    break;
	  }
	}
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	switch (BYTE_TYPE(enc, ptr)) {
	case BT_S:
	case BT_CR:
	case BT_LF:
	  break;
	case BT_SOL:
	  goto sol;
	case BT_GT:
	  goto gt;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
	/* ptr points to closing quote */
	for (;;) {
	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  switch (BYTE_TYPE(enc, ptr)) {
	  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
	  case BT_S: case BT_CR: case BT_LF:
	    continue;
	  case BT_GT:
          gt:
	    *nextTokPtr = ptr + MINBPC(enc);
	    return XML_TOK_START_TAG_WITH_ATTS;
	  case BT_SOL:
          sol:
	    ptr += MINBPC(enc);
	    if (ptr == end)
	      return XML_TOK_PARTIAL;
	    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	      *nextTokPtr = ptr;
	      return XML_TOK_INVALID;
	    }
	    *nextTokPtr = ptr + MINBPC(enc);
	    return XML_TOK_EMPTY_ELEMENT_WITH_ATTS;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  break;
	}
	break;
      }
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "<" */

static
int PREFIX(scanLt)(const ENCODING *enc, const char *ptr, const char *end,
		   const char **nextTokPtr)
{

  int hadColon;

  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_EXCL:
    if ((ptr += MINBPC(enc)) == end)
      return XML_TOK_PARTIAL;
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_MINUS:
      return PREFIX(scanComment)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    case BT_LSQB:
      return PREFIX(scanCdataSection)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  case BT_QUEST:
    return PREFIX(scanPi)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_SOL:
    return PREFIX(scanEndTag)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }

  hadColon = 0;

  /* we have a start-tag */
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)

    case BT_COLON:
      if (hadColon) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      hadColon = 1;
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
      default:
        *nextTokPtr = ptr;
        return XML_TOK_INVALID;
      }
      break;

    case BT_S: case BT_CR: case BT_LF:
      {
        ptr += MINBPC(enc);
	while (ptr != end) {
	  switch (BYTE_TYPE(enc, ptr)) {
	  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
	  case BT_GT:
	    goto gt;
	  case BT_SOL:
	    goto sol;
	  case BT_S: case BT_CR: case BT_LF:
	    ptr += MINBPC(enc);
	    continue;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  return PREFIX(scanAtts)(enc, ptr, end, nextTokPtr);
	}
	return XML_TOK_PARTIAL;
      }
    case BT_GT:
    gt:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_START_TAG_NO_ATTS;
    case BT_SOL:
    sol:
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_EMPTY_ELEMENT_NO_ATTS;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(contentTok)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_LT:
    return PREFIX(scanLt)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_AMP:
    return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_CR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_CR;
    if (BYTE_TYPE(enc, ptr) == BT_LF)
      ptr += MINBPC(enc);
    *nextTokPtr = ptr;
    return XML_TOK_DATA_NEWLINE;
  case BT_LF:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DATA_NEWLINE;
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_RSQB;
    if (!CHAR_MATCHES(enc, ptr, ASCII_RSQB))
      break;
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_RSQB;
    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
      ptr -= MINBPC(enc);
      break;
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  INVALID_CASES(ptr, nextTokPtr)
  default:
    ptr += MINBPC(enc);
    break;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (end - ptr < n || IS_INVALID_CHAR(enc, ptr, n)) { \
	*nextTokPtr = ptr; \
	return XML_TOK_DATA_CHARS; \
      } \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_RSQB:
      if (ptr + MINBPC(enc) != end) {
	 if (!CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_RSQB)) {
	   ptr += MINBPC(enc);
	   break;
	 }
	 if (ptr + 2*MINBPC(enc) != end) {
	   if (!CHAR_MATCHES(enc, ptr + 2*MINBPC(enc), ASCII_GT)) {
	     ptr += MINBPC(enc);
	     break;
	   }
	   *nextTokPtr = ptr + 2*MINBPC(enc);
	   return XML_TOK_INVALID;
	 }
      }
      /* fall through */
    case BT_AMP:
    case BT_LT:
    case BT_NONXML:
    case BT_MALFORM:
    case BT_TRAIL:
    case BT_CR:
    case BT_LF:
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

/* ptr points to character following "%" */

static
int PREFIX(scanPercent)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_S: case BT_LF: case BT_CR: case BT_PERCNT:
    *nextTokPtr = ptr;
    return XML_TOK_PERCENT;
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_SEMI:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_PARAM_ENTITY_REF;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(scanPoundName)(const ENCODING *enc, const char *ptr, const char *end,
			  const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_CR: case BT_LF: case BT_S:
    case BT_RPAR: case BT_GT: case BT_PERCNT: case BT_VERBAR:
      *nextTokPtr = ptr;
      return XML_TOK_POUND_NAME;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return -XML_TOK_POUND_NAME;
}

static
int PREFIX(scanLit)(int open, const ENCODING *enc,
		    const char *ptr, const char *end,
		    const char **nextTokPtr)
{
  while (ptr != end) {
    int t = BYTE_TYPE(enc, ptr);
    switch (t) {
    INVALID_CASES(ptr, nextTokPtr)
    case BT_QUOT:
    case BT_APOS:
      ptr += MINBPC(enc);
      if (t != open)
	break;
      if (ptr == end)
	return -XML_TOK_LITERAL;
      *nextTokPtr = ptr;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_S: case BT_CR: case BT_LF:
      case BT_GT: case BT_PERCNT: case BT_LSQB:
	return XML_TOK_LITERAL;
      default:
	return XML_TOK_INVALID;
      }
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(prologTok)(const ENCODING *enc, const char *ptr, const char *end,
		      const char **nextTokPtr)
{
  int tok;
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_QUOT:
    return PREFIX(scanLit)(BT_QUOT, enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_APOS:
    return PREFIX(scanLit)(BT_APOS, enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_LT:
    {
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_EXCL:
	return PREFIX(scanDecl)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      case BT_QUEST:
	return PREFIX(scanPi)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      case BT_NMSTRT:
      case BT_HEX:
      case BT_NONASCII:
      case BT_LEAD2:
      case BT_LEAD3:
      case BT_LEAD4:
	*nextTokPtr = ptr - MINBPC(enc);
	return XML_TOK_INSTANCE_START;
      }
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  case BT_CR:
    if (ptr + MINBPC(enc) == end)
      return -XML_TOK_PROLOG_S;
    /* fall through */
  case BT_S: case BT_LF:
    for (;;) {
      ptr += MINBPC(enc);
      if (ptr == end)
	break;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_S: case BT_LF:
	break;
      case BT_CR:
	/* don't split CR/LF pair */
	if (ptr + MINBPC(enc) != end)
	  break;
	/* fall through */
      default:
	*nextTokPtr = ptr;
	return XML_TOK_PROLOG_S;
      }
    }
    *nextTokPtr = ptr;
    return XML_TOK_PROLOG_S;
  case BT_PERCNT:
    return PREFIX(scanPercent)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_COMMA:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_COMMA;
  case BT_LSQB:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OPEN_BRACKET;
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return -XML_TOK_CLOSE_BRACKET;
    if (CHAR_MATCHES(enc, ptr, ASCII_RSQB)) {
      if (ptr + MINBPC(enc) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_GT)) {
	*nextTokPtr = ptr + 2*MINBPC(enc);
	return XML_TOK_COND_SECT_CLOSE;
      }
    }
    *nextTokPtr = ptr;
    return XML_TOK_CLOSE_BRACKET;
  case BT_LPAR:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OPEN_PAREN;
  case BT_RPAR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return -XML_TOK_CLOSE_PAREN;
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_AST:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_ASTERISK;
    case BT_QUEST:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_QUESTION;
    case BT_PLUS:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_PLUS;
    case BT_CR: case BT_LF: case BT_S:
    case BT_GT: case BT_COMMA: case BT_VERBAR:
    case BT_RPAR:
      *nextTokPtr = ptr;
      return XML_TOK_CLOSE_PAREN;
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  case BT_VERBAR:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OR;
  case BT_GT:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DECL_CLOSE;
  case BT_NUM:
    return PREFIX(scanPoundName)(enc, ptr + MINBPC(enc), end, nextTokPtr);
#define LEAD_CASE(n) \
  case BT_LEAD ## n: \
    if (end - ptr < n) \
      return XML_TOK_PARTIAL_CHAR; \
    if (IS_NMSTRT_CHAR(enc, ptr, n)) { \
      ptr += n; \
      tok = XML_TOK_NAME; \
      break; \
    } \
    if (IS_NAME_CHAR(enc, ptr, n)) { \
      ptr += n; \
      tok = XML_TOK_NMTOKEN; \
      break; \
    } \
    *nextTokPtr = ptr; \
    return XML_TOK_INVALID;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
  case BT_NMSTRT:
  case BT_HEX:
    tok = XML_TOK_NAME;
    ptr += MINBPC(enc);
    break;
  case BT_DIGIT:
  case BT_NAME:
  case BT_MINUS:

  case BT_COLON:

    tok = XML_TOK_NMTOKEN;
    ptr += MINBPC(enc);
    break;
  case BT_NONASCII:
    if (IS_NMSTRT_CHAR_MINBPC(enc, ptr)) {
      ptr += MINBPC(enc);
      tok = XML_TOK_NAME;
      break;
    }
    if (IS_NAME_CHAR_MINBPC(enc, ptr)) {
      ptr += MINBPC(enc);
      tok = XML_TOK_NMTOKEN;
      break;
    }
    /* fall through */
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_GT: case BT_RPAR: case BT_COMMA:
    case BT_VERBAR: case BT_LSQB: case BT_PERCNT:
    case BT_S: case BT_CR: case BT_LF:
      *nextTokPtr = ptr;
      return tok;

    case BT_COLON:
      ptr += MINBPC(enc);
      switch (tok) {
      case XML_TOK_NAME:
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	tok = XML_TOK_PREFIXED_NAME;
	switch (BYTE_TYPE(enc, ptr)) {
	CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
	default:
	  tok = XML_TOK_NMTOKEN;
	  break;
	}
	break;
      case XML_TOK_PREFIXED_NAME:
	tok = XML_TOK_NMTOKEN;
	break;
      }
      break;

    case BT_PLUS:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_PLUS;
    case BT_AST:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_ASTERISK;
    case BT_QUEST:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_QUESTION;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return -tok;
}

static
int PREFIX(attributeValueTok)(const ENCODING *enc, const char *ptr, const char *end,
			      const char **nextTokPtr)
{
  const char *start;
  if (ptr == end)
    return XML_TOK_NONE;
  start = ptr;
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_AMP:
      if (ptr == start)
	return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_LT:
      /* this is for inside entity references */
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    case BT_LF:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_CR:
      if (ptr == start) {
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_TRAILING_CR;
	if (BYTE_TYPE(enc, ptr) == BT_LF)
	  ptr += MINBPC(enc);
	*nextTokPtr = ptr;
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_S:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_ATTRIBUTE_VALUE_S;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

static
int PREFIX(entityValueTok)(const ENCODING *enc, const char *ptr, const char *end,
			   const char **nextTokPtr)
{
  const char *start;
  if (ptr == end)
    return XML_TOK_NONE;
  start = ptr;
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_AMP:
      if (ptr == start)
	return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_PERCNT:
      if (ptr == start)
	return PREFIX(scanPercent)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_LF:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_CR:
      if (ptr == start) {
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_TRAILING_CR;
	if (BYTE_TYPE(enc, ptr) == BT_LF)
	  ptr += MINBPC(enc);
	*nextTokPtr = ptr;
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}



static
int PREFIX(ignoreSectionTok)(const ENCODING *enc, const char *ptr, const char *end,
			     const char **nextTokPtr)
{
  int level = 0;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      end = ptr + n;
    }
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    INVALID_CASES(ptr, nextTokPtr)
    case BT_LT:
      if ((ptr += MINBPC(enc)) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_EXCL)) {
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_LSQB)) {
	  ++level;
	  ptr += MINBPC(enc);
	}
      }
      break;
    case BT_RSQB:
      if ((ptr += MINBPC(enc)) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_RSQB)) {
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	  ptr += MINBPC(enc);
	  if (level == 0) {
	    *nextTokPtr = ptr;
	    return XML_TOK_IGNORE_SECT;
	  }
	  --level;
	}
      }
      break;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  return XML_TOK_PARTIAL;
}



static
int PREFIX(isPublicId)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **badPtr)
{
  ptr += MINBPC(enc);
  end -= MINBPC(enc);
  for (; ptr != end; ptr += MINBPC(enc)) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
    case BT_HEX:
    case BT_MINUS:
    case BT_APOS:
    case BT_LPAR:
    case BT_RPAR:
    case BT_PLUS:
    case BT_COMMA:
    case BT_SOL:
    case BT_EQUALS:
    case BT_QUEST:
    case BT_CR:
    case BT_LF:
    case BT_SEMI:
    case BT_EXCL:
    case BT_AST:
    case BT_PERCNT:
    case BT_NUM:

    case BT_COLON:

      break;
    case BT_S:
      if (CHAR_MATCHES(enc, ptr, ASCII_TAB)) {
	*badPtr = ptr;
	return 0;
      }
      break;
    case BT_NAME:
    case BT_NMSTRT:
      if (!(BYTE_TO_ASCII(enc, ptr) & ~0x7f))
	break;
    default:
      switch (BYTE_TO_ASCII(enc, ptr)) {
      case 0x24: /* $ */
      case 0x40: /* @ */
	break;
      default:
	*badPtr = ptr;
	return 0;
      }
      break;
    }
  }
  return 1;
}

/* This must only be called for a well-formed start-tag or empty element tag.
Returns the number of attributes.  Pointers to the first attsMax attributes
are stored in atts. */

static
int PREFIX(getAtts)(const ENCODING *enc, const char *ptr,
		    int attsMax, ATTRIBUTE *atts)
{
  enum { other, inName, inValue } state = inName;
  int nAtts = 0;
  int open = 0; /* defined when state == inValue;
		   initialization just to shut up compilers */

  for (ptr += MINBPC(enc);; ptr += MINBPC(enc)) {
    switch (BYTE_TYPE(enc, ptr)) {
#define START_NAME \
      if (state == other) { \
	if (nAtts < attsMax) { \
	  atts[nAtts].name = ptr; \
	  atts[nAtts].normalized = 1; \
	} \
	state = inName; \
      }
#define LEAD_CASE(n) \
    case BT_LEAD ## n: START_NAME ptr += (n - MINBPC(enc)); break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONASCII:
    case BT_NMSTRT:
    case BT_HEX:
      START_NAME
      break;
#undef START_NAME
    case BT_QUOT:
      if (state != inValue) {
	if (nAtts < attsMax)
	  atts[nAtts].valuePtr = ptr + MINBPC(enc);
        state = inValue;
        open = BT_QUOT;
      }
      else if (open == BT_QUOT) {
        state = other;
	if (nAtts < attsMax)
	  atts[nAtts].valueEnd = ptr;
	nAtts++;
      }
      break;
    case BT_APOS:
      if (state != inValue) {
	if (nAtts < attsMax)
	  atts[nAtts].valuePtr = ptr + MINBPC(enc);
        state = inValue;
        open = BT_APOS;
      }
      else if (open == BT_APOS) {
        state = other;
	if (nAtts < attsMax)
	  atts[nAtts].valueEnd = ptr;
	nAtts++;
      }
      break;
    case BT_AMP:
      if (nAtts < attsMax)
	atts[nAtts].normalized = 0;
      break;
    case BT_S:
      if (state == inName)
        state = other;
      else if (state == inValue
	       && nAtts < attsMax
	       && atts[nAtts].normalized
	       && (ptr == atts[nAtts].valuePtr
		   || BYTE_TO_ASCII(enc, ptr) != ASCII_SPACE
		   || BYTE_TO_ASCII(enc, ptr + MINBPC(enc)) == ASCII_SPACE
	           || BYTE_TYPE(enc, ptr + MINBPC(enc)) == open))
	atts[nAtts].normalized = 0;
      break;
    case BT_CR: case BT_LF:
      /* This case ensures that the first attribute name is counted
         Apart from that we could just change state on the quote. */
      if (state == inName)
        state = other;
      else if (state == inValue && nAtts < attsMax)
	atts[nAtts].normalized = 0;
      break;
    case BT_GT:
    case BT_SOL:
      if (state != inValue)
	return nAtts;
      break;
    default:
      break;
    }
  }
  /* not reached */
}

static
int PREFIX(charRefNumber)(const ENCODING *enc, const char *ptr)
{
  int result = 0;
  /* skip &# */
  ptr += 2*MINBPC(enc);
  if (CHAR_MATCHES(enc, ptr, ASCII_x)) {
    for (ptr += MINBPC(enc); !CHAR_MATCHES(enc, ptr, ASCII_SEMI); ptr += MINBPC(enc)) {
      int c = BYTE_TO_ASCII(enc, ptr);
      switch (c) {
      case ASCII_0: case ASCII_1: case ASCII_2: case ASCII_3: case ASCII_4:
      case ASCII_5: case ASCII_6: case ASCII_7: case ASCII_8: case ASCII_9:
	result <<= 4;
	result |= (c - ASCII_0);
	break;
      case ASCII_A: case ASCII_B: case ASCII_C: case ASCII_D: case ASCII_E: case ASCII_F:
	result <<= 4;
	result += 10 + (c - ASCII_A);
	break;
      case ASCII_a: case ASCII_b: case ASCII_c: case ASCII_d: case ASCII_e: case ASCII_f:
	result <<= 4;
	result += 10 + (c - ASCII_a);
	break;
      }
      if (result >= 0x110000)
	return -1;
    }
  }
  else {
    for (; !CHAR_MATCHES(enc, ptr, ASCII_SEMI); ptr += MINBPC(enc)) {
      int c = BYTE_TO_ASCII(enc, ptr);
      result *= 10;
      result += (c - ASCII_0);
      if (result >= 0x110000)
	return -1;
    }
  }
  return checkCharRefNumber(result);
}

static
int PREFIX(predefinedEntityName)(const ENCODING *enc, const char *ptr, const char *end)
{
  switch ((end - ptr)/MINBPC(enc)) {
  case 2:
    if (CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_t)) {
      switch (BYTE_TO_ASCII(enc, ptr)) {
      case ASCII_l:
	return ASCII_LT;
      case ASCII_g:
	return ASCII_GT;
      }
    }
    break;
  case 3:
    if (CHAR_MATCHES(enc, ptr, ASCII_a)) {
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_m)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_p))
	  return ASCII_AMP;
      }
    }
    break;
  case 4:
    switch (BYTE_TO_ASCII(enc, ptr)) {
    case ASCII_q:
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_u)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_o)) {
	  ptr += MINBPC(enc);
  	  if (CHAR_MATCHES(enc, ptr, ASCII_t))
	    return ASCII_QUOT;
	}
      }
      break;
    case ASCII_a:
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_p)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_o)) {
	  ptr += MINBPC(enc);
  	  if (CHAR_MATCHES(enc, ptr, ASCII_s))
	    return ASCII_APOS;
	}
      }
      break;
    }
  }
  return 0;
}

static
int PREFIX(sameName)(const ENCODING *enc, const char *ptr1, const char *ptr2)
{
  for (;;) {
    switch (BYTE_TYPE(enc, ptr1)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (*ptr1++ != *ptr2++) \
	return 0;
    LEAD_CASE(4) LEAD_CASE(3) LEAD_CASE(2)
#undef LEAD_CASE
      /* fall through */
      if (*ptr1++ != *ptr2++)
	return 0;
      break;
    case BT_NONASCII:
    case BT_NMSTRT:

    case BT_COLON:

    case BT_HEX:
    case BT_DIGIT:
    case BT_NAME:
    case BT_MINUS:
      if (*ptr2++ != *ptr1++)
	return 0;
      if (MINBPC(enc) > 1) {
	if (*ptr2++ != *ptr1++)
	  return 0;
	if (MINBPC(enc) > 2) {
	  if (*ptr2++ != *ptr1++)
	    return 0;
          if (MINBPC(enc) > 3) {
	    if (*ptr2++ != *ptr1++)
      	      return 0;
	  }
	}
      }
      break;
    default:
      if (MINBPC(enc) == 1 && *ptr1 == *ptr2)
	return 1;
      switch (BYTE_TYPE(enc, ptr2)) {
      case BT_LEAD2:
      case BT_LEAD3:
      case BT_LEAD4:
      case BT_NONASCII:
      case BT_NMSTRT:

      case BT_COLON:

      case BT_HEX:
      case BT_DIGIT:
      case BT_NAME:
      case BT_MINUS:
	return 0;
      default:
	return 1;
      }
    }
  }
  /* not reached */
}

static
int PREFIX(nameMatchesAscii)(const ENCODING *enc, const char *ptr1,
			     const char *end1, const char *ptr2)
{
  for (; *ptr2; ptr1 += MINBPC(enc), ptr2++) {
    if (ptr1 == end1)
      return 0;
    if (!CHAR_MATCHES(enc, ptr1, *ptr2))
      return 0;
  }
  return ptr1 == end1;
}

static
int PREFIX(nameLength)(const ENCODING *enc, const char *ptr)
{
  const char *start = ptr;
  for (;;) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONASCII:
    case BT_NMSTRT:

    case BT_COLON:

    case BT_HEX:
    case BT_DIGIT:
    case BT_NAME:
    case BT_MINUS:
      ptr += MINBPC(enc);
      break;
    default:
      return ptr - start;
    }
  }
}

static
const char *PREFIX(skipS)(const ENCODING *enc, const char *ptr)
{
  for (;;) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_LF:
    case BT_CR:
    case BT_S:
      ptr += MINBPC(enc);
      break;
    default:
      return ptr;
    }
  }
}

static
void PREFIX(updatePosition)(const ENCODING *enc,
			    const char *ptr,
			    const char *end,
			    POSITION *pos)
{
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_LF:
      pos->columnNumber = (unsigned)-1;
      pos->lineNumber++;
      ptr += MINBPC(enc);
      break;
    case BT_CR:
      pos->lineNumber++;
      ptr += MINBPC(enc);
      if (ptr != end && BYTE_TYPE(enc, ptr) == BT_LF)
	ptr += MINBPC(enc);
      pos->columnNumber = (unsigned)-1;
      break;
    default:
      ptr += MINBPC(enc);
      break;
    }
    pos->columnNumber++;
  }
}

#undef DO_LEAD_CASE
#undef MULTIBYTE_CASES
#undef INVALID_CASES
#undef CHECK_NAME_CASE
#undef CHECK_NAME_CASES
#undef CHECK_NMSTRT_CASE
#undef CHECK_NMSTRT_CASES


/**************** xmltok impl end *************************************/


#undef MINBPC
#undef BYTE_TYPE
#undef BYTE_TO_ASCII
#undef CHAR_MATCHES
#undef IS_NAME_CHAR
#undef IS_NAME_CHAR_MINBPC
#undef IS_NMSTRT_CHAR
#undef IS_NMSTRT_CHAR_MINBPC
#undef IS_INVALID_CHAR

enum {  /* UTF8_cvalN is value of masked first byte of N byte sequence */
  UTF8_cval1 = 0x00,
  UTF8_cval2 = 0xc0,
  UTF8_cval3 = 0xe0,
  UTF8_cval4 = 0xf0
};

static
void utf8_toUtf8(const ENCODING *enc,
		 const char **fromP, const char *fromLim,
		 char **toP, const char *toLim)
{
  char *to;
  const char *from;
  if (fromLim - *fromP > toLim - *toP) {
    /* Avoid copying partial characters. */
    for (fromLim = *fromP + (toLim - *toP); fromLim > *fromP; fromLim--)
      if (((unsigned char)fromLim[-1] & 0xc0) != 0x80)
	break;
  }
  for (to = *toP, from = *fromP; from != fromLim; from++, to++)
    *to = *from;
  *fromP = from;
  *toP = to;
}

static
void utf8_toUtf16(const ENCODING *enc,
		  const char **fromP, const char *fromLim,
		  unsigned short **toP, const unsigned short *toLim)
{
  unsigned short *to = *toP;
  const char *from = *fromP;
  while (from != fromLim && to != toLim) {
    switch (((struct normal_encoding *)enc)->type[(unsigned char)*from]) {
    case BT_LEAD2:
      *to++ = ((from[0] & 0x1f) << 6) | (from[1] & 0x3f);
      from += 2;
      break;
    case BT_LEAD3:
      *to++ = ((from[0] & 0xf) << 12) | ((from[1] & 0x3f) << 6) | (from[2] & 0x3f);
      from += 3;
      break;
    case BT_LEAD4:
      {
	unsigned long n;
	if (to + 1 == toLim)
	  break;
	n = ((from[0] & 0x7) << 18) | ((from[1] & 0x3f) << 12) | ((from[2] & 0x3f) << 6) | (from[3] & 0x3f);
	n -= 0x10000;
	to[0] = (unsigned short)((n >> 10) | 0xD800);
	to[1] = (unsigned short)((n & 0x3FF) | 0xDC00);
	to += 2;
	from += 4;
      }
      break;
    default:
      *to++ = *from++;
      break;
    }
  }
  *fromP = from;
  *toP = to;
}


static const struct normal_encoding utf8_encoding_ns = {
  { VTABLE1, utf8_toUtf8, utf8_toUtf16, 1, 1, 0 },
  {
#include "asciitab.h"
#include "utf8tab.h"
  },
  STANDARD_VTABLE(sb_) NORMAL_VTABLE(utf8_)
};


static const struct normal_encoding utf8_encoding = {
  { VTABLE1, utf8_toUtf8, utf8_toUtf16, 1, 1, 0 },
  {
#define BT_COLON BT_NMSTRT
#include "asciitab.h"
#undef BT_COLON
#include "utf8tab.h"
  },
  STANDARD_VTABLE(sb_) NORMAL_VTABLE(utf8_)
};



static const struct normal_encoding internal_utf8_encoding_ns = {
  { VTABLE1, utf8_toUtf8, utf8_toUtf16, 1, 1, 0 },
  {
#include "iasciitab.h"
#include "utf8tab.h"
  },
  STANDARD_VTABLE(sb_) NORMAL_VTABLE(utf8_)
};



static const struct normal_encoding internal_utf8_encoding = {
  { VTABLE1, utf8_toUtf8, utf8_toUtf16, 1, 1, 0 },
  {
#define BT_COLON BT_NMSTRT
#include "iasciitab.h"
#undef BT_COLON
#include "utf8tab.h"
  },
  STANDARD_VTABLE(sb_) NORMAL_VTABLE(utf8_)
};

static
void latin1_toUtf8(const ENCODING *enc,
		   const char **fromP, const char *fromLim,
		   char **toP, const char *toLim)
{
  for (;;) {
    unsigned char c;
    if (*fromP == fromLim)
      break;
    c = (unsigned char)**fromP;
    if (c & 0x80) {
      if (toLim - *toP < 2)
	break;
      *(*toP)++ = ((c >> 6) | UTF8_cval2);
      *(*toP)++ = ((c & 0x3f) | 0x80);
      (*fromP)++;
    }
    else {
      if (*toP == toLim)
	break;
      *(*toP)++ = *(*fromP)++;
    }
  }
}

static
void latin1_toUtf16(const ENCODING *enc,
		    const char **fromP, const char *fromLim,
		    unsigned short **toP, const unsigned short *toLim)
{
  while (*fromP != fromLim && *toP != toLim)
    *(*toP)++ = (unsigned char)*(*fromP)++;
}



static const struct normal_encoding latin1_encoding_ns = {
  { VTABLE1, latin1_toUtf8, latin1_toUtf16, 1, 0, 0 },
  {
#include "asciitab.h"
#include "latin1tab.h"
  },
  STANDARD_VTABLE(sb_)
};



static const struct normal_encoding latin1_encoding = {
  { VTABLE1, latin1_toUtf8, latin1_toUtf16, 1, 0, 0 },
  {
#define BT_COLON BT_NMSTRT
#include "asciitab.h"
#undef BT_COLON
#include "latin1tab.h"
  },
  STANDARD_VTABLE(sb_)
};

static
void ascii_toUtf8(const ENCODING *enc,
		  const char **fromP, const char *fromLim,
		  char **toP, const char *toLim)
{
  while (*fromP != fromLim && *toP != toLim)
    *(*toP)++ = *(*fromP)++;
}



static const struct normal_encoding ascii_encoding_ns = {
  { VTABLE1, ascii_toUtf8, latin1_toUtf16, 1, 1, 0 },
  {
#include "asciitab.h"
/* BT_NONXML == 0 */
  },
  STANDARD_VTABLE(sb_)
};



static const struct normal_encoding ascii_encoding = {
  { VTABLE1, ascii_toUtf8, latin1_toUtf16, 1, 1, 0 },
  {
#define BT_COLON BT_NMSTRT
#include "asciitab.h"
#undef BT_COLON
/* BT_NONXML == 0 */
  },
  STANDARD_VTABLE(sb_)
};

static int unicode_byte_type(char hi, char lo)
{
  switch ((unsigned char)hi) {
  case 0xD8: case 0xD9: case 0xDA: case 0xDB:
    return BT_LEAD4;
  case 0xDC: case 0xDD: case 0xDE: case 0xDF:
    return BT_TRAIL;
  case 0xFF:
    switch ((unsigned char)lo) {
    case 0xFF:
    case 0xFE:
      return BT_NONXML;
    }
    break;
  }
  return BT_NONASCII;
}

#define DEFINE_UTF16_TO_UTF8(E) \
static \
void E ## toUtf8(const ENCODING *enc, \
		 const char **fromP, const char *fromLim, \
		 char **toP, const char *toLim) \
{ \
  const char *from; \
  for (from = *fromP; from != fromLim; from += 2) { \
    int plane; \
    unsigned char lo2; \
    unsigned char lo = GET_LO(from); \
    unsigned char hi = GET_HI(from); \
    switch (hi) { \
    case 0: \
      if (lo < 0x80) { \
        if (*toP == toLim) { \
          *fromP = from; \
	  return; \
        } \
        *(*toP)++ = lo; \
        break; \
      } \
      /* fall through */ \
    case 0x1: case 0x2: case 0x3: \
    case 0x4: case 0x5: case 0x6: case 0x7: \
      if (toLim -  *toP < 2) { \
        *fromP = from; \
	return; \
      } \
      *(*toP)++ = ((lo >> 6) | (hi << 2) |  UTF8_cval2); \
      *(*toP)++ = ((lo & 0x3f) | 0x80); \
      break; \
    default: \
      if (toLim -  *toP < 3)  { \
        *fromP = from; \
	return; \
      } \
      /* 16 bits divided 4, 6, 6 amongst 3 bytes */ \
      *(*toP)++ = ((hi >> 4) | UTF8_cval3); \
      *(*toP)++ = (((hi & 0xf) << 2) | (lo >> 6) | 0x80); \
      *(*toP)++ = ((lo & 0x3f) | 0x80); \
      break; \
    case 0xD8: case 0xD9: case 0xDA: case 0xDB: \
      if (toLim -  *toP < 4) { \
	*fromP = from; \
	return; \
      } \
      plane = (((hi & 0x3) << 2) | ((lo >> 6) & 0x3)) + 1; \
      *(*toP)++ = ((plane >> 2) | UTF8_cval4); \
      *(*toP)++ = (((lo >> 2) & 0xF) | ((plane & 0x3) << 4) | 0x80); \
      from += 2; \
      lo2 = GET_LO(from); \
      *(*toP)++ = (((lo & 0x3) << 4) \
	           | ((GET_HI(from) & 0x3) << 2) \
		   | (lo2 >> 6) \
		   | 0x80); \
      *(*toP)++ = ((lo2 & 0x3f) | 0x80); \
      break; \
    } \
  } \
  *fromP = from; \
}

#define DEFINE_UTF16_TO_UTF16(E) \
static \
void E ## toUtf16(const ENCODING *enc, \
		  const char **fromP, const char *fromLim, \
		  unsigned short **toP, const unsigned short *toLim) \
{ \
  /* Avoid copying first half only of surrogate */ \
  if (fromLim - *fromP > ((toLim - *toP) << 1) \
      && (GET_HI(fromLim - 2) & 0xF8) == 0xD8) \
    fromLim -= 2; \
  for (; *fromP != fromLim && *toP != toLim; *fromP += 2) \
    *(*toP)++ = (GET_HI(*fromP) << 8) | GET_LO(*fromP); \
}

#define SET2(ptr, ch) \
  (((ptr)[0] = ((ch) & 0xff)), ((ptr)[1] = ((ch) >> 8)))
#define GET_LO(ptr) ((unsigned char)(ptr)[0])
#define GET_HI(ptr) ((unsigned char)(ptr)[1])

DEFINE_UTF16_TO_UTF8(little2_)
DEFINE_UTF16_TO_UTF16(little2_)

#undef SET2
#undef GET_LO
#undef GET_HI

#define SET2(ptr, ch) \
  (((ptr)[0] = ((ch) >> 8)), ((ptr)[1] = ((ch) & 0xFF)))
#define GET_LO(ptr) ((unsigned char)(ptr)[1])
#define GET_HI(ptr) ((unsigned char)(ptr)[0])

DEFINE_UTF16_TO_UTF8(big2_)
DEFINE_UTF16_TO_UTF16(big2_)

#undef SET2
#undef GET_LO
#undef GET_HI

#define LITTLE2_BYTE_TYPE(enc, p) \
 ((p)[1] == 0 \
  ? ((struct normal_encoding *)(enc))->type[(unsigned char)*(p)] \
  : unicode_byte_type((p)[1], (p)[0]))
#define LITTLE2_BYTE_TO_ASCII(enc, p) ((p)[1] == 0 ? (p)[0] : -1)
#define LITTLE2_CHAR_MATCHES(enc, p, c) ((p)[1] == 0 && (p)[0] == c)
#define LITTLE2_IS_NAME_CHAR_MINBPC(enc, p) \
  UCS2_GET_NAMING(namePages, (unsigned char)p[1], (unsigned char)p[0])
#define LITTLE2_IS_NMSTRT_CHAR_MINBPC(enc, p) \
  UCS2_GET_NAMING(nmstrtPages, (unsigned char)p[1], (unsigned char)p[0])

#ifdef XML_MIN_SIZE

static
int little2_byteType(const ENCODING *enc, const char *p)
{
  return LITTLE2_BYTE_TYPE(enc, p);
}

static
int little2_byteToAscii(const ENCODING *enc, const char *p)
{
  return LITTLE2_BYTE_TO_ASCII(enc, p);
}

static
int little2_charMatches(const ENCODING *enc, const char *p, int c)
{
  return LITTLE2_CHAR_MATCHES(enc, p, c);
}

static
int little2_isNameMin(const ENCODING *enc, const char *p)
{
  return LITTLE2_IS_NAME_CHAR_MINBPC(enc, p);
}

static
int little2_isNmstrtMin(const ENCODING *enc, const char *p)
{
  return LITTLE2_IS_NMSTRT_CHAR_MINBPC(enc, p);
}

#undef VTABLE
#define VTABLE VTABLE1, little2_toUtf8, little2_toUtf16

#else /* not XML_MIN_SIZE */

#undef PREFIX
#define PREFIX(ident) little2_ ## ident
#define MINBPC(enc) 2
/* CHAR_MATCHES is guaranteed to have MINBPC bytes available. */
#define BYTE_TYPE(enc, p) LITTLE2_BYTE_TYPE(enc, p)
#define BYTE_TO_ASCII(enc, p) LITTLE2_BYTE_TO_ASCII(enc, p)
#define CHAR_MATCHES(enc, p, c) LITTLE2_CHAR_MATCHES(enc, p, c)
#define IS_NAME_CHAR(enc, p, n) 0
#define IS_NAME_CHAR_MINBPC(enc, p) LITTLE2_IS_NAME_CHAR_MINBPC(enc, p)
#define IS_NMSTRT_CHAR(enc, p, n) (0)
#define IS_NMSTRT_CHAR_MINBPC(enc, p) LITTLE2_IS_NMSTRT_CHAR_MINBPC(enc, p)

/************** xmltok impl add *****************************************/

#ifndef IS_INVALID_CHAR
#define IS_INVALID_CHAR(enc, ptr, n) (0)
#endif



#define INVALID_LEAD_CASE(n, ptr, nextTokPtr) \
    case BT_LEAD ## n: \
      if (end - ptr < n) \
	return XML_TOK_PARTIAL_CHAR; \
      if (IS_INVALID_CHAR(enc, ptr, n)) { \
        *(nextTokPtr) = (ptr); \
        return XML_TOK_INVALID; \
      } \
      ptr += n; \
      break;

#define INVALID_CASES(ptr, nextTokPtr) \
  INVALID_LEAD_CASE(2, ptr, nextTokPtr) \
  INVALID_LEAD_CASE(3, ptr, nextTokPtr) \
  INVALID_LEAD_CASE(4, ptr, nextTokPtr) \
  case BT_NONXML: \
  case BT_MALFORM: \
  case BT_TRAIL: \
    *(nextTokPtr) = (ptr); \
    return XML_TOK_INVALID;

#define CHECK_NAME_CASE(n, enc, ptr, end, nextTokPtr) \
   case BT_LEAD ## n: \
     if (end - ptr < n) \
       return XML_TOK_PARTIAL_CHAR; \
     if (!IS_NAME_CHAR(enc, ptr, n)) { \
       *nextTokPtr = ptr; \
       return XML_TOK_INVALID; \
     } \
     ptr += n; \
     break;

#define CHECK_NAME_CASES(enc, ptr, end, nextTokPtr) \
  case BT_NONASCII: \
    if (!IS_NAME_CHAR_MINBPC(enc, ptr)) { \
      *nextTokPtr = ptr; \
      return XML_TOK_INVALID; \
    } \
  case BT_NMSTRT: \
  case BT_HEX: \
  case BT_DIGIT: \
  case BT_NAME: \
  case BT_MINUS: \
    ptr += MINBPC(enc); \
    break; \
  CHECK_NAME_CASE(2, enc, ptr, end, nextTokPtr) \
  CHECK_NAME_CASE(3, enc, ptr, end, nextTokPtr) \
  CHECK_NAME_CASE(4, enc, ptr, end, nextTokPtr)

#define CHECK_NMSTRT_CASE(n, enc, ptr, end, nextTokPtr) \
   case BT_LEAD ## n: \
     if (end - ptr < n) \
       return XML_TOK_PARTIAL_CHAR; \
     if (!IS_NMSTRT_CHAR(enc, ptr, n)) { \
       *nextTokPtr = ptr; \
       return XML_TOK_INVALID; \
     } \
     ptr += n; \
     break;

#define CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr) \
  case BT_NONASCII: \
    if (!IS_NMSTRT_CHAR_MINBPC(enc, ptr)) { \
      *nextTokPtr = ptr; \
      return XML_TOK_INVALID; \
    } \
  case BT_NMSTRT: \
  case BT_HEX: \
    ptr += MINBPC(enc); \
    break; \
  CHECK_NMSTRT_CASE(2, enc, ptr, end, nextTokPtr) \
  CHECK_NMSTRT_CASE(3, enc, ptr, end, nextTokPtr) \
  CHECK_NMSTRT_CASE(4, enc, ptr, end, nextTokPtr)

#ifndef PREFIX
#define PREFIX(ident) ident
#endif

/* ptr points to character following "<!-" */

static
int PREFIX(scanComment)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr != end) {
    if (!CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    ptr += MINBPC(enc);
    while (ptr != end) {
      switch (BYTE_TYPE(enc, ptr)) {
      INVALID_CASES(ptr, nextTokPtr)
      case BT_MINUS:
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
	  if ((ptr += MINBPC(enc)) == end)
	    return XML_TOK_PARTIAL;
	  if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  *nextTokPtr = ptr + MINBPC(enc);
	  return XML_TOK_COMMENT;
	}
	break;
      default:
	ptr += MINBPC(enc);
	break;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "<!" */

static
int PREFIX(scanDecl)(const ENCODING *enc, const char *ptr, const char *end,
		     const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_MINUS:
    return PREFIX(scanComment)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_LSQB:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_COND_SECT_OPEN;
  case BT_NMSTRT:
  case BT_HEX:
    ptr += MINBPC(enc);
    break;
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_PERCNT:
      if (ptr + MINBPC(enc) == end)
	return XML_TOK_PARTIAL;
      /* don't allow <!ENTITY% foo "whatever"> */
      switch (BYTE_TYPE(enc, ptr + MINBPC(enc))) {
      case BT_S: case BT_CR: case BT_LF: case BT_PERCNT:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      /* fall through */
    case BT_S: case BT_CR: case BT_LF:
      *nextTokPtr = ptr;
      return XML_TOK_DECL_OPEN;
    case BT_NMSTRT:
    case BT_HEX:
      ptr += MINBPC(enc);
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(checkPiTarget)(const ENCODING *enc, const char *ptr, const char *end, int *tokPtr)
{
  int upper = 0;
  *tokPtr = XML_TOK_PI;
  if (end - ptr != MINBPC(enc)*3)
    return 1;
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_x:
    break;
  case ASCII_X:
    upper = 1;
    break;
  default:
    return 1;
  }
  ptr += MINBPC(enc);
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_m:
    break;
  case ASCII_M:
    upper = 1;
    break;
  default:
    return 1;
  }
  ptr += MINBPC(enc);
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_l:
    break;
  case ASCII_L:
    upper = 1;
    break;
  default:
    return 1;
  }
  if (upper)
    return 0;
  *tokPtr = XML_TOK_XML_DECL;
  return 1;
}

/* ptr points to character following "<?" */

static
int PREFIX(scanPi)(const ENCODING *enc, const char *ptr, const char *end,
		   const char **nextTokPtr)
{
  int tok;
  const char *target = ptr;
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_S: case BT_CR: case BT_LF:
      if (!PREFIX(checkPiTarget)(enc, target, ptr, &tok)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      ptr += MINBPC(enc);
      while (ptr != end) {
        switch (BYTE_TYPE(enc, ptr)) {
        INVALID_CASES(ptr, nextTokPtr)
	case BT_QUEST:
	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	    *nextTokPtr = ptr + MINBPC(enc);
	    return tok;
	  }
	  break;
	default:
	  ptr += MINBPC(enc);
	  break;
	}
      }
      return XML_TOK_PARTIAL;
    case BT_QUEST:
      if (!PREFIX(checkPiTarget)(enc, target, ptr, &tok)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	*nextTokPtr = ptr + MINBPC(enc);
	return tok;
      }
      /* fall through */
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}


static
int PREFIX(scanCdataSection)(const ENCODING *enc, const char *ptr, const char *end,
			     const char **nextTokPtr)
{
  static const char CDATA_LSQB[] = { ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, ASCII_LSQB };
  int i;
  /* CDATA[ */
  if (end - ptr < 6 * MINBPC(enc))
    return XML_TOK_PARTIAL;
  for (i = 0; i < 6; i++, ptr += MINBPC(enc)) {
    if (!CHAR_MATCHES(enc, ptr, CDATA_LSQB[i])) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_CDATA_SECT_OPEN;
}

static
int PREFIX(cdataSectionTok)(const ENCODING *enc, const char *ptr, const char *end,
			    const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (!CHAR_MATCHES(enc, ptr, ASCII_RSQB))
      break;
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
      ptr -= MINBPC(enc);
      break;
    }
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_CDATA_SECT_CLOSE;
  case BT_CR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (BYTE_TYPE(enc, ptr) == BT_LF)
      ptr += MINBPC(enc);
    *nextTokPtr = ptr;
    return XML_TOK_DATA_NEWLINE;
  case BT_LF:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DATA_NEWLINE;
  INVALID_CASES(ptr, nextTokPtr)
  default:
    ptr += MINBPC(enc);
    break;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (end - ptr < n || IS_INVALID_CHAR(enc, ptr, n)) { \
	*nextTokPtr = ptr; \
	return XML_TOK_DATA_CHARS; \
      } \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONXML:
    case BT_MALFORM:
    case BT_TRAIL:
    case BT_CR:
    case BT_LF:
    case BT_RSQB:
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

/* ptr points to character following "</" */

static
int PREFIX(scanEndTag)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_S: case BT_CR: case BT_LF:
      for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
	switch (BYTE_TYPE(enc, ptr)) {
	case BT_S: case BT_CR: case BT_LF:
	  break;
	case BT_GT:
	  *nextTokPtr = ptr + MINBPC(enc);
          return XML_TOK_END_TAG;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
      }
      return XML_TOK_PARTIAL;

    case BT_COLON:
      /* no need to check qname syntax here, since end-tag must match exactly */
      ptr += MINBPC(enc);
      break;

    case BT_GT:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_END_TAG;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&#X" */

static
int PREFIX(scanHexCharRef)(const ENCODING *enc, const char *ptr, const char *end,
			   const char **nextTokPtr)
{
  if (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
    case BT_HEX:
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_DIGIT:
      case BT_HEX:
	break;
      case BT_SEMI:
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_CHAR_REF;
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&#" */

static
int PREFIX(scanCharRef)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr != end) {
    if (CHAR_MATCHES(enc, ptr, ASCII_x))
      return PREFIX(scanHexCharRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_DIGIT:
	break;
      case BT_SEMI:
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_CHAR_REF;
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&" */

static
int PREFIX(scanRef)(const ENCODING *enc, const char *ptr, const char *end,
		    const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_NUM:
    return PREFIX(scanCharRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_SEMI:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_ENTITY_REF;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following first character of attribute name */

static
int PREFIX(scanAtts)(const ENCODING *enc, const char *ptr, const char *end,
		     const char **nextTokPtr)
{

  int hadColon = 0;

  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)

    case BT_COLON:
      if (hadColon) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      hadColon = 1;
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      break;

    case BT_S: case BT_CR: case BT_LF:
      for (;;) {
	int t;

	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	t = BYTE_TYPE(enc, ptr);
	if (t == BT_EQUALS)
	  break;
	switch (t) {
	case BT_S:
	case BT_LF:
	case BT_CR:
	  break;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
      }
    /* fall through */
    case BT_EQUALS:
      {
	int open;

	hadColon = 0;

	for (;;) {

	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  open = BYTE_TYPE(enc, ptr);
	  if (open == BT_QUOT || open == BT_APOS)
	    break;
	  switch (open) {
	  case BT_S:
	  case BT_LF:
	  case BT_CR:
	    break;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	}
	ptr += MINBPC(enc);
	/* in attribute value */
	for (;;) {
	  int t;
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  t = BYTE_TYPE(enc, ptr);
	  if (t == open)
	    break;
	  switch (t) {
	  INVALID_CASES(ptr, nextTokPtr)
	  case BT_AMP:
	    {
	      int tok = PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, &ptr);
	      if (tok <= 0) {
		if (tok == XML_TOK_INVALID)
		  *nextTokPtr = ptr;
		return tok;
	      }
	      break;
	    }
	  case BT_LT:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  default:
	    ptr += MINBPC(enc);
	    break;
	  }
	}
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	switch (BYTE_TYPE(enc, ptr)) {
	case BT_S:
	case BT_CR:
	case BT_LF:
	  break;
	case BT_SOL:
	  goto sol;
	case BT_GT:
	  goto gt;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
	/* ptr points to closing quote */
	for (;;) {
	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  switch (BYTE_TYPE(enc, ptr)) {
	  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
	  case BT_S: case BT_CR: case BT_LF:
	    continue;
	  case BT_GT:
          gt:
	    *nextTokPtr = ptr + MINBPC(enc);
	    return XML_TOK_START_TAG_WITH_ATTS;
	  case BT_SOL:
          sol:
	    ptr += MINBPC(enc);
	    if (ptr == end)
	      return XML_TOK_PARTIAL;
	    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	      *nextTokPtr = ptr;
	      return XML_TOK_INVALID;
	    }
	    *nextTokPtr = ptr + MINBPC(enc);
	    return XML_TOK_EMPTY_ELEMENT_WITH_ATTS;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  break;
	}
	break;
      }
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "<" */

static
int PREFIX(scanLt)(const ENCODING *enc, const char *ptr, const char *end,
		   const char **nextTokPtr)
{

  int hadColon;

  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_EXCL:
    if ((ptr += MINBPC(enc)) == end)
      return XML_TOK_PARTIAL;
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_MINUS:
      return PREFIX(scanComment)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    case BT_LSQB:
      return PREFIX(scanCdataSection)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  case BT_QUEST:
    return PREFIX(scanPi)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_SOL:
    return PREFIX(scanEndTag)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }

  hadColon = 0;

  /* we have a start-tag */
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)

    case BT_COLON:
      if (hadColon) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      hadColon = 1;
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
      default:
        *nextTokPtr = ptr;
        return XML_TOK_INVALID;
      }
      break;

    case BT_S: case BT_CR: case BT_LF:
      {
        ptr += MINBPC(enc);
	while (ptr != end) {
	  switch (BYTE_TYPE(enc, ptr)) {
	  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
	  case BT_GT:
	    goto gt;
	  case BT_SOL:
	    goto sol;
	  case BT_S: case BT_CR: case BT_LF:
	    ptr += MINBPC(enc);
	    continue;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  return PREFIX(scanAtts)(enc, ptr, end, nextTokPtr);
	}
	return XML_TOK_PARTIAL;
      }
    case BT_GT:
    gt:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_START_TAG_NO_ATTS;
    case BT_SOL:
    sol:
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_EMPTY_ELEMENT_NO_ATTS;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(contentTok)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_LT:
    return PREFIX(scanLt)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_AMP:
    return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_CR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_CR;
    if (BYTE_TYPE(enc, ptr) == BT_LF)
      ptr += MINBPC(enc);
    *nextTokPtr = ptr;
    return XML_TOK_DATA_NEWLINE;
  case BT_LF:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DATA_NEWLINE;
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_RSQB;
    if (!CHAR_MATCHES(enc, ptr, ASCII_RSQB))
      break;
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_RSQB;
    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
      ptr -= MINBPC(enc);
      break;
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  INVALID_CASES(ptr, nextTokPtr)
  default:
    ptr += MINBPC(enc);
    break;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (end - ptr < n || IS_INVALID_CHAR(enc, ptr, n)) { \
	*nextTokPtr = ptr; \
	return XML_TOK_DATA_CHARS; \
      } \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_RSQB:
      if (ptr + MINBPC(enc) != end) {
	 if (!CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_RSQB)) {
	   ptr += MINBPC(enc);
	   break;
	 }
	 if (ptr + 2*MINBPC(enc) != end) {
	   if (!CHAR_MATCHES(enc, ptr + 2*MINBPC(enc), ASCII_GT)) {
	     ptr += MINBPC(enc);
	     break;
	   }
	   *nextTokPtr = ptr + 2*MINBPC(enc);
	   return XML_TOK_INVALID;
	 }
      }
      /* fall through */
    case BT_AMP:
    case BT_LT:
    case BT_NONXML:
    case BT_MALFORM:
    case BT_TRAIL:
    case BT_CR:
    case BT_LF:
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

/* ptr points to character following "%" */

static
int PREFIX(scanPercent)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_S: case BT_LF: case BT_CR: case BT_PERCNT:
    *nextTokPtr = ptr;
    return XML_TOK_PERCENT;
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_SEMI:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_PARAM_ENTITY_REF;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(scanPoundName)(const ENCODING *enc, const char *ptr, const char *end,
			  const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_CR: case BT_LF: case BT_S:
    case BT_RPAR: case BT_GT: case BT_PERCNT: case BT_VERBAR:
      *nextTokPtr = ptr;
      return XML_TOK_POUND_NAME;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return -XML_TOK_POUND_NAME;
}

static
int PREFIX(scanLit)(int open, const ENCODING *enc,
		    const char *ptr, const char *end,
		    const char **nextTokPtr)
{
  while (ptr != end) {
    int t = BYTE_TYPE(enc, ptr);
    switch (t) {
    INVALID_CASES(ptr, nextTokPtr)
    case BT_QUOT:
    case BT_APOS:
      ptr += MINBPC(enc);
      if (t != open)
	break;
      if (ptr == end)
	return -XML_TOK_LITERAL;
      *nextTokPtr = ptr;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_S: case BT_CR: case BT_LF:
      case BT_GT: case BT_PERCNT: case BT_LSQB:
	return XML_TOK_LITERAL;
      default:
	return XML_TOK_INVALID;
      }
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(prologTok)(const ENCODING *enc, const char *ptr, const char *end,
		      const char **nextTokPtr)
{
  int tok;
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_QUOT:
    return PREFIX(scanLit)(BT_QUOT, enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_APOS:
    return PREFIX(scanLit)(BT_APOS, enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_LT:
    {
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_EXCL:
	return PREFIX(scanDecl)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      case BT_QUEST:
	return PREFIX(scanPi)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      case BT_NMSTRT:
      case BT_HEX:
      case BT_NONASCII:
      case BT_LEAD2:
      case BT_LEAD3:
      case BT_LEAD4:
	*nextTokPtr = ptr - MINBPC(enc);
	return XML_TOK_INSTANCE_START;
      }
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  case BT_CR:
    if (ptr + MINBPC(enc) == end)
      return -XML_TOK_PROLOG_S;
    /* fall through */
  case BT_S: case BT_LF:
    for (;;) {
      ptr += MINBPC(enc);
      if (ptr == end)
	break;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_S: case BT_LF:
	break;
      case BT_CR:
	/* don't split CR/LF pair */
	if (ptr + MINBPC(enc) != end)
	  break;
	/* fall through */
      default:
	*nextTokPtr = ptr;
	return XML_TOK_PROLOG_S;
      }
    }
    *nextTokPtr = ptr;
    return XML_TOK_PROLOG_S;
  case BT_PERCNT:
    return PREFIX(scanPercent)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_COMMA:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_COMMA;
  case BT_LSQB:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OPEN_BRACKET;
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return -XML_TOK_CLOSE_BRACKET;
    if (CHAR_MATCHES(enc, ptr, ASCII_RSQB)) {
      if (ptr + MINBPC(enc) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_GT)) {
	*nextTokPtr = ptr + 2*MINBPC(enc);
	return XML_TOK_COND_SECT_CLOSE;
      }
    }
    *nextTokPtr = ptr;
    return XML_TOK_CLOSE_BRACKET;
  case BT_LPAR:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OPEN_PAREN;
  case BT_RPAR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return -XML_TOK_CLOSE_PAREN;
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_AST:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_ASTERISK;
    case BT_QUEST:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_QUESTION;
    case BT_PLUS:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_PLUS;
    case BT_CR: case BT_LF: case BT_S:
    case BT_GT: case BT_COMMA: case BT_VERBAR:
    case BT_RPAR:
      *nextTokPtr = ptr;
      return XML_TOK_CLOSE_PAREN;
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  case BT_VERBAR:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OR;
  case BT_GT:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DECL_CLOSE;
  case BT_NUM:
    return PREFIX(scanPoundName)(enc, ptr + MINBPC(enc), end, nextTokPtr);
#define LEAD_CASE(n) \
  case BT_LEAD ## n: \
    if (end - ptr < n) \
      return XML_TOK_PARTIAL_CHAR; \
    if (IS_NMSTRT_CHAR(enc, ptr, n)) { \
      ptr += n; \
      tok = XML_TOK_NAME; \
      break; \
    } \
    if (IS_NAME_CHAR(enc, ptr, n)) { \
      ptr += n; \
      tok = XML_TOK_NMTOKEN; \
      break; \
    } \
    *nextTokPtr = ptr; \
    return XML_TOK_INVALID;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
  case BT_NMSTRT:
  case BT_HEX:
    tok = XML_TOK_NAME;
    ptr += MINBPC(enc);
    break;
  case BT_DIGIT:
  case BT_NAME:
  case BT_MINUS:

  case BT_COLON:

    tok = XML_TOK_NMTOKEN;
    ptr += MINBPC(enc);
    break;
  case BT_NONASCII:
    if (IS_NMSTRT_CHAR_MINBPC(enc, ptr)) {
      ptr += MINBPC(enc);
      tok = XML_TOK_NAME;
      break;
    }
    if (IS_NAME_CHAR_MINBPC(enc, ptr)) {
      ptr += MINBPC(enc);
      tok = XML_TOK_NMTOKEN;
      break;
    }
    /* fall through */
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_GT: case BT_RPAR: case BT_COMMA:
    case BT_VERBAR: case BT_LSQB: case BT_PERCNT:
    case BT_S: case BT_CR: case BT_LF:
      *nextTokPtr = ptr;
      return tok;

    case BT_COLON:
      ptr += MINBPC(enc);
      switch (tok) {
      case XML_TOK_NAME:
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	tok = XML_TOK_PREFIXED_NAME;
	switch (BYTE_TYPE(enc, ptr)) {
	CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
	default:
	  tok = XML_TOK_NMTOKEN;
	  break;
	}
	break;
      case XML_TOK_PREFIXED_NAME:
	tok = XML_TOK_NMTOKEN;
	break;
      }
      break;

    case BT_PLUS:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_PLUS;
    case BT_AST:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_ASTERISK;
    case BT_QUEST:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_QUESTION;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return -tok;
}

static
int PREFIX(attributeValueTok)(const ENCODING *enc, const char *ptr, const char *end,
			      const char **nextTokPtr)
{
  const char *start;
  if (ptr == end)
    return XML_TOK_NONE;
  start = ptr;
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_AMP:
      if (ptr == start)
	return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_LT:
      /* this is for inside entity references */
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    case BT_LF:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_CR:
      if (ptr == start) {
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_TRAILING_CR;
	if (BYTE_TYPE(enc, ptr) == BT_LF)
	  ptr += MINBPC(enc);
	*nextTokPtr = ptr;
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_S:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_ATTRIBUTE_VALUE_S;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

static
int PREFIX(entityValueTok)(const ENCODING *enc, const char *ptr, const char *end,
			   const char **nextTokPtr)
{
  const char *start;
  if (ptr == end)
    return XML_TOK_NONE;
  start = ptr;
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_AMP:
      if (ptr == start)
	return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_PERCNT:
      if (ptr == start)
	return PREFIX(scanPercent)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_LF:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_CR:
      if (ptr == start) {
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_TRAILING_CR;
	if (BYTE_TYPE(enc, ptr) == BT_LF)
	  ptr += MINBPC(enc);
	*nextTokPtr = ptr;
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}



static
int PREFIX(ignoreSectionTok)(const ENCODING *enc, const char *ptr, const char *end,
			     const char **nextTokPtr)
{
  int level = 0;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      end = ptr + n;
    }
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    INVALID_CASES(ptr, nextTokPtr)
    case BT_LT:
      if ((ptr += MINBPC(enc)) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_EXCL)) {
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_LSQB)) {
	  ++level;
	  ptr += MINBPC(enc);
	}
      }
      break;
    case BT_RSQB:
      if ((ptr += MINBPC(enc)) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_RSQB)) {
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	  ptr += MINBPC(enc);
	  if (level == 0) {
	    *nextTokPtr = ptr;
	    return XML_TOK_IGNORE_SECT;
	  }
	  --level;
	}
      }
      break;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  return XML_TOK_PARTIAL;
}



static
int PREFIX(isPublicId)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **badPtr)
{
  ptr += MINBPC(enc);
  end -= MINBPC(enc);
  for (; ptr != end; ptr += MINBPC(enc)) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
    case BT_HEX:
    case BT_MINUS:
    case BT_APOS:
    case BT_LPAR:
    case BT_RPAR:
    case BT_PLUS:
    case BT_COMMA:
    case BT_SOL:
    case BT_EQUALS:
    case BT_QUEST:
    case BT_CR:
    case BT_LF:
    case BT_SEMI:
    case BT_EXCL:
    case BT_AST:
    case BT_PERCNT:
    case BT_NUM:

    case BT_COLON:

      break;
    case BT_S:
      if (CHAR_MATCHES(enc, ptr, ASCII_TAB)) {
	*badPtr = ptr;
	return 0;
      }
      break;
    case BT_NAME:
    case BT_NMSTRT:
      if (!(BYTE_TO_ASCII(enc, ptr) & ~0x7f))
	break;
    default:
      switch (BYTE_TO_ASCII(enc, ptr)) {
      case 0x24: /* $ */
      case 0x40: /* @ */
	break;
      default:
	*badPtr = ptr;
	return 0;
      }
      break;
    }
  }
  return 1;
}

/* This must only be called for a well-formed start-tag or empty element tag.
Returns the number of attributes.  Pointers to the first attsMax attributes
are stored in atts. */

static
int PREFIX(getAtts)(const ENCODING *enc, const char *ptr,
		    int attsMax, ATTRIBUTE *atts)
{
  enum { other, inName, inValue } state = inName;
  int nAtts = 0;
  int open = 0; /* defined when state == inValue;
		   initialization just to shut up compilers */

  for (ptr += MINBPC(enc);; ptr += MINBPC(enc)) {
    switch (BYTE_TYPE(enc, ptr)) {
#define START_NAME \
      if (state == other) { \
	if (nAtts < attsMax) { \
	  atts[nAtts].name = ptr; \
	  atts[nAtts].normalized = 1; \
	} \
	state = inName; \
      }
#define LEAD_CASE(n) \
    case BT_LEAD ## n: START_NAME ptr += (n - MINBPC(enc)); break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONASCII:
    case BT_NMSTRT:
    case BT_HEX:
      START_NAME
      break;
#undef START_NAME
    case BT_QUOT:
      if (state != inValue) {
	if (nAtts < attsMax)
	  atts[nAtts].valuePtr = ptr + MINBPC(enc);
        state = inValue;
        open = BT_QUOT;
      }
      else if (open == BT_QUOT) {
        state = other;
	if (nAtts < attsMax)
	  atts[nAtts].valueEnd = ptr;
	nAtts++;
      }
      break;
    case BT_APOS:
      if (state != inValue) {
	if (nAtts < attsMax)
	  atts[nAtts].valuePtr = ptr + MINBPC(enc);
        state = inValue;
        open = BT_APOS;
      }
      else if (open == BT_APOS) {
        state = other;
	if (nAtts < attsMax)
	  atts[nAtts].valueEnd = ptr;
	nAtts++;
      }
      break;
    case BT_AMP:
      if (nAtts < attsMax)
	atts[nAtts].normalized = 0;
      break;
    case BT_S:
      if (state == inName)
        state = other;
      else if (state == inValue
	       && nAtts < attsMax
	       && atts[nAtts].normalized
	       && (ptr == atts[nAtts].valuePtr
		   || BYTE_TO_ASCII(enc, ptr) != ASCII_SPACE
		   || BYTE_TO_ASCII(enc, ptr + MINBPC(enc)) == ASCII_SPACE
	           || BYTE_TYPE(enc, ptr + MINBPC(enc)) == open))
	atts[nAtts].normalized = 0;
      break;
    case BT_CR: case BT_LF:
      /* This case ensures that the first attribute name is counted
         Apart from that we could just change state on the quote. */
      if (state == inName)
        state = other;
      else if (state == inValue && nAtts < attsMax)
	atts[nAtts].normalized = 0;
      break;
    case BT_GT:
    case BT_SOL:
      if (state != inValue)
	return nAtts;
      break;
    default:
      break;
    }
  }
  /* not reached */
}

static
int PREFIX(charRefNumber)(const ENCODING *enc, const char *ptr)
{
  int result = 0;
  /* skip &# */
  ptr += 2*MINBPC(enc);
  if (CHAR_MATCHES(enc, ptr, ASCII_x)) {
    for (ptr += MINBPC(enc); !CHAR_MATCHES(enc, ptr, ASCII_SEMI); ptr += MINBPC(enc)) {
      int c = BYTE_TO_ASCII(enc, ptr);
      switch (c) {
      case ASCII_0: case ASCII_1: case ASCII_2: case ASCII_3: case ASCII_4:
      case ASCII_5: case ASCII_6: case ASCII_7: case ASCII_8: case ASCII_9:
	result <<= 4;
	result |= (c - ASCII_0);
	break;
      case ASCII_A: case ASCII_B: case ASCII_C: case ASCII_D: case ASCII_E: case ASCII_F:
	result <<= 4;
	result += 10 + (c - ASCII_A);
	break;
      case ASCII_a: case ASCII_b: case ASCII_c: case ASCII_d: case ASCII_e: case ASCII_f:
	result <<= 4;
	result += 10 + (c - ASCII_a);
	break;
      }
      if (result >= 0x110000)
	return -1;
    }
  }
  else {
    for (; !CHAR_MATCHES(enc, ptr, ASCII_SEMI); ptr += MINBPC(enc)) {
      int c = BYTE_TO_ASCII(enc, ptr);
      result *= 10;
      result += (c - ASCII_0);
      if (result >= 0x110000)
	return -1;
    }
  }
  return checkCharRefNumber(result);
}

static
int PREFIX(predefinedEntityName)(const ENCODING *enc, const char *ptr, const char *end)
{
  switch ((end - ptr)/MINBPC(enc)) {
  case 2:
    if (CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_t)) {
      switch (BYTE_TO_ASCII(enc, ptr)) {
      case ASCII_l:
	return ASCII_LT;
      case ASCII_g:
	return ASCII_GT;
      }
    }
    break;
  case 3:
    if (CHAR_MATCHES(enc, ptr, ASCII_a)) {
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_m)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_p))
	  return ASCII_AMP;
      }
    }
    break;
  case 4:
    switch (BYTE_TO_ASCII(enc, ptr)) {
    case ASCII_q:
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_u)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_o)) {
	  ptr += MINBPC(enc);
  	  if (CHAR_MATCHES(enc, ptr, ASCII_t))
	    return ASCII_QUOT;
	}
      }
      break;
    case ASCII_a:
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_p)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_o)) {
	  ptr += MINBPC(enc);
  	  if (CHAR_MATCHES(enc, ptr, ASCII_s))
	    return ASCII_APOS;
	}
      }
      break;
    }
  }
  return 0;
}

static
int PREFIX(sameName)(const ENCODING *enc, const char *ptr1, const char *ptr2)
{
  for (;;) {
    switch (BYTE_TYPE(enc, ptr1)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (*ptr1++ != *ptr2++) \
	return 0;
    LEAD_CASE(4) LEAD_CASE(3) LEAD_CASE(2)
#undef LEAD_CASE
      /* fall through */
      if (*ptr1++ != *ptr2++)
	return 0;
      break;
    case BT_NONASCII:
    case BT_NMSTRT:

    case BT_COLON:

    case BT_HEX:
    case BT_DIGIT:
    case BT_NAME:
    case BT_MINUS:
      if (*ptr2++ != *ptr1++)
	return 0;
      if (MINBPC(enc) > 1) {
	if (*ptr2++ != *ptr1++)
	  return 0;
	if (MINBPC(enc) > 2) {
	  if (*ptr2++ != *ptr1++)
	    return 0;
          if (MINBPC(enc) > 3) {
	    if (*ptr2++ != *ptr1++)
      	      return 0;
	  }
	}
      }
      break;
    default:
      if (MINBPC(enc) == 1 && *ptr1 == *ptr2)
	return 1;
      switch (BYTE_TYPE(enc, ptr2)) {
      case BT_LEAD2:
      case BT_LEAD3:
      case BT_LEAD4:
      case BT_NONASCII:
      case BT_NMSTRT:

      case BT_COLON:

      case BT_HEX:
      case BT_DIGIT:
      case BT_NAME:
      case BT_MINUS:
	return 0;
      default:
	return 1;
      }
    }
  }
  /* not reached */
}

static
int PREFIX(nameMatchesAscii)(const ENCODING *enc, const char *ptr1,
			     const char *end1, const char *ptr2)
{
  for (; *ptr2; ptr1 += MINBPC(enc), ptr2++) {
    if (ptr1 == end1)
      return 0;
    if (!CHAR_MATCHES(enc, ptr1, *ptr2))
      return 0;
  }
  return ptr1 == end1;
}

static
int PREFIX(nameLength)(const ENCODING *enc, const char *ptr)
{
  const char *start = ptr;
  for (;;) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONASCII:
    case BT_NMSTRT:

    case BT_COLON:

    case BT_HEX:
    case BT_DIGIT:
    case BT_NAME:
    case BT_MINUS:
      ptr += MINBPC(enc);
      break;
    default:
      return ptr - start;
    }
  }
}

static
const char *PREFIX(skipS)(const ENCODING *enc, const char *ptr)
{
  for (;;) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_LF:
    case BT_CR:
    case BT_S:
      ptr += MINBPC(enc);
      break;
    default:
      return ptr;
    }
  }
}

static
void PREFIX(updatePosition)(const ENCODING *enc,
			    const char *ptr,
			    const char *end,
			    POSITION *pos)
{
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_LF:
      pos->columnNumber = (unsigned)-1;
      pos->lineNumber++;
      ptr += MINBPC(enc);
      break;
    case BT_CR:
      pos->lineNumber++;
      ptr += MINBPC(enc);
      if (ptr != end && BYTE_TYPE(enc, ptr) == BT_LF)
	ptr += MINBPC(enc);
      pos->columnNumber = (unsigned)-1;
      break;
    default:
      ptr += MINBPC(enc);
      break;
    }
    pos->columnNumber++;
  }
}

#undef DO_LEAD_CASE
#undef MULTIBYTE_CASES
#undef INVALID_CASES
#undef CHECK_NAME_CASE
#undef CHECK_NAME_CASES
#undef CHECK_NMSTRT_CASE
#undef CHECK_NMSTRT_CASES

/************** xmltok impl end ****************************************/

#undef MINBPC
#undef BYTE_TYPE
#undef BYTE_TO_ASCII
#undef CHAR_MATCHES
#undef IS_NAME_CHAR
#undef IS_NAME_CHAR_MINBPC
#undef IS_NMSTRT_CHAR
#undef IS_NMSTRT_CHAR_MINBPC
#undef IS_INVALID_CHAR

#endif /* not XML_MIN_SIZE */



static const struct normal_encoding little2_encoding_ns = {
  { VTABLE, 2, 0,
#if XML_BYTE_ORDER == 12
    1
#else
    0
#endif
  },
  {
#include "asciitab.h"
#include "latin1tab.h"
  },
  STANDARD_VTABLE(little2_)
};



static const struct normal_encoding little2_encoding = {
  { VTABLE, 2, 0,
#if XML_BYTE_ORDER == 12
    1
#else
    0
#endif
  },
  {
#define BT_COLON BT_NMSTRT
#include "asciitab.h"
#undef BT_COLON
#include "latin1tab.h"
  },
  STANDARD_VTABLE(little2_)
};

#if XML_BYTE_ORDER != 21



static const struct normal_encoding internal_little2_encoding_ns = {
  { VTABLE, 2, 0, 1 },
  {
#include "iasciitab.h"
#include "latin1tab.h"
  },
  STANDARD_VTABLE(little2_)
};



static const struct normal_encoding internal_little2_encoding = {
  { VTABLE, 2, 0, 1 },
  {
#define BT_COLON BT_NMSTRT
#include "iasciitab.h"
#undef BT_COLON
#include "latin1tab.h"
  },
  STANDARD_VTABLE(little2_)
};

#endif


#define BIG2_BYTE_TYPE(enc, p) \
 ((p)[0] == 0 \
  ? ((struct normal_encoding *)(enc))->type[(unsigned char)(p)[1]] \
  : unicode_byte_type((p)[0], (p)[1]))
#define BIG2_BYTE_TO_ASCII(enc, p) ((p)[0] == 0 ? (p)[1] : -1)
#define BIG2_CHAR_MATCHES(enc, p, c) ((p)[0] == 0 && (p)[1] == c)
#define BIG2_IS_NAME_CHAR_MINBPC(enc, p) \
  UCS2_GET_NAMING(namePages, (unsigned char)p[0], (unsigned char)p[1])
#define BIG2_IS_NMSTRT_CHAR_MINBPC(enc, p) \
  UCS2_GET_NAMING(nmstrtPages, (unsigned char)p[0], (unsigned char)p[1])

#ifdef XML_MIN_SIZE

static
int big2_byteType(const ENCODING *enc, const char *p)
{
  return BIG2_BYTE_TYPE(enc, p);
}

static
int big2_byteToAscii(const ENCODING *enc, const char *p)
{
  return BIG2_BYTE_TO_ASCII(enc, p);
}

static
int big2_charMatches(const ENCODING *enc, const char *p, int c)
{
  return BIG2_CHAR_MATCHES(enc, p, c);
}

static
int big2_isNameMin(const ENCODING *enc, const char *p)
{
  return BIG2_IS_NAME_CHAR_MINBPC(enc, p);
}

static
int big2_isNmstrtMin(const ENCODING *enc, const char *p)
{
  return BIG2_IS_NMSTRT_CHAR_MINBPC(enc, p);
}

#undef VTABLE
#define VTABLE VTABLE1, big2_toUtf8, big2_toUtf16

#else /* not XML_MIN_SIZE */

#undef PREFIX
#define PREFIX(ident) big2_ ## ident
#define MINBPC(enc) 2
/* CHAR_MATCHES is guaranteed to have MINBPC bytes available. */
#define BYTE_TYPE(enc, p) BIG2_BYTE_TYPE(enc, p)
#define BYTE_TO_ASCII(enc, p) BIG2_BYTE_TO_ASCII(enc, p)
#define CHAR_MATCHES(enc, p, c) BIG2_CHAR_MATCHES(enc, p, c)
#define IS_NAME_CHAR(enc, p, n) 0
#define IS_NAME_CHAR_MINBPC(enc, p) BIG2_IS_NAME_CHAR_MINBPC(enc, p)
#define IS_NMSTRT_CHAR(enc, p, n) (0)
#define IS_NMSTRT_CHAR_MINBPC(enc, p) BIG2_IS_NMSTRT_CHAR_MINBPC(enc, p)


/************** xmltok impl add *****************************************/

#ifndef IS_INVALID_CHAR
#define IS_INVALID_CHAR(enc, ptr, n) (0)
#endif



#define INVALID_LEAD_CASE(n, ptr, nextTokPtr) \
    case BT_LEAD ## n: \
      if (end - ptr < n) \
	return XML_TOK_PARTIAL_CHAR; \
      if (IS_INVALID_CHAR(enc, ptr, n)) { \
        *(nextTokPtr) = (ptr); \
        return XML_TOK_INVALID; \
      } \
      ptr += n; \
      break;

#define INVALID_CASES(ptr, nextTokPtr) \
  INVALID_LEAD_CASE(2, ptr, nextTokPtr) \
  INVALID_LEAD_CASE(3, ptr, nextTokPtr) \
  INVALID_LEAD_CASE(4, ptr, nextTokPtr) \
  case BT_NONXML: \
  case BT_MALFORM: \
  case BT_TRAIL: \
    *(nextTokPtr) = (ptr); \
    return XML_TOK_INVALID;

#define CHECK_NAME_CASE(n, enc, ptr, end, nextTokPtr) \
   case BT_LEAD ## n: \
     if (end - ptr < n) \
       return XML_TOK_PARTIAL_CHAR; \
     if (!IS_NAME_CHAR(enc, ptr, n)) { \
       *nextTokPtr = ptr; \
       return XML_TOK_INVALID; \
     } \
     ptr += n; \
     break;

#define CHECK_NAME_CASES(enc, ptr, end, nextTokPtr) \
  case BT_NONASCII: \
    if (!IS_NAME_CHAR_MINBPC(enc, ptr)) { \
      *nextTokPtr = ptr; \
      return XML_TOK_INVALID; \
    } \
  case BT_NMSTRT: \
  case BT_HEX: \
  case BT_DIGIT: \
  case BT_NAME: \
  case BT_MINUS: \
    ptr += MINBPC(enc); \
    break; \
  CHECK_NAME_CASE(2, enc, ptr, end, nextTokPtr) \
  CHECK_NAME_CASE(3, enc, ptr, end, nextTokPtr) \
  CHECK_NAME_CASE(4, enc, ptr, end, nextTokPtr)

#define CHECK_NMSTRT_CASE(n, enc, ptr, end, nextTokPtr) \
   case BT_LEAD ## n: \
     if (end - ptr < n) \
       return XML_TOK_PARTIAL_CHAR; \
     if (!IS_NMSTRT_CHAR(enc, ptr, n)) { \
       *nextTokPtr = ptr; \
       return XML_TOK_INVALID; \
     } \
     ptr += n; \
     break;

#define CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr) \
  case BT_NONASCII: \
    if (!IS_NMSTRT_CHAR_MINBPC(enc, ptr)) { \
      *nextTokPtr = ptr; \
      return XML_TOK_INVALID; \
    } \
  case BT_NMSTRT: \
  case BT_HEX: \
    ptr += MINBPC(enc); \
    break; \
  CHECK_NMSTRT_CASE(2, enc, ptr, end, nextTokPtr) \
  CHECK_NMSTRT_CASE(3, enc, ptr, end, nextTokPtr) \
  CHECK_NMSTRT_CASE(4, enc, ptr, end, nextTokPtr)

#ifndef PREFIX
#define PREFIX(ident) ident
#endif

/* ptr points to character following "<!-" */

static
int PREFIX(scanComment)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr != end) {
    if (!CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    ptr += MINBPC(enc);
    while (ptr != end) {
      switch (BYTE_TYPE(enc, ptr)) {
      INVALID_CASES(ptr, nextTokPtr)
      case BT_MINUS:
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
	  if ((ptr += MINBPC(enc)) == end)
	    return XML_TOK_PARTIAL;
	  if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  *nextTokPtr = ptr + MINBPC(enc);
	  return XML_TOK_COMMENT;
	}
	break;
      default:
	ptr += MINBPC(enc);
	break;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "<!" */

static
int PREFIX(scanDecl)(const ENCODING *enc, const char *ptr, const char *end,
		     const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_MINUS:
    return PREFIX(scanComment)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_LSQB:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_COND_SECT_OPEN;
  case BT_NMSTRT:
  case BT_HEX:
    ptr += MINBPC(enc);
    break;
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_PERCNT:
      if (ptr + MINBPC(enc) == end)
	return XML_TOK_PARTIAL;
      /* don't allow <!ENTITY% foo "whatever"> */
      switch (BYTE_TYPE(enc, ptr + MINBPC(enc))) {
      case BT_S: case BT_CR: case BT_LF: case BT_PERCNT:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      /* fall through */
    case BT_S: case BT_CR: case BT_LF:
      *nextTokPtr = ptr;
      return XML_TOK_DECL_OPEN;
    case BT_NMSTRT:
    case BT_HEX:
      ptr += MINBPC(enc);
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(checkPiTarget)(const ENCODING *enc, const char *ptr, const char *end, int *tokPtr)
{
  int upper = 0;
  *tokPtr = XML_TOK_PI;
  if (end - ptr != MINBPC(enc)*3)
    return 1;
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_x:
    break;
  case ASCII_X:
    upper = 1;
    break;
  default:
    return 1;
  }
  ptr += MINBPC(enc);
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_m:
    break;
  case ASCII_M:
    upper = 1;
    break;
  default:
    return 1;
  }
  ptr += MINBPC(enc);
  switch (BYTE_TO_ASCII(enc, ptr)) {
  case ASCII_l:
    break;
  case ASCII_L:
    upper = 1;
    break;
  default:
    return 1;
  }
  if (upper)
    return 0;
  *tokPtr = XML_TOK_XML_DECL;
  return 1;
}

/* ptr points to character following "<?" */

static
int PREFIX(scanPi)(const ENCODING *enc, const char *ptr, const char *end,
		   const char **nextTokPtr)
{
  int tok;
  const char *target = ptr;
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_S: case BT_CR: case BT_LF:
      if (!PREFIX(checkPiTarget)(enc, target, ptr, &tok)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      ptr += MINBPC(enc);
      while (ptr != end) {
        switch (BYTE_TYPE(enc, ptr)) {
        INVALID_CASES(ptr, nextTokPtr)
	case BT_QUEST:
	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	    *nextTokPtr = ptr + MINBPC(enc);
	    return tok;
	  }
	  break;
	default:
	  ptr += MINBPC(enc);
	  break;
	}
      }
      return XML_TOK_PARTIAL;
    case BT_QUEST:
      if (!PREFIX(checkPiTarget)(enc, target, ptr, &tok)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	*nextTokPtr = ptr + MINBPC(enc);
	return tok;
      }
      /* fall through */
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}


static
int PREFIX(scanCdataSection)(const ENCODING *enc, const char *ptr, const char *end,
			     const char **nextTokPtr)
{
  static const char CDATA_LSQB[] = { ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, ASCII_LSQB };
  int i;
  /* CDATA[ */
  if (end - ptr < 6 * MINBPC(enc))
    return XML_TOK_PARTIAL;
  for (i = 0; i < 6; i++, ptr += MINBPC(enc)) {
    if (!CHAR_MATCHES(enc, ptr, CDATA_LSQB[i])) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_CDATA_SECT_OPEN;
}

static
int PREFIX(cdataSectionTok)(const ENCODING *enc, const char *ptr, const char *end,
			    const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (!CHAR_MATCHES(enc, ptr, ASCII_RSQB))
      break;
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
      ptr -= MINBPC(enc);
      break;
    }
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_CDATA_SECT_CLOSE;
  case BT_CR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_PARTIAL;
    if (BYTE_TYPE(enc, ptr) == BT_LF)
      ptr += MINBPC(enc);
    *nextTokPtr = ptr;
    return XML_TOK_DATA_NEWLINE;
  case BT_LF:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DATA_NEWLINE;
  INVALID_CASES(ptr, nextTokPtr)
  default:
    ptr += MINBPC(enc);
    break;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (end - ptr < n || IS_INVALID_CHAR(enc, ptr, n)) { \
	*nextTokPtr = ptr; \
	return XML_TOK_DATA_CHARS; \
      } \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONXML:
    case BT_MALFORM:
    case BT_TRAIL:
    case BT_CR:
    case BT_LF:
    case BT_RSQB:
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

/* ptr points to character following "</" */

static
int PREFIX(scanEndTag)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_S: case BT_CR: case BT_LF:
      for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
	switch (BYTE_TYPE(enc, ptr)) {
	case BT_S: case BT_CR: case BT_LF:
	  break;
	case BT_GT:
	  *nextTokPtr = ptr + MINBPC(enc);
          return XML_TOK_END_TAG;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
      }
      return XML_TOK_PARTIAL;

    case BT_COLON:
      /* no need to check qname syntax here, since end-tag must match exactly */
      ptr += MINBPC(enc);
      break;

    case BT_GT:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_END_TAG;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&#X" */

static
int PREFIX(scanHexCharRef)(const ENCODING *enc, const char *ptr, const char *end,
			   const char **nextTokPtr)
{
  if (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
    case BT_HEX:
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_DIGIT:
      case BT_HEX:
	break;
      case BT_SEMI:
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_CHAR_REF;
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&#" */

static
int PREFIX(scanCharRef)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr != end) {
    if (CHAR_MATCHES(enc, ptr, ASCII_x))
      return PREFIX(scanHexCharRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
      break;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    for (ptr += MINBPC(enc); ptr != end; ptr += MINBPC(enc)) {
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_DIGIT:
	break;
      case BT_SEMI:
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_CHAR_REF;
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "&" */

static
int PREFIX(scanRef)(const ENCODING *enc, const char *ptr, const char *end,
		    const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_NUM:
    return PREFIX(scanCharRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_SEMI:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_ENTITY_REF;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following first character of attribute name */

static
int PREFIX(scanAtts)(const ENCODING *enc, const char *ptr, const char *end,
		     const char **nextTokPtr)
{

  int hadColon = 0;

  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)

    case BT_COLON:
      if (hadColon) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      hadColon = 1;
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
      default:
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      break;

    case BT_S: case BT_CR: case BT_LF:
      for (;;) {
	int t;

	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	t = BYTE_TYPE(enc, ptr);
	if (t == BT_EQUALS)
	  break;
	switch (t) {
	case BT_S:
	case BT_LF:
	case BT_CR:
	  break;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
      }
    /* fall through */
    case BT_EQUALS:
      {
	int open;

	hadColon = 0;

	for (;;) {

	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  open = BYTE_TYPE(enc, ptr);
	  if (open == BT_QUOT || open == BT_APOS)
	    break;
	  switch (open) {
	  case BT_S:
	  case BT_LF:
	  case BT_CR:
	    break;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	}
	ptr += MINBPC(enc);
	/* in attribute value */
	for (;;) {
	  int t;
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  t = BYTE_TYPE(enc, ptr);
	  if (t == open)
	    break;
	  switch (t) {
	  INVALID_CASES(ptr, nextTokPtr)
	  case BT_AMP:
	    {
	      int tok = PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, &ptr);
	      if (tok <= 0) {
		if (tok == XML_TOK_INVALID)
		  *nextTokPtr = ptr;
		return tok;
	      }
	      break;
	    }
	  case BT_LT:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  default:
	    ptr += MINBPC(enc);
	    break;
	  }
	}
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	switch (BYTE_TYPE(enc, ptr)) {
	case BT_S:
	case BT_CR:
	case BT_LF:
	  break;
	case BT_SOL:
	  goto sol;
	case BT_GT:
	  goto gt;
	default:
	  *nextTokPtr = ptr;
	  return XML_TOK_INVALID;
	}
	/* ptr points to closing quote */
	for (;;) {
	  ptr += MINBPC(enc);
	  if (ptr == end)
	    return XML_TOK_PARTIAL;
	  switch (BYTE_TYPE(enc, ptr)) {
	  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
	  case BT_S: case BT_CR: case BT_LF:
	    continue;
	  case BT_GT:
          gt:
	    *nextTokPtr = ptr + MINBPC(enc);
	    return XML_TOK_START_TAG_WITH_ATTS;
	  case BT_SOL:
          sol:
	    ptr += MINBPC(enc);
	    if (ptr == end)
	      return XML_TOK_PARTIAL;
	    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	      *nextTokPtr = ptr;
	      return XML_TOK_INVALID;
	    }
	    *nextTokPtr = ptr + MINBPC(enc);
	    return XML_TOK_EMPTY_ELEMENT_WITH_ATTS;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  break;
	}
	break;
      }
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

/* ptr points to character following "<" */

static
int PREFIX(scanLt)(const ENCODING *enc, const char *ptr, const char *end,
		   const char **nextTokPtr)
{

  int hadColon;

  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_EXCL:
    if ((ptr += MINBPC(enc)) == end)
      return XML_TOK_PARTIAL;
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_MINUS:
      return PREFIX(scanComment)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    case BT_LSQB:
      return PREFIX(scanCdataSection)(enc, ptr + MINBPC(enc), end, nextTokPtr);
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  case BT_QUEST:
    return PREFIX(scanPi)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_SOL:
    return PREFIX(scanEndTag)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }

  hadColon = 0;

  /* we have a start-tag */
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)

    case BT_COLON:
      if (hadColon) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      hadColon = 1;
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
      default:
        *nextTokPtr = ptr;
        return XML_TOK_INVALID;
      }
      break;

    case BT_S: case BT_CR: case BT_LF:
      {
        ptr += MINBPC(enc);
	while (ptr != end) {
	  switch (BYTE_TYPE(enc, ptr)) {
	  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
	  case BT_GT:
	    goto gt;
	  case BT_SOL:
	    goto sol;
	  case BT_S: case BT_CR: case BT_LF:
	    ptr += MINBPC(enc);
	    continue;
	  default:
	    *nextTokPtr = ptr;
	    return XML_TOK_INVALID;
	  }
	  return PREFIX(scanAtts)(enc, ptr, end, nextTokPtr);
	}
	return XML_TOK_PARTIAL;
      }
    case BT_GT:
    gt:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_START_TAG_NO_ATTS;
    case BT_SOL:
    sol:
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_EMPTY_ELEMENT_NO_ATTS;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(contentTok)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_LT:
    return PREFIX(scanLt)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_AMP:
    return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_CR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_CR;
    if (BYTE_TYPE(enc, ptr) == BT_LF)
      ptr += MINBPC(enc);
    *nextTokPtr = ptr;
    return XML_TOK_DATA_NEWLINE;
  case BT_LF:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DATA_NEWLINE;
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_RSQB;
    if (!CHAR_MATCHES(enc, ptr, ASCII_RSQB))
      break;
    ptr += MINBPC(enc);
    if (ptr == end)
      return XML_TOK_TRAILING_RSQB;
    if (!CHAR_MATCHES(enc, ptr, ASCII_GT)) {
      ptr -= MINBPC(enc);
      break;
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  INVALID_CASES(ptr, nextTokPtr)
  default:
    ptr += MINBPC(enc);
    break;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (end - ptr < n || IS_INVALID_CHAR(enc, ptr, n)) { \
	*nextTokPtr = ptr; \
	return XML_TOK_DATA_CHARS; \
      } \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_RSQB:
      if (ptr + MINBPC(enc) != end) {
	 if (!CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_RSQB)) {
	   ptr += MINBPC(enc);
	   break;
	 }
	 if (ptr + 2*MINBPC(enc) != end) {
	   if (!CHAR_MATCHES(enc, ptr + 2*MINBPC(enc), ASCII_GT)) {
	     ptr += MINBPC(enc);
	     break;
	   }
	   *nextTokPtr = ptr + 2*MINBPC(enc);
	   return XML_TOK_INVALID;
	 }
      }
      /* fall through */
    case BT_AMP:
    case BT_LT:
    case BT_NONXML:
    case BT_MALFORM:
    case BT_TRAIL:
    case BT_CR:
    case BT_LF:
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

/* ptr points to character following "%" */

static
int PREFIX(scanPercent)(const ENCODING *enc, const char *ptr, const char *end,
			const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  case BT_S: case BT_LF: case BT_CR: case BT_PERCNT:
    *nextTokPtr = ptr;
    return XML_TOK_PERCENT;
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_SEMI:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_PARAM_ENTITY_REF;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(scanPoundName)(const ENCODING *enc, const char *ptr, const char *end,
			  const char **nextTokPtr)
{
  if (ptr == end)
    return XML_TOK_PARTIAL;
  switch (BYTE_TYPE(enc, ptr)) {
  CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_CR: case BT_LF: case BT_S:
    case BT_RPAR: case BT_GT: case BT_PERCNT: case BT_VERBAR:
      *nextTokPtr = ptr;
      return XML_TOK_POUND_NAME;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return -XML_TOK_POUND_NAME;
}

static
int PREFIX(scanLit)(int open, const ENCODING *enc,
		    const char *ptr, const char *end,
		    const char **nextTokPtr)
{
  while (ptr != end) {
    int t = BYTE_TYPE(enc, ptr);
    switch (t) {
    INVALID_CASES(ptr, nextTokPtr)
    case BT_QUOT:
    case BT_APOS:
      ptr += MINBPC(enc);
      if (t != open)
	break;
      if (ptr == end)
	return -XML_TOK_LITERAL;
      *nextTokPtr = ptr;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_S: case BT_CR: case BT_LF:
      case BT_GT: case BT_PERCNT: case BT_LSQB:
	return XML_TOK_LITERAL;
      default:
	return XML_TOK_INVALID;
      }
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  return XML_TOK_PARTIAL;
}

static
int PREFIX(prologTok)(const ENCODING *enc, const char *ptr, const char *end,
		      const char **nextTokPtr)
{
  int tok;
  if (ptr == end)
    return XML_TOK_NONE;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      if (n == 0)
	return XML_TOK_PARTIAL;
      end = ptr + n;
    }
  }
  switch (BYTE_TYPE(enc, ptr)) {
  case BT_QUOT:
    return PREFIX(scanLit)(BT_QUOT, enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_APOS:
    return PREFIX(scanLit)(BT_APOS, enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_LT:
    {
      ptr += MINBPC(enc);
      if (ptr == end)
	return XML_TOK_PARTIAL;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_EXCL:
	return PREFIX(scanDecl)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      case BT_QUEST:
	return PREFIX(scanPi)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      case BT_NMSTRT:
      case BT_HEX:
      case BT_NONASCII:
      case BT_LEAD2:
      case BT_LEAD3:
      case BT_LEAD4:
	*nextTokPtr = ptr - MINBPC(enc);
	return XML_TOK_INSTANCE_START;
      }
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  case BT_CR:
    if (ptr + MINBPC(enc) == end)
      return -XML_TOK_PROLOG_S;
    /* fall through */
  case BT_S: case BT_LF:
    for (;;) {
      ptr += MINBPC(enc);
      if (ptr == end)
	break;
      switch (BYTE_TYPE(enc, ptr)) {
      case BT_S: case BT_LF:
	break;
      case BT_CR:
	/* don't split CR/LF pair */
	if (ptr + MINBPC(enc) != end)
	  break;
	/* fall through */
      default:
	*nextTokPtr = ptr;
	return XML_TOK_PROLOG_S;
      }
    }
    *nextTokPtr = ptr;
    return XML_TOK_PROLOG_S;
  case BT_PERCNT:
    return PREFIX(scanPercent)(enc, ptr + MINBPC(enc), end, nextTokPtr);
  case BT_COMMA:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_COMMA;
  case BT_LSQB:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OPEN_BRACKET;
  case BT_RSQB:
    ptr += MINBPC(enc);
    if (ptr == end)
      return -XML_TOK_CLOSE_BRACKET;
    if (CHAR_MATCHES(enc, ptr, ASCII_RSQB)) {
      if (ptr + MINBPC(enc) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_GT)) {
	*nextTokPtr = ptr + 2*MINBPC(enc);
	return XML_TOK_COND_SECT_CLOSE;
      }
    }
    *nextTokPtr = ptr;
    return XML_TOK_CLOSE_BRACKET;
  case BT_LPAR:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OPEN_PAREN;
  case BT_RPAR:
    ptr += MINBPC(enc);
    if (ptr == end)
      return -XML_TOK_CLOSE_PAREN;
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_AST:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_ASTERISK;
    case BT_QUEST:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_QUESTION;
    case BT_PLUS:
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_CLOSE_PAREN_PLUS;
    case BT_CR: case BT_LF: case BT_S:
    case BT_GT: case BT_COMMA: case BT_VERBAR:
    case BT_RPAR:
      *nextTokPtr = ptr;
      return XML_TOK_CLOSE_PAREN;
    }
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  case BT_VERBAR:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_OR;
  case BT_GT:
    *nextTokPtr = ptr + MINBPC(enc);
    return XML_TOK_DECL_CLOSE;
  case BT_NUM:
    return PREFIX(scanPoundName)(enc, ptr + MINBPC(enc), end, nextTokPtr);
#define LEAD_CASE(n) \
  case BT_LEAD ## n: \
    if (end - ptr < n) \
      return XML_TOK_PARTIAL_CHAR; \
    if (IS_NMSTRT_CHAR(enc, ptr, n)) { \
      ptr += n; \
      tok = XML_TOK_NAME; \
      break; \
    } \
    if (IS_NAME_CHAR(enc, ptr, n)) { \
      ptr += n; \
      tok = XML_TOK_NMTOKEN; \
      break; \
    } \
    *nextTokPtr = ptr; \
    return XML_TOK_INVALID;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
  case BT_NMSTRT:
  case BT_HEX:
    tok = XML_TOK_NAME;
    ptr += MINBPC(enc);
    break;
  case BT_DIGIT:
  case BT_NAME:
  case BT_MINUS:

  case BT_COLON:

    tok = XML_TOK_NMTOKEN;
    ptr += MINBPC(enc);
    break;
  case BT_NONASCII:
    if (IS_NMSTRT_CHAR_MINBPC(enc, ptr)) {
      ptr += MINBPC(enc);
      tok = XML_TOK_NAME;
      break;
    }
    if (IS_NAME_CHAR_MINBPC(enc, ptr)) {
      ptr += MINBPC(enc);
      tok = XML_TOK_NMTOKEN;
      break;
    }
    /* fall through */
  default:
    *nextTokPtr = ptr;
    return XML_TOK_INVALID;
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
    case BT_GT: case BT_RPAR: case BT_COMMA:
    case BT_VERBAR: case BT_LSQB: case BT_PERCNT:
    case BT_S: case BT_CR: case BT_LF:
      *nextTokPtr = ptr;
      return tok;

    case BT_COLON:
      ptr += MINBPC(enc);
      switch (tok) {
      case XML_TOK_NAME:
	if (ptr == end)
	  return XML_TOK_PARTIAL;
	tok = XML_TOK_PREFIXED_NAME;
	switch (BYTE_TYPE(enc, ptr)) {
	CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)
	default:
	  tok = XML_TOK_NMTOKEN;
	  break;
	}
	break;
      case XML_TOK_PREFIXED_NAME:
	tok = XML_TOK_NMTOKEN;
	break;
      }
      break;

    case BT_PLUS:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_PLUS;
    case BT_AST:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_ASTERISK;
    case BT_QUEST:
      if (tok == XML_TOK_NMTOKEN)  {
	*nextTokPtr = ptr;
	return XML_TOK_INVALID;
      }
      *nextTokPtr = ptr + MINBPC(enc);
      return XML_TOK_NAME_QUESTION;
    default:
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
  }
  return -tok;
}

static
int PREFIX(attributeValueTok)(const ENCODING *enc, const char *ptr, const char *end,
			      const char **nextTokPtr)
{
  const char *start;
  if (ptr == end)
    return XML_TOK_NONE;
  start = ptr;
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_AMP:
      if (ptr == start)
	return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_LT:
      /* this is for inside entity references */
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    case BT_LF:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_CR:
      if (ptr == start) {
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_TRAILING_CR;
	if (BYTE_TYPE(enc, ptr) == BT_LF)
	  ptr += MINBPC(enc);
	*nextTokPtr = ptr;
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_S:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_ATTRIBUTE_VALUE_S;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}

static
int PREFIX(entityValueTok)(const ENCODING *enc, const char *ptr, const char *end,
			   const char **nextTokPtr)
{
  const char *start;
  if (ptr == end)
    return XML_TOK_NONE;
  start = ptr;
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_AMP:
      if (ptr == start)
	return PREFIX(scanRef)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_PERCNT:
      if (ptr == start)
	return PREFIX(scanPercent)(enc, ptr + MINBPC(enc), end, nextTokPtr);
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_LF:
      if (ptr == start) {
	*nextTokPtr = ptr + MINBPC(enc);
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    case BT_CR:
      if (ptr == start) {
	ptr += MINBPC(enc);
	if (ptr == end)
	  return XML_TOK_TRAILING_CR;
	if (BYTE_TYPE(enc, ptr) == BT_LF)
	  ptr += MINBPC(enc);
	*nextTokPtr = ptr;
	return XML_TOK_DATA_NEWLINE;
      }
      *nextTokPtr = ptr;
      return XML_TOK_DATA_CHARS;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  *nextTokPtr = ptr;
  return XML_TOK_DATA_CHARS;
}



static
int PREFIX(ignoreSectionTok)(const ENCODING *enc, const char *ptr, const char *end,
			     const char **nextTokPtr)
{
  int level = 0;
  if (MINBPC(enc) > 1) {
    size_t n = end - ptr;
    if (n & (MINBPC(enc) - 1)) {
      n &= ~(MINBPC(enc) - 1);
      end = ptr + n;
    }
  }
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
    INVALID_CASES(ptr, nextTokPtr)
    case BT_LT:
      if ((ptr += MINBPC(enc)) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_EXCL)) {
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_LSQB)) {
	  ++level;
	  ptr += MINBPC(enc);
	}
      }
      break;
    case BT_RSQB:
      if ((ptr += MINBPC(enc)) == end)
	return XML_TOK_PARTIAL;
      if (CHAR_MATCHES(enc, ptr, ASCII_RSQB)) {
	if ((ptr += MINBPC(enc)) == end)
	  return XML_TOK_PARTIAL;
	if (CHAR_MATCHES(enc, ptr, ASCII_GT)) {
	  ptr += MINBPC(enc);
	  if (level == 0) {
	    *nextTokPtr = ptr;
	    return XML_TOK_IGNORE_SECT;
	  }
	  --level;
	}
      }
      break;
    default:
      ptr += MINBPC(enc);
      break;
    }
  }
  return XML_TOK_PARTIAL;
}



static
int PREFIX(isPublicId)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **badPtr)
{
  ptr += MINBPC(enc);
  end -= MINBPC(enc);
  for (; ptr != end; ptr += MINBPC(enc)) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_DIGIT:
    case BT_HEX:
    case BT_MINUS:
    case BT_APOS:
    case BT_LPAR:
    case BT_RPAR:
    case BT_PLUS:
    case BT_COMMA:
    case BT_SOL:
    case BT_EQUALS:
    case BT_QUEST:
    case BT_CR:
    case BT_LF:
    case BT_SEMI:
    case BT_EXCL:
    case BT_AST:
    case BT_PERCNT:
    case BT_NUM:

    case BT_COLON:

      break;
    case BT_S:
      if (CHAR_MATCHES(enc, ptr, ASCII_TAB)) {
	*badPtr = ptr;
	return 0;
      }
      break;
    case BT_NAME:
    case BT_NMSTRT:
      if (!(BYTE_TO_ASCII(enc, ptr) & ~0x7f))
	break;
    default:
      switch (BYTE_TO_ASCII(enc, ptr)) {
      case 0x24: /* $ */
      case 0x40: /* @ */
	break;
      default:
	*badPtr = ptr;
	return 0;
      }
      break;
    }
  }
  return 1;
}

/* This must only be called for a well-formed start-tag or empty element tag.
Returns the number of attributes.  Pointers to the first attsMax attributes
are stored in atts. */

static
int PREFIX(getAtts)(const ENCODING *enc, const char *ptr,
		    int attsMax, ATTRIBUTE *atts)
{
  enum { other, inName, inValue } state = inName;
  int nAtts = 0;
  int open = 0; /* defined when state == inValue;
		   initialization just to shut up compilers */

  for (ptr += MINBPC(enc);; ptr += MINBPC(enc)) {
    switch (BYTE_TYPE(enc, ptr)) {
#define START_NAME \
      if (state == other) { \
	if (nAtts < attsMax) { \
	  atts[nAtts].name = ptr; \
	  atts[nAtts].normalized = 1; \
	} \
	state = inName; \
      }
#define LEAD_CASE(n) \
    case BT_LEAD ## n: START_NAME ptr += (n - MINBPC(enc)); break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONASCII:
    case BT_NMSTRT:
    case BT_HEX:
      START_NAME
      break;
#undef START_NAME
    case BT_QUOT:
      if (state != inValue) {
	if (nAtts < attsMax)
	  atts[nAtts].valuePtr = ptr + MINBPC(enc);
        state = inValue;
        open = BT_QUOT;
      }
      else if (open == BT_QUOT) {
        state = other;
	if (nAtts < attsMax)
	  atts[nAtts].valueEnd = ptr;
	nAtts++;
      }
      break;
    case BT_APOS:
      if (state != inValue) {
	if (nAtts < attsMax)
	  atts[nAtts].valuePtr = ptr + MINBPC(enc);
        state = inValue;
        open = BT_APOS;
      }
      else if (open == BT_APOS) {
        state = other;
	if (nAtts < attsMax)
	  atts[nAtts].valueEnd = ptr;
	nAtts++;
      }
      break;
    case BT_AMP:
      if (nAtts < attsMax)
	atts[nAtts].normalized = 0;
      break;
    case BT_S:
      if (state == inName)
        state = other;
      else if (state == inValue
	       && nAtts < attsMax
	       && atts[nAtts].normalized
	       && (ptr == atts[nAtts].valuePtr
		   || BYTE_TO_ASCII(enc, ptr) != ASCII_SPACE
		   || BYTE_TO_ASCII(enc, ptr + MINBPC(enc)) == ASCII_SPACE
	           || BYTE_TYPE(enc, ptr + MINBPC(enc)) == open))
	atts[nAtts].normalized = 0;
      break;
    case BT_CR: case BT_LF:
      /* This case ensures that the first attribute name is counted
         Apart from that we could just change state on the quote. */
      if (state == inName)
        state = other;
      else if (state == inValue && nAtts < attsMax)
	atts[nAtts].normalized = 0;
      break;
    case BT_GT:
    case BT_SOL:
      if (state != inValue)
	return nAtts;
      break;
    default:
      break;
    }
  }
  /* not reached */
}

static
int PREFIX(charRefNumber)(const ENCODING *enc, const char *ptr)
{
  int result = 0;
  /* skip &# */
  ptr += 2*MINBPC(enc);
  if (CHAR_MATCHES(enc, ptr, ASCII_x)) {
    for (ptr += MINBPC(enc); !CHAR_MATCHES(enc, ptr, ASCII_SEMI); ptr += MINBPC(enc)) {
      int c = BYTE_TO_ASCII(enc, ptr);
      switch (c) {
      case ASCII_0: case ASCII_1: case ASCII_2: case ASCII_3: case ASCII_4:
      case ASCII_5: case ASCII_6: case ASCII_7: case ASCII_8: case ASCII_9:
	result <<= 4;
	result |= (c - ASCII_0);
	break;
      case ASCII_A: case ASCII_B: case ASCII_C: case ASCII_D: case ASCII_E: case ASCII_F:
	result <<= 4;
	result += 10 + (c - ASCII_A);
	break;
      case ASCII_a: case ASCII_b: case ASCII_c: case ASCII_d: case ASCII_e: case ASCII_f:
	result <<= 4;
	result += 10 + (c - ASCII_a);
	break;
      }
      if (result >= 0x110000)
	return -1;
    }
  }
  else {
    for (; !CHAR_MATCHES(enc, ptr, ASCII_SEMI); ptr += MINBPC(enc)) {
      int c = BYTE_TO_ASCII(enc, ptr);
      result *= 10;
      result += (c - ASCII_0);
      if (result >= 0x110000)
	return -1;
    }
  }
  return checkCharRefNumber(result);
}

static
int PREFIX(predefinedEntityName)(const ENCODING *enc, const char *ptr, const char *end)
{
  switch ((end - ptr)/MINBPC(enc)) {
  case 2:
    if (CHAR_MATCHES(enc, ptr + MINBPC(enc), ASCII_t)) {
      switch (BYTE_TO_ASCII(enc, ptr)) {
      case ASCII_l:
	return ASCII_LT;
      case ASCII_g:
	return ASCII_GT;
      }
    }
    break;
  case 3:
    if (CHAR_MATCHES(enc, ptr, ASCII_a)) {
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_m)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_p))
	  return ASCII_AMP;
      }
    }
    break;
  case 4:
    switch (BYTE_TO_ASCII(enc, ptr)) {
    case ASCII_q:
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_u)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_o)) {
	  ptr += MINBPC(enc);
  	  if (CHAR_MATCHES(enc, ptr, ASCII_t))
	    return ASCII_QUOT;
	}
      }
      break;
    case ASCII_a:
      ptr += MINBPC(enc);
      if (CHAR_MATCHES(enc, ptr, ASCII_p)) {
	ptr += MINBPC(enc);
	if (CHAR_MATCHES(enc, ptr, ASCII_o)) {
	  ptr += MINBPC(enc);
  	  if (CHAR_MATCHES(enc, ptr, ASCII_s))
	    return ASCII_APOS;
	}
      }
      break;
    }
  }
  return 0;
}

static
int PREFIX(sameName)(const ENCODING *enc, const char *ptr1, const char *ptr2)
{
  for (;;) {
    switch (BYTE_TYPE(enc, ptr1)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      if (*ptr1++ != *ptr2++) \
	return 0;
    LEAD_CASE(4) LEAD_CASE(3) LEAD_CASE(2)
#undef LEAD_CASE
      /* fall through */
      if (*ptr1++ != *ptr2++)
	return 0;
      break;
    case BT_NONASCII:
    case BT_NMSTRT:

    case BT_COLON:

    case BT_HEX:
    case BT_DIGIT:
    case BT_NAME:
    case BT_MINUS:
      if (*ptr2++ != *ptr1++)
	return 0;
      if (MINBPC(enc) > 1) {
	if (*ptr2++ != *ptr1++)
	  return 0;
	if (MINBPC(enc) > 2) {
	  if (*ptr2++ != *ptr1++)
	    return 0;
          if (MINBPC(enc) > 3) {
	    if (*ptr2++ != *ptr1++)
      	      return 0;
	  }
	}
      }
      break;
    default:
      if (MINBPC(enc) == 1 && *ptr1 == *ptr2)
	return 1;
      switch (BYTE_TYPE(enc, ptr2)) {
      case BT_LEAD2:
      case BT_LEAD3:
      case BT_LEAD4:
      case BT_NONASCII:
      case BT_NMSTRT:

      case BT_COLON:

      case BT_HEX:
      case BT_DIGIT:
      case BT_NAME:
      case BT_MINUS:
	return 0;
      default:
	return 1;
      }
    }
  }
  /* not reached */
}

static
int PREFIX(nameMatchesAscii)(const ENCODING *enc, const char *ptr1,
			     const char *end1, const char *ptr2)
{
  for (; *ptr2; ptr1 += MINBPC(enc), ptr2++) {
    if (ptr1 == end1)
      return 0;
    if (!CHAR_MATCHES(enc, ptr1, *ptr2))
      return 0;
  }
  return ptr1 == end1;
}

static
int PREFIX(nameLength)(const ENCODING *enc, const char *ptr)
{
  const char *start = ptr;
  for (;;) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: ptr += n; break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_NONASCII:
    case BT_NMSTRT:

    case BT_COLON:

    case BT_HEX:
    case BT_DIGIT:
    case BT_NAME:
    case BT_MINUS:
      ptr += MINBPC(enc);
      break;
    default:
      return ptr - start;
    }
  }
}

static
const char *PREFIX(skipS)(const ENCODING *enc, const char *ptr)
{
  for (;;) {
    switch (BYTE_TYPE(enc, ptr)) {
    case BT_LF:
    case BT_CR:
    case BT_S:
      ptr += MINBPC(enc);
      break;
    default:
      return ptr;
    }
  }
}

static
void PREFIX(updatePosition)(const ENCODING *enc,
			    const char *ptr,
			    const char *end,
			    POSITION *pos)
{
  while (ptr != end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      ptr += n; \
      break;
    LEAD_CASE(2) LEAD_CASE(3) LEAD_CASE(4)
#undef LEAD_CASE
    case BT_LF:
      pos->columnNumber = (unsigned)-1;
      pos->lineNumber++;
      ptr += MINBPC(enc);
      break;
    case BT_CR:
      pos->lineNumber++;
      ptr += MINBPC(enc);
      if (ptr != end && BYTE_TYPE(enc, ptr) == BT_LF)
	ptr += MINBPC(enc);
      pos->columnNumber = (unsigned)-1;
      break;
    default:
      ptr += MINBPC(enc);
      break;
    }
    pos->columnNumber++;
  }
}

#undef DO_LEAD_CASE
#undef MULTIBYTE_CASES
#undef INVALID_CASES
#undef CHECK_NAME_CASE
#undef CHECK_NAME_CASES
#undef CHECK_NMSTRT_CASE
#undef CHECK_NMSTRT_CASES

/************** xmltok impl end ****************************************/


#undef MINBPC
#undef BYTE_TYPE
#undef BYTE_TO_ASCII
#undef CHAR_MATCHES
#undef IS_NAME_CHAR
#undef IS_NAME_CHAR_MINBPC
#undef IS_NMSTRT_CHAR
#undef IS_NMSTRT_CHAR_MINBPC
#undef IS_INVALID_CHAR

#endif /* not XML_MIN_SIZE */



static const struct normal_encoding big2_encoding_ns = {
  { VTABLE, 2, 0,
#if XML_BYTE_ORDER == 21
  1
#else
  0
#endif
  },
  {
#include "asciitab.h"
#include "latin1tab.h"
  },
  STANDARD_VTABLE(big2_)
};



static const struct normal_encoding big2_encoding = {
  { VTABLE, 2, 0,
#if XML_BYTE_ORDER == 21
  1
#else
  0
#endif
  },
  {
#define BT_COLON BT_NMSTRT
#include "asciitab.h"
#undef BT_COLON
#include "latin1tab.h"
  },
  STANDARD_VTABLE(big2_)
};

#if XML_BYTE_ORDER != 12



static const struct normal_encoding internal_big2_encoding_ns = {
  { VTABLE, 2, 0, 1 },
  {
#include "iasciitab.h"
#include "latin1tab.h"
  },
  STANDARD_VTABLE(big2_)
};



static const struct normal_encoding internal_big2_encoding = {
  { VTABLE, 2, 0, 1 },
  {
#define BT_COLON BT_NMSTRT
#include "iasciitab.h"
#undef BT_COLON
#include "latin1tab.h"
  },
  STANDARD_VTABLE(big2_)
};

#endif

#undef PREFIX

static
int streqci(const char *s1, const char *s2)
{
  for (;;) {
    char c1 = *s1++;
    char c2 = *s2++;
    if (ASCII_a <= c1 && c1 <= ASCII_z)
      c1 += ASCII_A - ASCII_a;
    if (ASCII_a <= c2 && c2 <= ASCII_z)
      c2 += ASCII_A - ASCII_a;
    if (c1 != c2)
      return 0;
    if (!c1)
      break;
  }
  return 1;
}

static
void initUpdatePosition(const ENCODING *enc, const char *ptr,
			const char *end, POSITION *pos)
{
  normal_updatePosition(&utf8_encoding.enc, ptr, end, pos);
}

static
int toAscii(const ENCODING *enc, const char *ptr, const char *end)
{
  char buf[1];
  char *p = buf;
  XmlUtf8Convert(enc, &ptr, end, &p, p + 1);
  if (p == buf)
    return -1;
  else
    return buf[0];
}

static
int isSpace(int c)
{
  switch (c) {
  case 0x20:
  case 0xD:
  case 0xA:
  case 0x9:
    return 1;
  }
  return 0;
}

/* Return 1 if there's just optional white space
or there's an S followed by name=val. */
static
int parsePseudoAttribute(const ENCODING *enc,
			 const char *ptr,
			 const char *end,
			 const char **namePtr,
			 const char **nameEndPtr,
			 const char **valPtr,
			 const char **nextTokPtr)
{
  int c;
  char open;
  if (ptr == end) {
    *namePtr = 0;
    return 1;
  }
  if (!isSpace(toAscii(enc, ptr, end))) {
    *nextTokPtr = ptr;
    return 0;
  }
  do {
    ptr += enc->minBytesPerChar;
  } while (isSpace(toAscii(enc, ptr, end)));
  if (ptr == end) {
    *namePtr = 0;
    return 1;
  }
  *namePtr = ptr;
  for (;;) {
    c = toAscii(enc, ptr, end);
    if (c == -1) {
      *nextTokPtr = ptr;
      return 0;
    }
    if (c == ASCII_EQUALS) {
      *nameEndPtr = ptr;
      break;
    }
    if (isSpace(c)) {
      *nameEndPtr = ptr;
      do {
	ptr += enc->minBytesPerChar;
      } while (isSpace(c = toAscii(enc, ptr, end)));
      if (c != ASCII_EQUALS) {
	*nextTokPtr = ptr;
	return 0;
      }
      break;
    }
    ptr += enc->minBytesPerChar;
  }
  if (ptr == *namePtr) {
    *nextTokPtr = ptr;
    return 0;
  }
  ptr += enc->minBytesPerChar;
  c = toAscii(enc, ptr, end);
  while (isSpace(c)) {
    ptr += enc->minBytesPerChar;
    c = toAscii(enc, ptr, end);
  }
  if (c != ASCII_QUOT && c != ASCII_APOS) {
    *nextTokPtr = ptr;
    return 0;
  }
  open = c;
  ptr += enc->minBytesPerChar;
  *valPtr = ptr;
  for (;; ptr += enc->minBytesPerChar) {
    c = toAscii(enc, ptr, end);
    if (c == open)
      break;
    if (!(ASCII_a <= c && c <= ASCII_z)
	&& !(ASCII_A <= c && c <= ASCII_Z)
	&& !(ASCII_0 <= c && c <= ASCII_9)
	&& c != ASCII_PERIOD
	&& c != ASCII_MINUS
	&& c != ASCII_UNDERSCORE) {
      *nextTokPtr = ptr;
      return 0;
    }
  }
  *nextTokPtr = ptr + enc->minBytesPerChar;
  return 1;
}

static const char KW_version[] = {
  ASCII_v, ASCII_e, ASCII_r, ASCII_s, ASCII_i, ASCII_o, ASCII_n, '\0'
};

static const char KW_encoding[] = {
  ASCII_e, ASCII_n, ASCII_c, ASCII_o, ASCII_d, ASCII_i, ASCII_n, ASCII_g, '\0'
};

static const char KW_standalone[] = {
  ASCII_s, ASCII_t, ASCII_a, ASCII_n, ASCII_d, ASCII_a, ASCII_l, ASCII_o, ASCII_n, ASCII_e, '\0'
};

static const char KW_yes[] = {
  ASCII_y, ASCII_e, ASCII_s,  '\0'
};

static const char KW_no[] = {
  ASCII_n, ASCII_o,  '\0'
};

static
int doParseXmlDecl(const ENCODING *(*encodingFinder)(const ENCODING *,
		                                     const char *,
						     const char *),
		   int isGeneralTextEntity,
		   const ENCODING *enc,
		   const char *ptr,
		   const char *end,
		   const char **badPtr,
		   const char **versionPtr,
		   const char **versionEndPtr,
		   const char **encodingName,
		   const ENCODING **encoding,
		   int *standalone)
{
  const char *val = 0;
  const char *name = 0;
  const char *nameEnd = 0;
  ptr += 5 * enc->minBytesPerChar;
  end -= 2 * enc->minBytesPerChar;
  if (!parsePseudoAttribute(enc, ptr, end, &name, &nameEnd, &val, &ptr) || !name) {
    *badPtr = ptr;
    return 0;
  }
  if (!XmlNameMatchesAscii(enc, name, nameEnd, KW_version)) {
    if (!isGeneralTextEntity) {
      *badPtr = name;
      return 0;
    }
  }
  else {
    if (versionPtr)
      *versionPtr = val;
    if (versionEndPtr)
      *versionEndPtr = ptr;
    if (!parsePseudoAttribute(enc, ptr, end, &name, &nameEnd, &val, &ptr)) {
      *badPtr = ptr;
      return 0;
    }
    if (!name) {
      if (isGeneralTextEntity) {
	/* a TextDecl must have an EncodingDecl */
	*badPtr = ptr;
	return 0;
      }
      return 1;
    }
  }
  if (XmlNameMatchesAscii(enc, name, nameEnd, KW_encoding)) {
    int c = toAscii(enc, val, end);
    if (!(ASCII_a <= c && c <= ASCII_z) && !(ASCII_A <= c && c <= ASCII_Z)) {
      *badPtr = val;
      return 0;
    }
    if (encodingName)
      *encodingName = val;
    if (encoding)
      *encoding = encodingFinder(enc, val, ptr - enc->minBytesPerChar);
    if (!parsePseudoAttribute(enc, ptr, end, &name, &nameEnd, &val, &ptr)) {
      *badPtr = ptr;
      return 0;
    }
    if (!name)
      return 1;
  }
  if (!XmlNameMatchesAscii(enc, name, nameEnd, KW_standalone) || isGeneralTextEntity) {
    *badPtr = name;
    return 0;
  }
  if (XmlNameMatchesAscii(enc, val, ptr - enc->minBytesPerChar, KW_yes)) {
    if (standalone)
      *standalone = 1;
  }
  else if (XmlNameMatchesAscii(enc, val, ptr - enc->minBytesPerChar, KW_no)) {
    if (standalone)
      *standalone = 0;
  }
  else {
    *badPtr = val;
    return 0;
  }
  while (isSpace(toAscii(enc, ptr, end)))
    ptr += enc->minBytesPerChar;
  if (ptr != end) {
    *badPtr = ptr;
    return 0;
  }
  return 1;
}

static
int checkCharRefNumber(int result)
{
  switch (result >> 8) {
  case 0xD8: case 0xD9: case 0xDA: case 0xDB:
  case 0xDC: case 0xDD: case 0xDE: case 0xDF:
    return -1;
  case 0:
    if (latin1_encoding.type[result] == BT_NONXML)
      return -1;
    break;
  case 0xFF:
    if (result == 0xFFFE || result == 0xFFFF)
      return -1;
    break;
  }
  return result;
}

int XmlUtf8Encode(int c, char *buf)
{
  enum {
    /* minN is minimum legal resulting value for N byte sequence */
    min2 = 0x80,
    min3 = 0x800,
    min4 = 0x10000
  };

  if (c < 0)
    return 0;
  if (c < min2) {
    buf[0] = (c | UTF8_cval1);
    return 1;
  }
  if (c < min3) {
    buf[0] = ((c >> 6) | UTF8_cval2);
    buf[1] = ((c & 0x3f) | 0x80);
    return 2;
  }
  if (c < min4) {
    buf[0] = ((c >> 12) | UTF8_cval3);
    buf[1] = (((c >> 6) & 0x3f) | 0x80);
    buf[2] = ((c & 0x3f) | 0x80);
    return 3;
  }
  if (c < 0x110000) {
    buf[0] = ((c >> 18) | UTF8_cval4);
    buf[1] = (((c >> 12) & 0x3f) | 0x80);
    buf[2] = (((c >> 6) & 0x3f) | 0x80);
    buf[3] = ((c & 0x3f) | 0x80);
    return 4;
  }
  return 0;
}

int XmlUtf16Encode(int charNum, unsigned short *buf)
{
  if (charNum < 0)
    return 0;
  if (charNum < 0x10000) {
    buf[0] = charNum;
    return 1;
  }
  if (charNum < 0x110000) {
    charNum -= 0x10000;
    buf[0] = (charNum >> 10) + 0xD800;
    buf[1] = (charNum & 0x3FF) + 0xDC00;
    return 2;
  }
  return 0;
}

struct unknown_encoding {
  struct normal_encoding normal;
  int (*convert)(void *userData, const char *p);
  void *userData;
  unsigned short utf16[256];
  char utf8[256][4];
};

int XmlSizeOfUnknownEncoding(void)
{
  return sizeof(struct unknown_encoding);
}

static
int unknown_isName(const ENCODING *enc, const char *p)
{
  int c = ((const struct unknown_encoding *)enc)
	  ->convert(((const struct unknown_encoding *)enc)->userData, p);
  if (c & ~0xFFFF)
    return 0;
  return UCS2_GET_NAMING(namePages, c >> 8, c & 0xFF);
}

static
int unknown_isNmstrt(const ENCODING *enc, const char *p)
{
  int c = ((const struct unknown_encoding *)enc)
	  ->convert(((const struct unknown_encoding *)enc)->userData, p);
  if (c & ~0xFFFF)
    return 0;
  return UCS2_GET_NAMING(nmstrtPages, c >> 8, c & 0xFF);
}

static
int unknown_isInvalid(const ENCODING *enc, const char *p)
{
  int c = ((const struct unknown_encoding *)enc)
	   ->convert(((const struct unknown_encoding *)enc)->userData, p);
  return (c & ~0xFFFF) || checkCharRefNumber(c) < 0;
}

static
void unknown_toUtf8(const ENCODING *enc,
		    const char **fromP, const char *fromLim,
		    char **toP, const char *toLim)
{
  char buf[XML_UTF8_ENCODE_MAX];
  for (;;) {
    const char *utf8;
    int n;
    if (*fromP == fromLim)
      break;
    utf8 = ((const struct unknown_encoding *)enc)->utf8[(unsigned char)**fromP];
    n = *utf8++;
    if (n == 0) {
      int c = ((const struct unknown_encoding *)enc)
	      ->convert(((const struct unknown_encoding *)enc)->userData, *fromP);
      n = XmlUtf8Encode(c, buf);
      if (n > toLim - *toP)
	break;
      utf8 = buf;
      *fromP += ((const struct normal_encoding *)enc)->type[(unsigned char)**fromP]
	         - (BT_LEAD2 - 2);
    }
    else {
      if (n > toLim - *toP)
	break;
      (*fromP)++;
    }
    do {
      *(*toP)++ = *utf8++;
    } while (--n != 0);
  }
}

static
void unknown_toUtf16(const ENCODING *enc,
		     const char **fromP, const char *fromLim,
		     unsigned short **toP, const unsigned short *toLim)
{
  while (*fromP != fromLim && *toP != toLim) {
    unsigned short c
      = ((const struct unknown_encoding *)enc)->utf16[(unsigned char)**fromP];
    if (c == 0) {
      c = (unsigned short)((const struct unknown_encoding *)enc)
	   ->convert(((const struct unknown_encoding *)enc)->userData, *fromP);
      *fromP += ((const struct normal_encoding *)enc)->type[(unsigned char)**fromP]
	         - (BT_LEAD2 - 2);
    }
    else
      (*fromP)++;
    *(*toP)++ = c;
  }
}

ENCODING *
XmlInitUnknownEncoding(void *mem,
		       int *table,
		       int (*convert)(void *userData, const char *p),
		       void *userData)
{
  int i;
  struct unknown_encoding *e = mem;
  for (i = 0; i < (int)sizeof(struct normal_encoding); i++)
    ((char *)mem)[i] = ((char *)&latin1_encoding)[i];
  for (i = 0; i < 128; i++)
    if (latin1_encoding.type[i] != BT_OTHER
        && latin1_encoding.type[i] != BT_NONXML
	&& table[i] != i)
      return 0;
  for (i = 0; i < 256; i++) {
    int c = table[i];
    if (c == -1) {
      e->normal.type[i] = BT_MALFORM;
      /* This shouldn't really get used. */
      e->utf16[i] = 0xFFFF;
      e->utf8[i][0] = 1;
      e->utf8[i][1] = 0;
    }
    else if (c < 0) {
      if (c < -4)
	return 0;
      e->normal.type[i] = BT_LEAD2 - (c + 2);
      e->utf8[i][0] = 0;
      e->utf16[i] = 0;
    }
    else if (c < 0x80) {
      if (latin1_encoding.type[c] != BT_OTHER
	  && latin1_encoding.type[c] != BT_NONXML
	  && c != i)
	return 0;
      e->normal.type[i] = latin1_encoding.type[c];
      e->utf8[i][0] = 1;
      e->utf8[i][1] = (char)c;
      e->utf16[i] = c == 0 ? 0xFFFF : c;
    }
    else if (checkCharRefNumber(c) < 0) {
      e->normal.type[i] = BT_NONXML;
      /* This shouldn't really get used. */
      e->utf16[i] = 0xFFFF;
      e->utf8[i][0] = 1;
      e->utf8[i][1] = 0;
    }
    else {
      if (c > 0xFFFF)
	return 0;
      if (UCS2_GET_NAMING(nmstrtPages, c >> 8, c & 0xff))
	e->normal.type[i] = BT_NMSTRT;
      else if (UCS2_GET_NAMING(namePages, c >> 8, c & 0xff))
	e->normal.type[i] = BT_NAME;
      else
	e->normal.type[i] = BT_OTHER;
      e->utf8[i][0] = (char)XmlUtf8Encode(c, e->utf8[i] + 1);
      e->utf16[i] = c;
    }
  }
  e->userData = userData;
  e->convert = convert;
  if (convert) {
    e->normal.isName2 = unknown_isName;
    e->normal.isName3 = unknown_isName;
    e->normal.isName4 = unknown_isName;
    e->normal.isNmstrt2 = unknown_isNmstrt;
    e->normal.isNmstrt3 = unknown_isNmstrt;
    e->normal.isNmstrt4 = unknown_isNmstrt;
    e->normal.isInvalid2 = unknown_isInvalid;
    e->normal.isInvalid3 = unknown_isInvalid;
    e->normal.isInvalid4 = unknown_isInvalid;
  }
  e->normal.enc.utf8Convert = unknown_toUtf8;
  e->normal.enc.utf16Convert = unknown_toUtf16;
  return &(e->normal.enc);
}

/* If this enumeration is changed, getEncodingIndex and encodings
must also be changed. */
enum {
  UNKNOWN_ENC = -1,
  ISO_8859_1_ENC = 0,
  US_ASCII_ENC,
  UTF_8_ENC,
  UTF_16_ENC,
  UTF_16BE_ENC,
  UTF_16LE_ENC,
  /* must match encodingNames up to here */
  NO_ENC
};

static const char KW_ISO_8859_1[] = {
  ASCII_I, ASCII_S, ASCII_O, ASCII_MINUS, ASCII_8, ASCII_8, ASCII_5, ASCII_9, ASCII_MINUS, ASCII_1, '\0'
};
static const char KW_US_ASCII[] = {
  ASCII_U, ASCII_S, ASCII_MINUS, ASCII_A, ASCII_S, ASCII_C, ASCII_I, ASCII_I, '\0'
};
static const char KW_UTF_8[] =	{
  ASCII_U, ASCII_T, ASCII_F, ASCII_MINUS, ASCII_8, '\0'
};
static const char KW_UTF_16[] =	{
  ASCII_U, ASCII_T, ASCII_F, ASCII_MINUS, ASCII_1, ASCII_6, '\0'
};
static const char KW_UTF_16BE[] = {
  ASCII_U, ASCII_T, ASCII_F, ASCII_MINUS, ASCII_1, ASCII_6, ASCII_B, ASCII_E, '\0'
};
static const char KW_UTF_16LE[] = {
  ASCII_U, ASCII_T, ASCII_F, ASCII_MINUS, ASCII_1, ASCII_6, ASCII_L, ASCII_E, '\0'
};

static
int getEncodingIndex(const char *name)
{
  static const char *encodingNames[] = {
    KW_ISO_8859_1,
    KW_US_ASCII,
    KW_UTF_8,
    KW_UTF_16,
    KW_UTF_16BE,
    KW_UTF_16LE,
  };
  int i;
  if (name == 0)
    return NO_ENC;
  for (i = 0; i < (int)(sizeof(encodingNames)/sizeof(encodingNames[0])); i++)
    if (streqci(name, encodingNames[i]))
      return i;
  return UNKNOWN_ENC;
}

/* For binary compatibility, we store the index of the encoding specified
at initialization in the isUtf16 member. */

#define INIT_ENC_INDEX(enc) ((int)(enc)->initEnc.isUtf16)
#define SET_INIT_ENC_INDEX(enc, i) ((enc)->initEnc.isUtf16 = (char)i)

/* This is what detects the encoding.
encodingTable maps from encoding indices to encodings;
INIT_ENC_INDEX(enc) is the index of the external (protocol) specified encoding;
state is XML_CONTENT_STATE if we're parsing an external text entity,
and XML_PROLOG_STATE otherwise.
*/


static
int initScan(const ENCODING **encodingTable,
	     const INIT_ENCODING *enc,
	     int state,
	     const char *ptr,
	     const char *end,
	     const char **nextTokPtr)
{
  const ENCODING **encPtr;

  if (ptr == end)
    return XML_TOK_NONE;
  encPtr = enc->encPtr;
  if (ptr + 1 == end) {
    /* only a single byte available for auto-detection */

    /* so we're parsing an external text entity... */
    /* if UTF-16 was externally specified, then we need at least 2 bytes */
    switch (INIT_ENC_INDEX(enc)) {
    case UTF_16_ENC:
    case UTF_16LE_ENC:
    case UTF_16BE_ENC:
      return XML_TOK_PARTIAL;
    }
    switch ((unsigned char)*ptr) {
    case 0xFE:
    case 0xFF:
    case 0xEF: /* possibly first byte of UTF-8 BOM */
      if (INIT_ENC_INDEX(enc) == ISO_8859_1_ENC
	  && state == XML_CONTENT_STATE)
	break;
      /* fall through */
    case 0x00:
    case 0x3C:
      return XML_TOK_PARTIAL;
    }
  }
  else {
    switch (((unsigned char)ptr[0] << 8) | (unsigned char)ptr[1]) {
    case 0xFEFF:
      if (INIT_ENC_INDEX(enc) == ISO_8859_1_ENC
	  && state == XML_CONTENT_STATE)
	break;
      *nextTokPtr = ptr + 2;
      *encPtr = encodingTable[UTF_16BE_ENC];
      return XML_TOK_BOM;
    /* 00 3C is handled in the default case */
    case 0x3C00:
      if ((INIT_ENC_INDEX(enc) == UTF_16BE_ENC
	   || INIT_ENC_INDEX(enc) == UTF_16_ENC)
	  && state == XML_CONTENT_STATE)
	break;
      *encPtr = encodingTable[UTF_16LE_ENC];
      return XmlTok(*encPtr, state, ptr, end, nextTokPtr);
    case 0xFFFE:
      if (INIT_ENC_INDEX(enc) == ISO_8859_1_ENC
	  && state == XML_CONTENT_STATE)
	break;
      *nextTokPtr = ptr + 2;
      *encPtr = encodingTable[UTF_16LE_ENC];
      return XML_TOK_BOM;
    case 0xEFBB:
      /* Maybe a UTF-8 BOM (EF BB BF) */
      /* If there's an explicitly specified (external) encoding
         of ISO-8859-1 or some flavour of UTF-16
         and this is an external text entity,
	 don't look for the BOM,
         because it might be a legal data. */
      if (state == XML_CONTENT_STATE) {
	int e = INIT_ENC_INDEX(enc);
	if (e == ISO_8859_1_ENC || e == UTF_16BE_ENC || e == UTF_16LE_ENC || e == UTF_16_ENC)
	  break;
      }
      if (ptr + 2 == end)
	return XML_TOK_PARTIAL;
      if ((unsigned char)ptr[2] == 0xBF) {
	*encPtr = encodingTable[UTF_8_ENC];
	return XML_TOK_BOM;
      }
      break;
    default:
      if (ptr[0] == '\0') {
	/* 0 isn't a legal data character. Furthermore a document entity can only
	   start with ASCII characters.  So the only way this can fail to be big-endian
	   UTF-16 if it it's an external parsed general entity that's labelled as
	   UTF-16LE. */
	if (state == XML_CONTENT_STATE && INIT_ENC_INDEX(enc) == UTF_16LE_ENC)
	  break;
	*encPtr = encodingTable[UTF_16BE_ENC];
	return XmlTok(*encPtr, state, ptr, end, nextTokPtr);
      }
      else if (ptr[1] == '\0') {
	/* We could recover here in the case:
	    - parsing an external entity
	    - second byte is 0
	    - no externally specified encoding
	    - no encoding declaration
	   by assuming UTF-16LE.  But we don't, because this would mean when
	   presented just with a single byte, we couldn't reliably determine
	   whether we needed further bytes. */
	if (state == XML_CONTENT_STATE)
	  break;
	*encPtr = encodingTable[UTF_16LE_ENC];
	return XmlTok(*encPtr, state, ptr, end, nextTokPtr);
      }
      break;
    }
  }
  *encPtr = encodingTable[INIT_ENC_INDEX(enc)];
  return XmlTok(*encPtr, state, ptr, end, nextTokPtr);
}


#define NS(x) x
#define ns(x) x

/*********************** xmltok ns *****************************/
const ENCODING *NS(XmlGetUtf8InternalEncoding)(void)
{
  return &ns(internal_utf8_encoding).enc;
}

const ENCODING *NS(XmlGetUtf16InternalEncoding)(void)
{
#if XML_BYTE_ORDER == 12
  return &ns(internal_little2_encoding).enc;
#elif XML_BYTE_ORDER == 21
  return &ns(internal_big2_encoding).enc;
#else
  const short n = 1;
  return *(const char *)&n ? &ns(internal_little2_encoding).enc : &ns(internal_big2_encoding).enc;
#endif
}

static
const ENCODING *NS(encodings)[] = {
  &ns(latin1_encoding).enc,
  &ns(ascii_encoding).enc,
  &ns(utf8_encoding).enc,
  &ns(big2_encoding).enc,
  &ns(big2_encoding).enc,
  &ns(little2_encoding).enc,
  &ns(utf8_encoding).enc /* NO_ENC */
};

static
int NS(initScanProlog)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  return initScan(NS(encodings), (const INIT_ENCODING *)enc, XML_PROLOG_STATE, ptr, end, nextTokPtr);
}

static
int NS(initScanContent)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  return initScan(NS(encodings), (const INIT_ENCODING *)enc, XML_CONTENT_STATE, ptr, end, nextTokPtr);
}

int NS(XmlInitEncoding)(INIT_ENCODING *p, const ENCODING **encPtr, const char *name)
{
  int i = getEncodingIndex(name);
  if (i == UNKNOWN_ENC)
    return 0;
  SET_INIT_ENC_INDEX(p, i);
  p->initEnc.scanners[XML_PROLOG_STATE] = NS(initScanProlog);
  p->initEnc.scanners[XML_CONTENT_STATE] = NS(initScanContent);
  p->initEnc.updatePosition = initUpdatePosition;
  p->encPtr = encPtr;
  *encPtr = &(p->initEnc);
  return 1;
}

static
const ENCODING *NS(findEncoding)(const ENCODING *enc, const char *ptr, const char *end)
{
#define ENCODING_MAX 128
  char buf[ENCODING_MAX];
  char *p = buf;
  int i;
  XmlUtf8Convert(enc, &ptr, end, &p, p + ENCODING_MAX - 1);
  if (ptr != end)
    return 0;
  *p = 0;
  if (streqci(buf, KW_UTF_16) && enc->minBytesPerChar == 2)
    return enc;
  i = getEncodingIndex(buf);
  if (i == UNKNOWN_ENC)
    return 0;
  return NS(encodings)[i];
}

int NS(XmlParseXmlDecl)(int isGeneralTextEntity,
			const ENCODING *enc,
			const char *ptr,
			const char *end,
			const char **badPtr,
			const char **versionPtr,
			const char **versionEndPtr,
			const char **encodingName,
			const ENCODING **encoding,
			int *standalone)
{
  return doParseXmlDecl(NS(findEncoding),
			isGeneralTextEntity,
			enc,
			ptr,
			end,
			badPtr,
			versionPtr,
			versionEndPtr,
			encodingName,
			encoding,
			standalone);
}


/************************ xmltok ns end *************************/

#undef NS
#undef ns



#define NS(x) x ## NS
#define ns(x) x ## _ns


/*********************** xmltok ns *****************************/
const ENCODING *NS(XmlGetUtf8InternalEncoding)(void)
{
  return &ns(internal_utf8_encoding).enc;
}

const ENCODING *NS(XmlGetUtf16InternalEncoding)(void)
{
#if XML_BYTE_ORDER == 12
  return &ns(internal_little2_encoding).enc;
#elif XML_BYTE_ORDER == 21
  return &ns(internal_big2_encoding).enc;
#else
  const short n = 1;
  return *(const char *)&n ? &ns(internal_little2_encoding).enc : &ns(internal_big2_encoding).enc;
#endif
}

static
const ENCODING *NS(encodings)[] = {
  &ns(latin1_encoding).enc,
  &ns(ascii_encoding).enc,
  &ns(utf8_encoding).enc,
  &ns(big2_encoding).enc,
  &ns(big2_encoding).enc,
  &ns(little2_encoding).enc,
  &ns(utf8_encoding).enc /* NO_ENC */
};

static
int NS(initScanProlog)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  return initScan(NS(encodings), (const INIT_ENCODING *)enc, XML_PROLOG_STATE, ptr, end, nextTokPtr);
}

static
int NS(initScanContent)(const ENCODING *enc, const char *ptr, const char *end,
		       const char **nextTokPtr)
{
  return initScan(NS(encodings), (const INIT_ENCODING *)enc, XML_CONTENT_STATE, ptr, end, nextTokPtr);
}

int NS(XmlInitEncoding)(INIT_ENCODING *p, const ENCODING **encPtr, const char *name)
{
  int i = getEncodingIndex(name);
  if (i == UNKNOWN_ENC)
    return 0;
  SET_INIT_ENC_INDEX(p, i);
  p->initEnc.scanners[XML_PROLOG_STATE] = NS(initScanProlog);
  p->initEnc.scanners[XML_CONTENT_STATE] = NS(initScanContent);
  p->initEnc.updatePosition = initUpdatePosition;
  p->encPtr = encPtr;
  *encPtr = &(p->initEnc);
  return 1;
}

static
const ENCODING *NS(findEncoding)(const ENCODING *enc, const char *ptr, const char *end)
{
#define ENCODING_MAX 128
  char buf[ENCODING_MAX];
  char *p = buf;
  int i;
  XmlUtf8Convert(enc, &ptr, end, &p, p + ENCODING_MAX - 1);
  if (ptr != end)
    return 0;
  *p = 0;
  if (streqci(buf, KW_UTF_16) && enc->minBytesPerChar == 2)
    return enc;
  i = getEncodingIndex(buf);
  if (i == UNKNOWN_ENC)
    return 0;
  return NS(encodings)[i];
}

int NS(XmlParseXmlDecl)(int isGeneralTextEntity,
			const ENCODING *enc,
			const char *ptr,
			const char *end,
			const char **badPtr,
			const char **versionPtr,
			const char **versionEndPtr,
			const char **encodingName,
			const ENCODING **encoding,
			int *standalone)
{
  return doParseXmlDecl(NS(findEncoding),
			isGeneralTextEntity,
			enc,
			ptr,
			end,
			badPtr,
			versionPtr,
			versionEndPtr,
			encodingName,
			encoding,
			standalone);
}


/************************ xmltok ns end *************************/


#undef NS
#undef ns

ENCODING *
XmlInitUnknownEncodingNS(void *mem,
		         int *table,
		         int (*convert)(void *userData, const char *p),
		         void *userData)
{
  ENCODING *enc = XmlInitUnknownEncoding(mem, table, convert, userData);
  if (enc)
    ((struct normal_encoding *)enc)->type[ASCII_COLON] = BT_COLON;
  return enc;
}


