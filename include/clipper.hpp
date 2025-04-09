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
#include <filesystem>
#include <cuchar>



#ifndef CLIPPER_HELP_ARG_FIELD_WIDTH
    /// \brief Defines the width of the argument name field in help output.
    #define CLIPPER_HELP_ARG_FIELD_WIDTH    22
#endif



/**
 *  \brief Contains all the clipper utilities.
 */
namespace CLI
{
    /// \brief Checks whether a type is std::basic_string.
    template<typename>
    struct is_basic_string : public std::false_type { };

    /// \brief Checks whether a type is std::basic_string.
    template<typename CharT, typename Traits, typename Alloc>
    struct is_basic_string<std::basic_string<CharT, Traits, Alloc>>
    : public std::true_type { };

    /// \brief Alias to ::value property of is_basic_string.
    template<typename T>
    inline constexpr bool is_basic_string_v = is_basic_string<T>::value;
    
    /// \brief Checks whether a type is a character type.
    template<typename T>
    concept is_character = 
        std::is_same_v<T, char> ||
        std::is_same_v<T, wchar_t> ||
        std::is_same_v<T, char8_t> ||
        std::is_same_v<T, char16_t> ||
        std::is_same_v<T, char32_t> ||
        std::is_same_v<T, signed char> ||
        std::is_same_v<T, unsigned char>;

    /// \brief Checks whether a type is a string type
    template<typename T>
    concept is_string = 
        is_basic_string_v<T> ||
        std::is_same_v<T,std::filesystem::path>;

    /// \brief Allowed option types. (int, float, char, std::string)
    template<typename T>
    concept option_types = 
        std::negation_v<std::is_const<T>> && (
            std::is_integral_v<T>       ||
            std::is_floating_point_v<T> ||
            std::is_same_v<T, std::string> ||
            std::is_same_v<T, std::filesystem::path>
        );


    template<typename CharT, typename Traits = std::char_traits<CharT>,
             typename Alloc = std::allocator<CharT>>
    class basic_clipper;
    

    using native_char_type      = std::filesystem::path::value_type;
    using native_string_type    = std::filesystem::path::string_type;
    using native_clipper        = basic_clipper<native_char_type>;
    using clipper               = basic_clipper<char>;
    using wclipper              = basic_clipper<wchar_t>;
    using u8clipper             = basic_clipper<char8_t>;
    using u16clipper            = basic_clipper<char16_t>;
    using u32clipper            = basic_clipper<char32_t>;


    /**
     *  \brief Allows casting option pointers.
     *  \see option< Tp > option< booL > clipper
     */
    template<typename CharT, typename Traits, typename Alloc>
    class option_base {
        friend class basic_clipper<CharT, Traits, Alloc>;
    public:
        using string_type       = std::basic_string<CharT, Traits, Alloc>;
        using string_view_type  = std::basic_string_view<CharT, Traits>;

        /// \cond

        string_view_type _vname { "value" }; ///< Name of the type that the option holds.
        string_type _doc; ///< Documentation of the option
        bool _req { false }; ///< Stores information about optioin requirement

        inline static std::size_t any_req { }; ///< Holds the number of required options.


    // protected:
        /**
         * \brief Returns option synopsis in format: alt_name(or name) [value_info].
         * \return Option synopsis
         */
        string_type synopsis() const noexcept
        { return string_type(alt_name) + " " + value_info(); }
        
        /**
         * \brief Creates detailed option synopsis in format: [alt_name], name [value_info].
         * \return Detailed option synopsis
         */
        string_type detailed_synopsis() const noexcept
        { return (alt_name.empty() ? string_type() : string_type(alt_name) + ", ") + string_type(name) + " " + value_info(); }

        /**
         * \brief Creates option value info.
         * \return Option value info (empty by default)
         */
        virtual string_type value_info() const noexcept
        { return ""; };


