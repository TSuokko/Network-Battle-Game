// UDP_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#pragma comment (lib, "ws2_32.lib")

#define IP_ADDRESS "127.0.0.1"
#define PORT "2315"
#define DEFAULT_BUFLEN 512
const char OPT_VAL = 1;
int num_players = 0;
const int MAX_PLAYERS = 6;


enum Type {
	GRASS, 
	FIRE,
	WATER, 
	NORMAL
};

struct Attack {

	std::string Name;
	Type AttackType;
	int Damage;
	int HitChance;

};

struct Blokemon {

	int HP;
	std::string Name;
	Type BlokemonType;
	Type Weakness;
	Type Resistance;
	std::vector<Attack> Attacks;

}tempB, Blurbasaur, Blarmander, Blurtle;

struct PlayerClient {
	int id;
	SOCKET socket;
	std::string PlayerName;
	bool CreatureChosen;
	bool nameChanged;
	bool choosingOpponent;
	bool fighting;
	int fightingPlayerWithID;
	int MonsterID;
	Blokemon ChosenBlokemon;
};





//USE FOR ALL PLAYERS
void sendMessageToPlayers(std::string ResponseMessage, std::string MessageToOthers, int playerID, std::vector<PlayerClient> &player_array, int iResult) {
	
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (player_array[i].socket != INVALID_SOCKET)
			{
				if (playerID == i) {
					iResult = send(player_array[i].socket, ResponseMessage.c_str(), strlen(ResponseMessage.c_str()), 0);
				}
				if (playerID != i) {
					iResult = send(player_array[i].socket, MessageToOthers.c_str(), strlen(MessageToOthers.c_str()), 0);
				}
			}
		}
	
}

//USE FOR TWO SPECIFIC PLAYERS
void sendMessageToPlayer(std::string MessageToYou, std::string MessageToTheOtherPlayer, int MainPlayerID, int otherPlayerID, std::vector<PlayerClient> &player_array, int iResult) {

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (player_array[i].socket != INVALID_SOCKET)
		{
			if (MainPlayerID == i) {
				iResult = send(player_array[i].socket, MessageToYou.c_str(), strlen(MessageToYou.c_str()), 0);
			}
			if (otherPlayerID == i) {
				iResult = send(player_array[i].socket, MessageToTheOtherPlayer.c_str(), strlen(MessageToTheOtherPlayer.c_str()), 0);
			}
		}
	}
}

int processDamage(PlayerClient &new_player, PlayerClient &opponent, Attack attack, std::vector<PlayerClient> &player_array, int iResult) {

	//calculate hit chance
	//no hit = 0 dmg
	//first take attacks damage
	//calculate weakness / resistance
	//deal damage to opponent

	srand(time(NULL));
	unsigned int HitChance = rand() % 100 + 1;

	std::cout << "Hit Chance:	"<<HitChance << std::endl;

	if (HitChance <= attack.HitChance)
	{
		if(attack.AttackType == opponent.ChosenBlokemon.Weakness)
		{	//massive damage
			sendMessageToPlayer("\nIt's super effective!!", "\nIt's super effective!!", new_player.id, opponent.id, player_array, iResult);
			return attack.Damage * 2;
		}
		if (attack.AttackType == opponent.ChosenBlokemon.Resistance)
		{	//resisting damage
			sendMessageToPlayer("\nIt's not very effective...", "\nIt's not very effective...", new_player.id, opponent.id, player_array, iResult);
			return attack.Damage / 2;
		}
		else {
			//normal damage
			sendMessageToPlayer("\nThe attack hit!", "The attack hit!", new_player.id, opponent.id, player_array, iResult);
			return attack.Damage;
		}
	}
	else {
		//attack missed
		sendMessageToPlayer("\nThe attack missed!", "The attack missed!", new_player.id, opponent.id, player_array, iResult);
		return 0;
	}

}

