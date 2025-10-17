#include <cstdio>
#include <mutex>
#include <vector>
#include <memory>

template <typename _Type>
class ControledData {
private:
    struct ctx {
        std::mutex mutex;
        _Type data;
    };
    std::shared_ptr<ctx> _ctx;

public:
    class Lock {
    private:
        std::shared_ptr<ctx> _ctx;
        std::shared_ptr<std::lock_guard<std::mutex>> _lock;
    public:
        Lock(std::shared_ptr<ctx> ctx) : _ctx(ctx) {
            // printf("locking\n");
            _lock = std::make_shared<std::lock_guard<std::mutex>>(_ctx->mutex);
        }
        ~Lock() {
            // printf("unlocking\n");
        }
        _Type & ref() const {
            return _ctx->data;
        }
        _Type const& cref() const {
            return _ctx->data;
        }
    };

    ControledData() {
        _ctx = std::make_shared<ctx>();
    }
    Lock genLock() const {
        return Lock(_ctx);
    }
};

int main()
{
    ControledData<std::vector<int>> data;

    {
        auto lock = data.genLock();
        printf("take ref\n");
        lock.ref().push_back(1);    
        printf("take ref end\n");
    }
    

    {
        auto lock = data.genLock();
        printf("take ref\n");
        lock.ref().push_back(2);
        printf("take ref end\n");
    }

    {
        auto lock = data.genLock();
        for (auto const & ref_item : lock.cref()) {
            printf("%d\n", ref_item);
        }
    }
    return 0;
}