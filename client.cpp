#include <iostream>
#include <winsock2.h>

using namespace std;

///ip = 192.168.0.101
///port = 5555

class Client{
public:
    WSADATA WSAData;
    SOCKET server;
    SOCKADDR_IN addr;
    char buffer[4096];

    Client(){
        WSAStartup(MAKEWORD(2,0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);
        addr.sin_family = AF_INET;
    }

    int conectar(char* ip, int port){
        addr.sin_addr.s_addr = inet_addr(ip);
        addr.sin_port = htons(port);
        int resultado =  connect(server, (SOCKADDR *)&addr, sizeof(addr));

        if(resultado != 0){
            cout << "Error: ip o puerto incorrectos\n" << endl;
        }else{
            cout << "Conectado al Servidor!" << endl;
        }

        return resultado;
    }

    void Enviar(){
        //cout<<"Escribe el mensaje a enviar: ";
        cin>>this->buffer;
        send(server, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
        cout << "Mensaje enviado!" << endl;
    }

    void Recibir(){
        if(recv(server, buffer, sizeof(buffer), 0) > 0){
            cout << buffer << endl;
        }else{
            CerrarSocket();
        }
        memset(buffer, 0, sizeof(buffer));
    }

    void CerrarSocket(){
       closesocket(server);
       WSACleanup();
       cout << "Socket cerrado." << endl << endl;
       std::cin.ignore();
       exit(1);
    }
};

int main(){
    Client *Cliente = new Client();
    char ip[32];
    int port;

    /*
    do{
        ZeroMemory(ip, 32);

        cout<<"Ingrese ip: ";
        cin >> ip;
        cout<<"Ingrese puerto: ";
        cin >> port;

        system("cls");

    }while( Cliente->conectar(ip, port) != 0 );
    */

    Cliente->conectar((char*)"192.168.0.101", 5555);

    while(true){
        system("cls");
        Cliente->Recibir();
        Cliente->Enviar();
    }
}
