/*
 * parse.hpp
 *
 *  Created on: Jan 21, 2014
 *      Author: Pranjal Raihan
 */

#ifndef __number_utils_parse_hpp__
#define __number_utils_parse_hpp__

#include <cmath>

namespace numerics
{

/**
 * Parses a null-terminated string of an integral value in base 10.
 *
 * @param str the string to parse
 * @param holder a pointer of where the parsed value should be placed
 * @param integral_t the data type
 *
 * @return if parsing was successful
 */
template <class intregral_t>
bool parse_integral(const char* const str, intregral_t* const holder)
{
    // quick return false on empty string
    if (!str)
    {
        return false;
    }
    // the final value
    intregral_t value = 0;
    // the first char
    char first = str[0];
    bool isNegative = first == '-';

    int i = 0;
    bool i_inc = isNegative || first == '+';
    if (i_inc)
    {
        // if string has a leading '+' or '-', account for it and move on
        i++;
    }
    for (;; ++i)
    {
        char c = str[i];
        if (!c)
        {
            // if the number is empty, like "", "+" or "-" return false
            if (i == (i_inc ? 1 : 0))
            {
                return false;
            }
            // otherwise, we've reached the end of the string,
            // set value and return true
            *holder = (intregral_t) (isNegative ? -value : value);
            return true;
        }

        // quickly, if c = 0, multiply by 10 and continue
        if (c == '0')
        {
            value *= 10;
            continue;
        }

        // get the numeric equivalent of the char
        int val = (int) (c - '0');
        // if the value is outside the range of base 10, return false
        if (val < 0 || val > 9)
        {
            return false;
        }
        // add next digit to value
        value = value * 10 + val;
    }

    // if it reached here, something went wrong, return false
    return false;
}

/**
 * Parses a null-terminated string of an integral value in the specified base.
 * Possible valid chars include numbers 0-9, and values greater than 'A' for bases
 * greater than 10.
 *
 * @param str the string to parse
 * @param holder a pointer of where the parsed value should be placed
 * @param base the base to parse in
 * @param integral_t the data type
 *
 * @return if parsing was successful
 */
template <class intregral_t>
bool parse_integral(const char* const str, intregral_t* const holder,
        const unsigned int base)
{
    // quick check for empty string or base 0 or 1
    if (base < 2 || !str)
    {
        return false;
    }
    // the final value
    intregral_t value = 0;
    // the first char
    char first = str[0];
    bool isNegative = first == '-';
    int i = 0;
    bool i_inc = isNegative || first == '+';
    if (i_inc)
    {
        // if string has a leading '+' or '-', account for it and move on
        i++;
    }
    for (;; ++i)
    {
        char c = str[i];
        if (!c)
        {
            // if the number is empty, like "", "+" or "-" return false
            if (i == i_inc ? 1 : 0)
            {
                return false;
            }
            // otherwise, we've reached the end of the string,
            // set the value and return true
            *holder = (intregral_t) (isNegative ? -value : value);
            return true;
        }

        // if 0, shift digits and continue
        if (c == '0')
        {
            value *= base;
            continue;
        }

        // if string contains any symbols between '9' and 'A' return false
        if (c > 57 && c < 65)
        {
            return false;
        }

        int val = (int) (c - '0');
        // so account for the ignored value, we must subtract
        if (val > 9)
        {
            val -= 7;
        }

        // if any digit is outside the base's range, return false
        if (val < 0 || val > base - 1)
        {
            return false;
        }

        // add next digit
        value = value * base + val;
    }

    // if it reached here, then it failed, return false
    return false;
}

/**
 * Parses a null-terminated string of an integral value in base 16.
 * Valid values include 0-9, A-F and a-f. The string must be in the form
 * [+/-]0x[xx...].
 *
 * @param str the string to parse
 * @param holder a pointer of where the parsed value should be placed
 * @param integral_t the data type
 *
 * @return if parsing was successful
 */
template <class intregral_t>
bool parse_hex(const char* const str, intregral_t* const holder)
{
    // quick check for empty string
    if (!str)
    {
        return false;
    }
    // the first char
    char first = str[0];
    bool isNegative = first == '-';
    int i = 0;
    bool i_inc = isNegative || first == '+';
    if (i_inc)
    {
        // if string has a leading '+' or '-', account for it and move on
        i++;
    }
    // hex string must start with "0x"
    if (str[i] != '0' || str[i + 1] != 'x')
    {
        return false;
    }
    intregral_t value = 0;
    for (;; ++i)
    {
        char c = str[i + 2];
        if (!c)
        {
            if (i == (i_inc ? 1 : 0))
            {
                // if the number is empty, like "", "+" or "-" return false
                return false;
            }
            *holder = (intregral_t) (isNegative ? -value : value);
            return true;
        }

        // if c = 0, shift digit and move on
        if (c == '0')
        {
            value *= 16;
            continue;
        }

        // if c is a symbol between '9' and 'A' return false
        if (c > 57 && c < 65)
        {
            return false;
        }

        // if c is a small letter, lower the value into the range
        if (c > 96)
        {
            c -= 39;
        }
        // or if c is a capital, lower the value but by less
        else if (c > 64)
        {
            c -= 7;
        }

        int val = (int) (c - '0');
        // if the value is outside the hex digits range, return false
        if (val < 0 || val > 15)
        {
            return false;
        }

        // add next digit
        value = value * 16 + val;
    }

    // if it got here, then something went wrong, return false
    return false;
}

/**
 * Parses a null-terminated string of an integral value in binary.
 * Valid values include 0 and 1. The string must be in the form [+/-]0b[xx...].
 *
 * @param str the string to parse
 * @param holder a pointer of where the parsed value should be placed
 * @param integral_t the data type
 *
 * @return if parsing was successful
 */
template <class intregral_t>
bool parse_bin(const char* const str, intregral_t* const holder)
{
    if (!str)
    {
        return false;
    }
    char first = str[0];
    bool isNegative = first == '-';
    int i = 0;
    bool i_inc = isNegative || first == '+';

    if (i_inc)
    {
        i++;
    }
    if (str[i] != '0' || str[i + 1] != 'b')
    {
        return false;
    }
    intregral_t value = 0;
    for (;; ++i)
    {
        char c = str[i + 2];
        if (!c)
        {
            if (i == (i_inc ? 1 : 0))
            {
                return false;
            }
            *holder = (intregral_t) (isNegative ? -value : value);
            return true;
        }
        switch (c)
        {
            case 48:
                value = value << 1;
                break;
            case 49:
                value = (value << 1) + 1;
                break;
            default:
                return false;
        }
    }
    return false;
}

/**
 * Parses a null-terminated string of a decimal or exponent notation to a
 * floating point (or optionally integral) value.
 * The string must be in the form [+/-]xx...[.][xx...][e/E][(exponent)].
 *
 * @param str the string to parse
 * @param holder a pointer of where the parsed value should be placed
 * @param fp_t the data type
 *
 * @return if parsing was successful
 */
template <class fp_t>
bool parse_fp(const char* const str, fp_t* const holder)
{
    if (!str)
    {
        return false;
    }

    long long value = 0;
    char first = str[0];

    bool isNegative = first == '-';
    int i = 0;
    bool i_inc = isNegative || first == '+';
    if (i_inc)
    {
        i++;
    }

    bool exp_inc = false;
    int pt_div_idx = -1;
    for (;; ++i)
    {
        char c = str[i];
        if (!c)
        {
            if (exp_inc || (i == (i_inc ? 1 : 0)))
            {
                return false;
            }
            if (!pt_div_idx)
            {
                return false;
            }
            fp_t fp_val = (fp_t) (isNegative ? -value : value);
            if (pt_div_idx != -1)
            {
                fp_val /= (fp_t) std::pow(10, pt_div_idx);
            }
            *holder = fp_val;
            return true;
        }
        switch (c)
        {
            case 'e':
            case 'E':
                if (exp_inc || !pt_div_idx)
                {
                    return false;
                }
                exp_inc = true;
                break;
            case '.':
                if (pt_div_idx > -1)
                {
                    return false;
                }
                pt_div_idx++;
                break;
            default:
                int val = (int) (c - '0');
                if (exp_inc)
                {
                    int exp_val;
                    bool success = parse_integral<int>(str + i, &exp_val);
                    if (!success) return false;
                    fp_t fp_val = (fp_t) (isNegative ? -value : value);
                    if (pt_div_idx > -1)
                    {
                        fp_val /= (fp_t) std::pow(10, pt_div_idx - exp_val);
                    }
                    *holder = fp_val;
                    return true;
                }
                else
                {
                    if (pt_div_idx > -1)
                    {
                        pt_div_idx++;
                    }
                    if (val < 0 || val > 9)
                    {
                        return false;
                    }
                    value = value * 10 + val;
                }
        }
    }
}

}

#endif /* __number_utils_parse_hpp__ */