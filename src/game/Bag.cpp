#include "game/Bag.h"
#include "game/Board.h"

#include <algorithm>
#include <chrono>

namespace game {

Bag::Bag(uint64_t seed) : pieceCount_(0) {
  if (seed == 0) {
    // generate random seed from high-resolution clock
    seed = static_cast<uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
  }
  reset(seed);
}

void Bag::reset(uint64_t seed) {
  seed_ = seed;
  rng_.seed(seed_);
  queue_.clear();
  pieceCount_ = 0;

  // prefill queue with enough pieces for preview
  ensureQueue();
}

uint64_t Bag::getSeed() const { return seed_; }

uint8_t Bag::next() {
  ensureQueue();

  uint8_t piece = queue_.front();
  queue_.pop_front();
  ++pieceCount_;

  return piece;
}

uint8_t Bag::peek(int index) const {
  if (index < 0 || static_cast<size_t>(index) >= queue_.size()) {
    return 0;
  }
  return queue_[static_cast<size_t>(index)];
}

std::array<uint8_t, PREVIEW_SIZE> Bag::getPreview() const {
  std::array<uint8_t, PREVIEW_SIZE> preview{};
  for (int i = 0; i < PREVIEW_SIZE; ++i) {
    preview[static_cast<size_t>(i)] = peek(i);
  }
  return preview;
}

uint32_t Bag::getPieceCount() const { return pieceCount_; }

void Bag::generateBag() {
  // create a bag with all 7 pieces
  std::array<uint8_t, PIECE_COUNT> bag = {CELL_I, CELL_O, CELL_T, CELL_S,
                                          CELL_Z, CELL_J, CELL_L};

  // shuffle the bag
  for (int i = PIECE_COUNT - 1; i > 0; --i) {
    std::uniform_int_distribution<int> dist(0, i);
    int j = dist(rng_);
    std::swap(bag[static_cast<size_t>(i)], bag[static_cast<size_t>(j)]);
  }

  // add shuffled pieces to queue
  for (uint8_t piece : bag) {
    queue_.push_back(piece);
  }
}

void Bag::ensureQueue() {
  // ensure enough pieces for preview + current piece
  while (queue_.size() < static_cast<size_t>(PREVIEW_SIZE + 1)) {
    generateBag();
  }
}

} // namespace game
