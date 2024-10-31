#include <iostream>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include "string.h"
#include <ctime>

using namespace std;



/// ---------------------------------------- Clases ----------------------------------------



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
    }

    void escuchar(){
        listen(server, 0);

        int clientAddrSize = sizeof(clientAddr);
        client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize);
    }

    int Recibir(char* datos){
        int resultado = recv(client, buffer, sizeof(buffer), 0);

        strcpy(datos, buffer);

        memset(buffer, 0, sizeof(buffer));

        return resultado;
    }


    void Enviar(char* datos){
        strcpy(buffer, datos);
        send(client, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
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



/// ---------------------------------------- Declaración de funciones ----------------------------------------



//PRE: se debe enviar el buffer con este formato: username;password
//POST: si los datos de usuario son correctos y el usuario está activo devuelve una clase user (inicia sesion),
// si son incorrectos o el usuario está inactivo devuelve NULL.
// escribe un mensaje de resultado en el buffer
User* iniciarSesion(char* datos);

//PRE: recibe el nombre del usuario a buscar
//POST: si el usuario está registrado en el archivo "autenticacion.txt" devuelve sus datos en una clase user,
// de lo contrario retorna NULL
User* buscarUsuario(string username);

//PRE: recibe un mensaje a escribir
//POST: escribe el mensaje en el archivo "server.log" con la hora actual
void serverLog(string log);

//PRE: recibe el buffer, servidor y usuario actual
//POST: ofrece un menú para el registro de figuritas del COLECCIONISTA
void registrarFiguritaMenu(char * datos, Server * server, User * user);

//PRE: recibe el buffer con este formato: pais;jugador
//POST: si los datos son correctos añade la nueva figurita en el archivo "figuritas.txt" con el nombre del usuario.
// escribe un mensaje de resultado en el buffer
void registrarFigurita(char * datos, User * user);

//PRE: se deben enviar los datos de la figurita a buscar
//POST: si se encuentra una figurita con estos datos en el archivo "figuritas.txt" los retorna en forma de clase
// de lo contrario retorna NULL
Figurita* buscarFigurita(string usuario, string pais, string jugador);

//PRE: se debe enviar el id de la figurita a buscar
//POST: si se encuentra una figurita con este id en el archivo "figuritas.txt" la retorna en forma de clase
// de lo contrario retorna NULL
Figurita* buscarFiguritaId(int id);

//PRE: se debe enviar un string con el nombre de un pais
//POST: si se encuentra el pais en el archivo "paises.txt" retorna true,
// si no se encuentra retorna false
bool existePais(string pais);

//PRE: se debe enviar el usuario actual y el buffer, no importan los datos del buffer
//POST: retorna la cantidad de figuritas disponibles del usuario.
// escribe la lista de figuritas en el buffer
int listadoFiguritas(User* user, char* datos);

//PRE: se debe enviar el servidor, el usuario actual y el buffer, no importan los datos del buffer
//POST: ofrece un menú para los intercambios de figuritas del COLECCIONISTA
void menuIntercambio(Server* server, User* user, char* datos);

//PRE: recibe el usuario actual y el buffer con el formato de peticion: id_figurita_ofrecida;pais_requerido;jugador_requerido
//POST: si los datos no son válidos para realizar la petición (id incorrecto, datos incompletos, etc)
// retorna false y escribe un mensaje de error en el buffer,
// si los datos son correctos retorna true y no escribe el buffer
bool validarDatosPeticion(User* user, char*datos);

//PRE: recibe el usuario actual y el buffer con el formato de peticion: id_figurita_ofrecida;pais_requerido;jugador_requerido
//POST: registra la nueva petición en el archivo "peticiones.txt" si no está duplicada.
// busca otra petición que se empareje con la nueva y si la encuentra realiza el intercambio.
// escribe un mensaje de resultado en el buffer
void crearPeticion(User* user, char*datos);

//PRE: recibe las dos figuritas a intercambiar
//POST: reescribe el archivo "figuritas.txt" intercambiando el campo usuario de ambas
void intercambiarFiguritas(Figurita* figurita1, Figurita* figurita2);

//PRE: recibe el usuario actual y el buffer con el numero de petición a cancelar
//POST: si el número es correcto reescribe el archivo "peticiones.txt" con la petición del usuario en estado CANCELADA.
// escribe un mensaje de resultado en el buffer
void cancelarPeticion(User * user, char* datos);

//PRE: se debe enviar un string con el formato de la petición a modificar: usuario;pais_ofrecido;jugador_ofrecido;pais_requerido;jugador_requerido;
// se debe enviar un booleano con el estado a modificar: true = REALIZADA, false = CANCELADA
//POST: si la petición existe retorna true y reescribe el archivo "peticiones.txt" con la modificación hecha
bool modificarEstadoPeticion(string peticion, bool estado);

//PRE: recibe el usuario actual y el buffer, no importan los datos del buffer
//POST: retorna la cantidad de peticiones pendientes del usuario.
// escribe la lista de peticiones pendientes en el buffer
int listaPeticionesPendientes(User * user, char*datos);

//PRE: recibe el buffer y el servidor
//POST: ofrece un menú para la gestión de usuarios del ADMIN
void usuariosMenu(char* datos, Server* server);

//PRE: se debe enviar el buffer con este formato: username;password
//POST: registra el nuevo usuario en el archivo "autenticacion.log"
// si el usuario ya estaba registrado e inactivo, lo activa.
// escribe un mensaje de resultado en el buffer
void altaUsuario(char* datos);

//PRE: se debe enviar el usuario y el estado del atributo "activo" a modificar
//POST: reescribe el archivo "autenticacion.log" modificando el atributo "activo" del usuario enviado
void modificarActivoUsuario(User* usuario, bool activo);

//PRE: recibe el buffer, no importa su contenido
//POST: retorna la cantidad de usuarios activos en el sistema.
// escribe la lista de usuarios en el buffer
int listaUsuariosActivos(char* datos);

//PRE: recibe el buffer con el nombre de usuario a dar de baja
//POST: si el usuario está activo y tiene rol coleccionista, lo da de baja.
// escribe un mensaje de resultado en el buffer
void bajaUsuario(char*datos);

//PRE: recibe el nombre de usuario dado de baja
//POST: reescribe el archivo "peticiones.txt"
// cambiando las peticiones del usuario en estado PENDIENTE por CANCELADA
void bajaPeticiones(string username);

//PRE: recibe el nombre de usuario dado de baja
//POST: reescribe el archivo "figuritas.txt"
// cambiando a 0 el estado "disponible" de las tarjetas del usuario
void bajaFiguritas(string username);

//PRE: recibe el buffer, no importa su contenido
//POST: escribe el registro de "server.log" en el buffer
void verRegistro(char* datos);



/// ---------------------------------------- Programa main ----------------------------------------



int main(){

    int puertoEscucha;

    cout << "Ingrese puerto de escucha:" << endl;
    cin >> puertoEscucha;
    system("cls");


    /// crear servidor
    Server *Servidor = new Server(puertoEscucha);
    char datos[4096];
    strcpy(datos, "\n");
    int resultado;

    //iniciar con un log vacio
    remove("server.log");

    serverLog("========================================================================================\n");
    serverLog("Servidor iniciado\n");
    serverLog("========================================================================================\n");
    serverLog("Socket creado. Puerto de escucha: " + std::to_string(Servidor->port) + "\n");

    string mensajeSesion = "--------------------------------------------------------------------------------\n";
    mensajeSesion += "Servicio de intercambio de figuritas -- iniciar sesion\n";
    mensajeSesion += "--------------------------------------------------------------------------------\n\n";
    mensajeSesion += "Ingrese sus datos de la siguiente forma: usuario;contrasenia\n\n";
    mensajeSesion += "--------------------------------------------------------------------------------\n";

    do{
        Servidor->escuchar();
        serverLog("Cliente conectado\n");

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
                    opciones += "1) registrar\n\n";
                    opciones += "2) intercambio\n\n";
                }

                if(usuario->role == "ADMIN"){
                    opciones += "1) usuarios\n\n";
                    opciones += "2) registro\n\n";
                }

                opciones += "0) salir\n\n";
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
                    if(comando == "1" && usuario->role == "COLECCIONISTA"){         //registrar
                        registrarFiguritaMenu(datos, Servidor, usuario);
                    }
                    else if(comando == "2" && usuario->role == "COLECCIONISTA"){    //intercambio
                        menuIntercambio(Servidor, usuario, datos);
                    }
                    else if(comando == "1" && usuario->role == "ADMIN"){            //usuarios
                        usuariosMenu(datos, Servidor);
                    }
                    else if(comando == "2" && usuario->role == "ADMIN"){            //registro
                        verRegistro(datos);
                    }
                    else if(comando == "0"){                                        //salir
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

                }while(comando != "0" && resultado > 0);
            }

        }while(resultado > 0);

        serverLog("Cliente desconectado\n");
        serverLog("Escuchando para conexiones entrantes en el puerto " + std::to_string(Servidor->port) + "\n");
    }while(true);
}



