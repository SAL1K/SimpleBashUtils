#define _GNU_SOURCE
#define BUFFSIZE 8192
#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
  int f;
  int o;
  long long int string_count;
  int match_lines_count;
  int match_files_count;
  int empty_lines;
  int file_s;
  int e_count;
  char search_expres[BUFFSIZE];
} flagss;

void check_flags(int argc, char **argv, flagss *flag);
void open_file(int argc, char **argv, flagss *flag);
void flag_e(flagss *flag);
void flag_f(char *argv[], flagss *flag);
void search_and_print(char *argv[], FILE *file, flagss *flag, int optindi);
void is_it_files(int argc, char **argv, flagss *flag);
extern int optind, opterr, optopt;

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf(
        "usage: grep [-abcdDEFGHhIiJLlMmnOopqRSsUVvwXxZz] [-A num] [-B num] "
        "[-C[num]]\n\t[-e pattern] [-f file] [--binary-files=value] "
        "[--color=when]\n\t[--context[=num]] [--directories=action] [--label] "
        "[--line-buffered]\n\t[--null] [pattern] [file ...]\n");
    exit(EXIT_FAILURE);
  }
  static flagss flag = {0};
  check_flags(argc, argv, &flag);
  open_file(argc, argv, &flag);
  return 0;
}

struct option flags[] = {{"e", no_argument, 0, 'e'},  // -b
                         {"ignore-case", no_argument, 0, 'i'},
                         {"invert-match", no_argument, 0, 'v'},
                         {"count", no_argument, 0, 'c'},
                         {"files-with-matches", no_argument, 0, 'l'},
                         {"line-number", no_argument, 0, 'n'},
                         {"no-filename", no_argument, 0, 't'},
                         {"s", no_argument, 0, 's'},
                         {"f", no_argument, 0, 'f'},
                         {"o", no_argument, 0, 'o'},
                         {0, 0, 0, 0}};

void check_flags(int argc, char **argv, flagss *flag) {
  int res = 0;
  opterr = 0;
  while ((res = getopt_long(argc, argv, "e:ivclnhsf:o", flags, 0)) != -1) {
    switch (res) {
      case 'e':
        flag->e = 1;
        flag_e(flag);
        break;
      case 'i':
        flag->i = 1;
        break;
      case 'v':
        flag->v = 1;
        break;
      case 'c':
        flag->c = 1;
        break;
      case 'l':
        flag->l = 1;
        break;
      case 'n':
        flag->n = 1;
        break;
      case 'h':
        flag->h = 1;
        break;
      case 's':
        flag->s = 1;
        break;
      case 'f':
        flag->f = 1;
        flag_f(argv, flag);
        break;
      case 'o':
        flag->o = 1;
        break;
      case '?':
        fprintf(stderr, "%s: invalid option -- %s\n", argv[0] + 2,
                ((argv[optind - 1]) + 1));
        fprintf(stderr,
                "usage: %s [-eivclnhsfo] [file ...] [-e pattern] [-f file] \n",
                argv[0] + 2);
        exit(1);
    }
  }
}

void flag_e(flagss *flag) {
  if (flag->e_count > 1) strcat(flag->search_expres, "|");
  strcat(flag->search_expres, optarg);
  flag->e_count++;
}

