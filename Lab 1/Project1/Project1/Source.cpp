/*
Stephen Anderson
CSE 512 - Winter 2020 
Professor Salloum
Lab #1
Description:	This program will find the shortest travel path from a given start to a given goal using four different search algorithms: BFS, DFS, UCS, and A*. 
				It will take a file, input.txt, which will provide the search algorithm to use, the starting and ending locations, live traffic information as a list of 
				current traveling times in minutes between different locations, and traffic information from a traffic free Sunday (hueristic).
				It will ouput its results into a file, output.txt, which will include the list of locations traveled over in the solution path 
				(including starting and ending locations), and the accumulated time from start to each location in order of travel.

Input.txt format: 
	<ALGO>
	<START>
	<GOAL>
	<NUMBER OF TRAFFIC LINES>
	<...LIVE TRAFFIC LINES...>
	<NUMBER OF SUNDAY LINES>
	<...SUNDAY LINES...>
	<EOF>
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>

using namespace std;

struct traffic_line 
{
	string state_a;
	string state_b;
	int time; // travel time from start to end in minutes
};

struct sunday_line
{
	string state; // current location / state
	int sun_time; // time to get from state to goal
};

struct output_line
{
	string state;
	int cum_time;
};

struct input_data
{
	int algorithm; // 1 = BFS, 2 = DFS, 3 = UCS, 4 = A*, -1 = invalid
	string start;
	string goal;
	string all_states; // a string that stores the names of all locations / states, it can be searched for any state present in the graph, and its length is the number of states in the graph
	int number_of_states;
	int number_of_lines; // number of traffic lines included in input.txt
	vector<traffic_line> live_traffic_lines;
	int number_of_sun_lines; // number of Sunday traffic lines in input.txt
	vector<sunday_line> sunday_traffic_lines;
	int max_time; // tracks the max distance listed in input.txt to check for int overflow when we generate our graph
};

// the below structs are used in our adjacency list representation of our graph. An adjacency list is represented using a vector of vectors of traffic lines.

struct state_list
{
	vector<traffic_line> nodes;
};

struct adj_list
{
	vector<state_list> lists;
};

// this function takes our collected input data and converts it into a graph using an adjacency list representation 

adj_list generate_graph(input_data input)
{
	adj_list graph;
	state_list sl_temp;
	sl_temp.nodes.push_back(input.live_traffic_lines[0]);
	graph.lists.push_back(sl_temp);

	input.number_of_states = 0;
	string states_copy = input.all_states;
	int p;
	vector<string> state_names;

	for (int i = 0; i < states_copy.length(); ++i)
	{
		if (states_copy[i] == ' ') // in input.all_states, each state name is followed by a white space character. We can count the states by counting these white spaces.
		{
			state_names.push_back(states_copy.substr(0, i)); // copy the substring from 0 to 
			states_copy.erase(0, (i + 1));
			i = 0;
			input.number_of_states += 1;
		}
	}
	// graph.lists.resize(input.number_of_states);

	string curr_state;

	vector<string> known_states;
	known_states.push_back(input.start);

	for (int j = 0; j < input.number_of_states; ++j) // ensures that we check every state
	{
		if (j >= graph.lists.size()) // this can happen if there's a state that only exists as a state_b. Search for state_b's that don't already have a state list.
		{
			for (int h = 0; h < input.number_of_lines; ++h) // check each of our traffic lines to see if they have our missing state as a state_b.
			{
				string temp = input.live_traffic_lines[h].state_b;
				for (int n = 0; n < known_states.size(); ++n) // check our vector of known states to see if the state_b for our current traffic line is included somehwere
				{
					if (temp == known_states[n]) // if state_b matches the current element in our vector of known states; leave the for loop
					{
						break;
					}
					else if (n == (known_states.size() - 1)) // if n has checked all known states and not found a match, the current state_b is our missing state.
					{
						known_states.push_back(input.live_traffic_lines[h].state_b);
						traffic_line tl_temp;
						tl_temp.state_a = input.live_traffic_lines[h].state_b;
						tl_temp.time = -1;
						state_list sl_temp2;
						sl_temp2.nodes.push_back(tl_temp);
						graph.lists.push_back(sl_temp2);
					}
				}
			}
		}
		for (int i = 0; i < input.number_of_lines; ++i) // checks each traffic line to see if it starts from our given state
		{
			if (input.live_traffic_lines[i].state_a == known_states[j] && j + i != 0) // if our state_a matches our current known_state, add the child to the list
			{
				graph.lists[j].nodes.push_back(input.live_traffic_lines[i]);
			}
			else
			{
				bool known = false;
				for (int k = 0; k < known_states.size(); ++k) // check and see if state_a is currently 
				{
					if (input.live_traffic_lines[i].state_a == known_states[k])
					{
						known = true;
					}
				}
				if (!known) // if we have not yet added this state to our adjacency list, do so
				{
					known_states.push_back(input.live_traffic_lines[i].state_a); // add our state to our list of known states
					state_list new_list; // create a new state_list struct. Each of this is a single adjacency linked list.
					//new_list.nodes.push_back(input.live_traffic_lines[i]); // add our current traffic line to our new state_list
					graph.lists.push_back(new_list); // add our new_state list to our graph adjacency list.
				}
			}
		}
	}
	/*for (int i = 0; i < graph.lists.size(); ++i)
	{
		for (int j = 0; j < graph.lists[i].nodes.size(); ++j)
		{
			if (graph.lists[i].nodes[j].time > input.max_time) // if any node has an invalid time value, it is childless. Set time = -1 to show this.
			{
				graph.lists[i].nodes[j].time = -1;
			}
		}
	}*/
	return graph;
}