/// ---------------------------------------- Implementación de funciones ----------------------------------------



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
    cout << timeOutput << ": " + log;
    archivo.flush();
    archivo.close();
}



void registrarFiguritaMenu(char * datos, Server * server, User * user){

    int resultado;
    string comando;

    string opciones = "--------------------------------------------------------------------------------\n";
    opciones += "Registrar figurita:\n";
    opciones += "--------------------------------------------------------------------------------\n\n";
    opciones += "-> ingresar datos de la siguiente manera: pais;jugador\n\n";
    opciones += "0) volver\n\n";
    opciones += "--------------------------------------------------------------------------------\n";

    strcpy(datos, (char*)"\n");

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = server->Recibir(datos);
        comando = datos;

        if(comando != "0"){
            registrarFigurita(datos, user);
            comando = "0";
        }else{
            strcpy(datos, (char*)"\n");
        }
    }while(resultado > 0 && comando != "0");
}



void registrarFigurita(char * datos, User * user){

    string pais, jugador;
    stringstream datosAux;

    datosAux << datos;

    getline(datosAux, pais, ';');
    getline(datosAux, jugador, ';');

    string mensaje = "--------------------------------------------------------------------------------\n";

    // los datos no pueden ser vacios
    if(pais.empty() || jugador.empty()){
        mensaje += "Error al registrar figurita: datos incompletos\n";

        if(pais.empty()){
            mensaje += "Falta pais\n";
        }else{
            mensaje += "Falta jugador\n";
        }
    }
    else{
        // el pais tiene que existir
        pais[0] = toupper(pais[0]);

        if(!existePais(pais)){
            mensaje += "Error al registrar figurita: el pais ingresado no se encuentra en el txt de paises\n";
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
    }

    mensaje += "--------------------------------------------------------------------------------\n\n";
    strcpy(datos, mensaje.data());
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



Figurita* buscarFiguritaId(int id){
    ifstream figuritas("figuritas.txt");
    Figurita * resultado = NULL;
    bool encontrado = false;
    string linea;

    while(getline(figuritas, linea) && !encontrado){
        stringstream datos;
        string idAux, usuario, pais, jugador, disponible;
        datos << linea;

        getline(datos, idAux, ';');
        getline(datos, usuario, ';');
        getline(datos, pais, ';');
        getline(datos, jugador, ';');
        getline(datos, disponible, ';');

        if(stoi(idAux) == id){
            resultado = new Figurita(id, usuario, pais, jugador, disponible);
            encontrado = true;
        }

        linea = "";
    }

    return resultado;
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
            listado+= id +") " + pais + ";" + jugador + "\n";
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
    opciones += "1) peticion\n\n";
    opciones += "2) cancelacion\n\n";
    opciones += "0) volver\n\n";
    opciones += "--------------------------------------------------------------------------------\n";

    string opcionesPeticion = "--------------------------------------------------------------------------------\n";
    opcionesPeticion += "Peticion de intercambio:\n";
    opcionesPeticion += "--------------------------------------------------------------------------------\n\n";
    opcionesPeticion += "-> ingresar datos: id_figurita_ofrecida;pais_requerido;jugador_requerido\n\n";
    opcionesPeticion += "0) volver\n\n";
    opcionesPeticion += "--------------------------------------------------------------------------------\n";

    string opcionesCancelacion = "--------------------------------------------------------------------------------\n";
    opcionesCancelacion += "Cancelacion de intercambio:\n";
    opcionesCancelacion += "--------------------------------------------------------------------------------\n\n";
    opcionesCancelacion += "-> ingresar numero de la peticion a cancelar: \n\n";
    opcionesCancelacion += "0) volver\n\n";
    opcionesCancelacion += "--------------------------------------------------------------------------------\n";

    strcpy(datos, (char*)"\n");

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = server->Recibir(datos);
        comando = datos;

        if(comando == "1"){                 //peticion

            if(listadoFiguritas(user, datos) > 0){
                strcat(datos, opcionesPeticion.data());
                server->Enviar(datos);
                ZeroMemory(datos, 4096);

                resultado = server->Recibir(datos);

                comando = datos;

                if(comando == "0"){         //volver
                    strcpy(datos,(char*)"\n");
                }
                else if(validarDatosPeticion(user, datos)){
                    crearPeticion(user, datos);
                }
            }
            comando = "0";
        }
        else if(comando == "2"){            //cancelacion

            if(listaPeticionesPendientes(user, datos) > 0){
                strcat(datos, opcionesCancelacion.data());
                server->Enviar(datos);
                ZeroMemory(datos, 4096);

                resultado = server->Recibir(datos);

                comando = datos;

                if(comando == "0"){         //volver
                    strcpy(datos,(char*)"\n");
                }else{
                    cancelarPeticion(user, datos);
                }

            }

            comando = "0";
        }
        else if(comando == "0"){            //volver
            strcpy(datos, (char*)"\n");
        }
        else{
            string mensaje= "--------------------------------------------------------------------------------\n";
            mensaje+= "Comando invalido\n";
            mensaje+= "--------------------------------------------------------------------------------\n\n";
            strcpy(datos, mensaje.data());

        }
    }while(resultado > 0 && comando != "0");
}



bool validarDatosPeticion(User* user, char*datos){

    bool valido = true;
    string idOf, paisReq, jugadorReq;
    stringstream datosAux;

    datosAux << datos;

    getline(datosAux, idOf, ';');
    getline(datosAux, paisReq, ';');
    getline(datosAux, jugadorReq, ';');

    string error = "--------------------------------------------------------------------------------\n";
    error += "Error al crear la peticion de intercambio:\n";

    // validar datos completos

    if(idOf.empty() || paisReq.empty() || jugadorReq.empty()){
        error+= "Datos incompletos\n";
        valido = false;
    }
    if(idOf.empty()){
        error+= "Falta id de la figurita ofrecida\n";
    }
    if(paisReq.empty()){
        error+= "Falta pais requerido\n";
    }
    if(jugadorReq.empty()){
        error+= "Falta jugador requerido\n";
    }


    // validar figurita ofrecida
    Figurita * figuritaOf = NULL;

    //comprobar que el id es un número
    stringstream idAux(idOf);
    int id;
    idAux >> id;

    if(idAux.eof() && !idAux.fail()){
        figuritaOf = buscarFiguritaId(id);
    }else{
        error+= "El id es incorrecto\n";
        valido = false;
    }

    if(figuritaOf == NULL){
        error+= "No existe una figurita con el id [" + idOf + "]\n";
        valido = false;
    }
    else if(figuritaOf->usuario != user->username){
        error+= "La figurita con el id [" + idOf + "] no pertenece a [" + user->username + "]\n";
        valido = false;
    }

    // validar figurita requerida

    paisReq[0] = toupper(paisReq[0]);

    if(!existePais(paisReq)){
        error+= "El pais requerido no se encuentra en el txt de paises\n";
        valido = false;
    }

    // si no pasa la validación tira el error

    if(!valido){
        error += "--------------------------------------------------------------------------------\n\n";
        strcpy(datos, error.data());
    }

    return valido;
}



void crearPeticion(User* user, char*datos){

    string idOf, paisReq, jugadorReq, linea;
    stringstream datosAux;

    string mensaje = "--------------------------------------------------------------------------------\n";

    datosAux << datos;

    getline(datosAux, idOf, ';');
    getline(datosAux, paisReq, ';');
    getline(datosAux, jugadorReq, ';');

    paisReq[0] = toupper(paisReq[0]);

    Figurita* f1 = buscarFiguritaId(stoi(idOf));

    // verificar peticion duplicada

    bool duplicada = false;
    fstream peticiones("peticiones.txt");

    string peticionAux = user->username + ";" + f1->pais + ";" + f1->jugador + ";" + paisReq + ";" + jugadorReq + ";";

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

    string emparejamientoPeticion= paisReq + ";" + jugadorReq + ";" + f1->pais + ";" + f1->jugador + ";";
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

    peticiones.close();
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



void cancelarPeticion(User * user, char* datos){

    string mensaje = "--------------------------------------------------------------------------------\n";

    stringstream numAux;
    numAux << datos;
    int num;
    numAux >> num;

    // verifica que se ingresó un numero
    if(numAux.eof() && numAux.fail()){
        mensaje+= "Error: datos incorrectos\n";
    }
    else{
        // verifica que se ingresó el numero correcto
        int cantPeticiones = listaPeticionesPendientes(user, datos);
        if(num <= 0 || num > cantPeticiones){
            mensaje+= "Error: datos incorrectos\n";
        }
        // busca la n petición pendiente y la cancela
        else{
            string linea, username, paisOf, jugadorOf, paisReq, jugadorReq;
            ifstream peticiones("peticiones.txt");
            int i = 0;

            while(getline(peticiones, linea) && i != num){
                stringstream lineaAux;
                lineaAux << linea;

                getline(lineaAux, username, ';');

                if(username == user->username && linea.find("PENDIENTE") != std::string::npos){
                    i++;

                    if(i == num){
                        getline(lineaAux, paisOf, ';');
                        getline(lineaAux, jugadorOf, ';');
                        getline(lineaAux, paisReq, ';');
                        getline(lineaAux, jugadorReq, ';');
                    }
                }

                linea= "";
            }

            peticiones.close();

            string peticion =  user->username + ";" + paisOf + ";" + jugadorOf + ";" + paisReq + ";" + jugadorReq + ";";

            if(modificarEstadoPeticion(peticion, false)){
                mensaje += "La peticion fue cancelada con exito!!\n";
            }else{
                mensaje += "Error: La peticion ingresada no existe\n";
            }
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



void usuariosMenu(char* datos, Server* server){
    int resultado;
    strcpy(datos, (char*)"\n");
    string comando;

    string opciones = "--------------------------------------------------------------------------------\n";
    opciones += "Usuarios:\n";
    opciones += "--------------------------------------------------------------------------------\n\n";
    opciones += "1) alta\n\n";
    opciones += "2) baja\n\n";
    opciones += "0) volver\n\n";
    opciones += "--------------------------------------------------------------------------------\n";

    string opcionesAlta = "--------------------------------------------------------------------------------\n";
    opcionesAlta += "Alta de usuarios:\n";
    opcionesAlta += "--------------------------------------------------------------------------------\n\n";
    opcionesAlta += "-> ingresar datos de la siguiente manera: nombre;contrasenia\n\n";
    opcionesAlta += "0) volver\n\n";
    opcionesAlta += "--------------------------------------------------------------------------------\n";

    string opcionesBaja = "--------------------------------------------------------------------------------\n";
    opcionesBaja += "Baja de usuarios:\n";
    opcionesBaja += "--------------------------------------------------------------------------------\n\n";
    opcionesBaja += "-> nombre del usuario\n\n";
    opcionesBaja += "0) volver\n\n";
    opcionesBaja += "--------------------------------------------------------------------------------\n";

    do{
        strcat(datos, opciones.data());

        server->Enviar(datos);
        ZeroMemory(datos, 4096);

        resultado = server->Recibir(datos);
        comando = datos;

        /// menú de alta usuarios
        if(comando == "1"){                     //alta

            strcpy(datos, opcionesAlta.data());

            server->Enviar(datos);
            ZeroMemory(datos, 4096);

            resultado = server->Recibir(datos);
            comando = datos;

            if(comando != "0"){                 //volver
                altaUsuario(datos);
                comando = "0";
            }else{
                strcpy(datos, (char*)"\n");
            }
        }


        /// menú de baja usuarios
        else if(comando == "2"){                //baja
            if(listaUsuariosActivos(datos) > 0){
                strcat(datos, opcionesBaja.data());

                server->Enviar(datos);
                ZeroMemory(datos, 4096);

                resultado = server->Recibir(datos);
                comando = datos;

                if(comando != "0"){             //volver
                    bajaUsuario(datos);
                    comando = "0";
                }else{
                    strcpy(datos, (char*)"\n");
                }
            }else{
                comando = "0";
            }
        }
        else if(comando == "0"){                //volver
            strcpy(datos, (char*)"\n");
        }
        else{
            string mensaje= "--------------------------------------------------------------------------------\n";
            mensaje+= "Comando invalido\n";
            mensaje+= "--------------------------------------------------------------------------------\n\n";
            strcpy(datos, mensaje.data());

        }
    }while(resultado > 0 && comando != "0");
}



void altaUsuario(char* datos){

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



void verRegistro(char* datos){
    ifstream registro("server.log");

    //si el archivo está vacío
    if(registro.peek() == std::ifstream::traits_type::eof()){
        string mensaje = "--------------------------------------------------------------------------------\n";
        mensaje += "No hay registros\n";
        mensaje += "--------------------------------------------------------------------------------\n\n";
        strcpy(datos, mensaje.data());

    }else{
        //copio todo el log en un string
        string mensaje((std::istreambuf_iterator<char>(registro)), std::istreambuf_iterator<char>());
        strcpy(datos, mensaje.data());
        strcat(datos, (char*)"\n\n");
    }

}
