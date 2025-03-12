/*
 * MIT License
 * Copyright (c) 2024 Paweł Rapacz
 *
 * See the LICENSE file in the root directory of this source tree for more information.
 */


#pragma once


/**
 *  \file       clipper.hpp
 *  \brief      clipper is a simple, header-only library that handles commad line arguments and parsing.
 *  \author     Paweł Rapacz
 *  \version    1.1.2
 *  \date       2024
 *  \copyright  MIT License
 */


#include <stdexcept>
#include <type_traits>
#include <memory>
#include <utility>
#include <unordered_map>
#include <queue>
#include <vector>
#include <set>
#include <string>
#include <string_view>
#include <charconv>
#include <sstream>
#include <iomanip>



#ifndef CLIPPER_HELP_ARG_FIELD_WIDTH
    /// \brief Defines the width of the argument name field in help output.
    #define CLIPPER_HELP_ARG_FIELD_WIDTH    22
#endif



/**
 *  \brief Contains all the clipper utilities.
 */
namespace CLI
{
    class clipper;

    class option_base;

    /// \cond
    template<typename Tp>
    class option;
    /// \endcond


    using option_name_map = std::unordered_map<std::string, std::size_t>; ///< Container for storing option names
    using option_vec = std::vector<std::unique_ptr<option_base>>; ///< Container for storing options



    /// \brief Allowed option types. (int, float, char, std::string)
    template<typename T>
    concept option_types = 
        std::negation_v<std::is_const<T>> &&
        std::negation_v<std::is_same<T, bool>> && (
            std::is_integral_v<T>       ||
            std::is_floating_point_v<T> ||
            std::is_same_v<T, std::string>
        );


    /// \brief Checks if a given type is a character type.
    template<typename T>
    concept is_character_type = 
        std::is_same_v<T, char> ||
        std::is_same_v<T, wchar_t> ||
        std::is_same_v<T, char8_t> ||
        std::is_same_v<T, char16_t> ||
        std::is_same_v<T, char32_t> ||
        std::is_same_v<T, signed char> ||
        std::is_same_v<T, unsigned char>;



    /**
     *  \brief Allows casting option pointers.
     *  \see option< Tp > option< booL > clipper
     */
    class option_base {
        friend class clipper;
    protected:
        /// \cond

        std::string _vname { "value" }; ///< Name of the type that the option holds.
        std::string _doc; ///< Documentation of the option
        bool _req { false }; ///< Stores information about optioin requirement

        inline static std::size_t any_req { }; ///< Holds the number of required options.


    protected:
        /**
         * \brief Returns option synopsis in format: alt_name(or name) [value_info].
         * \return Option synopsis
         */
        std::string synopsis() const noexcept
        { return alt_name + " " + value_info(); }
        
        /**
         * \brief Creates detailed option synopsis in format: [alt_name], name [value_info].
         * \return Detailed option synopsis
         */
        std::string detailed_synopsis() const noexcept
        { return (name == alt_name ? "" : alt_name + ", ") + name + " " + value_info(); }

        /**
         * \brief Creates option value info.
         * \return Option value info (empty by default)
         */
        virtual std::string value_info() const noexcept
        { return ""; };


        virtual void assign(std::string_view) = 0; ///< Converts and assigns a value to an option.
        virtual void operator=(std::string_view) = 0; ///< Converts and assigns a value to an option.
        
        /// \endcond

    public:
        const std::string& name; ///< Reference to name of the option.
        const std::string& alt_name; ///< Reference to alternative name of the option.


    public:
        /// \brief Constructs a new instance and sets its name reference.
        /// \brief Name and alternative name are the same.
        option_base(const std::string& nm)
            : name(nm), alt_name(nm) {}

        /// \brief Constructs a new instance and sets its name and alternative name reference.
        option_base(const std::string& nm, const std::string& anm)
            : name(nm), alt_name(anm) {}

        /// \brief Virtual default constructor.
        virtual ~option_base() = default;


        /**
         *  \brief  Accesses option documentation.
         *  \return Documentation reference.
         */
        const std::string& doc() const noexcept
        { return _doc; }


        /**
         *  \brief  Checks whether the option is required.
         *  \return True if required, false othrerwise.
         */
        bool req() const noexcept
        { return _req; }
    };


