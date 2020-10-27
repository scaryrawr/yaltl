#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/container.hpp>
#include <ftxui/component/input.hpp>
#include <ftxui/component/menu.hpp>

#include "mode.h"

namespace tofi
{
    enum class Move
    {
        Up,
        Down
    };

    class Tofi : public ftxui::Component
    {
    public:
        Tofi(Modes &&);

        void next_mode();
        void previous_mode();
        void move(Move move);
        bool OnEvent(ftxui::Event) override;
        ftxui::Element Render() override;

        std::function<void(int)> on_exit;

    private:
        ftxui::Container m_container;
        ftxui::Input m_search;
        ftxui::Menu m_results;
        int32_t m_mode{};
        Modes m_modes;
        Results m_activeResults;
        std::optional<std::wstring> m_previousSearch;
        std::optional<int32_t> m_previousMode;
    };
} // namespace tofi
