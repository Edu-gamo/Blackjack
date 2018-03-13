#include <SFML\Network.hpp>
#include <iostream>
#include <list>

// Create a socket to listen to new connections
sf::TcpListener listener;
// Create a list to store the future clients
std::list<sf::TcpSocket*> clients;
// Create a selector
sf::SocketSelector selector;


//Variables para el control de mensajes
char buffer[2000];
std::size_t received;
#define MAX_MENSAJES_LENGTH 100

//Send a message to all clients
void sendToAll(std::string text) {
	for (sf::TcpSocket* client : clients) {
		client->send(text.c_str(), text.size());
	}
}

int main() {
	bool running = true;
	sf::Socket::Status status = listener.listen(5000);
	if (status != sf::Socket::Done) {
		std::cout << "Error al abrir listener\n";
		exit(0);
	}
	// Add the listener to the selector
	selector.add(listener);
	std::cout << "Esperando conexiones\n";
	// Endless loop that waits for new connections
	while (running) {
		// Make the selector wait for data on any socket
		if (selector.wait()) {
			// Test the listener
			if (selector.isReady(listener)) {
				// The listener is ready: there is a pending connection
				sf::TcpSocket* client = new sf::TcpSocket;
				if (listener.accept(*client) == sf::Socket::Done) {
					// Add the new client to the clients list
					std::cout << "Llega el cliente con puerto: " << client->getRemotePort() << std::endl;
					//if (clients.size() > 0) sendToAll("Se ha conectado un nuevo cliente");
					clients.push_back(client);
					// Add the new client to the selector so that we will
					// be notified when he sends something
					selector.add(*client);
				} else {
					// Error, we won't get a new connection, delete the socket
					std::cout << "Error al recoger conexion nueva\n";
					delete client;
				}
			} else {
				// The listener socket is not ready, test all other sockets (the clients)
				for (sf::TcpSocket* client : clients) {
					if (selector.isReady(*client)) {
						// The client has sent some data, we can receive it
						//sf::Packet packet;
						//status = client->receive(packet);
						status = client->receive(buffer, MAX_MENSAJES_LENGTH, received);
						if (status == sf::Socket::Done) {
							/*std::string strRec;
							packet >> strRec;*/
							std::cout << "He recibido " << buffer << " del puerto " << client->getRemotePort() << std::endl;
							sendToAll(buffer);
						} else if (status == sf::Socket::Disconnected) {
							selector.remove(*client);
							std::cout << "Elimino el socket que se ha desconectado\n";
							clients.remove(client);
							if (clients.size() > 0) sendToAll("Se ha desconectado un cliente");
						} else {
							std::cout << "Error al recibir de " << client->getRemotePort() << std::endl;
						}
						memset(buffer, 0, sizeof(buffer));
					}
				}
			}
		}
	}
}