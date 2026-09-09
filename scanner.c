#include "scanner.h"

#include <ctype.h>
#include <stdio.h>

typedef enum {
  CLASE_LETRA = 0,
  CLASE_DIGITO,
  CLASE_PUNTO,
  CLASE_MAS,
  CLASE_MENOS,
  CLASE_POR,
  CLASE_BARRA,
  CLASE_CIRCUNFLEJO,
  CLASE_IGUAL,
  CLASE_PARIZQ,
  CLASE_PARDER,
  CLASE_ESPACIO,
  CLASE_EOF,
  CLASE_OTRO,
  NUM_CLASES
} t_clase;

enum {
  EST_INICIAL = 0,
  EST_PUNTO_PENDIENTE = 1,

  EST_ENTERO = 20,
  EST_DECIMAL = 21,
  EST_IDENT = 22,
  EST_SUMA = 23,
  EST_RESTA = 24,
  EST_MULT = 25,
  EST_DIV = 26,
  EST_POT = 27,
  EST_ASIGNAR = 28,
  EST_MAS_ASIGNAR = 29,
  EST_MENOS_ASIGNAR = 30,
  EST_MULT_ASIGNAR = 31,
  EST_DIV_ASIGNAR = 32,
  EST_PAR_ABRE = 33,
  EST_PAR_CIERRA = 34,
  EST_FDT = 35,

  EST_ERROR = 50
};

static constexpr int NUM_FILAS = 27;

