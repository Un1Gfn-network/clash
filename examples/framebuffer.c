// ./framebuffer.sh
// https://github.com/Un1Gfn/sphinx.public/kms.rst

#include <assert.h>
#include <curses.h>
#include <fcntl.h> // open()
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h> // ioctl()
#include <sys/mman.h> // mmap() munmap()
#include <time.h>
#include <unistd.h> // close()
// #include <errno.h>

#define PICH 160
#define PICW 280

#define XRES 1366
#define YRES 768

#define BPP 4 // bytes_per_pixel
// #define BIPP 32 // bits_per_pixel

static int fbfd=-1;
static __u32 smem_len=0;

static __u32 xres_padded=0;

#define BK 0x00000000 // Black
#define WH 0x00FFFFFF // White
#define RD 0x00FF0000 // Red
#define GN 0x0000FF00 // Green
#define BU 0x000000FF // Blue
#define RAND_COLOR ((__u32)random()%0x01000000)
typedef uint32_t Color;

static void *scr0=NULL;
#define SCR ((Color(*)[xres_padded])(scr0))
// Color (*buf)[xres_padded]
// #define A(P) ((Color(*)[xres_padded])(P))

static struct fb_var_screeninfo fvs={};

static inline void f_f_s(){

  struct fb_fix_screeninfo FI={};
  assert(0==ioctl(fbfd,FBIOGET_FSCREENINFO,&FI));

  assert(0==strcmp("i915",FI.id));
  // assert(0==strcmp("i915""drm""fb",FI.id));

  assert(0==FI.smem_start);

  assert(FB_TYPE_PACKED_PIXELS==FI.type);
  assert(!FI.type_aux);

  assert(FB_VISUAL_TRUECOLOR==FI.visual);

  assert(1==FI.xpanstep);
  assert(1==FI.ypanstep);
  assert(0==FI.ywrapstep);

  assert(0==FI.line_length%BPP);
  xres_padded=FI.line_length/BPP;
  printf("%u bytes per line\n",xres_padded);
  assert(XRES<=xres_padded&&xres_padded<XRES+XRES);

  smem_len=FI.smem_len;
  assert(FI.line_length*YRES==smem_len);

  assert(0==FI.mmio_start);
  assert(0==FI.mmio_len);

  assert(0==FI.accel);

  assert(0==(FB_CAP_FOURCC&FI.capabilities));
  assert(0==FI.capabilities);

  assert(0==FI.reserved[0]);
  assert(0==FI.reserved[1]);

}

static inline void f_v_s(){

  assert(0==ioctl(fbfd,FBIOGET_VSCREENINFO,&fvs));

  assert(XRES==fvs.xres);
  assert(YRES==fvs.yres);
  assert(XRES==fvs.xres_virtual);
  assert(YRES==fvs.yres_virtual);
  assert(0==fvs.xoffset);
  assert(0==fvs.yoffset);

  assert(32==fvs.bits_per_pixel);
  assert(0==fvs.grayscale);

  assert(16==fvs.red.offset&&
         8==fvs.green.offset&&
         0==fvs.blue.offset);
  assert(8==fvs.red.length&&
         8==fvs.green.length&&
         8==fvs.blue.length);
  assert(0==fvs.red.msb_right&&
         0==fvs.green.msb_right&&
         0==fvs.blue.msb_right);
  
  // printf("%u %u %u\n",
  //        fvs.transp.offset,
  //        fvs.transp.length,
  //        fvs.transp.msb_right);
  assert(0==fvs.transp.offset&&
         0==fvs.transp.length&&
         0==fvs.transp.msb_right);

  assert(!fvs.nonstd);

  assert(FB_ACTIVATE_NOW==fvs.activate);
  
  // printf("%0umm x %umm\n",fvs.height,fvs.width);
  assert(PICH==fvs.height);
  assert(PICW==fvs.width);

  assert(1==fvs.accel_flags);

  assert(0==fvs.pixclock);
  assert(0==fvs.left_margin);
  assert(0==fvs.right_margin);
  assert(0==fvs.upper_margin);
  assert(0==fvs.lower_margin);
  assert(0==fvs.hsync_len);
  assert(0==fvs.vsync_len);
  assert(0==fvs.sync);
  assert(0==fvs.vmode);
  assert(0==fvs.rotate);
  assert(0==fvs.colorspace);

  // assert(0==fvs.reserved[0]);
  // assert(0==fvs.reserved[1]);
  // assert(0==fvs.reserved[2]);
  // assert(0==fvs.reserved[3]);
  static_assert(16==sizeof(fvs.reserved));
  for(size_t i=0;i<sizeof(fvs.reserved)/sizeof(fvs.reserved[0]);++i)
    assert(0==fvs.reserved[i]);

}

