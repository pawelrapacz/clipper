/*
 * MIT License
 * Copyright (c) 2024 Paweł Rapacz
 *
 * See the LICENSE file in the root directory of this source tree for more information.
 */


#pragma once


/**
 *  \file       clipper.hpp
 *  \brief      Clipper is a simple, header-only library that handles commad line arguments and parsing.
 *  \author     Paweł Rapacz
 *  \version    1.1.0
 *  \date       2024
 *  \copyright  MIT License
 */


#include <stdexcept>
#include <type_traits>
#include <memory>
#include <map>
#include <unordered_map>
#include <queue>
#include <vector>
#include <set>
#include <utility>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>


/**
 *  \brief Contains all the clipper utilities
 */
namespace CLI
{
    /**
     *  \brief Allowed option types. (int, float, char, std::string)
     */
    template<typename T>
    concept option_types = 
        std::negation_v<std::is_const<T>> &&
        std::negation_v<std::is_same<T, bool>> && (
            std::is_integral_v<T>       ||
            std::is_floating_point_v<T> ||
            std::is_same_v<T, std::string>
        );



    class clipper;

    class option_base;

    template<option_types Tp>
    class option;

    class flag;


    using cstr = const char*;
    using arg_name_map = std::map<std::string, std::string>;
    using option_map = std::unordered_map<std::string, std::shared_ptr<option_base>>;


    /**
     *  \brief Allows casting option pointers.
     *  \see option flag clipper
     */
    class option_base {
    protected:
        std::string _vname { "value" }; ///< Name of the type that the option holds.
        std::string _doc;
        bool _req { false };

        inline static uint32_t any_req { }; ///< Holds the number of required options.

    public:
        virtual ~option_base() = default;

        virtual std::string value_info() const noexcept = 0;

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
     *  \see    option_types clipper
     *  \tparam Tp Option (option value) type.
     */
    template<option_types Tp>
    class option : public option_base {
        friend class clipper;

        Tp* _ref = nullptr;         ///< Pointer where to write parsed value to.
        std::set<Tp> _match_list;   ///< Allowed values (if empty all viable values are allowed).



    protected:
        /**
         *  \brief Assigns a value to an option.
         *  \param val Assigned value.
         */
        inline void operator=(Tp val) {
            if (_match_list.empty() || _match_list.contains(val)) {
                *_ref = val;
            }
            else {
                throw std::logic_error("Value is not allowed");
            }
        }



    public:
        using option_base::doc;
        option() = default;     ///< Default constructor.
        ~option() = default;    ///< Default destructor.


