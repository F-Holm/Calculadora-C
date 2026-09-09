// ===========================================================================
// Includes
// ===========================================================================

// printf, fflush, FILE, stdin, stdout.
#include <stdio.h>

// t_tipo_token, scanner_siguiente_token, scanner_lexema, scanner_nombre_token,
// es_error_token, scanner_fin_de_linea: toda la interfaz pública del escáner
// que implementamos en scanner.c.
#include "scanner.h"

// ===========================================================================
// Declaraciones de funciones privadas (static)
// ===========================================================================

// Imprime en stdout una línea describiendo un único token ya reconocido.
static void imprimir_token(t_tipo_token token);

// ===========================================================================
// Definiciones de funciones públicas
// ===========================================================================

int main(void) {
  // El escáner lee directamente de stdin, de punta a punta, sin que main
  // tenga que ocuparse de partirlo en líneas ni de pasarle la fuente:
  // fgetc/ungetc sobre stdin (que usa scanner.c por dentro) son estándar
  // de C, no hace falta ninguna función específica del sistema operativo.

  // 'inicio_de_linea' arranca en true para que se imprima el primer
  // prompt antes de reconocer el primer token (como el "> " inicial del
  // ejemplo de la calculadora). Después, scanner_fin_de_linea() lo vuelve
  // a poner en true cada vez que se termina una línea de la entrada.
  bool inicio_de_linea = true;
  t_tipo_token token;
  do {
    // El prompt se imprime ANTES de pedir el token, y se fuerza el vaciado
    // del buffer de salida con fflush: así el "> " aparece en la terminal
    // de inmediato, antes de que scanner_siguiente_token() se quede
    // esperando lo que el usuario tipee (printf por sí solo no garantiza
    // que el "> ", al no terminar en '\n', se muestre antes de la lectura).
    if (inicio_de_linea) {
      printf("> ");
      fflush(stdout);
    }

    token = scanner_siguiente_token();

    if (token == TOKEN_FDT) {
      // FDT no tiene lexema que mostrar: solo se informa el nombre del
      // token, marcando que se llegó al verdadero fin de la entrada.
      printf("%s\n", scanner_nombre_token(token));
    } else {
      imprimir_token(token);
    }

    // scanner_fin_de_linea() consume los espacios y el '\n' que puedan
    // quedar después de este token. Si devuelve true, la próxima vuelta
    // arranca una línea nueva (o el fin de la entrada), así que toca
    // volver a mostrar el prompt. Como consume todos los '\n' seguidos que
    // encuentre, varias líneas en blanco producen un solo "> ", nunca varios.
    inicio_de_linea = scanner_fin_de_linea();
  } while (token != TOKEN_FDT);

  return 0;
}

// ===========================================================================
// Definiciones de funciones privadas (static)
// ===========================================================================

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
