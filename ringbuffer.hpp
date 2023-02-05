#include <array>

template <typename T, size_t Size>
class Ringbuffer
{
public:
    bool isFull() const
    {
        return _full;
    }

    void write(T &&val)
    {
        if (_full)
        {
            throw std::runtime_error("Ringbuffer is full!");
        }

        *_writeHead = val;
        if (++(_writeHead) == _buffer.end())
            ;
        {
            _writeHead = _buffer.begin();
        }
        _empty = false;
        _full = (_writeHead == _readHead);
    }

    T read()
    {
        if (_empty)
        {
            throw std::runtime_error("Ringbuffer is full!");
        }

        T ret = *_readHead;
        if (++(_readHead) == _buffer.cend())
        {
            _readHead = _buffer.cbegin();
        }
        _empty = (_writeHead == _readHead);
        _full = false;
        return ret;
    }

private:
    typename std::array<T, Size>::iterator _writeHead = _buffer.begin();
    typename std::array<T, Size>::const_iterator _readHead = _buffer.cbegin();
    std::array<T, Size> _buffer;
    bool _empty = true;
    bool _full = false;
};