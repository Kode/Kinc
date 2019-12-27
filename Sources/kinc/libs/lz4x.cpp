/*

LZ4X - An optimized LZ4 compressor

Written and placed in the public domain by Ilya Muravyov

*/

#ifndef _MSC_VER
#  define _FILE_OFFSET_BITS 64

#  define _fseeki64 fseeko
#  define _ftelli64 ftello
#  define _stati64 stat

#  define __min(a, b) ((a)<(b)?(a):(b))
#  define __max(a, b) ((a)>(b)?(a):(b))
#endif

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
//#define _CRT_SECURE_NO_WARNINGS
#define _CRT_DISABLE_PERFCRIT_LOCKS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef NO_UTIME
#  include <sys/types.h>
#  include <sys/stat.h>

#  ifdef _MSC_VER
#    include <sys/utime.h>
#  else
#    include <utime.h>
#  endif
#endif

typedef unsigned char byte;
typedef unsigned int uint;

#define LZ4_MAGIC 0x184C2102
#define BLOCK_SIZE (8<<20) // 8 MB
#define PADDING_LITERALS 8

#define WLOG 16
#define WSIZE (1<<WLOG)
#define WMASK (WSIZE-1)

#define MIN_MATCH 4

#define COMPRESS_BOUND (16+BLOCK_SIZE+(BLOCK_SIZE/255))

//byte buf[BLOCK_SIZE+COMPRESS_BOUND];

#define HASH_LOG 18
#define HASH_SIZE (1<<HASH_LOG)
#define NIL (-1)

#if 0
#ifdef FORCE_UNALIGNED
#  define load32(p) (*((const uint*)&buf[p]))
#else
  inline uint load32(int p)
  {
    uint x;
    memcpy(&x, &buf[p], sizeof(uint));
    return x;
  }
#endif
#endif
#define hash32(p) ((load32(p)*0x125A517D)>>(32-HASH_LOG))
#define copy128(p, s) memcpy(&buf[p], &buf[s], 16)

#define get_byte() buf[BLOCK_SIZE+(bp++)]
#define put_byte(c) (buf[BLOCK_SIZE+(bsize++)]=(c))

#if 0
FILE* fin;
FILE* fout;

void compress(const int max_chain)
{
  static int head[HASH_SIZE];
  static int tail[WSIZE];

#ifdef LZ4_MAGIC
  const uint magic=LZ4_MAGIC;
  fwrite(&magic, 1, sizeof(magic), fout);
#endif

  _fseeki64(fin, 0, SEEK_END);
  const long long flen=_ftelli64(fin);
  _fseeki64(fin, 0, SEEK_SET);

  int n;
  while ((n=fread(buf, 1, BLOCK_SIZE, fin))>0)
  {
    for (int i=0; i<HASH_SIZE; ++i)
      head[i]=NIL;

    int bsize=0;
    int pp=0;

    int p=0;
    while (p<n)
    {
      int best_len=0;
      int dist;

      const int max_match=(n-PADDING_LITERALS)-p;
      if (max_match>=MIN_MATCH)
      {
        int chain_len=max_chain;
        const int wstart=__max(p-WSIZE, NIL);

        int s=head[hash32(p)];
        while (s>wstart)
        {
          if (buf[s+best_len]==buf[p+best_len] && load32(s)==load32(p))
          {
            int len=MIN_MATCH;
            while (len<max_match && buf[s+len]==buf[p+len])
              ++len;

            if (len>best_len)
            {
              best_len=len;
              dist=p-s;

              if (len==max_match)
                break;
            }
          }

          if (!--chain_len)
            break;

          s=tail[s&WMASK];
        }
      }

      if (best_len>=MIN_MATCH)
      {
        int len=best_len-MIN_MATCH;
        const int ml=__min(len, 15);

        if (pp<p)
        {
          int run=p-pp;
          if (run>=15)
          {
            put_byte((15<<4)+ml);

            run-=15;
            for (; run>=255; run-=255)
              put_byte(255);
            put_byte(run);
          }
          else
            put_byte((run<<4)+ml);

          while (pp<p)
            put_byte(buf[pp++]);
        }
        else
          put_byte(ml);

        put_byte(dist);
        put_byte(dist>>8);

        if (len>=15)
        {
          len-=15;
          for (; len>=255; len-=255)
            put_byte(255);
          put_byte(len);
        }

        pp=p+best_len;

        while (p<pp)
        {
          const uint h=hash32(p);
          tail[p&WMASK]=head[h];
          head[h]=p++;
        }
      }
      else
      {
        const uint h=hash32(p);
        tail[p&WMASK]=head[h];
        head[h]=p++;
      }
    }

    if (pp<p)
    {
      int run=p-pp;
      if (run>=15)
      {
        put_byte(15<<4);

        run-=15;
        for (; run>=255; run-=255)
          put_byte(255);
        put_byte(run);
      }
      else
        put_byte(run<<4);

      while (pp<p)
        put_byte(buf[pp++]);
    }

    fwrite(&bsize, 1, sizeof(bsize), fout);
    fwrite(&buf[BLOCK_SIZE], 1, bsize, fout);

    if (flen>0)
      fprintf(stderr, "%3d%%\r", int((_ftelli64(fin)*100)/flen));
  }
}

