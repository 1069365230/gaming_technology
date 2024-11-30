#include<iostream>
#include<list>
using namespace std;

class Node {
	private:
		Node* previousNode;
		int x;
		int y;
		float costSoFar = 9999;
		string name = "not set yet";
		float weight = 0.1;
		list<string> unvisitedNeighbours{};
	public:
		int getX();
		int getY();
		float getCost();
		void setCoord(int x, int y);
		void setCost(float c);
		void setupName();
		string getName();
		void setWeight(int x, int y);
		float getWeight();
		void addneighbourNode(int x, int y, list<Node*> & l);
		list<string> getNeighbourList();
		Node* findNode(int x, int y, list<Node*> & l);
		bool hasCoord(int x, int y, list<Node*> & l);
		bool operator == (const Node& n) const { return name == n.name && x == n.x && y == n.y; };
		bool operator != (const Node& n) const { return !operator==(n); }

		void setPreviousNode(Node* previous);
		Node* getPreviousNode();
};

int Node::getX() {
	return x;
}

int Node::getY() {
	return y;
}
float Node::getCost() {
	return costSoFar;
}
void Node::setCoord(int x, int y) {
	this->x = x;
	this->y = y;
}
void Node::setCost(float c) {
	this->costSoFar = c;
}

string Node::getName() {
	return name;
}
void Node::setupName() {
	string firstname;
	string lastname;
	switch (this->x) {
		case 0:
			firstname = "A";
			break;
		case 1:
			firstname = "B";
			break;
		case 2:
			firstname = "C";
			break;
		case 3:
			firstname = "D";
			break;
		case 4:
			firstname = "E";
			break;
		case 5:
			firstname = "F";
			break;
	}
	switch (this->y) {
		case 0:
			lastname = "1";
			break;
		case 1:
			lastname = "2";
			break;
		case 2:
			lastname = "3";
			break;
		case 3:
			lastname = "4";
			break;
		case 4:
			lastname = "5";
			break;
		case 5:
			lastname = "6";
			break;
	}

	this->name = firstname + lastname;
}

void Node::setWeight(int x, int y) {
	if (x == 0 && y != 0) {
		this->weight = 1;
	}
	//second column
	if (x == 1 && y == 2) {
		this->weight = 1;
	}
	if (x == 1 && y == 3) {
		this->weight = 0.5;
	}
	//third column
	if (x == 2 && y == 0) {
		this->weight = 1;
	}
	if (x == 2 && y == 1) {
		this->weight = 0.5;
	}
	if (x == 2 && y == 2) {
		this->weight = 0.5;
	}
	if (x == 2 && y == 3) {
		this->weight = 0.5;
	}
	//fourth column
	if (x == 3 && y == 0) {
		this->weight = 1;
	}
	if (x == 3 && y == 5) {
		this->weight = 0.5;
	}
	//fifth column
	if (x == 4 && y == 0) {
		this->weight = 1;
	}
	if (x == 4 && y == 5) {
		this->weight = 1;
	}
	//sixth column
	if (x == 5) {
		this->weight = 1;
	}
}

float Node::getWeight() {
	return this->weight;
}


void Node::addneighbourNode(int currentX, int currentY, list<Node*> & l) {
	//pushback all the neighbour nodes of this current node
	if (hasCoord(currentX, currentY + 1, l)) {
		Node* node = findNode(currentX, currentY + 1, l);
		this->unvisitedNeighbours.push_back(node->getName());
	}
	if (hasCoord(currentX, currentY - 1, l)) {
		Node* node = findNode(currentX, currentY - 1, l);
		this->unvisitedNeighbours.push_back(node->getName());
	}
	if (hasCoord(currentX + 1, currentY, l)) {
		Node* node = findNode(currentX + 1, currentY, l);
		this->unvisitedNeighbours.push_back(node->getName());
	}
	if (hasCoord(currentX - 1, currentY, l)) {
		Node* node = findNode(currentX - 1, currentY, l);
		this->unvisitedNeighbours.push_back(node->getName());
	}
	
}

list<string> Node::getNeighbourList() {
	return this->unvisitedNeighbours;
}

ostream& operator<< (ostream& o, Node node) {
	o <<"CSF: "<< node.getCost() << " weight: " << node.getWeight() << " " << node.getName() << ": (" << node.getX() << ", " << node.getY() << ")";
	return o;
}



