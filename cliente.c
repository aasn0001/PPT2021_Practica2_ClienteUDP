/*******************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. Ingeníería de Telecomunicación
 * Universidad de Jaén
 *
 *******************
 * Práctica 1.
 * Fichero: cliente.c
 * Versión: 3.0
 * Fecha: 09/2020
 * Descripción:
 * Cliente sencillo TCP para IPv4 e IPv6
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************
 * Alumno 1:Alejandro Antonio Salvador Navarro
 ******************/

#include <stdio.h>
#include <ws2tcpip.h>//Necesaria para las funciones IPv6
#include <conio.h>
#include "protocol.h"

 //------------------------------------------------------------------------------------------------------------------------------
 // COMENTARIOS PREVIOS
 //------------------------------------------------------------------------------------------------------------------------------
  /*En esta práctica, lo que tenemos es una comunicación SMTP.
  La estructura viene dada por una máquina de estados que podemos utilizar de la práctica 1 de un cliente TCP.
  La máquina de estados viene definida por BIENVENIDA, HELO, MAIL, RCPT, DATA, MENSAJE, FIN
  También, tenemos un estado QUIT que solamente no lo podremos usar en DATA y MENSAJE*/


  /*Lo primero que tenemos es hacer un cliente en c con la primitiva socket.
  * -Debemos introducir un destinatario para que no vaya perdido.
  * -La interfaz del usuario debe ocultar los comandos, ni pedirle que escriba comandos ni que meta errores 220 ni nada
  * -Los correos no tendrán limitación de tamaño
  * -Se deberá preservar la longitud máxima de línea especificada para SMTP*/


  //------------------------------------------------------------------------------------------------------------------------------
  // ARGOSOFT
  //------------------------------------------------------------------------------------------------------------------------------
  /*En el laboratorio, labtelema tiene un servidor de correo, en casa tenemos un software que es un miniservidor
  * SMTP de correo que se instaura en el puerto 25.
  * Creamos usuarios dandole al muñeco*/

  //------------------------------------------------------------------------------------------------------------------------------
  // ANÁLISIS MÁQUINA DE ESTADOS
  //------------------------------------------------------------------------------------------------------------------------------
  /*Con un cliente telnet podemos conectarnos a un servidor de correo con protocolo SMTPM, POP3 o IMAP
  * Revisamos el cliente de la práctica 1, el servidor lo cambiaremos.
  * La máquina de estados la vamos a analizar con un cliente telnet.
  * Debemos meternos en activar o desactivar propiedades de windows y activar protocolo telnet
  * Accedemos cmd y ponemos telnet y la direccion ip o dominio con el puerto del servidor SMTP (25) ----> telnet labtelema.ujaen.es 25
  * 1- Primero saludamos al servidor. helo y el nombre del host ---> helo mipc          solamente al principio, despues podemos enviar un numero de correos indeterminados
  *    pero no hace falta poner nunca mas helo. Si pongo quit se corta la conexión y ya está
  *    ehlo funciona igual, te da las opciones al principio
  * 2- Remitente, no lo comprueba porque no lo tiene activado ---> mail from: jccuevas
  * 3- rcpt to:ppt01
  * 4- rcpt to:jccuevas      no lo reconocería
  * 5- rcpt to:jccuevas@ujaen.es     vale, da igual no es mío, es externo, pero no lo pongas porque no lo voy a poder enviar hacia fuera
  * 6- rcpt to:ppt02         puede llegar a una o mas personas, si está antes descrito no habrá ningún error
  * 7- data
  * 8- a partir de aquí, todo es contenido. Primero cabeceras, para ello en esta practicas le ponemos subjecto, to y from de la siguiente forma
  * subject:Practica3 2020/2021
  * to:jccuevas
  * from: eeeeeev
  *
  * (enter)
  *
  * Aqui escribimos lo que queramos, es el mensaje
  * . (con un enter y un punto decimos que se acaba el mensaje)
  *
  * 9- Nuevo correo: mail from:jccuevas, rcpt to:ppt1, rcpt to:ppt02.....
  * 10- me salgo con quit
  *
  * ------------------------------------------------------------------------------------------------------------------------------
  * ------------------------------------------------------------------------------------------------------------------------------
  *
  * Para comprobar los correos, podemos configurar un cliente de correos o directamente y más fácil, con pop3, de la siguiente forma
  * 1- telnet labtelema.ujaen.es 110     (este es el puerto pop3)
  * 2- user ppt01
  * 3- pass 123456
  * 4- Una vez autenticado, podemos obtenerlos por----> list (me informa de numero y bytes)
  * 5- rtr y numero de correo ---> rtr 2    (correo numero 2, nos indica las cabeceras, que tambien vemos que el servidor les ha añadido
  * y el mensaje que le hemos escrito
  * 6- rtr 3....
  * 7- para borrar correos ---> dele 2
  * 8- se borran cuando me salga (quit)
  * 9- stat me dice cuantos correos tengo en total y cuantos bytes en total
  * 10- el comando top da las cabeceras y lineas que yo le diga del cuerpo ---> top 2 1    (dame la cabecera del 2 y una linea del cuerpo)
  *
  *
  * La máquina de estados debe ser más completa, en el caso de que no haya errores 220 es para ehlo, 250 para todos los demás y 221 para quit
  * Por lo que hay que asegurarse siempre de que la respuesta del servidor es la correcta, sea el estado que sea
  * También hay que implementar el comando Rset (Ya lo veremos)  sirve para resetearlo
  *
  * Con SMTP nos conectamos, recibimos el saludo (ya lo hace este código), cambiamos la ip por defecto que teníamos y el puerto por defecto
  * cogemos la direccion de labtelema y la ponemos por defecto, para ello miramos ArgoSoft y copiamos direccion local ---> 169.254.129.33 en mi caso.
  * Para el puerto nos vamos a protocol.h y ponemos por defecto 25 que es el puerto SMTP
  *
  * El primer comando que tenemos era HELO y el segundo MAIL FROM:     pues nos vamos a protocol y lo cambiamos en SC y PW
  * La comprobación no es con Ok, para que me cambie de estado tengo que distinguir entre varios estados, para ello, iremos a recibidos y haremos
  * una comprobación de respuestas más detallado.
  *
  * Una vez aquí, haremos correr el cliente y se nos conectará al servidor SMTP de ArGoSoft
  * Si accedo y doy a enter, lo que hace es un quit
  * Si accedo y le pongo host me saludo y ya le pongo yo mail from y despues quit porque rcpt no está programado
  * Ya tendríamos un cliente que habla SMTP
  */

  //------------------------------------------------------------------------------------------------------------------------------
  // GITHUB
  //------------------------------------------------------------------------------------------------------------------------------
  /*Instalamos extensiones github, nos creamos una cuenta y creamos un repositorio privado con un nombre específico, ya está publicado.
  * Para las actualizaciones le damos al lapiz de la esquina inferior a la derecha, ponemos comentario sobre lo que cambiamos y confirmar y guardar todo.
  * Si le damos a master y view history nos dice las actualizaciones y también donde hemos hecho las correcciones
  * La flecha indica que es un cambio local que no está en remoto, le daríamos a insertar y ya estaría en remoto*/




