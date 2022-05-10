#include "wasm.hpp"

#include <fstream>
#include <sstream>

#include <actor-zeta/core.hpp>

using namespace std;

namespace services::wasm {

    string read_wasm_file(const boost::filesystem::path& path) {
        ifstream file(path.string(), ios::binary);
        stringstream file_string_stream;

        file_string_stream << file.rdbuf();

        return file_string_stream.str();
    }

    wasm_runner_t::wasm_runner_t(manager_wasm_runner_t* env)
        : actor_zeta::basic_async_actor(env, "wasm_runner")
        , wasm_manager_(components::wasm_runner::engine_t::wamr) {
        add_handler(handler_id(route::create), &wasm_runner_t::load_code);
    }

    auto wasm_runner_t::load_code(const boost::filesystem::path& path) -> void {
        auto code = read_wasm_file(path);

        wasm_manager_.initialize("", "", "", "", false, "", "", {}, {}, code, false);
        wasm_ = wasm_manager_.get_or_create_thread_local_plugin();
    }

} // namespace services::wasm
