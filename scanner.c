// Enum t_tipo_token, TAM_LEXEMA y los prototipos públicos del escáner que
// implementamos acá abajo.
#include "scanner.h"

// isalpha(), isdigit(), isspace(): para clasificar cada carácter leído.
#include <ctype.h>

// Clases de columna de la tabla (ver tabla.md): agrupamos los caracteres
// de entrada en categorías, en vez de tener una columna por cada uno de
// los ~128 caracteres ASCII posibles. Todos los caracteres de una misma
// clase producen siempre la misma transición desde cualquier estado del
// autómata, así que agruparlos no le hace perder información a la tabla,
// y la deja mucho más chica y fácil de escribir a mano.
typedef enum {
  // 'a'-'z', 'A'-'Z'.
  CLASE_LETRA = 0,
  // '0'-'9'.
  CLASE_DIGITO,
  // '.'.
  CLASE_PUNTO,
  // '+'.
  CLASE_MAS,
  // '-'.
  CLASE_MENOS,
  // '*'.
  CLASE_POR,
  // '/'.
  CLASE_BARRA,
  // '^'.
  CLASE_CIRCUNFLEJO,
  // '='.
  CLASE_IGUAL,
  // '('.
  CLASE_PARIZQ,
  // ')'.
  CLASE_PARDER,
  // Espacio, tabulador, salto de línea, retorno de carro, etc.
  CLASE_ESPACIO,
  // Fin de la entrada (lo que devuelve fgetc/peek al no quedar más
  // caracteres para leer).
  CLASE_EOF,
  // Cualquier carácter que no encaja en ninguna clase anterior.
  CLASE_OTRO,
  // No es una clase de carácter real: al ser el último valor del enum,
  // el compilador la deja automáticamente en "cantidad de clases
  // anteriores", así que NUM_CLASES siempre es exactamente la cantidad
  // de columnas de la tabla, sin tener que contarlas a mano ni
  // arriesgarnos a que quede desactualizado si se agrega o saca una
  // clase.
  NUM_CLASES
} t_clase;

// Estados del autómata (ver tabla.md para el diagrama completo y el
// porqué de cada rango). A diferencia de los tokens de scanner.h, estos
// estados son un detalle interno del escáner: quien usa scanner.h nunca
// los ve.
enum {
  // Estado de arranque: todavía no se empezó a reconocer ningún token.
  EST_INICIAL = 0,
  // Se leyó un '.' sin ningún dígito antes, y todavía no se sabe si va a
  // venir un dígito después (que lo haría un NUMERO válido) o no (que
  // sería un punto aislado, un error).
  EST_PUNTO_PENDIENTE = 1,

  // A partir de acá arrancan los estados ACEPTORES (token reconocido),
  // en el rango 20-35, separado a propósito del rango de los estados de
  // arriba (0-1, que todavía no aceptan nada).
  // Ya se leyó al menos un dígito antes de cualquier punto (por ejemplo
  // "3", o el "3" de "3.14" antes de llegar al punto).
  EST_ENTERO = 20,
  // Ya se leyó el punto decimal y, opcionalmente, dígitos después (por
  // ejemplo "3.", "3.1" o ".5").
  EST_DECIMAL = 21,
  // Se está leyendo un identificador (una letra, y después letras o
  // dígitos).
  EST_IDENT = 22,
  // Se leyó un '+' y todavía no se sabe si el próximo carácter es un '='
  // (que lo haría "+=") o no (que lo dejaría como "+" solo).
  EST_SUMA = 23,
  // Igual que EST_SUMA pero para '-' / "-=".
  EST_RESTA = 24,
  // Igual que EST_SUMA pero para '*' / "*=".
  EST_MULT = 25,
  // Igual que EST_SUMA pero para '/' / "/=".
  EST_DIV = 26,
  // Estados TERMINALES: no hace falta mirar el próximo carácter para
  // saber que el token ya está completo, porque no hay ninguna forma de
  // que un carácter más lo siga extendiendo (por ejemplo, un '^' suelto
  // siempre es un TOKEN_POT completo apenas se lo lee). Por eso no
  // tienen fila propia en la tabla: ver tiene_fila() más abajo.
  EST_POT = 27,
  EST_ASIGNAR = 28,
  EST_MAS_ASIGNAR = 29,
  EST_MENOS_ASIGNAR = 30,
  EST_MULT_ASIGNAR = 31,
  EST_DIV_ASIGNAR = 32,
  EST_PAR_ABRE = 33,
  EST_PAR_CIERRA = 34,
  EST_FDT = 35,