void compress_optimal()
{
  static int head[HASH_SIZE];
  static int nodes[WSIZE][2];
  static struct
  {
    int cum;

    int len;
    int dist;
  } path[BLOCK_SIZE+1];

#ifdef LZ4_MAGIC
  const uint magic=LZ4_MAGIC;
  fwrite(&magic, 1, sizeof(magic), fout);
#endif

  _fseeki64(fin, 0, SEEK_END);
  const long long flen=_ftelli64(fin);
  _fseeki64(fin, 0, SEEK_SET);

  int n;
  while ((n=fread(buf, 1, BLOCK_SIZE, fin))>0)
  {
    // Pass 1: Find all matches

    for (int i=0; i<HASH_SIZE; ++i)
      head[i]=NIL;

    for (int p=0; p<n; ++p)
    {
      int best_len=0;
      int dist;

      const int max_match=__min(1<<14, (n-PADDING_LITERALS)-p); // [!]
      if (max_match>=MIN_MATCH)
      {
        int* left=&nodes[p&WMASK][1];
        int* right=&nodes[p&WMASK][0];

        int left_len=0;
        int right_len=0;

        const int wstart=__max(p-WSIZE, NIL);

        const uint h=hash32(p);
        int s=head[h];
        head[h]=p;

        while (s>wstart)
        {
          int len=__min(left_len, right_len);

          if (buf[s+len]==buf[p+len])
          {
            while (++len<max_match && buf[s+len]==buf[p+len]);

            if (len>best_len)
            {
              best_len=len;
              dist=p-s;

              if (len==max_match)
                break;
            }
          }

          if (buf[s+len]<buf[p+len])
          {
            *right=s;
            right=&nodes[s&WMASK][1];
            s=*right;
            right_len=len;
          }
          else
          {
            *left=s;
            left=&nodes[s&WMASK][0];
            s=*left;
            left_len=len;
          }
        }

        *left=NIL;
        *right=NIL;
      }

      path[p].len=best_len;
      path[p].dist=dist;
    }

    // Pass 2: Build the shortest path

    path[n].cum=0;

    int cnt=15;

    for (int p=n-1; p>0; --p)
    {
      int c0=path[p+1].cum+1;

      if (!--cnt)
      {
        cnt=255;
        ++c0;
      }

      if (path[p].len>=MIN_MATCH)
      {
        path[p].cum=1<<30;

        for (int i=path[p].len; i>=MIN_MATCH; --i)
        {
          int c1=path[p+i].cum+3;

          int len=i-MIN_MATCH;
          if (len>=15)
          {
            len-=15;
            for (; len>=255; len-=255)
              ++c1;
            ++c1;
          }

          if (c1<path[p].cum)
          {
            path[p].cum=c1;
            path[p].len=i;
          }
        }

        if (c0<path[p].cum)
        {
          path[p].cum=c0;
          path[p].len=0;
        }
        else
          cnt=15;
      }
      else
        path[p].cum=c0;
    }

    // Pass 3: Output the codes

    int bsize=0;
    int pp=0;

    int p=0;
    while (p<n)
    {
      if (path[p].len>=MIN_MATCH)
      {
        int len=path[p].len-MIN_MATCH;
        const int ml=__min(len, 15);

        if (pp<p)
        {
          int run=p-pp;
          if (run>=15)
          {
            put_byte((15<<4)+ml);

            run-=15;
            for (; run>=255; run-=255)
              put_byte(255);
            put_byte(run);
          }
          else
            put_byte((run<<4)+ml);

          while (pp<p)
            put_byte(buf[pp++]);
        }
        else
          put_byte(ml);

        put_byte(path[p].dist);
        put_byte(path[p].dist>>8);

        if (len>=15)
        {
          len-=15;
          for (; len>=255; len-=255)
            put_byte(255);
          put_byte(len);
        }

        p+=path[p].len;

        pp=p;
      }
      else
        ++p;
    }

    if (pp<p)
    {
      int run=p-pp;
      if (run>=15)
      {
        put_byte(15<<4);

        run-=15;
        for (; run>=255; run-=255)
          put_byte(255);
        put_byte(run);
      }
      else
        put_byte(run<<4);

      while (pp<p)
        put_byte(buf[pp++]);
    }

    fwrite(&bsize, 1, sizeof(bsize), fout);
    fwrite(&buf[BLOCK_SIZE], 1, bsize, fout);

    if (flen>0)
      fprintf(stderr, "%3d%%\r", int((_ftelli64(fin)*100)/flen));
  }
}
#endif

#include <Kore/Math/Core.h>

int kread(void* dst, size_t size, const char* src, uint* offset, size_t compressedSize) {
	size_t realSize = Kore::min(size, compressedSize - *offset);
	memcpy(dst, &src[*offset], realSize);
	*offset += realSize;
	return realSize;
}

