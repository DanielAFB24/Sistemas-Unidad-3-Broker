#include "filemanager.h"
#include "gestorClientes.h"

// Tipos de mensajes del Broker
enum BrokerMSGTypes {
    BrokerRegisterServer,
    BrokerRequestServer,
    BrokerAck,
    BrokerError
};

std::string searchIpServerBroker(){
	std::vector<unsigned char> buffer;
	std::string brokerIp = "3.93.250.107";
	std::string serverIp;

	auto connToBroker = initClient(brokerIp, 5000);

	 // Solicitar servidor al Broker
    pack(buffer, BrokerRequestServer);
    sendMSG(connToBroker.serverId, buffer);

	// Recibir respuesta del Broker
    buffer.clear();
    recvMSG(connToBroker.serverId, buffer);

	//verificamos que no llegue vacio
	if (buffer.empty()) {
        std::cerr << "ERROR: No se recibió respuesta del Broker" << std::endl;
        closeConnection(connToBroker.serverId);
        throw std::runtime_error("No se pudo obtener un servidor del Broker");
    }

	//Desempaquetamos el tipo de mensajes
	BrokerMSGTypes msgType = unpack<BrokerMSGTypes>(buffer);

	//Verificamos que el tipo de mensaje sea el correcto
	if(msgType != BrokerAck){
		std::cerr << "ERROR: No se recibió respuesta esperada" << std::endl;
        closeConnection(connToBroker.serverId);
        throw std::runtime_error("No se pudo obtener un servidor del Broker");
	}

	//Desempaquetamos el serverID
    serverIp.resize(unpack<int>(buffer));
    unpackv(buffer, (char*)serverIp.data(), serverIp.size());

    closeConnection(connToBroker.serverId);

	return serverIp;

}

FileManager::FileManager(){

	//solicitamos direccion ip del servidor a travez del broker
	std::string serverIp = searchIpServerBroker();


    //iniciar conexion
	auto conn=initClient(serverIp,5000);

	//invocar constructor en server:
	//crear buffer
	vector<unsigned char> buffer;
	//empaquetar tipo de llamada constructor por defecto de persona

    pack(buffer,FileDF);
	//mandar mensaje
	sendMSG(conn.serverId, buffer);

    buffer.clear(); //limpiar buffer antes de reutilizarlo
	recvMSG(conn.serverId,buffer); //recibir respuesta de servidor
	
	if(buffer.size()==0) //Error estándar, no hay datos recibidos
	{
			cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<
						"Tamaño de mensaje inválido"<<endl; //Mensaje estándar error
	}
	else{//Debería haber un mensaje, debe ser un ACK	
		FileMSGTypes ack=unpack<FileMSGTypes>(buffer);
		//si no recibo confirmación
		if(ack!=ackMSG)
			cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<
						"Tipo de mensaje inválido"<<endl;	
		//error
		else{ 
			//Todo correcto, se almancena esta conexión y devuelve el control
			//a programa main
			GestorClientes::connections[this]=conn;
		}
	}		
}

FileManager::FileManager(std::string path) {
    //solicitamos direccion ip del servidor a travez del broker
	std::string serverIp = searchIpServerBroker();


    //iniciar conexion
	auto conn=initClient(serverIp,5000);

	//invocar constructor en server:
	//crear buffer
	vector<unsigned char> buffer;
	//empaquetar tipo de llamada constructor por parametro
    pack(buffer, FilePP);
    //Empaquetar tamaño de path
    pack(buffer,(int)path.size());
    //Empaquetar path
    packv(buffer,(char*)path.data(),(int)path.size()); 

    //mandar mensaje
	sendMSG(conn.serverId, buffer);
	//esperar confirmación de ejecución
	buffer.clear();
	recvMSG(conn.serverId,buffer);
	
	if(buffer.size()==0)
	{
			cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<"Tamaño de mensaje inválido"<<endl;							//error
	}
	else{	
		FileMSGTypes ack=unpack<FileMSGTypes>(buffer);
		if(ack!=ackMSG)
			cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<	"Tipo de mensaje inválido"<<endl;	
		else{
			GestorClientes::connections[this]=conn;
		}
	}

}

FileManager::~FileManager() {
   //Buscar datos de conexión
	auto conn=GestorClientes::connections[this];
	
	vector<unsigned char> buffer;
	//Empaquetar invocación de destructor
	pack(buffer,FileDE);
	//Enviar
	sendMSG(conn.serverId, buffer);
	//recibir confirmación de ejecución
	buffer.clear();
	recvMSG(conn.serverId,buffer);
	
	if(buffer.size()==0)
	{
			cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<
						"Tamaño de mensaje inválido"<<endl;							//error
	}
	else{	
		FileMSGTypes ack=unpack<FileMSGTypes>(buffer);
		if(ack!=ackMSG)
			cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<
						"Tipo de mensaje inválido"<<endl;
		else{
			//si todo es correcto, cerrar conexión
			closeConnection(conn.serverId);
			//eliminar datos de conexión
			GestorClientes::connections.erase(this);
		}
	}	
}

