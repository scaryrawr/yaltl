#include "tofi.h"

#include <algorithm>
#include <ftxui/screen/terminal.hpp>
#include <sys/ioctl.h>
#include <unistd.h>

namespace tofi
{
    Tofi::Tofi(Modes &&modes) : m_container{ftxui::Container::Vertical()}, m_search{}, m_mode{}, m_modes{std::move(modes)}
    {
        m_search.placeholder = L"Search";
        m_search.on_enter = [this] {
            if (!m_activeResults.empty())
            {
                Result result{m_activeResults[this->m_results.selected]};
                result.display = m_search.content;
                const bool success{m_modes[m_mode]->execute(result)};
                if (on_exit)
                {
                    on_exit(success ? 0 : -1);
                }
            }
        };

        Add(&m_container);
        m_container.Add(&m_search);

        m_results.selected_style = ftxui::bold | ftxui::color(ftxui::Color::CyanLight);
        m_results.selected = 0;
        m_container.Add(&m_results);
    }

    void Tofi::next_mode()
    {
        m_mode = (m_mode + 1) % m_modes.size();
    }
    void Tofi::previous_mode()
    {
        m_mode = (m_mode + m_modes.size() - 1) % m_modes.size();
    }

    void Tofi::move(Move move)
    {
        switch (move)
        {
        case Move::Up:
            if (m_results.selected > 0)
            {
                --m_results.selected;
            }

            break;

        case Move::Down:
            if (m_results.selected < m_results.entries.size() - 1)
            {
                ++m_results.selected;
            }

            break;
        }
    }

    bool Tofi::OnEvent(ftxui::Event event)
    {
        static const ftxui::Event UnixEscape{ftxui::Event::Special("\035")};
        static const ftxui::Event CtrlV{ftxui::Event::Special("\026")};
        if (ftxui::Event::Escape == event || UnixEscape == event)
        {
            if (on_exit)
            {
                on_exit(0);
                return true;
            }
        }

        if (CtrlV == event && !m_results.entries.empty())
        {
            m_search.content = m_results.entries[m_results.selected];
            m_search.cursor_position = m_search.content.length() + 1;

            return true;
        }

        if (ftxui::Event::Tab == event)
        {
            next_mode();
            return true;
        }

        if (ftxui::Event::TabReverse == event)
        {
            previous_mode();
            return true;
        }

        if (ftxui::Event::ArrowDown == event)
        {
            move(Move::Down);
            return true;
        }

        if (ftxui::Event::ArrowUp == event)
        {
            move(Move::Up);
            return true;
        }

        return ftxui::Component::OnEvent(event);
    }

    ftxui::Element Tofi::Render()
    {
        m_activeResults = m_modes[m_mode]->results(m_search.content);
        m_results.entries.clear();
        m_results.entries.reserve(m_activeResults.size());
        std::transform(std::begin(m_activeResults), std::end(m_activeResults), std::back_insert_iterator(m_results.entries), [](const Result &result) {
            return result.display;
        });

        if (m_results.selected >= m_results.entries.size())
        {
            m_results.selected = m_results.entries.size() - 1;
        }

        if (m_results.selected < 0)
        {
            m_results.selected = 0;
        }

        if (!m_activeResults.empty())
        {
            Result res{m_activeResults[m_results.selected]};
            res.display = m_search.content;
            m_modes[m_mode]->preview(res);
        }

        struct winsize size = {};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

        return ftxui::vbox({ftxui::hbox({ftxui::text(m_modes[m_mode]->name() + L": "), m_search.Render()}),
                            m_results.Render() | ftxui::yframe | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, size.ws_row - 1)});
    }
} // namespace tofi
