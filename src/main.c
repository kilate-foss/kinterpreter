#include <stdio.h>
#include <string.h>

#include "kilate/config.h"
#include "kilate/error.h"
#include "kilate/file.h"
#include "kilate/interpreter.h"
#include "kilate/lexer.h"
#include "kilate/native.h"
#include "kilate/parser.h"
#include "kilate/string.h"
#include "kilate/vector.h"

bool interpret(char *src) {
  lexer_t* lexer = lexer_make(src);
  if (lexer == NULL)
    error_fatal("Lexer is null.");
  lexer_tokenize(lexer);

  native_init();

  parser_t* parser = parser_make(lexer->tokens);
  if (parser == NULL)
    error_fatal("Parser is null.");

  parser_parse_program(parser);

  interpreter_t* interpreter =
      interpreter_make(parser->nodes, native_functions);
  if (interpreter == NULL)
    error_fatal("Interpreter is null.");
  interpreter_run(interpreter);

  parser_delete(parser);
  interpreter_delete(interpreter);
  lexer_delete(lexer);
  native_end();
  return true;
}

bool run(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Invalid usage. Use '%s help' for more info.\n", argv[0]);
    return false;
  }

  if (str_equals(argv[1], "help")) {
    printf("Usage:\n");
    printf("  %s run <file(s)> [-I<path>] [-l<lib>]\n", argv[0]);
    printf("Options:\n");
    printf("  -L<path>    Kilate Libraries path\n");
    printf("  -LN<path>   Kilate Native Libraries path\n");
    return true;
  }

  if (!str_equals(argv[1], "run")) {
    printf("Unknown command: %s\n", argv[1]);
    return false;
  }

  if (argc < 3) {
    printf("Usage: %s run <file(s)> [-I<path>] [-l<lib>]\n", argv[0]);
    return false;
  }
  
  // Config
  {
    const char *PREFIX = getenv("PREFIX");
    if (PREFIX != NULL) {
      char path[512];
      snprintf(path, sizeof(path), "%s/kilate/native_libs/", PREFIX);
      char *dup = strdup(path);
      vector_push_back(libs_native_directories, &dup);

      memset(path, 0, sizeof(path));
      snprintf(path, sizeof(path), "%s/kilate/libs/", PREFIX);
      dup = strdup(path);
      vector_push_back(libs_directories, &dup);
    }
  }

  for (int i = 2; i < argc; i++) {
    char* arg = argv[i];
    if (arg[0] == '-') {
      if (strncmp(arg, "-LN", 3) == 0) {
        char *dup = strdup(&arg[3]);
        vector_push_back(libs_native_directories, &dup);
      } else if (strncmp(arg, "-L", 2) == 0) {
        char *dup = strdup(&arg[2]);
        vector_push_back(libs_directories, &dup);
      } else {
        printf("Unknown option: %s\n", arg);
        return false;
      }
    } else {
      char *dup = strdup(arg);
      vector_push_back(files, &dup);
    }
  }

  if (files->size == 0) {
    error_fatal("No input provided.");
    return false;
  }

  for (size_t i = 0; i < files->size; ++i) {
    char *filename = *(char**)vector_get(files, i);
    file_t* file = file_open(filename, FILE_MODE_READ);
    if (!file) {
      error_fatal("Failed to open %s", filename);
      return false;
    }

    char *src = file_read_text(file);
    if (!src) {
      error_fatal("Failed to read %s", filename);
      return false;
    }

    if (files->size > 1)
      printf("------%s------\n", filename);

    bool interRes = interpret(src);
    free(src);
    file_close(file);

    if (!interRes)
      return false;
  }

  return true;
}

int main(int argc, char* argv[]) {
  config_init();
  bool runRes = run(argc, argv);
  config_end();
  return runRes ? 0 : 1;
}