  // Estado "trampa": significa "no hay ninguna transición válida desde
  // acá". No es un estado que el autómata pueda seguir usando después
  // (nunca se busca una fila para EST_ERROR en la tabla); es solo un
  // valor que devuelve la búsqueda en la tabla para decirle al código de
  // scanner_siguiente_token() que tiene que decidir entre "el token
  // anterior ya estaba completo" o "esto es un error léxico de verdad"
  // (ver esa función más abajo). Queda en su propio rango numérico (50),
  // separado tanto de los estados no aceptores (0-1) como de los
  // aceptores (20-35), tal como pide la consigna.
  EST_ERROR = 50
};

// Alcanza para indexar directo hasta EST_DIV (26, el estado con número
// más alto que sí tiene fila propia en la tabla); no hace falta llegar
// hasta 36 porque a los estados terminales (27 en adelante) nunca se los
// busca en la tabla, así que no necesitan fila.
static constexpr int NUM_FILAS = 27;

// Tabla de transición: dado el estado actual (fila) y la clase del
// próximo carácter (columna), da el estado al que pasa el autómata.
// Solo las filas de estados con salidas reales (INICIAL, PUNTO_PENDIENTE
// y los aceptores no terminales) están completas; el resto de las filas
// nunca se consulta porque esos estados son terminales y el escáner
// corta apenas los alcanza (ver tiene_fila()). Esta es la misma tabla
// documentada en tabla.md: cualquier cambio acá debería reflejarse
// también ahí.
static const int tabla[NUM_FILAS][NUM_CLASES] = {
    // Desde el estado inicial, cada clase de carácter dispara el
    // arranque de un token distinto (o, en el caso de un espacio, se
    // queda en el mismo estado inicial para descartarlo: ver el chequeo
    // especial en scanner_siguiente_token()).
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
    // Desde acá, la ÚNICA forma de seguir hacia un token válido es que
    // el próximo carácter sea un dígito (pasando a EST_DECIMAL). Todo lo
    // demás (otra letra, otro punto, un operador, un espacio, EOF, etc.)
    // da EST_ERROR; y como EST_PUNTO_PENDIENTE no es un estado aceptor,
    // eso se traduce en un ERROR_NUMERO_MAL_FORMADO real (el punto
    // quedó aislado).
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
    // Ya venimos leyendo dígitos de la parte entera. Otro dígito sigue
    // en este mismo estado (el número puede seguir creciendo); un punto
    // pasa a EST_DECIMAL (empieza la parte fraccionaria); cualquier otra
    // cosa corta el número acá (y como EST_ENTERO SÍ es aceptor, cortar
    // acá significa "el NUMERO ya está completo", no un error).
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
    // Ya pasamos el punto decimal (con o sin dígitos previos: "3." y
    // ".5" llegan acá igual). Solo un dígito extiende el número; no se
    // acepta un segundo punto (por eso "3.14.15" se corta en dos
    // NUMERO distintos, no en un error).
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
    // Un identificador sigue extendiéndose con letras o dígitos (por
    // ejemplo "variable1"); cualquier otra cosa lo corta (y como
    // EST_IDENT es aceptor, ahí termina un TOKEN_IDENT válido).
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
    // Se leyó un '+'. Si el próximo carácter es '=', se completa como
    // "+=" (EST_MAS_ASIGNAR); cualquier otra cosa deja el '+' como
    // token SUMA ya completo (EST_SUMA es aceptor).
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
    // Igual que EST_SUMA pero para '-' / "-=".
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
    // Igual que EST_SUMA pero para '*' / "*=".
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
    // Igual que EST_SUMA pero para '/' / "/=".
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

// Estados aceptores que además tienen fila propia en la tabla, es decir,
// que TODAVÍA pueden seguir consumiendo más caracteres antes de que el
// token quede cerrado (por ejemplo EST_IDENT: "v", "va", "var" son todos
// identificadores válidos por sí solos, pero hay que seguir mirando el
// próximo carácter para saber si el identificador sigue creciendo).
// Cualquier estado >= 20 que NO está en esta lista es un estado
// TERMINAL: el token se cierra automáticamente apenas se entra en él,
// sin necesidad de mirar (ni consumir) ningún carácter más. Por eso el
// resto de las filas de la tabla de arriba directamente no existen: esta
// función es la que le dice a scanner_siguiente_token() cuándo puede
// saltearse la consulta a la tabla.
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

// true si 'estado' es uno de los estados aceptores (rango 20-35 del
// enum de más arriba), es decir, si detenerse ahí significa "se
// reconoció un token válido" en vez de "hubo un error léxico".
static int es_aceptor(int estado) { return estado >= 20 && estado <= 35; }

// Traduce un estado aceptor del autómata (interno de este archivo) al
// t_tipo_token público correspondiente (el que main.c y el resto del mundo
// conocen, definido en scanner.h). Solo tiene sentido llamarla con un
// estado para el que es_aceptor() da verdadero.
static t_tipo_token token_de_estado(int estado) {
  switch (estado) {
    // Tanto un entero puro como un decimal terminan siendo el mismo
    // TOKEN_NUMERO: la diferencia entre "3" y "3.14" está en el
    // lexema (el texto), no en el tipo de token.
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
    // El único caso que queda (dado que esta función solo se llama con
    // estados aceptores) es EST_FDT.
    default:
      return TOKEN_FDT; /* EST_FDT */
  }
}

// Traduce un carácter crudo (int, porque puede valer EOF, que no entra
// en un char) a su t_clase de columna dentro de la tabla.
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
      // Todo lo que no matcheó nada de arriba: si es espacio en blanco
      // (isspace cubre ' ', '\t', '\n', '\r', etc.) es CLASE_ESPACIO; si
      // no, es un carácter que el lenguaje de la calculadora no conoce.
      if (isspace(c)) return CLASE_ESPACIO;
      return CLASE_OTRO;
  }
}

