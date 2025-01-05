#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "serial.h"

#define TIMEOUT_USEC 10000  // Timeout para lectura en microsegundos
#define PINZA_ABIERTA 215   // Valor para la pinza abierta
#define PINZA_CERRADA 115   // Valor para la pinza abierta

// Estructura para almacenar los datos recibidos del robot
typedef struct {
    int angulo_giro;
    int angulo_brazo;
    int angulo_antebrazo;
    int angulo_muneca_a;
    int angulo_muneca_b;
    int distancia_pinza;
    int pos_x;
    int pos_y;
    int pos_z;
} EstadoRobot;

//Variables globales
int inicial_x;
int inicial_y;
int inicial_z;
int aux = 0;

//Creacion de la variable donde se guardará el estado del robot en cada iteración
EstadoRobot estado;

//Creación de la variable archivo para copiar la recepción
FILE *archivo;

// Declaración de las funciones utilizadas
void recibir_datos(int fd, FILE *archivo);
void guardarEstadoEnArchivo(FILE *archivo);
void enviar_comando_robot(int fd, char comando);
void mover_a_posicion(int fd, int objetivo_x, int objetivo_y, int objetivo_z);
void girar_muneca(int fd, int angulo);
void abrir_y_cerrar_mano(int fd);
void volver_a_posicion_original(int fd, int inicial_x, int inicial_y, int inicial_z);
char elegir_direccion_giro();

int mostrar_menu();


int main()
{
    char direccion;
    int angulo;
	//Apertura del puerto serie
	int fd = serial_open("/dev/ttyS0", B9600);
    if (fd == -1) {
        printf("Error al abrir el puerto serie\n");
        return 1;
    }
	
    // Abrir el archivo una vez en modo "append"
   	archivo = fopen("posiciones.txt", "w");
   	
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }
    
    //Envio movimiento inicial
    enviar_comando_robot(fd, 'M');
    
    // Guardar la posición inicial
    // printf("%d\n",inicial_x);
    // printf("%d\n",inicial_y);
    // printf("%d\n",inicial_z);
      
    int opcion;
    
    do {
        opcion = mostrar_menu();
        switch (opcion) {
        	
            case 1: { // Mover a posición escogida por el usuario
                int x, y, z;
                printf("Introduce la posición objetivo (X Y Z): ");
                scanf("%d %d %d", &x, &y, &z);
                mover_a_posicion(fd, x, y, z);
                break;
            }
            
            case 2: // Giro de 180º de la muñeca, en sentido horario o antihorario dependiendo de la elección del usuario
                direccion = elegir_direccion_giro();
                if (direccion == 'H') {
		    		if(estado.angulo_muneca_b < 360){
						angulo = estado.angulo_muneca_b + 180;
		    		} else {
					angulo = 360;
		    		}	
                } else if (direccion == 'A') {
		    		if(estado.angulo_muneca_b > 0){
						angulo = estado.angulo_muneca_b - 180;
		    		} else {
						angulo = 0;
		    		}
                }
				girar_muneca(fd, angulo);
                break;
                
            case 3: // Abrir y cerrar la pinza
                abrir_y_cerrar_mano(fd);
                break;
                
            case 4: // Volver a la posición inicial con el mismo orden de movimiento
                volver_a_posicion_original(fd, inicial_x, inicial_y, inicial_z);
                break;
                
            case 5: // Salida del programa
                printf("Saliendo del programa...\n");
                break;
                
            default: // Caso de opción no valida
                printf("Opción no válida. Inténtalo de nuevo.\n");
        }
    } while (opcion != 5);

    // Cerrar puerto serie
    serial_close(fd);
    fclose(archivo);
    return 0;
}

// Menu de inicio de la aplicación
int mostrar_menu() {
    int opcion;
    printf("\n--- Menú de opciones ---\n");
    printf("1. Mover a posición (X, Y, Z)\n");
    printf("2. Girar muñeca 180º\n");
    printf("3. Abrir y cerrar pinzas\n");
    printf("4. Volver a la posición inicial\n");
    printf("5. Salir\n");
    printf("Selecciona una opción: ");
    scanf("%d", &opcion);
    return opcion;
}

 // Elección del sentido de giro si se escoge el giro de muñeca
char elegir_direccion_giro() {
    char direccion;
    do {
        printf("Elige el sentido de giro: \n");
        printf("H para horario, A para antihorario: ");
        scanf(" %c", &direccion);
        if (direccion != 'H' && direccion != 'A') {
            printf("Opción no válida. Inténtalo de nuevo.\n");
        }
    } while (direccion != 'H' && direccion != 'A');
    return direccion;
}

 // Función para llevar a cabo la recepción de los datos
