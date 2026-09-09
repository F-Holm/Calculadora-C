// #pragma once le pide al compilador que, si este archivo se vuelve a
// incluir más de una vez en la misma unidad de compilación (algo que puede
// pasar por cadenas de #include), lo ignore la segunda vez. Cumple la
// misma función que la guarda clásica "#ifndef X / #define X / #endif",
// pero sin necesidad de declarar un macro.
#pragma once

// Necesario porque más abajo usamos el tipo FILE (scanner_iniciar recibe
// un FILE*).
#include <stdio.h>

// Tamaño del buffer estático donde se informa el lexema reconocido.
// 'static' en un constexpr de header significa que cada archivo .c que
// incluya scanner.h obtiene su propia copia interna de esta constante (no
// se comparte entre archivos), evitando así un error de "símbolo
// duplicado" al enlazar main.c y scanner.c. 'constexpr' (C23) garantiza
// que el compilador conoce su valor en tiempo de compilación, así que se
// puede usar, por ejemplo, como tamaño de un array (cosa que un simple
// #define también permite, pero sin que el compilador la trate como una
// variable con tipo real).
static constexpr int TAM_LEXEMA = 64;

// Tokens que puede devolver el escáner.
//
// Los valores están separados en dos rangos numéricos, tal como pide la
// consigna:
//   - Tokens válidos:      0  .. 14
//   - Errores léxicos:   100 .. 101
//
// Ver tabla.md para la tabla de transición completa del autómata que
// produce estos tokens.
typedef enum {
  // ---- Tokens válidos (0-14) ----
  // Como no le asignamos un número a cada uno, el compilador les da
  // valores consecutivos automáticamente empezando en el primero (0).
  // Una constante: entera o con punto decimal.
  TOKEN_NUMERO = 0,
  // Un identificador (nombre de variable).
  TOKEN_IDENT,
  // El carácter '+' solo (sin '=' después).
  TOKEN_SUMA,
  // El carácter '-' solo (sin '=' después).
  TOKEN_RESTA,
  // El carácter '*' solo (sin '=' después).
  TOKEN_MULT,
  // El carácter '/' solo (sin '=' después).
  TOKEN_DIV,
  // El carácter '^'.
  TOKEN_POT,
  // El carácter '=' solo.
  TOKEN_ASIGNAR,
  // El operador compuesto '+='.
  TOKEN_MAS_ASIGNAR,
  // El operador compuesto '-='.
  TOKEN_MENOS_ASIGNAR,
  // El operador compuesto '*='.
  TOKEN_MULT_ASIGNAR,
  // El operador compuesto '/='.
  TOKEN_DIV_ASIGNAR,
  // El carácter '('.
  TOKEN_PAR_ABRE,
  // El carácter ')'.
  TOKEN_PAR_CIERRA,
  // Fin de trama/archivo: se llegó al EOF real.
  TOKEN_FDT,

  // ---- Errores léxicos (100-101) ----
  // Le asignamos 100 al primero a propósito, para que este grupo de
  // valores quede en un rango numérico separado del de los tokens
  // válidos de arriba (0-14). El segundo (ERROR_NUMERO_MAL_FORMADO) sigue
  // la numeración automática desde ahí, así que le queda 101.
  // Se encontró un carácter que no forma parte de ningún token válido
  // (p. ej. '#', '@').
  ERROR_CARACTER_INVALIDO = 100,
  // Un punto decimal aislado, sin ningún dígito ni antes ni después
  // (p. ej. un "." suelto).
  ERROR_NUMERO_MAL_FORMADO
} t_tipo_token;

// true si el token es un error léxico (segundo rango numérico).
// Al ser 'static inline', el compilador puede insertar el cuerpo de esta
// función directamente en el lugar donde se la llama (como si fuera un
// macro), pero a diferencia de un macro de verdad, acá el compilador SÍ
// revisa que 'token' sea del tipo correcto (t_tipo_token), lo cual evita
// errores de tipeo que un macro de texto no detectaría.
static inline bool es_error_token(t_tipo_token token) {
  // Cualquier valor >= 100 (ver el comentario del enum de arriba) cae en
  // el rango de errores léxicos.
  return token >= ERROR_CARACTER_INVALIDO;
}

// Fija el origen de caracteres del escáner. Debe llamarse antes del
// primer scanner_siguiente_token() (y de nuevo cada vez que se lo quiera
// reiniciar para leer de otra fuente, como hace main.c una vez por cada
// línea de la entrada).
void scanner_iniciar(FILE* entrada);

// Reconoce y devuelve el siguiente token, avanzando sobre la entrada que
// se pasó en scanner_iniciar(). Cada llamada consume de la entrada
// exactamente los caracteres que forman un token (ni uno más ni uno
// menos), y dejan el resto disponible para la próxima llamada.
// El lexema correspondiente queda disponible via scanner_lexema().
t_tipo_token scanner_siguiente_token(void);

// Devuelve el lexema del último token reconocido, es decir, el texto
// exacto que se leyó de la entrada para formar ese token (por ejemplo
// "2.3" para un TOKEN_NUMERO, o "b" para un TOKEN_IDENT).
// Apunta a un buffer estático interno de scanner.c (de tamaño
// TAM_LEXEMA), así que el contenido solo es válido hasta el próximo
// llamado a scanner_siguiente_token(): si se necesita conservarlo más
// tiempo, hay que copiarlo a otro lado antes de pedir el siguiente token.
const char* scanner_lexema(void);

// Nombre legible de un t_tipo_token (por ejemplo "TOKEN_NUMERO" o
// "ERROR_CARACTER_INVALIDO"), útil para reportar por consola qué se
// reconoció, en vez de imprimir directamente el número del enum.
const char* scanner_nombre_token(t_tipo_token token);

// Consume lo que quede de la línea actual DESPUÉS del último token
// reconocido: primero los espacios y tabuladores sueltos, y después el
// '\n' que cierra la línea (si lo hay). Devuelve true si efectivamente
// se llegó al fin de una línea (se consumió un '\n') o al fin de la
// entrada; devuelve false si, en cambio, todavía queda otro token en la
// misma línea.
//
// Sirve para que main.c pueda mostrar el prompt "> " ANTES de pedir el
// siguiente token (y no después de que el usuario ya tipeó la línea):
// apenas termina de imprimir un token, llama a esta función y, si dio
// true, sabe que la próxima llamada a scanner_siguiente_token() se va a
// quedar esperando una línea nueva, así que conviene imprimir el prompt
// primero.
bool scanner_fin_de_linea(void);
