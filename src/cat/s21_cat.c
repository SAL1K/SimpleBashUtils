#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const struct option lONG_OPTIONS[] = {
    {"number-nonblank", 0, NULL, 'b'},  {"number", 0, NULL, 'n'},
    {"show-ends", 0, NULL, 'e'},        {"show-tabs", 0, NULL, 't'},
    {"show-nonprinting", 0, NULL, 'v'}, {"squeeze-blank", 0, NULL, 's'}};

struct flag {
  int b;
  int e;
  int s;
  int n;
  int t;
  int v;
  int T;
  int E;
};

struct flag check_flags(int argc, char **argv);
void print_file(char *argv, struct flag catflag);
void print_file(char *argv, struct flag catflag) {
  int c, prec, empty_str, num_str;
  empty_str = num_str = 0;
  FILE *fp;
  if ((fp = fopen(argv, "r")) == NULL) {
    printf("Can't open file\n");
    exit(EXIT_FAILURE);
  }
  for (prec = '\n'; (c = getc(fp)) != EOF; prec = c) {
    if (prec == '\n') {
      if (catflag.s) {
        if (c == '\n') {
          if (empty_str) continue;
          empty_str = 1;
        } else
          empty_str = 0;
      }
      if (catflag.n && (!catflag.b || c != '\n')) {
        fprintf(stdout, "%6d\t", ++num_str);
        if (ferror(stdout)) break;
      }
    }
    if (c == '\n' && catflag.e) {
      printf("$");
    } else if (c == '\t') {
      if (catflag.t) {
        printf("^");
        printf("I");
        continue;
      }

    } else if (catflag.v && !catflag.T && !catflag.E) {
      if (!isascii(c) && !isprint(c) && !(c > 160 && c < 256)) {
        printf("M-");
        if (c == 138) {
          printf("^J");
          continue;
        }
        c = toascii(c);
      }
      if (iscntrl(c) && c != '\n') {
        printf("^");
        printf("%c", c == '\177' ? '?' : c | 0100);
        continue;
      }
    }
    printf("%c", c);
  }
  fclose(fp);
}

struct flag check_flags(int argc, char **argv) {
  struct flag catflag = {0};
  int opt;
  int long_options_index = 0;
  while ((opt = getopt_long(argc, argv, "+besntETv", lONG_OPTIONS,
                            &long_options_index)) != -1) {
    switch (opt) {
      case 'b':
        catflag.b = catflag.n = 1;
        break;
      case 'e':
        catflag.e = catflag.v = 1;
        break;
      case 's':
        catflag.s++;
        break;
      case 'n':
        catflag.n++;
        break;
      case 't':
        catflag.t = catflag.v = 1;
        break;
      case 'v':
        catflag.v++;
        break;
      case 'E':
        catflag.E++;
        break;
      case 'T':
        catflag.T++;
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }
  return catflag;
}

int main(int argc, char **argv) {
  struct flag catflag = check_flags(argc, argv);
  for (int i = 1; i < argc; i++) {
    if (*argv[i] == '-') {
      continue;
    }
    print_file(argv[i], catflag);
  }
  return 1;
}