    /**
     *  \brief  Contains option properties.
     *  \tparam Tp Option (option value) type.
     *  \see    option_types clipper clipper::add_option()
     *  \anchor opt
     */
    template<option_types Tp>
    class option<Tp> : public option_base {
        friend class clipper;

    public:
        /// \brief Type of function that checks whether the given value meets some requirements
        /// \anchor optPredicate
        using predicate = bool (*)(const Tp&);
        using option_base::doc;

        /// \brief Constructs a new instance and sets its name reference.
        /// \brief Name and alternative name are the same.
        option(const std::string& nm)
            : option_base(nm) {}
        
        /// \brief Constructs a new instance and sets its name and alternative name reference.
        option(const std::string& nm, const std::string& anm)
            : option_base(nm, anm) {}

        /// \brief Default destructor.
        ~option() = default;


        /**
         *  \brief  Sets the variable to write to and the value name.
         *  \param  value_name Name of the value type e.g. file, charset.
         *  \param[out] ref Variable to write the option value to.
         *  \return Reference to itself.
         */
        option& set(std::string_view value_name, Tp& ref) {
            _vname = value_name;
            _ptr = &ref;
            *_ptr = { };
            return *this;
        }


        /**
         *  \brief  Sets the variable to write to and the value name.
         *  \param  value_name Name of the value type e.g. file, charset.
         *  \param[out] ref Variable to write the option value to.
         *  \tparam V Type of def. It must be convertible to the option type.
         *  \param  def Default value of the option.
         *  \return Reference to itself.
         */
        template<typename V>
        option& set(std::string_view value_name, Tp& ref, V def) {
            static_assert(std::is_convertible<V, Tp>::value, "Type V must be convertible to type Tp");

            _vname = value_name;
            _ptr = &ref;
            *_ptr = static_cast<Tp>(def);
            return *this;
        }


        /**
         *  \brief  Sets allowed values.
         *  \param  val Values of the types convertible to the option types.
         *  \return Reference to itself.
         *  \see allow()
         */
        template<typename... Args>
        option& match(Args&&... val) {
            static_assert((std::is_convertible_v<std::decay_t<Args>, Tp> && ...), "All arguments must be of type Tp or convertible to type Tp");
            (_match_list.insert(std::forward<Args>(val)), ... );
            return *this;
        }


        /**
         *  \brief  Sets allowed values (same as \ref match()).
         *  \param  val Values of the types convertible to the option types.
         *  \return Reference to itself.
         *  \see match()
         */
        template<typename... Args>
        option& allow(Args&&... val) {
            return match(std::forward<Args>(val)...);
        }


        /**
         *  \brief  Sets a function that validates the option value.
         *  \param  doc Description of the requirements of the given function, i.e. [0; 1], length < 10, lower case.
         *  \param  pred Function of type \ref predicate that checks whether the given value is valid (meets some requirements).
         *  \return Reference to itself.
         *  \see predicate CLI::pred
         *  \anchor optValidate
         */
        option& validate(std::string_view doc, predicate pred) {
            _doc.append(" ").append(doc);
            _match_func = pred;
            return *this;
        }


        /**
         *  \brief  Sets a function that validates the option value (same as \ref validate()).
         *  \param  doc Description of the requirements of the given function, i.e. [0; 1], length < 10, lower case.
         *  \param  pred Function of type \ref predicate that checks whether the given value is valid (meets some requirements).
         *  \return Reference to itself.
         *  \see predicate validate() CLI::pred
         *  \anchor optRequire
         */
        option& require(std::string_view doc, predicate pred) {
            return validate(doc, pred);
        }
        
        /**
         *  \brief  Sets the option description.
         *  \param  doc Option information (documentation).
         *  \return Reference to itself.
         */
        option& doc(std::string_view doc) {
            _doc = doc;
            return *this;
        }


        /**
         *  \brief  Sets the option to be required.
         *  \return Reference to itself.
         */
        option& req() {
            _req = true;
            any_req++;
            return *this;
        }


