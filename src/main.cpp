#include "tofi.h"

#include "modes/dmenu.h"
#include "modes/drun.h"
#include "modes/i3wm.h"
#include "modes/run.h"
#include "modes/script.h"
#include "utils/string.h"

#include <getopt.h>
#include <giomm/init.h>
#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
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
	std::cout << "tofi usage:" << std::endl
			  << "\ttofi [--options]" << std::endl
			  << "Options:" << std::endl
			  << "\t-d, --dmenu\tRun in dmenu mode" << std::endl
			  << "\t-m, --modes\tStart with modes enabled [drun,run,i3wm]" << std::endl
			  << "\t-h, --help \tDisplay this message" << std::endl
			  << "Modes:" << std::endl
			  << "\tdrun\tRun from list of desktop installed applications" << std::endl
			  << "\trun \tRun from binaries on $PATH" << std::endl
			  << "\ti3wm\tSwitch between active windows using i3ipc" << std::endl
			  << "Script:" << std::endl
			  << "\tPass a custom command disp:command" << std::endl
			  << "\t\t -m list:ls" << std::endl
			  << std::endl
			  << "\tcommand will be called with selected result" << std::endl
			  << "\ttofi will stay open as long as command prints output" << std::endl;
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
			tofi::string::split<char, std::string_view>(optarg, ",", std::back_inserter(modes));

			launch.modes.reserve(modes.size());
			std::transform(std::begin(modes), std::end(modes), std::back_inserter(launch.modes), [](std::string_view mode) {
				std::vector<std::string_view> parts;
				parts.reserve(2);
				tofi::string::split<char, std::string_view>(mode, ":", std::back_inserter(parts));
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

	Gio::init();

	tofi::Modes modes;

	if (options.dmenu)
	{
		modes.emplace_back(std::make_unique<tofi::modes::dmenu>());
	}
	else
	{
		std::transform(std::begin(options.modes), std::end(options.modes), std::back_inserter(modes), [](const LaunchMode &mode) -> std::unique_ptr<tofi::Mode> {
			if (mode.script.has_value())
			{
				return std::make_unique<tofi::modes::script>(mode.mode, mode.script.value());
			}

			if ("i3wm" == mode.mode)
			{
				return std::make_unique<tofi::modes::i3wm>("tofi");
			}

			if ("run" == mode.mode)
			{
				return std::make_unique<tofi::modes::run>();
			}

			if ("drun" == mode.mode)
			{
				return std::make_unique<tofi::modes::drun>();
			}

			return nullptr;
		});
	}

	modes.erase(std::remove(std::begin(modes), std::end(modes), nullptr), std::end(modes));

	if (modes.empty())
	{
		help();
	}

	// Use active wal theme if available
	system("[ -f $HOME/.cache/wal/sequences ] && cat $HOME/.cache/wal/sequences");

	tofi::Tofi tofi{std::move(modes)};
	int exit{};
	auto screen = ftxui::ScreenInteractive::TerminalOutput();
	tofi.on_exit = [&exit, &screen](int code) {
		exit = code;
		screen.ExitLoopClosure()();
	};

	screen.Loop(&tofi);
	return exit;
}
