#include "utils.h"
#include "gestorClientes.h"
#include <iostream>
#include <string>
#include <thread>
#include <list>

enum BrokerMSGTypes {
    BrokerRegisterServer,
    BrokerRequestServer,
    BrokerAck,
    BrokerError
};

void registerBroker(const std::string& serverIp){
	//ip broker
	std::string brokerIp = "3.93.250.107";
	//Iniciamos conexion con el broker
	auto conn = initClient(brokerIp, 5000);
    std::vector<unsigned char> buffer;

	//empaquetamos la informacion
    pack(buffer, BrokerRegisterServer); // Tipo de mensaje
    pack(buffer, (int)serverIp.size()); // Tamaño de la IP
    packv(buffer, (char*)serverIp.data(), (int)serverIp.size()); // La IP del servidor

	// Enviar mensaje al Broker
    sendMSG(conn.serverId, buffer);

    // Recibir confirmación del Broker
    buffer.clear();
    recvMSG(conn.serverId, buffer);

	if (buffer.empty()) {
        std::cerr << "ERROR: No se recibió respuesta del Broker" << std::endl;
        closeConnection(conn.serverId);
        return;
    }

	BrokerMSGTypes msgType = unpack<BrokerMSGTypes>(buffer);
	
    if (msgType != BrokerAck) {
        std::cerr << "ERROR: El Broker no pudo registrar el servidor" << std::endl;
    } else {
        std::cout << "Servidor registrado exitosamente en el Broker" << std::endl;
    }

    // Cerrar conexión con el Broker
    closeConnection(conn.serverId);


}

int main(int argc, char** argv)
{
	//std::string ipServer;
	//std::cin>>ipServer;

    std::string ipServer = "52.87.132.254";
    std::getline(std::cin, ipServer);

    // Verificar si se leyó correctamente
    if (ipServer.empty()) {
        std::cerr << "ERROR: No se pudo leer la IP desde la entrada estándar." << std::endl;
        return 1;
    }

    // Mostrar la IP leída para confirmar
    std::cout << "IP recibida: " << ipServer << std::endl;
	
    registerBroker(ipServer);

	//iniciar server, puerto 5000
	int serverSocketID=initServer(5000);
	std::vector<unsigned char> buffer;
	//bucle de gestión de conexiones
	while(1){
		//esperar conexion
		while(!checkClient()) usleep(100);
		std::cout<<"Cliente conectado\n";
		//recuperar identificador de conexión
		int clientId=getLastClientID();
		//crear hilo de gestión de clientes
		std::thread *th=new std::thread( GestorClientes::atiendeCliente,clientId);
	}
	//cerrar
	close(serverSocketID);
    return 0; 
}


