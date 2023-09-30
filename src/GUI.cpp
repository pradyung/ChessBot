#include "Headers/GUI.hpp"

namespace Chess
{
  Board board;

  Images images;

  int draggingPieceIndex;

  Texture squares[5];

  Sprite boardSquares[64];

  Sprite whitePawns[64];
  Sprite whiteKnights[64];
  Sprite whiteBishops[64];
  Sprite whiteRooks[64];
  Sprite whiteQueens[64];
  Sprite whiteKings[64];

  Sprite blackPawns[64];
  Sprite blackKnights[64];
  Sprite blackBishops[64];
  Sprite blackRooks[64];
  Sprite blackQueens[64];
  Sprite blackKings[64];

  Bitboard redHighlightsBitboard;
  Bitboard yellowHighlightsBitboard;
  Bitboard grayHighlightsBitboard;

  Sprite redHighlightsSprites[64];
  Sprite yellowHighlightsSprites[64];
  Sprite grayHighlightsSprites[64];

  Sprite whitePromotionPieces[4];
  Sprite blackPromotionPieces[4];

  Sprite draggingPieceSprite;

  bool gameOver;

  int GUIHandler::getSquareIndex(int x, int y)
  {
    int adjustedX = x - WIDTH_PADDING;
    int adjustedY = y - HEIGHT_PADDING;

    return (adjustedY / SQUARE_SIZE) * 8 + (adjustedX / SQUARE_SIZE);

    // return (y / SQUARE_SIZE) * 8 + (x / SQUARE_SIZE);
  }

  int GUIHandler::getPromotionPiece(int x, int y)
  {
    int index = getSquareIndex(x, y);

    switch (index)
    {
    case 10:
      return Piece::WHITE_QUEEN;
    case 11:
      return Piece::WHITE_ROOK;
    case 12:
      return Piece::WHITE_BISHOP;
    case 13:
      return Piece::WHITE_KNIGHT;
    case 50:
      return Piece::BLACK_QUEEN;
    case 51:
      return Piece::BLACK_ROOK;
    case 52:
      return Piece::BLACK_BISHOP;
    case 53:
      return Piece::BLACK_KNIGHT;
    default:
      return -1;
    }
  }

  Vector2f GUIHandler::getSquareCoordinates(int index)
  {
    return getSquareCoordinates(index % 8, index / 8);
  }

  Vector2f GUIHandler::getSquareCoordinates(int x, int y)
  {
    return Vector2f(x * SQUARE_SIZE, y * SQUARE_SIZE) + Vector2f(WIDTH_PADDING, HEIGHT_PADDING);
  }

  GUIHandler::GUIHandler(RenderWindow &window, Board board)
  {
    this->board = board;

    this->window = &window;

    loadSquareTextures();
    loadBoardSquares();

    loadPieceTextures();
    loadPieces();
    loadPromotionPieces();

    Image icon;
    icon.loadFromMemory(images.WHITE_KNIGHT, images.WHITE_KNIGHT_SIZE);
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
  }

  GUIHandler::GUIHandler(RenderWindow &window, std::string fen)
  {
    GUIHandler(window, Board(fen));
  }

