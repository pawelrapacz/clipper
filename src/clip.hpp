#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <system_error>
#include <expected>


namespace CLI
{
    using cstr = const char*;

    enum class unknown_arg_policy {
        ignore,
        error       // default
    };


    enum class type {
        INT,
        STRING,
        FLOAT,
        BOOL
    };


    struct option;
    struct flag;


    class clip
    {
    public:
        clip() = default;
        clip(cstr app_name);
        clip(cstr app_name, cstr version, cstr author, cstr license);

        ~clip() = default;

        /*
        *
        *   app info
        * 
        */
        void name(cstr name) noexcept;
        void description(cstr description) noexcept;
        void synopsis(cstr synopsis) noexcept;
        void version(cstr version) noexcept;
        void author(cstr author) noexcept;
        void license(cstr license) noexcept;
        void web_link(cstr link) noexcept;

        const std::string& name() const noexcept;
        const std::string& description() const noexcept;
        const std::string& synopsis() const noexcept;
        const std::string& version() const noexcept;
        const std::string& author() const noexcept;
        const std::string& license() const noexcept;
        const std::string& web_link() const noexcept;


        /*
        *
        *   argument handling and parsing
        * 
        */
        void add_option(cstr name, cstr des = "", bool req = false, cstr def_val = "");
        void add_option(cstr name, cstr alt_name, cstr des = "", bool req = false, cstr def_val = "");
        void add_option(cstr name, type t, cstr des = "", bool req = false, cstr def_val = "");
        void add_option(cstr name, cstr alt_name, type t, cstr des = "", bool req = false, cstr def_val = "");
        void add_flag(cstr name, bool req, cstr des = "");
        void add_flag(cstr name, cstr alt_name, bool req, cstr des = "");

        std::error_code parse(int argc, char** argv);
        bool get_option(cstr name) const;
        bool get_flag(cstr name) const;
        const std::string& get_option_value(cstr name) const;
        const std::string help() const noexcept;

        std::size_t count() const noexcept;
        std::size_t flag_count() const noexcept;
        std::size_t option_count() const noexcept;


    private:
        using option_map = std::unordered_map<std::string, std::shared_ptr<option>>;
        using flag_map = std::unordered_map<std::string, std::shared_ptr<flag>>;
        using name_map = std::map<std::string, std::string>;
        using count_t = unsigned int;
        

        /*
        *
        *   attributes
        * 
        */
        std::string _app_name;
        std::string _app_description;
        std::string _synopsis;
        std::string _version;
        std::string _author;
        std::string _license;
        std::string _web_link;
        unknown_arg_policy _uap = unknown_arg_policy::error;

        option_map _options;
        flag_map _flags;

        count_t _ocount{};
        count_t _fcount{};


        /*
        *
        *   _Xnames stores flag and option names and short names
        * 
        */
        name_map _onames;
        name_map _fnames;
    };

    struct option
    {
        std::error_code convert();
        std::string description;
        bool is_req;
        std::string str_value;
        type tp{type::STRING};
        bool is_set{ false };
        // union {
        //     int ival;
        //     float fval;
        //     bool bval;
        // };
    };

    struct flag
    {
        std::string description;
        bool is_req;
        bool is_set{ false };
    };





    /*
    *
    *   error handling
    * 
    */
    enum class errc {
        sucess,
        unknown_argument,
        missing_required_option,
        missing_required_flag,
        invalid_value,
        no_value,
    };

    class clip_error : public std::error_category {
        const char* name() const noexcept override;
        std::string message(int condition) const override;
    };
    
    const std::error_category& clip_category() noexcept;
    std::error_code make_error_code(errc e);
} // namespace CLI


namespace std {
    template<>
    struct is_error_code_enum<CLI::errc> : public true_type { };
}