//MAIN INPUT SCREEN
int processBattle(PlayerClient &new_player, std::vector<PlayerClient> &player_array, int fightingID, int iResult) {

	sendMessageToPlayers(
	    "\n---------------------------------\n"
		"| Choose your attack or action! |"
        "\n---------------------------------\n"
		"| " + new_player.ChosenBlokemon.Attacks[0].Name +"			|\n"
		"| " + new_player.ChosenBlokemon.Attacks[1].Name + "				|\n"
		"| Run Away			|\n"
		"---------------------------------\n", "", new_player.id, player_array, iResult);

	char attackMessage[DEFAULT_BUFLEN] = "";
	std::string OtherMessage = "";

	while (true)
	{
		memset(attackMessage, 0, DEFAULT_BUFLEN);
		if (new_player.socket != 0)
		{
			iResult = recv(new_player.socket, attackMessage, DEFAULT_BUFLEN, 0);
			if (iResult != SOCKET_ERROR) {

				if (strcmp("", attackMessage))
				{
					//OtherMessage = new_player.PlayerName + " #0" + std::to_string(new_player.id) + ": " + attackMessage;
				}
				if (!strcmp(new_player.ChosenBlokemon.Attacks[0].Name.c_str(), attackMessage)) {
					//first attack chosen
					sendMessageToPlayers(new_player.PlayerName +": " + new_player.ChosenBlokemon.Name + ", use " + new_player.ChosenBlokemon.Attacks[0].Name.c_str(),
						new_player.PlayerName + ": " + new_player.ChosenBlokemon.Name + ", use " + new_player.ChosenBlokemon.Attacks[0].Name.c_str(), 
						new_player.id, player_array, iResult);

					player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP -= 
						processDamage(new_player, player_array[new_player.fightingPlayerWithID], 
							new_player.ChosenBlokemon.Attacks[0], player_array, iResult);

					sendMessageToPlayers(player_array[new_player.fightingPlayerWithID].ChosenBlokemon.Name + " \nHP: " + std::to_string(player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP),
						player_array[new_player.fightingPlayerWithID].ChosenBlokemon.Name + " \nHP: " + std::to_string(player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP),
						new_player.id, player_array, iResult);
					//sendMessageToPlayer("It did ");

					if (player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP <= 0)
					{
						//TODO(Mechanic): Win game lol
						sendMessageToPlayers("\n " + player_array[new_player.fightingPlayerWithID].ChosenBlokemon.Name + " fainted!\n" + new_player.PlayerName + " has won the battle!",
							"\n"+player_array[new_player.fightingPlayerWithID].ChosenBlokemon.Name + " fainted!\n" + new_player.PlayerName + " has won the battle!\n\n", new_player.id, player_array, iResult);
						new_player.fighting = false;
						player_array[new_player.fightingPlayerWithID].fighting = false;
						new_player.ChosenBlokemon.HP = 150;
						player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP = 150;
						break;
					}



				}
				if (!strcmp(new_player.ChosenBlokemon.Attacks[1].Name.c_str(), attackMessage)) {
					//second attack chosen
					sendMessageToPlayers(new_player.PlayerName + ": " + new_player.ChosenBlokemon.Name + ", use " + new_player.ChosenBlokemon.Attacks[1].Name.c_str(),
						new_player.PlayerName + ": " + new_player.ChosenBlokemon.Name + ", use " + new_player.ChosenBlokemon.Attacks[1].Name.c_str(), 
						new_player.id, player_array, iResult);

					player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP -= 
						processDamage(new_player, player_array[new_player.fightingPlayerWithID], 
							new_player.ChosenBlokemon.Attacks[1], player_array, iResult);

					sendMessageToPlayers(player_array[new_player.fightingPlayerWithID].ChosenBlokemon.Name + " \nHP: " + std::to_string(player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP),
						player_array[new_player.fightingPlayerWithID].ChosenBlokemon.Name + " \nHP: " + std::to_string(player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP),
						new_player.id, player_array, iResult);

					if (player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP <= 0)
					{
						//TODO(Mechanic): Win game lol
						sendMessageToPlayers("\n "+player_array[new_player.fightingPlayerWithID].ChosenBlokemon.Name + " fainted!\n" + new_player.PlayerName + " has won the battle!",
							"\n" + player_array[new_player.fightingPlayerWithID].ChosenBlokemon.Name + " fainted!\n" + new_player.PlayerName + " has won the battle!\n\nYou have lost the battle. Type 'OK' to return to main menu",
							new_player.id, player_array, iResult);

						new_player.fighting = false;
						player_array[new_player.fightingPlayerWithID].fighting = false;
						new_player.ChosenBlokemon.HP = 150;
						player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP = 150;
						break;
					}


				}
				if (!strcmp("Run Away", attackMessage) || !strcmp("run away", attackMessage) || !strcmp("Run", attackMessage) || !strcmp("run", attackMessage)) {
					//player runs away from battle. 
					sendMessageToPlayer("\nSERVER:You ran away! The Battle has ended!", "\nSERVER:Your opponent ran away from battle!",
						new_player.id, new_player.fightingPlayerWithID, player_array, iResult);

					new_player.fighting = false;
					player_array[new_player.fightingPlayerWithID].fighting = false;
					new_player.ChosenBlokemon.HP = 150;
					player_array[new_player.fightingPlayerWithID].ChosenBlokemon.HP = 150;

					break;

				}
				
				//Broadcast chat message to the other players that are not commands
				else {
					sendMessageToPlayers("", OtherMessage, new_player.id, player_array, iResult);
				}
			}
			//disconnection
			else {
				break;
			}
		}
		if (new_player.ChosenBlokemon.HP <= 0) {
			//sendMessageToPlayers("You have lost the battle. Type 'OK' to return to main menu", "",new_player.id, player_array, iResult);
			break;
		}
		if (player_array[new_player.fightingPlayerWithID].fighting = false){
			sendMessageToPlayer("\n\nSERVER: The battle has ended. Type 'OK' to return to main menu", "", new_player.id, new_player.fightingPlayerWithID, player_array, iResult);
			break;
		}
	}

	return 0;
}


