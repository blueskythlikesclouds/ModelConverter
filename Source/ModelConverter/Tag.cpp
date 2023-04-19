#include "Tag.h"

std::string_view Tag::getValue(size_t index, const std::string_view& defaultValue) const
{
    return values.size() > index ? values[index] : defaultValue;
}

bool Tag::getBoolValue(size_t index, bool defaultValue) const
{
    if (values.size() > index && !values[index].empty())
    {
        return
            values[index][0] == '1' ||
            values[index][0] == 't' ||
            values[index][0] == 'T' ||
            values[index][0] == 'y' ||
            values[index][0] == 'Y';
    }

    return defaultValue;
}

int Tag::getIntValue(size_t index, int defaultValue) const
{
    if (values.size() > index)
        std::from_chars(values[index].data(), values[index].data() + values[index].size(), defaultValue);

    return defaultValue;
}

float Tag::getFloatValue(size_t index, float defaultValue) const
{
    if (values.size() > index)
        std::from_chars(values[index].data(), values[index].data() + values[index].size(), defaultValue);

    return defaultValue;
}

std::string_view Tags::getName(const std::string_view& input)
{
    const size_t index = input.find('@');
    return input.substr(0, index);
}

Tags::Tags(const std::string_view& input)
{
    size_t pos = 0;
    while (pos < input.size())
    {
        const size_t start = input.find('@', pos);

        if (start == std::string::npos)
        {
            if (pos == 0)
                name = input;

            break;
        }
        if (pos == 0)
            name = std::string_view(input.data(), start);

        size_t end = input.find('@', start + 1);
        if (end == std::string::npos)
            end = input.size();

        pos = end;

        const size_t index = size();
        auto& tag = emplace_back();
        tag.name = std::string_view(input.data() + start + 1, end - start - 1);

        if (tag.name.find('(') != std::string::npos)
        {
            const size_t valueStart = tag.name.find('(');
            const size_t valueEnd = tag.name.find(')');

            if (valueEnd != std::string::npos && valueEnd > valueStart) 
            {
                std::string_view values(input.data() + valueStart + start + 2, valueEnd - valueStart - 1);
                size_t valuePos = 0;

                while (valuePos < values.size()) 
                {
                    size_t valueSep = values.find(',', valuePos);
                    if (valueSep == std::string::npos)
                        valueSep = values.size();

                    std::string_view value = values.substr(valuePos, valueSep - valuePos);
                    value.remove_prefix(std::min(value.find_first_not_of(" \t\n\r\f\v"), value.size()));
                    value.remove_suffix(std::min(value.size() - value.find_last_not_of(" \t\n\r\f\v") - 1, value.size()));

                    tag.values.emplace_back(value);

                    valuePos = valueSep + 1;
                }

                tag.name = std::string_view(input.data() + start + 1, valueStart);
            }
        }

        indices.emplace(tag.name, index);
    }
}

std::string_view Tags::getValue(const std::string_view& tag, size_t index, const std::string_view& defaultValue) const
{
    const auto pair = indices.find(tag);
    return pair != indices.end() ? operator[](pair->second).getValue(index, defaultValue) : defaultValue;
}

bool Tags::getBoolValue(const std::string_view& tag, size_t index, bool defaultValue) const
{
    const auto pair = indices.find(tag);
    return pair != indices.end() ? operator[](pair->second).getBoolValue(index, defaultValue) : defaultValue;
}

int Tags::getIntValue(const std::string_view& tag, size_t index, int defaultValue) const
{
    const auto pair = indices.find(tag);
    return pair != indices.end() ? operator[](pair->second).getIntValue(index, defaultValue) : defaultValue;
}

float Tags::getFloatValue(const std::string_view& tag, size_t index, float defaultValue) const
{
    const auto pair = indices.find(tag);
    return pair != indices.end() ? operator[](pair->second).getFloatValue(index, defaultValue) : defaultValue;
}