void recibir_datos(int fd, FILE *archivo)
{	
	char cadena [256];
	char *dato;
	
	serial_read(fd, cadena, 256 - 1, TIMEOUT_USEC); //Lectura de datos por el puerto serie
	
	// Usamos strtok para dividir la cadena y atoi para convertir a entero
    dato = strtok(cadena, ",");
    if (dato != NULL) estado.angulo_giro = atoi(dato);

	dato = strtok(NULL, ",");
	if (dato != NULL) estado.angulo_brazo = atoi(dato);
	
	dato = strtok(NULL, ",");
	if (dato != NULL) estado.angulo_antebrazo = atoi(dato);
	
	dato = strtok(NULL, ",");
	if (dato != NULL) estado.angulo_muneca_a = atoi(dato);
	
	dato = strtok(NULL, ",");
	if (dato != NULL) estado.angulo_muneca_b = atoi(dato);
	
	dato = strtok(NULL, ",");
	if (dato != NULL) estado.distancia_pinza = atoi(dato);
	
	dato = strtok(NULL, ",");
	if (dato != NULL) estado.pos_x = atoi(dato);
	
	dato = strtok(NULL, ",");
	if (dato != NULL) estado.pos_y = atoi(dato);
	
	dato = strtok(NULL, ",");
	if (dato != NULL) estado.pos_z = atoi(dato);
	    
	    
	// Muestra los valores almacenados en la estructura
	printf("Valores almacenados en la estructura:\n");
	printf("Angulo Giro: %d\n", estado.angulo_giro);
	printf("Angulo Brazo: %d\n", estado.angulo_brazo);
	printf("Angulo Antebrazo: %d\n", estado.angulo_antebrazo);
	printf("Angulo Muñeca A: %d\n", estado.angulo_muneca_a);
	printf("Angulo Muñeca B: %d\n", estado.angulo_muneca_b);
	printf("Distancia Pinza: %d\n", estado.distancia_pinza);
	printf("Posición X: %d\n", estado.pos_x);
	printf("Posición Y: %d\n", estado.pos_y);
	printf("Posición Z: %d\n", estado.pos_z);
	
	// Guarda las posiciones iniciales del robot
	if(aux == 0){
	    inicial_x = estado.pos_x;
	    inicial_y = estado.pos_y;
	    inicial_z = estado.pos_z;
	    aux = 1;
	}
		
	// Guardar los valores en el archivo
    guardarEstadoEnArchivo(archivo);
}


//Guarda en el archivo la informacion recibida
void guardarEstadoEnArchivo(FILE *archivo)
{
	if (archivo == NULL) {
        perror("Error: archivo no válido");
        return;
    }

    // Escribir los valores en una sola línea, separados por comas
    fprintf(archivo, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            estado.angulo_giro,
            estado.angulo_brazo,
            estado.angulo_antebrazo,
            estado.angulo_muneca_a,
            estado.angulo_muneca_b,
            estado.distancia_pinza,
            estado.pos_x,
            estado.pos_y,
            estado.pos_z);
}

// Función para enviar un comando simple al robot
void enviar_comando_robot(int fd, char comando) {
    //char cmd[2] = {comando, '\0'};
    serial_send(fd, &comando, 1);
    usleep(50000);
    recibir_datos(fd, archivo);
}


// Mover el robot a una posición específica
void mover_a_posicion(int fd, int objetivo_x, int objetivo_y, int objetivo_z) {
	
	//Movimiento en X
    while (estado.pos_x != objetivo_x) {
        if (estado.pos_x < objetivo_x) {
            enviar_comando_robot(fd, 'D');
        } else {
            enviar_comando_robot(fd, 'A');
        }
    }
	
	//Movimiento en Y
    while (estado.pos_y != objetivo_y) {
        if (estado.pos_y < objetivo_y) {
            enviar_comando_robot(fd, 'W');
        } else {
            enviar_comando_robot(fd, 'S');
        }
    }

	//Movimiento en Z
    while (estado.pos_z != objetivo_z) {
        if (estado.pos_z < objetivo_z) {
            enviar_comando_robot(fd, 'E');
        } else {
            enviar_comando_robot(fd, 'Q');
        }
    }
}

// Girar la muñeca del robot
void girar_muneca(int fd, int angulo) {
    while (estado.angulo_muneca_b != angulo) {
        if (estado.angulo_muneca_b < angulo) {
            enviar_comando_robot(fd, 'C');  // Giro horario
        } else {
            enviar_comando_robot(fd, 'V');  // Giro antihorario
        }
    }
}

// Abrir y cerrar las pinzas
void abrir_y_cerrar_mano(int fd) {
	
	while(estado.distancia_pinza < PINZA_ABIERTA){
    	enviar_comando_robot(fd, 'M');  // Abrir pinzas
	}
	
	while(estado.distancia_pinza > PINZA_CERRADA){
    	enviar_comando_robot(fd, 'N');  // Cerrar pinzas
	}
}

// Volver a la posición original
void volver_a_posicion_original(int fd, int objetivo_x, int objetivo_y, int objetivo_z) {
	
	//Movimiento en Z
	while (estado.pos_z != objetivo_z) {
    	if (estado.pos_z < objetivo_z) {
        	enviar_comando_robot(fd, 'E');
    	} else {
        	enviar_comando_robot(fd, 'Q');
    	}
    }
	
	//Movimiento en Y
    while (estado.pos_y != objetivo_y) {
        if (estado.pos_y < objetivo_y) {
            enviar_comando_robot(fd, 'W');
        } else {
            enviar_comando_robot(fd, 'S');
        }
    }
    
    //Movimiento en X
    while (estado.pos_x != objetivo_x) {
        if (estado.pos_x < objetivo_x) {
            enviar_comando_robot(fd, 'D');
        } else {
            enviar_comando_robot(fd, 'A');
        }
    }
}

