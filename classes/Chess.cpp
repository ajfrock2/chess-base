#include "Chess.h"
#include <limits>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <random> //Delete later maybe

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
    initializeStaticMoves();

    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"); // Vanilla
    //FENtoBoard("8/1p3pp1/p2p4/3P4/4P3/5N2/PP3PPP/RNBQKB1R"); // Test 1
    //FENtoBoard("r1bqkbnr/pppppppp/2n5/8/8/2N5/PPPPPPPP/R1BQKBNR"); // Test 2
    //FENtoBoard("rnbqkbnr/pppppppp/2k5/8/8/5K2/PPPPPPPP/RNBQKBNR"); // King Test
    //FENtoBoard("8/8/1R4r1/8/1r4R1/8/8/8"); // Rook test
    //FENtoBoard("8/8/1B4b1/8/1b4B1/8/8/8"); // Bishop test
    //FENtoBoard("8/8/1Q4q1/8/1q4Q1/8/8/8"); // Queen test

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    validMoves.clear();
    initMagicBitboards();
    findValidMoves(validMoves, /*HUMAN_PLAYER*/ getCurrentPlayer()->playerNumber());
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
            int tag = 0;
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
                    tag = 1;
                    break;
                case 'r':
                    piece = PieceForPlayer(playerColor, ChessPiece::Rook);
                    tag = 4;
                    break;
                case 'n':
                    piece = PieceForPlayer(playerColor, ChessPiece::Knight);
                    tag = 2;
                    break;
                case 'b':
                    piece = PieceForPlayer(playerColor, ChessPiece::Bishop);
                    tag = 3;
                    break;
                case 'k':
                    piece = PieceForPlayer(playerColor, ChessPiece::King);
                    tag = 6;
                    break;
                case 'q':
                    piece = PieceForPlayer(playerColor, ChessPiece::Queen);
                    tag = 5;
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
            piece->setGameTag(tag + 128 * playerColor);
            
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
    for(BitMove move: validMoves){
        if(_grid->getSquareByIndex(move.from) == &src
        && _grid->getSquareByIndex(move.to) == &dst){
            return true;
        }
    }
    return false;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    Game::bitMovedFromTo(bit, src, dst);
    validMoves.clear();
    findValidMoves(validMoves, getCurrentPlayer()->playerNumber());
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

// Find all the static moves for Knights and Kings
void Chess::initializeStaticMoves(){
    std::pair<int, int> knightOffsets[8] = {{1,2}, {1,-2}, {-1,2}, {-1,-2}, {2,1}, {2,-1}, {-2,1}, {-2,-1}};
    std::pair<int, int> kingOffsets[8] = {{1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}};
    for(int i=0; i<64; i++){
        int x = i % 8;  // column/file
        int y = i / 8;  // row/rank

        addStaticMoves(staticKnightMoves, knightOffsets, 8, i, x, y);
        addStaticMoves(staticKingMoves, kingOffsets, 8, i, x, y);
    }
}
// Add static moves to the each square array based on offsets
void Chess::addStaticMoves(BitboardElement (&staticPieceMoves)[64], const std::pair<int,int> offsets[], int numOffsets, int i, int x, int y){
    staticPieceMoves[i] = 0ULL;
    for(int j=0; j<numOffsets; j++){
        std::pair<int, int> offset = offsets[j];
            int newX = x + offset.first;
            int newY = y + offset.second;

            // Check for no off board moves
            if(newX < 8 && newX > -1 && newY < 8 && newY >-1){
                staticPieceMoves[i] |= (1ULL << (newX + 8 * newY));
            }
        }
}

