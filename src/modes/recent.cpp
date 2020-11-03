#include "modes/recent.h"

#include "utils/spawn.h"

#include <gtkmm/recentmanager.h>
#include <gtkmm/main.h>

#include <codecvt>
#include <locale>
#include <sstream>

namespace tofi
{
	namespace modes
	{
		struct RecentEntry : public Entry
		{
			using Entry::Entry;

			Glib::RefPtr<Gtk::RecentInfo> info;
		};

		std::future<Entries> load_recent()
		{
			static std::once_flag GTK_INIT_FLAG;
			std::call_once(GTK_INIT_FLAG, Gtk::Main::init_gtkmm_internals);

			return std::async(std::launch::async, []() {
				Glib::RefPtr<Gtk::RecentManager> manager{Gtk::RecentManager::get_default()};
				std::vector<Glib::RefPtr<Gtk::RecentInfo>> items{manager->get_items()};
				items.erase(std::remove_if(std::begin(items), std::end(items), [](Glib::RefPtr<Gtk::RecentInfo> &info) {
								return !info->exists();
							}),
							std::end(items));

				Entries entries;
				entries.reserve(items.size());
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				std::transform(std::begin(items), std::end(items), std::back_inserter(entries), [&converter](Glib::RefPtr<Gtk::RecentInfo> &info) {
					auto entry{std::make_shared<RecentEntry>(converter.from_bytes(info->get_display_name() + ": " + info->get_uri_display()))};
					entry->info = info;

					return entry;
				});

				return entries;
			});
		}

		recent::recent() : m_loading{load_recent()}
		{
		}

		const Entries &recent::Results()
		{
			if (m_entries.empty() && m_loading.valid())
			{
				m_entries = m_loading.get();
			}

			return m_entries;
		}

		PostExec recent::Execute(const Entry &result, const std::wstring &)
		{
			const RecentEntry *entry{reinterpret_cast<const RecentEntry *>(&result)};
			std::ostringstream cmd;
			cmd << "xdg-open " << entry->info->get_uri();

			return spawn(commands::parse(cmd.str())) ? PostExec::CloseSuccess : PostExec::CloseFailure;
		}
	} // namespace modes
} // namespace tofi
