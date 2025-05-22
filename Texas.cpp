#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <sstream>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <dirent.h>
#include <filesystem>
#include <limits>

bool readInt(int &value)
{
	std::string input;
	while (true)
	{
		std::cin >> input;
		try
		{
			value = std::stoi(input);
			return true;
		}
		catch (const std::invalid_argument &)
		{
			std::cerr << "Invalid input. Please enter a valid integer: ";
		}
		catch (const std::out_of_range &)
		{
			std::cerr << "Input out of range. Please enter a valid integer: ";
		}
	}
}

struct Player
{
	std::string name;
	int chips;
	bool active;
	bool disabled; // New field to indicate if the player is disabled
};

class PokerScorekeeper
{
public:
	PokerScorekeeper(const std::string &saveFile) : gameNumber(1), pot(0), saveFileName(saveFile) {}

	void newGame(int numPlayers, int baseBet)
	{
		pot = 0;
		players.resize(numPlayers);
		for (int i = 0; i < numPlayers; ++i)
		{
			std::cout << "Enter name and starting chips for player " << i + 1 << ": ";
			std::cin >> players[i].name;
			if (!readInt(players[i].chips))
			{
				std::cerr << "Invalid input. Please enter a valid integer for chips.\n";
				return;
			}
			players[i].active = true;
			players[i].disabled = false; // Initialize disabled status to false
		}
		this->baseBet = baseBet;
		chargeBaseBets();
		saveGame();
		printGameState(); // Print game state after creating a new game
	}

	void newGame()
	{
		for (auto &player : players)
		{
			player.active = true;
		}
		pot = 0;
		chargeBaseBets();
		saveGame();
		printGameState(); // Print game state after creating a new game
	}

	void raise(int amount)
	{
		storeState();
		for (auto &player : players)
		{
			if (player.active && !player.disabled && player.chips >= amount)
			{
				player.chips -= amount;
				pot += amount;
			}
		}
		saveGame();
	}

	void fold(const std::vector<int> &playerIndices)
	{
		storeState();
		for (int index : playerIndices)
		{
			if (index > 0 && index <= players.size())
			{
				players[index - 1].active = false;
			}
		}
		saveGame();
	}

	void show()
	{
		printGameState();
	}

	void adjustChips(const std::vector<int> &playerIndices, int amount, bool add)
	{
		storeState();
		for (int index : playerIndices)
		{
			if (index > 0 && index <= players.size())
			{
				players[index - 1].chips += add ? amount : -amount;
			}
		}
		saveGame();
	}

	void adjustPot(int amount, bool add)
	{
		storeState();
		pot += add ? amount : -amount;
		saveGame();
	}

	void win(const std::vector<int> &playerIndices)
	{
		storeState();
		int share = pot / playerIndices.size();
		for (int index : playerIndices)
		{
			if (index > 0 && index <= players.size())
			{
				players[index - 1].chips += share;
			}
		}
		pot = 0;
		gameNumber++;	  // Increase game number only when a player wins
		printGameState(); // Print game state after a player wins
		newGame();		  // Start a new game immediately after someone wins
	}

	void disable(const std::vector<int> &playerIndices)
	{
		storeState();
		for (int index : playerIndices)
		{
			if (index > 0 && index <= players.size())
			{
				players[index - 1].disabled = true;
			}
		}
		saveGame();
	}

	void enable(const std::vector<int> &playerIndices)
	{
		storeState();
		for (int index : playerIndices)
		{
			if (index > 0 && index <= players.size())
			{
				players[index - 1].disabled = false;
			}
		}
		saveGame();
	}

	void undo()
	{
		if (!undoStack.empty())
		{
			redoStack.push(currentState());
			restoreState(undoStack.top());
			undoStack.pop();
			saveGame();
		}
	}

	void redo()
	{
		if (!redoStack.empty())
		{
			undoStack.push(currentState());
			restoreState(redoStack.top());
			redoStack.pop();
			saveGame();
		}
	}

	void setBaseBet(int bet)
	{
		baseBet = bet;
		saveGame();
	}

	std::vector<int> parsePlayerIndices(const std::string &input)
	{
		return calc_test(input);
	}
	struct GameState
	{
		int gameNumber;
		int pot;
		std::vector<Player> players;
	};

	int gameNumber;
	int pot;
	int baseBet;
	std::vector<Player> players;
	std::stack<GameState> undoStack;
	std::stack<GameState> redoStack;
	std::string saveFileName;

	void printGameState()
	{
		std::cout << "Game #" << gameNumber << ":\n";
		for (const auto &player : players)
		{
			std::cout << "Player " << &player - &players.front() + 1 << " " << player.name << " : " << player.chips;
			if (!player.active)
			{
				std::cout << " (Folded)";
			}
			else if (player.disabled)
			{
				std::cout << " (Disabled)";
			}
			std::cout << "\n";
		}
		std::cout << "Pot: " << pot << "\n";
	}