static const int tabla[NUM_FILAS][NUM_CLASES] = {
    [EST_INICIAL] = {[CLASE_LETRA] = EST_IDENT,
                     [CLASE_DIGITO] = EST_ENTERO,
                     [CLASE_PUNTO] = EST_PUNTO_PENDIENTE,
                     [CLASE_MAS] = EST_SUMA,
                     [CLASE_MENOS] = EST_RESTA,
                     [CLASE_POR] = EST_MULT,
                     [CLASE_BARRA] = EST_DIV,
                     [CLASE_CIRCUNFLEJO] = EST_POT,
                     [CLASE_IGUAL] = EST_ASIGNAR,
                     [CLASE_PARIZQ] = EST_PAR_ABRE,
                     [CLASE_PARDER] = EST_PAR_CIERRA,
                     [CLASE_ESPACIO] = EST_INICIAL,
                     [CLASE_EOF] = EST_FDT,
                     [CLASE_OTRO] = EST_ERROR},
    [EST_PUNTO_PENDIENTE] = {[CLASE_LETRA] = EST_ERROR,
                             [CLASE_DIGITO] = EST_DECIMAL,
                             [CLASE_PUNTO] = EST_ERROR,
                             [CLASE_MAS] = EST_ERROR,
                             [CLASE_MENOS] = EST_ERROR,
                             [CLASE_POR] = EST_ERROR,
                             [CLASE_BARRA] = EST_ERROR,
                             [CLASE_CIRCUNFLEJO] = EST_ERROR,
                             [CLASE_IGUAL] = EST_ERROR,
                             [CLASE_PARIZQ] = EST_ERROR,
                             [CLASE_PARDER] = EST_ERROR,
                             [CLASE_ESPACIO] = EST_ERROR,
                             [CLASE_EOF] = EST_ERROR,
                             [CLASE_OTRO] = EST_ERROR},
    [EST_ENTERO] = {[CLASE_LETRA] = EST_ERROR,
                    [CLASE_DIGITO] = EST_ENTERO,
                    [CLASE_PUNTO] = EST_DECIMAL,
                    [CLASE_MAS] = EST_ERROR,
                    [CLASE_MENOS] = EST_ERROR,
                    [CLASE_POR] = EST_ERROR,
                    [CLASE_BARRA] = EST_ERROR,
                    [CLASE_CIRCUNFLEJO] = EST_ERROR,
                    [CLASE_IGUAL] = EST_ERROR,
                    [CLASE_PARIZQ] = EST_ERROR,
                    [CLASE_PARDER] = EST_ERROR,
                    [CLASE_ESPACIO] = EST_ERROR,
                    [CLASE_EOF] = EST_ERROR,
                    [CLASE_OTRO] = EST_ERROR},
    [EST_DECIMAL] = {[CLASE_LETRA] = EST_ERROR,
                     [CLASE_DIGITO] = EST_DECIMAL,
                     [CLASE_PUNTO] = EST_ERROR,
                     [CLASE_MAS] = EST_ERROR,
                     [CLASE_MENOS] = EST_ERROR,
                     [CLASE_POR] = EST_ERROR,
                     [CLASE_BARRA] = EST_ERROR,
                     [CLASE_CIRCUNFLEJO] = EST_ERROR,
                     [CLASE_IGUAL] = EST_ERROR,
                     [CLASE_PARIZQ] = EST_ERROR,
                     [CLASE_PARDER] = EST_ERROR,
                     [CLASE_ESPACIO] = EST_ERROR,
                     [CLASE_EOF] = EST_ERROR,
                     [CLASE_OTRO] = EST_ERROR},
    [EST_IDENT] = {[CLASE_LETRA] = EST_IDENT,
                   [CLASE_DIGITO] = EST_IDENT,
                   [CLASE_PUNTO] = EST_ERROR,
                   [CLASE_MAS] = EST_ERROR,
                   [CLASE_MENOS] = EST_ERROR,
                   [CLASE_POR] = EST_ERROR,
                   [CLASE_BARRA] = EST_ERROR,
                   [CLASE_CIRCUNFLEJO] = EST_ERROR,
                   [CLASE_IGUAL] = EST_ERROR,
                   [CLASE_PARIZQ] = EST_ERROR,
                   [CLASE_PARDER] = EST_ERROR,
                   [CLASE_ESPACIO] = EST_ERROR,
                   [CLASE_EOF] = EST_ERROR,
                   [CLASE_OTRO] = EST_ERROR},
    [EST_SUMA] = {[CLASE_LETRA] = EST_ERROR,
                  [CLASE_DIGITO] = EST_ERROR,
                  [CLASE_PUNTO] = EST_ERROR,
                  [CLASE_MAS] = EST_ERROR,
                  [CLASE_MENOS] = EST_ERROR,
                  [CLASE_POR] = EST_ERROR,
                  [CLASE_BARRA] = EST_ERROR,
                  [CLASE_CIRCUNFLEJO] = EST_ERROR,
                  [CLASE_IGUAL] = EST_MAS_ASIGNAR,
                  [CLASE_PARIZQ] = EST_ERROR,
                  [CLASE_PARDER] = EST_ERROR,
                  [CLASE_ESPACIO] = EST_ERROR,
                  [CLASE_EOF] = EST_ERROR,
                  [CLASE_OTRO] = EST_ERROR},
    [EST_RESTA] = {[CLASE_LETRA] = EST_ERROR,
                   [CLASE_DIGITO] = EST_ERROR,
                   [CLASE_PUNTO] = EST_ERROR,
                   [CLASE_MAS] = EST_ERROR,
                   [CLASE_MENOS] = EST_ERROR,
                   [CLASE_POR] = EST_ERROR,
                   [CLASE_BARRA] = EST_ERROR,
                   [CLASE_CIRCUNFLEJO] = EST_ERROR,
                   [CLASE_IGUAL] = EST_MENOS_ASIGNAR,
                   [CLASE_PARIZQ] = EST_ERROR,
                   [CLASE_PARDER] = EST_ERROR,
                   [CLASE_ESPACIO] = EST_ERROR,
                   [CLASE_EOF] = EST_ERROR,
                   [CLASE_OTRO] = EST_ERROR},
    [EST_MULT] = {[CLASE_LETRA] = EST_ERROR,
                  [CLASE_DIGITO] = EST_ERROR,
                  [CLASE_PUNTO] = EST_ERROR,
                  [CLASE_MAS] = EST_ERROR,
                  [CLASE_MENOS] = EST_ERROR,
                  [CLASE_POR] = EST_ERROR,
                  [CLASE_BARRA] = EST_ERROR,
                  [CLASE_CIRCUNFLEJO] = EST_ERROR,
                  [CLASE_IGUAL] = EST_MULT_ASIGNAR,
                  [CLASE_PARIZQ] = EST_ERROR,
                  [CLASE_PARDER] = EST_ERROR,
                  [CLASE_ESPACIO] = EST_ERROR,
                  [CLASE_EOF] = EST_ERROR,
                  [CLASE_OTRO] = EST_ERROR},
    [EST_DIV] = {[CLASE_LETRA] = EST_ERROR,
                 [CLASE_DIGITO] = EST_ERROR,
                 [CLASE_PUNTO] = EST_ERROR,
                 [CLASE_MAS] = EST_ERROR,
                 [CLASE_MENOS] = EST_ERROR,
                 [CLASE_POR] = EST_ERROR,
                 [CLASE_BARRA] = EST_ERROR,
                 [CLASE_CIRCUNFLEJO] = EST_ERROR,
                 [CLASE_IGUAL] = EST_DIV_ASIGNAR,
                 [CLASE_PARIZQ] = EST_ERROR,
                 [CLASE_PARDER] = EST_ERROR,
                 [CLASE_ESPACIO] = EST_ERROR,
                 [CLASE_EOF] = EST_ERROR,
                 [CLASE_OTRO] = EST_ERROR}};

static char lexema[TAM_LEXEMA];

static int tiene_fila(int estado);
static int es_aceptor(int estado);
static t_tipo_token token_de_estado(int estado);
static t_clase clasificar(int c);
static int peek(void);
static int avanzar(void);

