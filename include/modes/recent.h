#pragma once

#include "../mode.h"

#include <future>

#define TOFI_HAS_RECENT

namespace tofi
{
	namespace modes
	{
		class recent : public Mode
		{
		public:
			recent();

			std::wstring name() const override
			{
				return L"recent";
			}

			const Entries &results() override;

			PostExec execute(const Entry &result, const std::wstring &text) override;

		private:
			Entries m_entries;
			std::future<Entries> m_loading;
		};
	} // namespace modes
} // namespace tofi
