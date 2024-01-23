#include "macros.h"
#include "panda_executable_path_getter.h"

namespace test::utils {

std::string PandaExecutablePathGetter::Get() const noexcept
{
#ifdef BUILD_FOLDER
    return BUILD_FOLDER + std::string("/bin/es2panda test");
#else
    ASSERT_PRINT(false, "BUILD FOLDER not set");
    return std::string {};
#endif
}

}  // namespace test::utils
