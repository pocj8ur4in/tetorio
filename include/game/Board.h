#ifndef TETORIO_GAME_BOARD_H
#define TETORIO_GAME_BOARD_H

#include <array>
#include <cstdint>

namespace game {

// board dimensions
constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 20;
constexpr int BOARD_BUFFER = 4;

// cell values
constexpr uint8_t CELL_EMPTY = 0;
constexpr uint8_t CELL_I = 1;
constexpr uint8_t CELL_O = 2;
constexpr uint8_t CELL_T = 3;
constexpr uint8_t CELL_S = 4;
constexpr uint8_t CELL_Z = 5;
constexpr uint8_t CELL_J = 6;
constexpr uint8_t CELL_L = 7;
constexpr uint8_t CELL_GARBAGE = 8;

/**
 * Board represents board for game.
 */
class Board {
public:
  /**
   * constructor
   */
  Board();

  /**
   * destructor
   */
  ~Board() = default;

  /**
   * set all cells of the board to empty
   */
  void clear();

  /**
   * get cell value at position
   * @param x x coordinate (0-9)
   * @param y y coordinate (0-23, 0-19 visible, 20-23 buffer)
   * @return cell value, CELL_EMPTY if out of bounds
   */
  uint8_t getCell(int x, int y) const;

  /**
   * set cell value at position
   * @param x x coordinate (0-9)
   * @param y y coordinate (0-23)
   * @param value cell value to set
   * @return true if successful, false if out of bounds
   */
  bool setCell(int x, int y, uint8_t value);

  /**
   * check if position is within bounds
   * @param x x coordinate
   * @param y y coordinate
   * @return true if within bounds, false otherwise
   */
  bool isInBounds(int x, int y) const;

  /**
   * check if position is empty
x   * @param x x coordinate
   * @param y y coordinate
   * @return true if empty or out of bounds, false if occupied
   */
  bool isEmpty(int x, int y) const;

  /**
   * check if a row is completely filled
   * @param y y coordinate of the row
   * @return true if row is full, false otherwise
   */
  bool isRowFull(int y) const;

  /**
   * check if a row is completely empty
   * @param y y coordinate of the row
   * @return true if row is empty, false otherwise
   */
  bool isRowEmpty(int y) const;

  /**
   * clear a row and shift rows above down
   * @param y y coordinate of the row to clear
   */
  void clearRow(int y);

  /**
   * clear all full rows and return the number of cleared rows
   * @return number of cleared rows (0-4)
   */
  int clearFullRows();

  /**
   * add garbage lines from the bottom
   * @param lines number of garbage lines to add
   * @param holeColumn column index for the hole (0-9)
   * @return true if successful, false if would cause game over
   */
  bool addGarbageLines(int lines, int holeColumn);

  /**
   * check if the board has any blocks above the visible area
   * @return true if blocks exist above visible area, false otherwise
   */
  bool hasBlocksAboveVisible() const;

  /**
   * get the height of the highest block in a column
   * @param x column index (0-9)
   * @return height of highest block (0 if empty)
   */
  int getColumnHeight(int x) const;

  /**
   * get the overall height of the board (highest block)
   * @return height of the board
   */
  int getBoardHeight() const;

  /**
   * get raw grid data for serialization
   * @return reference to the grid array
   */
  const std::array<std::array<uint8_t, BOARD_WIDTH>,
                   BOARD_HEIGHT + BOARD_BUFFER> &
  getGrid() const {
    return grid_;
  }

private:
  // grid[y][x]: y=0 is bottom, y=23 is top (buffer)
  std::array<std::array<uint8_t, BOARD_WIDTH>, BOARD_HEIGHT + BOARD_BUFFER>
      grid_;
};

} // namespace game

#endif // TETORIO_GAME_BOARD_H
