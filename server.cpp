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
void serverLog(string log);
void verRegistro(char* datos);
void usuariosMenu(char* datos, Server* server);
void altaUsuario(char* datos);
void registrarFiguritaMenu(char * datos, Server * server, User * user);
void registrarFigurita(char * datos, User * user);
bool existePais(string pais);
void paisesRegistrados(char * datos);
Figurita* buscarFigurita(string usuario, string pais, string jugador);
int listadoFiguritas(User* user, char* datos);
void menuIntercambio(Server* server, User* user, char* datos);
bool validarDatosPeticion(User* user, char*datos);
void crearPeticion(User* user, char*datos);
void intercambiarFiguritas(Figurita* figurita1, Figurita* figurita2);
void peticionRealizada(string peticion);


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
                opciones += "-> registrar\n";
                opciones += "-> intercambio\n";
                opciones += "-> paises\n";
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
                if(comando == "registrar" && usuario->role == "COLECCIONISTA"){
                    registrarFiguritaMenu(datos, Servidor, usuario);
                }
                else if(comando == "intercambio" && usuario->role == "COLECCIONISTA"){
                    //listadoFiguritas(usuario, datos);
                    menuIntercambio(Servidor, usuario, datos);
                }
                else if(comando == "paises" && usuario->role == "COLECCIONISTA"){
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
            Figurita * fig = buscarFigurita("queso", "argentina", "messi");
            if(fig != NULL){
                cout << "\nFIGURITA:\n";
                cout << fig->id << ", ";
                cout << fig->usuario << ", ";
                cout << fig->pais << ", ";
                cout << fig->jugador << ", ";
                cout << fig->disponible << endl;
            }
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
    opciones += "-> ingresar datos de la siguiente manera: pais;jugador\n\n";
    opciones += "-> volver\n\n";
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

    //cout << datos <<endl;

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

    if(!existePais(pais)){
        strcpy(datos, (char*)"Error al registrar figurita: el pais ingresado no esta en la lista de paises registrados.\nLa lista puede verse con la opcion paises.\n\n");
        return;
    }

    // guardar los datos
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

    string mensaje = "Figurita registrada correctamente!\n\n";
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

    //mensaje += "--------------------------------------------------------------------------------\n\n";

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

    mensaje += "--------------------------------------------------------------------------------\n";

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
    opciones += "-> volver\n\n";
    opciones += "--------------------------------------------------------------------------------\n";

    string opcionesPeticion = "--------------------------------------------------------------------------------\n";
    opcionesPeticion += "Peticion:\n";
    opcionesPeticion += "--------------------------------------------------------------------------------\n\n";
    opcionesPeticion += "-> ingresar datos: paisOf;jugadorOf;paisReq;jugadorReq\n\n";
    opcionesPeticion += "-> volver\n\n";
    opcionesPeticion += "--------------------------------------------------------------------------------\n";

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

                if(validarDatosPeticion(user, datos)){
                    crearPeticion(user, datos);
                }
                //strcpy(datos, (char*)"Peticion creada\n\n");
            }
            comando = "volver";
        }
        else if(comando == "cancelacion"){
            strcpy(datos, (char*)"cancelada crack\n\n");
            comando = "volver";
        }
        else if(comando == "volver"){
            strcpy(datos, (char*)"\n");
        }
        else{
            strcpy(datos, (char*)"comando invalido\n");
        }
    }while(resultado > 0 && comando != "volver");
}

bool validarDatosPeticion(User* user, char*datos){
    string paisOf, jugadorOf, paisReq, jugadorReq;
    string error = "Error al crear la peticion de intercambio:\n";
    bool valido = true;
    stringstream datosAux;

    // extraer datos
    datosAux << datos;

    getline(datosAux, paisOf, ';');
    getline(datosAux, jugadorOf, ';');
    getline(datosAux, paisReq, ';');
    getline(datosAux, jugadorReq, ';');

    // validar datos completos

    if(paisOf.empty() || jugadorOf.empty() || paisReq.empty() || jugadorReq.empty()){
        error+= "Datos incompletos.\n";
        valido = false;
    }

    if(paisOf.empty()){
        error+= "Falta pais ofrecido.\n";
    }
    if(jugadorOf.empty()){
        error+= "Falta jugador ofrecido.\n";
    }
    if(paisReq.empty()){
        error+= "Falta pais requerido.\n";
    }
    if(jugadorReq.empty()){
        error+= "Falta jugador requerido.\n";
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

    // peticion duplicada

    // tira el error

    if(!valido){
        strcpy(datos, error.data());
    }

    return valido;
}

void crearPeticion(User* user, char*datos){

    string paisOf, jugadorOf, paisReq, jugadorReq, linea, mensaje;
    stringstream datosAux;

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

    if(duplicada){
        strcpy(datos, (char *)"Error al crear la peticion: PETICION DUPLICADA\n\n");
        return;
    }

    peticiones.close();

    // busco emparejamiento

    string emparejamientoAux= paisReq + ";" + jugadorReq + ";" + paisOf + ";" + jugadorOf + ";PENDIENTE;";
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

        emparejamientoAux= usuarioEmparejamiento + ";" + emparejamientoAux;
        peticionRealizada(emparejamientoAux);
    }


    // guardar la nueva peticion

    strcpy(datos, (char*)"Nueva peticion creada correctamente\n");
    peticiones.open("peticiones.txt", std::ios_base::app);

    if(emparejado){
        peticiones << peticionAux << "REALIZADA;\n";
        strcat(datos, (char*)"Intercambio realizado!!\n\n");
    }else{
        peticiones << peticionAux << "PENDIENTE;\n";
        strcat(datos, (char*)"Su peticion se encuentra en estado pendiente\n\n");
    }

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

void peticionRealizada(string peticion){
    ifstream original("peticiones.txt");
    fstream copia("peticionesCopia.txt", std::ios_base::app);
    string linea;

    while(original >> linea){

        if(linea == peticion){
            string palabra;
            stringstream lineaAux;
            lineaAux << linea;
            linea = "";

            getline(lineaAux, palabra, ';');

            while(palabra != "PENDIENTE"){
                linea += palabra + ";";
                getline(lineaAux, palabra, ';');
            };

            linea += "REALIZADA;";
        }

        copia << linea << "\n";
    }

    original.close();
    copia.close();

    remove("peticiones.txt");
    rename("peticionesCopia.txt", "peticiones.txt");
};
