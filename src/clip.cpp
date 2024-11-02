#include "clip.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <memory>
#include <system_error>
#include <expected>
#include <map>
    
namespace CLI {
/*
*
*   option class
*
*/
std::error_code option::convert() {
    static const std::map<cstr const, const bool> bool_allowed {{"true", true}, {"false", false}, {"1", true}, {"0", false}};

    switch (tp) {
    case type::STRING:
        break;
    case type::INT:
        try { std::stoi(str_value); }
        catch(std::exception e) { return make_error_code(errc::invalid_value); }
        break;

    case type::FLOAT:
        try { std::stof(str_value); }
        catch(std::exception e) { return make_error_code(errc::invalid_value); }
        break;

    case type::BOOL:
        try { bool_allowed.at(str_value.c_str()); }
        catch(std::exception e) { return make_error_code(errc::invalid_value); }
        break;
    }

    return make_error_code(errc::sucess);
}



/*
*
*   clip class
*
*/
clip::clip(cstr app_name)
    : _app_name(app_name) {}

clip::clip(cstr app_name, cstr version, cstr author, cstr license)
    : _app_name(app_name), _version(version), _author(author), _license(license) {}




void clip::name(cstr name) noexcept
{ _app_name = name; }

void clip::description(cstr description) noexcept
{ _app_description = description; }

void clip::synopsis(cstr synopsis) noexcept
{ _synopsis = synopsis; }

void clip::version(cstr version) noexcept
{ _version = version; }

void clip::author(cstr author) noexcept
{ _author = author; }

void clip::license(cstr license) noexcept
{ _license = license; }

void clip::web_link(cstr link) noexcept
{ _web_link = link; }


const std::string& clip::name() const noexcept
{ return _app_name; }

const std::string& clip::description() const noexcept 
{ return _app_description; }

const std::string& clip::synopsis() const noexcept 
{ return _synopsis; }

const std::string& clip::version() const noexcept 
{ return _version; }

const std::string& clip::author() const noexcept 
{ return _author; }

const std::string& clip::license() const noexcept 
{ return _license; }

const std::string& clip::web_link() const noexcept 
{ return _web_link; }





void clip::add_option(cstr name, cstr des, bool req, cstr def_val) {
    _options[name] = std::make_shared<option>(des, req, def_val);
    _onames[name];
}

void clip::add_option(cstr name, cstr alt_name, cstr des, bool req, cstr def_val) {
    _options[name] = std::make_shared<option>(des, req, def_val);
    _options[alt_name] = _options.at(name);
    _onames[name] = alt_name;
}

void clip::add_option(cstr name, type t, cstr des, bool req, cstr def_val) {
    _options[name] = std::make_shared<option>(des, req, def_val, t);
    _onames[name];
}

void clip::add_option(cstr name, cstr alt_name, type t, cstr des, bool req, cstr def_val) {
    _options[name] = std::make_shared<option>(des, req, def_val, t);
    _options[alt_name] = _options.at(name);
    _onames[name] = alt_name;
}

void clip::add_flag(cstr name, bool req, cstr des) {
    _flags[name] = std::make_shared<flag>(des, req);
    _fnames[name];
}

void clip::add_flag(cstr name, cstr alt_name, bool req, cstr des) {
    _flags[name] = std::make_shared<flag>(des, req);
    _flags[alt_name] = _flags.at(name);
    _fnames[name] = alt_name;
}

std::error_code clip::parse(int argc, char** argv) {
    // read the arguments and walidate them
    for (int i = 1; i < argc; i++) {
        if (_options.contains(argv[i])) {
            _options[argv[i]]->is_set = true;
            _options[argv[i]]->str_value = argv[i + 1];
            _options[argv[i]]->convert();
            _ocount++;
            i++; // beacause of the option value
        }
        else if (_flags.contains(argv[i])) {
            _flags[argv[i]]->is_set = true;
            _fcount++;
        }
        else if (_uap == unknown_arg_policy::error) {
            return make_error_code(errc::unknown_argument);
        }
    }



    for (const auto& [name, data] : _options) {
        if (data->is_req && (!data->is_set || data->str_value.empty()))
            return make_error_code(errc::missing_required_option);
    }

    for (const auto& [name, data] : _flags) {
        if (data->is_req && !data->is_set)
            return make_error_code(errc::missing_required_flag);
    }

    return make_error_code(errc::sucess);
}

std::size_t clip::count() const noexcept
{ return _ocount + _fcount; }

std::size_t clip::flag_count() const noexcept
{ return _fcount; }

std::size_t clip::option_count() const noexcept
{ return _ocount; }

bool clip::get_option(cstr name) const
{ return _options.at(name)->is_set; }

bool clip::get_flag(cstr name) const
{ return _flags.at(name)->is_set; }

const std::string& clip::get_option_value(cstr name) const
{ return _options.at(name)->str_value; }




const std::string clip::help() const noexcept {
    constexpr int offset = 40;
    std::ostringstream text;
    text << _app_name << " " << _version;

    if (!_app_description.empty())
        text << "\n\nDESCRIPTION\n\t" << _app_description;

    text << "\n\nSYNOPSIS\n\t" << _synopsis;

    if (!_options.empty()) {
        text << "\n\nOPTIONS\n";
        for (const auto& [name, alt_name] : _onames)
            text << '\t' << (alt_name.empty() ? "" : alt_name + ", ") + name << "\t\t" << _options.at(name)->description << '\n';
    }

    if (!_flags.empty()) {
        text << "\n\nFLAGS\n";
        for (const auto& [name, alt_name] : _fnames)
            text << '\t' << (alt_name.empty() ? "" : alt_name + ", ") + name << "\t\t" << _flags.at(name)->description << '\n';
    }

    text << "\n" << _web_link;

    return text.str();
}


} // namespace CLI