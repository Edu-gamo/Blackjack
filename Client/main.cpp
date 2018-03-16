#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_MENSAJES 25

sf::IpAddress ip = sf::IpAddress::getLocalAddress();
//sf::TcpSocket socket;
sf::Socket::Status status;

std::vector<std::string> aMensajes;

std::mutex mut;

sf::Packet packetOut;

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
	std::vector<Card> hand;

	sf::TcpSocket sock;

};
Player::Player() {

}
Player::~Player() {

}

Player me;

enum Commands {
	JoinTable_, ExitTable_, DecideEntryMoney_, EntryMoney_, PlaceBetOrder_, PlaceBet_, GiveInitialCards_, IncorrectBet_, StartPlayerTurn_, AskForCard_, NomoreCards_, DoubleBet_, EndRound_, ChatMSG_
};

void receiveText(std::string text) {
	aMensajes.push_back(text);
	if (aMensajes.size() > 25) {
		aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
	}
}

void thread_function() {
	sf::Packet packetIn;
	std::string strRec;
	int intRec;
	int enumVar;
	Commands com;
	do {
		status = me.sock.receive(packetIn);
		packetIn >> enumVar;
		com = (Commands)enumVar;
		switch (com) {
			case DecideEntryMoney_:
				std::cout << "Introduce el dinero inicial de la mesa: ";
				std::cin >> me.money;

				packetOut << Commands::EntryMoney_ << me.money;
				me.sock.send(packetOut);
				break;
			case PlaceBetOrder_:
				std::cout << "Introduce tu apuesta: ";
				std::cin >> me.bet;
				
				packetOut << Commands::PlaceBet_ << me.bet;
				me.sock.send(packetOut);
				break;
			case IncorrectBet_:
				std::cout << "Error en tu apuesta, dinero insuficiente\nIntroduce tu apuesta: ";
				std::cin >> me.bet;

				packetOut << Commands::PlaceBet_ << me.bet;
				me.sock.send(packetOut);
				break;
			case GiveInitialCards_:
				break;
			case StartPlayerTurn_:
				break;
			case EndRound_:
				break;
			case ChatMSG_:
				packetIn >> strRec;
				receiveText(strRec);
				break;
			default:
				break;
		}
		packetIn.clear();
		packetOut.clear();
	} while (status == sf::Socket::Done);
	if (status == sf::Socket::Disconnected) receiveText("Se ha perdido la conexion con el servidor");
}

int main() {

	std::cout << "Nombre de usuario: ";
	std::cin >> me.name;

	status = me.sock.connect(ip, 5000, sf::seconds(5.f));

	if (status == sf::Socket::Done) {
		std::cout << "Conectado al Servidor " << ip << "\n";
		packetOut << Commands::JoinTable_ << me.name;
		me.sock.send(packetOut);
		packetOut.clear();
	}
	else {
		std::cout << "Fallo al Conectar con el Servidor " << ip << "\n";
		system("pause");
		exit(0);
	}

	//*************************************************************************//

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

	std::thread t(&thread_function); //Thread start

	while (window.isOpen()) {

		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return) {

					//SEND
					mensaje.erase(0, 1);
					if (mensaje == "exit") {
						window.close();
						continue;
					}
					mensaje = me.name + ": " + mensaje;

					packetOut << Commands::ChatMSG_ << mensaje.toAnsiString().c_str();
					me.sock.send(packetOut);
					packetOut.clear();

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
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
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
	packetOut << Commands::ExitTable_;
	me.sock.send(packetOut);

	t.join(); //Thread end

	me.sock.disconnect();
	return 0;
}