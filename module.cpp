#include "modules/bz_module.h"
#include "sketch3d.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <string>
#include <string_view>

namespace {

using Json = nlohmann::json;
using pipes::Point3;
using pipes::Sketch3d;

constexpr char kPanelId[] = "pipes";
constexpr char kAction[] = R"({
    "id": "pipes",
    "label": "3D-эскиз трассы",
    "menu": "file",
    "ribbon": {"tab": "assembly", "group": "Трубы и трубопроводы"}
})";
constexpr char kNoAssembly[] = R"({
    "id": "pipes",
    "title": "3D-эскиз трассы",
    "items": [{"type": "text", "text": "Откройте сборку, чтобы построить трассу."}]
})";

const BzHostApi* host = nullptr;
Sketch3d draft;

Json Number(const char* id, const char* label, double value) {
    return {{"type", "number"}, {"id", id}, {"label", label}, {"value", value}};
}

void ShowEditor(std::size_t selected, Point3 input, const char* error = nullptr) {
    const auto points = draft.Points();
    Json items = Json::array();
    Json coordinates = Json::array();
    Json labels = Json::array();
    for (const auto& point : points) {
        coordinates.push_back({point.x, point.y, point.z});
        labels.push_back("Точка " + std::to_string(labels.size() + 1));
    }
    if (!points.empty()) {
        items.push_back({{"type", "choice"}, {"id", "point"}, {"label", "Точки трассы"}, {"options", labels},
                         {"value", selected}, {"dropdown", true}, {"on_change", "select"}});
    }
    items.push_back(Number("x", "X, мм", input.x));
    items.push_back(Number("y", "Y, мм", input.y));
    items.push_back(Number("z", "Z, мм", input.z));
    if (error) items.push_back({{"type", "text"}, {"text", error}});

    Json buttons = Json::array({{{"id", "add"}, {"label", "Добавить"}, {"style", "primary"}}});
    if (!points.empty()) {
        buttons.push_back({{"id", "replace"}, {"label", "Изменить"}});
        buttons.push_back({{"id", "remove"}, {"label", "Удалить"}});
    }
    items.push_back({{"type", "buttons"}, {"items", buttons}});
    const Json panel{{"id", kPanelId}, {"title", "3D-эскиз трассы"}, {"assembly_viewport", true},
                     {"polyline_xyz_mm", coordinates}, {"items", items}};
    host->show_panel(panel.dump().c_str());
}

int32_t Init(const BzHostApi* api) noexcept {
    if (!api || api->abi_version != BZ_MODULE_ABI_VERSION || !BZ_HAS(api, add_action) ||
        !BZ_HAS(api, show_panel) || !BZ_HAS(api, close_panel) || !BZ_HAS(api, assembly_name)) {
        return BZ_EFAIL;
    }
    const int32_t status = api->add_action(kAction);
    if (status == BZ_OK) host = api;
    return status;
}

void Shutdown() noexcept {
    if (host) host->close_panel(kPanelId);
    host = nullptr;
    draft = {};
}

void OnAction(const char* id) noexcept {
    if (!host || !id || id != std::string_view(kPanelId)) return;
    try {
        const char* name = nullptr;
        const char* designation = nullptr;
        if (host->assembly_name(&name, &designation) != BZ_OK) {
            host->show_panel(kNoAssembly);
            return;
        }
        const auto points = draft.Points();
        if (points.empty()) ShowEditor(0, {});
        else ShowEditor(points.size() - 1, points.back());
    } catch (...) {
        if (BZ_HAS(host, log)) host->log(BZ_LOG_ERROR, "Не удалось открыть 3D-эскиз трассы");
    }
}

const char* EditError(Sketch3d::Edit result) {
    switch (result) {
        case Sketch3d::Edit::zero_length_segment: return "Соседние точки совпадают.";
        case Sketch3d::Edit::invalid_point: return "Введите конечные координаты XYZ.";
        case Sketch3d::Edit::invalid_index: return "Выберите точку трассы.";
        case Sketch3d::Edit::ok: return nullptr;
    }
    return nullptr;
}

void OnPanelEvent(const char* eventJson) noexcept {
    if (!host || !eventJson) return;
    try {
        const Json event = Json::parse(eventJson);
        if (event.at("panel") != kPanelId) return;
        const auto action = event.at("id").get<std::string>();
        if (action == "close") {
            draft = {};
            return;
        }

        const auto& values = event.at("values");
        const auto selected = values.value("point", std::size_t{0});
        const auto points = draft.Points();
        if (action == "select" && selected < points.size()) {
            ShowEditor(selected, points[selected]);
            return;
        }

        if (action == "remove") {
            const auto result = draft.Erase(selected);
            const auto remaining = draft.Points();
            const auto index = remaining.empty() ? 0 : std::min(selected, remaining.size() - 1);
            ShowEditor(index, remaining.empty() ? Point3{} : remaining[index], EditError(result));
            return;
        }

        if (action != "add" && action != "replace") return;
        if (!values.at("x").is_number() || !values.at("y").is_number() || !values.at("z").is_number()) {
            ShowEditor(selected, selected < points.size() ? points[selected] : Point3{}, "Введите конечные координаты XYZ.");
            return;
        }
        const Point3 point{values.at("x").get<double>(), values.at("y").get<double>(), values.at("z").get<double>()};
        const auto index = action == "add" ? points.size() : selected;
        const auto result = action == "add" ? draft.Insert(index, point) : draft.Replace(index, point);
        if (result == Sketch3d::Edit::ok) {
            ShowEditor(index, point);
        } else {
            ShowEditor(selected, selected < points.size() ? points[selected] : Point3{}, EditError(result));
        }
    } catch (...) {
        if (BZ_HAS(host, log)) host->log(BZ_LOG_ERROR, "Не удалось изменить 3D-эскиз трассы");
    }
}

const BzModuleApi api{BZ_MODULE_ABI_VERSION, sizeof(BzModuleApi), Init, Shutdown, nullptr, OnAction, OnPanelEvent};

}  // namespace

extern "C" BZ_MODULE_EXPORT const BzModuleApi* bz_module_entry() noexcept {
    return &api;
}
