//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
// Copyright (c) 2022-2023 Alexander Grund
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale/encoding.hpp>
#include <boost/locale/generator.hpp>
#include <algorithm>

#include "boostLocale/test/tools.hpp"
#include "boostLocale/test/unit_test.hpp"

const bool test_iso_8859_8 =
#if defined(BOOST_LOCALE_WITH_ICU) || defined(BOOST_LOCALE_WITH_ICONV)
  true;
#else
  hasWinCodepage(28598);
#endif

template<typename Char>
void test_to_from_utf(std::string source, std::basic_string<Char> target, std::string encoding)
{
    std::cout << "-- " << encoding << std::endl;
    const std::locale l = boost::locale::generator{}("en_US." + encoding);

    using boost::locale::conv::to_utf;
    TEST_EQ(to_utf<Char>(source, encoding), target);
    TEST_EQ(to_utf<Char>(source.c_str(), encoding), target);
    TEST_EQ(to_utf<Char>(source.c_str(), source.c_str() + source.size(), encoding), target);

    TEST_EQ(to_utf<Char>(source, l), target);
    TEST_EQ(to_utf<Char>(source.c_str(), l), target);
    TEST_EQ(to_utf<Char>(source.c_str(), source.c_str() + source.size(), l), target);

    using boost::locale::conv::from_utf;
    TEST_EQ(from_utf<Char>(target, encoding), source);
    TEST_EQ(from_utf<Char>(target.c_str(), encoding), source);
    TEST_EQ(from_utf<Char>(target.c_str(), target.c_str() + target.size(), encoding), source);

    TEST_EQ(from_utf<Char>(target, l), source);
    TEST_EQ(from_utf<Char>(target.c_str(), l), source);
    TEST_EQ(from_utf<Char>(target.c_str(), target.c_str() + target.size(), l), source);
}

#define TEST_FAIL_CONVERSION(X) TEST_THROWS(X, boost::locale::conv::conversion_error)

template<typename Char>
void test_error_to_utf(std::string source, std::basic_string<Char> target, std::string encoding)
{
    using namespace boost::locale::conv;
    boost::locale::generator g;
    std::locale l = g("en_US." + encoding);

    TEST_EQ(to_utf<Char>(source, encoding), target);
    TEST_EQ(to_utf<Char>(source.c_str(), encoding), target);
    TEST_EQ(to_utf<Char>(source.c_str(), source.c_str() + source.size(), encoding), target);
    TEST_EQ(to_utf<Char>(source, l), target);
    TEST_EQ(to_utf<Char>(source.c_str(), l), target);
    TEST_EQ(to_utf<Char>(source.c_str(), source.c_str() + source.size(), l), target);

    TEST_FAIL_CONVERSION(to_utf<Char>(source, encoding, stop));
    TEST_FAIL_CONVERSION(to_utf<Char>(source.c_str(), encoding, stop));
    TEST_FAIL_CONVERSION(to_utf<Char>(source.c_str(), source.c_str() + source.size(), encoding, stop));
    TEST_FAIL_CONVERSION(to_utf<Char>(source, l, stop));
    TEST_FAIL_CONVERSION(to_utf<Char>(source.c_str(), l, stop));
    TEST_FAIL_CONVERSION(to_utf<Char>(source.c_str(), source.c_str() + source.size(), l, stop));
}