// This function obtains all of the necessary input values from input.txt and returns them in an input_data struct

input_data get_input(fstream &in) 
{
	input_data input;
	input.max_time = 0;
	string line;
	int n = 0; // tracks how many lines we've read in from the file
	int sunday_start; // this is the line in the input where the sunday line information starts. It is 3 + number_of_lines + 1
	getline(in, line);
	while (!in.eof())
	{
		if (n == 0) // if this is the first line of the file, we get the algorithm 
		{
			if (line == "BFS")
			{
				input.algorithm = 1;
			}
			else if (line == "DFS")
			{
				input.algorithm = 2;
			}
			else if (line == "UCS")
			{
				input.algorithm = 3;
			}
			else if (line == "A*")
			{
				input.algorithm = 4;
			}
			else
			{
				input.algorithm = -1;
				cout << endl << "ERROR: input.txt does not specify a valid search algorithm.";
			}
		}
		else if (n == 1) // this is where START is stored
		{
			input.start = line;
		}
		else if (n == 2) // this is where GOAL is stored
		{
			input.goal = line;
		}
		else if (n == 3) // this is where the number of traffic lines are stored
		{
			input.number_of_lines = stoi(line); // converts the string line to an int and assigns it to number_of_lines, perfect for my lazy self!
			sunday_start = n + input.number_of_lines + 1;
			input.live_traffic_lines.resize(input.number_of_lines);
		}
		else if (n == 4 && input.number_of_lines > 0) // this will be the first line of traffic info
		{
			for (int i = 0; i < input.number_of_lines; ++i) // this loop goes through each of the traffic lines listed in input.txt
			{
				int first_space = 0, second_space = 0;
				first_space = line.find_first_of(" "); // this gives us the location of the first whitespace character
				input.live_traffic_lines[i].state_a = line.substr(0, first_space); // makes state_a a substring of line from the start of the line to the first space
				line.erase(0, first_space + 1);	
				second_space = line.find_first_of(" "); // this gives us the location of the second whitespace character
				input.live_traffic_lines[i].state_b = line.substr(0, second_space); // makes state_b a substring of line from the first space to the second space
				line.erase(0, second_space + 1); // erases everything in line except for the travel time
				input.live_traffic_lines[i].time = stoi(line);
				if (input.live_traffic_lines[i].time > input.max_time) // keep track of the greatest distance
				{
					input.max_time = input.live_traffic_lines[i].time;
				}
				if (input.all_states.find(input.live_traffic_lines[i].state_a) == -1) // if state_a does not exist in our list of all_states, add it
				{
					input.all_states.append(input.live_traffic_lines[i].state_a);
					input.all_states.append(" ");
				}
				if (input.all_states.find(input.live_traffic_lines[i].state_b) == -1) // if state_a does not exist in our list of all_states, add it
				{
					input.all_states.append(input.live_traffic_lines[i].state_b);
					input.all_states.append(" ");
				}
				getline(in, line);
			}
			input.number_of_sun_lines = stoi(line);
			input.sunday_traffic_lines.resize(input.number_of_sun_lines);
			for (int j = 0; j < input.number_of_sun_lines; ++j)
			{
				getline(in, line);
				int space = line.find(" ");
				input.sunday_traffic_lines[j].state = line.substr(0, space);
				line.erase(0, space + 1);
				input.sunday_traffic_lines[j].sun_time = stoi(line);
			}
		}
		n++;
		getline(in, line);
	}
	return input;
}

