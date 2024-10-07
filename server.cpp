#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <iostream>
using namespace std;
#include <sstream>
#include <fstream>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


int verificarUsuario(char* buffEntrada, char* buffSalida);

///server

int __cdecl main(void)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    char sendbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    cout << "--------------------------------------" << endl;
    cout << "se acaba de conectar un pibe..." << endl;
    cout << "--------------------------------------" << endl;

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    /*
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

        // Echo the buffer back to the sender
            iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if(iResult > 0){
            cout << "el cliente dice: " << recvbuf << endl;

        }

    } while (/*iResult > 0/true);
    */
    /*
    do{
        strcpy(sendbuf, "Ingrese usuario y contrasenia separados por ;");
        cout << "enviando" << endl;
        iSendResult = send( ClientSocket, sendbuf, iResult, 0 );
        cout << "esperando" << endl;
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

        string user, password;
        stringstream cadena;

        cadena << recvbuf;

        getline(cadena, user, ';');
        getline(cadena, password, ';');

        cout << "usuario: " << user << endl;
        cout << "contrasenia: " << password << endl;
    }while(true);

    */


    /// --- conexión de test
    /*
    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
        cout << "recibido " << recvbuf << endl;

        // Echo the buffer back to the sender
        iSendResult = send( ClientSocket, recvbuf, iResult, 0 );

        cout << "enviado" << endl;
    }
    */
    /// ---

    memset(sendbuf, 0, sizeof(sendbuf));
    memset(recvbuf, 0, sizeof(recvbuf));

    do{

        strcat(sendbuf, "Ingrese usuario y contrasenia separados por ;\n");
        iSendResult = send( ClientSocket, sendbuf, sizeof(sendbuf), 0 );

        memset(sendbuf, 0, sizeof(sendbuf));


        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

        int result = verificarUsuario(recvbuf, sendbuf);
        //iSendResult = send( ClientSocket, sendbuf, sizeof(sendbuf), 0 );

        memset(recvbuf, 0, sizeof(recvbuf));


    }while(iResult > 0);


    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}


int verificarUsuario(char* buffEntrada, char* buffSalida){

    int msg = 1;

    string user, password;
    stringstream cadena;

    cadena << buffEntrada;

    getline(cadena, user, ';');
    getline(cadena, password, ';');

    /*
    cout << "usuario: " << user << endl;
    cout << "contrasenia: " << password << endl;
    */

    fstream autenticacion("autenticacion.txt");

    string userAux, passwordAux, texto;
    stringstream texto2;

    while(getline (autenticacion, texto) && msg == 1){

        texto2 << texto;

        getline(texto2, userAux, ';');
        getline(texto2, passwordAux, ';');

        /*
        cout << "\nuser-> " << userAux << endl;
        cout << "\npassword-> " << passwordAux << endl;
        */

        if(user == userAux && password == passwordAux){
            msg = 2;
        }
    }

    switch(msg){
        case 1: strcpy(buffSalida, "ERROR: el usuario no esta registrado!\n\n"); break;
        case 2: strcpy(buffSalida, "Usuario encontrado!\n\n"); break;
    }

    return msg;
}