//int decompress()
#ifdef KORE_LZ4X
extern "C" int LZ4_decompress_safe(const char *source, char *buf, int compressedSize, int maxOutputSize)
{
  uint offset = 0;
  int bsize;
  while (kread(&bsize, sizeof(bsize), source, &offset, compressedSize) > 0)
  {
    if (bsize<0 || bsize>COMPRESS_BOUND
        || kread(&buf[BLOCK_SIZE], bsize, source, &offset, compressedSize)!=bsize)
      return 1;

    int p=0;

    int bp=0;
    while (bp<bsize)
    {
      const int tag=get_byte();
      if (tag>=16)
      {
        int run=tag>>4;
        if (run==15)
        {
          for (;;)
          {
            const int c=get_byte();
            run+=c;
            if (c!=255)
              break;
          }

          for (int i=0; i<run; i+=16)
            copy128(p+i, BLOCK_SIZE+bp+i);
        }
        else
          copy128(p, BLOCK_SIZE+bp);

        p+=run;
        bp+=run;

        if (bp>=bsize)
          break;
      }

      int s=p-get_byte();
      s-=get_byte()<<8;

      int len=tag&15;
      if (len==15)
      {
        for (;;)
        {
          const int c=get_byte();
          len+=c;
          if (c!=255)
            break;
        }
      }
      len+=4;

      if ((p-s)>=16)
      {
        for (int i=0; i<len; i+=16)
          copy128(p+i, s+i);
        p+=len;
      }
      else
      {
        while (len--)
          buf[p++]=buf[s++];
      }
    }

    if (bp!=bsize)
      return 1;

    //fwrite(buf, 1, p, fout);
  }

  return 0;
}
#endif

#if 0
int lz4x_main(int argc, char** argv)
{
  const clock_t start=clock();

  int level=5;
  bool do_decomp=false;
  bool overwrite=false;

  while (argc>1 && *argv[1]=='-')
  {
    for (int i=1; argv[1][i]; ++i)
    {
      switch (argv[1][i])
      {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        level=argv[1][i]-'0';
        break;
      case 'd':
        do_decomp=true;
        break;
      case 'f':
        overwrite=true;
        break;
      default:
        fprintf(stderr, "Unknown option: -%c\n", argv[1][i]);
        exit(1);
      }
    }
    --argc;
    ++argv;
  }

  if (argc<2)
  {
    fprintf(stderr,
        "LZ4X - An optimized LZ4 compressor, v1.30\n"
        "\n"
        "Usage: %s [options] infile [outfile]\n"
        "\n"
        "Options:\n"
        "  -1 Compress faster\n"
        "  -9 Compress better\n"
        "  -d Decompress\n"
        "  -f Force overwrite of output file\n", argv[0]);
    exit(1);
  }

  fin=fopen(argv[1], "rb");
  if (!fin)
  {
    perror(argv[1]);
    exit(1);
  }

  char ofname[FILENAME_MAX];
  if (argc<3)
  {
    strcpy(ofname, argv[1]);
    if (do_decomp)
    {
      const int p=strlen(ofname)-4;
      if (p>0 && !strcmp(&ofname[p], ".lz4"))
        ofname[p]='\0';
      else
        strcat(ofname, ".out");
    }
    else
      strcat(ofname, ".lz4");
  }
  else
    strcpy(ofname, argv[2]);

  if (!overwrite)
  {
    FILE* f=fopen(ofname, "rb");
    if (f)
    {
      fclose(f);

      fprintf(stderr, "%s already exists. Overwrite (y/n)? ", ofname);
      fflush(stderr);

      if (getchar()!='y')
        exit(1);
    }
  }

  fout=fopen(ofname, "wb");
  if (!fout)
  {
    perror(ofname);
    exit(1);
  }

  if (do_decomp)
  {
    fprintf(stderr, "Decompressing %s:\n", argv[1]);

    switch (decompress())
    {
    case 1:
      fprintf(stderr, "File corrupted!\n");
      exit(1);
#ifdef LZ4_MAGIC
    case 2:
      fprintf(stderr, "Not in Legacy format!\n");
      exit(1);
    }
#endif
  }
  else
  {
    fprintf(stderr, "Compressing %s:\n", argv[1]);

    if (level==9)
      compress_optimal();
    else
      compress(level==8?WSIZE:1<<level);
  }

  fprintf(stderr, "%lld -> %lld in %1.2fs\n", _ftelli64(fin), _ftelli64(fout),
      double(clock()-start)/CLOCKS_PER_SEC);

  fclose(fin);
  fclose(fout);

#ifndef NO_UTIME
  struct _stati64 sb;
  if (_stati64(argv[1], &sb))
  {
    perror("Stat failed");
    exit(1);
  }
  struct utimbuf ub;
  ub.actime=sb.st_atime;
  ub.modtime=sb.st_mtime;
  if (utime(ofname, &ub))
  {
    perror("Utime failed");
    exit(1);
  }
#endif

  return 0;
}
#endif