static inline void fill(const Color c){
  // console_codes(4)
  // printf("\033[999E\n"); // Move cursor to the bottom
  // printf("\033[2K\n"); // ESC [ 2 K: erase whole line
  // printf("\033[3J\n"); // ESC [ 3 J: erase whole display including scroll-back buffer
  // explicit_bzero(scr0,smem_len);
  for(__u32 x=0;x<XRES;++x)for(__u32 y=0;y<YRES;++y)
    SCR[y][x]=c;
}

static inline void demo0_rgb(){

  for(size_t y=0;y<YRES/2;++y){
    for(size_t x=0;     x<XRES/2;++x)SCR[y][x]=RD;
    for(size_t x=XRES/2;x<XRES  ;++x)SCR[y][x]=GN;
  }

  for(size_t y=YRES/2;y<YRES;++y)
    for(size_t x=0;x<XRES;++x)SCR[y][x]=BU;

}

static inline void demo1_noise(){
  for(size_t y=0;y<YRES;++y)
    for(size_t x=0;x<XRES;++x)
      SCR[y][x]=random();
}

static inline void demo2_sector(const Color c,const __u32 radius){
  for(size_t y=0;y<radius;y+=1)
    for(size_t x=0;x<radius;x+=1)
      if(x*x+y*y<radius*radius)
        SCR[y][x]=c;
}

static inline void demo3_sector_msaaNx(const Color c,const __u32 radius,const __u32 N,const Color bg){

  // n^2=N
  bool N_valid=false;
  __u32 n=2;
  for(;n<20;++n)if(N==n*n){
    N_valid=true;
    break;
  }
  assert(N_valid);

  // Oversample
  static_assert(8/8==sizeof(bool));
  void *scros0=calloc((n*XRES)*(n*YRES),sizeof(bool));
  #define SCROS0 ((bool(*)[n*XRES])(scros0))

  for(__u32 y=0;y<(n*radius);y+=1)
    for(__u32 x=0;x<(n*radius);x+=1)
      if(x*x+y*y<(n*radius)*(n*radius))
        SCROS0[y][x]=1;

  for(__u32 y=0;y<radius;++y)for(__u32 x=0;x<radius;++x){
    int count=0;
    for(__u32 i=0;i<n;++i)
      for(__u32 j=0;j<n;++j)
        count+=SCROS0[n*y+i][n*x+j];
    // assert(c==BU);
    // SCR[y][x]=c*count/N;
    SCR[y][x]=( ((count*(c&0x000000FF)+(N-count)*(bg&0x000000FF))/N) & 0x000000FF )|
              ( ((count*(c&0x0000FF00)+(N-count)*(bg&0x0000FF00))/N) & 0x0000FF00 )|
              ( ((count*(c&0x00FF0000)+(N-count)*(bg&0x00FF0000))/N) & 0x00FF0000 );
  }

  #undef SCROS0
  free(scros0);scros0=NULL;

}

// Horizontal rule
static inline void hr(const Color c,const __u32 y){
  assert(y<YRES); // SCR[768][*] => 100% segfault
  for(size_t x=0;x<=XRES;++x)
    SCR[y][x]=c;
}

static inline void demo4_hstripe(const Color c0,const Color c1,const __u32 width){
  for(__u32 y=2*width;y<YRES;y+=2*width){
    for(__u32 i=0;i<width;++i)           hr(c1,y-i);
    for(__u32 i=width;i<width+width;++i) hr(c0,y-i);
    // for(__u32 x=0;x<XRES;++x){
    //   for(__u32 i=0;i<width;++i)
    //     SCR[y-i][x]=c1;
    //   for(__u32 i=width;i<width+width;++i)
    //     SCR[y-i][x]=c0;
    // }
  }
}

// ioctl FBIOPUT_VSCREENINFO
// EINVAL 22 Invalid argument
/*static inline void grayscale(){
  struct fb_var_screeninfo fvs2={};
  assert(0==ioctl(fbfd,FBIOGET_VSCREENINFO,&fvs2));
  // struct fb_var_screeninfo fvs2=fvs;
  fvs2.bits_per_pixel=8;
  fvs2.grayscale=1;
  // fvs2.red=(struct fb_bitfield){4,3,0};
  // fvs2.green=(struct fb_bitfield){0,0,0};
  // fvs2.blue=(struct fb_bitfield){0,0,0};
  // fvs2.transp=(struct fb_bitfield){0,0,0}
  explicit_bzero(&fvs2.red,sizeof(struct fb_bitfield));
  explicit_bzero(&fvs2.green,sizeof(struct fb_bitfield));
  explicit_bzero(&fvs2.blue,sizeof(struct fb_bitfield));
  explicit_bzero(&fvs2.transp,sizeof(struct fb_bitfield));
  // assert(-1!=ioctl(fbfd,FBIOPUT_VSCREENINFO,&fvs2));
  if(-1==ioctl(fbfd,FBIOPUT_VSCREENINFO,&fvs2)){
    printf("errno %d\n",errno);
    abort();
  }
  assert(0==ioctl(fbfd,FBIOGET_VSCREENINFO,&fvs2));
  printf("%u\n",fvs2.bits_per_pixel);
  assert(0==ioctl(fbfd,FBIOPUT_VSCREENINFO,&fvs));
}*/

