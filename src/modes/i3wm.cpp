#include "modes/i3wm.h"

#include "utils/string.h"

#include <i3ipc++/log.hpp>

#include <sstream>

#define APP_ID "app_id"
#define INSTANCE_ID "instance"

namespace tofi
{
    namespace modes
    {
        using con_t = std::shared_ptr<i3ipc::container_t>;

        namespace tree
        {
            static bool ignore_listed(con_t con)
            {
                if (!con)
                {
                    return true;
                }

                static const char *IGNORE_LIST[] = {
                    "polybar",
                };

                const std::string &winclass{con->map.contains(APP_ID) ? con->map[APP_ID] : con->window_properties.instance};
                return std::any_of(std::begin(IGNORE_LIST), std::end(IGNORE_LIST), [&winclass](const char *toIgnore) {
                    return winclass == toIgnore;
                });
            }

            static void windows(con_t con, std::vector<con_t> &wins, const std::string &self)
            {
                if (!con)
                {
                    return;
                }

                const bool isTofi{(con->map.contains(APP_ID) && con->map[APP_ID] == self) ||
                                  (con->window_properties.instance == self)};
                if (!isTofi && !ignore_listed(con) && con->nodes.empty() && con->floating_nodes.empty() && (con->type == "con" || con->type == "floating_con"))
                {
                    wins.push_back(con);
                }

                for (auto &node : con->nodes)
                {
                    windows(node, wins, self);
                }

                for (auto &node : con->floating_nodes)
                {
                    windows(node, wins, self);
                }
            }

            static std::vector<con_t> windows(con_t tree, const std::string &self)
            {
                std::vector<con_t> wins;
                windows(tree, wins, self);

                return wins;
            }
        } // namespace tree

        namespace commands
        {
            std::string switch_to_workspace(const std::string &name)
            {
                std::ostringstream command;
                command << "workspace number " << name;

                return command.str();
            }

            std::string move_window(bool appId, const std::string &window_id, const std::string &workspace)
            {
                std::ostringstream command;
                command << "[" << (appId ? APP_ID : INSTANCE_ID) << "=\"" << window_id << "\"] move to workspace " << workspace;

                return command.str();
            }

            std::string focus_window(bool appId, const std::string &window_id)
            {
                std::ostringstream command;
                command << "[" << (appId ? APP_ID : INSTANCE_ID) << "=\"" << window_id << "\"] focus";

                return command.str();
            }

            std::string focus_window(const i3ipc::container_t &con)
            {
                std::ostringstream command;
                command << "[title=\"" << con.name << "\"] focus";

                return command.str();
            }
        } // namespace commands

        i3wm::i3wm(std::string &&self_id) : m_self_id(std::move(self_id))
        {
            i3ipc::g_logging_outs.clear();
        }

        Results i3wm::results(const std::wstring &search)
        {
            m_active = tree::windows(m_conn.get_tree(), m_self_id);
            m_active.erase(std::remove_if(std::begin(m_active), std::end(m_active), [&search](con_t con) {
                               std::wstring name{string::converter.from_bytes(con->name)};
                               return !(string::insensitive::contains(name, search));
                           }),
                           std::end(m_active));

            Results results;
            results.reserve(m_active.size());
            std::transform(std::begin(m_active), std::end(m_active), std::back_inserter(results), [](con_t con) {
                con_t::weak_type wptr = con;
                return Result{string::converter.from_bytes(con->name), con.get()};
            });

            return results;
        }

        void i3wm::preview(const Result &selected)
        {
            auto con{static_cast<const i3ipc::container_t *>(selected.context)};
            if (!con->workspace.has_value())
            {
                return;
            }

            const bool usesAppid{con->map.contains(APP_ID)};
            m_conn.send_command(commands::switch_to_workspace(con->workspace.value()));
            m_conn.send_command(commands::move_window(usesAppid, m_self_id, con->workspace.value()));
            m_conn.send_command(commands::focus_window(usesAppid, m_self_id));
        }

        bool i3wm::execute(const Result &result)
        {
            auto con{static_cast<const i3ipc::container_t *>(result.context)};
            if (!con)
            {
                return false;
            }

            return m_conn.send_command(commands::focus_window(*con));
        }
    } // namespace modes

} // namespace tofi