#include <array>
#include <optional>
#include <span>

template <typename T, size_t Size>
class Ringbuffer
{
public:
    bool empty() const 
    {
        return _empty;
    }

    bool full() const
    {
        return _full;
    }

    int count() const
    {
        return full() ? size() : std::distance(_readHead, _writeHead);
    }

    int size() const
    {
        return _buffer.size();
    }

    void write(T &&val)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_full)
        {
            std::cout << "Full. \n";
            throw std::runtime_error("Ringbuffer is full!");
        }

        *_writeHead = val;
        if (++(_writeHead) == _buffer.end())
        {
            _writeHead = _buffer.begin();
        }

        _empty = false;
        _full = (_writeHead == _readHead);
    }

    template<size_t N>
    void write(std::span<T, N> &&vals) {
        for (T val : vals) {
            this->write(std::move(val));
        }
    }

    std::optional<T> read()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_empty)
        {
            std::cout << "Empty! \n";
            return std::nullopt;
        }

        auto ret = std::optional<T>(*_readHead);
        if (++(_readHead) == _buffer.cend())
        {
            _readHead = _buffer.cbegin();
        }

        _empty = (_writeHead == _readHead);
        _full = false;

        return ret;
    }

    // template <size_t Num> 
    // std::optional<std::span<T, Num>> read(Num) {
    //     if (Num > count()) {
    //         return std::nullopt;
    //     }


    // }

    T peek()
    {
        return *_readHead;
    }

    void print() const {
        std::string one;
        std::string three;
        for(int i = 0; i<Size; ++i) {
            if (_buffer.cbegin() + i == _readHead) {
                one = " r ->";
            } else {
                one = "";
            }
            if (_buffer.begin() + i == _writeHead) {
                three = " <- w";
            } else {
                three = "";
            }
            printf("%5s %-5x %-5s\n", one.c_str(), *(_buffer.cbegin()+i), three.c_str() );
        }
    }

private:
    std::mutex _mutex;
    std::array<T, Size> _buffer;
    typename std::array<T, Size>::iterator _writeHead = _buffer.begin();
    typename std::array<T, Size>::const_iterator _readHead = _buffer.cbegin();
    bool _empty = true; // !full && (_readHead == _writeHead)
    bool _full = false;
};