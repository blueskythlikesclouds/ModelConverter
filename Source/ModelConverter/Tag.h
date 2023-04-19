#pragma once

struct Tag
{
    std::string_view name;
    std::vector<std::string_view> values;

    std::string_view getValue(size_t index, const std::string_view& defaultValue) const;
    bool getBoolValue(size_t index, bool defaultValue) const;
    float getFloatValue(size_t index, float defaultValue) const;
};

struct Tags : std::vector<Tag>
{
    std::string_view name;
    std::unordered_multimap<std::string_view, size_t> indices;

    static std::string_view getName(const std::string_view& input);

    Tags(const std::string_view& input);

    std::string_view getValue(const std::string_view& tag, size_t index, const std::string_view& defaultValue) const;
    bool getBoolValue(const std::string_view& tag, size_t index, bool defaultValue) const;
    float getFloatValue(const std::string_view& tag, size_t index, float defaultValue) const;
};