  void GUIHandler::runMainLoop()
  {
    while (window->isOpen())
    {
      Event event;

      if (board.sideToMove == Piece::BLACK && !gameOver)
      {
        makeBotMove();

        while (window->pollEvent(event))
        {
          if (event.type == Event::Closed)
          {
            window->close();
          }
        }
      }

      while (window->pollEvent(event))
      {
        if (event.type == Event::Closed)
        {
          window->close();
        }

        if (event.type == Event::MouseButtonPressed)
        {
          if (event.mouseButton.button == Mouse::Left && !gameOver)
          {
            int index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

            if (board.sideToMove != board.board[index].getPieceColor())
              continue;

            if (awaitingPromotion)
            {
              int promotionPiece = getPromotionPiece(event.mouseButton.x, event.mouseButton.y);

              if (promotionPiece == -1)
                continue;

              promotionMove.promotionPiece = promotionPiece;

              draggingPieceIndex = -1;

              awaitingPromotion = false;

              makeMove(promotionMove);
            }
            else
            {
              if (board.sideToMove != board.board[index].getPieceColor())
                continue;

              draggingPieceIndex = index;

              grayHighlightsBitboard = board.getLegalPieceMovesBitboard(index);
            }
          }
        }

        if (event.type == Event::MouseButtonReleased)
        {
          if (event.mouseButton.button == Mouse::Left)
          {
            if (draggingPieceIndex == -1)
              continue;

            if (!(grayHighlightsBitboard.hasBit(GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y))))
            {
              draggingPieceIndex = -1;
              clearHighlights(GUIHandler::GRAY_HIGHLIGHT);
              continue;
            }

            int index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

            if (index != draggingPieceIndex)
            {
              Move move(draggingPieceIndex, index, board.board[draggingPieceIndex].piece, board.board[index].piece, board.enPassantFile, board.castlingRights);

              if (!(move.flags & Move::PROMOTION))
              {
                makeMove(move);
              }
              else
              {
                awaitingPromotion = true;
                promotionMove = move;
              }
            }

            draggingPieceIndex = -1;
          }
        }
      }

      window->clear();

      drawBoardSquares();

      if (awaitingPromotion)
      {
        drawPromotionPieces();
      }
      else
      {
        drawHighlights();
        drawPieces();
      }

      window->display();
    }
  }

  void GUIHandler::loadSquareTextures()
  {
    squares[0].loadFromMemory(images.WHITE_SQUARE, images.WHITE_SQUARE_SIZE);
    squares[1].loadFromMemory(images.BLACK_SQUARE, images.BLACK_SQUARE_SIZE);
    squares[YELLOW_HIGHLIGHT].loadFromMemory(images.YELLOW_HIGHLIGHT, images.YELLOW_HIGHLIGHT_SIZE);
    squares[RED_HIGHLIGHT].loadFromMemory(images.RED_HIGHLIGHT, images.RED_HIGHLIGHT_SIZE);
    squares[GRAY_HIGHLIGHT].loadFromMemory(images.GRAY_HIGHLIGHT, images.GRAY_HIGHLIGHT_SIZE);

    for (int i = 0; i < 5; i++)
    {
      squares[i].setRepeated(true);
    }
  }

  void GUIHandler::loadBoardSquares()
  {
    int squareIndex = 0;
    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        boardSquares[squareIndex].setTexture(squares[(i + j) % 2]);
        redHighlightsSprites[squareIndex].setTexture(squares[RED_HIGHLIGHT]);
        yellowHighlightsSprites[squareIndex].setTexture(squares[YELLOW_HIGHLIGHT]);
        grayHighlightsSprites[squareIndex].setTexture(squares[GRAY_HIGHLIGHT]);

        boardSquares[squareIndex].setPosition(getSquareCoordinates(j, i));
        redHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        yellowHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        grayHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));

        boardSquares[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
        redHighlightsSprites[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
        yellowHighlightsSprites[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
        grayHighlightsSprites[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);

        squareIndex++;
      }
    }
  }

  void GUIHandler::loadPieceTextures()
  {
    piecesTextures[Piece::WHITE_PAWN].loadFromMemory(images.WHITE_PAWN, images.WHITE_PAWN_SIZE);
    piecesTextures[Piece::WHITE_KNIGHT].loadFromMemory(images.WHITE_KNIGHT, images.WHITE_KNIGHT_SIZE);
    piecesTextures[Piece::WHITE_BISHOP].loadFromMemory(images.WHITE_BISHOP, images.WHITE_BISHOP_SIZE);
    piecesTextures[Piece::WHITE_ROOK].loadFromMemory(images.WHITE_ROOK, images.WHITE_ROOK_SIZE);
    piecesTextures[Piece::WHITE_QUEEN].loadFromMemory(images.WHITE_QUEEN, images.WHITE_QUEEN_SIZE);
    piecesTextures[Piece::WHITE_KING].loadFromMemory(images.WHITE_KING, images.WHITE_KING_SIZE);

    piecesTextures[Piece::BLACK_PAWN].loadFromMemory(images.BLACK_PAWN, images.BLACK_PAWN_SIZE);
    piecesTextures[Piece::BLACK_KNIGHT].loadFromMemory(images.BLACK_KNIGHT, images.BLACK_KNIGHT_SIZE);
    piecesTextures[Piece::BLACK_BISHOP].loadFromMemory(images.BLACK_BISHOP, images.BLACK_BISHOP_SIZE);
    piecesTextures[Piece::BLACK_ROOK].loadFromMemory(images.BLACK_ROOK, images.BLACK_ROOK_SIZE);
    piecesTextures[Piece::BLACK_QUEEN].loadFromMemory(images.BLACK_QUEEN, images.BLACK_QUEEN_SIZE);
    piecesTextures[Piece::BLACK_KING].loadFromMemory(images.BLACK_KING, images.BLACK_KING_SIZE);

    for (int i = 0; i < (Piece::BLACK | Piece::KING) + 1; i++)
    {
      piecesTextures[i].setSmooth(true);
    }
  }

  void GUIHandler::loadPieces()
  {
    for (int i = 0; i < 64; i++)
    {
      whitePawns[i].setTexture(piecesTextures[Piece::WHITE_PAWN]);
      whiteKnights[i].setTexture(piecesTextures[Piece::WHITE_KNIGHT]);
      whiteBishops[i].setTexture(piecesTextures[Piece::WHITE_BISHOP]);
      whiteRooks[i].setTexture(piecesTextures[Piece::WHITE_ROOK]);
      whiteQueens[i].setTexture(piecesTextures[Piece::WHITE_QUEEN]);
      whiteKings[i].setTexture(piecesTextures[Piece::WHITE_KING]);

      blackPawns[i].setTexture(piecesTextures[Piece::BLACK_PAWN]);
      blackKnights[i].setTexture(piecesTextures[Piece::BLACK_KNIGHT]);
      blackBishops[i].setTexture(piecesTextures[Piece::BLACK_BISHOP]);
      blackRooks[i].setTexture(piecesTextures[Piece::BLACK_ROOK]);
      blackQueens[i].setTexture(piecesTextures[Piece::BLACK_QUEEN]);
      blackKings[i].setTexture(piecesTextures[Piece::BLACK_KING]);

      whitePawns[i].setPosition(getSquareCoordinates(i));
      whiteKnights[i].setPosition(getSquareCoordinates(i));
      whiteBishops[i].setPosition(getSquareCoordinates(i));
      whiteRooks[i].setPosition(getSquareCoordinates(i));
      whiteQueens[i].setPosition(getSquareCoordinates(i));
      whiteKings[i].setPosition(getSquareCoordinates(i));

      blackPawns[i].setPosition(getSquareCoordinates(i));
      blackKnights[i].setPosition(getSquareCoordinates(i));
      blackBishops[i].setPosition(getSquareCoordinates(i));
      blackRooks[i].setPosition(getSquareCoordinates(i));
      blackQueens[i].setPosition(getSquareCoordinates(i));
      blackKings[i].setPosition(getSquareCoordinates(i));

      whitePawns[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      whiteKnights[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      whiteBishops[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      whiteRooks[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      whiteQueens[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      whiteKings[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);

      blackPawns[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      blackKnights[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      blackBishops[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      blackRooks[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      blackQueens[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      blackKings[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);

      draggingPieceSprite.setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
    }
  }

  void GUIHandler::loadPromotionPieces()
  {
    whitePromotionPieces[0].setTexture(piecesTextures[Piece::WHITE_QUEEN]);
    whitePromotionPieces[1].setTexture(piecesTextures[Piece::WHITE_ROOK]);
    whitePromotionPieces[2].setTexture(piecesTextures[Piece::WHITE_BISHOP]);
    whitePromotionPieces[3].setTexture(piecesTextures[Piece::WHITE_KNIGHT]);

    blackPromotionPieces[0].setTexture(piecesTextures[Piece::BLACK_QUEEN]);
    blackPromotionPieces[1].setTexture(piecesTextures[Piece::BLACK_ROOK]);
    blackPromotionPieces[2].setTexture(piecesTextures[Piece::BLACK_BISHOP]);
    blackPromotionPieces[3].setTexture(piecesTextures[Piece::BLACK_KNIGHT]);

    whitePromotionPieces[0].setPosition(getSquareCoordinates(2, 1));
    whitePromotionPieces[1].setPosition(getSquareCoordinates(3, 1));
    whitePromotionPieces[2].setPosition(getSquareCoordinates(4, 1));
    whitePromotionPieces[3].setPosition(getSquareCoordinates(5, 1));

    blackPromotionPieces[0].setPosition(getSquareCoordinates(2, 6));
    blackPromotionPieces[1].setPosition(getSquareCoordinates(3, 6));
    blackPromotionPieces[2].setPosition(getSquareCoordinates(4, 6));
    blackPromotionPieces[3].setPosition(getSquareCoordinates(5, 6));

    for (int i = 0; i < 4; i++)
    {
      whitePromotionPieces[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      blackPromotionPieces[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
    }
  }

  void GUIHandler::drawBoardSquares()
  {
    for (int i = 0; i < 64; i++)
    {
      window->draw(boardSquares[i]);
    }
  }

  void GUIHandler::drawPieces()
  {
    for (int i = 0; i < 64; i++)
    {
      if (draggingPieceIndex == i || (awaitingPromotion && promotionMove.from == i))
      {
        continue;
      }

      if (board.whitePawnsBitboard.hasBit(i))
      {
        window->draw(whitePawns[i]);
      }
      else if (board.whiteKnightsBitboard.hasBit(i))
      {
        window->draw(whiteKnights[i]);
      }
      else if (board.whiteBishopsBitboard.hasBit(i))
      {
        window->draw(whiteBishops[i]);
      }
      else if (board.whiteRooksBitboard.hasBit(i))
      {
        window->draw(whiteRooks[i]);
      }
      else if (board.whiteQueensBitboard.hasBit(i))
      {
        window->draw(whiteQueens[i]);
      }
      else if (board.whiteKingIndex == i)
      {
        window->draw(whiteKings[i]);
      }

      else if (board.blackPawnsBitboard.hasBit(i))
      {
        window->draw(blackPawns[i]);
      }
      else if (board.blackKnightsBitboard.hasBit(i))
      {
        window->draw(blackKnights[i]);
      }
      else if (board.blackBishopsBitboard.hasBit(i))
      {
        window->draw(blackBishops[i]);
      }
      else if (board.blackRooksBitboard.hasBit(i))
      {
        window->draw(blackRooks[i]);
      }
      else if (board.blackQueensBitboard.hasBit(i))
      {
        window->draw(blackQueens[i]);
      }
      else if (board.blackKingIndex == i)
      {
        window->draw(blackKings[i]);
      }
    }

    if (draggingPieceIndex != -1)
    {
      draggingPieceSprite.setTexture(piecesTextures[board.board[draggingPieceIndex].piece]);
      draggingPieceSprite.setPosition(Mouse::getPosition(*window).x - SQUARE_SIZE / 2, Mouse::getPosition(*window).y - SQUARE_SIZE / 2);
      window->draw(draggingPieceSprite);
    }
  }

  void GUIHandler::drawHighlights()
  {
    for (int i = 0; i < 64; i++)
    {
      if (redHighlightsBitboard.hasBit(i))
      {
        window->draw(redHighlightsSprites[i]);
      }
      else if (grayHighlightsBitboard.hasBit(i))
      {
        window->draw(grayHighlightsSprites[i]);
      }
      else if (yellowHighlightsBitboard.hasBit(i))
      {
        window->draw(yellowHighlightsSprites[i]);
      }
    }
  }

  void GUIHandler::drawPromotionPieces()
  {
    for (int i = 0; i < 4; i++)
    {
      if (board.sideToMove == Piece::WHITE)
      {
        window->draw(whitePromotionPieces[i]);
      }
      else
      {
        window->draw(blackPromotionPieces[i]);
      }
    }
  }

  void GUIHandler::clearHighlights()
  {
    redHighlightsBitboard.bitboard = 0;
    yellowHighlightsBitboard.bitboard = 0;
    grayHighlightsBitboard.bitboard = 0;
  }

  void GUIHandler::clearHighlights(int highlight)
  {
    if (highlight == RED_HIGHLIGHT)
    {
      redHighlightsBitboard.bitboard = 0;
    }
    else if (highlight == YELLOW_HIGHLIGHT)
    {
      yellowHighlightsBitboard.bitboard = 0;
    }
    else if (highlight == GRAY_HIGHLIGHT)
    {
      grayHighlightsBitboard.bitboard = 0;
    }
  }

  void GUIHandler::makeMove(Move move)
  {
    if (move.from == move.to)
      return;

    board.makeMove(move);

    clearHighlights();

    if (board.isInCheck(board.sideToMove))
    {
      redHighlightsBitboard.addBit(board.sideToMove == Piece::WHITE ? board.whiteKingIndex : board.blackKingIndex);
    }

    int gameStatus = board.getGameStatus(board.sideToMove);

    if (gameStatus)
    {
      gameOver = true;

      if (gameStatus == Board::LOSE)
      {
        std::cout << "Checkmate" << std::endl;
      }
      else
      {
        std::cout << "Stalemate" << std::endl;
      }
    }

    yellowHighlightsBitboard.addBit(move.from);
    yellowHighlightsBitboard.addBit(move.to);
  }

  void GUIHandler::makeBotMove()
  {
    Move move = board.generateBotMove();

    makeMove(move);
  }
}