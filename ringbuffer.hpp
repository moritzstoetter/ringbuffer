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
    inline bool empty() const
    {
        return _count == 0;
    }

    inline bool full() const
    {
        return _count == Size;
    }

    inline unsigned int count() const
    {   
        return _count;
    }

    inline int size() const
    {
        return _buffer.size();
    }

    /*
     * single value write
     */
    void write(T &&val)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (full())
        {
            throw std::runtime_error("Ringbuffer is full!");
        }

        *_writeHead = val;
        if (++_writeHead == _buffer.end())
        {
            _writeHead = _buffer.begin();
        }

        ++_count;
    }

    /*
     * bulk write 
     */
    template <size_t N>
    void write(std::span<T, N> &&vals)
    {
        for (T val : vals)
        {
            this->write(std::move(val));
        }
    }

    /* 
     * single value read
     */
    std::optional<T> read()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (empty())
        {
            std::cout << "Empty! \n";
            return std::nullopt;
        }

        auto ret = std::optional<T>(*_readHead);

        if (++_readHead == _buffer.end())
        {
            _readHead = _buffer.begin();
        }
        --_count;

        return ret;
    }

    /* 
     * bulk value read 
     */
    auto readAll()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        auto ret = rotate_view(_buffer | std::views::all, std::distance(_buffer.begin(), _readHead)) | std::views::take(_count);  

        _readHead = _writeHead;
        _count = 0; 

        return ret;
    }

    T peek() const
    {
        return *_readHead;
    }

    void print() const
    {
        std::cout << "Size: " << size() << ", Count: " << count() << std::endl;
        std::string one;
        std::string three;
        for (int i = 0; i < Size; ++i)
        {
            one = _buffer.cbegin() + i == _readHead ? " r ->" : "";
            three = _buffer.begin() + i == _writeHead ? "<- w" : "";
            printf("%5s 0x%02x %-5s\n", one.c_str(), *(_buffer.cbegin() + i), three.c_str());
        }
    }

private:
    std::mutex _mutex;
    std::array<T, Size> _buffer;
    unsigned int _count = 0;
    typename std::array<T, Size>::iterator _writeHead = _buffer.begin();
    typename std::array<T, Size>::iterator _readHead = _buffer.begin();
    
};