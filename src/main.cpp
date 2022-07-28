#include <YaltlConfig.h>

#include "yaltl.h"
#include "modes/dmenu.h"
#ifdef GIOMM_FOUND
#include "modes/drun.h"
#endif

#ifdef I3IPC_FOUND
#include "modes/i3wm.h"
#endif

#ifdef GTKMM_FOUND
#include "modes/recent.h"
#endif

#include "modes/run.h"
#include "modes/script.h"

#include <getopt.h>
#include <ftxui/component/screen_interactive.hpp>
#include <mtl/string.hpp>

struct LaunchMode
{
	std::string_view mode;
	std::optional<std::string_view> script;
};

struct LaunchOptions
{
	bool dmenu{};
	std::vector<LaunchMode> modes;
};

void help()
{
	std::cout << "yaltl usage:" << std::endl
			  << "\tyaltl [--options]" << std::endl
			  << "Options:" << std::endl;
	std::cout << "\t-d, --dmenu\tRun in dmenu mode" << std::endl;
	std::cout << "\t-m, --modes\tStart with modes enabled [drun,run,i3wm]" << std::endl
			  << "\t-h, --help \tDisplay this message" << std::endl
			  << "Modes:" << std::endl;
#ifdef GIOMM_FOUND
	std::cout << "\tdrun\tRun from list of desktop installed applications" << std::endl;
#endif

#ifdef GTKMM_FOUND
	std::cout << "\recent\tOpen a recently opened file" << std::endl;
#endif

	std::cout << "\trun \tRun from binaries on $PATH" << std::endl;
#ifdef I3IPC_FOUND
	std::cout << "\ti3wm\tSwitch between active windows using i3ipc" << std::endl;
#endif

	std::cout << "Script:" << std::endl
			  << "\tPass a custom command disp:command" << std::endl
			  << "\t\t -m list:ls" << std::endl
			  << std::endl
			  << "\tcommand will be called with selected result" << std::endl
			  << "\tyaltl will stay open as long as command prints output" << std::endl;

	exit(-1);
}

LaunchOptions parse_args(int argc, char **argv)
{
	LaunchOptions launch{};

	enum class Option
	{
		modes,
		dmenu,
		help,
	};

	static option options[] = {
		{"modes", required_argument, nullptr, 0},
		{"dmenu", no_argument, nullptr, 0},
		{"help", no_argument, nullptr, 0},
	};

	for (int index{}, code{getopt_long(argc, argv, "m:dh", options, &index)}; code >= 0; code = getopt_long(argc, argv, "m:dh", options, &index))
	{
		switch (code)
		{
		case 'd':
			index = static_cast<int>(Option::dmenu);
			break;
		case 'm':
			index = static_cast<int>(Option::modes);
			break;
		case 'h':
			index = static_cast<int>(Option::help);
			break;
		}

		switch (static_cast<Option>(index))
		{
		case Option::dmenu:
		{
			launch.dmenu = true;
			break;
		}
		case Option::modes:
		{
			std::vector<std::string_view> modes;
			mtl::string::split(optarg, ",", std::back_inserter(modes));

			launch.modes.reserve(modes.size());
			std::transform(std::begin(modes), std::end(modes), std::back_inserter(launch.modes), [](std::string_view mode) {
				std::vector<std::string_view> parts;
				parts.reserve(2);
				mtl::string::split(mode, ":", std::back_inserter(parts));
				return parts.size() == 1 ? LaunchMode{parts[0], std::nullopt} : LaunchMode{parts[0], parts[1]};
			});
			break;
		}
		case Option::help:
		{
			help();
			break;
		}
		}
	}

	return launch;
}

int main(int argc, char **argv)
{
	LaunchOptions options{parse_args(argc, argv)};

	yaltl::Modes modes;

	// If we're in dmenu mode, other modes might break, so... just dmenu
	if (options.dmenu)
	{
		modes.emplace_back(std::make_unique<yaltl::modes::dmenu>());
	}
	else
	{
		std::transform(std::begin(options.modes), std::end(options.modes), std::back_inserter(modes), [](const LaunchMode &mode) -> std::unique_ptr<yaltl::Mode> {
			if (mode.script.has_value())
			{
				return std::make_unique<yaltl::modes::script>(mode.mode, mode.script.value());
			}

#ifdef I3IPC_FOUND
			if ("i3wm" == mode.mode)
			{
				return std::make_unique<yaltl::modes::i3wm>("yaltl");
			}
#endif

			if ("run" == mode.mode)
			{
				return std::make_unique<yaltl::modes::run>();
			}

#ifdef GIOMM_FOUND
			if ("drun" == mode.mode)
			{
				return std::make_unique<yaltl::modes::drun>();
			}
#endif

#ifdef GTKMM_FOUND
			if ("recent" == mode.mode)
			{
				return std::make_unique<yaltl::modes::recent>();
			}
#endif

			return nullptr;
		});
	}

	modes.erase(std::remove(std::begin(modes), std::end(modes), nullptr), std::end(modes));
	if (modes.empty())
	{
		// We don't have any modes, so show user help.
		help();
	}

	// Use active wal theme if available
	system("[ -f $HOME/.cache/wal/sequences ] && cat $HOME/.cache/wal/sequences");

	yaltl::Yaltl yaltl{std::move(modes)};
	int exit{};
	auto screen = ftxui::ScreenInteractive::TerminalOutput();
	yaltl.on_exit = [&exit, &screen](int code) {
		exit = code;
		screen.ExitLoopClosure()();
	};

	screen.Loop(&yaltl);
	return exit;
}
