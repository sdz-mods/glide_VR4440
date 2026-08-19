#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_EXPORTS 512

static char seenExports[MAX_EXPORTS][128];
static int seenCount = 0;

static void rstrip(char* text)
{
  size_t len = strlen(text);

  while ((len > 0) &&
         ((text[len - 1] == '\n') || (text[len - 1] == '\r') ||
          isspace((unsigned char)text[len - 1]))) {
    text[--len] = '\0';
  }
}

static int isSeen(const char* name)
{
  int i;

  for (i = 0; i < seenCount; i++) {
    if (strcmp(seenExports[i], name) == 0) return 1;
  }

  return 0;
}

static void rememberExport(const char* name)
{
  if (seenCount >= MAX_EXPORTS) return;

  strncpy(seenExports[seenCount], name, sizeof(seenExports[seenCount]) - 1);
  seenExports[seenCount][sizeof(seenExports[seenCount]) - 1] = '\0';
  seenCount++;
}

static void writeAliasExport(const char* exportName, const char* internalName, const char* suffix)
{
  if ((exportName == NULL) || (*exportName == '\0')) return;
  if ((internalName == NULL) || (*internalName == '\0')) return;

  printf("    %s=%s", exportName, internalName);
  if ((suffix != NULL) && (*suffix != '\0')) printf(" %s", suffix);
  printf("\n");
}

int main(int argc, char** argv)
{
  FILE* inFile = stdin;
  char line[MAX_LINE];

  if (argc > 2) {
    fprintf(stderr, "usage: %s [input.def]\n", argv[0]);
    return 1;
  }

  if (argc == 2) {
    inFile = fopen(argv[1], "r");
    if (inFile == NULL) {
      perror(argv[1]);
      return 1;
    }
  }

  puts("EXPORTS");

  while (fgets(line, sizeof(line), inFile) != NULL) {
    char* name;
    char* suffix;
    char* cursor = line;

    while (isspace((unsigned char)*cursor)) cursor++;
    if ((*cursor == '\0') || (*cursor == ';')) continue;
    if (strncmp(cursor, "EXPORTS", 7) == 0) continue;

    rstrip(cursor);
    if (*cursor == '\0') continue;

    name = cursor;
    while ((*cursor != '\0') && !isspace((unsigned char)*cursor)) cursor++;

    if (*cursor != '\0') {
      *cursor++ = '\0';
      while (isspace((unsigned char)*cursor)) cursor++;
      suffix = cursor;
    } else {
      suffix = "";
    }

    if (!isSeen(name)) rememberExport(name);

    printf("    %s", name);
    if ((suffix != NULL) && (*suffix != '\0')) printf(" %s", suffix);
    printf("\n");

    {
      char compatName[160];

      snprintf(compatName, sizeof(compatName), "_%s", name);
      writeAliasExport(compatName, name, "");
    }
  }

  if (inFile != stdin) fclose(inFile);

  return 0;
}
