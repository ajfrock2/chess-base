#include "Chess.h"
#include <limits>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    
    //FENtoBoard("8/1p3pp1/p2p4/3P4/4P3/5N2/PP3PPP/RNBQKB1R"); // Test 1
    //FENtoBoard("r1bqkbnr/pppppppp/2n5/8/8/2N5/PPPPPPPP/R1BQKBNR"); // Test 2

    startGame();
}

// Takes in a fen string and modifies the board to match it 
// ------------------------------TODO expand logic for task 2,3,4,5------------------------------

// convert a FEN string to a board
// FEN is a space delimited string with 6 fields
// 1: piece placement (from white's perspective)
// NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
// ARE BELOW
// 2: active color (W or B)
// 3: castling availability (KQkq or -)
// 4: en passant target square (in algebraic notation, or -)
// 5: halfmove clock (number of halfmoves since the last capture or pawn advance)
void Chess::FENtoBoard(const std::string& fen) 
{
    // Parsing into 8 strings
    std::vector<std::string> rows;
    std::stringstream ss(fen);
    std::string item;
    while (std::getline(ss, item, '/')) {
        rows.push_back(item);
    }

    int rank = 0;
    int file = 0;
    // Iterate over each letter correlating to different square
    for(int i = 7; i>=0; i--){
        std::string letters = rows[i];
        for(char letter: letters){
            Bit* piece;
            int playerColor = 1; // Default Black
            int asciiVal = static_cast<int> (letter);

            // If uppercase, change to white
            if(asciiVal >= 65 && asciiVal <= 90){
                playerColor = 0;
                letter = std::tolower(letter);
            }

            // Get piece based on char and player number
            switch(letter)
            {
                case 'p':
                    piece = PieceForPlayer(playerColor, ChessPiece::Pawn);
                    break;
                case 'r':
                    piece = PieceForPlayer(playerColor, ChessPiece::Rook);
                    break;
                case 'n':
                    piece = PieceForPlayer(playerColor, ChessPiece::Knight);
                    break;
                case 'b':
                    piece = PieceForPlayer(playerColor, ChessPiece::Bishop);
                    break;
                case 'k':
                    piece = PieceForPlayer(playerColor, ChessPiece::King);
                    break;
                case 'q':
                    piece = PieceForPlayer(playerColor, ChessPiece::Queen);
                    break;
                default:
                    // Number logic
                    int num = letter - '0';
                    file+=num;
                    continue;
            }
            // Add the piece to board
            piece->setPosition(_grid->getSquare(file, rank)->getPosition());
            _grid->getSquare(file, rank)->setBit(piece);
            
            file++;
        }
        rank++;
        file=0;
    }
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return true;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}