int main(){

  srand(time(NULL));

  fbfd=open("/dev/fb0",O_RDWR);
  assert(fbfd>=3);

  f_f_s();
  f_v_s();

  scr0=mmap(NULL,
           smem_len,
           PROT_READ|PROT_WRITE,
           MAP_SHARED,
           fbfd,
           0);
  assert(scr0&&scr0!=MAP_FAILED);
  initscr();
  assert(1==curs_set(0));

  // fill(BK);
  // demo0_rgb();
  // getchar();

  // fill(BK);
  // demo1_noise();
  // getchar();

  const Color bg=BU;
  const Color fg=GN;
  // fill(bg);
  // // demo2_sector(RAND_COLOR,YRES);
  // demo2_sector(fg,YRES);
  // getchar();

  fill(bg);
  demo3_sector_msaaNx(fg,YRES,4,bg);
  // demo3_sector_msaaNx(fg,YRES,16,bg);
  // demo3_sector_msaaNx(fg,YRES,121,bg);
  getchar();

  // fill(BK);demo4_hstripe(RD,GN,5);getchar();
  // fill(BK);demo4_hstripe(RD,WH,1);getchar();
  // fill(BK);demo4_hstripe(GN,WH,1);getchar();
  // fill(BK);demo4_hstripe(BU,WH,1);getchar();
  // fill(BK);demo4_hstripe(RD,BK,1);getchar();
  // fill(BK);demo4_hstripe(GN,BK,1);getchar();
  // fill(BK);demo4_hstripe(BU,BK,1);getchar();

  // fill(BK);
  // hr(RD,0);
  // hr(GN,1);
  // // hr(BU,2);
  // // hr(RD,3);
  // hr(GN,384);
  // hr(GN,767);
  // getchar();

  assert(0==curs_set(1));
  assert(OK==endwin());
  munmap(scr0,smem_len);scr0=NULL;

  close(fbfd);

  // for(size_t i=0;i<vinfo.xres*vinfo.yres;++i)
  //   buf[i]=0x00FFFF00;

  // for(size_t i=0;i<500;++i){
  //   buf[]=0x00FFFF00;
  //   buf[10*vinfo.xres+i]=0x00FFFF00;
  // }

  // // for(uint32_t c=0;c<100;++c)
  // //   if(c%2)
  // //     buf[100*vinfo.xres+c] = 0x00FFFFFF;


  // #define buf(r,c,color) buf[r*vinfo.xres+c]=color & 0x00FFFFFF
  // #define vline(c,color,rBeg,rEnd) {for(uint32_t r=rBeg;r<=rEnd;++r)buf(r,c,color);}
  // #define hline(r,color,cBeg,cEnd) {for(uint32_t c=cBeg;c<=cEnd;++c)buf(r,c,color);}
  // #define area(color,rBeg,rEnd,cBeg,cEnd) {for(uint32_t r=rBeg;r<=rEnd;++r)for(uint32_t c=cBeg;c<=cEnd;++c)buf(r,c,color);}

  // // vline(10,0xFFFFFF,10,100);

  // const uint32_t rBeg=10;
  // const uint32_t rEnd=110;
  // uint32_t cBeg=0;
  // uint32_t cEnd=0;
  // #define next {cBeg=cEnd+10;cEnd=cBeg+100;}

  // next;area(0xFFFFFF,rBeg,rEnd,cBeg,cEnd);

  // // next;area(0x00FFFF,rBeg,rEnd,cBeg,cEnd);
  // // next;area(0xFF00FF,rBeg,rEnd,cBeg,cEnd);
  // // next;area(0xFFFF00,rBeg,rEnd,cBeg,cEnd);

  // // next;area(0xFF0000,rBeg,rEnd,cBeg,cEnd);
  // // next;area(0x00FF00,rBeg,rEnd,cBeg,cEnd);
  // // next;area(0x0000FF,rBeg,rEnd,cBeg,cEnd);

  return 0;
}
