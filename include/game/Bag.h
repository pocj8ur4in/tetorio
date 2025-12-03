#ifndef TETORIO_GAME_BAG_H
#define TETORIO_GAME_BAG_H

#include <array>
#include <cstdint>
#include <deque>
#include <random>

namespace game {

// piece types
constexpr int PIECE_COUNT = 7;
constexpr int PREVIEW_SIZE = 5;

/**
 * Bag is 7-Bag randomizer system on game.
 * each player should have their own Bag with a unique seed.
 */
class Bag {
public:
  /**
   * constructor
   * @param seed random seed
   */
  explicit Bag(uint64_t seed = 0);

  /**
   * destructor
   */
  ~Bag() = default;

  /**
   * reset the bag with new seed
   * @param seed new random seed
   */
  void reset(uint64_t seed);

  /**
   * get the current seed
   * @return current seed
   */
  uint64_t getSeed() const;

  /**
   * get the next piece and advance the queue
   * @return next piece of cell value (CELL_I ~ CELL_L)
   */
  uint8_t next();

  /**
   * peek at upcoming pieces without consuming them
   * @param index 0 = next piece, 1 = second next, etc.
   * @return piece of cell value at index, or 0 if out of range
   */
  uint8_t peek(int index) const;

  /**
   * get preview of next N pieces
   * @param count number of pieces to preview
   * @return array of piece of cell values
   */
  std::array<uint8_t, PREVIEW_SIZE> getPreview() const;

  /**
   * get total number of pieces generated so far
   * @return count of piec
   */
  uint32_t getPieceCount() const;

private:
  /**
   * generate new shuffled bag and add to queue
   */
  void generateBag();

  /**
   * ensure queue has enough pieces for preview
   */
  void ensureQueue();

  uint64_t seed_;
  std::mt19937_64 rng_;
  std::deque<uint8_t> queue_;
  uint32_t pieceCount_;
};

} // namespace game

#endif // TETORIO_GAME_BAG_H

