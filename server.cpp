#include <iostream>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include "string.h"
#include <ctime>

using namespace std;

/// clases

class Server{
public:
    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;
    char buffer[4096];
    int port;

    Server(int port){
        this->port = port;
        WSAStartup(MAKEWORD(2,0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);

        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
        listen(server, 0);

        cout << "Escuchando para conexiones entrantes en el puerto " << this->port << endl;

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
        cout << "\nSocket cerrado, cliente desconectado." << endl;
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
};

class Figurita{
public:
    int id;
    string usuario;
    string pais;
    string jugador;
    string disponible;

    Figurita(int id, string usuario, string pais, string jugador, string disponible){
        this->id = id;
        this->usuario = usuario;
        this->pais = pais;
        this->jugador = jugador;
        this->disponible = disponible;
    }
};

/// funciones

User* iniciarSesion(char* datos);
User* buscarUsuario(string username);
void serverLog(string log);
void verRegistro(char* datos);
void usuariosMenu(char* datos, Server* server);
void altaUsuario(char* datos);
void modificarActivoUsuario(User* usuario, bool activo);
void registrarFiguritaMenu(char * datos, Server * server, User * user);
void registrarFigurita(char * datos, User * user);
bool existePais(string pais);
Figurita* buscarFigurita(string usuario, string pais, string jugador);
int listadoFiguritas(User* user, char* datos);
void menuIntercambio(Server* server, User* user, char* datos);
bool validarDatosPeticion(User* user, char*datos);
void crearPeticion(User* user, char*datos);
void intercambiarFiguritas(Figurita* figurita1, Figurita* figurita2);
bool modificarEstadoPeticion(string peticion, bool estado);
int listaPeticionesPendientes(User * user, char*datos);
void cancelarPeticion(User * user, char* datos);
int listaUsuariosActivos(char* datos);
void bajaUsuario(char*datos);
void bajaPeticiones(string username);
void bajaFiguritas(string username);


int main(){
    /// crear servidor
    Server *Servidor = new Server(5555);
    char datos[4096];
    strcpy(datos, "\n");
    int resultado;

    //iniciar con un log vacio
    remove("server.log");

    serverLog("============================================\n");
    serverLog("Servidor iniciado\n");
    serverLog("============================================\n");
    serverLog("Socket creado. Puerto de escucha: " + std::to_string(Servidor->port) + "\n");

    string mensajeSesion = "--------------------------------------------------------------------------------\n";
    mensajeSesion += "Servicio de intercambio de figuritas -- iniciar sesion\n";
    mensajeSesion += "--------------------------------------------------------------------------------\n\n";
    mensajeSesion += "Ingrese sus datos de la siguiente forma: usuario;contrasenia\n\n";
    mensajeSesion += "--------------------------------------------------------------------------------\n";

    /// menú de inicio de sesion
    do{
        strcat(datos, mensajeSesion.data());

        Servidor->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = Servidor->Recibir(datos);

        User *usuario = iniciarSesion(datos);

        /// si el logueo es exitoso
        if(usuario != NULL){

            serverLog("Inicio de sesion -- Usuario " + usuario->username +"\n");

            string opciones = "--------------------------------------------------------------------------------\n";
            opciones += "Lista de opciones " + usuario->role + ":\n";
            opciones += "--------------------------------------------------------------------------------\n\n";

            if(usuario->role == "COLECCIONISTA"){
                opciones += "-> registrar\n\n";
                opciones += "-> intercambio\n\n";
            }

            if(usuario->role == "ADMIN"){
                opciones += "-> usuarios\n\n";
                opciones += "-> registro\n\n";
            }

            opciones += "-> salir\n\n";
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
                if(comando == "registrar" && usuario->role == "COLECCIONISTA"){
                    registrarFiguritaMenu(datos, Servidor, usuario);
                }
                else if(comando == "intercambio" && usuario->role == "COLECCIONISTA"){
                    menuIntercambio(Servidor, usuario, datos);
                }
                else if(comando == "usuarios" && usuario->role == "ADMIN"){
                    usuariosMenu(datos, Servidor);
                }
                else if(comando == "registro" && usuario->role == "ADMIN"){
                    verRegistro(datos);
                }
                else if(comando == "salir"){
                    serverLog("Cierre de sesion -- Usuario " + usuario->username +"\n");

                    string mensaje= "--------------------------------------------------------------------------------\n";
                    mensaje+= "Chau " + usuario->username +  "!!\n";
                    mensaje+= "--------------------------------------------------------------------------------\n\n";

                    strcpy(datos, mensaje.data());

                    free(usuario);
                }
                else{
                    string mensaje= "--------------------------------------------------------------------------------\n";
                    mensaje+= "Comando invalido\n";
                    mensaje+= "--------------------------------------------------------------------------------\n\n";

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


User* iniciarSesion(char* datos){
    string username, password;
    stringstream datosAux;
    datosAux << datos;

    getline(datosAux, username, ';');
    getline(datosAux, password, ';');

    User * usuarioRespuesta = NULL;

    string mensaje = "--------------------------------------------------------------------------------\n";

    if(username.empty() || password.empty()){
        mensaje += "Error: datos incompletos\n";
    }else{
        User * usuarioAux = buscarUsuario(username);

        if(usuarioAux == NULL){
            mensaje += "Error: el usuario [" + username + "] no existe\n";
        }else if(usuarioAux->active == 0){
            mensaje += "Error: el usuario [" + username + "] esta inactivo\n";
        }else if(password != usuarioAux->password){
            mensaje += "Error: contrasenia incorrecta\n";
        }else{
            mensaje += "Bienvenido " + username + "!!\n";
            usuarioRespuesta = usuarioAux;
        }
    }

    mensaje += "--------------------------------------------------------------------------------\n\n";

    strcpy(datos, mensaje.data());

    return usuarioRespuesta;
}

User* buscarUsuario(string username){
    User * usuario = NULL;
    string linea;
    bool encontrado = false;
    ifstream autenticacion("autenticacion.txt");

    while(getline(autenticacion, linea) && !encontrado){
        string usernameAux;
        stringstream lineaAux;
        lineaAux << linea;

        getline(lineaAux, usernameAux, ';');

        if(username == usernameAux){
            encontrado = true;

            string password, role, active;
            getline(lineaAux, password, ';');
            getline(lineaAux, role, ';');
            getline(lineaAux, active, ';');

            usuario = new User(username, password, role, stoi(active));
        }

        linea = "";
    }

    return usuario;
}

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
        string mensaje = "--------------------------------------------------------------------------------\n";
        mensaje += "No hay registros\n";
        mensaje += "--------------------------------------------------------------------------------\n\n";
        strcpy(datos, mensaje.data());

    }else{
        string mensaje((std::istreambuf_iterator<char>(registro)), std::istreambuf_iterator<char>());
        strcpy(datos, mensaje.data());
        strcat(datos, (char*)"\n\n");
    }

}

void usuariosMenu(char* datos, Server* server){
    int resultado;
    strcpy(datos, (char*)"\n");
    string comando;

    string opciones = "--------------------------------------------------------------------------------\n";
    opciones += "Usuarios:\n";
    opciones += "--------------------------------------------------------------------------------\n\n";
    opciones += "-> alta\n\n";
    opciones += "-> baja\n\n";
    opciones += "-> |volver\n\n";
    opciones += "--------------------------------------------------------------------------------\n";

    string opcionesAlta = "--------------------------------------------------------------------------------\n";
    opcionesAlta += "Alta de usuarios:\n";
    opcionesAlta += "--------------------------------------------------------------------------------\n\n";
    opcionesAlta += "-> ingresar datos de la siguiente manera: nombre;contrasenia\n\n";
    opcionesAlta += "-> |volver\n\n";
    opcionesAlta += "--------------------------------------------------------------------------------\n";

    string opcionesBaja = "--------------------------------------------------------------------------------\n";
    opcionesBaja += "Baja de usuarios:\n";
    opcionesBaja += "--------------------------------------------------------------------------------\n\n";
    opcionesBaja += "-> nombre del usuario\n\n";
    opcionesBaja += "-> |volver\n\n";
    opcionesBaja += "--------------------------------------------------------------------------------\n";

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = server->Recibir(datos);
        comando = datos;

        /// menú de alta usuarios
        if(comando == "alta"){

            strcpy(datos, opcionesAlta.data());

            server->Enviar(datos);
            ZeroMemory(datos, 4096);

            resultado = server->Recibir(datos);
            comando = datos;

            if(comando != "|volver"){
                altaUsuario(datos);
                comando = "|volver";
            }else{
                strcpy(datos, (char*)"\n");
            }
        }


        /// menú de baja usuarios
        else if(comando == "baja"){
            if(listaUsuariosActivos(datos) > 0){
                strcat(datos, opcionesBaja.data());

                server->Enviar(datos);
                ZeroMemory(datos, 4096);

                resultado = server->Recibir(datos);
                comando = datos;

                if(comando != "|volver"){
                    bajaUsuario(datos);
                    comando = "|volver";
                }else{
                    strcpy(datos, (char*)"\n");
                }
            }else{
                comando = "|volver";
            }
        }
        else if(comando == "|volver"){
            strcpy(datos, (char*)"\n");
        }
        else{
            string mensaje= "--------------------------------------------------------------------------------\n";
            mensaje+= "Comando invalido\n";
            mensaje+= "--------------------------------------------------------------------------------\n\n";
            strcpy(datos, mensaje.data());

        }
    }while(resultado > 0 && comando != "|volver");
}

void altaUsuario(char* datos){

    // extraigo los datos
    string username, password;
    stringstream datosAux;

    datosAux << datos;

    getline(datosAux, username, ';');
    getline(datosAux, password, ';');

    string mensaje = "--------------------------------------------------------------------------------\n";

    // los datos no pueden ser vacios

    if(username.empty() || password.empty()){
        mensaje+= "Error al dar de alta el nuevo usuario: datos incompletos\n";
    }else{
        // el usuario no tiene que estar registrado
        User* usuario = buscarUsuario(username);

        if(usuario != NULL){
            // si ya está activo tira error
            // si está inactivo y la contraseña es la misma lo activa
            if(usuario->active == 1){
                mensaje+= "Error al dar de alta el nuevo usuario: el usuario [" + username + "] ya existe\n";
            }
            else if(password == usuario->password){
                modificarActivoUsuario(usuario, true);
                mensaje+= "El usuario [" + username + "] se ha dado de alta nuevamente!!\n";
            }
            else{
                mensaje+= "Error al reactivar al usuario [" + username + "]: contrasenia incorrecta\n";
            }

        }
        else{
            // si no está registrado guarda los datos
            ofstream autenticacion("autenticacion.txt",std::ios_base::app);
            autenticacion << username + ";" + password + ";COLECCIONISTA;1;\n";
            autenticacion.flush();
            autenticacion.close();

            mensaje+= "El usuario [" + username + "] se ha registrado correctamente!!\n";
        }

    }

    mensaje+= "--------------------------------------------------------------------------------\n\n";
    strcpy(datos, mensaje.data());


}

void registrarFiguritaMenu(char * datos, Server * server, User * user){

    int resultado;
    string comando;

    string opciones = "--------------------------------------------------------------------------------\n";
    opciones += "Registrar figurita:\n";
    opciones += "--------------------------------------------------------------------------------\n\n";
    opciones += "-> ingresar datos de la siguiente manera: pais;jugador\n\n";
    opciones += "-> |volver\n\n";
    opciones += "--------------------------------------------------------------------------------\n";

    strcpy(datos, (char*)"\n");

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = server->Recibir(datos);
        comando = datos;

        if(comando != "|volver"){
            registrarFigurita(datos, user);
            comando = "|volver";
        }else{
            strcpy(datos, (char*)"\n");
        }
    }while(resultado > 0 && comando != "|volver");
}


void registrarFigurita(char * datos, User * user){

    // extraigo los datos
    string pais, jugador;
    stringstream datosAux;

    datosAux << datos;

    getline(datosAux, pais, ';');
    getline(datosAux, jugador, ';');

    string mensaje = "--------------------------------------------------------------------------------\n";

    // los datos no pueden ser vacios
    if(pais.empty() || jugador.empty()){
        mensaje += "Error al registrar figurita: datos incompletos\n";
    }
    // el pais tiene que existir
    else if(!existePais(pais)){
        mensaje += "Error al registrar figurita: el pais ingresado no esta en el txt de paises\n";
    }
    // guardar los datos
    else{
        int id = 1;

        fstream figuritas("figuritas.txt");
        string linea;

        while(getline(figuritas, linea)){
            id++;
        }

        figuritas.close();

        figuritas.open("figuritas.txt", std::ios_base::app);
        figuritas << id << ";" + user->username + ";" + pais + ";" + jugador + ";1;\n";
        figuritas.flush();
        figuritas.close();

        mensaje += "Figurita registrada correctamente!!\n";
    }

    mensaje += "--------------------------------------------------------------------------------\n\n";
    strcpy(datos, mensaje.data());
}

bool existePais(string pais){
    bool existe = false;

    ifstream paises("paises.txt");
    string linea;

    while(getline(paises, linea) && !existe){

        if(linea == pais){
            existe = true;
        }

        linea = "";
    }

    paises.close();

    return existe;
}

Figurita* buscarFigurita(string usuario, string pais, string jugador){
    ifstream figuritas("figuritas.txt");
    Figurita * resultado = NULL;
    bool encontrado = false;
    string linea;

    while(getline(figuritas, linea) && !encontrado){
        stringstream datos;
        string id, usuarioAux, paisAux, jugadorAux, disponible;
        datos << linea;

        getline(datos, id, ';');
        getline(datos, usuarioAux, ';');
        getline(datos, paisAux, ';');
        getline(datos, jugadorAux, ';');
        getline(datos, disponible, ';');

        if(usuario == usuarioAux && pais == paisAux && jugador == jugadorAux && disponible == "1"){
            resultado = new Figurita(stoi(id), usuarioAux, paisAux, jugadorAux, disponible);
            encontrado = true;
        }

        linea = "";
    }

    return resultado;
}

int listadoFiguritas(User* user, char* datos){
    ifstream figuritas("figuritas.txt");
    int cant = 0;
    string linea;
    string listado = "";
    string mensaje = "--------------------------------------------------------------------------------\n";

    while(getline(figuritas, linea)){
        stringstream datos;
        string id, usuario, pais, jugador, disponible;
        datos << linea;

        getline(datos, id, ';');
        getline(datos, usuario, ';');
        getline(datos, pais, ';');
        getline(datos, jugador, ';');
        getline(datos, disponible, ';');

        if(usuario == user->username && disponible == "1"){
            listado+= "* " + pais + ";" + jugador + "\n";
            cant++;
        }

        linea = "";
    }

    if(cant > 0){
        mensaje += "Figuritas disponibles para intercambiar: " + std::to_string(cant) + "\n\n";
        mensaje += listado;
    }else{
        mensaje += "No hay figuritas disponibles para intercambiar\n";
    }

    mensaje += "--------------------------------------------------------------------------------\n\n";

    strcpy(datos, mensaje.data());

    return cant;
}

void menuIntercambio(Server* server, User* user, char* datos){
    int resultado;
    string comando;

    string opciones = "--------------------------------------------------------------------------------\n";
    opciones += "Intercambio:\n";
    opciones += "--------------------------------------------------------------------------------\n\n";
    opciones += "-> peticion\n\n";
    opciones += "-> cancelacion\n\n";
    opciones += "-> |volver\n\n";
    opciones += "--------------------------------------------------------------------------------\n";

    string opcionesPeticion = "--------------------------------------------------------------------------------\n";
    opcionesPeticion += "Peticion de intercambio:\n";
    opcionesPeticion += "--------------------------------------------------------------------------------\n\n";
    opcionesPeticion += "-> ingresar datos: paisOf;jugadorOf;paisReq;jugadorReq\n\n";
    opcionesPeticion += "-> |volver\n\n";
    opcionesPeticion += "--------------------------------------------------------------------------------\n";

    string opcionesCancelacion = "--------------------------------------------------------------------------------\n";
    opcionesCancelacion += "Cancelacion de intercambio:\n";
    opcionesCancelacion += "--------------------------------------------------------------------------------\n\n";
    opcionesCancelacion += "-> ingresar datos: paisOf;jugadorOf;paisReq;jugadorReq\n\n";
    opcionesCancelacion += "-> |volver\n\n";
    opcionesCancelacion += "--------------------------------------------------------------------------------\n";

    strcpy(datos, (char*)"\n");

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = server->Recibir(datos);
        comando = datos;

        if(comando == "peticion"){

            if(listadoFiguritas(user, datos) > 0){
                strcat(datos, opcionesPeticion.data());
                server->Enviar(datos);
                ZeroMemory(datos, 4096);

                resultado = server->Recibir(datos);

                comando = datos;

                if(comando == "|volver"){
                    strcpy(datos,(char*)"\n");
                }else if(validarDatosPeticion(user, datos)){
                    crearPeticion(user, datos);
                }
            }
            comando = "|volver";
        }
        else if(comando == "cancelacion"){

            if(listaPeticionesPendientes(user, datos) > 0){
                strcat(datos, opcionesCancelacion.data());
                server->Enviar(datos);
                ZeroMemory(datos, 4096);

                resultado = server->Recibir(datos);

                comando = datos;

                if(comando == "|volver"){
                    strcpy(datos,(char*)"\n");
                }else{
                    cancelarPeticion(user, datos);
                }

            }

            comando = "|volver";
        }
        else if(comando == "|volver"){
            strcpy(datos, (char*)"\n");
        }
        else{
            string mensaje= "--------------------------------------------------------------------------------\n";
            mensaje+= "Comando invalido\n";
            mensaje+= "--------------------------------------------------------------------------------\n\n";
            strcpy(datos, mensaje.data());

        }
    }while(resultado > 0 && comando != "|volver");
}

bool validarDatosPeticion(User* user, char*datos){
    string paisOf, jugadorOf, paisReq, jugadorReq;
    stringstream datosAux;
    bool valido = true;

    string error = "--------------------------------------------------------------------------------\n";
    error += "Error al crear la peticion de intercambio:\n";

    // extraer datos
    datosAux << datos;

    getline(datosAux, paisOf, ';');
    getline(datosAux, jugadorOf, ';');
    getline(datosAux, paisReq, ';');
    getline(datosAux, jugadorReq, ';');

    // validar datos completos

    if(paisOf.empty() || jugadorOf.empty() || paisReq.empty() || jugadorReq.empty()){
        error+= "Datos incompletos\n";
        valido = false;
    }

    if(paisOf.empty()){
        error+= "Falta pais ofrecido\n";
    }
    if(jugadorOf.empty()){
        error+= "Falta jugador ofrecido\n";
    }
    if(paisReq.empty()){
        error+= "Falta pais requerido\n";
    }
    if(jugadorReq.empty()){
        error+= "Falta jugador requerido\n";
    }

    // validar figurita ofrecida

    Figurita * figuritaOf = buscarFigurita(user->username, paisOf, jugadorOf);

    if(figuritaOf == NULL){
        error+= "La figurita ofrecida no pertenece a " + user->username + "\n";
        valido = false;
    }

    // validar figurita requerida

    if(!existePais(paisReq)){
        error+= "El pais de la figurita requerida no esta registrado\n";
        valido = false;
    }

    // tira el error

    if(!valido){
        error += "--------------------------------------------------------------------------------\n\n";
        strcpy(datos, error.data());
    }

    return valido;
}

void crearPeticion(User* user, char*datos){

    string paisOf, jugadorOf, paisReq, jugadorReq, linea;
    stringstream datosAux;

    string mensaje = "--------------------------------------------------------------------------------\n";

    // extraer datos

    datosAux << datos;

    getline(datosAux, paisOf, ';');
    getline(datosAux, jugadorOf, ';');
    getline(datosAux, paisReq, ';');
    getline(datosAux, jugadorReq, ';');

    // verifico peticion duplicada

    bool duplicada = false;
    fstream peticiones("peticiones.txt");

    string peticionAux = user->username + ";" + paisOf + ";" + jugadorOf + ";" + paisReq + ";" + jugadorReq + ";";

    while(getline(peticiones, linea) && !duplicada){
        if(linea == peticionAux + "PENDIENTE;"){
            duplicada = true;
        }

        linea = "";
    }

    peticiones.close();

    if(duplicada){
        mensaje += "Error al crear la peticion: peticion duplicada\n";
        mensaje += "--------------------------------------------------------------------------------\n\n";
        strcpy(datos, mensaje.data());
        return;
    }



    // busco emparejamiento

    string emparejamientoPeticion= paisReq + ";" + jugadorReq + ";" + paisOf + ";" + jugadorOf + ";";
    string emparejamientoAux= emparejamientoPeticion + "PENDIENTE;";
    string usuarioEmparejamiento;
    bool emparejado = false;

    peticiones.open("peticiones.txt");


    while(getline(peticiones, linea) && !emparejado){

        if(linea.find(emparejamientoAux) != std::string::npos){

            stringstream lineaAux;
            lineaAux << linea;
            getline(lineaAux, usuarioEmparejamiento, ';');

            // si el usuario es distinto al que hizo la peticion y si el emparejado aun posee la figurita
            if(usuarioEmparejamiento != user->username && buscarFigurita(usuarioEmparejamiento, paisReq, jugadorReq) != NULL){
                emparejado = true;
            }
        }

        linea = "";
    }

    peticiones.close();


    // realizar el intercambio y marcar como realizada la peticion emparejada

    if(emparejado){
        Figurita * f1 = buscarFigurita(user->username, paisOf, jugadorOf);
        Figurita * f2 = buscarFigurita(usuarioEmparejamiento, paisReq, jugadorReq);

        intercambiarFiguritas(f1, f2);

        emparejamientoAux= usuarioEmparejamiento + ";" + emparejamientoPeticion;
        modificarEstadoPeticion(emparejamientoAux, true);
    }


    // guardar la nueva peticion

    mensaje += "Nueva peticion creada correctamente!!\n";
    peticiones.open("peticiones.txt", std::ios_base::app);

    if(emparejado){
        peticiones << peticionAux << "REALIZADA;\n";
        mensaje += "\nIntercambio realizado con el usuario [" + usuarioEmparejamiento + "]!!\n";
    }else{
        peticiones << peticionAux << "PENDIENTE;\n";
        mensaje += "\nSu peticion se encuentra en estado pendiente\n";
    }

    mensaje += "--------------------------------------------------------------------------------\n\n";
    strcpy(datos, mensaje.data());

}

void intercambiarFiguritas(Figurita* figurita1, Figurita* figurita2){
    ifstream original("figuritas.txt");
    fstream copia("figuritasCopia.txt", std::ios_base::app);
    string linea;

    while(original >> linea){

        stringstream lineaAux;
        lineaAux << linea;
        string id;

        getline(lineaAux, id, ';');

        if(stoi(id) == figurita1->id){
            linea = id + ";" + figurita2->usuario + ";" + figurita1->pais + ";" + figurita1->jugador + ";" + figurita1->disponible + ";";
        }

        if(stoi(id) == figurita2->id){
            linea = id + ";" + figurita1->usuario + ";" + figurita2->pais + ";" + figurita2->jugador + ";" + figurita2->disponible + ";";
        }

        copia << linea << "\n";

        linea = "";
    }

    original.close();
    copia.close();

    remove("figuritas.txt");
    rename("figuritasCopia.txt", "figuritas.txt");
}

int listaPeticionesPendientes(User * user, char*datos){

    ifstream peticiones("peticiones.txt");
    string linea, lista = "";
    int cant = 0;
    string mensaje = "--------------------------------------------------------------------------------\n";

    while(getline(peticiones, linea)){

        stringstream lineaAux;
        string usuario;
        lineaAux << linea;

        getline(lineaAux, usuario, ';');

        if(usuario == user->username){
            string paisOf, jugadorOf, paisReq, jugadorReq, estado;

            getline(lineaAux, paisOf, ';');
            getline(lineaAux, jugadorOf, ';');
            getline(lineaAux, paisReq, ';');
            getline(lineaAux, jugadorReq, ';');
            getline(lineaAux, estado, ';');

            if(estado == "PENDIENTE"){
                cant++;
                lista+= std::to_string(cant) + ") Ofrecido: " + paisOf + ";" + jugadorOf + "  Requerido: " + paisReq + ";" + jugadorReq + "\n";
            }
        }

        linea="";
    }

    if(cant > 0){
        mensaje += "Peticiones pendientes: " + std::to_string(cant) + "\n\n";
        mensaje += lista;
    }else{
        mensaje += "No hay peticiones pendientes\n";
    }

    mensaje += "--------------------------------------------------------------------------------\n\n";

    strcpy(datos, mensaje.data());

    return cant;
}

void cancelarPeticion(User * user, char* datos){
    stringstream datosAux;
    datosAux << datos;
    string paisOf, jugadorOf, paisReq, jugadorReq;

    string mensaje = "--------------------------------------------------------------------------------\n";

    getline(datosAux, paisOf, ';');
    getline(datosAux, jugadorOf, ';');
    getline(datosAux, paisReq, ';');
    getline(datosAux, jugadorReq, ';');

    // valida que los datos sean correctos

    if(paisOf.empty() || jugadorOf.empty() || paisReq.empty() || jugadorReq.empty()){
        mensaje += "Error: Datos incompletos\n";
    }
    else{
        // cancela la peticion si existe

        string peticion =  user->username + ";" + paisOf + ";" + jugadorOf + ";" + paisReq + ";" + jugadorReq + ";";

        if(modificarEstadoPeticion(peticion, false)){
            mensaje += "La peticion fue cancelada con exito!!\n";
        }else{
            mensaje += "Error: La peticion ingresada no existe\n";
        }
    }

    mensaje += "--------------------------------------------------------------------------------\n\n";
    strcpy(datos, mensaje.data());
}

bool modificarEstadoPeticion(string peticion, bool estado){
    ifstream original("peticiones.txt");
    fstream copia("peticionesCopia.txt", std::ios_base::app);
    string linea;
    bool encontrado;
    string peticionAux = peticion + "PENDIENTE;";

    while(original >> linea){

        if(linea == peticionAux && !encontrado){
            encontrado = true;

            if(estado){
                linea = peticion + "REALIZADA;";
            }else{
                linea = peticion + "CANCELADA;";
            }
        }

        copia << linea << "\n";
    }

    original.close();
    copia.close();

    remove("peticiones.txt");
    rename("peticionesCopia.txt", "peticiones.txt");

    return encontrado;
}

int listaUsuariosActivos(char* datos){
    string mensaje = "--------------------------------------------------------------------------------\n";
    string lista = "";
    string linea;
    int cant = 0;

    ifstream autenticacion("autenticacion.txt");

    while(getline(autenticacion, linea)){
        string username, password, role, active;
        stringstream lineaAux;
        lineaAux << linea;

        getline(lineaAux, username, ';');
        getline(lineaAux, password, ';');
        getline(lineaAux, role, ';');
        getline(lineaAux, active, ';');

        if(role == "COLECCIONISTA" && active == "1"){
            lista+= "* " + username + "\n";
            cant++;
        }

        linea = "";
    }

    if(cant > 0){
        mensaje+= "Usuarios activos: " + std::to_string(cant) + "\n\n";
        mensaje+= lista;
    }else{
        mensaje+= "No hay usuarios activos\n";
    }

    mensaje+= "--------------------------------------------------------------------------------\n\n";

    strcpy(datos, mensaje.data());

    return cant;
}


void bajaUsuario(char*datos){

    string username = datos;
    User * usuario = buscarUsuario(username);

    string mensaje= "--------------------------------------------------------------------------------\n";

    if(usuario == NULL){
        mensaje+= "Error: el usuario [" + username + "] no existe\n";
    }
    else if(usuario->role != "COLECCIONISTA"){
        mensaje+= "Error: solo se pueden dar de baja usuarios de rol COLECCIONISTA\n";
    }
    else if(usuario->active == 0){
        mensaje+= "Error: el usuario [" + username + "] ya fue dado de baja\n";
    }
    else{
        modificarActivoUsuario(usuario, false);
        bajaPeticiones(username);
        bajaFiguritas(username);
        mensaje+= "El usuario [" + username + "] se dio de baja correctamente\n";
    }

    mensaje+= "--------------------------------------------------------------------------------\n\n";

    strcpy(datos, mensaje.data());
}

void modificarActivoUsuario(User* usuario, bool activo){
    ifstream original("autenticacion.txt");
    fstream copia("autenticacionCopia.txt", std::ios_base::app);
    string linea;

    while(original >> linea){

        if(linea.find(usuario->username) != std::string::npos){

            if(activo){
                linea = usuario->username + ";" + usuario->password + ";" + usuario->role + ";" + "1" + ";";
            }else{
                linea = usuario->username + ";" + usuario->password + ";" + usuario->role + ";" + "0" + ";";
            }

        }

        copia << linea << "\n";

        linea = "";
    }

    original.close();
    copia.close();

    remove("autenticacion.txt");
    rename("autenticacionCopia.txt", "autenticacion.txt");
}

void bajaPeticiones(string username){
    ifstream original("peticiones.txt");
    fstream copia("peticionesCopia.txt", std::ios_base::app);
    string linea;

    while(original >> linea){

        string usuario;
        stringstream lineaAux;
        lineaAux << linea;

        getline(lineaAux, usuario, ';');

        if(usuario == username){
            string paisOf, jugadorOf, paisReq, jugadorReq, estado;
            getline(lineaAux, paisOf, ';');
            getline(lineaAux, jugadorOf, ';');
            getline(lineaAux, paisReq, ';');
            getline(lineaAux, jugadorReq, ';');
            getline(lineaAux, estado, ';');

            if(estado == "PENDIENTE"){
                linea = usuario + ";" + paisOf + ";" + jugadorOf + ";" + paisReq + ";" + jugadorReq + ";" + "CANCELADA" + ";";
            }
        }

        copia << linea << "\n";

        linea = "";
    }

    original.close();
    copia.close();

    remove("peticiones.txt");
    rename("peticionesCopia.txt", "peticiones.txt");
}

void bajaFiguritas(string username){
    ifstream original("figuritas.txt");
    fstream copia("figuritasCopia.txt", std::ios_base::app);
    string linea;

    while(original >> linea){

        string id, usuario;
        stringstream lineaAux;
        lineaAux << linea;

        getline(lineaAux, id, ';');
        getline(lineaAux, usuario, ';');

        if(usuario == username){
            string pais, jugador;
            getline(lineaAux, pais, ';');
            getline(lineaAux, jugador, ';');

            linea = id + ";" + usuario + ";" + pais + ";" + jugador + ";" + "0" + ";";
        }

        copia << linea << "\n";

        linea = "";
    }

    original.close();
    copia.close();

    remove("figuritas.txt");
    rename("figuritasCopia.txt", "figuritas.txt");
}
