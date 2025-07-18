#include <unrolled_list.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <list>

/*
    В данном файле представлен ряд тестов, где используются (вместе, раздельно и по-очереди):
        - push_back
        - push_front
        - insert
    Методы применяются на unrolled_list и на std::list.
    Ожидается, что в итоге порядок элементов в контейнерах будут идентичен
*/

TEST(UnrolledLinkedList, pushBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, pushFront) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_front(i);
        unrolled_list.push_front(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, pushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, insertAndPushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else if (i % 3 == 1) {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        } else {
            auto std_it = std_list.begin();
            auto unrolled_it = unrolled_list.begin();
            std::advance(std_it, std_list.size() / 2);
            std::advance(unrolled_it, std_list.size() / 2);
            std_list.insert(std_it, i);
            unrolled_list.insert(unrolled_it, i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, popFrontBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    for (int i = 0; i < 500; ++i) {
        if (i % 2 == 0) {
            std_list.pop_back();
            unrolled_list.pop_back();
        } else {
            std_list.pop_front();
            unrolled_list.pop_front();
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    for (int i = 0; i < 500; ++i) {
        std_list.pop_back();
        unrolled_list.pop_back();
    }

    ASSERT_TRUE(unrolled_list.empty());
}


//My tests
TEST(UnrolledListTest, EraseSingleElement) {
    unrolled_list<int, 4> list = { 1, 2, 3, 4, 5 };

    auto it = list.begin();
    ++it;
    list.erase(it);

    ASSERT_EQ(list.size(), 4);
    ASSERT_EQ(*list.begin(), 1);
    ASSERT_EQ(*(++list.begin()), 3);
}

TEST(UnrolledListTest, Clear) {
    unrolled_list<int, 4> list = { 1, 2, 3, 4, 5 };

    list.clear();

    ASSERT_TRUE(list.empty());
    ASSERT_EQ(list.size(), 0);
}

TEST(UnrolledListTest, EraseFirstElement) {
    unrolled_list<int, 4> list = { 1, 2, 3 };

    list.erase(list.begin()); // Удаляем 1

    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(*list.begin(), 2); // Теперь первый элемент — 2
}

TEST(UnrolledListTest, InsertIntoEmptyList) {
    unrolled_list<int, 4> list;

    list.insert(list.begin(), 10); // Вставляем в пустой список

    ASSERT_EQ(list.size(), 1);
    ASSERT_EQ(*list.begin(), 10);
}

TEST(UnrolledListTest, ClearAndReuse) {
    unrolled_list<int, 4> list = { 1, 2, 3, 4 };

    list.clear();
    ASSERT_TRUE(list.empty());

    list.push_back(5);
    ASSERT_EQ(list.size(), 1);
    ASSERT_EQ(*list.begin(), 5);
}