template<typename Char>
void test_error_from_utf(std::basic_string<Char> source, std::string target, std::string encoding)
{
    using namespace boost::locale::conv;
    boost::locale::generator g;
    std::locale l = g("en_US." + encoding);

    TEST_EQ(from_utf<Char>(source, encoding), target);
    TEST_EQ(from_utf<Char>(source.c_str(), encoding), target);
    TEST_EQ(from_utf<Char>(source.c_str(), source.c_str() + source.size(), encoding), target);
    TEST_EQ(from_utf<Char>(source, l), target);
    TEST_EQ(from_utf<Char>(source.c_str(), l), target);
    TEST_EQ(from_utf<Char>(source.c_str(), source.c_str() + source.size(), l), target);

    TEST_FAIL_CONVERSION(from_utf<Char>(source, encoding, stop));
    TEST_FAIL_CONVERSION(from_utf<Char>(source.c_str(), encoding, stop));
    TEST_FAIL_CONVERSION(from_utf<Char>(source.c_str(), source.c_str() + source.size(), encoding, stop));
    TEST_FAIL_CONVERSION(from_utf<Char>(source, l, stop));
    TEST_FAIL_CONVERSION(from_utf<Char>(source.c_str(), l, stop));
    TEST_FAIL_CONVERSION(from_utf<Char>(source.c_str(), source.c_str() + source.size(), l, stop));
}

template<typename Char>
std::basic_string<Char> utf(const std::string& s)
{
    return to<Char>(s);
}

template<>
std::basic_string<char> utf(const std::string& s)
{
    return s;
}

template<typename Char>
void test_with_0()
{
    const char with_null[] = "foo\0\0 of\0";
    const std::string s_with_null(with_null, sizeof(with_null) - 1);
    const std::basic_string<Char> s_with_null2 = ascii_to<Char>(with_null);
    TEST_EQ(boost::locale::conv::to_utf<Char>(s_with_null, "UTF-8"), s_with_null2);
    TEST_EQ(boost::locale::conv::to_utf<Char>(s_with_null, "ISO8859-1"), s_with_null2);
    TEST_EQ(boost::locale::conv::from_utf<Char>(s_with_null2, "UTF-8"), s_with_null);
    TEST_EQ(boost::locale::conv::from_utf<Char>(s_with_null2, "ISO8859-1"), s_with_null);
}

template<typename Char, int n = sizeof(Char)>
struct utfutf;

#ifdef BOOST_MSVC
#    pragma warning(push)
#    pragma warning(disable : 4309) // narrowing static_cast warning
#endif
template<>
struct utfutf<char, 1> {
    static const char* ok() { return "grüßen"; }
    static const char* bad()
    {
        return "gr\xFF"
               "üßen";
        // split into 2 to make SunCC happy
    }
    static char bad_char() { return static_cast<char>(0xFF); }
};

template<>
struct utfutf<wchar_t, 2> {
    static const wchar_t* ok() { return L"\x67\x72\xfc\xdf\x65\x6e"; }
    static const wchar_t* bad()
    {
        static wchar_t buf[256] = L"\x67\x72\xFF\xfc\xFE\xFD\xdf\x65\x6e";
        buf[2] = 0xDC01; // second surrogate must not be
        buf[4] = 0xD801; // First
        buf[5] = 0xD801; // Must be surrogate trail
        return buf;
    }
    static wchar_t bad_char() { return static_cast<wchar_t>(0xDC01); }
};

template<>
struct utfutf<wchar_t, 4> {
    static const wchar_t* ok() { return L"\x67\x72\xfc\xdf\x65\x6e"; }
    static const wchar_t* bad()
    {
        static wchar_t buf[256] = L"\x67\x72\xFF\xfc\xdf\x65\x6e";
        buf[2] = static_cast<wchar_t>(0x1000000); // > 10FFFF
        return buf;
    }
    static wchar_t bad_char() { return static_cast<wchar_t>(0x1000000); }
};
#ifdef BOOST_MSVC
#    pragma warning(pop)
#endif

template<typename CharOut, typename CharIn>
void test_combinations()
{
    using boost::locale::conv::utf_to_utf;
    typedef utfutf<CharOut> out;
    typedef utfutf<CharIn> in;
    TEST((utf_to_utf<CharOut, CharIn>(in::ok()) == out::ok()));
    TEST_FAIL_CONVERSION((utf_to_utf<CharOut, CharIn>(in::bad(), boost::locale::conv::stop)));
    TEST((utf_to_utf<CharOut, CharIn>(in::bad()) == out::ok()));
}

