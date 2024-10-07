#include <iostream>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include "string.h"
#include <ctime>

using namespace std;

class Server{
public:
    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;
    char buffer[1024];

    Server(){
        WSAStartup(MAKEWORD(2,0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);

        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(5555);

        bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
        listen(server, 0);

        cout << "Escuchando para conexiones entrantes." << endl;

        int clientAddrSize = sizeof(clientAddr);
        if((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET){
            cout << "Cliente conectado!" << endl;
        }
    }

    void Recibir(char* datos){

        if(recv(client, buffer, sizeof(buffer), 0) > 0){
            cout << "El cliente dice: " << buffer << endl;
            strcpy(datos, buffer);
        }else{
            CerrarSocket();
        }
        memset(buffer, 0, sizeof(buffer));

    }


    void Enviar(char* datos){
        strcpy(buffer, datos);
        send(client, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
    }

    /*
    void Enviar(){
        cout<<"Escribe el mensaje a enviar: ";
        cin>>this->buffer;
        send(client, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
        cout << "Mensaje enviado!" << endl;
    }
    */

    void CerrarSocket(){
        closesocket(client);
        cout << "Socket cerrado, cliente desconectado." << endl;
        std::cin.ignore();
        exit(1);
    }
};

class User{
public:
    string username;
    string password;
    string role;
    int active;

    User(string username, string password, string role, int active){
        this->username = username;
        this->password = password;
        this->role = role;
        this->active = active;
    }

    void toString(){
        cout << "usuario: " << this->username + " "<< this->password + " "<< this->role + " "<< this->active + " "<< endl;
    }
};

User* verificarUsuario(char* datos);



int main(){

    Server *Servidor = new Server();
    char datos[1024];
    strcpy(datos, " ");


    time_t timestamp = time(NULL);
    struct tm datetime = *localtime(&timestamp);
    char timeOutput[50];
    strftime(timeOutput, 50, "%m/%d/%y %I:%M:%S %p", &datetime);

    ofstream serverLog;
    serverLog.open("server.log", std::ios_base::app);
    serverLog << timeOutput << ": ============================================\n";
    serverLog << timeOutput << ": Servidor iniciado\n";
    serverLog << timeOutput << ": ============================================\n";
    serverLog << timeOutput << ": Socket creado. Puerto de escucha:5555\n";
    serverLog.flush();
    serverLog.close();


    while(true){
        strcat(datos, "Ingrese usuario y contrasenia separado por ;\n");

        Servidor->Enviar(datos);
        ZeroMemory(datos, 1024);

        Servidor->Recibir(datos);

        User *usuario = verificarUsuario(datos);

        if(usuario != NULL){
            usuario->toString();

            timestamp = time(NULL);
            //ZeroMemory(timeOutput, 50);
            strftime(timeOutput, 50, "%m/%d/%y %I:%M:%S %p", &datetime);

            serverLog.open("server.log", std::ios_base::app);
            serverLog << timeOutput << ": Inicio de sesión -- Usuario " + usuario->username +"\n";
            serverLog.flush();
            serverLog.close();

            ZeroMemory(datos, 1024);

            string opciones = "\n----------------------------------------\n";
            opciones += "Lista de opciones:\n";
            opciones += "----------------------------------------\n";

            if(usuario->role == "COLECCIONISTA"){
                //strcat(datos, "-> RegistrarFigurita\n");
                //strcat(datos, "-> Intercambio\n");
                opciones += "-> RegistrarFigurita\n";
                opciones += "-> Intercambio\n";
            }

            if(usuario->role == "ADMIN"){
                //strcat(datos, "-> AltaUsuario\n");
                //strcat(datos, "-> BajaUsuario\n");
                //strcat(datos, "-> RegistroActividades\n");
                opciones += "-> AltaUsuario\n";
                opciones += "-> BajaUsuario\n";
                opciones += "-> RegistroActividades\n";
            }

            //strcat(datos, "-> Salir\n");
            //strcat(datos, "--------------------\n");
            opciones += "-> Salir\n";
            opciones += "----------------------------------------\n";

            while(true){
                /*
                strcpy(datos, "--------------------\nLista de opciones:\n--------------------\n");

                if(usuario->role == "COLECCIONISTA"){
                    strcat(datos, "-> RegistrarFigurita\n");
                    strcat(datos, "-> Intercambio\n");

                }

                if(usuario->role == "ADMIN"){
                    strcat(datos, "-> AltaUsuario\n");
                    strcat(datos, "-> BajaUsuario\n");
                    strcat(datos, "-> RegistroActividades\n");

                }

                strcat(datos, "-> Salir\n");
                strcat(datos, "--------------------\n");

                Servidor->Enviar(datos);
                */

                strcat(datos, opciones.data());
                Servidor->Enviar(datos);
                ZeroMemory(datos, 1024);
                Servidor->Recibir(datos);
            }
        }

    }
}


User* verificarUsuario(char* datos){
    bool encontrado = false;
    User* user = NULL;

    string username, password, role, active;
    stringstream datosAux;

    datosAux << datos;

    getline(datosAux, username, ';');
    getline(datosAux, password, ';');

    fstream autenticacion("autenticacion.txt");

    string usernameAux, passwordAux, linea;
    stringstream txt;

    while(getline (autenticacion, linea) && !encontrado){

        txt << linea;

        getline(txt, usernameAux, ';');
        getline(txt, passwordAux, ';');

        if(username == usernameAux && password == passwordAux){

            getline(txt, role, ';');
            getline(txt, active, ';');

            user = new User(username, password, role, stoi(active));

            encontrado = true;
        }
    }

    if(encontrado){
        string mensaje = "Bienvenido " + user->username + "!\n\n";
        strcpy(datos, mensaje.data());
    }else{
        strcpy(datos, (char*)"Datos de usuario incorrectos\n\n");
    }

    return user;
}
