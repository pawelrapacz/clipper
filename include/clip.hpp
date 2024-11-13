/**
 * MIT License
 * Copyright (c) 2024 Paweł Rapacz
 *
 * See the LICENSE file in the root directory of this source tree for more information.
 */


#pragma once

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <cstring>
#include <map>
#include <unordered_map>
#include <queue>
#include <memory>
#include <type_traits>
#include <utility>


namespace CLI
{
    
    template<typename T>
    concept arg_types =
        std::is_same_v<T, int>  ||
        std::is_same_v<T, float>||
        std::is_same_v<T, char> ||
        std::is_same_v<T, bool> ||
        std::is_same_v<T, std::string>;



    template<typename T>
    concept option_types = 
        std::is_same_v<T, int>  ||
        std::is_same_v<T, float>||
        std::is_same_v<T, char> ||
        std::is_same_v<T, std::string>;


    class clip;

    class option_base;

    template<arg_types Tp>
    class option;


    using cstr = const char*;
    using flag = option<bool>;
    using info_flag = std::pair<std::string, std::string>;
    using arg_name_map = std::map<std::string, std::string>;
    using option_map = std::unordered_map<std::string, std::shared_ptr<option_base>>;
    using flag_map = std::unordered_map<std::string, std::shared_ptr<option<bool>>>;



    // allows casting option pointers
    class option_base {
    protected:
        friend class clip;

        cstr _vname;
        std::string _doc;
        bool _req { false };

        inline static bool any_req { false };

    public:
        virtual ~option_base() = default;
    };



    template<arg_types Tp>
    class option : public option_base {
        friend class clip;

        Tp* _ref = nullptr;

    public:
        option() = default;
        ~option() = default;


        option& set(Tp& ref) {
            _ref = &ref;
            *_ref = { };
            return *this;
        }



        template<typename V>
        option& set(Tp& ref, V def) {
            static_assert(std::is_convertible<V, Tp>::value, "Type V must be convertible to type Tp");

            _ref = &ref;
            *_ref = static_cast<Tp>(def);
            return *this;
        }


        
        option& set(cstr value_name, Tp& ref) {
            _vname = value_name;
            _ref = &ref;
            *_ref = { };
            return *this;
        }



        template<typename V>
        option& set(cstr value_name, Tp& ref, V def) {
            static_assert(std::is_convertible<V, Tp>::value, "Type V must be convertible to type Tp");

            _vname = value_name;
            _ref = &ref;
            *_ref = static_cast<Tp>(def);
            return *this;
        }



        option& doc(cstr doc) {
            _doc = doc;
            return *this;
        }



        option& req() {
            _req = true;
            any_req = true;
            return *this;
        }
    };



    class clip {
    public:
        clip() = default;


        clip(cstr app_name)
            : _app_name(app_name) {}


        clip(cstr app_name, cstr version, cstr author, cstr license_notice)
            : _app_name(app_name), _version(version), _author(author), _license_notice(license_notice) {}


        ~clip() = default;



        /*
        *
        *   app info
        * 
        */

        clip& name(cstr name) noexcept {
            _app_name = name;
            return *this;
        }


        const std::string& name() const noexcept {
            return _app_name;
        }



        clip& description(cstr description) noexcept {
            _app_description = description;
            return *this;
        }


        const std::string& description() const noexcept {
            return _app_description;
        }



        // clip& synopsis(cstr synopsis) noexcept;
        //  const std::string& synopsis() const noexcept;



        clip& version(cstr version) noexcept {
            _version = version;
            return *this;
        }


        const std::string& version() const noexcept {
            return _version;
        }



        clip& author(cstr name) noexcept {
            _author = name;
            return *this;
        }


        const std::string& author() const noexcept {
            return _author;
        }



        clip& license(cstr license_notice) noexcept {
            _license_notice = license_notice;
            return *this;
        }


        const std::string& license() const noexcept {
            return _license_notice;
        }



        clip& web_link(cstr link) noexcept {
            _web_link = link;
            return *this;
        }


        const std::string& web_link() const noexcept {
            return _web_link;
        }



        /*
        *
        *   argument handling and parsing
        * 
        */

        template<option_types Tp>
        option<Tp>& add_option(cstr name) {
            _options[name] = std::make_shared<option<Tp>>();
            _option_names[name];
            return *std::static_pointer_cast<option<Tp>>(_options[name]);
        }



        template<option_types Tp>
        option<Tp>& add_option(cstr name, cstr alt_name) {
            _options[name] = std::make_shared<option<Tp>>();
            _options[alt_name] = _options[name];
            _option_names[name] = alt_name;
            return *std::static_pointer_cast<option<Tp>>(_options[name]);
        }



        flag& add_flag(cstr name) {
            _flags[name] = std::make_shared<flag>();
            _flag_names[name];
            return *_flags[name];
        }



        flag& add_flag(cstr name, cstr alt_name) {
            _flags[name] = std::make_shared<flag>();
            _flags[alt_name] = _flags[name];
            _flag_names[name] = alt_name;
            return *_flags[name];
        }



