//
// Created by Moritz Stötter on 07.02.23.
//
//         ╭────────────────────────────╮
// .mail ──┼──  hi@moritzstoetter.dev ──┼── send()
//  .web ──┼── www.moritzstoetter.dev ──┼── visit()
//         ╰────────────────────────────╯
//

#pragma once

#include <ranges>

template <std::ranges::view V>
class rotate_view : public std::ranges::view_interface<rotate_view<V>>
{
    struct iterator;
    struct sentinel;

public:
    rotate_view() = default;

    constexpr explicit rotate_view(V view, size_t offset)
        : _baseView{std::move(view)}, _offset{offset % view.size()}
    {}

    constexpr V base() const& requires std::copy_constructible<V> {
        return _baseView;
    }

    constexpr V base() && { return std::move(_baseView); }

    iterator begin() { return iterator{*this}; }
    sentinel end() { return sentinel{}; }
    
    constexpr auto size() const requires std::ranges::sized_range<V> {
        return std::ranges::size(_baseView);
    }

    constexpr auto size() requires std::ranges::sized_range<const V> {
        return static_cast<const rotate_view<V>&>(*this).size();
    }

private:
    struct iterator
    {
        using reference = std::ranges::range_reference_t<V>;
        using value_type = std::ranges::range_value_t<V>;
        using difference_type = std::ranges::range_difference_t<V>;

        iterator() = default;
        iterator(rotate_view &parent) 
            : _parent(&parent),
            _iter{std::ranges::next(parent._baseView.begin(), parent._offset)}
        {}

        reference operator*() const
        {
            return *_iter;
        }

        iterator &operator++()
        {
            if (++_iter == std::ranges::end(_parent->_baseView)) {
                _iter = std::ranges::begin(_parent->_baseView);
                _wrapped = true;
            }
            return *this;
        }

        void operator++(int) { ++*this; }

        bool operator==(const sentinel &sent) const
        {
            return _wrapped 
            ? _iter == std::ranges::next(_parent._baseView.begin(), _parent._offset)
            : _iter == std::ranges::end(_parent->_baseView);
        }

        rotate_view *_parent = nullptr;
        std::ranges::iterator_t<V> _iter;
        bool _wrapped = false;
    };

    struct sentinel
    {
        std::ranges::sentinel_t<V> s;
    };

    V _baseView = V{};
    size_t _offset = 0;
};


struct rotate_range_adaptor_closure
{
    size_t _rotation_offset;
    constexpr rotate_range_adaptor_closure(size_t rotation_offset) :
        _rotation_offset(std::move(rotation_offset))
    {}

    template<std::ranges::viewable_range Range>
    constexpr auto operator() (Range&& range) const
    {
        return rotate_view{std::forward<Range>(range), _rotation_offset};
    }
} ;


struct rotate_range_adaptor
{
    template<std::ranges::viewable_range Range>
    constexpr auto operator() (Range&& range, size_t rotation_offset)
    {
        return rotate_view{std::forward<Range>(range), std::move(rotation_offset)};
    }

    constexpr auto operator() (size_t rotation_offset)
    {
        return rotate_range_adaptor_closure{std::move(rotation_offset)};
    }
};

template<std::ranges::viewable_range Range>
constexpr auto operator| (Range&& range,
                                    rotate_range_adaptor_closure&& closure)
{
    return std::move(closure)(std::forward<Range>(range));
}

namespace views { 
    rotate_range_adaptor rotate;
}