        /**
         *  \brief  Creates information about the allowed values of an option.
         *  \return Information in format \<type\> or (val1 val2 ...) if the value has to match values set with match().
         */
        std::string value_info() const noexcept override {
            if (_match_list.empty()) {
                return "<" + _vname + ">";
            }
            else {
                std::string list;

                if constexpr (std::is_same_v<Tp, std::string>) {
                    for (Tp i : _match_list)
                        list.append(i).push_back(' ');
                }
                else if constexpr (std::is_same_v<Tp, char>) {
                    for (Tp i : _match_list)
                        list.append(1, i).push_back(' ');
                }
                else {
                    for (Tp i : _match_list)
                        list.append(std::to_string(i)).push_back(' ');
                }

                list.pop_back();
                return "(" + list + ")";
            }
        }


    protected:
        /// \cond

        /**
         *  \brief Converts and assigns a value to an option.
         *  \param val Value to assign.
         */
        inline void assign(std::string_view val) override {
            if constexpr (std::is_same_v<Tp, std::string>) {

                *_ptr = val;
                if (!validate(*_ptr))
                    throw std::logic_error("Value is not allowed");


            } else if constexpr (is_character_type<Tp>) {

                if (validate(val.front()))
                    *_ptr = val.front();
                else
                    throw std::logic_error("Value is not allowed");

            } else {

                Tp temp_v;

                if (std::from_chars(val.begin(), val.end(), temp_v).ec == std::errc{} && validate(temp_v))
                    *_ptr = temp_v;
                else
                    throw std::logic_error("Value is not allowed");
                
            }
        }


        /**
         *  \brief Converts and assigns a value to an option.
         *  \param val Value to assign.
         */
        inline void operator=(std::string_view val) override {
            assign(val);
        }


        /**
         *  \brief Assigns a value to an option.
         *  \param val Assigned value.
         */
        inline void operator=(Tp val) {
            if (_match_list.empty() || _match_list.contains(val)) {
                *_ptr = val;
            }
            else {
                throw std::logic_error("Value is not allowed");
            }
        }
        

        /**
         *  \brief Validates a given value with an option requirements.
         *  \param val Value to perform validation on
         *  \return True if the given value is valid, false otherwise.
         *  \see match() require()
         */
        bool validate(const Tp& val) const {
            if (nullptr == _match_func)
                return _match_list.empty() || _match_list.contains(val);
            else
                return _match_func(val) && (_match_list.empty() || _match_list.contains(val));
        }

        /// \endcond

    private:
        Tp* _ptr = nullptr;         ///< Pointer where to write parsed value to.
        // std::string _match_func_doc; ///< Documentation of the requirements of a \ref predicate function i.e. [0; 1], length < 10, lower case
        predicate _match_func = nullptr; ///< Function that checks wheather the value is allowed.
        std::set<Tp> _match_list;   ///< Contains allowed values (if empty all viable values are allowed).
    };


    /**
     *  \brief Contains flag properties (no argument option - boolean).
     *  \see   clipper clipper::add_flag() clipper::add_option()
     */
    template<>
    class option<bool> : public option_base {
        friend class clipper;

    public:
        using option_base::doc;

        /// \brief Constructs a new instance and sets its name reference.
        /// \brief Name and alternative name are the same.
        option(const std::string& nm)
            : option_base(nm) {}
        
        /// \brief Constructs a new instance and sets its name and alternative name reference.
        option(const std::string& nm, const std::string& anm)
            : option_base(nm, anm) {}

        /// \brief Default destructor.
        ~option() = default; 


        /**
         *  \brief  Sets the variable to write to.
         *  \param[out] ref Variable to write the \ref option< bool > "flag (option<bool>)" value (state) to.
         *  \return Reference to itself.
         */
        option& set(bool& ref) {
            _ptr = &ref;
            *_ptr = { };
            return *this;
        }


        /**
         *  \brief  Sets the \ref option< bool > "flag (option<bool>)" description.
         *  \param  doc \ref option< bool > "Flag (option<bool>)" information (documentation).
         *  \return Reference to itself.
         */
        option& doc(std::string_view doc) {
            _doc = doc;
            return *this;
        }


        /**
         *  \brief  Sets the \ref option< bool > "flag (option<bool>)" to be required.
         *  \return Reference to itself.
         */
        option& req() {
            _req = true;
            any_req++;
            return *this;
        }


    protected:
        /// \cond

        /**
         *  \brief Converts and assigns a value to an option.
         *  \param val Value to assign.
         */
        inline void assign(std::string_view val) override {
            *_ptr = true;
        }