// Finds all valid chess moves from the board state
void Chess::findValidMoves(std::vector<BitMove> &validMoves, int playerColor){
    BitboardElement whiteKnightPosBitboard = 0;
    BitboardElement blackKnightPosBitboard = 0;
    BitboardElement whiteKingPosBitboard = 0;
    BitboardElement blackKingPosBitboard = 0;
    BitboardElement whitePawnPosBitboard = 0;
    BitboardElement blackPawnPosBitboard = 0;
    BitboardElement whiteQueenPosBitboard = 0;
    BitboardElement blackQueenPosBitboard = 0;
    BitboardElement whiteRookPosBitboard = 0;
    BitboardElement blackRookPosBitboard = 0;
    BitboardElement whiteBishopPosBitboard = 0;
    BitboardElement blackBishopPosBitboard = 0;

    BitboardElement whitePosBitboard = 0;
    BitboardElement blackPosBitboard = 0;
    BitboardElement occupancyBitboard = 0;


    //Find all pieces and update positionBitBoards
    _grid->forEachSquare([&](ChessSquare* square, int x, int y){
        // Crash Prevention in case of empty squares
        Bit* piece = square->bit();
        if (!piece) return;

        int index = y * 8 + x;

        // Populate position bitboards
        // Magic numbers are the tags of the pieces
        findPiece(whitePawnPosBitboard, piece, index, 1); // White Pawn
        findPiece(whiteKnightPosBitboard, piece, index, 2); // White Knight
        findPiece(whiteBishopPosBitboard, piece, index, 3);// White Bishop
        findPiece(whiteRookPosBitboard, piece, index, 4);// White Rook
        findPiece(whiteQueenPosBitboard, piece, index, 5);// White Queen
        findPiece(whiteKingPosBitboard, piece, index, 6); // White King
        findPiece(blackPawnPosBitboard, piece, index, 129); // Black Pawn
        findPiece(blackKnightPosBitboard, piece, index, 130); // Black Knight
        findPiece(blackBishopPosBitboard, piece, index, 131);// Black Bishop
        findPiece(blackRookPosBitboard, piece, index, 132);// Black Rook
        findPiece(blackQueenPosBitboard, piece, index, 133);// Black Queen
        findPiece(blackKingPosBitboard, piece, index, 134); // Black King
    });

    whitePosBitboard = whiteKnightPosBitboard | whiteKingPosBitboard | whitePawnPosBitboard | 
        whiteBishopPosBitboard | whiteRookPosBitboard | whiteQueenPosBitboard;
    blackPosBitboard = blackKnightPosBitboard | blackKingPosBitboard | blackPawnPosBitboard | 
        blackBishopPosBitboard | blackRookPosBitboard | blackQueenPosBitboard;
    occupancyBitboard = whitePosBitboard | blackPosBitboard;
    

    // White moves
    if(playerColor != 1) // Black/AI is always 1, white/Human can be -1 or 0
    {
        generateKnightMoves(whiteKnightPosBitboard, whitePosBitboard, validMoves);
        generateKingMoves(whiteKingPosBitboard, whitePosBitboard, validMoves);
        generatePawnMoves(whitePawnPosBitboard, blackPosBitboard, occupancyBitboard, validMoves, true);
        generateRookMoves(whiteRookPosBitboard, whitePosBitboard, occupancyBitboard, validMoves);
        generateBishopMoves(whiteBishopPosBitboard, whitePosBitboard, occupancyBitboard, validMoves);
        generateQueenMoves(whiteQueenPosBitboard, whitePosBitboard, occupancyBitboard, validMoves);
    }
    // Black Moves
    else
    {
        generateKnightMoves(blackKnightPosBitboard, blackPosBitboard, validMoves);
        generateKingMoves(blackKingPosBitboard, blackPosBitboard, validMoves);
        generatePawnMoves(blackPawnPosBitboard, whitePosBitboard, occupancyBitboard, validMoves, false);
        generateRookMoves(blackRookPosBitboard, blackPosBitboard, occupancyBitboard, validMoves);
        generateBishopMoves(blackBishopPosBitboard, blackPosBitboard, occupancyBitboard, validMoves);
        generateQueenMoves(blackQueenPosBitboard, blackPosBitboard, occupancyBitboard, validMoves);
    }
}

// Looks at a piece, it if matches the tag, adds it to the bitboard
void Chess::findPiece(BitboardElement &PosBitboard, Bit* piece, int index, int tag){
    if(piece->gameTag() == tag){
    // Add both piece colors to bitboard based on stateString
        PosBitboard |= (1ULL << index);   
    }
}

void Chess::generateKnightMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement tempOffets = staticKnightMoves[indexOfFrom];
        tempOffets = tempOffets & (~friendlyBitboard);
        tempOffets.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, Knight);
        });
    });
}

void Chess::generateKingMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement tempOffets = staticKingMoves[indexOfFrom];
        tempOffets = tempOffets & (~friendlyBitboard);
        tempOffets.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, King);
        });
    });
}

