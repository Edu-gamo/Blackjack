#include <SFML\Network.hpp>
#include <iostream>
#include <list>
#include <vector>

#define Blackjack 21;
#define CasinoStop 17;

// Create a socket to listen to new connections
sf::TcpListener listener;
// Create a list to store the future clients
//std::list<sf::TcpSocket*> clients;
// Create a selector
sf::SocketSelector selector;

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

	sf::TcpSocket* sock;

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
				} else {
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
		} else {
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

enum Commands {
	JoinTable_, ExitTable_, DecideEntryMoney_, EntryMoney_, PlaceBetOrder_, PlaceBet_, GiveInitialCards_, IncorrectBet_, StartPlayerTurn_, AskForCard_, NomoreCards_, DoubleBet_, ChatMSG_
};

std::vector<Card> deck;
Player crupier;
std::vector<Player> players;
std::vector<std::vector<Player>::iterator> toRemove;
std::vector<Player>::iterator playerTurn;
int initMoney;

sf::Packet packetIn;
sf::Packet packetOut;

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

//Send a message to all clients
void sendToAll(std::string text) {
	sf::Packet packet;
	packet << Commands::ChatMSG_ << text;
	/*for (sf::TcpSocket* client : clients) {
		client->send(packet);
	}*/
	for (Player player : players) {
		player.sock->send(packet);
	}
	packet.clear();
}

Card giveRandomCard() {
	int pos = rand() % (deck.size() + 1);
	Card card = deck.at(pos);
	deck.erase(deck.begin() + pos);
	return card;
}

void checkWinner() {
	for (std::vector<Player>::iterator it = players.begin(); it != players.end(); it++) {
		int win = 0;
		if (it->lose) {
			it->money -= it->bet;
			win = -1;
		} else if (it->blackjack) {
			if (!crupier.blackjack) {
				it->money += it->bet * 2;
				win = 1;
			}
		} else {
			if (crupier.lose) {
				it->money += it->bet;
				win = 1;
			} else {
				if (it->score > crupier.score) {
					it->money += it->bet;
					win = 1;
				} else if (it->score < crupier.score) {
					it->money -= it->bet;
					win = -1;
				}
			}
		}
		switch (win) {
		case 0:
			sendToAll(it->name + " ha empatado");
			break;
		case 1:
			sendToAll(it->name + " ha ganado " + std::to_string(it->bet));
			break;
		case -1:
			sendToAll(it->name + " ha perdido " + std::to_string(it->bet));
			break;
		default:
			break;
		}
		it->reset();
		if (it->money == 0) {
			sendToAll(it->name + " se ha quedado sin fichas");
			toRemove.push_back(it);
		} else {
			sendToAll(it->name + " tiene " + std::to_string(it->money) + " fichas");
		}
	}
	crupier.reset();
}

void crupierTurn() {
	sendToAll("Turno del crupier");
	sendToAll(crupier.showCards() + " con " + crupier.showScore());
	while (crupier.score < 17) {
		crupier.hand.push_back(giveRandomCard());
		crupier.calculateScore();
		sendToAll(crupier.showCards() + " con " + crupier.showScore());
		if (crupier.score == 21 && crupier.hand.size() == 2) {
			crupier.blackjack = true;
			sendToAll("BLACKJACK de " + crupier.name);
		}
	}
	if (crupier.score > 21) crupier.lose = true;
	checkWinner();
	createDeck();
	packetOut << Commands::PlaceBetOrder_;
	for (int i = 0; i < players.size(); i++) {
		players[i].sock->send(packetOut);
	}
}

int main() {

	srand(time(NULL));
	createDeck();
	crupier.name = "Crupier";

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
				//sf::TcpSocket* client = new sf::TcpSocket;
				Player newPlayer;
				newPlayer.sock = new sf::TcpSocket;
				if (listener.accept(*newPlayer.sock) == sf::Socket::Done) {
					// Add the new client to the clients list
					std::cout << "Llega el cliente con puerto: " << newPlayer.sock->getRemotePort() << std::endl;
					//if (clients.size() > 0) sendToAll("Se ha conectado un nuevo cliente");
					//clients.push_back(client);
					players.push_back(newPlayer);
					// Add the new client to the selector so that we will
					// be notified when he sends something
					selector.add(*newPlayer.sock);
				} else {
					// Error, we won't get a new connection, delete the socket
					std::cout << "Error al recoger conexion nueva\n";
					delete newPlayer.sock;
				}
			} else {
				// The listener socket is not ready, test all other sockets (the clients)
				for (std::vector<Player>::iterator it = players.begin(); it != players.end(); it++) {
					if (selector.isReady(*it->sock)) {
						// The client has sent some data, we can receive it
						status = it->sock->receive(packetIn);
						if (status == sf::Socket::Done) {
							std::string strRec;
							int intRec;
							int enumVar;
							Commands com;
							packetIn >> enumVar;
							com = (Commands)enumVar;
							switch (com) {
								case JoinTable_:
									//Si es el primer jugador se le pide la cantidad inicial de dinero de la mesa
									if (players.size() == 1) {
										packetOut << Commands::DecideEntryMoney_;
										it->sock->send(packetOut);
										packetOut.clear();
									}
									packetIn >> strRec;
									it->name = strRec;
									sendToAll(it->name + " se ha conectado");

									//Al llegar a 4 jugadores empieza la partida
									if (players.size() >= 4) {
										packetOut << Commands::PlaceBetOrder_;
										for (int i = 0; i < players.size(); i++) {
											players[i].money = initMoney;
											players[i].sock->send(packetOut);
										}
										sendToAll("Fichas iniciales: " + std::to_string(initMoney));
									}
									break;
								case ChatMSG_:
									packetIn >> strRec;
									sendToAll(strRec);
									break;
								case ExitTable_:
									sendToAll(it->name + " se ha desconectado");
									break;
								case EntryMoney_:
									packetIn >> intRec;
									initMoney = intRec;
									break;
								case PlaceBet_:
								{
									packetIn >> it->bet;
									if (it->bet > it->money) {
										packetOut << Commands::IncorrectBet_;
										it->sock->send(packetOut);
										packetOut.clear();
										sendToAll("El jugador " + it->name + " no tiene ni idea");
									}
									else {
										sendToAll("El jugador " + it->name + " ha apostado: " + std::to_string(it->bet));
									}
									bool allBetOk = true;
									for each(Player player in players) {
										if (player.bet == 0) allBetOk = false;
									}
									if (allBetOk) {
										for (int i = 0; i < players.size(); i++) {
											players[i].hand.push_back(giveRandomCard());
											players[i].hand.push_back(giveRandomCard());
											players[i].calculateScore();
											sendToAll(players[i].showCards() + " con " + players[i].showScore());
										}
										crupier.hand.push_back(giveRandomCard());
										crupier.calculateScore();
										sendToAll(crupier.showCards() + " con " + crupier.showScore());

										playerTurn = players.begin();
										bool lastPlayer = false;
										while (playerTurn->score == 21 && !lastPlayer) {
											if (playerTurn == players.end() - 1) lastPlayer = true;
											sendToAll("BLACKJACK de " + playerTurn->name);
											playerTurn->blackjack = true;
											if(!lastPlayer) playerTurn++;
										}
										if (playerTurn->score != 21) {
											bool canDouble = false;
											if ((playerTurn->score >= 9 && playerTurn->score <= 11) && playerTurn->hand.size() == 2) canDouble = true;
											packetOut << Commands::StartPlayerTurn_ << canDouble;
											playerTurn->sock->send(packetOut);
											sendToAll("");
											sendToAll("Turno del jugador: " + playerTurn->name);
											sendToAll(playerTurn->showCards() + " con " + playerTurn->showScore());
										} else {
											crupierTurn();
										}
									}
								}
									break;
								case AskForCard_:
								{
									playerTurn->hand.push_back(giveRandomCard());
									playerTurn->calculateScore();
									sendToAll(playerTurn->showCards() + " con " + playerTurn->showScore());
									bool canDouble = false;
									if (playerTurn->score >= 21) {
										playerTurn->lose = true;
										if (playerTurn != players.end() - 1) {
											playerTurn++;
											sendToAll("");
											sendToAll("Turno del jugador: " + playerTurn->name);
											sendToAll(playerTurn->showCards() + " con " + playerTurn->showScore());
											if ((playerTurn->score >= 9 && playerTurn->score <= 11) && playerTurn->hand.size() == 2) canDouble = true;
											packetOut << Commands::StartPlayerTurn_ << canDouble;
											playerTurn->sock->send(packetOut);
										} else {
											crupierTurn();
										}
									} else {
										packetOut << Commands::StartPlayerTurn_ << canDouble;
										playerTurn->sock->send(packetOut);
									}
								}
									break;
								case NomoreCards_:
									if (playerTurn != players.end() - 1) {
										playerTurn++;
										sendToAll("");
										sendToAll("Turno del jugador: " + playerTurn->name);
										sendToAll(playerTurn->showCards() + " con " + playerTurn->showScore());
										bool canDouble = false;
										if ((playerTurn->score >= 9 && playerTurn->score <= 11) && playerTurn->hand.size() == 2) canDouble = true;
										packetOut << Commands::StartPlayerTurn_ << canDouble;
										playerTurn->sock->send(packetOut);
									} else {
										crupierTurn();
									}
									break;
								case DoubleBet_:
									it->bet *= 2;
									it->hand.push_back(giveRandomCard());
									it->calculateScore();
									sendToAll(it->showCards() + " con " + it->showScore());
									if (playerTurn != players.end() - 1) {
										playerTurn++;
										sendToAll("");
										sendToAll("Turno del jugador: " + playerTurn->name);
										sendToAll(playerTurn->showCards() + " con " + playerTurn->showScore());
										bool canDouble = false;
										if ((playerTurn->score >= 9 && playerTurn->score <= 11) && playerTurn->hand.size() == 2) canDouble = true;
										packetOut << Commands::StartPlayerTurn_ << canDouble;
										playerTurn->sock->send(packetOut);
									} else {
										crupierTurn();
									}
									break;
								default:
									break;
							}
							packetOut.clear();
						} else if (status == sf::Socket::Disconnected) {
							sendToAll(it->name + " se ha desconectado");
							selector.remove(*it->sock);
							toRemove.push_back(it);
							if (it == playerTurn && playerTurn != players.end() - 1) {
								playerTurn++;
								sendToAll("");
								sendToAll("Turno del jugador: " + playerTurn->name);
								sendToAll(playerTurn->showCards() + " con " + playerTurn->showScore());
								packetOut << Commands::StartPlayerTurn_ << false;
								playerTurn->sock->send(packetOut);
							} else {
								crupierTurn();
							}
							std::cout << "Elimino el socket que se ha desconectado\n";
						} else {
							std::cout << "Error al recibir de " << it->sock->getRemotePort() << std::endl;
						}
						packetIn.clear();
					}
				}
				if (!toRemove.empty()) {
					for (int i = 0; i < toRemove.size(); i++) {
						players.erase(toRemove[i]);
					}
					toRemove.clear();
					if (players.empty()) crupier.hand.clear();
				}
			}
		}
	}
}