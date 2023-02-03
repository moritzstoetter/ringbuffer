#include <array> 

template<typename T, size_t Size>
class Ringbuffer {
public:
    void write(T val) {
        *_writeHead = std::move(val);
        if (_writeHead != _buffer.end()-1) {
            ++_writeHead;
        } else {
            _writeHead = _buffer.begin();
        }
    };

    const T * read() {
        const T * ret = &(*_readHead);
        if (_readHead != _buffer.cend() - 1) {
            ++_readHead;
        } else {
            _readHead = _buffer.cbegin();
        }
        return ret;
    }
private:
    typename std::array<T, Size>::iterator _writeHead = _buffer.begin();
    typename std::array<T, Size>::const_iterator _readHead = _buffer.cbegin();

    std::array<T, Size> _buffer; 
};