void printList(list<Node*> l) {
	list<Node*>::iterator i;
	int row = 0;
	for (i = l.begin(); i != l.end(); i++) {
		if (row % 6 == 0) {
			cout << endl;
		
		}
		cout << **i << " ";
		row++;
	}
	cout << endl;
}
//used for printing neighbours
void printList(list<string> l) {
	list<string>::iterator i;
	for (i = l.begin(); i != l.end(); i++) {
		cout << *i << " ";
	}
	cout << endl;
}


Node* Node::findNode(int x, int y, list<Node*> & l) {
	list<Node*>::iterator i;
	for (i = l.begin(); i != l.end(); i++) {
		if ((*i)->getX() == x && (*i)->getY() == y) {
			//cout << "neighbours: " << *i << endl;
			return *i;
		
		}
		
		
	}

}

Node* findNode(string name, list<Node*> l) {
	list<Node*>::iterator i;
	for (i = l.begin(); i != l.end(); i++) {
		if ((*i)->getName() == name) {
			return *i;

		}
	}

}
Node* findSmallestCost(list<Node*> l) {
	list<Node*>::iterator i;
	float min = 99;
	
	for (i = l.begin(); i != l.end(); i++) {
		if ((*i)->getCost() < min) {
			min = (*i)->getCost();
		}
	}
	cout << "min is: " << min << endl;;
	for (i = l.begin(); i != l.end(); i++) {
		
		if ((*i)->getCost() == min) {
			//cout << "choosed: " << (*i)->getName() << endl;
			return *i;
		}
	}


}

bool Node::hasCoord(int x, int y, list<Node*> & l) { 
	list<Node*>::iterator i;
	for (i = l.begin(); i != l.end(); i++) {

		if ((*i)->getX() == x && (*i)->getY() == y) {
			return true;
		}
	}
	return false;
}


void Node::setPreviousNode(Node* previous) {
	this->previousNode = previous;
}

Node* Node::getPreviousNode() {
	return this->previousNode;
}

//with the referrence or not
void setupNeighbours(list<Node*> & l) {

	list<Node*>::iterator i;
	for (i = l.begin(); i != l.end(); i++) {
		int x = (*i)->getX();
		int y = (*i)->getY();
		cout << **i << endl;
		//Node temp = *i;
		//i->addneighbourNode(x, y, l);
		(*i)->addneighbourNode(x, y, l);
		cout << "neigbours: ";
		printList((*i)->getNeighbourList());
		cout << endl;
		cout << "-----------------------" << endl;
	}
}

bool checkInUnvisit(string nodeName, list<Node*>& unvisited) {
	list<Node*>::iterator i;
	for (i = unvisited.begin(); i != unvisited.end(); i++) {
		if ((*i)->getName() == nodeName) {
			return true;

		}
	}
	return false;

}

//add to open list and remove from unvisited
void processNode(Node* currentNode, list<string> neighbourNames, list<Node*>  & openList, list<Node*> & unvisited, list<Node*>& closed) {
	for (string name : neighbourNames) {
		//if the closing list has the neighbour then return
		//does not process the neighbour nodes where it came from
		/*if (currentNode->getPreviousNode()->getName() == name) {
			return;
		}*/
		cout << "NAMECHECKINLOOP" << name << endl;
		if (checkInUnvisit(name, closed)) {
			cout << "do nothing" << endl;

			continue;
			//do nothing
			//should not return 
			//return;
		}

		
		//bool if the unvisited graph has the node 
		if (checkInUnvisit(name, unvisited) == false) {
			cout << "node does not exisit in unvisited:" << endl;

			//if it doesnt exist 
			//search in the open list find in open list/ not in open list, in closed list
			Node* node = findNode(name, openList);
			cout << *node << endl;
			cout << "curent		NODE: " << endl;
			cout << *currentNode << endl;
			//check if current node + neighbours weight is smaller than the stored cost so far Yes? -> reset also reset prevouis node
			// NO? do nothing{ }
			float storedCost = node->getCost();
			float newCost = currentNode->getCost() + node->getWeight();
			if (newCost < storedCost) {
				node->setCost(newCost);
				node->setPreviousNode(currentNode);
			}
			continue;
		}
		
		//if the unvisited graph has the node
		Node* node = findNode(name, unvisited);
		cout << "does is find every node:? " << node->getName();

		//set the neighbours cost so far = current node + neighbours weight
		node->setCost(currentNode->getCost() + node->getWeight());

		//first add to the open list
		openList.push_back(node);

		//remove the node from the whole graph
		unvisited.remove(node);

		//store the prevouis node on the node(the node in the open list) 
		node->setPreviousNode(currentNode);

		
	}

}

