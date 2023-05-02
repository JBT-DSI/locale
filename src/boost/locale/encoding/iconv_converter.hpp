//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_LOCALE_IMPL_ICONV_CODEPAGE_HPP
#define BOOST_LOCALE_IMPL_ICONV_CODEPAGE_HPP

#include <boost/locale/encoding.hpp>
#include "boost/locale/encoding/conv.hpp"
#include "boost/locale/util/iconv.hpp"
#include <cerrno>
#include <string>

namespace boost { namespace locale { namespace conv { namespace impl {

    class iconverter_base {
    public:
        iconverter_base() : cvt_((iconv_t)(-1)) {}
        ~iconverter_base() { close(); }

        bool do_open(const char* to, const char* from, method_type how)
        {
            close();
            cvt_ = iconv_open(to, from);
            how_ = how;
            return cvt_ != (iconv_t)(-1);
        }

        template<typename OutChar, typename InChar>
        std::basic_string<OutChar> real_convert(const InChar* ubegin, const InChar* uend)
        {
            std::basic_string<OutChar> sresult;

            sresult.reserve(uend - ubegin);

            OutChar tmp_buf[64];

            char* out_start = reinterpret_cast<char*>(tmp_buf);
            const char* begin = reinterpret_cast<const char*>(ubegin);
            const char* end = reinterpret_cast<const char*>(uend);

            enum { normal, unshifting, done } state = normal;

            while(state != done) {
                size_t in_left = end - begin;
                size_t out_left = sizeof(tmp_buf);

                char* out_ptr = out_start;
                size_t res = 0;
                if(in_left == 0)
                    state = unshifting;

                if(state == normal)
                    res = conv(&begin, &in_left, &out_ptr, &out_left);
                else
                    res = conv(nullptr, nullptr, &out_ptr, &out_left);

                int err = errno;

                size_t output_count = (out_ptr - out_start) / sizeof(OutChar);

                if(res != 0 && res != (size_t)(-1)) {
                    if(how_ == stop) {
                        throw conversion_error();
                    }
                }

                sresult.append(tmp_buf, output_count);

                if(res == (size_t)(-1)) {
                    if(err == EILSEQ || err == EINVAL) {
                        if(how_ == stop) {
                            throw conversion_error();
                        }

                        if(begin != end) {
                            begin += sizeof(InChar);
                            if(begin >= end)
                                break;
                        } else {
                            break;
                        }
                    } else if(err == E2BIG) {
                        continue;
                    } else {
                        // We should never get there
                        // but if we do
                        if(how_ == stop)
                            throw conversion_error();
                        else
                            break;
                    }
                }
                if(state == unshifting)
                    state = done;
            }
            return sresult;
        }

    private:
        void close()
        {
            if(cvt_ != (iconv_t)(-1)) {
                iconv_close(cvt_);
                cvt_ = (iconv_t)(-1);
            }
        }

        size_t conv(const char** inbuf, size_t* inchar_left, char** outbuf, size_t* outchar_left)
        {
            return call_iconv(cvt_, inbuf, inchar_left, outbuf, outchar_left);
        }

        iconv_t cvt_;
        method_type how_;
    };

    template<typename CharType>
    class iconv_from_utf : public converter_from_utf<CharType> {
    public:
        bool open(const std::string& charset, method_type how) override
        {
            return self_.do_open(charset.c_str(), utf_name<CharType>(), how);
        }

        std::string convert(const CharType* ubegin, const CharType* uend) override
        {
            return self_.template real_convert<char>(ubegin, uend);
        }

    private:
        iconverter_base self_;
    };

    class iconv_between : public converter_between {
    public:
        bool open(const std::string& to_charset, const std::string& from_charset, method_type how) override
        {
            return self_.do_open(to_charset.c_str(), from_charset.c_str(), how);
        }
        std::string convert(const char* begin, const char* end) override
        {
            return self_.real_convert<char, char>(begin, end);
        }

    private:
        iconverter_base self_;
    };

    template<typename CharType>
    class iconv_to_utf : public converter_to_utf<CharType> {
    public:
        typedef std::basic_string<CharType> string_type;

        bool open(const std::string& charset, method_type how) override
        {
            return self_.do_open(utf_name<CharType>(), charset.c_str(), how);
        }

        string_type convert(const char* begin, const char* end) override
        {
            return self_.template real_convert<CharType>(begin, end);
        }

    private:
        iconverter_base self_;
    };

}}}} // namespace boost::locale::conv::impl

#endif