        /**
         *  \brief  Sets the variable to write to and the value name.
         *  \param  value_name Name of the value type e.g. file, charset.
         *  \param[out] ref Variable to write the option value to.
         *  \return Reference to itself.
         */
        option& set(cstr value_name, Tp& ref) {
            _vname = value_name;
            _ref = &ref;
            *_ref = { };
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
        option& set(cstr value_name, Tp& ref, V def) {
            static_assert(std::is_convertible<V, Tp>::value, "Type V must be convertible to type Tp");

            _vname = value_name;
            _ref = &ref;
            *_ref = static_cast<Tp>(def);
            return *this;
        }


        /**
         *  \brief  Sets allowed values.
         *  \param  val Values of the types convertible to the option types.
         *  \return Reference to itself.
         */
        template<typename... Args>
        option& match(Args... val) {
            static_assert((std::is_convertible<std::decay_t<Args>, Tp>::value && ...), "All arguments must be of type Tp");
            (_match_list.insert(std::forward<Args>(val)), ... );
            return *this;
        }


        /**
         *  \brief  Sets the option description.
         *  \param  doc Option information (documentation).
         *  \return Reference to itself.
         */
        option& doc(cstr doc) {
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
         *  \return Information in format <type> or (val1 val2 ...) if the value has to match values set with match().
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
    };


    /**
     *  \brief  Contains flag properties.
     *  \see    clipper
     */
    class flag : public option_base {
        friend class clipper;

        bool* _ref = nullptr; ///< Pointer where to write parsed value (state) to.

    protected:
        /**
         *  \brief Assigns a value to an flag.
         */
        inline void operator=(bool val) {
            *_ref = val;
        }



    public:
        using option_base::doc;
        flag() = default;   ///< Default constructor.
        ~flag() = default;  ///< Default destructor.


        /**
         *  \brief  Sets the variable to write to.
         *  \param[out] ref Variable to write the flag value (state) to.
         *  \return Reference to itself.
         */
        flag& set(bool& ref) {
            _ref = &ref;
            *_ref = { };
            return *this;
        }


        /**
         *  \brief  Sets the flag description.
         *  \param  doc Flag information (documentation).
         *  \return Reference to itself.
         */
        flag& doc(cstr doc) {
            _doc = doc;
            return *this;
        }


        /**
         *  \brief  Sets the flag to be required.
         *  \return Reference to itself.
         */
        flag& req() {
            _req = true;
            any_req++;
            return *this;
        }


        /**
         *  \brief  Unused.
         *  \return Empty string.
         */
        std::string value_info() const noexcept override { return ""; } // delete - unused function
    };



    /**
     *  \brief Used to hold information about flags like --help or --version.
     */
    struct info_flag {
        std::string name;
        std::string alt_name;
        flag fhndl;
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
     *  \see flag option option_types
     *  \ref index
     */
    class clipper {
    public:
        const std::vector<std::string>& wrong = _wrong; ///< Contains all errors encountered while parsing.

    public:
        clipper() = default; ///< Default constructor.

        /**
         *  \brief Constructs a clipper instance and sets the app name.
         */
        clipper(cstr app_name)
            : _app_name(app_name) {}

        /**
         *  \brief Constructs a clipper instance and sets the app name and other information.
         */
        clipper(cstr app_name, cstr version, cstr author, cstr license_notice)
            : _app_name(app_name), _version(version), _author(author), _license_notice(license_notice) {}


        ~clipper() = default; ///< Default destructor.


        /**
         *  \brief  Sets the (application) name.
         *  \return Reference to itself.
         */
        clipper& name(cstr name) noexcept {
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
        clipper& description(cstr description) noexcept {
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
        clipper& version(cstr version) noexcept {
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
        clipper& author(cstr name) noexcept {
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
        clipper& license(cstr license_notice) noexcept {
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
        clipper& web_link(cstr link) noexcept {
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
         *  \see    option_types option
         *  \tparam Tp Option (option value) type.
         *  \param  name Option name.
         *  \return Reference to the created option.
         */
        template<option_types Tp>
        option<Tp>& add_option(cstr name) {
            _options[name] = std::make_shared<option<Tp>>();
            _option_names[name];
            return *std::static_pointer_cast<option<Tp>>(_options[name]);
        }

        /**
         *  \brief  Adds an option of a given type.
         *  \see    option_types option
         *  \tparam Tp Option (option value) type.
         *  \param  name Primary option name.
         *  \param  alt_name Secondary option name.
         *  \return Reference to the created option.
         */
        template<option_types Tp>
        option<Tp>& add_option(cstr name, cstr alt_name) {
            _options[name] = std::make_shared<option<Tp>>();
            _options[alt_name] = _options[name];
            _option_names[name] = alt_name;
            return *std::static_pointer_cast<option<Tp>>(_options[name]);
        }

        /**
         *  \brief  Adds an flag of a given type.
         *  \see    flag
         *  \param  name Flag name.
         *  \return Reference to the created flag.
         */
        flag& add_flag(cstr name) {
            _options[name] = std::make_shared<flag>();
            _flag_names[name];
            return *std::static_pointer_cast<flag>(_options[name]);
        }

        /**
         *  \brief  Adds an flag of a given type.
         *  \see    flag
         *  \param  name Primary flag name.
         *  \param  alt_name Secondary flag name.
         *  \return Reference to the created flag.
         */
        flag& add_flag(cstr name, cstr alt_name) {
            _options[name] = std::make_shared<flag>();
            _options[alt_name] = _options[name];
            _flag_names[name] = alt_name;
            return *std::static_pointer_cast<flag>(_options[name]);
        }


        /**
         *  \brief  Sets/activates the help flag.
         *  \see    flag
         *  \param  name Primary flag name.
         *  \param  alt_name Secondary flag name. (optional)
         *  \return Help flag reference.
         */
        flag& help_flag(cstr name, cstr alt_name = "") {
            _help_flag = {name, alt_name};
            _help_flag.fhndl.doc("displays help");
            return _help_flag.fhndl;
        }


        /**
         *  \brief  Sets/activates the version flag.
         *  \see    flag
         *  \param  name Primary flag name.
         *  \param  alt_name Secondary flag name. (optional)
         *  \return Help flag reference.
         */
        flag& version_flag(cstr name, cstr alt_name = "") {
            _version_flag = {name, alt_name};
            _version_flag.fhndl.doc("displays version information");
            return _version_flag.fhndl;
        }


        /**
         *  \brief  Creates a documentation (help) of the application.
         *  \return Documentation.
         */
        inline std::string make_help() const noexcept {
            constexpr int space = 35;
            std::ostringstream help;

            if (not description().empty())
                help << "DESCRIPTION\n\t" << description() << "\n\n";



            help << "SYNOPSIS\n\t" << _app_name;

            for (auto [name, alt_name] : _option_names)
                if (_options.at(name)->req())
                   help << " " << (alt_name.empty() ? name : alt_name) << " " << _options.at(name)->value_info();

            for (auto [name, alt_name] : _flag_names)
                if (_options.at(name)->req())
                    help << " " << (alt_name.empty() ? name : alt_name);

            help << " [...]\n";




            

            help << "\nFLAGS\n";
            if (not _help_flag.name.empty()) {
                help << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (_help_flag.alt_name.empty() ? "" : _help_flag.alt_name + ", ") + _help_flag.name <<
                _help_flag.fhndl.doc() << "\n";
            }

            if (not _version_flag.name.empty()) {
                help << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (_version_flag.alt_name.empty() ? "" : _version_flag.alt_name + ", ") + _version_flag.name <<
                _version_flag.fhndl.doc() << "\n";
            }

            for (auto [name, alt_name] : _flag_names) {
                help << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (alt_name.empty() ? "" : alt_name + ", ") + name <<
                _options.at(name)->doc() << "\n";
            }


            help << "\nOPTIONS\n";
            for (auto [name, alt_name] : _option_names) {
                help << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (alt_name.empty() ? "" : alt_name + ", ") + name + " " +
                _options.at(name)->value_info()<<
                _options.at(name)->doc() << "\n";
            }


            if (not _license_notice.empty())
                help << "\nLICENSE\n\t" << _license_notice << "\n";

            if (not _author.empty())
                help << "\nAUTHOR\n\t" << _author << "\n";

            if (not _web_link.empty())
                help << "\n" << _web_link << "\n";

            return help.str();
        }


        /**
         *  \brief  Creates a version notice of the application.
         *  \return Version notice.
         */
        inline std::string make_version_info() const noexcept {
            return
                _app_name + " " + 
                _version + "\n" + 
                _author + "\n";
        }


        /**
         *  \brief Parses the command line input.
         *  \param argc Argument count.
         *  \param argv Arguments.
         */
        bool parse(int argc, char* argv[]) {
            uint32_t req_count = option_base::any_req;
            std::queue<std::string> args;
            for (int i = 1; i < argc; i++) // argv[0] is the command name, it is meant to be omitted
                args.push(argv[i]);


            if (args.size() == 1 && (args.front() == _help_flag.name || args.front() == _help_flag.alt_name)) {
                _help_flag.fhndl = true;
                return true;
            }

            if (args.size() == 1 && (args.front() == _version_flag.name || args.front() == _version_flag.alt_name)) {
                _version_flag.fhndl = true;
                return true;
            }


            while (not args.empty()) {
                if (_options.contains(args.front())) {
                    if (_options.at(args.front())->req()) req_count--;
                    
                    set_option(args);
                }
                else {
                    _wrong.emplace_back("Unkonown argument " + args.front());
                    return false;
                }
            }

            if (req_count) {
                _wrong.emplace_back("Missing required argument");
                return false;
            }

            return true;
        }



    private:
        /**
         *  \brief Parses value of an option/flag and catches errors.
         */
        inline void set_option(std::queue<std::string>& args) {
            std::shared_ptr<option_base>& opt = _options[args.front()];
            std::string temp_option_name = args.front();
            args.pop();

            if ( auto optFlag = std::dynamic_pointer_cast<flag>(opt) ) {
                *optFlag = true;
                return;
            }
            else if (args.empty()) {
                _wrong.emplace_back("Missing option value " + temp_option_name);
                return;
            }

            try {

            // TODO: implement the convertion properly 
            // if ( auto optString = std::dynamic_pointer_cast<option<std::string>>(opt) )
            //     *optString = args.front();
            
            // else if ( auto optInt = std::dynamic_pointer_cast<option<int>>(opt) )
            //     *optInt = std::stoi(args.front());
            
            // else if ( auto optFloat  = std::dynamic_pointer_cast<option<float>>(opt) )
            //     *optFloat = std::stof(args.front());
            
            // else if ( auto optChar = std::dynamic_pointer_cast<option<char>>(opt) )
            //     *optChar = args.front().front();
                
            args.pop();

            }
            catch (...) {
                _wrong.emplace_back("Value " + args.front() + " is not allowed " + temp_option_name);
                return;
            }
        }



    private:
        std::string _app_name;
        std::string _app_description;
        std::string _version;
        std::string _author;
        std::string _license_notice;
        std::string _web_link;

        info_flag _help_flag;
        info_flag _version_flag; 
        option_map _options;
        arg_name_map _option_names;
        arg_name_map _flag_names;
        std::vector<std::string> _wrong; ///< Contains all errors encountered while parsing.
    };

} // namespace CLI
