#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <vector>

using namespace std;

class GameImpl
{
public:
	GameImpl(int nRows, int nCols);
	int rows() const;
	int cols() const;
	bool isValid(Point p) const;
	Point randomPoint() const;
	bool addShip(int length, char symbol, string name);
	int nShips() const;
	int shipLength(int shipId) const;
	char shipSymbol(int shipId) const;
	string shipName(int shipId) const;
	Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause);
	~GameImpl();
private:
	int n_rows;
	int n_cols;
	struct Ship {
		Ship(int len, char sym, string nm)
			: m_length(len), m_symbol(sym), m_name(nm) {}
		int m_length;
		char m_symbol;
		string m_name;
	};
	vector<Ship*> myShips;
};

void waitForEnter()
{
	cout << "Press enter to continue: ";
	cin.ignore(10000, '\n');
}

GameImpl::GameImpl(int nRows, int nCols)
{
	n_rows = nRows;
	n_cols = nCols;
}

GameImpl::~GameImpl() { // do I need?
	int n = myShips.size();
	for (int i = 0; i < n; i++)
		delete myShips[i];
}

int GameImpl::rows() const
{
	return n_rows; 
}

int GameImpl::cols() const
{
	return n_cols;
}

bool GameImpl::isValid(Point p) const
{
	return p.r >= 0 && p.r < rows() && p.c >= 0 && p.c < cols();
}

Point GameImpl::randomPoint() const
{
	return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name)
{
	myShips.push_back(new Ship(length, symbol, name));
	return true;
}

int GameImpl::nShips() const
{
	return myShips.size();
}

int GameImpl::shipLength(int shipId) const
{
	return myShips[shipId]->m_length; 
}

char GameImpl::shipSymbol(int shipId) const
{
	return myShips[shipId]->m_symbol; 
}

string GameImpl::shipName(int shipId) const
{
	return myShips[shipId]->m_name; 
}

Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause)
{
	shouldPause = false;
	Player* winner = nullptr;
	if (!p1->placeShips(b1) || !p2->placeShips(b2)) // if there was any error placing the ships
		return nullptr;

	bool gameOn = true;
	while (gameOn) {
		{ // everything is within a scope, so that local variables eventually get destroyed
			cout << p1->name() << "'s turn. Board for " << p2->name() << endl;
			if (p1->isHuman())
				b2.display(true); // if human, don't display other player's ships
			else
				b2.display(false); // otherwise, do

			Point a = p1->recommendAttack(); // prompting player where to attack
			
			bool  shotHit, shipDestroyed, validShot;
			int shipId;

			if (!b2.attack(a, shotHit, shipDestroyed, shipId)) { 
				cout << p1->name() << " wasted a shot at (" << a.r << "," << a.c << ")." << endl; // wasted shot
				validShot = false;
			}
			else {
				validShot = true;
				string hitOrMiss;
				if (shotHit && !shipDestroyed) // a ship was hit but not destroyed
					hitOrMiss = "hit something";
				else if (shipDestroyed) // ship was destroyed
					hitOrMiss = "destroyed the " + shipName(shipId);
				else
					hitOrMiss = "missed"; // MISSED!!

				cout << p1->name() << " attacked (" << a.r << "," << a.c << ") and " << hitOrMiss \
					<< ", resulting in:" << endl; // stating what just happened
				
				if (p1->isHuman()) 
					b2.display(true); // if human, don't display other player's ships
				else
					b2.display(false); // otherwise, do
			}

			p1->recordAttackResult(a, true, shotHit, shipDestroyed, shipId);

			if (b2.allShipsDestroyed()) { // ball game
				cout << p1->name() << " wins!" << endl;
				winner = p1;
				gameOn = false;
				break;
			}

			if (shouldPause) { // if got to pause
				cout << "Press Enter to Continue: ";
				cin.ignore(10000, '\n');
			}

			// Player 2 (again within brackets), same thing except roles are reversed.
		} {
			cout << p2->name() << "'s turn. Board for " << p1->name() << endl;
			if (p2->isHuman())
				b1.display(true);
			else
				b1.display(false);

			Point a = p2->recommendAttack();
			bool  shotHit, shipDestroyed;
			int shipId;

			if (!b1.attack(a, shotHit, shipDestroyed, shipId))
				cout << p2->name() << " wasted a shot at (" << a.r << "," << a.c << ")." << endl;
			else {
				string hitOrMiss;
				if (shotHit && !shipDestroyed)
					hitOrMiss = "hit something";
				else if (shipDestroyed)
					hitOrMiss = "destroyed the " + shipName(shipId);
				else
					hitOrMiss = "missed";
				cout << p2->name() << " attacked (" << a.r << "," << a.c << ") and " << hitOrMiss \
					<< ", resulting in:" << endl;

				if (p2->isHuman())
					b1.display(true); 
				else
					b1.display(false);
			}

			p2->recordAttackResult(a, true, shotHit, shipDestroyed, shipId);

			if (b1.allShipsDestroyed()) {
				cout << p2->name() << " wins!" << endl;
				winner = p2;
				gameOn = false;
				break;
			}

			if (shouldPause) {
				cout << "Press Enter to Continue: ";
				cin.ignore(10000, '\n');
			}
		}
	}
	return winner;

}

//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
	if (nRows < 1 || nRows > MAXROWS)
	{
		cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
		exit(1);
	}
	if (nCols < 1 || nCols > MAXCOLS)
	{
		cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
		exit(1);
	}
	m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
	delete m_impl;
}

int Game::rows() const
{
	return m_impl->rows();
}

int Game::cols() const
{
	return m_impl->cols();
}

bool Game::isValid(Point p) const
{
	return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
	return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
	if (length < 1)
	{
		cout << "Bad ship length " << length << "; it must be >= 1" << endl;
		return false;
	}
	if (length > rows() && length > cols())
	{
		cout << "Bad ship length " << length << "; it won't fit on the board"
			<< endl;
		return false;
	}
	if (!isascii(symbol) || !isprint(symbol))
	{
		cout << "Unprintable character with decimal value " << symbol
			<< " must not be used as a ship symbol" << endl;
		return false;
	}
	if (symbol == 'X' || symbol == '.' || symbol == 'o')
	{
		cout << "Character " << symbol << " must not be used as a ship symbol"
			<< endl;
		return false;
	}
	int totalOfLengths = 0;
	for (int s = 0; s < nShips(); s++)
	{
		totalOfLengths += shipLength(s);
		if (shipSymbol(s) == symbol)
		{
			cout << "Ship symbol " << symbol
				<< " must not be used for more than one ship" << endl;
			return false;
		}
	}
	if (totalOfLengths + length > rows() * cols())
	{
		cout << "Board is too small to fit all ships" << endl;
		return false;
	}
	return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
	return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
	assert(shipId >= 0 && shipId < nShips());
	return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
	assert(shipId >= 0 && shipId < nShips());
	return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
	assert(shipId >= 0 && shipId < nShips());
	return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
	if (p1 == nullptr || p2 == nullptr || nShips() == 0)
		return nullptr;
	Board b1(*this);
	Board b2(*this);
	return m_impl->play(p1, p2, b1, b2, shouldPause);
}
