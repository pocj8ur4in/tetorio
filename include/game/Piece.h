#ifndef TETORIO_GAME_PIECE_H
#define TETORIO_GAME_PIECE_H

#include "Board.h"

#include <array>
#include <cstdint>

namespace game {

/**
 * Rotation states matches SRS standard.
 */
enum class Rotation : uint8_t {
  R0 = 0, // Spawn state
  R1 = 1, // CW from spawn
  R2 = 2, // 180 from spawn
  R3 = 3  // CCW from spawn
};

/**
 * Kickoffset matches kick offsets for SRS wall kicks.
 */
struct KickOffset {
  int dx;
  int dy;
};

// offset count of kick tests for 180 rotation
constexpr int KICK_180_COUNT = 6;

/**
 * Piece represents a piece with position and rotation.
 */
class Piece {
public:
  // 4x4 shape matrix
  using Shape = std::array<std::array<bool, 4>, 4>;

  /**
   * create a piece of given type at spawn position
   * @param type piece type
   */
  explicit Piece(CellType type = CellType::EMPTY);

  /**
   * default destructor
   */
  ~Piece() = default;

  /**
   * get the CellType of the piece
   * @return type of the piece
   */
  CellType getType() const { return type_; }

  /**
   * get the x position of the piece
   * @return x coordinate of the piece
   */
  int getX() const { return x_; }

  /**
   * get the y position of the piece
   * @return y coordinate of the piece
   */
  int getY() const { return y_; }

  /**
   * get the rotation of the piece
   * @return rotation of the piece
   */
  Rotation getRotation() const { return rotation_; }

  /**
   * get the shape for current rotation
   * @return reference to shape array
   */
  const Shape &getShape() const;

  /**
   * get the shape for a specific rotation
   * @param type piece type
   * @param rotation rotation state
   * @return reference to shape array
   */
  static const Shape &getShape(CellType type, Rotation rotation);

  /**
   * get spawn position for a piece type
   * @param type piece type
   * @param x output x coordinate
   * @param y output y coordinate
   */
  static void getSpawnPosition(CellType type, int &x, int &y);

  /**
   * get wall kick offsets for rotation from -> to
   * which returns array of 5 kick offsets to try (CW/CCW)
   * @param type piece type
   * @param from starting rotation
   * @param to target rotation
   * @return array of 5 kick offsets
   */
  static std::array<KickOffset, 5> getWallKicks(CellType type, Rotation from,
                                                Rotation to);

  /**
   * get wall kick offsets for 180 rotation
   * which returns array of 6 kick offsets to try
   * @param type piece type
   * @param from starting rotation
   * @return array of 6 kick offsets
   */
  static std::array<KickOffset, KICK_180_COUNT> getWallKicks180(CellType type,
                                                                Rotation from);

  /**
   * set the x and y of the piece
   * @param x x position
   * @param y y position
   */
  void setPosition(int x, int y) {
    x_ = x;
    y_ = y;
  }

  /**
   * set the rotation of the piece
   * @param r rotation
   */
  void setRotation(Rotation r) { rotation_ = r; }

  /**
   * move piece by offset
   * @param dx x offset
   * @param dy y offset
   */
  void move(int dx, int dy) {
    x_ += dx;
    y_ += dy;
  }

  /**
   * rotate clockwise which returns new rotation state
   * @return new rotation state after CW rotation
   */
  Rotation rotateCW() const;

  /**
   * rotate counter-clockwise which returns new rotation state
   * @return new rotation state after CCW rotation
   */
  Rotation rotateCCW() const;

  /**
   * rotate 180 degrees which returns new rotation state
   * @return new rotation state after 180 rotation
   */
  Rotation rotate180() const;

  /**
   * check if piece is valid
   * @return true if piece type is not NONE
   */
  bool isValid() const { return type_ != CellType::EMPTY; }

  /**
   * reset piece to spawn state
   * @param type new piece type
   */
  void reset(CellType type);

private:
  CellType type_;
  int x_;
  int y_;
  Rotation rotation_;

  // shape data for all pieces and rotations [7 types][4 rotations]
  static const Shape SHAPES[7][4];

  // wall kick data for J, L, S, T, Z pieces [4 rotations][5 kicks]
  static const KickOffset JLSTZ_KICKS[4][5];

  // wall kick data for I piece [4 rotations][5 kicks]
  static const KickOffset I_KICKS[4][5];

  // 180 rotation kick data for JLSTZ pieces [4 rotations][6 kicks]
  static const KickOffset JLSTZ_180_KICKS[4][KICK_180_COUNT];

  // 180 rotation kick data for I piece [4 rotations][6 kicks]
  static const KickOffset I_180_KICKS[4][KICK_180_COUNT];
};

} // namespace game

#endif // TETORIO_GAME_PIECE_H