        /**
         *  \brief Converts and assigns a value to an option.
         *  \param val Value to assign.
         */
        inline void operator=(std::string_view val) override {
            *_ptr = true;
        }


        /**
         *  \brief Assigns a value to an \ref option< bool > "flag (option<bool>)".
         */
        inline void operator=(bool val) {
            *_ptr = val;
        }

        /// \endcond

    private:
        bool* _ptr = nullptr; ///< Pointer where to write parsed value (state) to.
    };


    /**
     *  \brief Holds all the CLI information and performs the most important actions.
     * 
     * This class is practically the only interface of the clipper library,
     * that is meant to be directly used.
     * It holds the neccessary and optional information about
     * the application, options and flags.
     * 
     * Basically everything you need.
     * 
     *  \see \ref index "Main Page" option<bool> option< Tp > option_types
     */
    class clipper {
    public:
        const std::vector<std::string>& wrong = _wrong; ///< Contains all errors encountered while parsing.

    public:
        /// \brief Default constructor.
        clipper() = default;

        /// \brief Constructs a clipper instance and sets the app name.
        clipper(std::string_view app_name)
            : _app_name(app_name) {}

        /// \brief Constructs a clipper instance and sets the app name and other information.
        clipper(std::string_view app_name, std::string_view version, std::string_view author, std::string_view license_notice)
            : _app_name(app_name), _version(version), _author(author), _license_notice(license_notice) {}


        /// \brief Default destructor.
        ~clipper() = default;


        /**
         *  \brief  Sets the (application) name.
         *  \return Reference to itself.
         */
        clipper& name(std::string_view name) noexcept {
            _app_name = name;
            return *this;
        }

        /**
         *  \brief  Gets the (application) name.
         *  \return Application name.
         */
        const std::string& name() const noexcept {
            return _app_name;
        }


        /**
         *  \brief  Sets the description.
         *  \return Reference to itself.
         */
        clipper& description(std::string_view description) noexcept {
            _app_description = description;
            return *this;
        }

        /**
         *  \brief 	Gets the description.
         *  \return Description reference.
         */
        const std::string& description() const noexcept {
            return _app_description;
        }


        /**
         *  \brief  Sets the version.
         *  \return Reference to itself.
         */
        clipper& version(std::string_view version) noexcept {
            _version = version;
            return *this;
        }

        /**
         *  \brief  Gets the version.
         *  \return Version reference.
         */
        const std::string& version() const noexcept {
            return _version;
        }


        /**
         *  \brief  Sets the author.
         *  \return Reference to itself.
         */
        clipper& author(std::string_view name) noexcept {
            _author = name;
            return *this;
        }

        /**
         *  \brief  Gets the author.
         *  \return Author reference.
         */
        const std::string& author() const noexcept {
            return _author;
        }


        /**
         *  \brief  Sets the license notice.
         *  \return Reference to itself.
         */
        clipper& license(std::string_view license_notice) noexcept {
            _license_notice = license_notice;
            return *this;
        }

        /**
         *  \brief 	Gets the license notice.
         *  \return License notice reference.
         */
        const std::string& license() const noexcept {
            return _license_notice;
        }


        /**
         *  \brief  Sets the web link.
         *  \return Reference to itself.
         */
        clipper& web_link(std::string_view link) noexcept {
            _web_link = link;
            return *this;
        }

        /**
         *  \brief  Gets the web link.
         *  \return Web link reference.
         */
        const std::string& web_link() const noexcept {
            return _web_link;
        }


        /**
         *  \brief  Adds an option of a given type.
         *  \see    option_types option< Tp >
         *  \tparam Tp Option (option value) type.
         *  \param  name Option name.
         *  \return Reference to the created option.
         */
        template<typename Tp>
            requires std::is_same_v<Tp, bool> || option_types<Tp>
        option<Tp>& add_option(std::string_view name) {
            _names[name.data()] = _options.size();
            _options.emplace_back(std::make_unique<option<Tp>>(_names.find(name.data())->first));
            return *static_cast<option<Tp>*>(_options.back().get());
        }


