#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <vector>

using namespace std;

class BoardImpl
{
public:
	BoardImpl(const Game& g);
	void clear();
	void block();
	void unblock();
	bool placeShip(Point topOrLeft, int shipId, Direction dir);
	bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
	void display(bool shotsOnly) const;
	bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
	bool allShipsDestroyed() const;

private:
	const Game& m_game; 
	char grid[MAXROWS][MAXCOLS]; // board
	vector<int> shipIDs; // vector of current shipIds
	int n_shipsDestroyed; 
};

const char isWATER = '.'; // declaring some constants I will use later
const char isHIT = 'X';
const char isMISS = 'o';
const char isBLOCKED = '#';

BoardImpl::BoardImpl(const Game& g)
	: m_game(g)
{
	n_shipsDestroyed = 0;
	clear(); // reset the board
}

void BoardImpl::clear()
{
	for (int r = 0; r < m_game.rows(); r++)
		for (int c = 0; c < m_game.cols(); c++)
			grid[r][c] = isWATER; // making everything water
}

void BoardImpl::block()
{
	// Block cells with 50% probability
	for (int r = 0; r < m_game.rows(); r++)
		for (int c = 0; c < m_game.cols(); c++)
			if (randInt(2) == 0)
			{
				grid[r][c] = isBLOCKED;
			}
}

void BoardImpl::unblock()
{
	for (int r = 0; r < m_game.rows(); r++)
		for (int c = 0; c < m_game.cols(); c++)
		{
			if (grid[r][c] == isBLOCKED)
				grid[r][c] = isWATER;
		}
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir) 
{
	if (shipId < 0 || shipId >= m_game.nShips()) // invalid shipId
		return false;
	
	for (int i = 0; i < shipIDs.size(); i++) {
		if (shipId == shipIDs[i])
			return false; // already used this shipID.
	}

	if (dir == HORIZONTAL) {
		for (int k = 0; k < m_game.shipLength(shipId); k++) { 
			if (topOrLeft.c + k < 0 || topOrLeft.c + k >= m_game.cols()) // checking if falls off the board
				return false;
			if (grid[topOrLeft.r][topOrLeft.c + k] != isWATER) // checking if overlapping something
				return false;
		}
		for (int i = 0; i < m_game.shipLength(shipId); i++)
			grid[topOrLeft.r][topOrLeft.c+ i] = m_game.shipSymbol(shipId); // placing symbol on grid

		shipIDs.push_back(shipId); // everything was cool, so add to vector
		return true;
	}

	else if (dir == VERTICAL) {
		for (int k = 0; k < m_game.shipLength(shipId); k++) { 
			if (topOrLeft.r + k < 0 || topOrLeft.r + k >= m_game.rows()) // checking if falls off the board
				return false;
			if (grid[topOrLeft.r + k][topOrLeft.c] != isWATER) // checking if overlapping something
				return false;
		}
		for (int i = 0; i < m_game.shipLength(shipId); i++)
			grid[topOrLeft.r + i][topOrLeft.c] = m_game.shipSymbol(shipId); // placing on grid

		shipIDs.push_back(shipId); // everything was cool, so add to vector
		return true;
	}
	else {
		return false; // didn't enter valid direction
	}
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
	if (shipId < 0 || shipId >= m_game.nShips()) // invalid shipId
		return false;

	bool found = false; 
	for (int i = 0; i < shipIDs.size(); i++) { // checking whether shipId even exists
		if (shipId == shipIDs[i])
			found = true;
	}
	if (!found)
		return false;
	
	if (dir == HORIZONTAL) {
		for (int p = 0; p < m_game.shipLength(shipId); p++) {
			if (grid[topOrLeft.r][topOrLeft.c + p] != m_game.shipSymbol(shipId)) // checking if the spaces it's going over isn't the ship
				return false;
		}
		for (int k = 0; k < m_game.shipLength(shipId); k++) { // writing over ship
			grid[topOrLeft.r][topOrLeft.c + k] = isWATER;
		}
		shipIDs.erase(shipIDs.begin() + shipId); // removing shipId
		return true;
	}

	else if (dir == VERTICAL) {
		for (int p = 0; p < m_game.shipLength(shipId); p++) {
			if (grid[topOrLeft.r + p][topOrLeft.c] != m_game.shipSymbol(shipId)) // checking if the spaces it's going over isn't the ship
				return false;
		}
		for (int k = 0; k < m_game.shipLength(shipId); k++) { // writing over ship
			grid[topOrLeft.r + k][topOrLeft.c] = isWATER;
		}
		shipIDs.erase(shipIDs.begin() + shipId); // removing shipId
		return true;
	}
	else {
		return false; // didn't enter valid direction
	}
}

void BoardImpl::display(bool shotsOnly) const
{
	cout << "  "; // 2 space indent
	for (int c = 0; c < m_game.cols(); c++) // creating header of numbers
		cout << c;
	cout << endl;

	for (int r = 0; r < m_game.rows(); r++) {
		cout << r << " "; // beginning of each row
		for (int c = 0; c < m_game.cols(); c++) {
			if (shotsOnly) {
				if (grid[r][c] != isHIT && grid[r][c] != isMISS) // if there's something there other than a hit or missed shot, just put water to cover it
					cout << isWATER;
				else
					cout << grid[r][c]; // keep displaying X and o
			}
			else {
				cout << grid[r][c]; // show what it was, whether it be ship symbols or not
			}
		}
		cout << endl;
	}
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
	shotHit = false; // in case we get an early 'return false'
	shipDestroyed = false;
	int k = 0; // counting purposes

	if (p.r < 0 || p.r >= m_game.rows()) // invalid point location
		return false;
	if (p.c < 0 || p.c >= m_game.cols())
		return false;

	if (grid[p.r][p.c] == isHIT || grid[p.r][p.c] == isMISS) // if attacking an X or o, then return false
		return false;
	else {
		for (k; k < m_game.nShips(); k++)
			if (grid[p.r][p.c] == m_game.shipSymbol(k)) {  // if you hit a ship
				grid[p.r][p.c] = isHIT;
				shotHit = true;
				break;
			}
		if (!shotHit) 
			grid[p.r][p.c] = isMISS; // missed shot
	}
	if (shotHit) { // only if it's a hit, do we check if it's destroyed
		shipDestroyed = true;
		for (int r = 0; r < m_game.rows(); r++)
			for (int c = 0; c < m_game.cols(); c++)
				if (grid[r][c] == m_game.shipSymbol(k)) // if the symbol is still on the board, then it's not destroyed
					shipDestroyed = false;

		if (shipDestroyed) {
			n_shipsDestroyed++;
			shipId = k; // set shipId to ship that was destroyed, o/w don't change it.
		}
	}
	return true; // everything worked
}

bool BoardImpl::allShipsDestroyed() const
{
	return (n_shipsDestroyed == m_game.nShips());
}

//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
	m_impl = new BoardImpl(g);
}

Board::~Board()
{
	delete m_impl;
}

void Board::clear()
{
	m_impl->clear();
}

void Board::block()
{
	return m_impl->block();
}

void Board::unblock()
{
	return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
	return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
	return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
	m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
	return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
	return m_impl->allShipsDestroyed();
}
