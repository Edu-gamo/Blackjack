#include <SFML\Network.hpp>
#include <iostream>
#include <vector>

sf::IpAddress ip = sf::IpAddress::getLocalAddress();
sf::Socket::Status status;

enum Commands {
	Empty, NoEmpty, JoinTable_, ExitTable_, DecideEntryMoney_, EntryMoney_, PlaceBetOrder_, PlaceBet_, GiveInitialCards_, IncorrectBet_, StartPlayerTurn_, AskForCard_, NomoreCards_, DoubleBet_, ChatMSG_
};

class Card {
public:
	Card();
	~Card();

	enum Suits { Diamond, Heart, Spade, Club };

	int number;
	Suits suit;

};
Card::Card() {

}
Card::~Card() {

}

class Player {
public:
	Player();
	~Player();

	std::string name;
	int money;
	int bet = 0;
	int score;
	std::vector<Card> hand;
	bool blackjack = false;
	bool lose = false;

	sf::TcpSocket sock;
	sf::IpAddress ip;
	unsigned short port;

	std::string showCards();
	void calculateScore();
	std::string showScore();
	void reset();

};
Player::Player() {

}
Player::~Player() {

}
std::string Player::showCards() {
	std::string text = "El jugador " + name + " tiene:";
	for each (Card card in hand) {
		std::string num;
		switch (card.number) {
		case 1:
			num = "A";
			break;
		case 11:
			num = "J";
			break;
		case 12:
			num = "Q";
			break;
		case 13:
			num = "K";
			break;
		default:
			num = std::to_string(card.number);
			break;
		}
		switch (card.suit) {
		case 0:
			num += " de Diamantes";
			break;
		case 1:
			num += " de Corazones";
			break;
		case 2:
			num += " de Picas";
			break;
		case 3:
			num += " de Treboles";
			break;
		default:
			break;
		}
		text += " " + num;
	}
	return text;
}
void Player::calculateScore() {
	int s = 0;
	bool firstAs = true;
	for each (Card card in hand) {
		switch (card.number) {
		case 1:
			if (firstAs) {
				firstAs = false;
			}
			else {
				s += card.number;
			}
			break;
		case 11:
		case 12:
		case 13:
			s += 10;
			break;
		default:
			s += card.number;
			break;
		}
	}
	if (!firstAs) {
		if (s + 11 <= 21) {
			s += 11;
		}
		else {
			s += 1;
		}
	}
	score = s;
}
std::string Player::showScore() {
	return "Score: " + std::to_string(score);
}

void Player::reset() {
	bet = 0;
	score = 0;
	hand.clear();
	blackjack = false;
	lose = false;
}

Player me;
std::vector<Player*> players;

sf::Packet packetOut, packetIn;

void main() {

	std::cout << "Nombre de usuario: ";
	std::cin >> me.name;

	status = me.sock.connect(ip, 5000, sf::seconds(5.f));

	if (status == sf::Socket::Done) {
		std::cout << "Conectado al Servidor " << ip << "\n";
		packetOut << me.name;
		me.sock.send(packetOut);
		packetOut.clear();
	} else {
		std::cout << "Fallo al Conectar con el Servidor " << ip << "\n";
		system("pause");
		exit(0);
	}

	me.sock.receive(packetIn);

	int com;
	Commands command;
	packetIn >> com;
	command = (Commands)com;

	switch (command) {
		case Empty:
			std::cout << "Primer jugador\n";

			break;
		case NoEmpty: {
			int size;
			packetIn >> size;

			std::cout << "Total de jugadores anteriores " << size << "\n";

			for (int i = 0; i < size; i++) {
				Player* newPlayer = new Player;
				packetIn >> newPlayer->name;
				std::string newIp;
				packetIn >> newIp;
				newPlayer->ip = sf::IpAddress(newIp);
				packetIn >> newPlayer->port;

				players.push_back(newPlayer);

				sf::TcpSocket* socket = new sf::TcpSocket;
				socket->connect(newPlayer->ip, newPlayer->port, sf::seconds(5.f));

				packetOut << me.name;
				socket->send(packetOut);
				packetOut.clear();

				std::cout << "Jugador: " << newPlayer->name << "\n";
			}
		}
			break;
		default:
			break;
	}

	int port = me.sock.getLocalPort();
	me.sock.disconnect();

	while (players.size() < 3) {

		sf::TcpListener listener;
		sf::TcpSocket* sock = new sf::TcpSocket;

		Player* newPlayer = new Player;

		listener.listen(port);
		listener.accept(*sock);

		sock->receive(packetIn);
		packetIn >> newPlayer->name;
		std::string newIp;
		packetIn >> newIp;
		newPlayer->ip = sf::IpAddress(newIp);
		packetIn >> newPlayer->port;
		packetIn.clear();
		std::cout << "Se ha conectado " << newPlayer->name << " : " << newPlayer->ip << " : " << newPlayer->port << std::endl;

		players.push_back(newPlayer);

	}

}