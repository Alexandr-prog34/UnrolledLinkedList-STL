#pragma once

#include <cstddef>
#include <iterator>
#include <utility>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <type_traits>
#include <limits>


struct NodeTag {};

template<typename T,
    std::size_t NodeMaxSize = 10,
    typename Allocator = std::allocator<T>>
    class unrolled_list
{
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;


private:
    struct Node {
        Node* prev = nullptr;
        Node* next = nullptr;
        size_type count = 0;

        alignas(T) unsigned char storage[NodeMaxSize * sizeof(T)];

        T* elem_ptr(size_type i) {
            return reinterpret_cast<T*>(&storage[i * sizeof(T)]);
        }

        const T* elem_ptr(size_type i) const {
            return reinterpret_cast<const T*>(&storage[i * sizeof(T)]);
        }
    };

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    size_type size_ = 0;

    using alloc_traits = std::allocator_traits<Allocator>;

    using node_allocator_type = typename alloc_traits::template rebind_alloc<Node>;
    using node_alloc_traits = std::allocator_traits<node_allocator_type>;
    node_allocator_type node_alloc_;

    using data_allocator_type = typename alloc_traits::template rebind_alloc<T>;
    using data_alloc_traits = std::allocator_traits<data_allocator_type>;
    data_allocator_type data_alloc_;

    Node* create_node() {
        Node* nd = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(node_alloc_, nd);
        return nd;
    }

    void destroy_node(Node* nd) {
        if (!nd) return;

        for (size_type i = 0; i < nd->count; ++i) {
            T* obj = nd->elem_ptr(i);
            data_alloc_traits::destroy(data_alloc_, obj);
        }

        node_alloc_traits::destroy(node_alloc_, nd);
        node_alloc_traits::deallocate(node_alloc_, nd, 1);
    }

public:
    // Итератор
    class iterator {
    private:
        Node* node_ = nullptr;
        size_type index_ = 0;
        friend class unrolled_list;

        iterator(Node* n, size_type i)
            : node_(n), index_(i)
        {}
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() = default;

        reference operator*() const {
            return *(node_->elem_ptr(index_));
        }
        pointer operator->() const {
            return node_->elem_ptr(index_);
        }