void print_input(input_data input) 
{
	cout << endl << "Input test start:" << endl;
	cout << "1. Algo = " << input.algorithm << " = ";
	if (input.algorithm == 1) cout << "BFS" << endl;
	else if (input.algorithm == 2) cout << "DFS" << endl;
	else if (input.algorithm == 3) cout << "UCS" << endl;
	else if (input.algorithm == 4) cout << "A*" << endl;
	else if (input.algorithm == -1) cout << "Invalid Algorithm" << endl;
	cout << "2. Start = " << input.start << endl;
	cout << "3. Goal = " << input.goal << endl;
	cout << "4. Number of traffic lines = " << input.number_of_lines << endl;
	cout << "5. Traffic line info: " << endl;
	for (int i = 0; i < input.number_of_lines; ++i)
	{
		cout << "    Line " << i << ": Start = " << input.live_traffic_lines[i].state_a << ", End = " << input.live_traffic_lines[i].state_b << ", Time = " << input.live_traffic_lines[i].time << endl;
	}
	cout << "6. Number of Sunday lines: " << input.number_of_sun_lines << endl;
	for (int i = 0; i < input.number_of_sun_lines; ++i)
	{
		cout << "    Line " << i << ": State = " << input.sunday_traffic_lines[i].state << ", Time = " << input.sunday_traffic_lines[i].sun_time << endl;
	}
	cout << "7. List of Valid States: " << input.all_states << endl;
}

void print_adj_list(adj_list graph)
{
	// struct adj_list
	//		vector<state_list> lists;
	// struct state_list
	//		vector<traffic_line> nodes;

	cout << endl << "Graph test start:" << endl;
	cout << "1. Number of state lists = " << graph.lists.size() << endl;
	cout << "2. Lists: " << endl;
	for (int i = 0; i < graph.lists.size(); ++i)
	{
		cout << "    List " << i << ": " << graph.lists[i].nodes[0].state_a;
		for (int j = 0; j < graph.lists[i].nodes.size(); ++j)
		{
			cout << ", (" << graph.lists[i].nodes[j].state_b << "," << graph.lists[i].nodes[j].time << ")";
			//cout << " -" << graph.lists[i].nodes[j].time << "-> " << graph.lists[i].nodes[j].state_b;
		}
		cout << endl;
	}
}

int main(int argc, char * argv[]) 
{
	fstream in("input.txt");
	input_data input = get_input(in);
	adj_list graph = generate_graph(input);

	print_input(input);
	print_adj_list(graph);

	system("PAUSE");
	return 0;
}

// First attempt at making the graph below, now I'm going to try the same thing but with adjacency lists.

/*
struct state
{
	string name;
	vector<state *> child_states; // pointers to children
	vector<int> child_costs;
};

struct graph
{
	vector<state> states;
	string graph_states; // a string that contains the names of all states in the graph
};

graph make_graaph(input_data input)
{
	graph tree;
	queue<state> state_queue;
	int state_count = input.all_states.length();
	for (int i = 0; i < state_count; ++i) // ensures that we correctly generate all states in the graph
	{
		for (int j = 0; j < input.number_of_lines; ++j)
		{
			if (input.live_traffic_lines[j].state_a == input.start) // if the live traffic line starts at our start location
			{
				if (tree.graph_states.find(input.start) == -1) // if we haven't yet added the head to the graph, do so.
				{
					state head;
					head.name = input.start;
					head.child_states[0]->name = input.live_traffic_lines[j].state_b;
					head.child_costs[0] = input.live_traffic_lines[j].time;
					tree.graph_states.append(head.name);
					tree.graph_states.append(input.live_traffic_lines[j].state_b);
					tree.states[0] = head;
				}
				else if (tree.graph_states.find(input.live_traffic_lines[j].state_b) == -1) // if we already have head but this is a new child, add it
				{
					state child;
					child.name = input.live_traffic_lines[j].state_b;
					tree.states[0].child_states.push_back(&child);
					tree.states.push_back(child);
					tree.states[0].child_costs.push_back(input.live_traffic_lines[j].time);
					tree.graph_states.append(child.name);
				}
			}
			else if (tree.graph_states.find(input.live_traffic_lines[j].state_a) > 0) // if the parent is not the head, but already exists as a child somewhere in the graph, add its child
			{
				int index;
				for (int k = 0; k < tree.states.size; ++k) // find the index of the parent in the tree states
				{
					if (tree.states[k].name == input.live_traffic_lines[j].state_a)
					{
						index = k;
						k = tree.states.size; // break out of for loop
					}
				}

			}
		}
	}
	return tree;
}
*/