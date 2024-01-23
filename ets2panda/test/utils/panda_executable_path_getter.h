#include <string>

namespace test::utils {

class PandaExecutablePathGetter {
public:
    PandaExecutablePathGetter() = default;
    ~PandaExecutablePathGetter() = default;

    std::string Get() const noexcept;
};

}  // namespace test::utils