int process_PlayerClient(PlayerClient &new_player, std::vector<PlayerClient> &player_array, std::thread &thread) {

	std::string message = "";
	std::string response = "";
	std::string messageToOtherPlayers = "";
	std::string Blokemon = "nothing yet!";
	char tempMessage[DEFAULT_BUFLEN] = "";
	
	while (true) {

		memset(tempMessage, 0, DEFAULT_BUFLEN);

		if (new_player.socket != 0)
		{
			//recieve message from client
			int iResult = recv(new_player.socket, tempMessage, DEFAULT_BUFLEN, 0);
			if (iResult != SOCKET_ERROR) {
				//if the player client writes anything besides the available commands
				if (strcmp("", tempMessage))
				{
					message = new_player.PlayerName +" #0" + std::to_string(new_player.id) + ": " + tempMessage;
				}
				//if the player has written "name" and wants to choose themselves another name
				if (new_player.nameChanged == true) {
					if (strcmp("", tempMessage))
					{
						//broadcast the name change, and change the players name
						message = "SERVER: " + new_player.PlayerName + " #0" + std::to_string(new_player.id) + " has changed their name into: " + tempMessage;
						new_player.PlayerName = tempMessage;
						new_player.nameChanged = false;	
					}
				}

				std::cout << message << std::endl;

				//if a player wirtes "choose"
				if (!strcmp("choose", tempMessage)) {

					for (int i = 0; i < MAX_PLAYERS; i++)
					{
						//send the response message to the choosing player
						if (player_array[i].socket != INVALID_SOCKET) {	
							if (new_player.id == i) {
								player_array[i].CreatureChosen = true;
								response = "SERVER: Which Blokemon would you like?\n(0)Blurbasaur, (1)Blarmander, (2)Blurtle\n"; //this message is for the player who typed choose
								iResult = send(player_array[i].socket, response.c_str(), strlen(response.c_str()), 0);
							}
							//send the "messageToOtherPlayers" message to the rest
							if (new_player.id != i) {
								messageToOtherPlayers = "SERVER: " + new_player.PlayerName + " #" + std::to_string(new_player.id) + " is choosing a Blokemon!"; //this is for everyone else
								iResult = send(player_array[i].socket, messageToOtherPlayers.c_str(), strlen(messageToOtherPlayers.c_str()), 0);
							}
						}
					}
				}

				//PLAYER CHOICE BETWEEN BLOKEMON OR OPPONENT

				//else if(!strcmp("0", tempMessage)|| !strcmp("1", tempMessage)|| !strcmp("2", tempMessage)) {
				 else if ((*tempMessage) >= '0' && (*tempMessage) <= '2') {
					//if the player is choosing an opponent
					if (new_player.choosingOpponent == true) {
						//check which opponent the player wants

						new_player.fightingPlayerWithID = (*tempMessage) - 48;

						//if the player chooses his own id
						if (new_player.id == new_player.fightingPlayerWithID)
						{
							sendMessageToPlayers("SERVER: You cannot fight yourself!", "", new_player.id, player_array, iResult);
							new_player.fightingPlayerWithID = -1;

						}
					
						//otherwise, check if the opponent has a blokemon, and set both players .fighting boolean to true.
						else {
							if (new_player.fightingPlayerWithID > num_players) {
								sendMessageToPlayers("SERVER: That player does not exist", "", new_player.id, player_array, iResult);
							}
							else if (player_array[new_player.fightingPlayerWithID].fighting == true) {
								sendMessageToPlayers("\nSERVER: That player is already fighting!", "", new_player.id, player_array, iResult);

							}
							else {
								if (player_array[new_player.fightingPlayerWithID].MonsterID > -1)
								{
									sendMessageToPlayer("\n\nSERVER: You have chosen player ID #0" + std::to_string(new_player.fightingPlayerWithID) + " as your opponent!\n\n",
										"\n\nSERVER: Player ID #0" + std::to_string(new_player.id) + " has chosen you as his opponent!\n",
										new_player.id, new_player.fightingPlayerWithID, player_array, iResult);

									player_array[new_player.fightingPlayerWithID].fightingPlayerWithID = new_player.id;

									new_player.fighting = true;
									player_array[new_player.fightingPlayerWithID].fighting = true;
									new_player.choosingOpponent = false;

								}
								if (player_array[new_player.fightingPlayerWithID].MonsterID == -1) {
									sendMessageToPlayers("SERVER: You cannot fight a player with no Blokemon!", "", new_player.id, player_array, iResult);
									//set fightingID to default -1
									new_player.fightingPlayerWithID = -1;
								}
							}
						}
					}
					else 
					{	
						//information loop
/*choose*/				for (int i = 0; i < MAX_PLAYERS; i++)
						{
							if (player_array[i].socket != INVALID_SOCKET) {
								//if the typing player has not typed choose 	
								if (player_array[i].CreatureChosen == false && new_player.id == i)
								{
									//he is prompted to type choose first
									response = "SERVER: Please type choose first!";
									new_player.MonsterID = -1;
									iResult = send(player_array[i].socket, response.c_str(), strlen(response.c_str()), 0);
									//the message below is sent to other players
									for (int k = 0; k < MAX_PLAYERS; k++)
									{							//player tried to steal a blokemon!
										messageToOtherPlayers = "SERVER: " + new_player.PlayerName + " #" + std::to_string(new_player.id) + " tried to steal a Blokemon !\n";
										iResult = send(player_array[k].socket, messageToOtherPlayers.c_str(), strlen(messageToOtherPlayers.c_str()), 0);
									}
								}
								//if the typing player has typed choose before
								if (player_array[i].CreatureChosen == true && new_player.id == i)
								{
									//the player chooses a blokemon based on its id
									switch (*tempMessage) { //player is assigned a blokemon id
										case '0': {
											Blokemon = "Blurbasaur";
											new_player.MonsterID = 0;

											Blurbasaur.HP = 140;
											Blurbasaur.BlokemonType = GRASS;
											Blurbasaur.Name = "Blurbasaur";
											Blurbasaur.Weakness = FIRE;
											Blurbasaur.Resistance = WATER;
											std::vector<Attack> GrassAttacks = { { "Solar Beam", GRASS, 55, 80 },{ "Slam", NORMAL, 30, 90 } };
											Blurbasaur.Attacks = GrassAttacks;

											new_player.ChosenBlokemon = Blurbasaur;
											break; 
										}
										case '1': {
											Blokemon = "Blarmander"; 
											new_player.MonsterID = 1; 

											Blarmander.HP = 120;
											Blarmander.BlokemonType = FIRE;
											Blarmander.Name = "Blarmander";
											Blarmander.Weakness = WATER;
											Blarmander.Resistance = GRASS;
											std::vector<Attack> FireAttacks = { { "Flamethrower", FIRE, 50, 85 },{ "Slash", NORMAL, 35, 85 } };
											Blarmander.Attacks = FireAttacks;

											new_player.ChosenBlokemon = Blarmander; 
											break;
										}
										case '2': {
											Blokemon = "Blurtle"; 
											new_player.MonsterID = 2; 

											Blurtle.HP = 160;
											Blurtle.BlokemonType = WATER;
											Blurtle.Name = "Blurtle";
											Blurtle.Weakness = GRASS;
											Blurtle.Resistance = FIRE;
											std::vector<Attack> WaterAttacks = { { "Hydro Pump", WATER, 45, 90 },{ "Bite", NORMAL, 40, 80 } };
											Blurtle.Attacks = WaterAttacks;

											new_player.ChosenBlokemon = Blurtle; 
											break;
										}
									}
									player_array[i].CreatureChosen = false;
									//send message to all players what number the typing player chose
									for (int j = 0; j < MAX_PLAYERS; j++)
									{
										messageToOtherPlayers = "SERVER: " + Blokemon + " was chosen by " + new_player.PlayerName + " #" + std::to_string(new_player.id);
										iResult = send(player_array[j].socket, messageToOtherPlayers.c_str(), strlen(messageToOtherPlayers.c_str()), 0);
									}
								}
							}
						}
					}	
				}


/*attack*/		else if (!strcmp("attack", tempMessage)) {
					if (new_player.fighting != true) {
						sendMessageToPlayers("SERVER: You have not successfully challenged anyone to a fight!", "", new_player.id, player_array, iResult);
					}
					else {
						sendMessageToPlayer("SERVER: Which attack would you like to use?", "Your opponent is preparing an attack!", new_player.id, new_player.fightingPlayerWithID, player_array, iResult);
						//if the players have agreed to fight, go to process the battle
						processBattle(new_player, player_array, new_player.fightingPlayerWithID, iResult);

						sendMessageToPlayers("SERVER: Welcome back from the battle!", "", new_player.id, player_array, iResult);
					}

				}

/*blokemon*/	else if (!strcmp("blokemon", tempMessage)) {
					sendMessageToPlayers("SERVER: You have chosen " + new_player.ChosenBlokemon.Name, "", new_player.id, player_array, iResult);
					sendMessageToPlayers("\nHP:" + std::to_string(new_player.ChosenBlokemon.HP) + 
										 "\n Attacks: " + new_player.ChosenBlokemon.Attacks[0].Name + " and " + new_player.ChosenBlokemon.Attacks[1].Name,
						"", new_player.id, player_array, iResult);
				}

/*commands*/	else if (!strcmp("commands", tempMessage)) {
					sendMessageToPlayers("\n-----------------\n" 
										 "|choose		|\n"
										 "|blokemon	|\n"
										 "|fight		|\n"
										 "|name		|\n"
										 "|attack		|\n"
										 "-----------------\n","", new_player.id, player_array, iResult);
				}

/*fight*/		else if (!strcmp("fight", tempMessage)) {
					if (new_player.MonsterID > -1) {
						new_player.choosingOpponent = true;
						sendMessageToPlayers("SERVER: Which player would you like to fight against?\n", new_player.PlayerName + " is picking a fight!", new_player.id, player_array, iResult);
					}
					else {
						sendMessageToPlayers("SERVER: Please choose a Blokemon first!\n", new_player.PlayerName + " tried to fight without a Blokemon!\n", new_player.id, player_array, iResult);
					}
				}

/*name*/		else if (!strcmp("name", tempMessage)) {
					sendMessageToPlayers("\nSERVER: What would you like to change your name to?\n", "\nSERVER: "+ new_player.PlayerName + " #0" +std::to_string(new_player.id)+" is changing his name!\n", new_player.id, player_array, iResult);
					new_player.nameChanged = true;
				}
				//Broadcast chat message to the other players that are not commands
				else{
					sendMessageToPlayers("", message, new_player.id, player_array, iResult);
				}
			}


			//disconnection message
			else {		
				message ="SERVER: " + new_player.PlayerName + " #0" + std::to_string(new_player.id) + " Disconnected";
				//what the server sees
				std::cout << message << std::endl;
				//close disconnected players socket
				closesocket(new_player.socket);
				closesocket(player_array[new_player.id].socket);
				player_array[new_player.id].socket = INVALID_SOCKET;
				//Broadcast the disconnection message to the other players
				for (int i = 0; i < MAX_PLAYERS; i++)
				{
					if (player_array[i].socket != INVALID_SOCKET)
						iResult = send(player_array[i].socket, message.c_str(), strlen(message.c_str()), 0);
				}
				break;
			}
		}
	} 

	thread.detach();

	return 0;

}


