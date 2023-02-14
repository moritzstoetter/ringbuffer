//
// Created by Moritz Stötter on 13.02.23.
//
//         ╭────────────────────────────╮
// .mail ──┼──  hi@moritzstoetter.dev ──┼── send()
//  .web ──┼── www.moritzstoetter.dev ──┼── visit()
//         ╰────────────────────────────╯
//
//

#pragma  once

#include <algorithm>
#include <array>
#include <exception>
#include <mutex>
#include <ranges>
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
    constexpr size_t size() const { return Size; }

    void clear()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _offset = 0;
        _count = 0;
    }

    // single value write
    void write(T&& val) { write(std::ranges::views::single(val)); }
    void write(T& val){ write(std::move(val)); }

    // bulk write
    template <std::ranges::input_range InRange>
    void write(InRange&& inRange)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        const size_t n = std::min(inRange.size(), capacity());
        std::ranges::copy_n(inRange.begin(), n, unwoundRange(_offset + _count, Size).begin());
        _count += n;
        if (n < inRange.size())
        {
            throw std::runtime_error("Ringbuffer is full!");
        }
    }

    // bulk value read
    [[nodiscard]] auto read() { return read(_count); }
    [[nodiscard]] auto read(size_t n)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto ret = unwoundRange();
        _count = 0;
        _offset = (_offset+n) % Size;
        return ret;
    }

    T& peek() const { return unwoundRange().begin(); }

    void print() const
    {
        std::cout << "Size: " << size() << ", Count: " << count() << std::endl;
        for (int i = 0; i<Size; ++i) {
            printf("%5s 0x%02x %-5s\n", i==_offset ? " r ->" : "", _buffer.at(i), i==(_offset+_count)%Size ? "<- w" : "");
        }
    }

    template<std::ranges::input_range InRange>
    auto operator<<(InRange& rhs)
    {
        this->write(rhs);
    }

private:
    std::mutex _mutex;
    std::array<T, Size> _buffer;
    size_t _count{0};
    size_t _offset{0};

    inline auto unwoundRange() { return unwoundRange(_offset, _count); }
    inline auto unwoundRange(size_t pos, size_t count) {
        return _buffer
               | std::views::all
               | views::rotate(pos)
               | std::views::take(count);
    }
};
