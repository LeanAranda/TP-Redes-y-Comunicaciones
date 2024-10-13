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
    char buffer[4096];

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
            cout << "\nEl cliente dice: " << buffer << endl;
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
User* iniciarSesion(char* datos);
void serverLog(string log);
void verRegistro(char* datos);
void usuariosMenu(char* datos, Server* server);
void altaUsuario(char* datos);
void registrarFiguritaMenu(char * datos, Server * server, User * user);
void registrarFigurita(char * datos, User * user);
void paisesRegistrados(char * datos);


int main(){
    /// crear servidor
    Server *Servidor = new Server();
    char datos[4096];
    strcpy(datos, "\n");
    int resultado;

    //iniciar con un log vacío
    remove("server.log");

    serverLog("============================================\n");
    serverLog("Servidor iniciado\n");
    serverLog("============================================\n");
    serverLog("Socket creado. Puerto de escucha:5555\n");

    /// menú de inicio de sesión
    do{
        strcat(datos, "Ingrese usuario y contrasenia separado por ;\n");

        Servidor->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = Servidor->Recibir(datos);

        User *usuario = iniciarSesion(datos);

        /// si el logueo es exitoso
        if(usuario != NULL){

            serverLog("Inicio de sesion -- Usuario " + usuario->username +"\n");

            string opciones = "--------------------------------------------------------------------------------\n";
            opciones += "Lista de opciones " + usuario->role + ":\n";
            opciones += "--------------------------------------------------------------------------------\n";

            if(usuario->role == "COLECCIONISTA"){
                opciones += "-> registrar_figurita\n";
                opciones += "-> intercambio\n";
                opciones += "-> paises_registrados\n";
            }

            if(usuario->role == "ADMIN"){
                opciones += "-> usuarios\n";
                opciones += "-> registro\n";
            }

            opciones += "-> salir\n";
            opciones += "--------------------------------------------------------------------------------\n";

            string comando;

            ///menú principal
            do{
                strcat(datos, opciones.data());

                Servidor->Enviar(datos);
                ZeroMemory(datos, 4096);

                resultado = Servidor->Recibir(datos);
                comando = datos;

                /// comandos
                if(comando == "registrar_figurita" && usuario->role == "COLECCIONISTA"){
                    registrarFiguritaMenu(datos, Servidor, usuario);
                }
                else if(comando == "paises_registrados" && usuario->role == "COLECCIONISTA"){
                    paisesRegistrados(datos);
                }
                else if(comando == "usuarios" && usuario->role == "ADMIN"){
                    usuariosMenu(datos, Servidor);
                }
                else if(comando == "registro" && usuario->role == "ADMIN"){
                    verRegistro(datos);
                }
                else if(comando == "salir"){
                    serverLog("Cierre de sesion -- Usuario " + usuario->username +"\n");

                    string mensaje = "Chau " + usuario->username +  "!!\n\n";
                    strcpy(datos, mensaje.data());

                    free(usuario);
                }
                else{
                    strcpy(datos, (char*)"Comando invalido!\n\n");
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


User* iniciarSesion(char* datos){
    bool encontrado = false;
    User* user = NULL;

    string username, password, role, active;
    stringstream datosAux;

    datosAux << datos;

    getline(datosAux, username, ';');
    getline(datosAux, password, ';');

    ifstream autenticacion("autenticacion.txt");

    string usernameAux, passwordAux, linea;
    stringstream txt;

    while(getline (autenticacion, linea) && !encontrado){

        txt << linea;

        getline(txt, usernameAux, ';');
        getline(txt, passwordAux, ';');

        if(username == usernameAux && password == passwordAux){

            getline(txt, role, ';');
            getline(txt, active, ';');

            if(stoi(active) == 1){
                user = new User(username, password, role, stoi(active));
            }

            encontrado = true;
        }

        txt.str("");
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


void usuariosMenu(char* datos, Server* server){
    int resultado;
    strcpy(datos, (char*)"\n");
    string comando;

    string opciones = "--------------------------------------------------------------------------------\n";
    opciones += "Opciones de usuarios:\n";
    opciones += "--------------------------------------------------------------------------------\n";
    opciones += "-> alta\n";
    opciones += "-> baja\n";
    opciones += "-> volver\n(menu principal)\n";
    opciones += "--------------------------------------------------------------------------------\n";

    string opcionesAlta = "--------------------------------------------------------------------------------\n";
    opcionesAlta += "Alta de usuarios:\n";
    opcionesAlta += "--------------------------------------------------------------------------------\n";
    opcionesAlta += "-> ingresar datos de la siguiente manera: nombre;contrasenia\n";
    opcionesAlta += "-> volver\n(menu principal)\n";
    opcionesAlta += "--------------------------------------------------------------------------------\n";

    string opcionesBaja = "--------------------------------------------------------------------------------\n";
    opcionesBaja += "Baja de usuarios:\n";
    opcionesBaja += "--------------------------------------------------------------------------------\n";
    opcionesBaja += "-> nombre del usuario\n";
    opcionesBaja += "-> volver\n";
    opcionesBaja += "--------------------------------------------------------------------------------\n";

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = server->Recibir(datos);
        comando = datos;
        //string mensaje;

        /// menú de alta usuarios
        if(comando == "alta"){

            strcpy(datos, opcionesAlta.data());

            server->Enviar(datos);
            ZeroMemory(datos, 4096);

            resultado = server->Recibir(datos);
            comando = datos;

            if(comando != "volver"){
                altaUsuario(datos);
                comando = "volver";
            }else{
                strcpy(datos, (char*)"\n");
            }
        }


        /// menú de baja usuarios
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

void altaUsuario(char* datos){

    // extraigo los datos
    string username, password;
    stringstream datosAux;

    datosAux << datos;

    getline(datosAux, username, ';');
    getline(datosAux, password, ';');

    // los datos no pueden ser vacios

    if(username.empty() || password.empty()){
        strcpy(datos, (char*)"Error al dar de alta el nuevo usuario: datos incompletos.\n\n");
        return;
    }

    // el usuario no tiene que estar registrado

    bool encontrado = false;

    fstream autenticacion("autenticacion.txt");

    string usernameAux, linea;
    stringstream txt;

    while(getline(autenticacion, linea) && !encontrado){
        txt << linea;

        getline(txt, usernameAux, ';');
        txt.str("");

        if(usernameAux == username){
            encontrado = true;
        }

    }

    autenticacion.close();

    if(encontrado){
        strcpy(datos, (char*)"Error al dar de alta el nuevo usuario: usuario existente.\n\n");
        return;
    }

    // guardar los datos

    autenticacion.open("autenticacion.txt",std::ios_base::app);
    autenticacion << username + ";" + password + ";COLECCIONISTA;1;\n";
    autenticacion.flush();
    autenticacion.close();

    string mensaje = "Usuario " + username + " registrado correctamente!\n\n";
    strcpy(datos, mensaje.data());
}

void registrarFiguritaMenu(char * datos, Server * server, User * user){

    int resultado;
    string comando;

    string opciones = "--------------------------------------------------------------------------------\n";
    opciones += "Registrar figurita:\n";
    opciones += "--------------------------------------------------------------------------------\n\n";
    opciones += "-> ingresar datos de la siguiente manera: pais;jugador\n(El nombre del pais debe empezar con mayuscula!)\n\n";
    opciones += "-> volver\n(menu principal)\n\n";
    opciones += "--------------------------------------------------------------------------------\n";

    strcpy(datos, (char*)"\n");

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = server->Recibir(datos);
        comando = datos;

        if(comando != "volver"){
            registrarFigurita(datos, user);
            comando = "volver";
        }else{
            strcpy(datos, (char*)"\n");
        }
    }while(resultado > 0 && comando != "volver");
}

void registrarFigurita(char * datos, User * user){

    cout << datos <<endl;

    // extraigo los datos
    string pais, jugador;
    stringstream datosAux;

    datosAux << datos;

    getline(datosAux, pais, ';');
    getline(datosAux, jugador, ';');

    // los datos no pueden ser vacios
    if(pais.empty() || jugador.empty()){
        strcpy(datos, (char*)"Error al registrar figurita: datos incompletos.\n\n");
        return;
    }

    // el pais tiene que existir
    bool encontrado = false;

    ifstream paises("paises.txt");
    string linea;

    while(getline(paises, linea) && !encontrado){

        if(linea == pais){
            encontrado = true;
        }

        linea = "";
    }

    paises.close();

    if(!encontrado){
        strcpy(datos, (char*)"Error al registrar figurita: el pais ingresado no esta en la lista de paises registrados.\nLa lista puede verse en la opcion paises_registrados.\n\n");
        return;
    }

    // guardar los datos
    int id = 1;

    fstream figuritas("figuritas.txt");

    while(getline(figuritas, linea)){
        id++;
    }

    figuritas.close();

    figuritas.open("figuritas.txt", std::ios_base::app);
    figuritas << id << ";" + user->username + ";" + pais + ";" + jugador + ";1;\n";
    figuritas.flush();
    figuritas.close();

    string mensaje = "Figurita registrada correctamente!\n\n";
    strcpy(datos, mensaje.data());
}

void paisesRegistrados(char * datos){
    string mensaje = "--------------------------------------------------------------------------------\n";
    mensaje += "Lista de paises registrados para las figuritas\n";
    mensaje += "--------------------------------------------------------------------------------\n";

    ifstream paises("paises.txt");
    string linea;

    while(getline(paises, linea)){
        mensaje += linea + "\n";
        linea = "";
    }

    mensaje += "--------------------------------------------------------------------------------\n\n";

    strcpy(datos, mensaje.data());
}
