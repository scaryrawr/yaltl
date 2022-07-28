#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/container.hpp>
#include <ftxui/component/input.hpp>
#include <ftxui/component/menu.hpp>
#include <regex>

#include "mode.h"
#include "utils/fuzzyresult.h"
#include "utils/regex.h"

namespace yaltl
{
    enum class Move
    {
        Up,
        Down
    };

    class Yaltl : public ftxui::Component
    {
    public:
        Yaltl(Modes &&);

        void Execute();
        void NextMode();
        void PreviousMode();
        void Move(yaltl::Move move);
        bool OnEvent(ftxui::Event) override;
        ftxui::Element Render() override;

        std::function<void(int)> on_exit;

    private:
        //! Load entries from mode if needed
        void UpdateEntries();

    private:
        ftxui::Container m_container;
        ftxui::Input m_search;
        ftxui::Menu m_results;
        int32_t m_mode{};
        Modes m_modes;
        std::vector<FuzzyResult> m_activeResults;
    };
} // namespace yaltl