void test_all_combinations()
{
    std::cout << "Testing utf_to_utf\n";
    std::cout << "  char<-char" << std::endl;
    test_combinations<char, char>();
    std::cout << "  char<-wchar" << std::endl;
    test_combinations<char, wchar_t>();
    std::cout << "  wchar<-char" << std::endl;
    test_combinations<wchar_t, char>();
    std::cout << "  wchar<-wchar" << std::endl;
    test_combinations<wchar_t, wchar_t>();
}

template<typename Char>
void test_utf_for()
{
    using boost::locale::conv::invalid_charset_error;

    test_to_from_utf<Char>(to<char>("grüßen"), utf<Char>("grüßen"), "ISO8859-1");
    if(test_iso_8859_8)
        test_to_from_utf<Char>("\xf9\xec\xe5\xed", utf<Char>("שלום"), "ISO8859-8");
    test_to_from_utf<Char>("grüßen", utf<Char>("grüßen"), "UTF-8");
    test_to_from_utf<Char>("abc\"\xf0\xa0\x82\x8a\"", utf<Char>("abc\"\xf0\xa0\x82\x8a\""), "UTF-8");
    // Testing a codepage which may be an issue on Windows, see issue #121
    try {
        test_to_from_utf<Char>("\x1b$BE_5(\x1b(B", utf<Char>("冬季"), "iso-2022-jp");
    } catch(const invalid_charset_error&) { // LCOV_EXCL_LINE
        std::cout << "--- not supported\n"; // LCOV_EXCL_LINE
    }

    std::cout << "- Testing correct invalid bytes skipping\n";
    {
        std::cout << "-- UTF-8" << std::endl;

        // At start
        test_error_to_utf<Char>("\xFFgrüßen", utf<Char>("grüßen"), "UTF-8");
        test_error_to_utf<Char>("\xFF\xFFgrüßen", utf<Char>("grüßen"), "UTF-8");
        // Middle
        test_error_to_utf<Char>("g\xFFrüßen", utf<Char>("grüßen"), "UTF-8");
        test_error_to_utf<Char>("g\xFF\xFF\xFFrüßen", utf<Char>("grüßen"), "UTF-8");
        // End
        test_error_to_utf<Char>("grüßen\xFF", utf<Char>("grüßen"), "UTF-8");
        test_error_to_utf<Char>("grüßen\xFF\xFF", utf<Char>("grüßen"), "UTF-8");

        try {
            std::cout << "-- ISO-8859-8" << std::endl;
            test_error_to_utf<Char>("\xFB", utf<Char>(""), "ISO-8859-8");
            test_error_to_utf<Char>("\xFB-", utf<Char>("-"), "ISO-8859-8");
            test_error_to_utf<Char>("test \xE0\xE1\xFB", utf<Char>("test \xd7\x90\xd7\x91"), "ISO-8859-8");
            test_error_to_utf<Char>("test \xE0\xE1\xFB-", utf<Char>("test \xd7\x90\xd7\x91-"), "ISO-8859-8");
        } catch(const invalid_charset_error&) { // LCOV_EXCL_LINE
            std::cout << "--- not supported\n"; // LCOV_EXCL_LINE
        }
        try {
            std::cout << "-- cp932" << std::endl;
            test_error_to_utf<Char>("\x83\xF8", utf<Char>(""), "cp932");
            test_error_to_utf<Char>("\x83\xF8-", utf<Char>("-"), "cp932");
            test_error_to_utf<Char>("test\xE0\xA0 \x83\xF8", utf<Char>("test\xe7\x87\xbf "), "cp932");
            test_error_to_utf<Char>("test\xE0\xA0 \x83\xF8-", utf<Char>("test\xe7\x87\xbf -"), "cp932");
        } catch(const invalid_charset_error&) { // LCOV_EXCL_LINE
            std::cout << "--- not supported\n"; // LCOV_EXCL_LINE
        }
        // Error for encoding at start, middle and end
        test_error_from_utf<Char>(utf<Char>("שלום hello"), " hello", "ISO8859-1");
        test_error_from_utf<Char>(utf<Char>("hello שלום world"), "hello  world", "ISO8859-1");
        test_error_from_utf<Char>(utf<Char>("hello שלום"), "hello ", "ISO8859-1");
        // Error for decoding
        test_error_from_utf<Char>(utfutf<Char>::bad(), utfutf<char>::ok(), "UTF-8");
        test_error_from_utf<Char>(utfutf<Char>::bad(), to<char>(utfutf<char>::ok()), "Latin1");
        const std::basic_string<Char> onlyInvalidUtf(2, utfutf<Char>::bad_char()); // Consists only of invalid chars
        test_error_from_utf<Char>(onlyInvalidUtf, "", "UTF-8");
        test_error_from_utf<Char>(onlyInvalidUtf, "", "Latin1");
    }

    test_with_0<Char>();
}

