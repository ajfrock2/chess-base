#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include "MagicBitboards.h"

constexpr int pieceSize = 80;

/*
enum ChessPiece
{
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};
*/

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;


    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

    void initializeStaticMoves();
    void findValidMoves(std::vector<BitMove> &validMoves);
    void findPiece(BitboardElement &PosBitboard, Bit* piece, int index, int tag);
    void addStaticMoves(BitboardElement (&staticPieceMoves)[64], const std::pair<int,int> offsets[], int numOffsets, int i, int x, int y);
    void generateKnightMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, std::vector<BitMove> &validMoves);
    void generateKingMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, std::vector<BitMove> &validMoves);
    void generatePawnMoves(BitboardElement positionBitboard, BitboardElement enemyPositionBitboard, BitboardElement occupancyBitboard, 
        std::vector<BitMove> &validMoves, bool white);
    void generateRookMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves);
    void generateBishopMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves);
    void generateQueenMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves);
    void updateAI();
    int negamax(std::string& state, int depth, int a, int b, int playerColor);

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
    BitboardElement staticKnightMoves[64];
    BitboardElement staticKingMoves[64];
    BitboardElement staticWhitePawnMoves[64];
    BitboardElement staticBlackPawnMoves[64];
    std::vector<BitMove> validMoves;
};