vector<string> FileManager::listFiles() {
    // Implementación de listFiles
    auto conn=GestorClientes::connections[this];
    std::vector<std::string> resultado;
	vector<unsigned char> buffer;
	//Empaquetar invocación listFile
    pack(buffer,FileList);
    //Enviamos la peticion
    sendMSG(conn.serverId, buffer);
    //Limpiamos buffer
    buffer.clear();
    //Recibimos la informacion
    recvMSG(conn.serverId, buffer);

    if(buffer.size()==0)
	{
			cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<
						"Tamaño de mensaje inválido"<<endl;							//error
	}
	else{	
		FileMSGTypes ack=unpack<FileMSGTypes>(buffer);
		if(ack!=ackMSG)
			cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<
						"Tipo de mensaje inválido"<<endl;
		else{
			//si todo es correcto, desempaquetar datos
            int numFile = unpack<int>(buffer);

            for(int i = 0; i < numFile; ++i){
                std::string fileName = "";
                fileName.resize(unpack<int>(buffer));
                unpackv(buffer, (char*) fileName.data(), fileName.size());
                resultado.push_back(fileName);
            }            
		}
	}	
	//siempre devuelve un resultado
	return resultado;
}

void FileManager::writeFile(std::string filename, std::vector<unsigned char>& data) {
    // Implementación de writeFile
	auto conn = GestorClientes::connections[this];
	std::vector<std::string> resultado;
	vector<unsigned char> buffer;
	//Empaquetamos el tipo de accion
	pack(buffer, WriteFile);
	//Enpaquetamos el tamaño filename y su contenido
	pack(buffer, (int)filename.size());
	packv(buffer, (unsigned char*) filename.data(), (int) filename.size());
	//Empaquetamos el contenido
	pack(buffer, (int)data.size());
	packv(buffer, (unsigned char*)data.data(), (int) data.size());

	//Eviamos la informacion
	sendMSG(conn.serverId, buffer);
	//Limpiamos buffer
	buffer.clear();
	recvMSG(conn.serverId, buffer);
	/** 
	auto conn = GestorClientes::connections[this];
    vector<unsigned char> buffer;
    // Empaquetar el tipo de mensaje
    pack(buffer, WriteFile);

    // Empaquetar el tamaño del nombre del archivo
    int filenameSize = filename.size();
    pack(buffer, filenameSize);

    // Empaquetar el nombre del archivo
    packv(buffer, (unsigned char*)filename.data(), filenameSize);

    // Empaquetar el tamaño de los datos del archivo
    int dataSize = data.size();
    pack(buffer, dataSize);

    // Empaquetar los datos del archivo
    packv(buffer, data.data(), dataSize);

    // Enviar los datos al servidor
    sendMSG(conn.serverId, buffer);

    // Esperar la respuesta del servidor
    buffer.clear();
    recvMSG(conn.serverId, buffer);
**/
	if (buffer.size() == 0) {
        std::cerr << "ERROR: " << __FILE__ << ":" << __LINE__
                  << " - Tamaño de mensaje inválido" << std::endl;
    } else {
        FileMSGTypes ack = unpack<FileMSGTypes>(buffer);
        if (ack != ackMSG) {
            std::cerr << "ERROR: " << __FILE__ << ":" << __LINE__
                      << " - Tipo de mensaje inválido" << std::endl;
        } else {
            // Operación exitosa
            std::cout << "Archivo " << filename << " enviado al servidor correctamente." << std::endl;
        }
    }

}

void FileManager::readFile(std::string filename, std::vector<unsigned char>& data) {

	auto conn = GestorClientes::connections[this];
    std::vector<unsigned char> buffer;

    // Empaquetar la solicitud de lectura
    pack(buffer, RedFile); // FileReadF es el identificador del comando 'readFile'
    pack(buffer, (int)filename.size());
    packv(buffer, filename.data(), (int)filename.size());

    // Enviar la solicitud al servidor
    sendMSG(conn.serverId, buffer);

    // Recibir la respuesta del servidor
    buffer.clear();
    recvMSG(conn.serverId, buffer);

	if (buffer.size() == 0) {
        std::cerr << "ERROR: " << __FILE__ << ":" << __LINE__ << " - Tamaño de mensaje inválido" << std::endl;
        return;
    }

    // Desempaquetar el mensaje de respuesta
    FileMSGTypes ack = unpack<FileMSGTypes>(buffer);
    if (ack != ackMSG) {
        std::cerr << "ERROR: " << __FILE__ << ":" << __LINE__ << " - Tipo de mensaje inválido" << std::endl;
        return;
    }

    // Desempaquetar el tamaño del archivo
    int fileSize = unpack<int>(buffer);

    // Leer los datos del archivo
    data.resize(fileSize);
    unpackv(buffer, (char*)data.data(), fileSize);

}



