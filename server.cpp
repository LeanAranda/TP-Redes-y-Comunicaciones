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

    int Recibir(char* datos){
        int resultado = recv(client, buffer, sizeof(buffer), 0);

        if(resultado > 0){
            cout << "El cliente dice: " << buffer << endl;
        }

        strcpy(datos, buffer);

        memset(buffer, 0, sizeof(buffer));

        return resultado;
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

/// funciones
User* verificarUsuario(char* datos);
void serverLog(string log);
void verRegistro(char* datos);
void usuarios(char* datos, User* user, Server* server);


int main(){
    /// crear servidor
    Server *Servidor = new Server();
    char datos[1024];
    strcpy(datos, "\n");
    int resultado;

    serverLog("============================================\n");
    serverLog("Servidor iniciado\n");
    serverLog("============================================\n");
    serverLog("Socket creado. Puerto de escucha:5555\n");

    /// menú de inicio de sesion
    do{
        strcat(datos, "Ingrese usuario y contrasenia separado por ;\n");

        Servidor->Enviar(datos);
        ZeroMemory(datos, 1024);

        resultado = Servidor->Recibir(datos);

        User *usuario = verificarUsuario(datos);

        // si el logueo es exitoso
        if(usuario != NULL){
            //usuario->toString();

            serverLog("Inicio de sesion -- Usuario " + usuario->username +"\n");

            //ZeroMemory(datos, 1024);

            string opciones = "----------------------------------------\n";
            opciones += "Lista de opciones:\n";
            opciones += "----------------------------------------\n";

            if(usuario->role == "COLECCIONISTA"){
                opciones += "-> registrar-figurita\n";
                opciones += "-> intercambio\n";
            }

            if(usuario->role == "ADMIN"){
                opciones += "-> usuarios\n";
                opciones += "-> registro\n";
            }

            opciones += "-> salir\n";
            opciones += "----------------------------------------\n";

            string comando;

            ///menú de comandos
            do{
                strcat(datos, opciones.data());
                Servidor->Enviar(datos);
                ZeroMemory(datos, 1024);
                resultado = Servidor->Recibir(datos);

                comando = datos;

                string mensaje;

                if(comando == "usuarios" && usuario->role == "ADMIN"){
                    usuarios(datos, usuario, Servidor);
                }
                else if(comando == "registro" && usuario->role == "ADMIN"){
                    verRegistro(datos);
                }
                else if(comando == "salir"){
                    serverLog("Cierre de sesion -- Usuario " + usuario->username +"\n");

                    mensaje = "Adios " + usuario->username +  "!!\n\n";
                    strcpy(datos, mensaje.data());
                    free(usuario);
                }
                else{
                    mensaje = "Comando invalido!\n\n";
                    strcpy(datos, mensaje.data());
                }

            }while(comando != "salir" && resultado > 0);
        }

    }while(resultado > 0);

    /// cerrar servidor

    serverLog("============================================\n");
    serverLog("Cerrando servidor y socket...\n");
    serverLog("============================================\n");

    Servidor->CerrarSocket();
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
        getline(txt, role, ';');
        getline(txt, active, ';');

        if(username == usernameAux && password == passwordAux){

            if(stoi(active) == 1){
                user = new User(username, password, role, stoi(active));
            }

            encontrado = true;
        }
    }

    string mensaje;

    if(encontrado && stoi(active) == 1){
        mensaje = "Bienvenido " + username + "!\n\n";
    }
    else if(encontrado && stoi(active) == 0){
        mensaje = "Usuario " + username + " inactivo!\n\n";
    }
    else{
        mensaje = "Datos de usuario incorrectos\n\n";
    }

    strcpy(datos, mensaje.data());

    return user;
}


// la mejor función de la historia
void serverLog(string log){
    time_t timestamp = time(NULL);
    struct tm datetime = *localtime(&timestamp);
    char timeOutput[50];
    strftime(timeOutput, 50, "%m/%d/%y %I:%M:%S %p", &datetime);

    ofstream archivo;
    archivo.open("server.log", std::ios_base::app);
    archivo << timeOutput << ": " + log;
    archivo.flush();
}


void verRegistro(char* datos){
    ifstream registro("server.log");

    //si el archivo está vacío
    if(registro.peek() == std::ifstream::traits_type::eof()){
        strcpy(datos, (char*)"No hay registros\n\n");
    }else{
        string mensaje((std::istreambuf_iterator<char>(registro)), std::istreambuf_iterator<char>());
        strcpy(datos, mensaje.data());
        strcat(datos, (char*)"\n");
    }

}


void usuarios(char* datos, User* user, Server* server){
    int resultado;
    strcpy(datos, (char*)"\n");
    string comando;
    string opciones = "----------------------------------------\n";
    opciones += "Lista de opciones de usuarios:\n";
    opciones += "----------------------------------------\n";
    opciones += "-> alta\n";
    opciones += "-> baja\n";
    opciones += "-> volver\n";
    opciones += "----------------------------------------\n";

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 1024);

        resultado = server->Recibir(datos);
        comando = datos;
        //string mensaje;

        if(comando == "alta"){
            strcpy(datos, (char*)"alta\n");
        }
        else if(comando == "baja"){
            strcpy(datos, (char*)"baja\n");
        }
        else if(comando == "volver"){
            strcpy(datos, (char*)"\n");
        }
        else{
            strcpy(datos, (char*)"comando invalido\n");
        }
    }while(resultado > 0 && comando != "volver");
}
