#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cabeceras.h" // Archivo con definiciones y estructuras necesarias

#define LONGITUD_COMANDO 100 // Longitud máxima permitida para un comando

// Prototipos de funciones para organizar mejor el código
void PrintBytemaps(EXT_BYTE_MAPS *ext_bytemaps);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
void Info(EXT_SIMPLE_SUPERBLOCK *superblock);
int Renombrar(EXT_ENTRADA_DIR *directorio, char *nombre_antiguo, char *nombre_nuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, EXT_DATOS *memdatos, char *nombre_origen, char *nombre_destino);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, char *nombre);

// Función principal que ejecuta el ciclo de vida del programa
int main() {
    char comando[LONGITUD_COMANDO]; // Almacena el comando completo ingresado por el usuario
    char orden[LONGITUD_COMANDO];  // Almacena la acción principal del comando
    char argumento1[LONGITUD_COMANDO]; // Primer argumento del comando
    char argumento2[LONGITUD_COMANDO]; // Segundo argumento del comando

    // Estructuras principales que representan el sistema de archivos
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];

    // Abrimos el archivo binario que representa la partición
    FILE *fent = fopen("particion.bin", "r+b");
    if (fent == NULL) {
        perror("Error al abrir particion.bin");
        return 1;
    }

    // Cargar la partición completa en memoria
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);

    // Mapear los datos leídos en estructuras concretas del sistema de archivos
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE); // Superbock
    memcpy(&ext_bytemaps, (EXT_BYTE_MAPS *)&datosfich[1], SIZE_BLOQUE);           // Bytemaps
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);        // Tabla de inodos
    memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], sizeof(EXT_ENTRADA_DIR) * MAX_FICHEROS); // Directorio
    memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE); // Bloques de datos

    // Bucle principal para procesar comandos del usuario
    while (1) {
        printf(">> ");
        fflush(stdin); // Asegura que el buffer de entrada está limpio
        fgets(comando, LONGITUD_COMANDO, stdin); // Leer comando del usuario

        // Dividir el comando en orden y argumentos
        sscanf(comando, "%s %s %s", orden, argumento1, argumento2);

        // Procesar los diferentes comandos ingresados por el usuario
        if (strcmp(orden, "info") == 0) {
            Info(&ext_superblock);
        } else if (strcmp(orden, "bytemaps") == 0) {
            PrintBytemaps(&ext_bytemaps);
        } else if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        } else if (strcmp(orden, "rename") == 0) {
            if (Renombrar(directorio, argumento1, argumento2) == 0) {
                printf("Archivo renombrado correctamente.\n");
            } else {
                printf("Error al renombrar archivo.\n");
            }
        } else if (strcmp(orden, "imprimir") == 0) {
            if (Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1) == 0) {
                printf("Archivo impreso correctamente.\n");
            } else {
                printf("Error al imprimir archivo.\n");
            }
        } else if (strcmp(orden, "copy") == 0) {
            if (Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2) == 0) {
                printf("Archivo copiado correctamente.\n");
            } else {
                printf("Error al copiar archivo.\n");
            }
        } else if (strcmp(orden, "remove") == 0) {
            if (Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1) == 0) {
                printf("Archivo borrado correctamente.\n");
            } else {
                printf("Error al borrar archivo.\n");
            }
        } else if (strcmp(orden, "salir") == 0) {
            fclose(fent); // Cerramos el archivo antes de salir
            break;
        } else {
            printf("Comando desconocido.\n");
        }
    }

    return 0;
}

// Función que muestra información general del sistema de archivos
void Info(EXT_SIMPLE_SUPERBLOCK *superblock) {
    printf("Inodos totales: %u\n", superblock->s_inodes_count);
    printf("Bloques totales: %u\n", superblock->s_blocks_count);
    printf("Bloques libres: %u\n", superblock->s_free_blocks_count);
    printf("Inodos libres: %u\n", superblock->s_free_inodes_count);
    printf("Primer bloque de datos: %u\n", superblock->s_first_data_block);
}

