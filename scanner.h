#pragma once

static constexpr int TAM_LEXEMA = 64;

typedef enum {
  TOKEN_NUMERO = 0,
  TOKEN_IDENT,
  TOKEN_SUMA,
  TOKEN_RESTA,
  TOKEN_MULT,
  TOKEN_DIV,
  TOKEN_POT,
  TOKEN_ASIGNAR,
  TOKEN_MAS_ASIGNAR,
  TOKEN_MENOS_ASIGNAR,
  TOKEN_MULT_ASIGNAR,
  TOKEN_DIV_ASIGNAR,
  TOKEN_PAR_ABRE,
  TOKEN_PAR_CIERRA,
  TOKEN_FDT,

  ERROR_CARACTER_INVALIDO = 100,
  ERROR_NUMERO_MAL_FORMADO
} t_tipo_token;

static inline bool es_error_token(t_tipo_token token) {
  return token >= ERROR_CARACTER_INVALIDO;
}

t_tipo_token scanner_siguiente_token(void);

const char* scanner_lexema(void);

const char* scanner_nombre_token(t_tipo_token token);

bool scanner_fin_de_linea(void);
