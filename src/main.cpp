#include "tofi.h"
#include "modes/drun.h"
#include "modes/i3wm.h"
#include "modes/run.h"
#include <giomm/init.h>

#include <ftxui/component/screen_interactive.hpp>

int main(int argc, const char *argv[])
{
	// Use active wal theme if available
	system("[ -f $HOME/.cache/wal/sequences ] && cat $HOME/.cache/wal/sequences");

	Gio::init();
	auto screen = ftxui::ScreenInteractive::TerminalOutput();

	tofi::Modes modes;
	modes.emplace_back(std::make_unique<tofi::modes::drun>());
	modes.emplace_back(std::make_unique<tofi::modes::run>());
	modes.emplace_back(std::make_unique<tofi::modes::i3wm>("tofi"));

	tofi::Tofi tofi{std::move(modes)};
	int exit{};
	tofi.on_exit = [&exit, &screen](int code) {
		exit = code;
		screen.ExitLoopClosure()();
	};

	screen.Loop(&tofi);
	return exit;
}
