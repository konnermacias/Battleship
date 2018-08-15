#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <vector>
#include <queue>
using namespace std;

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
public:
	AwfulPlayer(string nm, const Game& g);
	virtual bool placeShips(Board& b);
	virtual Point recommendAttack();
	virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
		bool shipDestroyed, int shipId);
	virtual void recordAttackByOpponent(Point p);
private:
	Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
	: Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
	// Clustering ships is bad strategy
	for (int k = 0; k < game().nShips(); k++)
		if (!b.placeShip(Point(k, 0), k, HORIZONTAL))
			return false;
	return true;
}

Point AwfulPlayer::recommendAttack()
{
	if (m_lastCellAttacked.c > 0)
		m_lastCellAttacked.c--;
	else
	{
		m_lastCellAttacked.c = game().cols() - 1;
		if (m_lastCellAttacked.r > 0)
			m_lastCellAttacked.r--;
		else
			m_lastCellAttacked.r = game().rows() - 1;
	}
	return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point  p , bool  validShot ,
	bool  shotHit , bool  shipDestroyed ,
	int  shipId )
{
	// AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point  p )
{
	// AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
	bool result(cin >> r >> c);
	if (!result)
		cin.clear();  // clear error state so can do more input operations
	cin.ignore(10000, '\n');
	return result;
}

class HumanPlayer : public Player {
public:
	HumanPlayer(string nm, const Game& g);
	bool isHuman() const { return true; }
	bool placeShips(Board &b);
	Point recommendAttack();
	void recordAttackResult(Point p, bool validShot, bool shotHit,
		bool shipDestroyed, int shipId);
	virtual void recordAttackByOpponent(Point p);
private:
	Point m_lastCellAttacked;
};


HumanPlayer::HumanPlayer(string nm, const Game& g) : Player(nm, g){}

bool HumanPlayer::placeShips(Board &b) { 
	cout << Player::name() << " must place " << game().nShips() << " ships." << endl;
	for (int i = 0; i < game().nShips(); i++) { // placing each ship
		b.display(false); // we are showing the ship symbols

		bool messedUp = true;
		Direction dir;
		while (messedUp) { // as long as something is wrong, keep looping
			string d;
			cout << "Enter h or v for direction of " << game().shipName(i) << " (length " << game().shipLength(i) << "): ";
			getline(cin, d); // using getline bc they could enter like "ko24" and that would be a huge error

			if (d == "h") {
				dir = HORIZONTAL;
				messedUp = false;
			}
			else if (d == "v") {
				dir = VERTICAL;
				messedUp = false;
			}
			else {
				cout << "Direction must be h or v." << endl;
			}
		}

		messedUp = true;
		while (messedUp) {
			if (dir == HORIZONTAL)
				cout << "Enter row and column of leftmost cell (e.g. 3 5): ";
			else
				cout << "Enter row and column of topmost cell (e.g. 3 5): ";
			int r, c;
			if (getLineWithTwoIntegers(r, c)) // getting coordinates
				if (!b.placeShip(Point(r, c), i, dir)) // placing the ship
					cout << "The ship can not be placed there." << endl;
				else
					messedUp = false; // worked
			else
				cout << "You must enter two integers." << endl;
		}
	}
	return true;
}

Point HumanPlayer::recommendAttack() { // prompting human for attack coordinates
	int r, c;
	cout << "Enter the row and column to attack (e.g. 3 5): ";
	if (getLineWithTwoIntegers(r, c)) {
		m_lastCellAttacked = Point(r, c);
		return m_lastCellAttacked;
	}
}

void HumanPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, // these do nothing for humans
	bool shipDestroyed, int shipId) {
	// do nothing
}

void HumanPlayer::recordAttackByOpponent(Point p) {
	// do nothing
}


//*********************************************************************
//  MediocrePlayer
//*********************************************************************

Direction getRandDirection() {
	if (randInt(2) == 0)
		return HORIZONTAL;
	else
		return VERTICAL;
}



//// TODO:  You need to replace this with a real class declaration and
////        implementation.
class MediocrePlayer : public Player {
public:
	MediocrePlayer(string nm, const Game& g);
	bool placeShips(Board &b);
	Point recommendAttack();
	void recordAttackResult(Point p, bool validShot, bool shotHit,
		bool shipDestroyed, int shipId);
	virtual void recordAttackByOpponent(Point p);
	bool recursivePlaceShips(Board &b, int k, int& count);
private:
	char grid[MAXROWS][MAXCOLS]; // another board to keep track of where I have taken shots
	Point m_sourceCell;
	bool inSearch;
};


MediocrePlayer::MediocrePlayer(string nm, const Game& g) : Player(nm, g), inSearch(true) {
	
}

