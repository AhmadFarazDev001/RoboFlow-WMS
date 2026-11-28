// -- Written by Ahmad: Validated console input readers for numbers, IDs, and choices
#pragma once

#include "library.h"

namespace InputHelper
{
    // -- Trims leading and trailing whitespace characters from a string
    inline string trim(const string &s)
    {
        size_t start = 0;
        while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n'))
            start++;
        size_t end = s.size();
        while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n'))
            end--;
        return s.substr(start, end - start);
    }

    // -- Displays a formatted prompt and reads a trimmed line of input
    inline string readRawLine(const string &prompt)
    {
        UI::printSpaces(UI::getBoxPadding() + 2);
        cout << UI::accent() << ">> " << UI::reset() << UI::bold() << prompt << UI::reset() << flush;

        string line;
        if (!getline(cin, line))
            return "";
        return trim(line);
    }

    // -- Prompts repeatedly until a valid integer inside [minVal, maxVal] is entered
    inline int readIntInRange(const string &prompt, int minVal, int maxVal)
    {
        while (true)
        {
            if (cin.eof())
                return minVal;

            string raw = readRawLine(prompt);
            if (raw.empty())
            {
                UI::printError("Input cannot be empty. Enter a whole number from " +
                               to_string(minVal) + " to " + to_string(maxVal) + ".");
                continue;
            }

            size_t idx = (raw[0] == '-') ? 1 : 0;
            if (idx >= raw.size())
            {
                UI::printError("Invalid integer format.");
                continue;
            }

            bool digitsOnly = true;
            for (size_t i = idx; i < raw.size(); i++)
            {
                if (raw[i] < '0' || raw[i] > '9')
                {
                    digitsOnly = false;
                    break;
                }
            }
            if (!digitsOnly)
            {
                UI::printError("Only numeric digits are allowed. Please try again.");
                continue;
            }

            try
            {
                long long parsed = stoll(raw);
                if (parsed < minVal || parsed > maxVal)
                {
                    UI::printError("Choice out of range (" + to_string(minVal) +
                                   " to " + to_string(maxVal) + ").");
                    continue;
                }
                return static_cast<int>(parsed);
            }
            catch (...)
            {
                UI::printError("Number exceeds valid range. Please try again.");
            }
        }
    }

    // -- Prompts repeatedly until a valid decimal number within bounds is entered
    inline double readDoubleInRange(const string &prompt, double minVal, double maxVal, bool allowMinEqual = true)
    {
        while (true)
        {
            if (cin.eof())
                return minVal;

            string raw = readRawLine(prompt);
            if (raw.empty())
            {
                UI::printError("Input cannot be empty. Enter a decimal number.");
                continue;
            }

            bool seenDot = false;
            bool seenDigit = false;
            bool valid = true;
            size_t start = (raw[0] == '-') ? 1 : 0;

            for (size_t i = start; i < raw.size(); i++)
            {
                char c = raw[i];
                if (c >= '0' && c <= '9')
                    seenDigit = true;
                else if (c == '.' && !seenDot)
                    seenDot = true;
                else
                {
                    valid = false;
                    break;
                }
            }

            if (!valid || !seenDigit)
            {
                UI::printError("Invalid decimal format. Enter numbers only (e.g., 12.5).");
                continue;
            }

            try
            {
                double val = stod(raw);
                bool tooLow = allowMinEqual ? (val < minVal) : (val <= minVal);
                if (tooLow || val > maxVal)
                {
                    UI::printError("Value is outside the allowed range.");
                    continue;
                }
                return val;
            }
            catch (...)
            {
                UI::printError("Invalid numeric value.");
            }
        }
    }

    // -- Reads and validates an entity ID beginning with the expected prefix and digits
    inline string readEntityID(const string &prompt, const string &prefix)
    {
        while (true)
        {
            if (cin.eof())
                return "";

            string raw = readRawLine(prompt);
            if (raw.size() <= prefix.size() || raw.compare(0, prefix.size(), prefix) != 0)
            {
                UI::printError("ID must begin with '" + prefix + "' followed by digits (example: " + prefix + "01).");
                continue;
            }

            bool digitsOk = true;
            for (size_t i = prefix.size(); i < raw.size(); i++)
            {
                if (raw[i] < '0' || raw[i] > '9')
                {
                    digitsOk = false;
                    break;
                }
            }
            if (!digitsOk)
            {
                UI::printError("Characters after '" + prefix + "' must be digits only.");
                continue;
            }
            return raw;
        }
    }

    // -- Reads input matching one of the allowed string choices
    inline string readChoice(const string &prompt, const string allowed[], int count)
    {
        while (true)
        {
            if (cin.eof())
                return allowed[0];

            string raw = readRawLine(prompt);
            for (int i = 0; i < count; i++)
            {
                if (raw == allowed[i])
                    return raw;
            }

            string opts;
            for (int i = 0; i < count; i++)
            {
                opts += allowed[i];
                if (i + 1 < count)
                    opts += " / ";
            }
            UI::printError("Invalid option. Allowed values: " + opts);
        }
    }

    // -- Prompts for a y or n confirmation and returns true for yes
    inline bool readYesNo(const string &prompt)
    {
        while (true)
        {
            if (cin.eof())
                return false;

            string raw = readRawLine(prompt);
            if (raw == "y" || raw == "Y")
                return true;
            if (raw == "n" || raw == "N")
                return false;
            UI::printError("Please enter 'y' for Yes or 'n' for No.");
        }
    }

    // -- Reads a non-empty single token containing no whitespace
    inline string readNonEmptyToken(const string &prompt)
    {
        while (true)
        {
            if (cin.eof())
                return "N/A";

            string raw = readRawLine(prompt);
            if (raw.empty())
            {
                UI::printError("Input cannot be empty.");
                continue;
            }

            bool hasSpace = false;
            for (size_t i = 0; i < raw.size(); i++)
            {
                if (raw[i] == ' ' || raw[i] == '\t')
                {
                    hasSpace = true;
                    break;
                }
            }
            if (hasSpace)
            {
                UI::printError("Spaces are not allowed here (use '_' instead, e.g. Aisle_A).");
                continue;
            }
            return raw;
        }
    }
}