        iterator& operator++() {
            if (!node_) return *this;
            ++index_;
            if (index_ >= node_->count) {
                node_ = node_->next;
                index_ = 0;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        iterator& operator--() {
            if (!node_) return *this;
            if (index_ == 0) {
                node_ = node_->prev;
                if (node_) {
                    index_ = node_->count - 1;
                }
            }
            else {
                --index_;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator tmp(*this);
            --(*this);
            return tmp;
        }

        bool operator==(const iterator& rhs) const {
            return (node_ == rhs.node_) && (index_ == rhs.index_);
        }
        bool operator!=(const iterator& rhs) const {
            return !(*this == rhs);
        }
    };

    class const_iterator {
    private:
        const Node* node_ = nullptr;
        size_type index_ = 0;
        friend class unrolled_list;

        const_iterator(const Node* n, size_type i)
            : node_(n), index_(i)
        {}
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() = default;
        const_iterator(const iterator& it)
            : node_(it.node_), index_(it.index_)
        {}

        reference operator*() const {
            return *(node_->elem_ptr(index_));
        }
        pointer operator->() const {
            return node_->elem_ptr(index_);
        }

        const_iterator& operator++() {
            if (!node_) return *this;
            ++index_;
            if (index_ >= node_->count) {
                node_ = node_->next;
                index_ = 0;
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        const_iterator& operator--() {
            if (!node_) return *this;
            if (index_ == 0) {
                node_ = node_->prev;
                if (node_) {
                    index_ = node_->count - 1;
                }
            }
            else {
                --index_;
            }
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp(*this);
            --(*this);
            return tmp;
        }

        bool operator==(const const_iterator& rhs) const {
            return (node_ == rhs.node_) && (index_ == rhs.index_);
        }
        bool operator!=(const const_iterator& rhs) const {
            return !(*this == rhs);
        }
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Конструкторы
    explicit unrolled_list(const allocator_type& alloc = allocator_type())
        : data_alloc_(alloc)
        , node_alloc_(alloc)
    {
    }

    unrolled_list(size_type n, const T& value,
        const allocator_type& alloc = allocator_type())
        : unrolled_list(alloc)
    {
        for (size_type i = 0; i < n; i++) {
            push_back(value);
        }
    }

    unrolled_list(std::initializer_list<T> il,
        const allocator_type& alloc = allocator_type())
        : unrolled_list(alloc)
    {
        for (auto& val : il) {
            push_back(val);
        }
    }

    unrolled_list(const unrolled_list& other)
        : data_alloc_(other.data_alloc_)
        , node_alloc_(other.node_alloc_)
    {
        for (auto it = other.begin(); it != other.end(); ++it) {
            push_back(*it);
        }
    }

    unrolled_list& operator=(const unrolled_list& other)
    {
        if (this != &other) {
            clear();
            data_alloc_ = other.data_alloc_;
            node_alloc_ = other.node_alloc_;
            for (auto it = other.begin(); it != other.end(); ++it) {
                push_back(*it);
            }
        }
        return *this;
    }

    unrolled_list(unrolled_list&& other) noexcept
        : data_alloc_(std::move(other.data_alloc_))
        , node_alloc_(std::move(other.node_alloc_))
        , head_(other.head_)
        , tail_(other.tail_)
        , size_(other.size_)
    {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    unrolled_list(unrolled_list&& other, const allocator_type& alloc)
        : data_alloc_(alloc)
        , node_alloc_(alloc)
    {
        for (auto it = other.begin(); it != other.end(); ++it) {
            push_back(std::move(*it));
        }
        other.clear();
    }

    unrolled_list& operator=(unrolled_list&& other) noexcept
    {
        if (this != &other) {
            clear();
            data_alloc_ = std::move(other.data_alloc_);
            node_alloc_ = std::move(other.node_alloc_);
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;

            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }


    template <typename InputIt>
    unrolled_list(InputIt first, InputIt last, const allocator_type& alloc = allocator_type())
        : unrolled_list(alloc) // Инициализируем аллокаторы
    {
        Node* current_node = nullptr;
        size_type elements_in_current_node = 0;

        try {
            for (; first != last; ++first) {
                if (!current_node || elements_in_current_node == NodeMaxSize) {
                    // Создаем новый узел
                    Node* new_node = create_node();
                    elements_in_current_node = 0;

                    if (!head_) {
                        head_ = new_node;
                    }
                    else {
                        tail_->next = new_node;
                        new_node->prev = tail_;
                    }
                    tail_ = new_node;
                    current_node = new_node;
                }

                // Конструируем элемент напрямую в памяти узла
                data_alloc_traits::construct(data_alloc_, current_node->elem_ptr(elements_in_current_node), *first);
                elements_in_current_node++;
                current_node->count++;
                size_++;
            }
        }
        catch (...) {
            clear(); // Очищаем список в случае ошибки
            throw;
        }
    }


    ~unrolled_list() {
        clear();
    }

    allocator_type get_allocator() const {
        return data_alloc_;
    }

    
    // Сравнения
    bool operator==(const unrolled_list& other) const
    {
        if (size_ != other.size_) return false;
        auto it1 = cbegin();
        auto it2 = other.cbegin();
        while (it1 != cend()) {
            if (*it1 != *it2) return false;
            ++it1; ++it2;
        }
        return true;
    }
    bool operator!=(const unrolled_list& other) const {
        return !(*this == other);
    }

    bool empty() const {
        return size_ == 0;
    }
    size_type size() const {
        return size_;
    }
    size_type max_size() const {
        return (std::numeric_limits<size_type>::max)() / sizeof(T);
    }

    void clear() noexcept
    {
        Node* cur = head_;
        while (cur) {
            Node* nxt = cur->next;
            destroy_node(cur);
            cur = nxt;
        }
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }

    iterator begin() {
        return head_ ? iterator(head_, 0) : end();
    }
    iterator end() {
        return iterator(nullptr, 0);
    }
    const_iterator begin() const {
        return cbegin();
    }
    const_iterator end()   const {
        return cend();
    }
    const_iterator cbegin() const {
        return head_ ? const_iterator(head_, 0) : const_iterator(nullptr, 0);
    }
    const_iterator cend()   const {
        return const_iterator(nullptr, 0);
    }
    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }
    reverse_iterator rend() {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator rend()   const {
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }
    const_reverse_iterator crend()   const {
        return const_reverse_iterator(cbegin());
    }

    reference front() {
        return *(head_->elem_ptr(0));
    }
    const_reference front() const {
        return *(head_->elem_ptr(0));
    }
    reference back() {
        return *(tail_->elem_ptr(tail_->count - 1));
    }
    const_reference back() const {
        return *(tail_->elem_ptr(tail_->count - 1));
    }

    void push_back(const T& value)
    {
        if (!tail_) {
            tail_ = create_node();
            head_ = tail_;
        }

        if (tail_->count < NodeMaxSize) {
            size_type idx = tail_->count;
            try {
                data_alloc_traits::construct(data_alloc_, tail_->elem_ptr(idx), value);
                tail_->count++;
                ++size_;
            }
            catch (...) {
                data_alloc_traits::destroy(data_alloc_, tail_->elem_ptr(idx));
                throw;
            }
        }
        else {
            Node* nd = create_node();
            try {
                data_alloc_traits::construct(data_alloc_, nd->elem_ptr(0), value);
                nd->count = 1;
                nd->prev = tail_;
                tail_->next = nd;
                tail_ = nd;
                ++size_;
            }
            catch (...) {
                destroy_node(nd);
                throw;
            }
        }
    }


   
    void push_front(const T& value) {
        if (!head_) {
            head_ = create_node();
            tail_ = head_;
        }

        if (head_->count < NodeMaxSize) {
            try {
                // Сдвигаем элементы вправо, чтобы освободить место в начале
                for (size_type i = head_->count; i > 0; --i) {
                    T* from = head_->elem_ptr(i - 1);
                    T* to = head_->elem_ptr(i);
                    data_alloc_traits::construct(data_alloc_, to, *from);
                    data_alloc_traits::destroy(data_alloc_, from);
                }
                // Вставляем новый элемент в начало
                data_alloc_traits::construct(data_alloc_, head_->elem_ptr(0), value);
                head_->count++;
                ++size_;
            }
            catch (...) {
                for (size_type i = 1; i < head_->count; ++i) {
                    T* from = head_->elem_ptr(i);
                    T* to = head_->elem_ptr(i - 1);
                    data_alloc_traits::construct(data_alloc_, to, *from);
                    data_alloc_traits::destroy(data_alloc_, from);
                }
                throw;
            }
        }
        else {
            Node* nd = create_node();
            try {
                data_alloc_traits::construct(data_alloc_, nd->elem_ptr(0), value);
                nd->count = 1;
                nd->next = head_;
                head_->prev = nd;
                head_ = nd;
                ++size_;
            }
            catch (...) {
                destroy_node(nd);
                throw;
            }
        }
    }

    void pop_back() noexcept {
        if (empty())return;
        Node* nd = tail_;
        nd->count--;
        size_type idx = nd->count;
        T* obj = nd->elem_ptr(idx);
        obj->~T();
        --size_;

        if (nd->count == 0 && nd != head_) {
            tail_ = nd->prev;
            tail_->next = nullptr;
            destroy_node(nd);
        }
        if (size_ == 0) {
            destroy_node(tail_);
            head_ = tail_ = nullptr;
        }
    }

    void pop_front() noexcept {
        if (empty())return;
        Node* nd = head_;
        T* obj = nd->elem_ptr(0);
        obj->~T();
        for (size_type i = 1; i < nd->count; i++) {
            T* from = nd->elem_ptr(i);
            T* to = nd->elem_ptr(i - 1);
            new (to) T(std::move(*from));
            from->~T();
        }
        nd->count--;
        --size_;

        if (nd->count == 0 && nd != tail_) {
            head_ = nd->next;
            if (head_) head_->prev = nullptr;
            destroy_node(nd);
        }
        if (size_ == 0) {
            destroy_node(head_);
            head_ = tail_ = nullptr;
        }
    }

    iterator insert(const_iterator cpos, const T& value) {
        Node* node = const_cast<Node*>(cpos.node_);
        size_type idx = cpos.index_;

        if (!node) {
            push_back(value);
            return iterator(tail_, tail_->count - 1);
        }

        if (node->count < NodeMaxSize) {
            for (size_type i = node->count; i > idx; --i) {
                T* from = node->elem_ptr(i - 1);
                T* to = node->elem_ptr(i);
                data_alloc_traits::construct(data_alloc_, to, *from);
                data_alloc_traits::destroy(data_alloc_, from);
            }
            data_alloc_traits::construct(data_alloc_, node->elem_ptr(idx), value);
            node->count++;
        }
        else {
            Node* new_node = create_node();
            size_type half = NodeMaxSize / 2;

            for (size_type i = half; i < NodeMaxSize; ++i) {
                T* from = node->elem_ptr(i);
                T* to = new_node->elem_ptr(i - half);
                data_alloc_traits::construct(data_alloc_, to, *from);
                data_alloc_traits::destroy(data_alloc_, from);
            }

            new_node->count = NodeMaxSize - half;
            node->count = half;

            new_node->next = node->next;
            if (new_node->next) {
                new_node->next->prev = new_node;
            }
            new_node->prev = node;
            node->next = new_node;
            if (tail_ == node) {
                tail_ = new_node;
            }

            if (idx > half) {
                size_type new_idx = idx - half;
                for (size_type i = new_node->count; i > new_idx; --i) {
                    T* from = new_node->elem_ptr(i - 1);
                    T* to = new_node->elem_ptr(i);
                    data_alloc_traits::construct(data_alloc_, to, *from);
                    data_alloc_traits::destroy(data_alloc_, from);
                }
                data_alloc_traits::construct(data_alloc_, new_node->elem_ptr(new_idx), value);
                new_node->count++;
                node = new_node;
                idx = new_idx;
            }
            else {
                for (size_type i = node->count; i > idx; --i) {
                    T* from = node->elem_ptr(i - 1);
                    T* to = node->elem_ptr(i);
                    data_alloc_traits::construct(data_alloc_, to, *from);
                    data_alloc_traits::destroy(data_alloc_, from);
                }
                data_alloc_traits::construct(data_alloc_, node->elem_ptr(idx), value);
                node->count++;
            }
        }

        ++size_;
        return iterator(node, idx);
    }


    iterator insert(const_iterator cpos, size_type n, const T& value)
    {
        iterator ret;
        for (size_type i = 0; i < n; i++) {
            ret = insert(cpos, value);
        }
        return ret;
    }

    iterator erase(const_iterator cpos) noexcept {
        Node* nd = const_cast<Node*>(cpos.node_);
        size_type idx = cpos.index_;

        if (!nd) {
            return end();
        }

        T* victim = nd->elem_ptr(idx);
        data_alloc_traits::destroy(data_alloc_, victim);

        for (size_type i = idx + 1; i < nd->count; ++i) {
            T* from = nd->elem_ptr(i);
            T* to = nd->elem_ptr(i - 1);
            data_alloc_traits::construct(data_alloc_, to, std::move(*from));
            data_alloc_traits::destroy(data_alloc_, from);
        }

        nd->count--;
        --size_;

        if (nd->count == 0 && nd != head_ && nd != tail_) {
            Node* prev = nd->prev;
            Node* next = nd->next;

            if (prev != nullptr) {
                prev->next = next;
            }
            if (next != nullptr) {
                next->prev = prev;
            }

            destroy_node(nd);

            if (next != nullptr) {
                return iterator(next, 0);
            }
            else {
                return end();
            }
        }

        if (nd->count == 0) {
            if (nd == head_) {
                head_ = nd->next;
                if (head_ != nullptr) {
                    head_->prev = nullptr;
                }
                destroy_node(nd);
                if (head_ != nullptr) {
                    return iterator(head_, 0);
                }
                else {
                    return end();
                }
            }
            if (nd == tail_) {
                tail_ = nd->prev;
                if (tail_ != nullptr) {
                    tail_->next = nullptr;
                }
                destroy_node(nd);
                return end();
            }
        }

        if (size_ == 0) {
            destroy_node(nd);
            head_ = nullptr;
            tail_ = nullptr;
            return end();
        }

        if (idx >= nd->count) {
            Node* nxt = nd->next;
            if (nxt != nullptr) {
                return iterator(nxt, 0);
            }
            else {
                return end();
            }
        }

        return iterator(nd, idx);
    }


    iterator erase(const_iterator first, const_iterator last) noexcept
    {
        while (first != last) {
            first = erase(first);
        }
        return iterator(const_cast<Node*>(last.node_), last.index_);
    }

    
    void swap(unrolled_list& other) noexcept(std::is_nothrow_swappable_v<T>)
    {
        using std::swap;
        swap(head_, other.head_);
        swap(tail_, other.tail_);
        swap(size_, other.size_);
        swap(data_alloc_, other.data_alloc_);
        swap(node_alloc_, other.node_alloc_);
    }
};