void moveToClosingList(Node* node, list<Node*>& close, list<Node*> & open) {
	close.push_back(node);
	open.remove(node);
}

void dijkstra(list<Node*> & open, list<Node*> & closed, list<Node*> & unvisited) {
	cout << "------------------------------------------------------" << endl;
	Node* withSmallestCost = findSmallestCost(open);
	cout<<"choosed node: " << withSmallestCost->getName() << endl;
	list<string> neighbours = withSmallestCost->getNeighbourList();
	
	cout << "DIJIKSTRA" << endl;
	//cout << "N:" << endl;
	//printList(neighbours);
	cout << "OPENLIST:";
	printList(open);
	processNode(withSmallestCost, neighbours, open, unvisited, closed);
	cout << endl;
	cout << "OPENLIST after:";
	printList(open);
	//cout << "open list now: " << endl;
	//printList(open);

	/*debug for previous node
	list<Node*>::iterator i;
	for (i = open.begin(); i != open.end(); i++) {
		Node* pre = (*i)->getPreviousNode();
		cout << "NODE" << endl;
		cout << **i << endl;
		cout << "pre: " << endl;
		cout << *pre << endl;
	}*/
	
	//move to closing list
	moveToClosingList(withSmallestCost, closed, open);

}


bool closedHasGoal(string goalName, list<Node*> & closed) {
	list<Node*>::iterator i;
	for (i = closed.begin(); i != closed.end(); i++) {
		if ((*i)->getName() == goalName) {
			return true;

		}
	}
	return false;
}
string getPath(string start, string goal, list<Node*>& closed) {
	Node* traceback = findNode(goal, closed);
	cout << "totalCost: " << traceback->getCost() << endl;
	string path = goal;
	list<Node*>::iterator i;

	while (traceback->getPreviousNode()->getName() != start) {
		path = traceback->getPreviousNode()->getName()+path;
		traceback = traceback->getPreviousNode();
	}
	path = start + path;
	return path;

}
int main() {
	//construction of map
	//36 node, each node has its own weight
	//unvisited node:
	list<Node*> unvisited;
	for (int x = 0; x < 6; x++) {
		for (int y = 0; y < 6; y++) {
			Node* node = new Node();
			node->setCoord(x, y);
			node->setupName();
			node->setWeight(x, y);
			unvisited.push_back(node);
		}
	
	}
	printList(unvisited);
	setupNeighbours(unvisited);

	list<Node*> openList;
	list<Node*> closedList;
	//initial stage
	Node* start = findNode("B5", unvisited);
	//set the sofarcost to 0(might need to change)
	start->setCost(0);
	start->setPreviousNode(start);
	//add to the open list
	openList.push_back(start);
	//remove from the whole graph
	unvisited.remove(start);

	/*testing for the first two steps, works no problem
	dijkstra(openList, closedList, unvisited);
	cout << "unvisited now: " << endl;
	printList(unvisited);
	cout << "open list now: " << endl;
	//cout << openList.size() << endl;
	printList(openList);
	cout << "closing list now: " << endl;
	//cout << openList.size() << endl;
	printList(closedList);
	//////////////////////////////second one
	dijkstra(openList, closedList, unvisited);
	cout << "unvisited now: " << endl;
	printList(unvisited);
	cout << "open list now: " << endl;
	//cout << openList.size() << endl;
	printList(openList);
	cout << "closing list now: " << endl;
	//cout << openList.size() << endl;
	printList(closedList);
	*/
	
	while (closedHasGoal("D4", closedList) == false) {
		dijkstra(openList, closedList, unvisited);
	
	}
	cout << "closing list now: " << endl;
	printList(closedList);
	cout << closedList.size() << endl;
	string path = getPath("B5", "D4", closedList);
	cout <<"path: " << path << endl;
	cout << "unvisited now: " << endl;
	printList(unvisited);
	return 0;
}