	void storeState()
	{
		redoStack = {};
		undoStack.push(currentState());
	}

	GameState currentState()
	{
		return {gameNumber, pot, players};
	}

	void restoreState(GameState state)
	{
		gameNumber = state.gameNumber;
		pot = state.pot;
		players = state.players;
	}

	std::vector<int> calc_test(std::string s)
	{
		std::vector<int> r;
		s = "," + s;
		for (int i = 0, j; i < static_cast<int>(s.size()); i = j + 1)
		{
			for (j = i + 1; j + 1 < static_cast<int>(s.size()) && std::isdigit(s[j + 1]); j++)
				;
			if ((s[i] != ',' && s[i] != '-') || (i + 1 >= static_cast<int>(s.size())) || (!std::isdigit(s[i + 1])))
			{
				std::printf("Format error.\n");
				return {};
			}
			if (s[i] == ',')
			{
				r.push_back(std::stoi(s.substr(i + 1, j - i)));
			}
			else
			{
				int y = std::stoi(s.substr(i + 1, j - i));
				int x = r.back();
				for (int k = x + 1; k <= y; k++)
				{
					r.push_back(k);
				}
			}
		}
		return r;
	}

	void saveGame()
	{
		if (saveFileName.empty())
		{
			std::cout << "Please enter a filename to save the game (with .texas extension): ";
			std::cin >> saveFileName;
			if (saveFileName.find(".texas") == std::string::npos)
			{
				saveFileName += ".texas";
			}
		}

		std::ofstream outFile(saveFileName, std::ios::out | std::ios::trunc); // Open in write mode to overwrite existing content
		if (!outFile.is_open())
		{
			std::cerr << "Failed to open file for saving.\n";
			return;
		}
		outFile << gameNumber << "\n"
				<< pot << "\n"
				<< baseBet << "\n";
		outFile << players.size() << "\n";
		for (const auto &player : players)
		{
			outFile << player.name << " " << player.chips << " " << player.active << " " << player.disabled << "\n";
		}
		outFile.close();
	}

	bool loadGame()
	{
		std::ifstream inFile(saveFileName);
		if (!inFile.is_open())
		{
			return false;
		}
		inFile >> gameNumber >> pot >> baseBet;
		int numPlayers;
		inFile >> numPlayers;
		players.resize(numPlayers);
		for (auto &player : players)
		{
			inFile >> player.name >> player.chips >> player.active >> player.disabled;
		}
		inFile.close();
		return true;
	}

	void chargeBaseBets()
	{
		for (auto &player : players)
		{
			if (player.active && !player.disabled)
			{
				player.chips -= baseBet;
				pot += baseBet;
			}
		}
	}
};

void listSaveFiles(std::vector<std::string> &saveFiles)
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(".")) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			std::string fileName(ent->d_name);
			if (fileName.find(".texas") != std::string::npos)
			{
				saveFiles.push_back(fileName);
			}
		}
		closedir(dir);
	}
	else
	{
		perror("");
	}
}

