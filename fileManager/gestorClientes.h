#pragma once
#include "utils.h"
#include "filemanager.h"

using namespace std;

typedef enum{
	FileDF, //constructor por defecto
	FileDE, //Destructor
	FilePP,//constructor con parametros
	FileList, //Listar Fichero
	RedFile, // leer Fichero
	WriteFile,// Escribir Fichero
	ackMSG //mensaje extra de confirmación
}FileMSGTypes;

class GestorClientes{
	
	public:
	//Usado por el cliente
	//Mapa para almacenar y buscar datos de conexión
		static inline map<FileManager*,connection_t > connections;
	//Usado por el servidor
	//Mapa para almacenar y buscar personas reservadas por clientes
		static inline map<int,FileManager > clients; 
	//Método usado por el servidor para gestionar peticiones de invocación
	//de métodos recibido de programas tipo cliente.
	//Cada nuevo cliente conectado al sistema lleva asociado un "clientId" que 
	//se usará en sus comunicaciones
		static void atiendeCliente(int clientId);

		
};