        /**
         *  \brief  Adds an option of a given type.
         *  \see    option_types option< Tp >
         *  \tparam Tp Option (option value) type.
         *  \param  name Primary option name.
         *  \param  alt_name Secondary option name.
         *  \return Reference to the created option.
         */
        template<typename Tp>
            requires std::is_same_v<Tp, bool> || option_types<Tp>
        option<Tp>& add_option(std::string_view name, std::string_view alt_name) {
            _names[name.data()] = _options.size();
            _names[alt_name.data()] = _options.size();

            auto& nameref = _names.find(name.data())->first;
            auto& alt_nameref = _names.find(alt_name.data())->first;
            _options.emplace_back(std::make_unique<option<Tp>>(nameref, alt_nameref));

            return *static_cast<option<Tp>*>(_options.back().get());
        }


        /**
         *  \brief  Adds a \ref option< bool > "flag (option<bool>)".
         * 
         *  This function adds new \ref option< bool > "flag (option<bool>)".
         *  Internally it just calls \ref add_option() "add_option<bool>(...)".
         * 
         *  \see    option< bool >
         *  \param  name Flag name.
         *  \return Reference to the created flag.
         */
        option<bool>& add_flag(std::string_view name) {
            return add_option<bool>(name);
        }


        /**
         *  \brief  Adds a \ref option< bool > "flag (option<bool>)".
         * 
         *  This function adds new \ref option< bool > "flag (option<bool>)".
         *  Internally it just calls \ref add_option() "add_option<bool>(...)".
         * 
         *  \see    option< bool >
         *  \param  name Primary flag name.
         *  \param  alt_name Secondary flag name.
         *  \return Reference to the created flag.
         */
        option<bool>& add_flag(std::string_view name, std::string_view alt_name) {
            return add_option<bool>(name, alt_name);
        }


        /**
         *  \brief  Sets/activates the help/version \ref option< bool > "flag (option<bool>)".
         * 
         *  Help/Version flag is not a standard flag, it has to be used independently.
         *  When used with other options is treated as an unknown option.
         * 
         *  \see    option< bool >
         *  \param  name Primary flag name.
         *  \param  alt_name Secondary flag name. (optional)
         *  \return Help/Version flag reference.
         */
        option<bool>& help_flag(std::string_view name, std::string_view alt_name = "") {
            _help_flag.name = name;
            _help_flag.alt_name = alt_name;
            _help_flag.hndl.doc("displays help");
            return _help_flag.hndl;
        }


        /// \copydoc help_flag
        option<bool>& version_flag(std::string_view name, std::string_view alt_name = "") {
            _version_flag.name = name;
            _version_flag.alt_name = alt_name;
            _version_flag.hndl.doc("displays version information");
            return _version_flag.hndl;
        }


        /**
         *  \brief  Creates a documentation (man page, help) for the application.
         *  \return Documentation.
         */
        inline std::string make_help() const noexcept {
            auto add_help = [](const option_base* const opt, std::ostringstream& stream) {
                auto snps = std::move(opt->detailed_synopsis());

                if (CLIPPER_HELP_ARG_FIELD_WIDTH <= snps.length()) {
                    stream << '\t' << snps << "\n\t" << std::string(CLIPPER_HELP_ARG_FIELD_WIDTH, ' ') << opt->doc() << '\n';
                }
                else
                    stream << '\t' << std::left << std::setw(CLIPPER_HELP_ARG_FIELD_WIDTH) << snps << opt->doc() << '\n';
            };

            std::ostringstream flags;
            std::ostringstream options;
            
            if (_help_flag.is_set())
                add_help(&_help_flag.hndl, flags);

            if (_version_flag.is_set())
                add_help(&_version_flag.hndl, flags);

            for (auto& opt : _options) {
                if (dynamic_cast<option<bool>*>(opt.get()))
                    add_help(opt.get(), flags);
                else
                    add_help(opt.get(), options);
            }



            std::ostringstream help;

            if (not _app_description.empty())
                help << "DESCRIPTION\n\t" << description() << "\n\n";
            
            // SYNOPSIS
            help << "SYNOPSIS\n\t" << _app_name;

            for (auto& opt : _options)
                if (opt->req())
                    help << " " << opt->synopsis();

            help << " [...]\n";
            // end SYNOPSIS

            if (not flags.view().empty())
                help << "\nFLAGS\n" << flags.view();

            if (not options.view().empty())
                help << "\nOPTIONS\n" << options.view();

            if (not _license_notice.empty())
                help << "\nLICENSE\n\t" << _license_notice << "\n";

            if (not _author.empty())
                help << "\nAUTHOR\n\t" << _author << "\n";

            if (not _web_link.empty())
                help << "\n" << _web_link << "\n";

            return help.str();
        }


