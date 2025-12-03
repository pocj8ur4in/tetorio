#include "game/Board.h"

#include <algorithm>

namespace game {

Board::Board() { clear(); }

void Board::clear() {
  for (auto &row : grid_) {
    row.fill(CELL_EMPTY);
  }
}

uint8_t Board::getCell(int x, int y) const {
  if (!isInBounds(x, y)) {
    return CELL_EMPTY;
  }
  return grid_[y][x];
}

bool Board::setCell(int x, int y, uint8_t value) {
  if (!isInBounds(x, y)) {
    return false;
  }
  grid_[y][x] = value;
  return true;
}

bool Board::isInBounds(int x, int y) const {
  return x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT + BOARD_BUFFER;
}

bool Board::isEmpty(int x, int y) const { return getCell(x, y) == CELL_EMPTY; }

bool Board::isRowFull(int y) const {
  if (y < 0 || y >= BOARD_HEIGHT + BOARD_BUFFER) {
    return false;
  }

  for (int x = 0; x < BOARD_WIDTH; ++x) {
    if (grid_[y][x] == CELL_EMPTY) {
      return false;
    }
  }
  return true;
}

bool Board::isRowEmpty(int y) const {
  if (y < 0 || y >= BOARD_HEIGHT + BOARD_BUFFER) {
    return true;
  }

  for (int x = 0; x < BOARD_WIDTH; ++x) {
    if (grid_[y][x] != CELL_EMPTY) {
      return false;
    }
  }
  return true;
}

void Board::clearRow(int y) {
  if (y < 0 || y >= BOARD_HEIGHT + BOARD_BUFFER) {
    return;
  }

  // shift all rows above down by one
  for (int row = y; row < BOARD_HEIGHT + BOARD_BUFFER - 1; ++row) {
    grid_[row] = grid_[row + 1];
  }

  // clear the top row
  grid_[BOARD_HEIGHT + BOARD_BUFFER - 1].fill(CELL_EMPTY);
}

int Board::clearFullRows() {
  int clearedCount = 0;

  // check from bottom to top
  for (int y = 0; y < BOARD_HEIGHT + BOARD_BUFFER; ++y) {
    if (isRowFull(y)) {
      clearRow(y);
      ++clearedCount;
      --y;
    }
  }

  return clearedCount;
}

bool Board::addGarbageLines(int lines, int holeColumn) {
  if (lines <= 0 || holeColumn < 0 || holeColumn >= BOARD_WIDTH) {
    return false;
  }

  // check if adding garbage would cause game over
  for (int y = BOARD_HEIGHT + BOARD_BUFFER - lines;
       y < BOARD_HEIGHT + BOARD_BUFFER; ++y) {
    if (!isRowEmpty(y)) {
      return false; // would cause game over
    }
  }

  // shift all rows up by lines
  for (int y = BOARD_HEIGHT + BOARD_BUFFER - 1; y >= lines; --y) {
    grid_[y] = grid_[y - lines];
  }

  // add garbage lines at the bottom
  for (int y = 0; y < lines; ++y) {
    for (int x = 0; x < BOARD_WIDTH; ++x) {
      grid_[y][x] = (x == holeColumn) ? CELL_EMPTY : CELL_GARBAGE;
    }
  }

  return true;
}

bool Board::hasBlocksAboveVisible() const {
  for (int y = BOARD_HEIGHT; y < BOARD_HEIGHT + BOARD_BUFFER; ++y) {
    if (!isRowEmpty(y)) {
      return true;
    }
  }
  return false;
}

int Board::getColumnHeight(int x) const {
  if (x < 0 || x >= BOARD_WIDTH) {
    return 0;
  }

  for (int y = BOARD_HEIGHT + BOARD_BUFFER - 1; y >= 0; --y) {
    if (grid_[y][x] != CELL_EMPTY) {
      return y + 1;
    }
  }
  return 0;
}

int Board::getBoardHeight() const {
  int maxHeight = 0;
  for (int x = 0; x < BOARD_WIDTH; ++x) {
    maxHeight = std::max(maxHeight, getColumnHeight(x));
  }
  return maxHeight;
}

} // namespace game
