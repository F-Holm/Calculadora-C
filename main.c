// printf, FILE, stdin.
#include <stdio.h>

// t_tipo_token, scanner_iniciar, scanner_siguiente_token, scanner_lexema,
// scanner_nombre_token, es_error_token, scanner_hubo_salto_de_linea: toda
// la interfaz pública del escáner que implementamos en scanner.c.
#include "scanner.h"

// Imprime en stdout una línea describiendo un único token ya reconocido.
static void imprimir_token(t_tipo_token token) {
  // Para los tokens de error, y para NUMERO/IDENT, el nombre del token solo
  // no alcanza: necesitamos mostrar también el lexema concreto que se leyó
  // (por ejemplo qué número exacto, qué identificador, o qué carácter
  // inválido produjo el error). Para el resto de los tokens (operadores,
  // paréntesis, asignación, FDT) el nombre del token ya es toda la
  // información que hay, porque su lexema siempre es el mismo carácter fijo.
  if (es_error_token(token) || token == TOKEN_NUMERO || token == TOKEN_IDENT) {
    printf("%s: \"%s\"\n", scanner_nombre_token(token), scanner_lexema());
  } else {
    printf("%s\n", scanner_nombre_token(token));
  }
}

int main(void) {
  // El escáner lee directamente de stdin, de punta a punta, sin que main
  // tenga que ocuparse de partirlo en líneas: fgetc/ungetc (que usa
  // scanner.c por dentro) son estándar de C, no hace falta ninguna función
  // específica del sistema operativo para esto.
  scanner_iniciar(stdin);

  // Primer prompt, antes de reconocer el primer token (como el "> "
  // inicial del ejemplo de la calculadora).
  printf("> ");

  // 'primero' evita imprimir un "> " de más antes del primerísimo token:
  // ese ya lo mostramos arriba.
  bool primero = true;
  t_tipo_token token;
  do {
    token = scanner_siguiente_token();

    // Cada vez que el escáner saltó al menos un '\n' para llegar hasta acá
    // (ver scanner_hubo_salto_de_linea), significa que arrancamos una
    // línea nueva de la entrada, así que mostramos un prompt fresco antes
    // de reportar este token. Como puede haber más de un '\n' seguidos
    // (líneas en blanco), esto imprime un solo "> ", nunca varios.
    if (!primero && scanner_hubo_salto_de_linea()) printf("> ");
    primero = false;

    if (token == TOKEN_FDT) {
      // FDT no tiene lexema que mostrar: solo se informa el nombre del
      // token, marcando que se llegó al verdadero fin de la entrada.
      printf("%s\n", scanner_nombre_token(token));
    } else {
      imprimir_token(token);
    }
  } while (token != TOKEN_FDT);

  return 0;
}
