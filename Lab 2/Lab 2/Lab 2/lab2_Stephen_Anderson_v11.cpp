/*
Stephen Anderson
CSE 512 - Winter 2020
Professor Salloum
Lab #2
Description:	

Lab specs:

1.  The game board is an N x N grid. Columns are named A, B, C, .... (from left to right), and rows are named 1, 2, 3, ... (from top to bottom).
2.  Each player takes turns as in chess or tic-tac-toe. That is, player X takes a move, then player O, then back to player X, and so forth. 
3.  Each square has a fixed point value between 1 and 99.
4.  The objective of the game for each player is to score the most points. A player’s score is the sum of the point values of his or her occupied squares minus the sum of all point values of the squares occupied by the other player. Thus, one wants to capture the squares worth the most points while preventing the other player from doing so.
5.  The game ends when all the squares are occupied by the players since no more moves are left. 
6.  Players cannot pass their move, i.e., they must make a valid move if one exists.
7.  Movement and adjacency relations are always vertical and horizontal but never diagonal. 
8.  The values of the squares can be changed for different games, but remain constant within a game. 
9.  Game score is computed as the difference between (a) the sum of the values of all squares occupied by your player and (b) the sum of the values of all squares occupied by the other player
10. On each turn, a player can make one of two moves: Stake or Raid

Stake – You can take any unoccupied square on the board. This will create a new piece in that square. 
This move can be made as many times as one wants to during the game, but only once per turn. 
However, a Stake cannot conquer enemy pieces. Once you have done a Stake, your turn is complete.

Raid – From any square you occupy on the board, you can take the one next to it (up, down, left, right, but not diagonally) if it is unoccupied. 
Thus, you get to create a new piece in the raided square. 
In addition, any enemy adjacent to the raided squareis conquered (that is, you turn its piece to your side). 
If there are no enemies adjacent to the raided square, then no enemy pieces will be conquered. 
Once you have done a Raid, your turn is complete.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>

using namespace std;

struct input_data
{
	int size; // the size of the gameboard in any one dimension. actually game board is size x size
	string mode; // minimax or alpha beta pruning? AB Pruning is extra credit
	string player; // which side is the player
	int depth; // how deep into the graph do we search. Start is depth 0
	vector<string> cell_values;
	vector<string> board_state;
};

struct space
{
	bool owned; // if 1, the space is controlled by one of the players
	string owner; // if the space is owned, this string stores the name of the owner
	int value; // how many points a space is worth
	int i;
	int j;
};

struct player
{
	string name;
	vector<space> spaces;
	int score;
};

struct adj_list 
{
	vector< vector <space> > lists; // adjacency list representation for the graph. lists is a vector of vectors of spaces, which we use as our states.
};

input_data get_input(fstream &in)
{
	input_data input;
	string line;
	getline(in, line);
	for (int n = 0; !in.eof(); ++n)
	{
		if (n == 0) // if this is the first line of the input, we get the size
		{
			input.size = stoi(line);
			input.cell_values.resize(input.size);
			input.board_state.resize(input.size);
		}
		else if (n == 1) // this is where we get MODE
		{
			input.mode = line;
		}
		else if (n == 2) // where YOUPLAY is stored
		{
			input.player = line;
		}
		else if (n == 3) // where DEPTH is stored
		{
			input.depth = stoi(line);
		}
		else if (n == 4)
		{
			for (int i = 0; i < input.size; ++i)
			{
				input.cell_values[i] = line;
				getline(in, line);
			}
			for (int j = 0; j < input.size; ++j)
			{
				input.board_state[j] = line;
				getline(in, line);
			}
		}
		getline(in, line);
	}
	return input;
};

vector< vector<space> > generate_board(input_data input)
{
	vector<int> space_values;
	string line;
	space_values.resize(input.size);
	vector<space> col;
	vector< vector<space> > board;
	board.resize(input.size);
	vector< vector<int> > cvals;
	cvals.resize(input.size);
	int index;

	for (int i = 0; i < input.size; ++i) // iterates through our vector<string> cell_values
	{
		board[i].resize(input.size);
		vector<int> values;
		values.resize(input.size);
		string cell_line = input.cell_values[i];
		string own_line = input.board_state[i];
		for (int j = 0; j < input.size; ++j) // iterates through the individual cell values themselves
		{
			if (own_line[j] != '.') // sets ownership of the board spaces
			{
				board[i][j].owner = own_line[j];
				board[i][j].owned = true;
			}
			else
			{
				board[i][j].owner = own_line[j];
				board[i][j].owned = false;
			}

			if (j < input.size - 1) // sets values of the board spaces
			{
				index = cell_line.find_first_of(' '); // find index of first space
				values[j] = stoi(cell_line.substr(0, index)); // make a substring from 0 to index, make it an int, and put it in values.
				cell_line.erase(0, index + 1); // erase the bit we've already read in
			}
			else // this is the last loop, j = input.size - 1
			{
				values[j] = stoi(cell_line); // the only thing remaining in the string is the last integer
			}
		}
		cvals[i] = values; // stick our values into our cvals double vector
	}

	for (int i = 0; i < input.size; ++i)
	{
		for (int j = 0; j < input.size; ++j)
		{
			board[i][j].value = cvals[i][j];
			board[i][j].i = i;
			board[i][j].j = j;
		}
	}
	return board;
}

adj_list generate_graph(input_data input, vector< vector<space> > board)
{
	adj_list graph;
	graph.lists.resize(input.depth);
	for (int i = 0; i < input.depth; ++i)
	{
		graph.lists[i].resize(0);
		for (int j = 0; j < input.size; ++j)
		{
			for (int k = 0; k < input.size; ++k)
			{
				graph.lists[i].push_back(board[j][k]);
			}
		}
	}
	return graph;
}

vector<player> generate_players(input_data input, vector < vector <space> > board)
{
	vector<player> players;
	players.resize(2);
	players[0].name = input.player;
	if (input.player == "X")
		players[1].name = "O";
	else
		players[1].name = "X";

	players[0].score = 0;
	players[1].score = 0;

	for (int i = 0; i < input.size; ++i)
	{
		for (int j = 0; j < board[i].size(); ++j)
		{
			if (board[i][j].owned)
			{
				if (board[i][j].owner == players[0].name)
				{
					players[0].score += board[i][j].value;
					players[1].score -= board[i][j].value;
					players[0].spaces.push_back(board[i][j]);
				}
				else if (board[i][j].owner == players[1].name)
				{
					players[1].score += board[i][j].value;
					players[0].score -= board[i][j].value;
					players[1].spaces.push_back(board[i][j]);
				}
			}
		}
	}
}

space minimax_stake(input_data input, adj_list graph)
{
	vector<int> bestindex;
	bestindex.resize(input.depth);

	for (int i = 0; i < graph.lists.size; ++i)
	{
		if (i % 2 == 0) // max's turn
		{
			for (int j = 0; j < graph.lists[i].size(); ++j)
			{
				if (j == 0)
					continue;
				if (graph.lists[i][j].value > graph.lists[i][bestindex[i]].value)
				{
					bestindex[i] = j;
				}
			}
		}
		else // min's turn
		{
			for (int j = 0; j < graph.lists[i].size(); ++j)
			{
				if (j == 0)
					continue;
				if (graph.lists[i][j].value < graph.lists[i][bestindex[i]].value)
				{
					bestindex[i] = j;
				}
			}
		}
	}
}

void print_board(input_data input, vector< vector<space> > board)
{
	cout << endl << "Begin board value output: " << endl;
	for (int i = 0; i < input.size; ++i)
	{
		for (int j = 0; j < input.size; ++j)
		{
			cout << board[i][j].value << " ";
		}
		cout << endl;
	}
	cout << endl << "Begin board owner output: " << endl;
	for (int i = 0; i < input.size; ++i)
	{
		for (int j = 0; j < input.size; ++j)
		{
			cout << board[i][j].owner << " ";
		}
		cout << endl;
	}
}

void print_graph(input_data input, adj_list graph)
{
	cout << endl << "Begin graph value output: " << endl;
	for (int i = 0; i < input.depth; ++i)
	{
		for (int j = 0; j < graph.lists[i].size(); ++j)
		{
			cout << graph.lists[i][j].value << " ";
			if (j % input.size == 0 && j != 0)
			{
				cout << endl;
			}
		}
		cout << endl;
	}
}

int main(int argc, char **argv)
{
	fstream in("input.txt");
	fstream out;

	input_data input = get_input(in);
	vector< vector<space> > board = generate_board(input);
	adj_list graph = generate_graph(input, board);
	print_graph(input, graph);

	system("PAUSE");
	return 0;
}