// Función que imprime el mapa de bits de bloques e inodos
void PrintBytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Inodos :");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf(" %d", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");

    printf("Bloques [0-24] :");
    for (int i = 0; i < 25; i++) {
        printf(" %d", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

// Función que lista los archivos en el directorio y sus detalles
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    printf("%-15s %-10s %-10s %-10s", "Archivo", "Tama\u00f1o", "Inodo", "Bloques");
    printf("\n");
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[directorio[i].dir_inodo];
            printf("%-15s %-10d %-10d ", directorio[i].dir_nfich, inode->size_fichero, directorio[i].dir_inodo);
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inode->i_nbloque[j] != NULL_BLOQUE) {
                    printf("%d ", inode->i_nbloque[j]);
                }
            }
            printf("\n");
        }
    }
    printf("\n");
}

// Función para renombrar un archivo
int Renombrar(EXT_ENTRADA_DIR *directorio, char *nombre_antiguo, char *nombre_nuevo) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_antiguo) == 0) {
            strcpy(directorio[i].dir_nfich, nombre_nuevo);
            return 0; // Éxito
        }
    }
    return -1; // Error: archivo no encontrado
}

// Función para imprimir el contenido de un archivo
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    int inodo_archivo = -1;

    // Buscar el archivo en el directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            inodo_archivo = directorio[i].dir_inodo;
            break;
        }
    }

    if (inodo_archivo == -1) {
        printf("Archivo no encontrado.\n");
        return -1;
    }

    // Obtener el inodo correspondiente
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodo_archivo];

    // Verificar si el tamaño del archivo es válido
    if (inodo->size_fichero == 0) {
        printf("El archivo '%s' está vacío.\n", nombre);
        return 0;
    }

    // Asignar memoria dinámica para el contenido del archivo
    char *buffer = malloc(inodo->size_fichero + 1); // +1 para el terminador nulo
    if (!buffer) {
        perror("Error al asignar memoria");
        return -1;
    }
    memset(buffer, 0, inodo->size_fichero + 1); // Inicializar el buffer

    int offset = 0; // Variable para manejar el offset dentro del buffer
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] == NULL_BLOQUE) {
            continue;
        }

        int bloque_datos = inodo->i_nbloque[i] - PRIM_BLOQUE_DATOS;

        // Validar que el bloque está dentro de los límites
        if (bloque_datos < 0 || bloque_datos >= MAX_BLOQUES_DATOS) {
            printf("Error: Bloque inválido %d en el archivo '%s'.\n", inodo->i_nbloque[i], nombre);
            free(buffer);
            return -1;
        }

        // Copiar bloque de datos al buffer
        int bytes_restantes = inodo->size_fichero - offset;
        int bytes_a_copiar = bytes_restantes < SIZE_BLOQUE ? bytes_restantes : SIZE_BLOQUE;

        memcpy(buffer + offset, memdatos[bloque_datos].dato, bytes_a_copiar);
        offset += bytes_a_copiar;

        // Si se ha copiado todo el contenido del archivo, detenerse
        if (offset >= inodo->size_fichero) {
            break;
        }
    }

    // Asegurarse de que el contenido termina con '\0'
    buffer[inodo->size_fichero] = '\0';
    // Imprimir el contenido del archivo
    printf("Contenido del archivo '%s':\n%s\n", nombre, buffer);

    // Liberar la memoria asignada
    free(buffer);

    return 0;
}

