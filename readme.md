# Calculadora-C

TP2 de Compiladores/Lenguajes Formales: escáner (analizador léxico) para
la calculadora definida en el TP1.

## Contenido

- [scanner.h](scanner.h) — tokens reconocidos, buffer de lexema y funciones públicas del escáner.
- [scanner.c](scanner.c) — implementación del escáner mediante una tabla de transición.
- [tabla.md](tabla.md) — documentación de la tabla de transición: estados, columnas y tokens/errores producidos.
- [main.c](main.c) — programa de prueba: lee de `stdin` y reporta cada token (o error) hasta encontrar FDT.

## Compilar y correr

```sh
make
./calc
```

Termina el ciclo al ingresar EOF (Ctrl+D en Linux/macOS, Ctrl+Z + Enter en Windows).

También se puede probar contra un archivo:

```sh
./calc < entrada.txt
```