int main() {

	WSADATA wsaData;
	addrinfo hints;
	addrinfo *server = NULL;
	SOCKET serverSocket = INVALID_SOCKET;
	std::string playerCommand = "";
	std::vector<PlayerClient> player(MAX_PLAYERS);
	
	int temp_ID = -1;
	bool tempChoice = false;
	std::string tempName = "Player";
	std::thread m_thread[MAX_PLAYERS];

	//temporary blokemon struct
	std::vector<Attack> tempAttacks = { { "temp Attack 1", NORMAL, 0, 0 },{ "temp Attack 2", NORMAL, 0, 0 } };
	tempB.HP = 0;
	tempB.Attacks = tempAttacks;
	tempB.BlokemonType = NORMAL;
	tempB.Name = "temp";
	tempB.Weakness = NORMAL;
	tempB.Resistance = NORMAL;


	//Init WINSOCK
	std::cout << "Intializing Winsock..." << std::endl;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//Setup hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//SERVER SETUP

	std::cout << "Setting up the server.\n";
	getaddrinfo(IP_ADDRESS, PORT, &hints, &server);

	//Create a listening socket for connecting to server
	std::cout << "Creating server socket.\n";
	serverSocket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	//Setup socket options
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &OPT_VAL, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
	setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, &OPT_VAL, sizeof(int)); //Used for interactive programs

	//Assign an address to the server socket.
	std::cout << "Binding socket\n";
	bind(serverSocket, server->ai_addr, (int)server->ai_addrlen);

	//Listen for incoming connections.
	std::cout << "Listening..." << std::endl;
	listen(serverSocket, SOMAXCONN);

	//Initialize the client list
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		player[i] = { -1, INVALID_SOCKET };
	}

	while (true) {

		SOCKET incoming = INVALID_SOCKET;
		incoming = accept(serverSocket, NULL, NULL);

		if (incoming == INVALID_SOCKET)
			continue;

		//reset number of players
		num_players = -1;
		//Create a temporary id for the next player
		temp_ID = -1;

		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (player[i].socket == INVALID_SOCKET && temp_ID == -1)
			{
				//(temporary) statistics added to new player
				player[i].socket = incoming;
				player[i].id = i;
				player[i].PlayerName = tempName;

				player[i].CreatureChosen = tempChoice;
				player[i].nameChanged = tempChoice;
				player[i].choosingOpponent = tempChoice;
				player[i].fighting = tempChoice;

				player[i].MonsterID = -1;
				player[i].fightingPlayerWithID = -1;
				player[i].ChosenBlokemon = tempB;
				temp_ID = i;
			}
			if (player[i].socket != INVALID_SOCKET)
				num_players++;
		}

		if (temp_ID != -1)
		{
			//Send the id to that player
			std::cout << player[temp_ID].PlayerName + " #0" << player[temp_ID].id << " has connected!" << std::endl;
			playerCommand = std::to_string(player[temp_ID].id);
			send(player[temp_ID].socket, playerCommand.c_str(), strlen(playerCommand.c_str()), 0);

			//Create a thread process for that player
			m_thread[temp_ID] = std::thread(process_PlayerClient, std::ref(player[temp_ID]), std::ref(player), std::ref(m_thread[temp_ID]));
		}
		else
		{
			playerCommand = "Server is full";
			send(incoming, playerCommand.c_str(), strlen(playerCommand.c_str()), 0);
			std::cout << playerCommand << std::endl;
		}
	}

	//Close listening socket
	closesocket(serverSocket);

	//Close client socket
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		m_thread[i].detach();
		closesocket(player[i].socket);
	}

	//Clean up Winsock
	WSACleanup();
	std::cout << "Game has ended" << std::endl;

	system("pause");
	return 0;
}
