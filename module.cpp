#include "modules/bz_module.h"

#include <string_view>

namespace {

constexpr char kPanelId[] = "pipes";
constexpr char kAction[] = R"({
    "id": "pipes",
    "label": "Трубы и трубопроводы",
    "menu": "file"
})";
constexpr char kPanel[] = R"({
    "id": "pipes",
    "title": "Трубы и трубопроводы",
    "items": []
})";

const BzHostApi* host = nullptr;

int32_t Init(const BzHostApi* api) noexcept {
    if (!api || api->abi_version != BZ_MODULE_ABI_VERSION || !BZ_HAS(api, add_action) ||
        !BZ_HAS(api, show_panel) || !BZ_HAS(api, close_panel)) {
        return BZ_EFAIL;
    }
    const int32_t status = api->add_action(kAction);
    if (status == BZ_OK) host = api;
    return status;
}

void Shutdown() noexcept {
    if (host) host->close_panel(kPanelId);
    host = nullptr;
}

void OnAction(const char* id) noexcept {
    if (host && id && id == std::string_view(kPanelId)) host->show_panel(kPanel);
}

const BzModuleApi api{BZ_MODULE_ABI_VERSION, sizeof(BzModuleApi), Init, Shutdown, nullptr, OnAction, nullptr};

}  // namespace

extern "C" BZ_MODULE_EXPORT const BzModuleApi* bz_module_entry() noexcept {
    return &api;
}
