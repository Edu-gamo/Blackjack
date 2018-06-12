#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

sf::IpAddress ip = sf::IpAddress::getLocalAddress();
sf::Socket::Status status;

std::mutex mut;

std::vector<std::string> aMensajes;

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
	int id;
	int money = 100;
	int bet = 0;
	int score;
	std::vector<Card> hand;
	bool blackjack = false;
	bool lose = false;
	bool isCrupier = false;
	int turn = 0;

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

void receiveText(std::string text) {
	aMensajes.push_back(text);
	if (aMensajes.size() > 25) {
		aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
	}
}

void thread_function(int playerIndex) {
	mut.lock();
	std::cout << me.name << ":" << me.id << " --> " << players[playerIndex]->name << ":" << players[playerIndex]->id << std::endl;
	mut.unlock();
	//sf::Packet packetIn2;
	//std::string strRec;
	//int intRec;
	//int enumVar;
	//Commands com;
	//do {
	//	status = players[playerIndex]->sock.receive(packetIn2);
	//	packetIn2 >> enumVar;
	//	com = (Commands)enumVar;
	//	switch (com) {
	//	/*case DecideEntryMoney_:
	//		std::cout << "Introduce el dinero inicial de la mesa: ";
	//		std::cin >> me.money;

	//		packetOut << Commands::EntryMoney_ << me.money;
	//		me.sock.send(packetOut);
	//		break;*/
	//	case PlaceBetOrder_:
	//		do {
	//			std::cout << "Introduce tu apuesta: ";
	//			std::cin >> me.bet;
	//		} while (me.bet > me.money || me.bet < 0);

	//		packetOut << Commands::PlaceBet_ << me.bet;
	//		me.sock.send(packetOut);
	//		break;
	//	/*case IncorrectBet_:
	//		std::cout << "Error en tu apuesta, dinero insuficiente\nIntroduce tu apuesta: ";
	//		std::cin >> me.bet;

	//		packetOut << Commands::PlaceBet_ << me.bet;
	//		me.sock.send(packetOut);
	//		break;*/
	//	case GiveInitialCards_:
	//		break;
	//	case StartPlayerTurn_:
	//	{
	//		bool canDouble;
	//		packetIn >> canDouble;
	//		std::cout << "Introduce 1)Pedir carta   2)Plantarse" << (canDouble ? "   3)Doblar apuesta: " : ": ");
	//		std::cin >> intRec;

	//		switch (intRec) {
	//		case 1:
	//			packetOut << Commands::AskForCard_;
	//			break;
	//		case 2:
	//			packetOut << Commands::NomoreCards_;
	//			break;
	//		case 3:
	//			if (canDouble) {
	//				packetOut << Commands::DoubleBet_;
	//			}
	//			break;
	//		default:
	//			break;
	//		}
	//		me.sock.send(packetOut);
	//	}
	//	break;
	//	case ChatMSG_:
	//		packetIn2 >> strRec;
	//		receiveText(strRec);
	//		break;
	//	default:
	//		break;
	//	}
	//	packetIn2.clear();
	//	packetOut.clear();
	//} while (status == sf::Socket::Done);
	//if (status == sf::Socket::Disconnected) receiveText("Se ha perdido la conexion con el servidor");
}

//Send a message to all clients
void sendToAll(sf::Packet packet) {
	for (Player* player : players) {
		player->sock.send(packet);
	}
}

std::vector<Card> deck;

void createDeck() {
	deck.clear();
	for (int i = 0; i < 4; i++) {
		for (int j = 1; j <= 13; j++) {
			Card newCard;
			newCard.suit = (Card::Suits)i;
			newCard.number = j;
			deck.push_back(newCard);
		}
	}
}

Card giveRandomCard() {
	int pos = rand() % (deck.size() + 1);
	Card card = deck.at(pos);
	deck.erase(deck.begin() + pos);
	return card;
}

void main() {

	std::cout << "Nombre de usuario: ";
	std::cin >> me.name;

	status = me.sock.connect(ip, 5000, sf::seconds(5.f));

	if (status == sf::Socket::Done) {
		std::cout << "Conectado al Servidor " << ip << "\n";
		packetOut << me.name;
		me.ip = sf::IpAddress::getLocalAddress();
		me.port = me.sock.getLocalPort();
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
			me.isCrupier = true;
			me.id = 0;

			break;
		case NoEmpty: {
			int size;
			packetIn >> size;
			me.id = size;

			std::cout << "Total de jugadores anteriores " << size << "\n";

			for (int i = 0; i < size; i++) {
				Player* newPlayer = new Player;
				packetIn >> newPlayer->name;
				packetIn >> newPlayer->id;
				std::string newIp;
				packetIn >> newIp;
				newPlayer->ip = sf::IpAddress(newIp);
				packetIn >> newPlayer->port;

				newPlayer->sock.connect(newPlayer->ip, newPlayer->port, sf::seconds(5.f));

				players.push_back(newPlayer);

				packetOut << me.name << me.id << me.ip.toString() << me.port;
				newPlayer->sock.send(packetOut);
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

		Player* newPlayer = new Player;

		listener.listen(port);
		listener.accept(newPlayer->sock);

		newPlayer->sock.receive(packetIn);
		packetIn >> newPlayer->name;
		packetIn >> newPlayer->id;
		std::string newIp;
		packetIn >> newIp;
		std::cout << newIp << std::endl;
		newPlayer->ip = sf::IpAddress(newIp);
		packetIn >> newPlayer->port;
		packetIn.clear();
		std::cout << "Se ha conectado " << newPlayer->name << " : " << newPlayer->ip << " : " << newPlayer->port << std::endl;

		players.push_back(newPlayer);

	}

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), ("Chat (" + me.name + ")"));

	sf::Font font;
	if (!font.loadFromFile("comicSans.ttf")) {
		std::cout << "Can't load the font file" << std::endl;
	}

	sf::String mensaje = ">";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);


	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	std::vector<std::thread> threads;
	for (int val = 0; val < players.size(); val++) {
		threads.push_back(std::thread(&thread_function, val));
	}

	while (window.isOpen()) {

		sf::Event evento;
		while (window.pollEvent(evento)) {
			switch (evento.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape) window.close();
				else if (evento.key.code == sf::Keyboard::Return) {

					//SEND
					mensaje.erase(0, 1);
					if (mensaje == "exit") {
						window.close();
						continue;
					}
					mensaje = me.name + ": " + mensaje;

					packetOut << Commands::ChatMSG_ << mensaje.toAnsiString().c_str();
					sendToAll(packetOut);
					packetOut.clear();
					receiveText(mensaje.toAnsiString().c_str());

					//SEND END

					mensaje = ">";
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 1)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}
		window.draw(separator);
		for (size_t i = 0; i < aMensajes.size(); i++) {
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);

		window.draw(text);


		window.display();
		window.clear();

	}

	for (int val = 0; val < threads.size(); val++) {
		threads[val].join();
	}
	threads.clear();

}