int main()
{
	std::cout << "Welcome to Texas!\n";

	std::vector<std::string> saveFiles;
	listSaveFiles(saveFiles);

	for (size_t i = 0; i < saveFiles.size(); ++i)
	{
		std::cout << "[" << i + 1 << "] Load Game: " << saveFiles[i] << "\n";
	}

	std::cout << "[m] Manual configuration\n";

	std::string choice;
	std::cin >> choice;

	PokerScorekeeper scorekeeper("");

	if (choice == "m" || choice == "M")
	{
		int numPlayers;
		std::cout << "Enter number of players: ";
		if (!readInt(numPlayers))
		{
			std::cerr << "Invalid input. Please enter a valid integer for the number of players.\n";
			return 1;
		}

		int baseBet;
		std::cout << "Enter base bet per player: ";
		if (!readInt(baseBet))
		{
			std::cerr << "Invalid input. Please enter a valid integer for the base bet.\n";
			return 1;
		}

		scorekeeper.newGame(numPlayers, baseBet);
	}
	else
	{
		try
		{
			size_t index = std::stoul(choice) - 1;
			if (index < saveFiles.size())
			{
				scorekeeper.saveFileName = saveFiles[index];
				if (scorekeeper.loadGame())
				{
					scorekeeper.printGameState(); // Print game state after loading
				}
				else
				{
					std::cout << "Failed to load game. Starting a new game manually.\n";
					int numPlayers;
					std::cout << "Enter number of players: ";
					if (!readInt(numPlayers))
					{
						std::cerr << "Invalid input. Please enter a valid integer for the number of players.\n";
						return 1;
					}

					int baseBet;
					std::cout << "Enter base bet per player: ";
					if (!readInt(baseBet))
					{
						std::cerr << "Invalid input. Please enter a valid integer for the base bet.\n";
						return 1;
					}

					scorekeeper.newGame(numPlayers, baseBet);
				}
			}
			else
			{
				std::cout << "Invalid choice. Starting a new game manually.\n";
				int numPlayers;
				std::cout << "Enter number of players: ";
				if (!readInt(numPlayers))
				{
					std::cerr << "Invalid input. Please enter a valid integer for the number of players.\n";
					return 1;
				}

				int baseBet;
				std::cout << "Enter base bet per player: ";
				if (!readInt(baseBet))
				{
					std::cerr << "Invalid input. Please enter a valid integer for the base bet.\n";
					return 1;
				}

				scorekeeper.newGame(numPlayers, baseBet);
			}
		}
		catch (...)
		{
			std::cout << "Invalid choice. Starting a new game manually.\n";
			int numPlayers;
			std::cout << "Enter number of players: ";
			if (!readInt(numPlayers))
			{
				std::cerr << "Invalid input. Please enter a valid integer for the number of players.\n";
				return 1;
			}

			int baseBet;
			std::cout << "Enter base bet per player: ";
			if (!readInt(baseBet))
			{
				std::cerr << "Invalid input. Please enter a valid integer for the base bet.\n";
				return 1;
			}

			scorekeeper.newGame(numPlayers, baseBet);
		}
	}

	std::string command;
	while (true)
	{
		std::cout << "> ";
		for (std::getline(std::cin, command); command == ""; std::getline(std::cin, command))
			;
		if (command == "/exit" || command == "/e")
			break;
		else if (command.substr(0, 1) == "r")
		{
			std::istringstream iss(command.substr(2));
			int amount;
			if (!(iss >> amount))
			{
				std::cerr << "Invalid input. Please provide an integer amount to raise.\n";
				continue;
			}
			scorekeeper.raise(amount);
		}
		else if (command.substr(0, 1) == "f")
		{
			std::string playerIndices = command.substr(2);
			scorekeeper.fold(scorekeeper.parsePlayerIndices(playerIndices));
		}
		else if (command == "show" || command == "s")
		{
			scorekeeper.show();
		}
		else if (command.substr(0, 5) == "/plus" || command.substr(0, 2) == "/p")
		{
			std::istringstream iss(command.substr(command[1] == 'l' ? 6 : 3));
			std::string playerIndices;
			int amount;
			if (!(iss >> playerIndices >> amount))
			{
				std::cerr << "Invalid input. Please provide player indices and an integer amount.\n";
				continue;
			}
			scorekeeper.adjustChips(scorekeeper.parsePlayerIndices(playerIndices), amount, true);
		}
		else if (command.substr(0, 7) == "/minus" || command.substr(0, 3) == "/m")
		{
			std::istringstream iss(command.substr(command[2] == 'i' ? 8 : 4));
			std::string playerIndices;
			int amount;
			if (!(iss >> playerIndices >> amount))
			{
				std::cerr << "Invalid input. Please provide player indices and an integer amount.\n";
				continue;
			}
			scorekeeper.adjustChips(scorekeeper.parsePlayerIndices(playerIndices), amount, false);
		}
		else if (command.substr(0, 6) == "/plusj" || command.substr(0, 3) == "/pj")
		{
			std::istringstream iss(command.substr(command[2] == 'u' ? 7 : 4));
			int amount;
			if (!(iss >> amount))
			{
				std::cerr << "Invalid input. Please provide an integer amount to add to the pot.\n";
				continue;
			}
			scorekeeper.adjustPot(amount, true);
		}
		else if (command.substr(0, 8) == "/minusj" || command.substr(0, 4) == "/mj")
		{
			std::istringstream iss(command.substr(command[3] == 'i' ? 9 : 5));
			int amount;
			if (!(iss >> amount))
			{
				std::cerr << "Invalid input. Please provide an integer amount to subtract from the pot.\n";
				continue;
			}
			scorekeeper.adjustPot(amount, false);
		}
		else if (command.substr(0, 1) == "w")
		{
			std::string playerIndices = command.substr(2);
			scorekeeper.win(scorekeeper.parsePlayerIndices(playerIndices));
		}
		else if (command.substr(0, 7) == "disable")
		{
			std::string playerIndices = command.substr(8);
			scorekeeper.disable(scorekeeper.parsePlayerIndices(playerIndices));
		}
		else if (command.substr(0, 6) == "enable")
		{
			std::string playerIndices = command.substr(7);
			scorekeeper.enable(scorekeeper.parsePlayerIndices(playerIndices));
		}
		else if (command == "/undo")
		{
			scorekeeper.undo();
		}
		else if (command == "/redo")
		{
			scorekeeper.redo();
		}
		else
		{
			std::cout << "Unknown command.\n";
		}
	}

	return 0;
}
