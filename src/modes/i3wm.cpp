#include "modes/i3wm.h"

#include <i3ipc++/log.hpp>

#include <codecvt>
#include <locale>
#include <sstream>

#define APP_ID "app_id"
#define INSTANCE_ID "instance"

namespace tofi
{
    namespace modes
    {
        using con_t = std::shared_ptr<i3ipc::container_t>;

        struct ContainerEntry : public Entry
        {
            using Entry::Entry;

            con_t container;
        };

        namespace tree
        {
            /**
             * @brief Check if the container is to be ignored
             * 
             * @param con The container to check
             * @return true - Ignore it
             * @return false - Don't ignore it
             */
            static bool ignore_listed(con_t con)
            {
                if (!con)
                {
                    return true;
                }

                static const char *IGNORE_LIST[] = {
                    "polybar",
                };

                const std::string &winclass{con->app_id.value_or(con->window_properties.instance)};
                return std::any_of(std::begin(IGNORE_LIST), std::end(IGNORE_LIST), [&winclass](const char *toIgnore) {
                    return winclass == toIgnore;
                });
            }

            /**
             * @brief Gets all windows from the tree
             * 
             * @param con The current spot in the tree
             * @param wins [Out] the windows collected
             * @param self An identifier for self to ignore
             */
            static void windows(con_t con, std::vector<con_t> &wins, const std::string &self)
            {
                if (!con)
                {
                    return;
                }

                const bool isSelf{(con->app_id.value_or(con->window_properties.instance) == self)};
                if (!isSelf && !ignore_listed(con) && con->nodes.empty() && con->floating_nodes.empty() && (con->type == "con" || con->type == "floating_con"))
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

            /**
             * @brief Get all the windows in the tree
             * 
             * @param tree the tree
             * @param self Identifier for self to avoid
             * @return std::vector<con_t> 
             */
            static std::vector<con_t> windows(con_t tree, const std::string &self)
            {
                std::vector<con_t> wins;
                windows(tree, wins, self);

                return wins;
            }
        } // namespace tree

        namespace commands
        {
            /**
             * @brief Builds the command to switch to a workspace
             * 
             * @param name Workspace name to switch to
             * @return std::string The command to run to switch to the workspace
             */
            std::string switch_to_workspace(const std::string &name)
            {
                std::ostringstream command;
                command << "workspace number " << name;

                return command.str();
            }

            /**
             * @brief Get the instance string since it's different between sway and i3
             * 
             * @return const char* 
             */
            const char *get_instance_string()
            {
                static const char *instance{std::string(std::getenv("I3SOCK")).find("sway") != std::string::npos ? APP_ID : INSTANCE_ID};
                return instance;
            }

            /**
             * @brief Gets the command to move a window using instance/app_id
             * 
             * @param window_id The window to move
             * @param workspace The workspace to move to
             * @return std::string
             */
            std::string move_instance(const std::string &window_id, const std::string &workspace)
            {
                std::ostringstream command;
                command << "[" << get_instance_string() << "=\"" << window_id << "\"] move to workspace " << workspace;

                return command.str();
            }

            /**
             * @brief Gets the command to focus on a window using instance/app_id
             * 
             * @param window_id The window to focus
             * @return std::string 
             */
            std::string focus_instance(const std::string &window_id)
            {
                std::ostringstream command;
                command << "[" << get_instance_string() << "=\"" << window_id << "\"] focus";

                return command.str();
            }

            /**
             * @brief Gets a command to focus on a container
             * 
             * @param con The container to focus on
             * @return std::string 
             */
            std::string focus_window(const i3ipc::container_t &con)
            {
                std::ostringstream command;
                command << "[title=\"" << con.name << "\"] focus";

                return command.str();
            }
        } // namespace commands

        i3wm::i3wm(std::string &&self_id) : m_self_id(std::move(self_id))
        {
            // i3ipc will output to stdout or stderr, so make sure to clear it when not debugging issues.
            i3ipc::g_logging_outs.clear();
        }

        const Entries &i3wm::results()
        {
            // Always query for active windows since it can change as we go
            std::vector<con_t> tree{tree::windows(m_conn.get_tree(), m_self_id)};
            m_active.resize(tree.size());

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::transform(std::begin(tree), std::end(tree), std::begin(m_active), [&converter](con_t &con) {
                auto result{std::make_shared<ContainerEntry>(converter.from_bytes(con->name))};
                result->container = con;

                return result;
            });

            return m_active;
        }

        /**
         * @brief Move to window that may be of interest
         * 
         * @param selected The window to check out
         */
        void i3wm::Preview(const Entry &selected)
        {
            auto res = reinterpret_cast<const ContainerEntry *>(&selected);
            auto con{res->container};
            if (!con->workspace.has_value())
            {
                return;
            }

            // Fancy focus dance so we can have tofi always be on the active monitor but maintain focus
            m_conn.send_command(commands::switch_to_workspace(con->workspace.value()));
            m_conn.send_command(commands::move_instance(m_self_id, con->workspace.value()));
            m_conn.send_command(commands::focus_instance(m_self_id));
        }

        /**
         * @brief Focus the desired window
         * 
         * @param result The window to focus
         * @return PostExec 
         */
        PostExec i3wm::execute(const Entry &result, const std::wstring &)
        {
            auto res = reinterpret_cast<const ContainerEntry *>(&result);
            auto con{res->container};

            return m_conn.send_command(commands::focus_window(*con)) ? PostExec::CloseSuccess : PostExec::CloseFailure;
        }
    } // namespace modes

} // namespace tofi
