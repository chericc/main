#include <memory>

template <typename T>
class threadsafe_stack {
   public:
    threadsafe_stack();
    std::shared_ptr<T> pop();
    void push(T new_value);
};