void flag_f(char *argv[], flagss *flag) {
  FILE *fd;
  fd = fopen(optarg, "r");
  if (fd == NULL) {
    fprintf(stderr, "%s: %s: No such file or directory\n", argv[0] + 2,
            argv[optind - 1]);
    exit(EXIT_FAILURE);
  }
  static char buffer[BUFFSIZE] = {0};

  fseek(fd, 0, SEEK_SET);
  while (1) {
    char *estr;
    estr = fgets(buffer, BUFFSIZE, fd);
    if (estr == NULL) {
      if (feof(fd) != 0) {
        break;
      } else {
        fprintf(stderr, "%s: %s: No such file or directory\n", argv[0] + 2,
                argv[optind - 1]);
        exit(EXIT_FAILURE);
      }
    }
    if (buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = 0;
    if (flag->e_count > 0) strcat(flag->search_expres, "|");
    if (*buffer == '\0') {
      flag->empty_lines = 1;
      strcat(flag->search_expres, ".");
    } else {
      strcat(flag->search_expres, buffer);
    }
    ++flag->e_count;
  }
  fclose(fd);
}

int is_it_flag(char **argv, int i) {
  int e = 0;
  if (argv[i] != NULL) {
    if (argv[i][0] == '-') {
      e = 1;
      if ((argv[i][strlen(argv[i]) - 1] == 'e') ||
          (argv[i][strlen(argv[i]) - 1] == 'f'))
        e = 2;
    }
  }
  return e;
}

void is_it_files(int argc, char **argv, flagss *flag) {
  int fp = 0;
  int ii = 1;

  while (ii < argc) {
    int f;
    f = is_it_flag(argv, ii);
    if (f == 0 && fp != 2) {
      if ((argc - ii - 1) > 1) {
        flag->file_s++;
      }
      break;
    }
    fp = f;
    ii++;
  }
}

void open_file(int argc, char **argv, flagss *flag) {
  FILE *fd;
  is_it_files(argc, argv, flag);

  for (int i = 1; i < argc; i++) {
    int f = 0;
    f = is_it_flag(argv, i);
    if (f == 2) {
      i++;
      continue;
    }
    if (!(flag->f || flag->e) && !f && !flag->e_count) {
      strcat(flag->search_expres, argv[i] ? argv[i] : "");
      flag->e_count++;
      optind++;
      continue;
    }
    if (f == 0) {
      fd = fopen(argv[i], "r");

      if (fd == NULL) {
        if (flag->s != 1) {
          fprintf(stderr, "%s: %s: No such file or directory\n", argv[0] + 2,
                  argv[i]);
        }
        continue;
      }
      search_and_print(argv, fd, flag, i);
      fclose(fd);
    }
  }
}

void search_and_print(char *argv[], FILE *fd, flagss *flag, int optindi) {
  regex_t rx;
  int value;
  int cflags = REG_EXTENDED;
  if (flag->i) cflags |= REG_ICASE;
  regmatch_t regmatch[1] = {0};
  int nmatch = 1;
  value = regcomp(&rx, flag->search_expres, cflags);
  static char str_var[BUFFSIZE] = {0};
  if (value != 0) {
    exit(EXIT_FAILURE);
  }
  flag->match_lines_count = 0;
  flag->match_files_count = 0;
  flag->string_count = 0;

  while (1) {
    char *estr;
    estr = fgets(str_var, BUFFSIZE, fd);
    if (estr == NULL) {
      if (feof(fd) != 0) {
        break;
      } else {
        if (flag->s != 1)
          fprintf(stderr, "%s: %s: Error of read\n", argv[0] + 2,
                  argv[optindi]);
      }
      break;
    }
    flag->string_count++;
    char *ptr = str_var;
    value = regexec(&rx, ptr, nmatch, regmatch, 0);
    if ((value == 0 && flag->v == 0) ||
        (value == REG_NOMATCH && flag->v == 1)) {
      if (!flag->c && !flag->l) {
        if (flag->file_s && !flag->h && !(flag->o && flag->empty_lines))
          printf("%s:", argv[optindi]);
        if (flag->n) printf("%lld:", flag->string_count);

        if (flag->o) {
          for (int i = 0; !value && regmatch[0].rm_eo != regmatch[0].rm_so;
               i++) {
            if (!flag->empty_lines)
              printf("%.*s\n", (int)(regmatch[0].rm_eo - regmatch[0].rm_so),
                     ptr + regmatch[0].rm_so);
            ptr += regmatch[0].rm_eo;
            value = regexec(&rx, ptr, nmatch, regmatch, REG_NOTBOL);
          }
        }
        if (!flag->o || (flag->v && (flag->o == 1))) {
          printf("%s", str_var);
          if (str_var[strlen(str_var) - 1] != '\n') printf("\n");
        }
      } else {
        flag->match_lines_count++;
        flag->match_files_count++;
      }
    }
  }
  if (flag->file_s && !flag->h && flag->c)
    printf("%s:", argv[optindi]);
  else if (flag->file_s && !flag->h && flag->c && flag->match_files_count)
    printf("%s:", argv[optindi]);
  if (flag->c && flag->l && flag->match_lines_count)
    flag->match_lines_count = 1;
  if (flag->c && !flag->v) printf("%d\n", flag->match_lines_count);
  if (flag->c && flag->v) printf("%d\n", flag->match_lines_count);
  if (flag->l && flag->match_files_count && !flag->v && !flag->h)
    printf("%s\n", argv[optindi]);
  else if (flag->l && flag->v && flag->match_files_count && !flag->h)
    printf("%s\n", argv[optindi]);
  else if (flag->l && flag->match_files_count && !flag->v && flag->h &&
           !flag->c)
    printf("%s\n", argv[optindi]);
  regfree(&rx);
}
