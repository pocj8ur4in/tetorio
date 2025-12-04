#include "game/Piece.h"
#include "game/Board.h"

namespace game {

/**
 * Shape definitions for all pieces in all rotations,
 * which each shape is a 4x4 grid, true = filled cell
 * where coordinates are shape[row][col], row 0 is top
 */

const Piece::Shape Piece::SHAPES[7][4] = {
    // I piece: index 0, CellType::I = 1
    {// R0: Horizontal at row 1
     {{{false, false, false, false},
       {true, true, true, true},
       {false, false, false, false},
       {false, false, false, false}}},
     // R1: Vertical at col 2
     {{{false, false, true, false},
       {false, false, true, false},
       {false, false, true, false},
       {false, false, true, false}}},
     // R2: Horizontal at row 2
     {{{false, false, false, false},
       {false, false, false, false},
       {true, true, true, true},
       {false, false, false, false}}},
     // R3: Vertical at col 1
     {{{false, true, false, false},
       {false, true, false, false},
       {false, true, false, false},
       {false, true, false, false}}}},
    // O piece: index 1, CellType::O = 2
    {// all rotations are same; 2x2 at top-center
     {{{false, true, true, false},
       {false, true, true, false},
       {false, false, false, false},
       {false, false, false, false}}},
     {{{false, true, true, false},
       {false, true, true, false},
       {false, false, false, false},
       {false, false, false, false}}},
     {{{false, true, true, false},
       {false, true, true, false},
       {false, false, false, false},
       {false, false, false, false}}},
     {{{false, true, true, false},
       {false, true, true, false},
       {false, false, false, false},
       {false, false, false, false}}}},
    // T piece: index 2, CellType::T = 3
    {// R0: T pointing up
     {{{false, true, false, false},
       {true, true, true, false},
       {false, false, false, false},
       {false, false, false, false}}},
     // R1: T pointing right
     {{{false, true, false, false},
       {false, true, true, false},
       {false, true, false, false},
       {false, false, false, false}}},
     // R2: T pointing down
     {{{false, false, false, false},
       {true, true, true, false},
       {false, true, false, false},
       {false, false, false, false}}},
     // R3: T pointing left
     {{{false, true, false, false},
       {true, true, false, false},
       {false, true, false, false},
       {false, false, false, false}}}},
    // S piece: index 3, CellType::S = 4
    {// R0: S pointing up
     {{{false, true, true, false},
       {true, true, false, false},
       {false, false, false, false},
       {false, false, false, false}}},
     // R1: S pointing right
     {{{false, true, false, false},
       {false, true, true, false},
       {false, false, true, false},
       {false, false, false, false}}},
     // R2: S pointing down
     {{{false, false, false, false},
       {false, true, true, false},
       {true, true, false, false},
       {false, false, false, false}}},
     // R3: S pointing left
     {{{true, false, false, false},
       {true, true, false, false},
       {false, true, false, false},
       {false, false, false, false}}}},
    // Z piece: index 4, CellType::Z = 5
    {// R0: Z pointing up
     {{{true, true, false, false},
       {false, true, true, false},
       {false, false, false, false},
       {false, false, false, false}}},
     // R1: Z pointing right
     {{{false, false, true, false},
       {false, true, true, false},
       {false, true, false, false},
       {false, false, false, false}}},
     // R2: Z pointing down
     {{{false, false, false, false},
       {true, true, false, false},
       {false, true, true, false},
       {false, false, false, false}}},
     // R3: Z pointing left
     {{{false, true, false, false},
       {true, true, false, false},
       {true, false, false, false},
       {false, false, false, false}}}},
    // J piece: index 5, CellType::J = 6
    {// R0: J pointing up
     {{{true, false, false, false},
       {true, true, true, false},
       {false, false, false, false},
       {false, false, false, false}}},
     // R1: J pointing right
     {{{false, true, true, false},
       {false, true, false, false},
       {false, true, false, false},
       {false, false, false, false}}},
     // R2: J pointing down
     {{{false, false, false, false},
       {true, true, true, false},
       {false, false, true, false},
       {false, false, false, false}}},
     // R3: J pointing left
     {{{false, true, false, false},
       {false, true, false, false},
       {true, true, false, false},
       {false, false, false, false}}}},
    // L piece: index 6, CellType::L = 7
    {// R0: L pointing up
     {{{false, false, true, false},
       {true, true, true, false},
       {false, false, false, false},
       {false, false, false, false}}},
     // R1: L pointing right
     {{{false, true, false, false},
       {false, true, false, false},
       {false, true, true, false},
       {false, false, false, false}}},
     // R2: L pointing down
     {{{false, false, false, false},
       {true, true, true, false},
       {true, false, false, false},
       {false, false, false, false}}},
     // R3: L pointing left
     {{{true, true, false, false},
       {false, true, false, false},
       {false, true, false, false},
       {false, false, false, false}}}}};

/**
 * KickOffsets matches SRS Wall Kick data for J, L, S, T, Z pieces,
 * which format is {dx, dy} where positive y is up
 * and index corresponds to rotation state before rotation
 */
const KickOffset Piece::JLSTZ_KICKS[4][5] = {
    // 0->1: R0 to R1, CW
    {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
    // 1->2: R1 to R2, CW
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
    // 2->3: R2 to R3, CW
    {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
    // 3->0: R3 to R0, CW
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}};

// Wall kicks for I piece which are different from other pieces
const KickOffset Piece::I_KICKS[4][5] = {
    // 0->1: R0 to R1, CW
    {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},
    // 1->2: R1 to R2, CW
    {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},
    // 2->3: R2 to R3, CW
    {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},
    // 3->0: R3 to R0, CW
    {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}}};

// 180 rotation kick data for JLSTZ pieces
const KickOffset Piece::JLSTZ_180_KICKS[4][KICK_180_COUNT] = {
    // 0->2: R0 to R2, 180
    {{0, 0}, {0, 1}, {1, 1}, {-1, 1}, {1, 0}, {-1, 0}},
    // 1->3: R1 to R3, 180
    {{0, 0}, {1, 0}, {1, 2}, {1, 1}, {0, 2}, {0, 1}},
    // 2->0: R2 to R0, 180
    {{0, 0}, {0, -1}, {-1, -1}, {1, -1}, {-1, 0}, {1, 0}},
    // 3->1: R3 to R1, 180
    {{0, 0}, {-1, 0}, {-1, 2}, {-1, 1}, {0, 2}, {0, 1}}};

// 180 rotation kick data for I piece
const KickOffset Piece::I_180_KICKS[4][KICK_180_COUNT] = {
    // 0->2: R0 to R2, 180
    {{0, 0}, {0, 1}, {0, 2}, {1, 0}, {-1, 0}, {1, 1}},
    // 1->3: R1 to R3, 180
    {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, -1}, {1, 1}},
    // 2->0: R2 to R0, 180
    {{0, 0}, {0, -1}, {0, -2}, {-1, 0}, {1, 0}, {-1, -1}},
    // 3->1: R3 to R1, 180
    {{0, 0}, {-1, 0}, {-2, 0}, {0, 1}, {0, -1}, {-1, 1}}};

