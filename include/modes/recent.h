#pragma once

#include "../mode.h"

#include <future>

namespace yaltl
{
	namespace modes
	{
		class recent : public Mode
		{
		public:
			recent();

			std::wstring Name() const override
			{
				return L"recent";
			}

			const Entries &Results() override;

			PostExec Execute(const Entry &result, const std::wstring &text) override;

		private:
			Entries m_entries;
			std::future<Entries> m_loading;
		};
	} // namespace modes
} // namespace yaltl