template<typename Char1, typename Char2>
void test_utf_to_utf_for(const std::string& utf8_string)
{
    const auto utf_string1 = utf<Char1>(utf8_string);
    const auto utf_string2 = utf<Char2>(utf8_string);
    using boost::locale::conv::utf_to_utf;
    TEST_EQ(utf_to_utf<Char1>(utf_string2), utf_string1);
    TEST_EQ(utf_to_utf<Char2>(utf_string1), utf_string2);
    TEST_EQ(utf_to_utf<Char1>(utf_string1), utf_string1);
    TEST_EQ(utf_to_utf<Char2>(utf_string2), utf_string2);
}

template<typename Char>
void test_utf_to_utf_for()
{
    const std::string& utf8_string = "A-Za-z0-9grüße'\xf0\xa0\x82\x8a'\xf4\x8f\xbf\xbf";
    std::cout << "---- char\n";
    test_utf_to_utf_for<Char, char>(utf8_string);
    std::cout << "---- wchar_t\n";
    test_utf_to_utf_for<Char, wchar_t>(utf8_string);
#ifdef BOOST_LOCALE_ENABLE_CHAR16_T
    std::cout << "---- char16_t\n";
    test_utf_to_utf_for<Char, char16_t>(utf8_string);
#endif
#ifdef BOOST_LOCALE_ENABLE_CHAR32_T
    std::cout << "---- char32_t\n";
    test_utf_to_utf_for<Char, char32_t>(utf8_string);
#endif
}

void test_utf_to_utf()
{
    std::cout << "- Testing UTF to UTF conversion\n";
    std::cout << "-- char\n";
    test_utf_to_utf_for<char>();
    std::cout << "-- wchar_t\n";
    test_utf_to_utf_for<wchar_t>();
#ifdef BOOST_LOCALE_ENABLE_CHAR16_T
    std::cout << "-- char16_t\n";
    test_utf_to_utf_for<char16_t>();
#endif
#ifdef BOOST_LOCALE_ENABLE_CHAR32_T
    std::cout << "-- char32_t\n";
    test_utf_to_utf_for<char32_t>();
#endif
}

template<typename Char>
void test_latin1_conversions_for()
{
    const std::string utf8_string = "A-Za-z0-9grüße";
    const std::string latin1_string = to<char>(utf8_string);
    const std::basic_string<Char> wide_string = utf<Char>(utf8_string);

    using namespace boost::locale::conv;
    TEST_EQ(to_utf<Char>(latin1_string, "Latin1"), wide_string);
    TEST_EQ(from_utf(wide_string, "Latin1"), latin1_string);

    // Empty string doesn't error/assert
    TEST_EQ(to_utf<Char>("", encoding), utf<Char>(""));
    TEST_EQ(from_utf<Char>(utf<Char>(""), encoding), std::string());
}