bool MediocrePlayer::placeShips(Board &b) {
	
	int k = 0, trial = 0, count;// used for shipId inside recursivePlaceShips
	while (trial < 50) {
		b.block();
		count = 0;
		trial++;
		if (recursivePlaceShips(b, k, count)) {
			b.unblock();
			break;
		}
		b.unblock();
	}
	if (count == game().nShips())
		return true;
	return false;
}

bool MediocrePlayer::recursivePlaceShips(Board &b, int k, int& count) {

	if (count == game().nShips())
		return true;

	for (int r = 0; r < game().rows(); r++)
		for (int c = 0; c < game().cols(); c++) {
			Direction dir = getRandDirection();
			if (dir == HORIZONTAL) {
				if (b.placeShip(Point(r, c), k, HORIZONTAL)) {// if we can place the ship down
					count++;
					if (!recursivePlaceShips(b, k + 1, count)) { // if it fails other places, unplace the ship
						count--;
						b.unplaceShip(Point(r, c), k, HORIZONTAL);
					}
					else {
						return true;
					}

				}
				else if (b.placeShip(Point(r, c), k, VERTICAL)) {
					count++;
					if (!recursivePlaceShips(b, k + 1, count)) { // if it fails other places, unplace the ship
						b.unplaceShip(Point(r, c), k, VERTICAL);
					}
					else {
						return true;
					}
				}
			}
			else {
				if (b.placeShip(Point(r, c), k, VERTICAL)) {
					count++;
					if (!recursivePlaceShips(b, k + 1, count)) { // if it fails other places, unplace the ship
						--count;
						b.unplaceShip(Point(r, c), k, VERTICAL);
					}
					else {
						return true;
					}
				}
				else if (b.placeShip(Point(r, c), k, HORIZONTAL)) {
					count++;
					if (!recursivePlaceShips(b, k + 1, count)) { // if it fails other places, unplace the ship
						--count;
						b.unplaceShip(Point(r, c), k, HORIZONTAL);
					}
					else {
						return true;
					}
				}
			}
		}

	return false;
}

Point MediocrePlayer::recommendAttack() {
	if (inSearch) {
		Point ran(game().randomPoint()); // get random point
		while (grid[ran.r][ran.c] == '#') // if that spot is marked, get a new random point
			ran = game().randomPoint();
		grid[ran.r][ran.c] = '#'; // mark that spot for later
		return ran;
	}
	else {
		//  we need to attack in four directions! Never Eat Shredded Wheat
		bool bordered = true;
		bool worked;

		while (bordered) {
			int p = randInt(4); // random number between 0 and 3
			Point pnt = m_sourceCell; // reference to source cell

			switch (p) {
			case 0: // NORTH
				worked = true;
				while (grid[pnt.r][pnt.c] == '#') { // while it is blocked, keep finding new points

					// HARD CODE WARNING //
					bool allBlocked = true;
					for (int i = 1; i <= m_sourceCell.r; i++) { // testing if all the points
						if (i > 4) // testing points out of range
							break; // this will make it fail
						if (grid[m_sourceCell.r - i][m_sourceCell.c] != '#') {
							allBlocked = false; // there is a free space!
							break; // break out of for loop
						}
					}
					if (allBlocked) { // if they were all blocked, then let's cut out of the while loop
						worked = false;
						break;
					}
					if (m_sourceCell.r < 4)
						pnt = Point(m_sourceCell.r - (randInt(m_sourceCell.r) + 1), m_sourceCell.c); // if within 3 cells of the border adjust where to attack
					else
						pnt = Point(m_sourceCell.r - (randInt(4) + 1), m_sourceCell.c); // otherwise we got full range
				}
				if (worked) { // if everything was ok
					grid[pnt.r][pnt.c] = '#'; // mark on board
					bordered = false;
					return pnt;
				}
				break;
			case 1: // EAST
				worked = true;
				while (grid[pnt.r][pnt.c] == '#') {

					int dis = game().cols() - 1 - m_sourceCell.c; // dis from border
					// HARD CODE WARNING //
					bool allBlocked = true;
					for (int i = 1; i <= dis; i++) {
						if (i > 4) // testing points out of range
							break;
						if (grid[m_sourceCell.r][m_sourceCell.c + i] != '#') {
							allBlocked = false;
							break;
						}
					}
					if (allBlocked) {
						worked = false;
						break;
					}

					if (dis < 4)
						pnt = Point(m_sourceCell.r, m_sourceCell.c + (randInt(dis) + 1)); // if within 3 cells of the border adjust where to attack
					else
						pnt = Point(m_sourceCell.r, m_sourceCell.c + (randInt(4) + 1)); // otherwise we got full range
				}
				if (worked) {
					grid[pnt.r][pnt.c] = '#'; // mark on board
					bordered = false;
					return pnt;
				}
				break;
			case 2: // SOUTH
				worked = true;
				while (grid[pnt.r][pnt.c] == '#') {

					int dis = game().rows() - 1 - m_sourceCell.r; // dis from border
					// HARD CODE WARNING //
					bool allBlocked = true;
					for (int i = 1; i <= dis; i++) {
						if (i > 4)
							break;
						if (grid[m_sourceCell.r + i][m_sourceCell.c] != '#') {
							allBlocked = false;
							break;
						}
					}
					if (allBlocked) {
						worked = false;
						break;
					}

					if (dis < 4)
						pnt = Point(m_sourceCell.r + (randInt(dis) + 1), m_sourceCell.c); // if within 3 cells of the border adjust where to attack
					else
						pnt = Point(m_sourceCell.r + (randInt(4) + 1), m_sourceCell.c); // otherwise we got full range
				}
				if (worked) {
					grid[pnt.r][pnt.c] = '#'; // mark on board
					bordered = false;
					return pnt;
				}
				break;
			case 3: // WEST
				worked = true;
				while (grid[pnt.r][pnt.c] == '#') { // while blocked, keep trying to find a new point
					
					// HARD CODE WARNING //
					bool allBlocked = true;
					for (int i = 1; i <= m_sourceCell.c; i++) {
						if (i > 4)
							break;
						if (grid[m_sourceCell.r][m_sourceCell.c - i] != '#') {
							allBlocked = false;
							break;
						}
					}
					if (allBlocked) {
						worked = false;
						break;
					}

					if (m_sourceCell.c < 4)
						pnt = Point(m_sourceCell.r, m_sourceCell.c - (randInt(m_sourceCell.c) + 1)); // if within 3 cells of the border adjust where to attack
					else
						pnt = Point(m_sourceCell.r, m_sourceCell.c - (randInt(4) + 1)); // otherwise we got full range
				}
				if (worked) {
					grid[pnt.r][pnt.c] = '#'; // mark on board
					bordered = false;
					return pnt;
				}
				break;
			default:
				break;
			}
		}
	}
}

