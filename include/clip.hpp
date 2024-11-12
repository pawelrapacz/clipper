#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <utility>


namespace CLI
{

    using cstr = const char*;

    class clip;


    template<typename T>
    concept valid_option_type = std::is_same_v<T, int> ||
                                std::is_same_v<T, float> ||
                                std::is_same_v<T, char> ||
                                std::is_same_v<T, std::string>;



    // allows casting option pointers
    class option_base {
    protected:
        friend class clip;
    
    public:
        virtual ~option_base() = default;
    };



    template<valid_option_type Tp>
    class option : public option_base {
        friend class clip;

        Tp* _ref = nullptr;
        std::string _doc;
        bool _req { false };
        bool _is_set{ false };

    public:
        option() = default;
        ~option() = default;


        
        option& set(Tp& ref) {
            _ref = &ref;
            return *this;
        }



        template<typename V>
        option& set(Tp& ref, V def) {
            static_assert(std::is_convertible<V, Tp>::value, "Type V must be convertible to type Tp");

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
            return *this;
        }
    };



    class flag {
        friend class clip;

        bool* _ref = nullptr;
        std::string _doc;
        bool _req { false };
        bool _is_set{ false };

    public:
        flag() = default;
        ~flag() = default;



        flag& set(bool& ref, bool def = false) {
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
            return *this;
        }

        // flag& print() {
        //     std::cout << _doc << _req << _is_set;
        //     return *this;
        // }
    };



    using option_map = std::unordered_map<std::string, std::shared_ptr<option_base>>;
    using flag_map = std::unordered_map<std::string, std::shared_ptr<flag>>;
    using info_flag = std::pair<std::string, std::string>;




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

        template<valid_option_type Tp>
        option<Tp>& add_option(cstr name) {
            _options[name] = std::make_shared<option<Tp>>();
            return *std::static_pointer_cast<option<Tp>>(_options[name]);
        }



        template<valid_option_type Tp>
        option<Tp>& add_option(cstr name, cstr alt_name) {
            _options[name] = std::make_shared<option<Tp>>();
            _options[alt_name] = _options[name];
            return *std::static_pointer_cast<option<Tp>>(_options[name]);
        }



        flag& add_flag(cstr name) {
            _flags[name] = std::make_shared<flag>();
            return *_flags[name];
        }



        flag& add_flag(cstr name, cstr alt_name) {
            _flags[name] = std::make_shared<flag>();
            _flags[alt_name] = _flags[name];
            return *_flags[name];
        }



        void help_flag(cstr name, cstr alt_name = "") {
            _help_flag = std::make_pair<std::string, std::string>(name, alt_name);
        }



        void version_flag(cstr name, cstr alt_name = "") {
            _version_flag = std::make_pair<std::string, std::string>(name, alt_name);
        }



        bool parse(int argc, char* argv[]) {
            if (1 == argc)
                return true;

            const std::string iarg { argv[1] };
            if (argc == 2 && (iarg == _help_flag.first || iarg == _help_flag.second)) {
                /* implement help */
                return true;
            }
            else if (argc == 2 && (iarg == _version_flag.first || iarg == _version_flag.second)) {
                std::cout << 
                _app_name << " " << 
                _version << "\n" << 
                "Author: " << _author << "\n" <<
                _license_notice << std::endl;
                return true;
            }


            for (int i = 1; i < argc; i++) {
                if (_options.contains(argv[i])) {
                    if ( auto optString = std::dynamic_pointer_cast<option<std::string>>(_options[argv[i]]) ) {
                        i = parse_option_string_values(argc, argv, i + 1, optString);
                        continue;
                    }
                    else if (auto optInt = std::dynamic_pointer_cast<option<int>>(_options[argv[i]]) )
                        *optInt->_ref = std::atoi(argv[i + 1]);

                    else if ( auto optFloat  = std::dynamic_pointer_cast<option<float>>(_options[argv[i]]) )
                        *optFloat->_ref = std::atof(argv[i + 1]);

                    else if ( auto optChar = std::dynamic_pointer_cast<option<char>>(_options[argv[i]]) )
                        *optChar->_ref = argv[i + 1][0];

                    i++;
                }
                else if (_flags.contains(argv[i])) {
                    *(_flags[argv[i]]->_ref) = true;
                }
                else {
                    return false;
                }
            }

            return true;
        }


        // const std::string help() const noexcept;
 

    private:
        // returns the index of last argument (value) that isn't a option or flag
        int parse_option_string_values(int argc, char** argv, int value_index, std::shared_ptr<option<std::string>> opt) {
            if (value_index < argc && not ( _options.contains(argv[value_index]) || _flags.contains(argv[value_index]) )) {
                *(opt->_ref) = argv[value_index];
                value_index++;
            }

            while (value_index < argc && not ( _options.contains(argv[value_index]) || _flags.contains(argv[value_index]) )) {
                opt->_ref->append(" ").append(argv[value_index]); // weird but it's c++ <3
                value_index++;
            }

            return value_index - 1;
        }


    private:


        std::string _app_name;
        std::string _app_description;
        std::string _synopsis;
        std::string _version;
        std::string _author;
        std::string _license_notice;
        std::string _web_link;

        info_flag _help_flag;
        info_flag _version_flag; 
        option_map _options;
        flag_map _flags;
    };



} // namespace CLI