// Función para borrar un archivo
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, char *nombre) {
    int inodo_borrar = -1; // Inodo del archivo a borrar

    // Comprobar que el archivo existe en el directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            inodo_borrar = directorio[i].dir_inodo;

            // Limpiar la entrada del directorio
            directorio[i].dir_inodo = NULL_INODO;
            strcpy(directorio[i].dir_nfich, ""); // Vaciar el nombre del archivo
            break;
        }
    }

    if (inodo_borrar == -1) {
        // Si no se encuentra el archivo, informar al usuario
        printf("Archivo no encontrado.\n");
        return -1;
    }

    // Liberar recursos asociados al inodo
    EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[inodo_borrar];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inode->i_nbloque[i] != NULL_BLOQUE) {
            int bloque_datos = inode->i_nbloque[i] - PRIM_BLOQUE_DATOS;

            // Validar que el bloque está dentro de los límites
            if (bloque_datos < 0 || bloque_datos >= MAX_BLOQUES_DATOS) {
                printf("Error: Bloque inválido %d en el inodo %d.\n", inode->i_nbloque[i], inodo_borrar);
                continue; // Continuar liberando otros bloques
            }

            // Marcar el bloque como libre en el mapa de bits
            ext_bytemaps->bmap_bloques[bloque_datos] = 0;
            inode->i_nbloque[i] = NULL_BLOQUE; // Desasociar el bloque del inodo
        }
    }

    // Marcar el inodo como libre en el mapa de bits de inodos
    ext_bytemaps->bmap_inodos[inodo_borrar] = 0;

    // Restablecer el tamaño del archivo a 0
    inode->size_fichero = 0;

    // Confirmar que el archivo ha sido eliminado correctamente
    printf("Archivo '%s' borrado correctamente.\n", nombre);

    return 0; // Operación exitosa
}


// Función para copiar un archivo dentro del sistema de archivos
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, EXT_DATOS *memdatos, char *nombre_origen, char *nombre_destino) {
    int inodo_origen = -1;  // Inodo del archivo origen
    int inodo_destino = -1; // Inodo del archivo destino

    // Buscar el archivo origen en el directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_origen) == 0) {
            inodo_origen = directorio[i].dir_inodo;
            break;
        }
    }

    if (inodo_origen == -1) {
        printf("Archivo origen no encontrado.\n");
        return -1;
    }

    // Buscar un inodo libre para el archivo destino
    for (int i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            inodo_destino = i;
            ext_bytemaps->bmap_inodos[i] = 1; // Marcar el inodo como ocupado
            break;
        }
    }

    if (inodo_destino == -1) {
        printf("No hay inodos disponibles para el archivo destino.\n");
        return -1;
    }

    // Inicializar el inodo destino
    EXT_SIMPLE_INODE *inode_origen = &inodos->blq_inodos[inodo_origen];
    EXT_SIMPLE_INODE *inode_destino = &inodos->blq_inodos[inodo_destino];
    inode_destino->size_fichero = inode_origen->size_fichero; // Copiar tamaño del archivo

    // Copiar los bloques de datos
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inode_origen->i_nbloque[i] != NULL_BLOQUE) {
            int bloque_origen = inode_origen->i_nbloque[i] - PRIM_BLOQUE_DATOS;

            // Validar que el bloque origen es válido
            if (bloque_origen < 0 || bloque_origen >= MAX_BLOQUES_DATOS) {
                printf("Error: Bloque origen inválido %d.\n", inode_origen->i_nbloque[i]);
                return -1;
            }

            // Buscar un bloque libre para asignar
            int bloque_libre = -1;
            for (int j = 0; j < MAX_BLOQUES_DATOS; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    bloque_libre = j;
                    ext_bytemaps->bmap_bloques[j] = 1; // Marcar el bloque como ocupado
                    break;
                }
            }

            if (bloque_libre == -1) {
                printf("No hay bloques de datos disponibles.\n");
                return -1;
            }

            // Asignar el bloque al inodo destino
            inode_destino->i_nbloque[i] = bloque_libre + PRIM_BLOQUE_DATOS;

            // Copiar datos del bloque origen al bloque destino
            memcpy(memdatos[bloque_libre].dato, memdatos[bloque_origen].dato, SIZE_BLOQUE);
        } else {
            inode_destino->i_nbloque[i] = NULL_BLOQUE; // Marcar como bloque no asignado
        }
    }

    // Agregar el archivo destino al directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            directorio[i].dir_inodo = inodo_destino;
            strcpy(directorio[i].dir_nfich, nombre_destino);
            break;
        }
    }

    return 0; // Éxito
}