Piece::Piece(CellType type)
    : type_(type), x_(0), y_(0), rotation_(Rotation::R0) {
  if (type != CellType::EMPTY) {
    getSpawnPosition(type, x_, y_);
  }
}

const Piece::Shape &Piece::getShape() const {
  return getShape(type_, rotation_);
}

const Piece::Shape &Piece::getShape(CellType type, Rotation rotation) {
  static const Shape EMPTY_SHAPE = {{{false, false, false, false},
                                     {false, false, false, false},
                                     {false, false, false, false},
                                     {false, false, false, false}}};

  if (type == CellType::EMPTY) {
    return EMPTY_SHAPE;
  }

  int typeIdx = static_cast<int>(type) - 1; // 1-7 -> 0-6
  int rotIdx = static_cast<int>(rotation);
  return SHAPES[typeIdx][rotIdx];
}

void Piece::getSpawnPosition(CellType type, int &x, int &y) {
  x = 3;
  y = 20;

  // I and O pieces spawn higher due to their shape
  if (type == CellType::I) {
    y = 19;
  } else if (type == CellType::O) {
    y = 20;
  }
}

std::array<KickOffset, 5> Piece::getWallKicks(CellType type, Rotation from,
                                              Rotation to) {
  std::array<KickOffset, 5> kicks = {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}};

  if (type == CellType::EMPTY || type == CellType::O) {
    // O piece doesn't need kicks, just return zero offsets
    return kicks;
  }

  int fromIdx = static_cast<int>(from);
  int toIdx = static_cast<int>(to);

  // determine if this is CW or CCW rotation
  bool isCW = ((fromIdx + 1) % 4) == toIdx;
  bool isCCW = ((fromIdx + 3) % 4) == toIdx;

  const KickOffset(*kickTable)[5] =
      (type == CellType::I) ? I_KICKS : JLSTZ_KICKS;

  if (isCW) {
    // use kicks from 'from' rotation state
    for (int i = 0; i < 5; ++i) {
      kicks[i] = kickTable[fromIdx][i];
    }
  } else if (isCCW) {
    // CCW is reverse of CW from 'to' rotation state, negate offsets
    for (int i = 0; i < 5; ++i) {
      kicks[i].dx = -kickTable[toIdx][i].dx;
      kicks[i].dy = -kickTable[toIdx][i].dy;
    }
  }

  return kicks;
}

std::array<KickOffset, KICK_180_COUNT> Piece::getWallKicks180(CellType type,
                                                              Rotation from) {
  std::array<KickOffset, KICK_180_COUNT> kicks = {
      {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}};

  if (type == CellType::EMPTY || type == CellType::O) {
    return kicks;
  }

  int fromIdx = static_cast<int>(from);

  const KickOffset(*kickTable)[KICK_180_COUNT] =
      (type == CellType::I) ? I_180_KICKS : JLSTZ_180_KICKS;

  for (int i = 0; i < KICK_180_COUNT; ++i) {
    kicks[i] = kickTable[fromIdx][i];
  }

  return kicks;
}

Rotation Piece::rotateCW() const {
  return static_cast<Rotation>((static_cast<int>(rotation_) + 1) % 4);
}

Rotation Piece::rotateCCW() const {
  return static_cast<Rotation>((static_cast<int>(rotation_) + 3) % 4);
}

Rotation Piece::rotate180() const {
  return static_cast<Rotation>((static_cast<int>(rotation_) + 2) % 4);
}

void Piece::reset(CellType type) {
  type_ = type;
  rotation_ = Rotation::R0;
  if (type != CellType::EMPTY) {
    getSpawnPosition(type, x_, y_);
  } else {
    x_ = 0;
    y_ = 0;
  }
}

} // namespace game