        virtual void assign(string_view_type) = 0; ///< Converts and assigns a value to an option.
        virtual void operator=(string_view_type) = 0; ///< Converts and assigns a value to an option.
        
        /// \endcond

    public:
        const string_view_type name; ///< Reference to name of the option.
        const string_view_type alt_name; ///< Reference to alternative name of the option.


    public:
        /// \brief Constructs a new instance and sets its name reference.
        /// \brief Name and alternative name are the same.
        option_base(string_view_type nm)
            : name(nm) {}

        /// \brief Constructs a new instance and sets its name and alternative name reference.
        option_base(string_view_type nm, string_view_type anm)
            : name(nm), alt_name(anm) {}

        /// \brief Virtual default constructor.
        virtual ~option_base() = default;


        /**
         *  \brief  Accesses option documentation.
         *  \return Documentation reference.
         */
        const string_type& doc() const noexcept
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
    template<option_types Tp, typename CharT, typename Traits = std::char_traits<CharT>,
             typename Alloc = std::allocator<CharT>>
    class option
      : public option_base<CharT, Traits, Alloc>
      {
        friend class basic_clipper<CharT, Traits, Alloc>;

        using typename option_base<CharT, Traits, Alloc>::string_type;
        using typename option_base<CharT, Traits, Alloc>::string_view_type;

        using option_base<CharT, Traits, Alloc>::_vname;
        using option_base<CharT, Traits, Alloc>::_doc;
        using option_base<CharT, Traits, Alloc>::_req;
        using option_base<CharT, Traits, Alloc>::any_req;

    public:
        /// \brief Type of function that checks whether the given value meets some requirements
        /// \anchor optPredicate
        using predicate = bool (*)(const Tp&);
        using option_base<CharT, Traits, Alloc>::doc;

        /// \brief Constructs a new instance and sets its name reference.
        /// \brief Name and alternative name are the same.
        option(string_view_type nm)
            : option_base<CharT, Traits, Alloc>(nm) {}
        
        /// \brief Constructs a new instance and sets its name and alternative name reference.
        option(string_view_type nm, string_view_type anm)
            : option_base<CharT, Traits, Alloc>(nm, anm) {}

        /// \brief Default destructor.
        ~option() = default;


        /**
         *  \brief  Sets the variable to write to and the value name.
         *  \param  value_name Name of the value type e.g. file, charset.
         *  \param[out] ref Variable to write the option value to.
         *  \return Reference to itself.
         */
        option& set(string_view_type value_name, Tp& ref) {
            _vname = value_name;
            _ptr = &ref;
            *_ptr = Tp();
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
        option& set(string_view_type value_name, Tp& ref, V def) {
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
        option& validate(string_view_type doc, predicate pred) {
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
        option& require(string_view_type doc, predicate pred) {
            return validate(doc, pred);
        }
        
        /**
         *  \brief  Sets the option description.
         *  \param  doc Option information (documentation).
         *  \return Reference to itself.
         */
        option& doc(string_view_type doc) {
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
        string_type value_info() const noexcept override {
            if (_match_list.empty()) {
                return "<" + string_type(_vname) + ">";
            }
            else {
                string_type list;

                if constexpr (std::is_same_v<Tp, string_type>) {
                    for (Tp i : _match_list)
                        list.append(i).push_back(' ');
                }
                else if constexpr (std::is_same_v<Tp, std::filesystem::path>) {
                    for (Tp i : _match_list)
                        list.append(i.string()).push_back(' ');
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
        inline void assign(string_view_type val) override {
            if constexpr (is_string<Tp>) {

                *_ptr = val;
                if (!validate(*_ptr))
                    throw std::logic_error("Value is not allowed");


            } else if constexpr (is_character<Tp>) {

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
        inline void operator=(string_view_type val) override {
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
        // string_view_type _match_func_doc; ///< Documentation of the requirements of a \ref predicate function i.e. [0; 1], length < 10, lower case
        predicate _match_func = nullptr; ///< Function that checks wheather the value is allowed.
        std::set<Tp> _match_list;   ///< Contains allowed values (if empty all viable values are allowed).
    };


    /**
     *  \brief Contains flag properties (no argument option - boolean).
     *  \see   clipper clipper::add_flag() clipper::add_option()
     */
    template<typename CharT, typename Traits, typename Alloc>
    class option<bool, CharT, Traits, Alloc>
      : public option_base<CharT, Traits, Alloc> 
      {
        friend class basic_clipper<CharT, Traits, Alloc>;

        using typename option_base<CharT, Traits, Alloc>::string_type;
        using typename option_base<CharT, Traits, Alloc>::string_view_type;

        using option_base<CharT, Traits, Alloc>::_vname;
        using option_base<CharT, Traits, Alloc>::_doc;
        using option_base<CharT, Traits, Alloc>::_req;
        using option_base<CharT, Traits, Alloc>::any_req;

    public:
        using option_base<CharT, Traits, Alloc>::doc;

        /// \brief Constructs a new instance and sets its name reference.
        /// \brief Name and alternative name are the same.
        option(string_view_type nm)
            : option_base<CharT, Traits, Alloc>(nm) {}
        
        /// \brief Constructs a new instance and sets its name and alternative name reference.
        option(string_view_type nm, string_view_type anm)
            : option_base<CharT, Traits, Alloc>(nm, anm) {}

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
        option& doc(string_view_type doc) {
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
        inline void assign(string_view_type val) override {
            *_ptr = true;
        }


        /**
         *  \brief Converts and assigns a value to an option.
         *  \param val Value to assign.
         */
        inline void operator=(string_view_type val) override {
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
    template<typename CharT, typename Traits, typename Alloc>
    class basic_clipper {
    public:
        using string_type           = std::basic_string<CharT, Traits, Alloc>;
        using string_view_type      = std::basic_string_view<CharT, Traits>;
        using ostringstream_type    = std::basic_ostringstream<CharT, Traits, Alloc>;
        using option_base_type      = option_base<CharT, Traits, Alloc>;
          template<option_types Tp>
        using option_type           = option<Tp, CharT, Traits, Alloc>;
    private:
        using option_name_map       = std::unordered_map<string_view_type, std::size_t>; ///< Container for storing option names
        using option_vec            = std::vector<std::unique_ptr<option_base_type>>; ///< Container for storing options

    public:
        const std::vector<string_type>& wrong = _wrong; ///< Contains all errors encountered while parsing.
    
        /// \brief Default constructor.
        basic_clipper() = default;

        /// \brief Constructs a basic_clipper instance and sets the app name.
        basic_clipper(string_view_type app_name)
            : _app_name(app_name) {}

        /// \brief Constructs a basic_clipper instance and sets the app name and other information.
        basic_clipper(string_view_type app_name, string_view_type version, string_view_type author, string_view_type license_notice)
            : _app_name(app_name), _version(version), _author(author), _license_notice(license_notice) {}


        /// \brief Default destructor.
        ~basic_clipper() = default;


        /**
         *  \brief  Sets the (application) name.
         *  \return Reference to itself.
         */
        basic_clipper& name(string_view_type name) noexcept {
            _app_name = name;
            return *this;
        }

        /**
         *  \brief  Gets the (application) name.
         *  \return Application name.
         */
        string_view_type name() const noexcept {
            return _app_name;
        }


        /**
         *  \brief  Sets the description.
         *  \return Reference to itself.
         */
        basic_clipper& description(string_view_type description) noexcept {
            _app_description = description;
            return *this;
        }

        /**
         *  \brief 	Gets the description.
         *  \return Description reference.
         */
        string_view_type description() const noexcept {
            return _app_description;
        }


        /**
         *  \brief  Sets the version.
         *  \return Reference to itself.
         */
        basic_clipper& version(string_view_type version) noexcept {
            _version = version;
            return *this;
        }

        /**
         *  \brief  Gets the version.
         *  \return Version reference.
         */
        string_view_type version() const noexcept {
            return _version;
        }


        /**
         *  \brief  Sets the author.
         *  \return Reference to itself.
         */
        basic_clipper& author(string_view_type name) noexcept {
            _author = name;
            return *this;
        }

        /**
         *  \brief  Gets the author.
         *  \return Author reference.
         */
        string_view_type author() const noexcept {
            return _author;
        }


        /**
         *  \brief  Sets the license notice.
         *  \return Reference to itself.
         */
        basic_clipper& license(string_view_type license_notice) noexcept {
            _license_notice = license_notice;
            return *this;
        }

        /**
         *  \brief 	Gets the license notice.
         *  \return License notice reference.
         */
        string_view_type license() const noexcept {
            return _license_notice;
        }


        /**
         *  \brief  Sets the web link.
         *  \return Reference to itself.
         */
        basic_clipper& web_link(string_view_type link) noexcept {
            _web_link = link;
            return *this;
        }

        /**
         *  \brief  Gets the web link.
         *  \return Web link reference.
         */
        string_view_type web_link() const noexcept {
            return _web_link;
        }


        /**
         *  \brief  Adds an option of a given type.
         *  \see    option_types option< Tp >
         *  \tparam Tp Option (option value) type.
         *  \param  name Option name.
         *  \return Reference to the created option.
         */
        template<option_types Tp>
        option_type<Tp>& add_option(string_view_type name) {
            _names[name] = _options.size();
            _options.emplace_back(std::make_unique<option_type<Tp>>(name));
            return *static_cast<option_type<Tp>*>(_options.back().get());
        }


        /**
         *  \brief  Adds an option of a given type.
         *  \see    option_types option< Tp >
         *  \tparam Tp Option (option value) type.
         *  \param  name Primary option name.
         *  \param  alt_name Secondary option name.
         *  \return Reference to the created option.
         */
        template<option_types Tp>
        option_type<Tp>& add_option(string_view_type name, string_view_type alt_name) {
            _names[name] = _options.size();
            _names[alt_name] = _options.size();

            _options.emplace_back(std::make_unique<option_type<Tp>>(name, alt_name));

            return *static_cast<option_type<Tp>*>(_options.back().get());
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
        option_type<bool>& add_flag(string_view_type name) {
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
        option_type<bool>& add_flag(string_view_type name, string_view_type alt_name) {
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
        option_type<bool>& help_flag(string_view_type name, string_view_type alt_name = "") {
            _help_flag.name = name;
            _help_flag.alt_name = alt_name;
            _help_flag.hndl.doc("displays help");
            return _help_flag.hndl;
        }


        /// \copydoc help_flag
        option_type<bool>& version_flag(string_view_type name, string_view_type alt_name = "") {
            _version_flag.name = name;
            _version_flag.alt_name = alt_name;
            _version_flag.hndl.doc("displays version information");
            return _version_flag.hndl;
        }


        /**
         *  \brief  Creates a documentation (man page, help) for the application.
         *  \return Documentation.
         */
        inline string_type make_help() const noexcept {
            // auto add_help = [](const option_base_type* const opt, ostringstream_type& stream) {
            //     auto snps = opt->detailed_synopsis();

            //     if (CLIPPER_HELP_ARG_FIELD_WIDTH <= snps.length()) {
            //         stream << '\t' << snps << "\n\t" << string_type(CLIPPER_HELP_ARG_FIELD_WIDTH, ' ') << opt->doc() << '\n';
            //     }
            //     else
            //         stream << '\t' << std::left << std::setw(CLIPPER_HELP_ARG_FIELD_WIDTH) << snps << opt->doc() << '\n';
            // };

            // ostringstream_type flags;
            // ostringstream_type options;
            
            // if (_help_flag.is_set())
            //     add_help(&_help_flag.hndl, flags);

            // if (_version_flag.is_set())
            //     add_help(&_version_flag.hndl, flags);

            // for (auto& opt : _options) {
            //     if (dynamic_cast<option_type<bool>*>(opt.get()))
            //         add_help(opt.get(), flags);
            //     else
            //         add_help(opt.get(), options);
            // }



            // ostringstream_type help;

            // if (not _app_description.empty())
            //     help << "DESCRIPTION\n\t" << description() << "\n\n";
            
            // // SYNOPSIS
            // help << "SYNOPSIS\n\t" << _app_name;

            // for (auto& opt : _options)
            //     if (opt->req())
            //         help << " " << opt->synopsis();

            // help << " [...]\n";
            // // end SYNOPSIS

            // if (not flags.view().empty())
            //     help << "\nFLAGS\n" << flags.view();

            // if (not options.view().empty())
            //     help << "\nOPTIONS\n" << options.view();

            // if (not _license_notice.empty())
            //     help << "\nLICENSE\n\t" << _license_notice << "\n";

            // if (not _author.empty())
            //     help << "\nAUTHOR\n\t" << _author << "\n";

            // if (not _web_link.empty())
            //     help << "\n" << _web_link << "\n";

            // return help.str();
            return string_type();
        }
        
        
        /**
         *  \brief  Creates a version notice for the application.
         *  \return Version notice.
         */
        inline string_type make_version_info() const noexcept {
            // return
            //     string_type(_app_name).append(" ")
            //     .append(_version).append("\n")
            //     .append(_author).append("\n");
            return string_type();
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
        bool parse(int argc, const CharT* const* const argv) { /* <-- beautiful */
            _args_count = argc;
            bool err = false;
        

            if (_allow_no_args && argc == 1)
                return !err;

            auto req_count = option_base_type::any_req;
            std::queue<string_view_type> args;
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
                    _wrong.emplace_back("[" + string_type(args.front()) + "] Unkonown argument");
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
        inline void set_option(std::queue<string_view_type>& args, bool& error) {
            auto& opt = _options[_names[args.front()]];
            string_view_type temp_option_name = args.front();
            args.pop();

            if ( auto optFlag = dynamic_cast<option_type<bool>*>(opt.get()) ) {
                *optFlag = true;
            }
            else if (args.empty()) {
                _wrong.emplace_back("[" + string_type(temp_option_name) + "] Missing option value");
                error = true;
            }
            else {
                try { opt->assign(args.front()); }
                catch (...) {
                    _wrong.emplace_back("[" + string_type(temp_option_name) + "] Value " + string_type(args.front()) + " is not allowed \n\t{ " + opt->detailed_synopsis() + "  " + opt->doc() + " }");
                    error = true;
                }
                args.pop();
            }
        }



    private:
        string_view_type _app_name;
        string_view_type _app_description;
        string_view_type _version;
        string_view_type _author;
        string_view_type _license_notice;
        string_view_type _web_link;


        /// \brief Contains a \ref option<bool> "flag" information.
        /// \brief Primarly for version and help flags.
        struct {
            string_view_type name; ///< Name of the flag.
            string_view_type alt_name; ///< Alternative name of the flag.
            option_type<bool> hndl {name, alt_name}; ///< \ref option<bool> "Flag" handle.

            /**
             *  \brief Compares string with name and alt_name.
             *  \param str String to compare to
             *  \return True if the given string is equal to name or alt_name, false otherwise.
             */
            bool operator==(string_view_type str) const noexcept
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
        std::vector<string_type> _wrong; ///< Contains all errors encountered while parsing.
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
    inline bool between(const decltype(V1)& val) {
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
    inline bool ibetween(const decltype(V1)& val) {
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
    inline bool greater_than(const decltype(V)& val) {
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
    inline bool igreater_than(const decltype(V)& val) {
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
    inline bool less_than(const decltype(V)& val) {
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
    inline bool iless_than(const decltype(V)& val) {
        return V >= val;
    }

} // namespace CLI::pred