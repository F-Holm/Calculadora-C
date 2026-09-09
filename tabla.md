# Tabla de transición del escáner

## Definición formal del autómata

El escáner es un Autómata Finito Determinístico (AFD), definido como la
5-upla M = (Q, Σ, T, q0, F):

- **Q**: el conjunto de estados listado en la sección "Estados" más abajo.
- **Σ**: el alfabeto de entrada, agrupado en las 14 clases de caracteres
  de la siguiente sección (agrupar caracteres equivalentes en clases es
  la misma técnica de "definición regular" usada para simplificar
  expresiones regulares).
- **T: Q×Σ → Q**: la función de transición, dada por la tabla de la
  sección "Tabla de transición". No está definida para todo par de Q×Σ
  (no hace falta que lo esté): si en medio del reconocimiento de un
  token el par (estado, clase) no tiene transición, el análisis se
  detiene ahí. Si el estado en el que se detuvo es aceptor (∈ F), el
  token quedó completo y reconocido; si no, es un error léxico.
- **q0**: el estado INICIAL (0).
- **F**: el conjunto de estados aceptores, es decir todo el rango
  20-35 (ver sección "Estados").

## Clases de caracteres (columnas)

| # | Clase        | Caracteres que incluye              |
|---|--------------|--------------------------------------|
| 0 | LETRA        | `a`-`z`, `A`-`Z`                     |
| 1 | DIGITO       | `0`-`9`                              |
| 2 | PUNTO        | `.`                                   |
| 3 | MAS          | `+`                                   |
| 4 | MENOS        | `-`                                   |
| 5 | POR          | `*`                                   |
| 6 | BARRA        | `/`                                   |
| 7 | CIRCUNFLEJO  | `^`                                   |
| 8 | IGUAL        | `=`                                   |
| 9 | PARIZQ       | `(`                                    |
| 10| PARDER       | `)`                                    |
| 11| ESPACIO      | espacio, tab, `\n`, `\r`             |
| 12| EOF_C        | fin de archivo                        |
| 13| OTRO         | cualquier otro carácter no listado    |

## Estados

En la notación de diagramas de estado (estado inicial marcado "-",
estados aceptores marcados "+"), el estado 0 (INICIAL) es el "-" y
todo el rango 20-35 son los "+". Acá, en lugar de esa marca, la
condición de aceptor se codifica directamente en el rango numérico del
estado, que es la optimización que pide la consigna del TP2 (revisar
si un estado es aceptor es una sola comparación de rango en vez de un
lookup aparte):

| Rango     | Significado                                                            |
|-----------|-------------------------------------------------------------------------|
| 0 - 1     | Estados no aceptores (todavía no se completó ningún token)             |
| 20 - 35   | Estados aceptores (token válido reconocido)                            |
| 50        | Estado de error (trampa; el error específico depende del estado origen)|

| Estado | Nombre         | Aceptor | Token que produce      |
|--------|----------------|:-------:|--------------------------|
| 0      | INICIAL        | no      | —                         |
| 1      | PUNTO_PENDIENTE| no      | —                         |
| 20     | ENTERO         | sí      | TOKEN_NUMERO              |
| 21     | DECIMAL        | sí      | TOKEN_NUMERO              |
| 22     | IDENT          | sí      | TOKEN_IDENT                |
| 23     | SUMA           | sí      | TOKEN_SUMA                  |
| 24     | RESTA          | sí      | TOKEN_RESTA                  |
| 25     | MULT           | sí      | TOKEN_MULT                    |
| 26     | DIV            | sí      | TOKEN_DIV                      |
| 27     | POT            | sí      | TOKEN_POT                       |
| 28     | ASIGNAR        | sí      | TOKEN_ASIGNAR                    |
| 29     | MAS_ASIGNAR    | sí      | TOKEN_MAS_ASIGNAR                 |
| 30     | MENOS_ASIGNAR  | sí      | TOKEN_MENOS_ASIGNAR                |
| 31     | MULT_ASIGNAR   | sí      | TOKEN_MULT_ASIGNAR                  |
| 32     | DIV_ASIGNAR    | sí      | TOKEN_DIV_ASIGNAR                    |
| 33     | PAR_ABRE       | sí      | TOKEN_PAR_ABRE                        |
| 34     | PAR_CIERRA     | sí      | TOKEN_PAR_CIERRA                       |
| 35     | FDT            | sí      | TOKEN_FDT                               |
| 50     | ERROR          | —       | ver más abajo                            |

