#pragma once

#include <memory>
#include <string>
#include <vector>

namespace tofi
{
    struct Entry
    {
        explicit Entry(std::wstring &&disp) : display(std::move(disp))
        {
        }

        //! The string to display in the results list when coming from the mode, or the user input when being sent to the mode
        std::wstring display;

        //! Search criteria for finding matches, if not set, display will be used
        std::optional<std::vector<std::wstring>> criteria;
    };

    using Entries = std::vector<std::shared_ptr<Entry>>;

    /**
     * @brief What should happen after the mode executed
     * 
     */
    enum class PostExec
    {
        //! Close with successful exit
        CloseSuccess,

        //! Close with a failure code
        CloseFailure,

        //! Stay open to do more
        StayOpen,
    };

    /**
     * @brief Interface of functions a mode must implement to be supported by tofi.
     * 
     */
    class Mode
    {
    public:
        virtual ~Mode() = default;

        //! Gets the mode display name
        virtual std::wstring name() const = 0;

        /**
         * @brief Gets the results to display
         * 
         * @return Entries The results for the input box
         */
        virtual const Entries &results() = 0;

        /**
         * @brief Asks the mode to preview the selected result
         * 
         * @param selected The selected result to preview
         * @return The preview action to wait on if any
         */
        virtual void preview(const Entry &selected){};

        virtual bool first_word_only() const
        {
            return false;
        }

        /**
         * @brief Asks the mode to execute the selected result
         * 
         * @param result The result to execute
         * @param text The text box contents
         * @return The result of the execution
         */
        virtual PostExec execute(const Entry &result, const std::wstring &text) = 0;
    };

    using Modes = std::vector<std::unique_ptr<Mode>>;
} // namespace tofi