t_tipo_token scanner_siguiente_token(void) {
  int estado = EST_INICIAL;
  size_t pos = 0;

  for (;;) {
    int c = peek();
    t_clase clase = clasificar(c);
    int siguiente = tiene_fila(estado) ? tabla[estado][clase] : EST_ERROR;

    if (estado == EST_INICIAL && siguiente == EST_INICIAL) {
      avanzar();
      continue;
    }

    if (siguiente == EST_ERROR) {
      if (es_aceptor(estado)) break;

      if (c != EOF && clase != CLASE_ESPACIO && pos < TAM_LEXEMA - 1) {
        lexema[pos++] = (char)avanzar();
      }
      lexema[pos] = '\0';
      return (estado == EST_PUNTO_PENDIENTE) ? ERROR_NUMERO_MAL_FORMADO
                                             : ERROR_CARACTER_INVALIDO;
    }

    avanzar();
    if (c != EOF && pos < TAM_LEXEMA - 1) lexema[pos++] = (char)c;
    estado = siguiente;

    if (!tiene_fila(estado)) break;
  }

  lexema[pos] = '\0';
  return token_de_estado(estado);
}

const char* scanner_lexema(void) { return lexema; }

const char* scanner_nombre_token(t_tipo_token token) {
  switch (token) {
    case TOKEN_NUMERO:
      return "NUMERO";
    case TOKEN_IDENT:
      return "IDENT";
    case TOKEN_SUMA:
      return "SUMA";
    case TOKEN_RESTA:
      return "RESTA";
    case TOKEN_MULT:
      return "MULT";
    case TOKEN_DIV:
      return "DIV";
    case TOKEN_POT:
      return "POT";
    case TOKEN_ASIGNAR:
      return "ASIGNAR";
    case TOKEN_MAS_ASIGNAR:
      return "MAS_ASIGNAR";
    case TOKEN_MENOS_ASIGNAR:
      return "MENOS_ASIGNAR";
    case TOKEN_MULT_ASIGNAR:
      return "MULT_ASIGNAR";
    case TOKEN_DIV_ASIGNAR:
      return "DIV_ASIGNAR";
    case TOKEN_PAR_ABRE:
      return "PAR_ABRE";
    case TOKEN_PAR_CIERRA:
      return "PAR_CIERRA";
    case TOKEN_FDT:
      return "FDT";
    case ERROR_CARACTER_INVALIDO:
      return "ERROR_CARACTER_INVALIDO";
    case ERROR_NUMERO_MAL_FORMADO:
      return "ERROR_NUMERO_MAL_FORMADO";
    default:
      return "DESCONOCIDO";
  }
}

bool scanner_fin_de_linea(void) {
  int c = peek();
  while (c != EOF && c != '\n' && isspace(c)) {
    avanzar();
    c = peek();
  }

  if (c == EOF) return true;

  if (c == '\n') {
    avanzar();
    return true;
  }

  return false;
}

static int tiene_fila(int estado) {
  switch (estado) {
    case EST_INICIAL:
    case EST_PUNTO_PENDIENTE:
    case EST_ENTERO:
    case EST_DECIMAL:
    case EST_IDENT:
    case EST_SUMA:
    case EST_RESTA:
    case EST_MULT:
    case EST_DIV:
      return 1;
    default:
      return 0;
  }
}

static int es_aceptor(int estado) { return estado >= 20 && estado <= 35; }

static t_tipo_token token_de_estado(int estado) {
  switch (estado) {
    case EST_ENTERO:
    case EST_DECIMAL:
      return TOKEN_NUMERO;
    case EST_IDENT:
      return TOKEN_IDENT;
    case EST_SUMA:
      return TOKEN_SUMA;
    case EST_RESTA:
      return TOKEN_RESTA;
    case EST_MULT:
      return TOKEN_MULT;
    case EST_DIV:
      return TOKEN_DIV;
    case EST_POT:
      return TOKEN_POT;
    case EST_ASIGNAR:
      return TOKEN_ASIGNAR;
    case EST_MAS_ASIGNAR:
      return TOKEN_MAS_ASIGNAR;
    case EST_MENOS_ASIGNAR:
      return TOKEN_MENOS_ASIGNAR;
    case EST_MULT_ASIGNAR:
      return TOKEN_MULT_ASIGNAR;
    case EST_DIV_ASIGNAR:
      return TOKEN_DIV_ASIGNAR;
    case EST_PAR_ABRE:
      return TOKEN_PAR_ABRE;
    case EST_PAR_CIERRA:
      return TOKEN_PAR_CIERRA;
    default:
      return TOKEN_FDT;
  }
}

static t_clase clasificar(int c) {
  if (c == EOF) return CLASE_EOF;
  if (isalpha(c)) return CLASE_LETRA;
  if (isdigit(c)) return CLASE_DIGITO;
  switch (c) {
    case '.':
      return CLASE_PUNTO;
    case '+':
      return CLASE_MAS;
    case '-':
      return CLASE_MENOS;
    case '*':
      return CLASE_POR;
    case '/':
      return CLASE_BARRA;
    case '^':
      return CLASE_CIRCUNFLEJO;
    case '=':
      return CLASE_IGUAL;
    case '(':
      return CLASE_PARIZQ;
    case ')':
      return CLASE_PARDER;
    default:
      if (isspace(c)) return CLASE_ESPACIO;
      return CLASE_OTRO;
  }
}

static int peek(void) {
  int c = fgetc(stdin);
  if (c != EOF) ungetc(c, stdin);
  return c;
}

static int avanzar(void) { return fgetc(stdin); }
