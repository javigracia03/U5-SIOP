#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

// Prototipos de funciones
void PrintBytemaps(EXT_BYTE_MAPS *ext_bytemaps);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
void Info(EXT_SIMPLE_SUPERBLOCK *superblock);
int Renombrar(EXT_ENTRADA_DIR *directorio, char *nombre_antiguo, char *nombre_nuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);

int main() {
    char comando[LONGITUD_COMANDO];
    char orden[LONGITUD_COMANDO];
    char argumento1[LONGITUD_COMANDO];
    char argumento2[LONGITUD_COMANDO];

    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];

    FILE *fent = fopen("particion.bin", "r+b");
    if (fent == NULL) {
        perror("Error al abrir particion.bin");
        return 1;
    }

    // Leer la partición completa en memoria
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);

    // Cargar estructuras desde la partición
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BYTE_MAPS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], sizeof(EXT_ENTRADA_DIR) * MAX_FICHEROS);
    memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    // Bucle principal
    while (1) {
        printf(">> ");
        fflush(stdin);
        fgets(comando, LONGITUD_COMANDO, stdin);

        // Parsear comando
        sscanf(comando, "%s %s %s", orden, argumento1, argumento2);

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
        } else if (strcmp(orden, "salir") == 0) {
            fclose(fent);
            break;
        } else {
            printf("Comando desconocido.\n");
        }
    }

    return 0;
}

// Función para mostrar información del superbloque
void Info(EXT_SIMPLE_SUPERBLOCK *superblock) {
    printf("Inodos totales: %u\n", superblock->s_inodes_count);
    printf("Bloques totales: %u\n", superblock->s_blocks_count);
    printf("Bloques libres: %u\n", superblock->s_free_blocks_count);
    printf("Inodos libres: %u\n", superblock->s_free_inodes_count);
    printf("Primer bloque de datos: %u\n", superblock->s_first_data_block);
}

// Función para mostrar los bytemaps
void PrintBytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Bytemap de bloques:\n");
    for (int i = 0; i < MAX_BLOQUES_PARTICION; i++) {
        printf("%d", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\nBytemap de inodos:\n");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
}

// Función para listar directorio
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            printf("Archivo: %s\n", directorio[i].dir_nfich);
            printf("Inodo: %u\n", directorio[i].dir_inodo);
        }
    }
}

// Función para renombrar archivo
int Renombrar(EXT_ENTRADA_DIR *directorio, char *nombre_antiguo, char *nombre_nuevo) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_antiguo) == 0) {
            strcpy(directorio[i].dir_nfich, nombre_nuevo);
            return 0; // Éxito
        }
    }
    return -1; // Error: archivo no encontrado
}

// Función para imprimir archivo
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            int inodo_idx = directorio[i].dir_inodo;
            if (inodo_idx == NULL_INODO) {
                return -1; // Error: inodo no válido
            }

            EXT_SIMPLE_INODE inodo = inodos->blq_inodos[inodo_idx];
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodo.i_nbloque[j] != NULL_BLOQUE) {
                    fwrite(memdatos[inodo.i_nbloque[j]].dato, SIZE_BLOQUE, 1, stdout);
                }
            }
            printf("\n");
            return 0; // Éxito
        }
    }
    return -1; // Error: archivo no encontrado
}
