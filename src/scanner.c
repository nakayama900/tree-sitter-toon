#include "tree_sitter/array.h"
#include "tree_sitter/parser.h"
#include <wctype.h>

enum TokenType {
  INDENT,
  DEDENT,
};

typedef enum {
  NORMAL_CONTEXT,
  LIST_ITEM_CONTEXT,
} IndentContext;

typedef struct {
  uint32_t length;
  IndentContext context;
} IndentLevel;

typedef struct Scanner {
  Array(IndentLevel) indents;
} Scanner;

static inline void advance(TSLexer *lexer) { lexer->advance(lexer, false); }
static inline void skip(TSLexer *lexer) { lexer->advance(lexer, true); }

void *tree_sitter_toon_external_scanner_create() {
  Scanner *scanner = (Scanner *)calloc(1, sizeof(Scanner));
  array_init(&scanner->indents);
  array_push(&scanner->indents,
             ((IndentLevel){.length = 0, .context = NORMAL_CONTEXT}));
  return scanner;
}

void tree_sitter_toon_external_scanner_destroy(void *payload) {
  Scanner *scanner = (Scanner *)payload;
  array_delete(&scanner->indents);
  free(scanner);
}

unsigned tree_sitter_toon_external_scanner_serialize(void *payload,
                                                     char *buffer) {
  Scanner *scanner = (Scanner *)payload;
  size_t size = scanner->indents.size * sizeof(IndentLevel);

  if (size > TREE_SITTER_SERIALIZATION_BUFFER_SIZE) {
    size = TREE_SITTER_SERIALIZATION_BUFFER_SIZE;
  }

  size = (size / sizeof(IndentLevel)) * sizeof(IndentLevel);

  memcpy(buffer, scanner->indents.contents, size);
  return size;
}

void tree_sitter_toon_external_scanner_deserialize(void *payload,
                                                   const char *buffer,
                                                   unsigned length) {
  Scanner *scanner = (Scanner *)payload;
  array_clear(&scanner->indents);

  if (length == 0) {
    array_push(&scanner->indents,
               ((IndentLevel){.length = 0, .context = NORMAL_CONTEXT}));
    return;
  }

  size_t size = length / sizeof(IndentLevel);
  for (size_t i = 0; i < size; i++) {
    IndentLevel indent;
    memcpy(&indent, buffer + (i * sizeof(IndentLevel)), sizeof(IndentLevel));
    array_push(&scanner->indents, indent);
  }
}

static bool scan_whitespace(TSLexer *lexer, uint32_t *indent_length) {
  *indent_length = 0;

  while (true) {
    if (lexer->lookahead == ' ') {
      (*indent_length)++;
      skip(lexer);
    } else if (lexer->lookahead == '\t') {
      // Tabs count as moving to next tab stop (every 2 spaces for TOON)
      (*indent_length) = ((*indent_length) + 2) & ~1;
      skip(lexer);
    } else {
      break;
    }
  }

  return true;
}

bool tree_sitter_toon_external_scanner_scan(void *payload, TSLexer *lexer,
                                            const bool *valid_symbols) {
  Scanner *scanner = (Scanner *)payload;

  // Mark the end of the token (initially empty).
  // If we return true without calling mark_end again, the lexer will rewind to
  // this point.
  lexer->mark_end(lexer);

  bool has_newline = false;
  uint32_t indent_length = 0;
  uint32_t start_col = lexer->get_column(lexer);

  // 1. Skip newlines and blank lines
  while (true) {
    if (lexer->lookahead == '\n' || lexer->lookahead == '\r') {
      has_newline = true;
      skip(lexer);
      indent_length = 0; // Reset indent length on new line
      start_col = 0;
    } else if (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
      if (lexer->lookahead == ' ') {
        indent_length++;
      } else {
        indent_length = (indent_length + 2) & ~1;
      }
      skip(lexer);
    } else {
      break;
    }
  }

  if (lexer->eof(lexer)) {
    if (valid_symbols[DEDENT] && scanner->indents.size > 1) {
      array_pop(&scanner->indents);
      lexer->result_symbol = DEDENT;
      return true;
    }
    return false;
  }

  // If we didn't see a newline and we're not at the start of the file,
  // we are not processing indentation.
  if (!has_newline && start_col != 0) {
    return false;
  }

  uint32_t current_indent = array_back(&scanner->indents)->length;

  // Indent
  if (indent_length > current_indent) {
    if (valid_symbols[INDENT]) {
      IndentContext new_context = NORMAL_CONTEXT;
      if (lexer->lookahead == '-') {
        new_context = LIST_ITEM_CONTEXT;
      }
      array_push(&scanner->indents, ((IndentLevel){.length = indent_length,
                                                   .context = new_context}));
      lexer->result_symbol = INDENT;
      // Commit the consumed newlines and indentation
      lexer->mark_end(lexer);
      return true;
    }
  }

  // Dedent
  if (indent_length < current_indent) {
    if (valid_symbols[DEDENT]) {
      array_pop(&scanner->indents);
      lexer->result_symbol = DEDENT;
      // Do NOT call mark_end. This rewinds the lexer to the start of the
      // newlines. Next time scan is called, we will re-scan the newlines and
      // indentation.
      return true;
    }
  }

  return false;
}
