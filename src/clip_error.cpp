#include "clip.hpp"

const char* CLI::clip_error::name() const noexcept {
    return "clip error category";
}

std::string CLI::clip_error::message( int condition ) const {
    switch(condition) {
        default:
            return "Unknown error ocurred.";
    }
}

    
const std::error_category& CLI::clip_category() noexcept {
    static clip_error cat;
    return cat;
}

std::error_code CLI::make_error_code(errc e) {
    return {static_cast<int>(e), clip_category()};
}