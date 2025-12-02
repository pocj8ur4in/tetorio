#include <cstddef>
#include <cstdint>
#include <vector>

namespace network {

/**
 * ClientBuffer store client send buffer.
 */
struct ClientBuffer {
  std::vector<uint8_t> data; // send buffer data
  size_t offset = 0;         // current send offset
  bool wantWrite = false;    // whether EPOLLOUT is registered

  /**
   * append data to send buffer
   * @param buf pointer to data to append
   * @param len length of data to append
   */
  void append(const uint8_t *buf, size_t len) {
    data.insert(data.end(), buf, buf + len);
  }

  /**
   * compact buffer to remove sent data
   */
  void compact() {
    if (offset > 0) {
      data.erase(data.begin(), data.begin() + static_cast<ptrdiff_t>(offset));
      offset = 0;
    }
  }

  /**
   * get remaining data size
   * @return remaining data size
   */
  size_t remaining() const { return data.size() - offset; }

  /**
   * get current data pointer to send
   * @return current data pointer
   */
  const uint8_t *current() const { return data.data() + offset; }
  bool empty() const { return offset >= data.size(); }
};

} // namespace network