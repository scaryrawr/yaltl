#pragma once

#include <memory>
#include <string>
#include <vector>

namespace tofi
{
    struct Result
    {
        //! The string to display in the results list when coming from the mode, or the user input when being sent to the mode
        std::wstring display;

        //! The context for the result for the mode if the result is selected or executed
        const void *context{};

        auto operator<=>(const Result &) const = default;
    };

    using Results = std::vector<Result>;

    enum class PostExec
    {
        CloseSuccess,
        CloseFailure,
        StayOpen,
    };

    class Mode
    {
    public:
        virtual ~Mode() = default;

        //! Gets the mode display name
        virtual std::wstring name() const = 0;

        /**
         * @brief Gets the results to display
         * 
         * @param search The current text in the input box
         * @return Results The results for the inpux box
         */
        virtual Results results(const std::wstring &search) = 0;

        /**
         * @brief Asks the mode to preview the selected result
         * 
         * @param selected The selected result to preview
         * @return The preview action to wait on if any
         */
        virtual void preview(const Result &selected){};

        /**
         * @brief Asks the mode to execute the selected result
         * 
         * @param result The result to execute
         * @return The result of the execution
         */
        virtual PostExec execute(const Result &result) = 0;
    };

    using Modes = std::vector<std::unique_ptr<Mode>>;
} // namespace tofi
