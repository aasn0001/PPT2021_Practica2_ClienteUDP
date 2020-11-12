/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingenier�a Telem�tica
 * Dpto. Ingen�er�a de Telecomunicaci�n
 * Universidad de Ja�n
 *
 *******************************************************
 * Pr�ctica 2.
 * Fichero: cliente.c
 * Versi�n: 1.1
 * Fecha: 10/2020
 * Descripci�n:
 * 	Cliente de eco sencillo sobre UDP en IPv4
 * Autor: Juan Carlos Cuevas Mart�nez
 *
 ******************************************************
 * Alumno 1:
 * Alumno 2:
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca est�ndar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <locale.h>		// Para establecer el idioma de la codificaci�n de texto, n�meros, etc.


#pragma comment(lib, "Ws2_32.lib")//Enlaza la biblioteca Ws2_32.lib

#define UDP_CLIENT_PORT	6001
#define UDP_SERVER_PORT	6000

int main(int *argc, char *argv[]){
	// Variables de incializaci�n de los Windows Sockets
	WORD wVersionRequested;
	WSADATA wsaData;

	// Fin Variables de incializaci�n de los Windows Sockets
	SOCKET sockfd;
	struct sockaddr_in client_in,server_in,input_in;
	int input_l;
	char buffer_in[2048], buffer_out[2048];
	int in_len;
	char user_input[1024];
	int recibidos=0;
	int enviados=0;
	char iplocal[20]="127.0.0.1";
    char ipdest[20]="127.0.0.1";
	int n_secuencia=1;
	int err=0;

	//Inicializaci�n de idioma
	setlocale(LC_ALL, "es-ES");

	// Inicializaci�n Windows Sockets versi�n 2.2
	wVersionRequested=MAKEWORD(2,2);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=2||HIBYTE(wsaData.wVersion)!=2){
		WSACleanup();
		return(0);
	}// Fin Inicializaci�n Windows Sockets

	sockfd=socket(PF_INET,SOCK_DGRAM,0);
	if(sockfd==INVALID_SOCKET){
		printf("CLIENTE UDP> Error\r\n");
	}else{
		//Direcci�n local del cliente
		client_in.sin_family=AF_INET;
		client_in.sin_port=htons(UDP_CLIENT_PORT);
		
		
		inet_pton(AF_INET,ipdest,&client_in.sin_addr.s_addr);


		if(bind(sockfd,(struct sockaddr *)&client_in,sizeof(client_in))==SOCKET_ERROR){
			printf("CLIENTE UDP> Error %d\r\n",GetLastError());
		
		}else{
			char cadtemp[20];

				// Direcci�n remota del servidor para cada env�o
				printf("CLIENTE UDP> IP del servidor [%s] : ",ipdest);
				
				gets_s(cadtemp,sizeof(cadtemp));
				if(strcmp(cadtemp,"")!=0)
					strcpy_s(ipdest,sizeof(ipdest),cadtemp);

				server_in.sin_family=AF_INET;
				server_in.sin_port=htons(UDP_SERVER_PORT);
				
				inet_pton(AF_INET,ipdest,&server_in.sin_addr.s_addr);

			do{// Se estar�n enviando mensajes de eco hasta que se pulse solo un enter
				
				printf("CLIENTE UDP> Introduzca una cadena para enviar al servidor: ");
				gets_s(user_input,sizeof(user_input));
				sprintf_s(buffer_out,sizeof(buffer_out),"ECHO %d %s\r\n",n_secuencia,user_input);

				enviados=sendto(sockfd,buffer_out,(int)strlen(buffer_out),0,(struct sockaddr *)&server_in,sizeof(server_in));
				if(enviados!=SOCKET_ERROR){
					printf("CLIENTE UDP> Enviados %d bytes\r\n",enviados);					
					in_len=sizeof(buffer_in);
					input_l=sizeof(input_in);

					recibidos=recvfrom(sockfd,buffer_in,in_len,0,(struct sockaddr *)&input_in,&input_l);
					if(recibidos!=SOCKET_ERROR){
						char peer[32]="";
						int r_secuencia=0;
						char eco[1024] ="";
						buffer_in[recibidos]=0;
						
						inet_ntop(AF_INET, &input_in.sin_addr, peer, sizeof(peer));
						
						printf("CLIENTE UDP> Recibidos %d bytes de %s %d\r\n",recibidos,peer,ntohs(input_in.sin_port));
						sscanf_s(buffer_in,"OK %d %[^\r]s\r\n",&r_secuencia,eco,sizeof(eco));
						
						if(r_secuencia==n_secuencia && strlen(eco)>0){
							printf("CLIENTE UDP> Eco recibido: %s\r\n",eco);
						}else{
							printf("CLIENTE UDP> Error en la respuesta");
						}

					}
					n_secuencia++;
				}
			}while(strcmp("",user_input)!=0);
		}
		closesocket(sockfd);
	}//fin sockfd==INVALID_SOCKET
	
	WSACleanup();// solo Windows

}//fin main