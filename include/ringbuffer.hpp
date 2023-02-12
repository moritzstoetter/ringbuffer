#include <algorithm>
#include <array>
#include <exception>
#include <mutex> 
#include <optional>
#include <ranges> 
#include <span>
#include <string>
#include <utility>

#include "rotate_view.hpp"

template <typename T, size_t Size>
class Ringbuffer
{
public:
    inline bool empty() const { return _count == 0; }
    inline bool full() const { return _count == Size; }
    inline size_t capacity() const { return Size - _count; }
    inline size_t count() const { return _count; }
    inline size_t size() const { return Size; }

    void clear() 
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _readHead = _writeHead;
        _count = 0;
    }

    /*
     * single value write
     */
    void write(T &&val)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (++_count > Size)
        {
            _count = Size;
            throw std::runtime_error("Ringbuffer is full!");
        }
        *_writeHead = std::move(val);
        advance(_writeHead);
    }
    

    /*
     * bulk write 
     */
    template <std::ranges::input_range InRange>
    void write(InRange &&inRange)
    {
        for (T val : inRange)
        {
            this->write(std::move(val));
        }
    }

    /* 
     * bulk value read 
     */
    [[nodiscard]] auto read() 
    {
        return read(_count);
    }

    [[nodiscard]] auto read(size_t n)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto ret = _buffer 
            | std::views::all 
            | views::rotate(std::distance(_buffer.cbegin(), _readHead)) 
            | std::views::take(std::min(n, _count));

        _readHead = _writeHead;
        _count = 0; 

        return ret;
    }

    T& peek() const
    {
        return _readHead;
    }

    void print() const
    {
        std::cout << "Size: " << size() << ", Count: " << count() << std::endl;
        for (auto it = _buffer.cbegin(); it != _buffer.cend(); ++it) {
            printf("%5s 0x%02x %-5s\n", it == _readHead ? " r ->" : "", *it, it == _writeHead ? "<- w" : "");
        }
    }

    template<std::ranges::input_range InRange>
    auto operator<<(InRange& rhs) {
        this->write(rhs);
    }

private:
    std::mutex _mutex;
    std::array<T, Size> _buffer;
    size_t _count{0};
    typename std::array<T, Size>::iterator _writeHead{_buffer.begin()};
    typename std::array<T, Size>::const_iterator _readHead{_buffer.begin()};
    
    inline void advance(typename std::array<T, Size>::iterator &it) 
    {
        if (++it == _buffer.end()) {
            it = _buffer.begin();
        }
    }
};