#include "utils.h" // Para funciones como initServer, recvMSG, sendMSG
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <thread>
#include <algorithm>
#include <unistd.h> // Para usleep

// Tipos de mensajes del Broker
enum BrokerMSGTypes {
    BrokerRegisterServer,
    BrokerRequestServer,
    BrokerAck,
    BrokerError
};


int main(){
    int brokerPort = 5000; // Puerto del Broker
    int brokerSocketID = initServer(brokerPort); // Iniciar servidor en el puerto especificado
    // Mapa para registrar servidores y sus conexiones
    std::map<std::string, int> servers; 
    // Mutex para acceso concurrente al mapa de servidores
    std::mutex serversMutex; 

    std::cout << "Broker iniciado en el puerto " << brokerPort << std::endl;

    while (true)
    {
        // Esperar a que un cliente o servidor se conecte
        while (!checkClient()) usleep(100);
        int clientId = getLastClientID();

        //Manejamos la conexion en un hilo idenpendiente y con el uso de detach lo sepramos del proceso principal
        std::thread([clientId, &servers, &serversMutex]{
            std::vector<unsigned char> buffer;
            recvMSG(clientId, buffer);

            if (buffer.empty()) {
                std::cerr << "ERROR: Mensaje vacío recibido en el Broker" << std::endl;
                closeConnection(clientId);
                return;
            }

            BrokerMSGTypes msgType = unpack<BrokerMSGTypes>(buffer);

            switch (msgType)
            {
            case BrokerRegisterServer:
            {
               // Registrar un servidor
                std::string serverIp;
                //redimencionamos la variable
                serverIp.resize(unpack<int>(buffer));
                //Desempaquemos la ip
                unpackv(buffer, (char*)serverIp.data(), (int)serverIp.size());

                //Encerramos en corchete el area donde usamos el mutex para delimitar el bloqueo
                {
                        std::lock_guard<std::mutex> lock(serversMutex);
                        servers[serverIp] = 0; // Registrar el servidor con 0 conexiones iniciales
                }

                buffer.clear();
                pack(buffer, BrokerAck);
                sendMSG(clientId, buffer);

                std::cout << "Servidor registrado: " << serverIp << std::endl;
                
                break;
            }
            case BrokerRequestServer:

            {
                //variable para guardar la direccion ip que usara el cliente
                std::string serverIp;
                

                {
                    std::lock_guard<std::mutex> lock(serversMutex);
                    //validamos de que existan servidores disponibles
                    if(servers.empty()){
                        buffer.clear();
                        pack(buffer, BrokerError);
                        std::cout << "Servidor no dispoble"<< std::endl;

                        //Cerramos conexion
                        sendMSG(clientId, buffer);
                        closeConnection(clientId);
                        return;
                    }

                    // Encontrar el servidor con menos conexiones
                    auto minServer = std::min_element(servers.begin(), servers.end(),
                        [](const auto& a, const auto& b) {
                            return a.second < b.second;
                         });
                    //recuperamos la primera posicion que seria nuestro servidor menor
                    serverIp = minServer->first;
                    //aumentamo en nuestro map la cantidad de conexiones de ese servidor
                    servers[serverIp]++;

                }

                //limpiamos buffer
                buffer.clear();
                //empaquetamos la informacion
                pack(buffer, BrokerAck);
                pack(buffer, (int) serverIp.size());
                packv(buffer, (char *)serverIp.data(), (int)serverIp.size());
                //enviar ip al cliente
                sendMSG(clientId, buffer);

                std::cout << "Cliente asignado al servidor: " << serverIp << std::endl;
                break;
            }
            default:
                {
			//default: mensaje error
				std::cout<<"ERROR: "<<__FILE__<<":"<<__LINE__<<
						"Tipo de mensaje inválido"<<std::endl;
			}break;
            }
        }).detach();


    }
    
    close(brokerSocketID);
    return 0;

}