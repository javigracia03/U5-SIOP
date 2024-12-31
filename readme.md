# Simulador de Sistema de Archivos Ext

Este proyecto implementa un simulador de un sistema de archivos basado en EXT, diseñado para realizar diversas operaciones sobre archivos, como listado, copiado, borrado, renombrado, entre otros. También permite manipular estructuras internas como los bytemaps y bloques de datos.

## Funcionalidades

El programa soporta los siguientes comandos:

### 1. `info`

Muestra información del superbloque, incluyendo:

- Total de inodos.
- Total de bloques.
- Bloques libres.
- Inodos libres.
- Primer bloque de datos.

### 2. `bytemaps`

Muestra:

- El contenido del bytemap de inodos.
- Los primeros 25 elementos del bytemap de bloques.

### 3. `dir`

Lista los archivos existentes en el sistema de archivos con el siguiente formato:

- Nombre del archivo.
- Tamaño del archivo (en bytes).
- Número del inodo asociado.
- Bloques ocupados por el archivo.

### 4. `rename <archivo_original> <archivo_nuevo>`

Renombra un archivo en el sistema de archivos.

### 5. `imprimir <archivo>`

Muestra el contenido de un archivo especificado por su nombre.

### 6. `copiar <archivo_origen> <archivo_destino>`

Copia un archivo existente a un nuevo archivo dentro del sistema de archivos, asignando nuevos bloques y un nuevo inodo para el archivo destino.

### 7. `borrar <archivo>`

Elimina un archivo del sistema de archivos. Esto incluye:

- Marcar los bloques ocupados por el archivo como libres.
- Liberar el inodo asociado.
- Eliminar la entrada del directorio.

### 8. `salir`

Termina la ejecución del programa.

## Estructuras de Datos

El sistema de archivos se organiza utilizando las siguientes estructuras principales:

### 1. **Superbloque**

Contiene metadatos del sistema de archivos, como la cantidad de bloques e inodos.

### 2. **Bytemaps**

- Bytemap de inodos: Indica qué inodos están ocupados o libres.
- Bytemap de bloques: Indica qué bloques están ocupados o libres.

### 3. **Inodos**

Cada inodo almacena información sobre un archivo, como:

- Tamaño del archivo.
- Bloques asignados.

### 4. **Directorio**

Almacena las entradas de los archivos en el sistema. Cada entrada incluye:

- Nombre del archivo.
- Número del inodo asociado.

## Requisitos del Sistema

- Lenguaje: C.
- Compilador: GCC.
- Archivo binario de partición: `particion.bin` (debe estar disponible para ejecutar el programa).

## Uso

1. Compila el programa:

   ```bash
   gcc simul_ext.c -o simul_ext
   ```

2. Ejecuta el programa:

   ```bash
   ./simul_ext
   ```

3. Ingresa los comandos desde el prompt para interactuar con el sistema de archivos.

## Notas de Desarrollo

- El comando `bytemaps` es útil para depurar el estado de los inodos y bloques durante el desarrollo.
- Los bloques marcados como libres no se sobrescriben al borrar un archivo, cumpliendo con la integridad de datos.
- La funcionalidad de copiado asigna nuevos bloques libres para el archivo destino.

## Ejemplo de Uso

### Sesión de Ejemplo

```bash
>> info
Inodos totales: 10
Bloques totales: 100
Bloques libres: 90
Inodos libres: 8
Primer bloque de datos: 4

>> bytemaps
Inodos : 1 1 0 0 0 0 0 0 0 0
Bloques [0-24] : 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

>> dir
Archivo         Tamaño     Inodo      Bloques   
================================================
file1.txt       128        1          2 3       
file2.txt       64         2          4         

>> imprimir file1.txt
Contenido del archivo file1.txt...

>> copiar file1.txt file3.txt
Archivo copiado correctamente.

>> borrar file2.txt
Archivo borrado correctamente.

>> salir
```