void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit,
	bool shipDestroyed, int shipId) {

	bool bigShips = false;
	for (int i = 0; i < game().nShips(); i++) {
		if (game().shipLength(i) >= 6)
			bigShips = true;
	}
	if (bigShips) {
		inSearch = true; // if the game has ships of length of 6 or more, switch to state1
	}
	else {
		if (inSearch && shotHit) {
			inSearch = false;
			m_sourceCell = p; // we have a source cell
			if (shipDestroyed)
				inSearch = true;
		}
		else {
			if (shipDestroyed)
				inSearch = true;
		}
	}
}

void MediocrePlayer::recordAttackByOpponent(Point p) {
	// do nothing
}

// Remember that Mediocre::placeShips(Board& b) must start by calling
// b.block(), and must call b.unblock() just before returning.

//*********************************************************************
//  GoodPlayer
//*********************************************************************

// TODO:  You need to replace this with a real class declaration and
//        implementation.
class GoodPlayer : public Player {
public:
	GoodPlayer(string nm, const Game& g);
	bool placeShips(Board &b);
	Point recommendAttack();
	void recordAttackResult(Point p, bool validShot, bool shotHit,
		bool shipDestroyed, int shipId);
	virtual void recordAttackByOpponent(Point p);
bool recursivePlaceShips(Board &b, int k, int& count);
private:
	char grid[MAXROWS][MAXCOLS]; // another board to keep track of where I have taken shots
	Point m_sourceCell;
	bool inSearch;

};


GoodPlayer::GoodPlayer(string nm, const Game& g) : Player(nm, g), inSearch(true) {}

bool GoodPlayer::placeShips(Board &b) {

	int k = 0, trial = 0, count;// used for shipId inside recursivePlaceShips
	while (trial < 50) {
		b.block();
		count = 0;
		trial++;
		if (recursivePlaceShips(b, k, count)) {
			b.unblock();
			break;
		}
		b.unblock();
	}
	if (count == game().nShips())
		return true;
	return false;
}

