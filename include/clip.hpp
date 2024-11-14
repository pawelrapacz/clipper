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
#include <vector>
#include <set>
#include <memory>
#include <type_traits>
#include <utility>


namespace CLI
{
    template<typename T>
    concept option_types = 
        std::is_same_v<T, int>  ||
        std::is_same_v<T, float>||
        std::is_same_v<T, char> ||
        std::is_same_v<T, std::string>;



    class clip;

    class option_base;

    template<option_types Tp>
    class option;

    class flag;


    using cstr = const char*;
    using info_flag = std::pair<std::string, std::string>;
    using arg_name_map = std::map<std::string, std::string>;
    using option_map = std::unordered_map<std::string, std::shared_ptr<option_base>>;
    using flag_map = std::unordered_map<std::string, std::shared_ptr<flag>>;



    // allows casting option pointers
    class option_base {
    protected:
        friend class clip;

        std::string _vname;
        std::string _doc;
        bool _req { false };

        inline static uint32_t any_req { };

    protected:
        virtual std::string value_info() const noexcept = 0;

    public:
        virtual ~option_base() = default;
    };



    template<option_types Tp>
    class option : public option_base {
        friend class clip;

        Tp* _ref = nullptr;
        std::set<Tp> _match_list;



    protected:
        inline void operator=(Tp val) {
            if (_match_list.empty() || _match_list.contains(val)) {
                *_ref = val;
            }
            else {
                throw std::logic_error("Value is not allowed");
            }
        }



        std::string value_info() const noexcept override {
            if (_match_list.empty()) {
                return _vname;
            }
            else {
                std::string list;

                for (Tp i : _match_list)
                    list += i + " ";

                list.pop_back();
                return "[" + list + "]";
            }
        }



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


        template<typename... Args>
        option& match(Args... val) {
            static_assert((std::is_convertible<std::decay_t<Args>, Tp>::value && ...), "All arguments must be of type Tp");
            (_match_list.insert(std::forward<Args>(val)), ... );
            return *this;
        }



        option& doc(cstr doc) {
            _doc = doc;
            return *this;
        }



        option& req() {
            _req = true;
            any_req++;
            return *this;
        }
    };



    class flag : public option_base {
        friend class clip;

        bool* _ref = nullptr;

    protected:
        std::string value_info() const noexcept override { return ""; } // delete - unused function


    public:
        flag() = default;
        ~flag() = default;


        flag& set(bool& ref) {
            _ref = &ref;
            *_ref = { };
            return *this;
        }



        flag& set(bool& ref, bool def) {
            _ref = &ref;
            *_ref = def;
            return *this;
        }



        flag& doc(cstr doc) {
            _doc = doc;
            return *this;
        }



        flag& req() {
            _req = true;
            any_req++;
            return *this;
        }
    };



    class clip {
    public:
        const std::vector<std::string>& wrong = _wrong;

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
            uint32_t req_count = option_base::any_req;
            std::queue<std::string> args;
            for (int i = 1; i < argc; i++) // argv[0] is the command name, it is meant to be omitted
                args.push(argv[i]);


            if (args.size() == 1 && (args.front() == _help_flag.first || args.front() == _help_flag.second)) {
                display_help();
                return false;
            }

            if (args.size() == 1 && (args.front() == _version_flag.first || args.front() == _version_flag.second)) {
                std::cout << 
                _app_name << " " << 
                _version << "\n" << 
                _author << "\n";
                return false;
            }


            while (not args.empty()) {
                if (_options.contains(args.front())) {
                    if (_options.at(args.front())->_req) req_count--;
                    
                    set_option(args);
                }
                
                else if (_flags.contains(args.front())) {
                    if (_flags.at(args.front())->_req) req_count--;

                    set_flag(args);
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
        inline void set_option(std::queue<std::string>& args) {
            std::shared_ptr<option_base>& opt = _options[args.front()];
            std::string temp_option_name = args.front();
            args.pop();
            
            if (args.empty()) {
                _wrong.emplace_back("Missing option value " + temp_option_name);
                return;
            }

            try {

            if ( auto optString = std::dynamic_pointer_cast<option<std::string>>(opt) )
                *optString = args.front();
            
            else if ( auto optInt = std::dynamic_pointer_cast<option<int>>(opt) )
                *optInt = std::stoi(args.front());
            
            else if ( auto optFloat  = std::dynamic_pointer_cast<option<float>>(opt) )
                *optFloat = std::stof(args.front());
            
            else if ( auto optChar = std::dynamic_pointer_cast<option<char>>(opt) )
                *optChar = args.front().front();
                
            args.pop();

            }
            catch (...) {
                _wrong.emplace_back("Value " + args.front() + " is not allowed " + temp_option_name);
                return;
            }
        }



        inline void set_flag(std::queue<std::string>& args) {
            *_flags[args.front()]->_ref = true;
            args.pop();
        }


        
        inline void display_help() const noexcept {
            constexpr int space = 35;

            auto synopsis = [&]()->std::string {
                std::string snp = _app_name + " ";

                if (not option_base::any_req)
                    return snp + "[...]";


                for (auto [name, alt_name] : _option_names)
                    if (_options.at(name)->_req)
                        snp += alt_name + " <" + _options.at(name)->value_info() + "> ";

                for (auto [name, alt_name] : _flag_names)
                    if (_flags.at(name)->_req)
                        snp += alt_name + " ";

                return snp + "[...]";
            };



            if (not _app_description.empty())
                std::cout << "DESCRIPTION\n\t" << _app_description << "\n\n";


            std::cout << "SYNOPSIS\n\t" << synopsis() << "\n";
            

            std::cout << "\nFLAGS\n";
            if (not _help_flag.first.empty()) {
                std::cout << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (_help_flag.second.empty() ? "" : _help_flag.second + ", ") + _help_flag.first <<
                "displays help\n";
            }

            if (not _version_flag.first.empty()) {
                std::cout << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (_version_flag.second.empty() ? "" : _version_flag.second + ", ") + _version_flag.first <<
                "displays version information\n";
            }

            for (auto [name, alt_name] : _flag_names) {
                std::cout << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (alt_name.empty() ? "" : alt_name + ", ") + name <<
                _flags.at(name)->_doc << "\n";
            }


            std::cout << "\nOPTIONS\n";
            for (auto [name, alt_name] : _option_names) {
                std::cout << "\t" << std::left << std::setw(space) << std::setfill(' ') <<
                (alt_name.empty() ? "" : alt_name + ", ") + name + " " +
                _options.at(name)->value_info()<<
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
        std::vector<std::string> _wrong;
    };

} // namespace CLI
