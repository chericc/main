

#pragma once

#include <stdint.h>

#include <list>
#include <string>

std::list<std::string> xstring_split(const std::string& str,
                                     const std::string& token);

template <typename Char>
struct IsCompatibleCharTypeHelper
    : std::integral_constant<bool, std::is_same<Char, uint8_t>::value ||
                                       std::is_same<Char, int8_t>::value> {};

template <typename Char>
struct IsCompatibleCharType
    : IsCompatibleCharTypeHelper<typename std::remove_cv<
          typename std::remove_reference<Char>::type>::type> {};

class XStringView {
   public:
    typedef int8_t storage_type;
    typedef std::size_t size_type;

    template <typename Char>
    using if_compatible_char =
        typename std::enable_if<IsCompatibleCharType<Char>::value, bool>::type;

    XStringView() = default;
    XStringView(std::nullptr_t) : XStringView() {}

    template <typename Char, if_compatible_char<Char> = true>
    XStringView(const Char* str, size_type len) : m_data(str), m_size(len) {}

    const storage_type* m_data{nullptr};
    size_type m_size{0};
};