        void help_flag(cstr name, cstr alt_name = "") {
            _help_flag = std::make_pair<std::string, std::string>(name, alt_name);
        }



        void version_flag(cstr name, cstr alt_name = "") {
            _version_flag = std::make_pair<std::string, std::string>(name, alt_name);
        }



        bool parse(int argc, char* argv[]) {
            std::queue<std::string> args;
            for (int i = 1; i < argc; i++) // argv[0] is the command name, it is meant to be omitted
                args.push(argv[i]);


            if (args.size() == 1 && (args.front() == _help_flag.first || args.front() == _help_flag.second)) {
                display_help();
                return true;
            }

            if (args.size() == 1 && (args.front() == _version_flag.first || args.front() == _version_flag.second)) {
                std::cout << 
                _app_name << " " << 
                _version << "\n" << 
                _author << "\n";
                return true;
            }

            if (args.size() < 2 && option_base::any_req)
                return false;


            while (not args.empty()) {
                if (_options.contains(args.front())) 
                    try { set_option(args); }
                    catch (std::exception e) { return false; }
                
                else if (_flags.contains(args.front()))
                    set_flag(args);
                
                else
                    return false;
            }

            return true;
        }



    private:
        inline void set_option(std::queue<std::string>& args) {
            std::shared_ptr<option_base>& opt = _options[args.front()];
            args.pop();
            
            if (args.empty())
                throw std::logic_error("Not enough arguments");


            if ( auto optString = std::dynamic_pointer_cast<option<std::string>>(opt) )
                *optString->_ref = args.front();
            
            else if ( auto optInt = std::dynamic_pointer_cast<option<int>>(opt) )
                *optInt->_ref = std::stoi(args.front());
            
            else if ( auto optFloat  = std::dynamic_pointer_cast<option<float>>(opt) )
                *optFloat->_ref = std::stof(args.front());
            
            else if ( auto optChar = std::dynamic_pointer_cast<option<char>>(opt) )
                *optChar->_ref = args.front().front();
            
            args.pop();
        }



        inline void set_flag(std::queue<std::string>& args) {
            *_flags[args.front()]->_ref = true;
            args.pop();
        }



        // returns the index of last argument (value) that isn't a option or flag
        // int parse_option_string_values(int argc, char** argv, int value_index, std::shared_ptr<option<std::string>> opt) {
        //     if (value_index < argc && not ( _options.contains(argv[value_index]) || _flags.contains(argv[value_index]) )) {
        //         *(opt->_ref) = argv[value_index];
        //         value_index++;
        //     }

        //     while (value_index < argc && not ( _options.contains(argv[value_index]) || _flags.contains(argv[value_index]) )) {
        //         opt->_ref->append(" ").append(argv[value_index]); // weird but it's c++ <3
        //         value_index++;
        //     }

        //     return value_index - 1;
        // }


        
        inline void display_help() const noexcept {
            constexpr int space = 30;


            auto synopsis = [&]()->std::string {
                std::string snp = _app_name + " ";

                if (not option_base::any_req)
                    return snp + "[...]";


                for (auto [name, alt_name] : _option_names)
                    if (_options.at(name)->_req)
                        snp += alt_name + " <" + _options.at(name)->_vname + "> ";

                for (auto [name, alt_name] : _flag_names)
                    if (_flags.at(name)->_req)
                        snp += alt_name + " ";

                return snp + "[...]";
            };



            if (not _app_description.empty())
                std::cout << "DESCRIPTION\n\t" << _app_description << "\n\n";


            std::cout << "SYNOPSIS\n\t" << synopsis() << "\n";
            

            std::cout << "\nFLAGS\n";
            if (not _help_flag.first.empty())
                std::cout << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (_help_flag.second.empty() ? "" : _help_flag.second + ", ") + _help_flag.first <<
                "displays help\n";

            if (not _version_flag.first.empty())
                std::cout << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (_version_flag.second.empty() ? "" : _version_flag.second + ", ") + _version_flag.first <<
                "displays version information\n";

            for (auto [name, alt_name] : _flag_names) {
                std::cout << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (alt_name.empty() ? "" : alt_name + ", ") + name <<
                _flags.at(name)->_doc << "\n";
            }


            std::cout << "\nOPTIONS\n";
            for (auto [name, alt_name] : _option_names) {
                std::cout << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (alt_name.empty() ? "" : alt_name + ", ") + name +
                " <" + _options.at(name)->_vname + ">" <<
                _options.at(name)->_doc << "\n";
            }


            if (not _license_notice.empty())
                std::cout << "\nLICENSE\n\t" << _license_notice << "\n";

            if (not _author.empty())
                std::cout << "\nAUTHOR\n\t" << _author << "\n";

            if (not _web_link.empty())
                std::cout << "\n" << _web_link << "\n";
        }



    private:
        std::string _app_name;
        std::string _app_description;
        // std::string _synopsis;
        std::string _version;
        std::string _author;
        std::string _license_notice;
        std::string _web_link;

        info_flag _help_flag;
        info_flag _version_flag; 
        option_map _options;
        flag_map _flags;
        arg_name_map _option_names;
        arg_name_map _flag_names;
    };

} // namespace CLI
