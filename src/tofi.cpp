#include "tofi.h"

#include <algorithm>
#include <ftxui/screen/terminal.hpp>

namespace tofi
{
    Tofi::Tofi(Modes &&modes) : m_container{ftxui::Container::Vertical()}, m_search{}, m_mode{}, m_modes{std::move(modes)}
    {
        m_search.placeholder = L"Search";
        m_search.on_enter = std::bind(&Tofi::Execute, this);

        Add(&m_container);
        m_container.Add(&m_search);

        m_results.selected_style = ftxui::bold | ftxui::color(ftxui::Color::Black) | ftxui::bgcolor(ftxui::Color::Green);
        m_results.selected = 0;
        m_container.Add(&m_results);
    }

    void Tofi::Execute()
    {
        if (!m_activeResults.empty())
        {
            auto &result{m_activeResults[this->m_results.selected]};
            const PostExec postAction{m_modes[m_mode]->Execute(*result.result, m_search.content)};
            switch (postAction)
            {
            case PostExec::StayOpen:
                // Invalidate since we might get new things for staying open
                m_previousSearch.reset();
                m_previousMode.reset();

                break;
            case PostExec::CloseFailure:
                on_exit(-1);
                break;
            case PostExec::CloseSuccess:
                on_exit(0);
                break;
            }
        }
    }

    void Tofi::NextMode()
    {
        m_mode = (m_mode + 1) % m_modes.size();
    }

    void Tofi::PreviousMode()
    {
        m_mode = (m_mode + m_modes.size() - 1) % m_modes.size();
    }

    void Tofi::Move(tofi::Move move)
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
            NextMode();
            return true;
        }

        if (ftxui::Event::TabReverse == event)
        {
            PreviousMode();
            return true;
        }

        if (ftxui::Event::ArrowDown == event)
        {
            Move(Move::Down);
            return true;
        }

        if (ftxui::Event::ArrowUp == event)
        {
            Move(Move::Up);
            return true;
        }

        return ftxui::Component::OnEvent(event);
    }

    std::wstring_view get_search(std::wstring &search, bool oneWord)
    {
        if (!oneWord)
        {
            return search;
        }

        if (size_t pos{search.find(L' ')}; pos != std::wstring::npos)
        {
            return std::wstring_view(search.c_str(), pos);
        }

        return search;
    }

    void Tofi::UpdateEntries()
    {
        if (!m_previousSearch.has_value() ||                                         // First run
            m_search.content.find(m_previousSearch.value()) == std::wstring::npos || // Search scope increased
            !m_previousMode.has_value() ||                                           // First run mode
            m_mode != m_previousMode.value())                                        // mode change
        {
            const Entries &results{m_modes[m_mode]->Results()};
            m_activeResults.resize(results.size());
            std::transform(std::begin(results), std::end(results), std::begin(m_activeResults), [](std::shared_ptr<Entry> ptr) {
                return FuzzyResult{ptr, std::nullopt};
            });
        }
    }

    void Tofi::FilterEntries()
    {
        std::wstring_view realSearch{get_search(m_search.content, m_modes[m_mode]->FirstWordOnly())};
        if (!m_previousSearch.has_value() || m_previousSearch.value() != realSearch)
        {
            m_regex = regex::build_regex(realSearch);
        }

        if (!realSearch.empty() && (!m_previousSearch.has_value() ||
                                    m_previousSearch.value() != realSearch ||
                                    !m_previousMode.has_value() ||
                                    m_previousMode.value() != m_mode))
        {
            std::transform(std::begin(m_activeResults), std::end(m_activeResults), std::begin(m_activeResults), [&regex{m_regex}](const FuzzyResult &fuzzy) {
                auto &criteria = fuzzy.result->criteria;
                std::optional<std::wstring_view> fuzzFactor;
                if (criteria.has_value())
                {
                    std::vector<std::optional<std::wstring_view>> fuzz;
                    fuzz.reserve(criteria.value().size());
                    std::transform(std::begin(criteria.value()), std::end(criteria.value()), std::back_inserter(fuzz), [&regex](const std::wstring &critter) {
                        return regex::fuzzy_find(critter, regex);
                    });

                    fuzz.erase(std::remove(std::begin(fuzz), std::end(fuzz), std::nullopt), std::end(fuzz));
                    if (!fuzz.empty())
                    {
                        fuzzFactor = fuzz[0];
                    }
                }
                else
                {
                    fuzzFactor = regex::fuzzy_find(fuzzy.result->display, regex);
                }

                return FuzzyResult{fuzzy.result, fuzzFactor};
            });

            std::sort(std::begin(m_activeResults), std::end(m_activeResults));

            m_activeResults.erase(std::remove_if(std::begin(m_activeResults), std::end(m_activeResults), [](const FuzzyResult &fuzzy) {
                                      return !fuzzy.match.has_value();
                                  }),
                                  std::end(m_activeResults));
        }

        m_previousSearch = realSearch;
        m_previousMode = m_mode;
    }

    ftxui::Element Tofi::Render()
    {
        this->UpdateEntries();

        this->FilterEntries();

        m_results.entries.resize(m_activeResults.size());
        std::transform(std::begin(m_activeResults), std::end(m_activeResults), std::begin(m_results.entries), [](const FuzzyResult &result) {
            return result.result->display;
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
            auto &res{m_activeResults[m_results.selected]};
            m_modes[m_mode]->Preview(*res.result);
        }

        ftxui::Terminal::Dimensions size{ftxui::Terminal::Size()};

        return ftxui::vbox({ftxui::hbox({ftxui::text(m_modes[m_mode]->Name() + L": "), m_search.Render()}),
                            m_results.Render() | ftxui::yframe | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, size.dimy - 1)});
    }
} // namespace tofi