Los estados 27-35 son **terminales**: no tienen transiciones de salida porque
el token queda completo apenas se entra en ellos (no hay ningún carácter
siguiente que pueda extenderlos). Por eso no aparecen como filas en la
tabla: el escáner los resuelve sin necesidad de una consulta más, lo cual
es la optimización pedida en la consigna.

## Tabla de transición (solo estados con filas reales)

| Estado \ Clase    | LETRA | DIGITO | PUNTO | MAS | MENOS | POR | BARRA | CIRCUNFLEJO | IGUAL | PARIZQ | PARDER | ESPACIO | EOF_C | OTRO |
|-------------------|:-----:|:------:|:-----:|:---:|:-----:|:---:|:-----:|:-----------:|:-----:|:------:|:------:|:-------:|:-----:|:----:|
| 0  INICIAL        | 22    | 20     | 1     | 23  | 24    | 25  | 26    | 27          | 28    | 33     | 34     | 0       | 35    | 50   |
| 1  PUNTO_PENDIENTE| 50    | 21     | 50    | 50  | 50    | 50  | 50    | 50          | 50    | 50     | 50     | 50      | 50    | 50   |
| 20 ENTERO         | 50    | 20     | 21    | 50  | 50    | 50  | 50    | 50          | 50    | 50     | 50     | 50      | 50    | 50   |
| 21 DECIMAL        | 50    | 21     | 50    | 50  | 50    | 50  | 50    | 50          | 50    | 50     | 50     | 50      | 50    | 50   |
| 22 IDENT          | 22    | 22     | 50    | 50  | 50    | 50  | 50    | 50          | 50    | 50     | 50     | 50      | 50    | 50   |
| 23 SUMA           | 50    | 50     | 50    | 50  | 50    | 50  | 50    | 50          | 29    | 50     | 50     | 50      | 50    | 50   |
| 24 RESTA          | 50    | 50     | 50    | 50  | 50    | 50  | 50    | 50          | 30    | 50     | 50     | 50      | 50    | 50   |
| 25 MULT           | 50    | 50     | 50    | 50  | 50    | 50  | 50    | 50          | 31    | 50     | 50     | 50      | 50    | 50   |
| 26 DIV            | 50    | 50     | 50    | 50  | 50    | 50  | 50    | 50          | 32    | 50     | 50     | 50      | 50    | 50   |

Notas de lectura:

- Si el estado actual es **aceptor** (20-35) y la celda da 50, no es un
  error: significa que el token terminó ahí. El carácter que se estaba
  mirando **no se consume**, queda disponible para el próximo token.
- Si el estado actual es **no aceptor** (0-1) y la celda da 50, ahí sí es
  un error léxico real, y el carácter ofensivo se consume para poder
  informarlo en el lexema.
- La fila 0 (INICIAL) en la columna ESPACIO vuelve a 0: los espacios en
  blanco se descartan y no forman parte de ningún lexema.

## Errores léxicos que puede devolver el escáner

| Token                     | Cuándo se produce                                                                 | Ejemplo de entrada |
|---------------------------|-------------------------------------------------------------------------------------|---------------------|
| ERROR_CARACTER_INVALIDO   | Desde el estado INICIAL se lee un carácter que no pertenece a ninguna clase válida  | `a # 2`             |
| ERROR_NUMERO_MAL_FORMADO  | Se lee un `.` que no está seguido de al menos un dígito (punto aislado)             | `3 + . 5`           |

## Reconocimiento de constantes numéricas

Se aceptan enteros (`123`), decimales completos (`123.45`), decimales sin
parte entera (`.45`) y decimales sin parte fraccionaria (`123.`). No se
acepta un punto sin ningún dígito adyacente (`.` sola), que cae en
ERROR_NUMERO_MAL_FORMADO.