void Chess::generatePawnMoves(BitboardElement pawnPositionBitboard, BitboardElement EnemyPositionBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves, bool white){
    BitboardElement notAFile(0xFEFEFEFEFEFEFEFEULL);
    BitboardElement notHFile(0x7F7F7F7F7F7F7F7FULL);
    BitboardElement rank3(0x0000000000FF0000ULL);
    BitboardElement rank6(0x0000FF0000000000ULL);

    BitboardElement singlePush = 0;
    BitboardElement doublePush = 0;
    BitboardElement attacksLeft = 0;
    BitboardElement attacksRight = 0;
    int singleFrom;
    int doubleFrom;
    int leftFrom;
    int rightFrom;

    if(white){
        // White moves
        singlePush =(pawnPositionBitboard << 8) & (~occupancyBitboard);
        doublePush = ((singlePush & rank3) << 8) & (~occupancyBitboard);
        attacksLeft =(pawnPositionBitboard << 7) & notHFile & EnemyPositionBitboard;
        attacksRight =(pawnPositionBitboard << 9) & notAFile & EnemyPositionBitboard;
            
        singleFrom = -8;
        doubleFrom = -16;
        leftFrom = -7;
        rightFrom = -9;
    }else{
        // Black moves
        singlePush =(pawnPositionBitboard >> 8) & (~occupancyBitboard);
        doublePush = ((singlePush & rank6) >> 8) & (~occupancyBitboard);
        attacksLeft =(pawnPositionBitboard >> 9) & notHFile & EnemyPositionBitboard;
        attacksRight =(pawnPositionBitboard >> 7) &  notAFile & EnemyPositionBitboard;

        singleFrom = 8;
        doubleFrom = 16;
        leftFrom = 9;
        rightFrom = 7;
    }

    singlePush.forEachBit([&](int indexOfTo){
        validMoves.emplace_back(indexOfTo + singleFrom, indexOfTo, Pawn);
    });
    doublePush.forEachBit([&](int indexOfTo){
        validMoves.emplace_back(indexOfTo + doubleFrom, indexOfTo, Pawn);
    });
    attacksLeft.forEachBit([&](int indexOfTo){
        validMoves.emplace_back(indexOfTo + leftFrom, indexOfTo, Pawn);
    });
    attacksRight.forEachBit([&](int indexOfTo){
        validMoves.emplace_back(indexOfTo + rightFrom, indexOfTo, Pawn);
    });   
}

void Chess::generateRookMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement rookAttacks = getRookAttacks(indexOfFrom, occupancyBitboard.getData());
        rookAttacks = rookAttacks & (~friendlyBitboard);
        rookAttacks.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, Rook);
        });
    });
}

void Chess::generateBishopMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement bishopAttacks = getBishopAttacks(indexOfFrom, occupancyBitboard.getData());
        bishopAttacks = bishopAttacks & (~friendlyBitboard);
        bishopAttacks.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, Bishop);
        });
    });
}

void Chess::generateQueenMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement queenAttacks = getQueenAttacks(indexOfFrom, occupancyBitboard.getData());
        queenAttacks = queenAttacks & (~friendlyBitboard);
        queenAttacks.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, Bishop);
        });
    });    
}

//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
    int initialA = -10000000;
    int initialB = 10000000;
    int bestVal = -1000000;
    BitMove bestMove;
    bool foundBestMove = false;
    std::string state = stateString();

    for(BitMove move: validMoves){

        // Perform move
        char captured = state[move.to];
        state[move.to] = state[move.from];
        state[move.from] = '0';

        int moveVal = negamax(state, 0, initialA, initialB, AI_PLAYER);

        // Revert move
        state[move.from] = state[move.to];
        state[move.to] = captured;

        // If the value of the current move is more than the best value, update best
        if (moveVal > bestVal) {
            foundBestMove = true;
            bestMove = move; 
            bestVal = moveVal;
        }
    }
    
    // Finally make the move
    if(foundBestMove) {
        Bit* piece =_grid->getSquareByIndex(bestMove.from)->bit();
        piece->setPosition(_grid->getSquareByIndex(bestMove.to)->getPosition());
        _grid->getSquareByIndex(bestMove.to)->setBit(piece);
        bitMovedFromTo(*piece, *_grid->getSquareByIndex(bestMove.from), *_grid->getSquareByIndex(bestMove.to));
        
        //_grid->getSquareByIndex(bestMove.from)->destroyBit();
    }
}

//
// player is the current player's number (AI or human)
//
int Chess::negamax(std::string& state, int depth, int a, int b, int playerColor)
{
    // TODO adujst score calculation
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 100);
    int score = dist(gen);

    if(playerColor == HUMAN_PLAYER) {
        score = -score;
    }

    if (abs(score) >= 3000) return score; //TODO change Someone likely won, don't continue calculations
    if(depth == 3) return score; //TODO Don't recurse to long

 
    int bestVal = -1000000;
    
    std::vector<BitMove> newMoves;
    findValidMoves(newMoves, playerColor);
    
    for(BitMove move: newMoves){

        // Perform move
        char captured = state[move.to];
        state[move.to] = state[move.from];
        state[move.from] = '0';
        
        bestVal = std::max(bestVal, -negamax(state, depth+1, -b, -a, -playerColor));

        // Revert move
        state[move.from] = state[move.to];
        state[move.to] = captured;

        a = std::max(a, bestVal);
        if(a >= b){
            break;
        }
    }
    return bestVal;
}