bool GoodPlayer::recursivePlaceShips(Board &b, int k, int& count) {

	if (count == game().nShips())
		return true;

	for (int r = 0; r < game().rows(); r++)
		for (int c = 0; c < game().cols(); c++) {
			Direction dir = getRandDirection();
			if (dir == HORIZONTAL) {
				if (b.placeShip(Point(r, c), k, HORIZONTAL)) {// if we can place the ship down
					count++;
					if (!recursivePlaceShips(b, k + 1, count)) { // if it fails other places, unplace the ship
						count--;
						b.unplaceShip(Point(r, c), k, HORIZONTAL);
					}
					else {
						return true;
					}

				}
				else if (b.placeShip(Point(r, c), k, VERTICAL)) {
					count++;
					if (!recursivePlaceShips(b, k + 1, count)) { // if it fails other places, unplace the ship
						b.unplaceShip(Point(r, c), k, VERTICAL);
					}
					else {
						return true;
					}
				}
			}
			else {
				if (b.placeShip(Point(r, c), k, VERTICAL)) {
					count++;
					if (!recursivePlaceShips(b, k + 1, count)) { // if it fails other places, unplace the ship
						--count;
						b.unplaceShip(Point(r, c), k, VERTICAL);
					}
					else {
						return true;
					}
				}
				else if (b.placeShip(Point(r, c), k, HORIZONTAL)) {
					count++;
					if (!recursivePlaceShips(b, k + 1, count)) { // if it fails other places, unplace the ship
						--count;
						b.unplaceShip(Point(r, c), k, HORIZONTAL);
					}
					else {
						return true;
					}
				}
			}
		}

	return false;
}

Point GoodPlayer::recommendAttack() {
	if (inSearch) { // checker board random
		Point p;
		for (int r = 0; r < game().rows(); r++) {
			for (int c = 0; c < game().cols(); c++)
				if ((r + c) % 2 != 0 && grid[r][c] != '#') {
					p = Point(r, c);
					grid[p.r][p.c] = '#';
					return p;
				}
		}
		for (int r = 0; r < game().rows(); r++) {
			for (int c = 0; c < game().cols(); c++)
				if ((r + c) % 2 == 0 && grid[r][c] != '#'){
				p = Point(r, c);
				grid[p.r][p.c] = '#';
				return p;
				}
		}
		grid[p.r][p.c] = '#'; // safety in case rows() is zero or something
		return p;
	}
	else {
		queue<Point> myPoints;
		myPoints.push(m_sourceCell);

		while (!myPoints.empty()) {
			Point current = myPoints.front();
			myPoints.pop();
			if (grid[current.r][current.c] == '#') {
				if (current.r > 0) {
					if (grid[current.r - 1][current.c] != '#') { // NORTH //
						grid[current.r - 1][current.c] = '#';
						return Point(current.r - 1, current.c); // found available space
					}
					else
					{
						myPoints.push(Point(current.r - 1, current.c));
					}
				}

				if (game().cols() - 1 - current.c > 0) {
					if (grid[current.r][current.c + 1] != '#') { // EAST //
						grid[current.r][current.c + 1] = '#';
						return Point(current.r, current.c + 1); // found available space
					}
					else
					{
						myPoints.push(Point(current.r, current.c + 1));
					}
				}

				if (game().rows() - 1 - current.r > 0) {
					if (grid[current.r + 1][current.c] != '#') { // SOUTH //
						grid[current.r + 1][current.c] = '#';
						return Point(current.r + 1, current.c); // found available space
					}
					else
					{
						myPoints.push(Point(current.r + 1, current.c));
					}
				}

				if (current.c > 0) {
					if (grid[current.r][current.c - 1] != '#') { // WEST //
						grid[current.r][current.c - 1] = '#';
						return Point(current.r, current.c - 1); // found available space
					}
					else
					{
						myPoints.push(Point(current.r, current.c - 1));
					}
				}
			}
		}
	}
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit,
	bool shipDestroyed, int shipId) {
	
	bool bigShips = false;
	for (int i = 0; i < game().nShips(); i++) {
		if (game().shipLength(i) >= 6)
			bigShips = true;
	}
	if (bigShips) {
		inSearch = true; // if the game has ships of length of 6 or more, switch to state1
	}
	else {
		if (inSearch && shotHit) {
			inSearch = false;
			m_sourceCell = p; // we have a source cell
			if (shipDestroyed)
				inSearch = true;
		}
		else {
			if (shipDestroyed)
				inSearch = true;
		}
	}
}

void GoodPlayer::recordAttackByOpponent(Point p) {
	// do nothing. It really won't make him play any different. ALWAYS PLAY TO WIN
}

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
	static string types[] = {
		"human", "awful", "mediocre", "good"
	};

	int pos;
	for (pos = 0; pos != sizeof(types) / sizeof(types[0]) &&
		type != types[pos]; pos++)
		;
	switch (pos)
	{
	case 0:  return new HumanPlayer(nm, g);
	case 1:  return new AwfulPlayer(nm, g);
	case 2:  return new MediocrePlayer(nm, g);
	case 3:  return new GoodPlayer(nm, g);
	default: return nullptr;
	}
}