        /**
         *  \brief  Creates a version notice for the application.
         *  \return Version notice.
         */
        inline std::string make_version_info() const noexcept {
            return
                _app_name + " " + 
                _version + "\n" + 
                _author + "\n";
        }


        /**
         *  \brief Allows the app to be used without any arguments.
         * 
         *  This function allows for using the app without any arguments (even the ones marked as required).
         *  When this function is used, there will be no parsing errors when no arguments are given.
         *  
         *  \see no_args() parse()
         */
        inline void allow_no_args() {
            _allow_no_args = true;
        }


        /**
         *  \brief Checks if no arguments were given.
         *  \see allow_no_args() parse()
         *  \return True if no arguments were given (always true before parsing), false if any arguments were given.
         */
        inline bool no_args() const {
            return _args_count == 1;
        }



        /**
         *  \brief Parses the command line input.
         *  \param argc Argument count.
         *  \param argv Arguments.
         *  \return True if arguments were parsed successfully, false otherwise.
         */
        bool parse(int argc, char* argv[]) {
            _args_count = argc;
            bool err = false;
        

            if (_allow_no_args && argc == 0)
                return !err;

            auto req_count = option_base::any_req;
            std::queue<std::string> args;
            for (int i = 1; i < argc; i++) // argv[0] is the command name, it is meant to be omitted
                args.push(argv[i]);


            if (args.size() == 1 && args.front() == _help_flag) {
                _help_flag.hndl = true; 
                return true;
            }

            if (args.size() == 1 && args.front() == _version_flag) {
                _version_flag.hndl = true;
                return true;
            }

            while (not args.empty()) {
                if (_names.contains(args.front())) {
                    if (_options[_names[args.front()]]->req())
                        req_count--;

                    set_option(args, err); // it pops the option and its value
                }
                else {
                    _wrong.emplace_back("[" + args.front() + "] Unkonown argument");
                    err = true;
                    args.pop(); // necessary to properly continue
                }
            }

            if (req_count) {
                _wrong.emplace_back("Missing required argument(s) " + std::to_string(req_count));
                err = true;
            }

            return !err;
        }



    private:
        /// \brief Parses value of an option/flag and catches errors.
        inline void set_option(std::queue<std::string>& args, bool& error) {
            auto& opt = _options[_names[args.front()]];
            std::string temp_option_name = std::move(args.front());
            args.pop();

            if ( auto optFlag = dynamic_cast<option<bool>*>(opt.get()) ) {
                *optFlag = true;
            }
            else if (args.empty()) {
                _wrong.emplace_back("[" + temp_option_name + "] Missing option value");
                error = true;
            }
            else {
                try { opt->assign(args.front()); }
                catch (...) {
                    _wrong.emplace_back("[" + temp_option_name + "] Value " + args.front() + " is not allowed \n\t{ " + opt->detailed_synopsis() + "  " + opt->doc() + " }");
                    error = true;
                }
                args.pop();
            }
        }



    private:
        std::string _app_name;
        std::string _app_description;
        std::string _version;
        std::string _author;
        std::string _license_notice;
        std::string _web_link;


        /// \brief Contains a \ref option<bool> "flag" information.
        /// \brief Primarly for version and help flags.
        struct {
            std::string name; ///< Name of the flag.
            std::string alt_name; ///< Alternative name of the flag.
            option<bool> hndl {name, alt_name}; ///< \ref option<bool> "Flag" handle.

            /**
             *  \brief Compares string with name and alt_name.
             *  \param str String to compare to
             *  \return True if the given string is equal to name or alt_name, false otherwise.
             */
            bool operator==(const std::string& str) const noexcept
            { return name == str or alt_name == str; }

            /**
             * \brief Checks whether the option is set (name is not empty).
             * \return True if opiton is set, flase otherwise.
             */
            bool is_set() const noexcept
            { return not name.empty(); }

        } _help_flag, _version_flag;


