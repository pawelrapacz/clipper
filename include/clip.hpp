#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <unordered_map>
#include <memory>
#include <type_traits>


namespace CLI
{

    using cstr = const char*;


    class clip;


    template<typename T>
    concept valid_option_type = std::is_integral_v<T> ||
                                std::is_floating_point_v<T> ||
                                std::is_same_v<T, char> ||
                                std::is_same_v<T, std::string>;

    template<valid_option_type Tp>
    class option {
        Tp* _ref;
        std::string _doc;
        bool _req;
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
            static_assert(std::is_convertible<Tp, V>::value, "Type T must be implicitly convertible to type V");

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
        bool* _ref;
        std::string _doc;
        bool _req;
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



    using option_map = std::unordered_map<std::string, std::shared_ptr<void>>;
    using flag_map = std::unordered_map<std::string, std::shared_ptr<flag>>;
    using name_map = std::map<std::string, std::string>;





    class clip {
    public:
        clip() = default;


        clip(cstr app_name)
            : _app_name(app_name) {}


        clip(cstr app_name, cstr version, cstr author, cstr license)
            : _app_name(app_name), _version(version), _author(author), _license(license) {}


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



        clip& license(cstr license_info) noexcept {
            _license = license_info;
            return *this;
        }


        const std::string& license() const noexcept {
            return _license;
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



        // bool parse(int argc, char** argv) {

        // }


        // bool get_option(cstr name) const;
        // bool get_flag(cstr name) const;
        // const std::string help() const noexcept;


    private:

        std::string _app_name;
        std::string _app_description;
        std::string _synopsis;
        std::string _version;
        std::string _author;
        std::string _license;
        std::string _web_link;

        option_map _options;
        flag_map _flags;
    };



} // namespace CLI
