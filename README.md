# Unrolled Linked List (STL-compatible container)

Реализация STL-совместимого контейнера Unrolled LinkedList на C++ с поддержкой:  
✅ Аллокаторов  
✅ Двунаправленных итераторов  
✅ Полного набора операций (push/pop/emplace/insert/erase)  
✅ Гарантий исключений  
✅ Тестов на Google Test  

- **STL-совместимые итераторы** (включая reverse-итераторы)

Помимое этого обладать следующими методами 

| Метод     |  Алгоримическая сложность        | Гарантии исключений |
| --------  | -------                          | -------             |
| insert    |  O(1) для 1 элемента, O(M) для M |  strong             |
| erase     |  O(1) для 1 элемента, O(M) для M |  noexcept           |
| clear     |  O(N)                            |  noexcept           |
| push_back |  O(1)                            |  strong             |
| pop_back  |  O(1)                            |  noexcept           |
| push_front|  O(1)                            |  strong             |
| pop_front |  O(1)                            |  noexcept           |

## Тесты
Написаны тесты для покрытия всех методов
