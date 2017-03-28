/*
    __ _____ _____ _____
 __|  |   __|     |   | |  JSON for Modern C++ (test suite)
|  |  |__   |  |  | | | |  version 2.1.1
|_____|_____|_____|_|___|  https://github.com/nlohmann/json

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2013-2017 Niels Lohmann <http://nlohmann.me>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "catch.hpp"

#define private public
#include "json.hpp"
using nlohmann::json;

TEST_CASE("lexer class")
{
    SECTION("scan")
    {
        SECTION("structural characters")
        {
            CHECK((json::lexer("[", 1).scan() == json::lexer::token_type::begin_array));
            CHECK((json::lexer("]", 1).scan() == json::lexer::token_type::end_array));
            CHECK((json::lexer("{", 1).scan() == json::lexer::token_type::begin_object));
            CHECK((json::lexer("}", 1).scan() == json::lexer::token_type::end_object));
            CHECK((json::lexer(",", 1).scan() == json::lexer::token_type::value_separator));
            CHECK((json::lexer(":", 1).scan() == json::lexer::token_type::name_separator));
        }

        SECTION("literal names")
        {
            CHECK((json::lexer("null", 4).scan() == json::lexer::token_type::literal_null));
            CHECK((json::lexer("true", 4).scan() == json::lexer::token_type::literal_true));
            CHECK((json::lexer("false", 5).scan() == json::lexer::token_type::literal_false));
        }

        SECTION("numbers")
        {
            CHECK((json::lexer("0", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("1", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("2", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("3", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("4", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("5", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("6", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("7", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("8", 1).scan() == json::lexer::token_type::value_unsigned));
            CHECK((json::lexer("9", 1).scan() == json::lexer::token_type::value_unsigned));

            CHECK((json::lexer("-0", 2).scan() == json::lexer::token_type::value_integer));
            CHECK((json::lexer("-1", 2).scan() == json::lexer::token_type::value_integer));

            CHECK((json::lexer("1.1", 3).scan() == json::lexer::token_type::value_float));
            CHECK((json::lexer("-1.1", 4).scan() == json::lexer::token_type::value_float));
            CHECK((json::lexer("1E10", 4).scan() == json::lexer::token_type::value_float));
        }

        SECTION("whitespace")
        {
            // result is end_of_input, because not token is following
            CHECK((json::lexer(" ", 1).scan() == json::lexer::token_type::end_of_input));
            CHECK((json::lexer("\t", 1).scan() == json::lexer::token_type::end_of_input));
            CHECK((json::lexer("\n", 1).scan() == json::lexer::token_type::end_of_input));
            CHECK((json::lexer("\r", 1).scan() == json::lexer::token_type::end_of_input));
            CHECK((json::lexer(" \t\n\r\n\t ", 7).scan() == json::lexer::token_type::end_of_input));
        }
    }

    SECTION("token_type_name")
    {
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::uninitialized)) == "<uninitialized>"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::literal_true)) == "true literal"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::literal_false)) == "false literal"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::literal_null)) == "null literal"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::value_string)) == "string literal"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::value_unsigned)) == "number literal"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::value_integer)) == "number literal"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::value_float)) == "number literal"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::begin_array)) == "'['"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::begin_object)) == "'{'"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::end_array)) == "']'"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::end_object)) == "'}'"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::name_separator)) == "':'"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::value_separator)) == "','"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::parse_error)) == "<parse error>"));
        CHECK((std::string(json::lexer::token_type_name(json::lexer::token_type::end_of_input)) == "end of input"));
    }

    SECTION("parse errors on first character")
    {
        for (int c = 1; c < 128; ++c)
        {
            // create string from the ASCII code
            const auto s = std::string(1, static_cast<char>(c));
            // store scan() result
            const auto res = json::lexer(s.c_str(), 1).scan();

            switch (c)
            {
                // single characters that are valid tokens
                case ('['):
                case (']'):
                case ('{'):
                case ('}'):
                case (','):
                case (':'):
                case ('0'):
                case ('1'):
                case ('2'):
                case ('3'):
                case ('4'):
                case ('5'):
                case ('6'):
                case ('7'):
                case ('8'):
                case ('9'):
                {
                    CHECK((res != json::lexer::token_type::parse_error));
                    break;
                }

                // whitespace
                case (' '):
                case ('\t'):
                case ('\n'):
                case ('\r'):
                {
                    CHECK((res == json::lexer::token_type::end_of_input));
                    break;
                }

                // anything else is not expected
                default:
                {
                    CHECK((res == json::lexer::token_type::parse_error));
                    break;
                }
            }
        }
    }

    /* NOTE: to_unicode function has been removed
    SECTION("to_unicode")
    {
        // lexer to call to_unicode on
        json::lexer dummy_lexer("", 0);
        CHECK(dummy_lexer.to_unicode(0x1F4A9) == "💩");
        CHECK_THROWS_AS(dummy_lexer.to_unicode(0x200000), json::parse_error);
        CHECK_THROWS_WITH(dummy_lexer.to_unicode(0x200000), "[json.exception.parse_error.103] parse error: code points above 0x10FFFF are invalid");
    }
    */
}
