#include <ranges>

template <std::ranges::view V>
class rotate_view : public std::ranges::view_interface<rotate_view<V>>
{
    struct iterator;
    struct sentinel;

public:
    rotate_view() = default;

    rotate_view(V view, size_t&& offset)
        : _baseView{view}, _offset{offset % view.size()}
    {}

    iterator begin() { return iterator{*this}; }
    sentinel end() { return sentinel{std::ranges::end(_baseView)}; }
    

private:
    struct iterator
    {
        using reference = std::ranges::range_reference_t<V>;
        using value_type = std::ranges::range_value_t<V>;
        using difference_type = std::ranges::range_difference_t<V>;

        iterator() = default;
        iterator(rotate_view &parent) 
            : _parent(&parent),
            _iter{std::ranges::begin(parent._baseView) + parent._offset}
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
            ? _iter == std::ranges::begin(_parent->_baseView) + _parent->_offset 
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

    V _baseView;
    size_t _offset = 0;
};