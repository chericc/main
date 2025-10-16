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
    class Accessor {
    private:
        std::shared_ptr<ctx> _ctx;
        std::shared_ptr<std::lock_guard<std::mutex>> _lock;
    public:
        Accessor(std::shared_ptr<ctx> ctx) : _ctx(ctx) {
            _lock = std::make_shared<std::lock_guard<std::mutex>>(_ctx->mutex);
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
    Accessor genAccessor() const {
        return Accessor(_ctx);
    }
};

int main()
{
    ControledData<std::vector<int>> data;
    data.genAccessor().ref().push_back(1);

    auto const &ref = data.genAccessor().cref();
    for (auto const & ref_item : ref) {
        printf("%d\n", ref_item);
    }
    return 0;
}