#include <stdio.h>

#include "scanner.h"

static void imprimir_token(t_tipo_token token);

int main(void) {
  bool inicio_de_linea = true;
  t_tipo_token token;
  do {
    if (inicio_de_linea) {
      printf("> ");
    }

    token = scanner_siguiente_token();

    if (token == TOKEN_FDT) {
      printf("%s\n", scanner_nombre_token(token));
    } else {
      imprimir_token(token);
    }

    inicio_de_linea = scanner_fin_de_linea();
  } while (token != TOKEN_FDT);

  return 0;
}

static void imprimir_token(t_tipo_token token) {
  if (es_error_token(token) || token == TOKEN_NUMERO || token == TOKEN_IDENT) {
    printf("%s: \"%s\"\n", scanner_nombre_token(token), scanner_lexema());
  } else {
    printf("%s\n", scanner_nombre_token(token));
  }
}
