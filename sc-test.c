#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


static void
flush_stream(FILE *strm) {
  while (getc(strm) != EOF) { }
}

static void
croak(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  flush_stream(stderr);
}

static void
usage(const char *program) {
  fprintf(stderr, "\nExecute code : %s -e <file-containing-shellcode>\n", program);
  fprintf(stderr, "Convert code : %s -p <file-containing-shellcode>\n\n", program);
  flush_stream(stderr);
  exit(EXIT_FAILURE);
}

static void
barf(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int
main(int argc, char **argv) {
  FILE *fp;
  void *code;
  int arg, i, l, m = 15 /* max # of bytes to print on one line */;

  struct stat sbuf;
  long flen;
  void (*fptr)(void);

  memset(&sbuf, '\0', sizeof(struct stat));
  if (argc < 3) usage(argv[0]);
  if (stat(argv[2], &sbuf)) barf("failed to stat file");
  flen = (long) sbuf.st_size;
  if (!(code = calloc(1, flen))) barf("failed to grab memory");
  if (!(fp = fopen(argv[2], "rb"))) barf("failed to open file");
  if (fread(code, 1, flen, fp) != flen) barf("failed to slurp file");
  if (fclose(fp)) barf("failed to close file");

  while ((arg = getopt(argc, argv, "e:p:")) != -1) {
    switch (arg) {
      case 'e':
        croak("Calling code ...");
        fptr = (void (*)(void)) code;
        (*fptr)();
        break;
      case 'p':
        printf("\n/* The following shellcode is %ld bytes long */\n", flen);
        printf("\nchar shellcode[] =\n");
        l = m;
        for (i = 0; i < flen; i++) {
          if (l >= m) {
            if (i) printf("\"\n");
            printf("\t\"");
            l = 0;
          }
          ++l;
          printf("\\x%02x", ((unsigned char *)code)[i]);
        }
        printf("\";\n\n\n");
        break;
      default:
        usage(argv[0]);
        break;
    }
  }
  return (EXIT_SUCCESS);
}