#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculación (linking) la biblioteca Ws2_32.lib

  //*******************

  //Ejecutamos después de ejecutar el servidor, IPV4, enter, usuario, 123456, ya estarían cliente y servidor comunicándose
  //Actúa como un cliente ECO, es decir, enviamos información al servidor y el servidor nos lo devuelve de nuevo

  /*RSET --> Se debería de implementar en aquellos estados en los que el usuario ha introducido datos, por lo que se debe de 
  * implementar en MAIL y en RCPT.*/

int main(int* argc, char* argv[])
{
	SOCKET sockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024], input1[1024], input2[1024], input3[1024];
	int recibidos = 0, enviados = 0;
	int estado;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16] = "192.168.66.229";                         //"169.254.129.33" es la ip para que funcione en casa;
	char default_ip6[64] = "::1";
	int cambio = 0;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	int mensaje = 0;


	//*******************


	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		return(0);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets


	//*******************

	//Menú
	printf("*****\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n****\r\n");

	do {
		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			//Si se introduce 6 se empleará IPv6
			ipversion = AF_INET6;

		}
		else { //Distinto de 6 se elige la versión IPv4
			ipversion = AF_INET;
		}

		//*******************

		//Primitiva SOCKET()--> Se encarga de crear un descriptor socket de memoria.
		//Aquí no está funcionando, ya que solo está reservado. No tiene ni puerto ni IP
		sockfd = socket(ipversion, SOCK_STREAM, 0);
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*)&server_in6;
				address_size = sizeof(server_in6);
			}

			//Cada nueva conexión establece el estado incial en
			estado = S_HELO;

			//*******************

			//Primtiva CONNECT()--> Inicia una conexión con conector remoto hacia la primitiva
			//ACCEPT() de la parte de servidor (le envía una petición de conexión)
			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				//Inicio de la máquina de estados
				do {
					switch (estado) {
					case S_INIT:
						//Estado de bienvenida
						break;

					case S_HELO:
						//Se recibe el mensaje de bienvenida
						printf("Introduce el nombre del host: (enter para salir)");
						//El parámetro host es necesario pedirlo según el guión de prácticas
						gets_s(input, sizeof(input));
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", HE, input, CRLF);

						//Así sale del sistema una vez ha recibido la respuesta del servidor en el caso de que introdujese una cadena vacía
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						break;

						//-----------------------------------------------------------------------------------------------------------------------

					case S_MAIL_FROM: //Identifica al remitente (el que envía el correo)
						printf("CLIENTE> Introduzca el remitente (enter para salir y reset para resetear el sistema): ");
						gets_s(input, sizeof(input));

						/*RSET --> Esta herramienta, lo que nos tiene que permitir es dejar a un lado aquello que estábamos ejecutando en la máquina de
						* comandos. Por lo que sería muy fácil plantearlo cambiando al estado S_HELO en los estados que debería afectar.
						* Tenemos por lo tanto, que implementarlo tanto en MAIL(Para que olvide los datos introducidos) como en RCPT*/
						
						if (strlen(input) == 0) {
							estado = S_QUIT;
							break;
						}

						else{
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", MF, input, CRLF);
						}
						break;

						//-----------------------------------------------------------------------------------------------------------------------

					case S_RCPT_TO: //Identifica al destinatario (al que recibe el correo)
						cambio = 0;
						printf("CLIENTE> Introduzca el correo del destinatario: ");
						gets_s(input, sizeof(input));

						if (strlen(input) == 0) {
							estado = S_QUIT;
							break;
						}

						else{
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", RC, input, CRLF);
						}

						//Para poder enviar un correo a más de un destinatario
						printf("CLIENTE> ¿Quieres enviar el correo a más destinatarios? (S/N): (opcional)");
						gets_s(input, sizeof(input));
						if (strcmp(input, "S") == 0) {
							estado = S_RCPT_TO; //Así, nos olvidariamos de todo lo introducido anteriormente
							break;
						}
						else {
							cambio = 1;
						}
						
						//Función RESET
						printf("CLIENTE> ¿Desea resetear el sistema? Introduzca reset si desea hacerlo. ");
						gets_s(input3, sizeof(input3));

						if (strcmp(input3, "reset") == 0) {
							estado = S_INIT; //Así, nos olvidariamos de todo lo introducido anteriormente
							break;
						}

						break;

						//-----------------------------------------------------------------------------------------------------------------------

					case S_DATA:
						mensaje = 1; //Por lo que pasará al filtro definido más abajo y no permitirá recibir respuesta hasta que mensaje sea de nuevo 0
						printf("CLIENTE> Introduzca el correo (Introduzca un '.' al final de la última línea en blanco para finalizar el correo)");
						gets_s(input, sizeof(input));

						if (strcmp(input, ".") == 0) {
							mensaje = 0;
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", DT, input, CRLF);
						}
						break;
					}


					if (estado != S_HELO) {
						//*******************
						//Primitiva SEND()--> Primita encargada de solicitar información(petición) a la
						//primitiva RECV() de la parte del servidor
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						if (enviados == SOCKET_ERROR) {
							estado = S_QUIT;
							continue;// La sentencia continue hace que la ejecución dentro de un
							// bucle salte hasta la comprobación del mismo.
						}
					}

					//*******************

					//Primitiva RECV()--> Recibe los datos (respuesta) de la primitiva SEND() de
					//la parte del servidor
					//CONTROL DE TRANSPORTE
					if (mensaje != 1) { //Si mensaje distinto de 0, lo que hace es que no recibirá respuesta hasta una vez cambiado el valor de mensaje
						recibidos = recv(sockfd, buffer_in, 512, 0);
						if (recibidos <= 0) {
							DWORD error = GetLastError();
							if (recibidos < 0) {
								printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
								estado = S_QUIT;
							}
							else {
								printf("CLIENTE> Conexión con el servidor cerrada\r\n");
								estado = S_QUIT;
							}
						}
						else {
							buffer_in[recibidos] = 0x00;
							printf(buffer_in);

							//-----------------------------------------------------------------------------------------------------------------------
							/*Para distinguir entre varios estados. El saludo de bienvenida me da 220 y los otros 250, por ello hacemos:
							Las posibles respuestas correctas serán de 220 en el caso en la primera línea de comandos
							Para todas las demás, obtenemos una respuesta satisfactoria de 250
							Si es alguno de estas dos respuestas, cambiaremos de estado
							La comprobación y el cambio de estados será algo mas detallada que en la práctica 1*/

							//CAPA DE APLICACIÓN --> TRANSICIONES DE LA MÁQUINA DE ESTADOS
							//Máquina de estados, definida a continuación:
							switch (estado) {
							case S_INIT:
								if (strncmp(buffer_in, "250", 3) == 0) {
									// Cambio de estado si los 3 primeros caracteres del buffer_in son 220 /
									estado++;
								}
								else {
									//Si no se recibe un código 220, se cerrará el cliente
									estado = S_QUIT;
								}
								break;

							case S_HELO:
								if (strncmp(buffer_in, "220", 3) == 0) {
									// Cambio de estado si los 3 primeros caracteres del buffer_in son 220 /
									estado++;
								}
								else {
									//Si no se recibe un código 220, se cerrará el cliente
									estado = S_QUIT;
								}
								break;

								//-----------------------------------------------------------------------------------------------------------------------

							case S_MAIL_FROM:
								if (strncmp(buffer_in, "250", 3) == 0) {
									estado++;
								}
								else {
									//Si no se recibe un código 250, se cerrará el cliente
									estado = S_QUIT;
								}
								break;

								//-----------------------------------------------------------------------------------------------------------------------

							case S_RCPT_TO:
								if (strncmp(buffer_in, "250", 3) == 0 && cambio==0) {
									estado++;
								}
								else {
									//Si no se recibe un código 250, se cerrará el cliente
									estado = S_QUIT;
								}
								break;

								//-----------------------------------------------------------------------------------------------------------------------

							case S_DATA:
								if (strncmp(buffer_in, "354", 3) == 0) {
									estado++;
								}
								else {
									//Si no se recibe un código 354, se cerrará el cliente
									estado = S_QUIT;
								}
								break;

								//-----------------------------------------------------------------------------------------------------------------------

							case S_MESSAGE:
								/*Este es un estado especial, ya que tenemos que dar soporte a poder enviar tantos email como el cliente quiera.
								Por eso mismo*/
								printf("¿Quiere enviar otro mensaje? Introduzca Si para poder hacerlo, si no, se cortará la conexión: ");
								gets_s(input, sizeof(input));

								/*Si quiere enviar un nuevo correo, el estado S_HELO nos lo saltamos porque solamente se realiza al principio, por lo
								* que lo que hacemos es pasar al estado S_MAIL_FROM:*/
								if (strcmp(input, "Si") == 0) {
									//Simplemente lo que hace es comparar si lo que introducimos es un Si, si lo es, cambia de estado a S_MAIL_FROM
									estado = S_MAIL_FROM;
									break;
								}
								//Si el cliente no quiere enviar un nuevo correo, lo que haremos será cerrar la conexión
								else {
									estado = S_QUIT;
								}
								break;

								//-----------------------------------------------------------------------------------------------------------------------

							default:
								if (strncmp(buffer_in, "250", 3) == 0) {
									// Cambio de estado si los 3 primeros caracteres del buffer_in son 250 /
									estado++;
								}
								break;
							}//Fin del switch
						}//fin del else
					}//fin del if
				}while (estado != S_QUIT);
			}//fin de connect
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			closesocket(sockfd);

		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	//*******************

	//Primitiva CLOSE()--> La comunicación se corta en cuanto uno de los dos ejecuta este comando
	//Devuelve un 0 ya que esto indica que se ha cortado la comunicación con el servidor
	return(0);
}