        int _args_count { }; ///< Contains the argument count.
        bool _allow_no_args { false }; ///< Determines whether the app can be used without giving any arguments. \ref allow_no_args() "See more"
        option_name_map _names; ///< Contains option names.
        option_vec _options; ///< Contains all options.
        std::vector<std::string> _wrong; ///< Contains all errors encountered while parsing.
    };

} // namespace CLI




/**
 * \brief Namespace that contains template predicates for \ref opt "options".
 * \see numeric 
 *      \ref opt "option"
 *      \ref optPredicate "option::predicate"
 *      \ref optValidate "option::validate()"
 */
namespace CLI::pred {
    /**
     * \brief Allowed predicate types.
     * \see \ref opt "option"
     */
    template<typename Tp>
    concept numeric =
        std::negation_v<std::is_same<Tp, bool>> && (
            std::is_integral_v<Tp>       ||
            std::is_floating_point_v<Tp>
        );


    /**
     * \brief Predicate that checks whether a value is between bounds (excludes the bounds).
     * \brief V1 and V2 must be of the same type that is also \ref numeric.
     * \tparam V1 First (smaller) bound (compile-time constant of same type as V2).
     * \tparam V2 Second (greater) bound (compile-time constant of same type as V1).
     * \see numeric 
     *      \ref opt "option"
     *      \ref optPredicate "option::predicate"
     *      \ref optValidate "option::validate()"
     */
    template<auto V1, auto V2>
        requires numeric<decltype(V1)> && std::is_same_v<decltype(V1), decltype(V2)>
    bool between(const decltype(V1)& val) {
        static_assert(V1 < V2, "V1 must be less than V2."); 
        return V1 < val && val < V2;
    }


    /**
     * \brief Predicate that checks whether a value is between bounds (includes the bounds).
     * \brief V1 and V2 must be of the same type that is also \ref numeric.
     * \tparam V1 First (smaller) bound (compile-time constant of same type as V2).
     * \tparam V2 Second (greater) bound (compile-time constant of same type as V1).
     * \see numeric 
     *      \ref opt "option"
     *      \ref optPredicate "option::predicate"
     *      \ref optValidate "option::validate()"
     */
    template<auto V1, auto V2>
        requires numeric<decltype(V1)> && std::is_same_v<decltype(V1), decltype(V2)>
    bool ibetween(const decltype(V1)& val) {
        static_assert(V1 < V2, "V1 must be less than V2.");
        return V1 <= val && val <= V2;
    }


    /**
     * \brief Predicate that checks whether a value is greater than a number (excludes the number).
     * \brief Type of V must be \ref numeric.
     * \tparam V number that the given value will be compared to.
     * \see numeric 
     *      \ref opt "option"
     *      \ref optPredicate "option::predicate"
     *      \ref optValidate "option::validate()"
     */
    template<auto V>
        requires numeric<decltype(V)>
    bool greater_than(const decltype(V)& val) {
        return V < val;
    }


    /**
     * \brief Predicate that checks whether a value is greater than a number (includes the number).
     * \brief Type of V must be \ref numeric.
     * \tparam V number that the given value will be compared to.
     * \see numeric 
     *      \ref opt "option"
     *      \ref optPredicate "option::predicate"
     *      \ref optValidate "option::validate()"
     */
    template<auto V>
        requires numeric<decltype(V)>
    bool igreater_than(const decltype(V)& val) {
        return V <= val;
    }


    /**
     * \brief Predicate that checks whether a value is less than a number (excludes the number).
     * \brief Type of V must be \ref numeric.
     * \tparam V number that the given value will be compared to.
     * \see numeric 
     *      \ref opt "option"
     *      \ref optPredicate "option::predicate"
     *      \ref optValidate "option::validate()"
     */
    template<auto V>
        requires numeric<decltype(V)>
    bool less_than(const decltype(V)& val) {
        return V > val;
    }


    /**
     * \brief Predicate that checks whether a value is less than a number (includes the number).
     * \brief Type of V must be \ref numeric.
     * \tparam V number that the given value will be compared to.
     * \see numeric 
     *      \ref opt "option"
     *      \ref optPredicate "option::predicate"
     *      \ref optValidate "option::validate()"
     */
    template<auto V>
        requires numeric<decltype(V)>
    bool iless_than(const decltype(V)& val) {
        return V >= val;
    }

} // namespace CLI::pred