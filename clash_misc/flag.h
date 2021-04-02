#ifndef FLAG_H
#define FLAG_H

#include <stddef.h>
#include <stdarg.h>

#define MB_CUR_RIS 4
#define MAX_FLAGS_IN_GROUP 6

// +1 for appending a space after each flag
#define BUF_SZ ( \
  (MB_CUR_RIS+MB_CUR_RIS+1) * (MAX_FLAGS_IN_GROUP-1) + \
  (MB_CUR_RIS+MB_CUR_MAX+1) * 1 + \
  1 \
)

// https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2
// https://en.wikipedia.org/wiki/Country_code
typedef struct {
  char c[2];
} CC;

// __attribute__((deprecated))
typedef struct {
  wchar_t wcs[2+1];
} FlagWCS;

// https://en.wikipedia.org/wiki/UTF-8#Encoding
// U+10000...U+10FFFF 4-byte
// __attribute__((deprecated))
typedef struct {
  char mbs[4+4+1];
} FlagMBS;

wchar_t c2ris(const char);

void ccs2str(char *const, ...);

/*__attribute__((deprecated))*/ // void cc2wcs(FlagWCS *const, const CC *const);
/*__attribute__((deprecated))*/ // void wcs2mbs(FlagMBS *const, const FlagWCS *const);
/*__attribute__((deprecated))*/ // void cc2wcs2mbs(FlagMBS *const, const CC *const);

#endif
