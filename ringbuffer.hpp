#include <array>
#include <optional>
#include <span>

template <typename T, size_t Size>
class Ringbuffer
{
public:
    bool empty() const
    {
        return _count == 0;
    }

    bool full() const
    {
        return _count == Size;
    }

    unsigned int count() const
    {   
        return _count;
    }

    int size() const
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
            std::cout << "Full. \n";
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
        if (++_readHead == _buffer.cend())
        {
            _readHead = _buffer.cbegin();
        }

        --_count;

        return ret;
    }

    /* 
     * bulk value read 
     */
    std::span<T, std::dynamic_extent> readAll()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        std::span ret{_buffer.data(), count()};

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
        std::cout << "Size: " << size() << ", Count: " << count();
        std::string one;
        std::string three;
        for (int i = 0; i < Size; ++i)
        {
            one = _buffer.cbegin() + i == _readHead ? " r ->" : "";
            three = _buffer.begin() + i == _writeHead ? " <- w" : "";
            printf("%5s %-5x %-5s\n", one.c_str(), *(_buffer.cbegin() + i), three.c_str());
        }
    }

private:
    std::mutex _mutex;
    std::array<T, Size> _buffer;
    unsigned int _count = 0;
    typename std::array<T, Size>::iterator _writeHead = _buffer.begin();
    typename std::array<T, Size>::const_iterator _readHead = _buffer.cbegin();
    
};