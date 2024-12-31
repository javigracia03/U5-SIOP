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
void VerificarBloquesUnicos(EXT_BYTE_MAPS *ext_bytemaps, EXT_BLQ_INODOS *inodos, EXT_ENTRADA_DIR *directorio);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, EXT_DATOS *memdatos, char *nombre_origen, char *nombre_destino);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, char *nombre);

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

    // Verificar consistencia de bloques
    VerificarBloquesUnicos(&ext_bytemaps, &ext_blq_inodos, directorio);
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

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    printf("%-15s %-10s %-10s %-10s", "Archivo", "Tamaño", "Inodo", "Bloques");
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
};

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
    EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[inodo_archivo];

    // Concatenar los bloques de datos
    char contenido[MAX_BLOQUES_DATOS * SIZE_BLOQUE + 1];
    memset(contenido, '\0', sizeof(contenido));

    int offset = 0; // Variable para manejar el offset dentro del buffer
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inode->i_nbloque[i] != NULL_BLOQUE && inode->i_nbloque[i] < MAX_BLOQUES_DATOS) {
            // Imprimir el bloque que se está procesando
            printf("Procesando bloque: %d\n", inode->i_nbloque[i]);

            // Copiar bloque de datos al buffer
            memcpy(contenido + offset, memdatos[inode->i_nbloque[i]].dato, SIZE_BLOQUE);
            offset += SIZE_BLOQUE; // Incrementar el offset por el tamaño del bloque

            // Verificar si se ha copiado todo el tamaño del archivo
            if (offset >= inode->size_fichero) {
                break;
            }
        }
    }

    // Asegurarse de que el contenido termina con '\0'
    contenido[inode->size_fichero] = '\0';

    // Imprimir el contenido del archivo
    printf("Contenido del archivo %s:\n%s\n", nombre, contenido);

    return 0;
}


// Verifica que los bloques asignados a inodos sean únicos
void VerificarBloquesUnicos(EXT_BYTE_MAPS *ext_bytemaps, EXT_BLQ_INODOS *inodos, EXT_ENTRADA_DIR *directorio) {
    unsigned char bloques_usados[MAX_BLOQUES_DATOS] = {0};

    for (int i = 0; i < MAX_INODOS; i++) {
        EXT_SIMPLE_INODE inodo = inodos->blq_inodos[i];

        for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
            unsigned short bloque = inodo.i_nbloque[j];
            if (bloque != NULL_BLOQUE) {
                if (bloques_usados[bloque]) {
                    printf("Error: Bloque %d asignado a múltiples inodos.\n", bloque);
                }
                bloques_usados[bloque] = 1;
            }
        }
    }
}

// Función para borrar archivo
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, char *nombre) {
    int inodo_borrar = -1;

    // Buscar el archivo en el directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            inodo_borrar = directorio[i].dir_inodo;
            directorio[i].dir_inodo = NULL_INODO;
            strcpy(directorio[i].dir_nfich, "");
            break;
        }
    }

    if (inodo_borrar == -1) {
        printf("Archivo no encontrado.\n");
        return -1;
    }

    // Liberar bloques asociados al inodo
    EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[inodo_borrar];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inode->i_nbloque[i] != NULL_BLOQUE) {
            ext_bytemaps->bmap_bloques[inode->i_nbloque[i]] = 0;
            inode->i_nbloque[i] = NULL_BLOQUE;
        }
    }

    // Liberar el inodo
    ext_bytemaps->bmap_inodos[inodo_borrar] = 0;
    inode->size_fichero = 0;

    return 0;
}

// Función para copiar archivo
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, EXT_DATOS *memdatos, char *nombre_origen, char *nombre_destino) {
    int inodo_origen = -1;
    int inodo_destino = -1;

    // Buscar archivo origen en el directorio
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
            ext_bytemaps->bmap_inodos[i] = 1;
            break;
        }
    }

    if (inodo_destino == -1) {
        printf("No hay inodos disponibles para el archivo destino.\n");
        return -1;
    }

    // Copiar metadatos del archivo origen al destino
    EXT_SIMPLE_INODE *inode_origen = &inodos->blq_inodos[inodo_origen];
    EXT_SIMPLE_INODE *inode_destino = &inodos->blq_inodos[inodo_destino];

    inode_destino->size_fichero = inode_origen->size_fichero;
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inode_origen->i_nbloque[i] != NULL_BLOQUE) {
            // Buscar un bloque libre
            for (int j = 0; j < MAX_BLOQUES_DATOS; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    ext_bytemaps->bmap_bloques[j] = 1;
                    inode_destino->i_nbloque[i] = j;
                    memcpy(&memdatos[j], &memdatos[inode_origen->i_nbloque[i]], SIZE_BLOQUE);
                    break;
                }
            }
        }
    }

    // Añadir el archivo destino al directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            directorio[i].dir_inodo = inodo_destino;
            strcpy(directorio[i].dir_nfich, nombre_destino);
            break;
        }
    }

    return 0;
}
