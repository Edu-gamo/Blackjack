#include <SFML\Network.hpp>
#include <iostream>
#include <vector>

sf::TcpListener listener;
sf::TcpSocket socket;
sf::Socket::Status status;

enum Commands {
	Empty, NoEmpty
};

class Player {
public:
	Player();
	~Player();

	std::string name;
	std::string ip;
	unsigned short port;

private:

};

Player::Player() {
}

Player::~Player() {
}

std::vector<Player> players;

void main() {

	std::cout << "Servidor Bootstrap inicializado, esperando conexiones\n";

	status = listener.listen(5000);
	if (status != sf::Socket::Done) {
		std::cout << "Error al abrir listener\n";
		exit(0);
	}

	for (int i = 0; i < 4; i++) {

		status = listener.accept(socket);

		if (status == sf::Socket::Status::Done) {

			sf::Packet packetIn;
			Player newPlayer;

			status = socket.receive(packetIn);

			packetIn >> newPlayer.name;
			newPlayer.ip = socket.getRemoteAddress().toString();
			newPlayer.port = socket.getRemotePort();

			std::cout << "Se ha conectado " << newPlayer.name << " : " << newPlayer.ip << " : " << newPlayer.port << std::endl;

			sf::Packet packetOut;
			if (i == 0) {

				packetOut << Commands::Empty;

			} else {

				packetOut << Commands::NoEmpty << i;

				for (int i = 0; i < players.size(); i++) {
					packetOut << players[i].name << players[i].ip << players[i].port;
				}

			}

			socket.send(packetOut);

			packetOut.clear();
			packetIn.clear();

			players.push_back(newPlayer);

		}

	}

}