// FILE* del que se está leyendo actualmente. main.c lo cambia una vez
// por línea (ver scanner_iniciar), así que no hace falta que sea más
// que un simple puntero global privado de este archivo.
static FILE* fuente;
// Buffer estático que guarda el texto exacto del último token
// reconocido (el "lexema"). Es el buffer que pide la consigna del TP2 y
// el que devuelve scanner_lexema().
static char lexema[TAM_LEXEMA];

void scanner_iniciar(FILE* entrada) { fuente = entrada; }

// Mira el próximo carácter de 'fuente' SIN consumirlo: lo lee y
// enseguida lo devuelve con ungetc para que quede disponible para la
// próxima lectura. Así se puede decidir qué transición corresponde
// antes de comprometerse a avanzar sobre ese carácter.
static int peek(void) {
  int c = fgetc(fuente);
  if (c != EOF) ungetc(c, fuente);
  return c;
}

// Lee y consume (a diferencia de peek) el próximo carácter de 'fuente'.
static int avanzar(void) { return fgetc(fuente); }

// Corazón del escáner: reconoce y devuelve UN token, recorriendo el
// autómata de tabla.md carácter por carácter.
t_tipo_token scanner_siguiente_token(void) {
  // Arranca siempre desde el estado inicial del autómata.
  int estado = EST_INICIAL;
  // Cuántos caracteres del lexema actual ya se guardaron en el buffer.
  size_t pos = 0;

  // Un ciclo por cada carácter que se examina (no necesariamente se
  // consume: ver más abajo el caso en el que se corta sin avanzar).
  for (;;) {
    // Se mira el próximo carácter sin consumirlo todavía: la decisión de
    // si conviene consumirlo depende de qué transición exista para él.
    int c = peek();
    t_clase clase = clasificar(c);
    // Si el estado actual no tiene fila propia en la tabla (es un
    // estado terminal), directamente se lo trata como si la tabla
    // dijera EST_ERROR para cualquier clase: nunca hace falta mirar
    // realmente esa fila porque no existe ninguna transición posible
    // desde un estado terminal.
    int siguiente = tiene_fila(estado) ? tabla[estado][clase] : EST_ERROR;

    // Caso especial: estamos en el estado inicial y lo que sigue es un
    // espacio en blanco. No es parte de ningún token, así que se
    // descarta (se consume pero no se guarda en el lexema) y se sigue
    // esperando el próximo token desde el mismo estado inicial.
    if (estado == EST_INICIAL && siguiente == EST_INICIAL) {
      avanzar(); /* descarta espacios en blanco */
      continue;
    }

    // No hay transición válida desde el estado actual con este
    // carácter.
    if (siguiente == EST_ERROR) {
      // Si el estado en el que estábamos YA era aceptor, no es un error:
      // simplemente significa que el token anterior ya estaba completo,
      // y el carácter que se acaba de mirar (pero no consumir, gracias a
      // peek) es en realidad el primero del PRÓXIMO token. Por eso se
      // corta el ciclo sin llamar a avanzar(): ese carácter tiene que
      // seguir disponible para la siguiente llamada a esta función.
      if (es_aceptor(estado)) break; /* token completo, no consumir c */

      /* error léxico real: se consume el carácter ofensivo para
       * informarlo, salvo que sea espacio en blanco o EOF, que no
       * forman parte del error y quedan disponibles para el próximo
       * token. */
      if (c != EOF && clase != CLASE_ESPACIO && pos < TAM_LEXEMA - 1) {
        lexema[pos++] = (char)avanzar();
      }
      lexema[pos] = '\0';
      // El tipo de error depende de en qué estado nos agarró: si
      // veníamos de un punto sin dígitos (EST_PUNTO_PENDIENTE), es un
      // número mal formado; cualquier otro caso no aceptor es un
      // carácter que directamente no arranca ningún token conocido.
      return (estado == EST_PUNTO_PENDIENTE) ? ERROR_NUMERO_MAL_FORMADO
                                             : ERROR_CARACTER_INVALIDO;
    }

    // Hay una transición válida: se consume de verdad el carácter (esta
    // vez avanzar() sí mueve el cursor de lectura), se lo agrega al
    // lexema que se está armando, y se actualiza el estado del
    // autómata.
    avanzar();
    if (c != EOF && pos < TAM_LEXEMA - 1) lexema[pos++] = (char)c;
    estado = siguiente;

    // Si el nuevo estado es terminal, no tiene sentido seguir el ciclo:
    // no hay ninguna forma de que un carácter más lo extienda, así que
    // el token ya está completo.
    if (!tiene_fila(estado)) break; /* estado terminal: token completo */
  }

  lexema[pos] = '\0';
  return token_de_estado(estado);
}

const char* scanner_lexema(void) { return lexema; }

// Devuelve, para cada t_tipo_token, el nombre de su constante como texto
// (sin el prefijo "TOKEN_"/"ERROR_" para los tokens válidos, con el
// prefijo completo para los errores), pensado para que main.c lo
// imprima directamente por consola.
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
  // Primero se descartan los espacios y tabuladores que puedan quedar
  // entre el último token y el final de la línea (por ejemplo "2 + 3   ").
  // El '\n' NO entra en este lazo: es justamente la marca que se busca.
  int c = peek();
  while (c != EOF && c != '\n' && isspace(c)) {
    avanzar();
    c = peek();
  }

  // Si lo que sigue es el fin de la entrada, no hay más líneas: se informa
  // como fin de línea para que main.c muestre un último prompt antes del
  // token TOKEN_FDT.
  if (c == EOF) return true;

  // Si sigue un '\n', se lo consume y se confirma que la línea terminó.
  if (c == '\n') {
    avanzar();
    return true;
  }

  // Cualquier otra cosa es el comienzo de otro token en la misma línea.
  return false;
}
