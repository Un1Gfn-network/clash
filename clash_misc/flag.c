#include <assert.h>
#include <stdbool.h>
#include <stdio.h> // sprintf()
#include <stdlib.h> // wctomb()
#include <string.h> // memcmp()
#include <strings.h> // bzero()
#include <wchar.h>

#include "./flag.h"

// #define eprintf(...) fprintf(stderr,__VA_ARGS__)

// Convert a character to a RIS character (half a country code)
// https://en.wikipedia.org/wiki/Regional_indicator_symbol
// wchar_t aka int
wchar_t c2ris(const char c){
  assert('A'<=c&&c<='Z');
  // wchar_t ris_A=L'\U0001F1E6';
  // const wchar_t ris_Z=L'\U0001F1FF';
  // assert((ris_Z-ris_A)==('Z'-'A'));
  #define RIS_A (L'\U0001F1E6')
  #define RIS_Z (L'\U0001F1FF')
  static_assert((RIS_Z-RIS_A)==('Z'-'A'));
  wchar_t r=(wchar_t)RIS_A+(wchar_t)(c-'A');
  assert(RIS_A<=r&&r<=RIS_Z);
  return r;
}

// Print one country code
int cc2str(char *const s, const CC *const cc){
  assert(s);
  assert(cc);
  if(0==memcmp(cc->c,"CN",2)||0==memcmp(cc->c,"MO",2)){
    assert(3==sprintf(s,"\u2612")); // ☒
    return 3;
  }
  if(0==memcmp(cc->c,"HK",2)){
    // https://en.wikipedia.org/wiki/Specials_(Unicode_block)#Replacement_character
    // https://www.fileformat.info/info/unicode/char/1f3f4/index.htm
    // https://www.compart.com/en/unicode/U+1F3F4
    assert(4==sprintf(s,"\U0001F3F4")); // 🏴
    return 4;
  }
  assert(MB_CUR_RIS==wctomb(s,c2ris(cc->c[0])));
  assert(MB_CUR_RIS==wctomb(s+MB_CUR_RIS,c2ris(cc->c[1])));
  return (MB_CUR_RIS+MB_CUR_RIS);
}

// Print one or more country codes to a buffer
void ccs2str(char *const dest, ...){

  bzero(dest,BUF_SZ);

  va_list ap;
  va_start(ap,dest);
  const CC *p=NULL;

  char *s=dest;
  // eprintf("\n");
  // eprintf("--%ld\n",BUF_SZ-1);
  // eprintf("%ld\n",s-dest);
  while(NULL!=(p=(const CC*)va_arg(ap,const CC*))){
    // eprintf(" ------ %llu <= %llu\n",(long long unsigned)(s+MB_CUR_RIS+MB_CUR_MAX),(long long unsigned)(dest+BUF_SZ-1));
    assert( s>=dest && s+MB_CUR_RIS+MB_CUR_MAX+1<=dest+BUF_SZ-1 );
    s=s+cc2str(s,p);
    *(s++)=' ';
    // eprintf("%ld\n",s-dest);
  }

  // Remove trailing space
  char *const printed_just_now=s-1;
  if(printed_just_now>=dest&&' '==*printed_just_now)
    s=printed_just_now;

  *s='\0';

}

/*void cc2wcs(FlagWCS *const dest, const CC *const src){
  *dest=(FlagWCS){.wcs={
    [0]=c2ris((src->c)[0]),
    [1]=c2ris((src->c)[1]),
    [2]=L'\0'
  }};
  // dest->wcs[0]=c2ris((src->c)[0]);
  // dest->wcs[1]=c2ris((src->c)[1]);
  // dest->wcs[2]=L'\0';
}*/

/*void wcs2mbs(FlagMBS *const dest, const FlagWCS *const src){
  // Restartable
  // const wchar_t *p=src->wcs;
  // mbstate_t t={};
  // assert(1==mbsinit(&t));
  // assert(8==wcsrtombs(NULL,&p,0,&t));
  // assert(1==mbsinit(&t));
  // assert(p==src->wcs);
  // assert(8==wcsrtombs(dest->mbs,&p,9,&t));
  // assert(1==mbsinit(&t));
  // assert(p==NULL);
  // Non-restartable
  assert(8==wcstombs(NULL,src->wcs,0));
  assert(8==wcstombs(dest->mbs,src->wcs,9));
  assert(dest->mbs[8]=='\0');
}*/

/*void cc2wcs2mbs(FlagMBS *const dest, const CC *const src){
  FlagWCS w={};
  cc2wcs(&w,src);
  wcs2mbs(dest,&w);
}*/