void test_latin1_conversions()
{
    std::cout << "- Testing Latin1 conversion\n";
    std::cout << "-- char\n";
    test_latin1_conversions_for<char>();
    std::cout << "-- wchar_t\n";
    test_latin1_conversions_for<wchar_t>();
#ifdef BOOST_LOCALE_ENABLE_CHAR16_T
    std::cout << "-- char16_t\n";
    test_latin1_conversions_for<char16_t>();
#endif
#ifdef BOOST_LOCALE_ENABLE_CHAR32_T
    std::cout << "-- char32_t\n";
    test_latin1_conversions_for<char32_t>();
#endif
}

void test_utf_name();
void test_win_codepages();

void test_main(int /*argc*/, char** /*argv*/)
{
    // Sanity check to<char>
    TEST_EQ(to<char>("grüßen"),
            "gr\xFC\xDF"
            "en");
    TEST_THROWS(to<char>("€"), std::logic_error);
    // Sanity check internal details
    test_utf_name();
    test_win_codepages();

    test_latin1_conversions();
    test_utf_to_utf();

    std::cout << "Testing charset to/from UTF conversion functions\n";
    std::cout << "  char" << std::endl;
    test_utf_for<char>();
    std::cout << "  wchar_t" << std::endl;
    test_utf_for<wchar_t>();
#ifdef BOOST_LOCALE_ENABLE_CHAR16_T
    if(backendName == "icu" || backendName == "std") {
        std::cout << "  char16_t" << std::endl;
        test_utf_for<char16_t>();
    }
#endif
#ifdef BOOST_LOCALE_ENABLE_CHAR32_T
    if(backendName == "icu" || backendName == "std") {
        std::cout << "  char32_t" << std::endl;
        test_utf_for<char32_t>();
    }
#endif

    test_all_combinations();
}

// Internal tests, keep those out of the above scope

bool isLittleEndian()
{
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
    return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#elif defined(__LITTLE_ENDIAN__)
    return true;
#elif defined(__BIG_ENDIAN__)
    return false;
#endif
    const int endianMark = 1;
    return reinterpret_cast<const char*>(&endianMark)[0] == 1;
}

#include "../src/boost/locale/util/encoding.hpp"
#include "../src/boost/locale/util/win_codepages.hpp"

void test_utf_name()
{
    TEST_EQ(boost::locale::util::utf_name<char>(), std::string("UTF-8"));
#ifdef __cpp_char8_t
    TEST_EQ(boost::locale::util::utf_name<char8_t>(), std::string("UTF-8"));
#endif
    TEST_EQ(boost::locale::util::utf_name<char16_t>(), std::string(isLittleEndian() ? "UTF-16LE" : "UTF-16BE"));
    TEST_EQ(boost::locale::util::utf_name<char32_t>(), std::string(isLittleEndian() ? "UTF-32LE" : "UTF-32BE"));
}

void test_win_codepages()
{
    using namespace boost::locale::util;

    for(const windows_encoding *it = all_windows_encodings, *end = std::end(all_windows_encodings); it != end; ++it) {
        TEST_EQ(normalize_encoding(it->name), it->name); // Must be normalized
        auto is_same_win_codepage = [&it](const windows_encoding& rhs) -> bool {
            return it->codepage == rhs.codepage && std::strcmp(it->name, rhs.name) == 0;
        };
        const auto* it2 = std::find_if(it + 1, end, is_same_win_codepage);
        TEST(it2 == end);
        if(it2 != end)
            std::cerr << "Duplicate entry: " << it->name << ':' << it->codepage << '\n'; // LCOV_EXCL_LINE
    }
    const auto cmp = [](const windows_encoding& rhs, const windows_encoding& lhs) -> bool { return rhs < lhs.name; };
    const auto* it = std::is_sorted_until(all_windows_encodings, std::end(all_windows_encodings), cmp);
    TEST(it == std::end(all_windows_encodings));
    if(it != std::end(all_windows_encodings))
        std::cerr << "First wrongly sorted element: " << it->name << '\n'; // LCOV_EXCL_LINE
}

// boostinspect:noascii
