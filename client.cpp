#define WIN32_LEAN_AND_MEAN

#include <iostream>
using namespace std;

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

///cliente

int __cdecl main(void /*int argc, char **argv*/)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    //const char *sendbuf = "this is a test";
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    /*
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }
    */

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    /// Resolve the server address and port

    do{
        if(false){
            char ip[30], puerto[30];

            cout << "Ingrese ip" << endl;

            cin.getline(ip, 30);

            cout << "Ingrese puerto" << endl;
            cin.getline(puerto, 30);

            iResult = getaddrinfo(ip, puerto, &hints, &result);
        }else{
            iResult = getaddrinfo("192.168.0.101", DEFAULT_PORT, &hints, &result);
        }

        if ( iResult != 0 ) {
            cerr << "ERROR: ip o puerto incorrectos!\n" << endl;
            //printf("getaddrinfo failed with error: %d\n", iResult);
            /*
            WSACleanup();
            return 1;
            */
        }
    }while(iResult != 0);

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    /*
    // Send an initial buffer
    iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
            printf("Bytes received: %d\n", iResult);
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );


    do{
        cout << "escribe un mensaje" << endl;
        //cin >> buff;
        cin.getline(sendbuf, DEFAULT_BUFLEN);
        cout << "\n";
        iResult = send( ConnectSocket, sendbuf, sizeof(sendbuf)/*(int)strlen(buff)/, 0 );
        memset(sendbuf, 0, sizeof(sendbuf));
    }while(true);
    */
    /*
    do{
        cout << "esperando" << endl;
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

        cout << recvbuf << endl;
        cout << "enviar" << endl;
        cin.getline(sendbuf, DEFAULT_BUFLEN);

        iResult = send( ConnectSocket, sendbuf, sizeof(sendbuf)/*(int)strlen(buff)/, 0 );
        memset(sendbuf, 0, sizeof(sendbuf));

    }while(true);
    */


    /// --- conexión de test
    /*
    // Send an initial buffer
    iResult = send( ConnectSocket, sendbuf, sizeof(sendbuf), 0 );
    cout << "enviado" << endl;

    iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0){
        cout << "recibido " << recvbuf << endl;
    }
    */
    /// ---


    do{
        system("cls");

        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

        cout << recvbuf << endl;

        cin.getline(sendbuf, DEFAULT_BUFLEN);
        cout << "\n" << endl;

        iResult = send( ConnectSocket, sendbuf, sizeof(sendbuf), 0);
    }while(iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
