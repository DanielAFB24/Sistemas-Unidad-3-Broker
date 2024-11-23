#include "gestorClientes.h"
#include <sstream>      

void GestorClientes::atiendeCliente(int clientId){
	bool salir=false;
	vector<unsigned char> buffer;//Buffer de datos recibidos desde le cliente
	//Mantener la conexión hasta que el cliente invoque el destructor
	while(!salir)
	{
		//recibir mensaje
		recvMSG(clientId,buffer);
		//tratar mensaje
		//desempaquetar tipo de invocación
		auto tipo=unpack<FileMSGTypes>(buffer);
		//swith tipo
		switch(tipo){ //crear un caso para cada tipo de mensaje
			case FileDF:{
				//crear una persona llamando al constructor por defecto
				FileManager p;
				//almacenar la persona usando el identificador de cliente
				//para poder referenciarlo en el resto de llamadas
				clients[clientId]=p;
				//retornar un mensaje ACK
				buffer.clear();//limpiar el buffer antes de reutilizarlo
				pack(buffer,ackMSG);//almacenar mensaje
				//en este punto termina la invocación, el mensaje se envía
				//al final del cuerpo switch-case
			}break;
			
			case FilePP:{
				string path; //Variables para almacenar nombre/edad
				
				//Reconstruimos el path desempaquetando sus datos
				//- Desempaquetamos y redimensionamos el tamaño del path
				path.resize(unpack<int>(buffer));
				//- Desempaquetamos datos/caracteres del nombre
				unpackv<char>(buffer,(char*)path.data(),(int)path.size());
				//crear una persona llamando al constructor con parámetros
				FileManager fm(path);
				//almacenar la file usando el identificador de cliente
				//para poder referenciarlo en el resto de llamadas
				clients[clientId]=fm;
				//retornar un mensaje ACK
				buffer.clear();//limpiar el buffer antes de reutilizarlo
				pack(buffer,ackMSG);//almacenar mensaje
				//en este punto termina la invocación, el mensaje se envía
				//al final del cuerpo switch-case
				
			}break;
			
			case FileDE:{
				//Invocamos el destructor eliminando la instancia de filemanager
				//de la lista de clientes
				clients.erase(clientId);
				//acabamos el bucle "while" que mantiene la conexión con el cliente
				salir=true;
				//retornar un mensaje ACK
				buffer.clear();
				//limpiar el buffer antes de reutilizarlo
				pack(buffer,ackMSG);//almacenar mensaje
				//en este punto termina la invocación, el mensaje se envía
				//al final del cuerpo switch-case
			}break;
			
			case FileList:{
				//obtenemos el filemanager asociado al clientr
				FileManager& fm = clients[clientId];
				//obtener fichero
				std::vector<std::string> fileList = fm.listFiles();

				//limpiamos buffer
				buffer.clear();
				pack(buffer,ackMSG);
				//Empaquetamos tmaño de la lista
				pack(buffer, (int) fileList.size());
				//Empaquetamos cada datos de los archivos
				// Empaquetar cada archivo en el buffer
        		for (const std::string& file : fileList) {
					// Empaquetar la longitud del nombre del archivo
					pack(buffer, (int)file.size());
					// Empaquetar el contenido del nombre del archivo
					packv(buffer, file.data(), (int)file.size());
        		}

				//sendMSG(clientId, buffer);


			}break;
			
			case WriteFile:{
				// Obtener el FileManager asociado al cliente
				FileManager& fm = clients[clientId];

				std::string fileName;
				int filesize = unpack<int>(buffer);
				fileName.resize(filesize);
				unpackv(buffer, (unsigned char*)fileName.data(), (int)filesize);

				// Desempaquetar el contenido del archivo
				int dataSize = unpack<int>(buffer);
				std::vector<unsigned char> data(dataSize);
				unpackv(buffer, (unsigned char*)data.data(), (int)data.size());

				
				// Escribir el archivo en el directorio del servidor
				fm.writeFile(fileName, data);

				// Enviar confirmación al cliente
				buffer.clear();
				pack(buffer, ackMSG);
				//sendMSG(clientId, buffer);

				/*
				FileManager& fm = clients[clientId];

				// Desempaquetar el tamaño del nombre del archivo
				int filenameSize = unpack<int>(buffer);

				// Desempaquetar el nombre del archivo
				std::string filename;
				filename.resize(filenameSize);
				unpackv(buffer, (unsigned char*)filename.data(),(int) filenameSize);

				// Desempaquetar el tamaño de los datos del archivo
				int dataSize = unpack<int>(buffer);

				// Desempaquetar los datos del archivo
				std::vector<unsigned char> data;
				data.resize(dataSize);
				unpackv(buffer, data.data(), (int)dataSize);

				// Escribir el archivo en el servidor
				fm.writeFile(filename, data);

				// Enviar confirmación al cliente
				buffer.clear();
				pack(buffer, ackMSG);*/
			}break;
			
			case RedFile:{
				// Desempaquetar el nombre del archivo
				std::string filename;
				filename.resize(unpack<int>(buffer));
				unpackv(buffer, (char*)filename.data(), filename.size());

				// Obtener el FileManager asociado al cliente
				FileManager& fm = clients[clientId];

				// Leer el archivo desde el directorio gestionado por FileManager
				std::vector<unsigned char> fileData;
				fm.readFile(filename, fileData);

				// Empaquetar la respuesta
				buffer.clear();
				pack(buffer, ackMSG); // Confirmar la lectura
				pack(buffer, (int)fileData.size()); // Tamaño del archivo
				packv(buffer, fileData.data(), fileData.size()); // Datos del archivo
				}break;
			
			case ackMSG: //No debemos recibir mensajes de ACK, ERROR
			default:
			{
			//default: mensaje error
				cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<
						"Tipo de mensaje inválido"<<endl;
			}break;
		};
		//En cada caso, se debe rellenar el buffer de respuesta
		//Una vez relleno, se envía al cliente:
		sendMSG(clientId,buffer);
	}
	//Al acabar, cerrar conexion
	closeConnection(clientId);
}





