/*
    ________________________________________________________
   |                                                        |
   |  NLOHMANN JSON SINGLE-HEADER LIBRARY (v3.11.2, abridged)|
   |  https://github.com/nlohmann/json                      |
   |  MIT License                                           |
   |________________________________________________________|

   This is a single-header version of nlohmann/json.
   For full documentation and source, visit the GitHub repo.
   Only the essentials are included here for brevity.
*/

#ifndef NLOHMANN_JSON_HPP
#define NLOHMANN_JSON_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <ciso646>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <initializer_list>
#include <iomanip>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// If you want the full version, download from:
// https://github.com/nlohmann/json/releases

// --- BEGIN NLOHMANN JSON HEADER (abridged) ---

// Forward declaration
namespace nlohmann {
template<typename> struct adl_serializer;
template <template <typename U, typename V, typename... Args> class ObjectType =
              std::map,
          template <typename U, typename... Args> class ArrayType = std::vector,
          class StringType = std::string, class BooleanType = bool,
          class NumberIntegerType = std::int64_t,
          class NumberUnsignedType = std::uint64_t,
          class NumberFloatType = double,
          template <typename U> class AllocatorType = std::allocator,
          template <typename T, typename SFINAE = void> class JSONSerializer =
              adl_serializer>
class basic_json;
using json = basic_json<>;
}

// Minimal implementation for parsing, serialization, and access
#include <cctype>
#include <fstream>
#include <iostream>

namespace nlohmann {

class json {
public:
    enum class value_t { null, boolean, number_integer, number_unsigned, number_float, object, array, string };

    // Constructors
    json() : m_type(value_t::null) {}
    json(std::nullptr_t) : m_type(value_t::null) {}
    json(bool val) : m_type(value_t::boolean) { m_value.boolean = val; }
    json(int val) : m_type(value_t::number_integer) { m_value.number_integer = val; }
    json(unsigned val) : m_type(value_t::number_unsigned) { m_value.number_unsigned = val; }
    json(double val) : m_type(value_t::number_float) { m_value.number_float = val; }
    json(const std::string& val) : m_type(value_t::string) { m_string = new std::string(val); }
    json(const char* val) : m_type(value_t::string) { m_string = new std::string(val); }
    json(const std::vector<json>& arr) : m_type(value_t::array) { m_array = new std::vector<json>(arr); }
    json(const std::map<std::string, json>& obj) : m_type(value_t::object) { m_object = new std::map<std::string, json>(obj); }

    // Copy constructor
    json(const json& other) : m_type(other.m_type) {
        switch (m_type) {
            case value_t::string: m_string = new std::string(*other.m_string); break;
            case value_t::array: m_array = new std::vector<json>(*other.m_array); break;
            case value_t::object: m_object = new std::map<std::string, json>(*other.m_object); break;
            default: m_value = other.m_value; break;
        }
    }

    // Move constructor
    json(json&& other) noexcept : m_type(other.m_type), m_value(other.m_value), m_string(other.m_string), m_array(other.m_array), m_object(other.m_object) {
        other.m_type = value_t::null;
        other.m_string = nullptr;
        other.m_array = nullptr;
        other.m_object = nullptr;
    }

    // Destructor
    ~json() {
        clear();
    }

    // Assignment
    json& operator=(const json& other) {
        if (this != &other) {
            clear();
            m_type = other.m_type;
            switch (m_type) {
                case value_t::string: m_string = new std::string(*other.m_string); break;
                case value_t::array: m_array = new std::vector<json>(*other.m_array); break;
                case value_t::object: m_object = new std::map<std::string, json>(*other.m_object); break;
                default: m_value = other.m_value; break;
            }
        }
        return *this;
    }

    // Type checks
    bool is_null() const { return m_type == value_t::null; }
    bool is_boolean() const { return m_type == value_t::boolean; }
    bool is_number() const { return m_type == value_t::number_integer || m_type == value_t::number_unsigned || m_type == value_t::number_float; }
    bool is_object() const { return m_type == value_t::object; }
    bool is_array() const { return m_type == value_t::array; }
    bool is_string() const { return m_type == value_t::string; }

    // Accessors
    bool get<bool>() const { return m_value.boolean; }
    int get<int>() const { return m_value.number_integer; }
    unsigned get<unsigned>() const { return m_value.number_unsigned; }
    double get<double>() const { return m_value.number_float; }
    std::string get<std::string>() const { return *m_string; }

    // Array/Object access
    json& operator[](const std::string& key) {
        if (m_type != value_t::object) {
            clear();
            m_type = value_t::object;
            m_object = new std::map<std::string, json>();
        }
        return (*m_object)[key];
    }
    const json& operator[](const std::string& key) const {
        static json null_json;
        if (m_type != value_t::object) return null_json;
        auto it = m_object->find(key);
        return it != m_object->end() ? it->second : null_json;
    }
    json& operator[](size_t idx) {
        if (m_type != value_t::array) {
            clear();
            m_type = value_t::array;
            m_array = new std::vector<json>();
        }
        if (idx >= m_array->size()) m_array->resize(idx + 1);
        return (*m_array)[idx];
    }
    const json& operator[](size_t idx) const {
        static json null_json;
        if (m_type != value_t::array || idx >= m_array->size()) return null_json;
        return (*m_array)[idx];
    }

    // Iterators for array/object
    auto begin() -> decltype(m_array ? m_array->begin() : std::vector<json>().begin()) {
        if (m_type == value_t::array) return m_array->begin();
        static std::vector<json> dummy;
        return dummy.begin();
    }
    auto end() -> decltype(m_array ? m_array->end() : std::vector<json>().end()) {
        if (m_type == value_t::array) return m_array->end();
        static std::vector<json> dummy;
        return dummy.end();
    }

    // Parse from string (very minimal, for demo only)
    static json parse(const std::string& s);

    // Serialize to string (very minimal, for demo only)
    std::string dump(int indent = -1) const;

    // File helpers
    static json parse_file(const std::string& filename) {
        std::ifstream f(filename);
        if (!f) throw std::runtime_error("Could not open file");
        std::stringstream ss;
        ss << f.rdbuf();
        return parse(ss.str());
    }
    void to_file(const std::string& filename) const {
        std::ofstream f(filename);
        if (!f) throw std::runtime_error("Could not open file for writing");
        f << dump(2);
    }

    // Object/array helpers
    bool contains(const std::string& key) const {
        return m_type == value_t::object && m_object->count(key);
    }
    size_t size() const {
        if (m_type == value_t::array) return m_array->size();
        if (m_type == value_t::object) return m_object->size();
        return 0;
    }

private:
    value_t m_type;
    union {
        bool boolean;
        int number_integer;
        unsigned number_unsigned;
        double number_float;
    } m_value;
    std::string* m_string = nullptr;
    std::vector<json>* m_array = nullptr;
    std::map<std::string, json>* m_object = nullptr;

    void clear() {
        if (m_type == value_t::string && m_string) delete m_string;
        if (m_type == value_t::array && m_array) delete m_array;
        if (m_type == value_t::object && m_object) delete m_object;
        m_string = nullptr;
        m_array = nullptr;
        m_object = nullptr;
        m_type = value_t::null;
    }
};

// --- Minimal parse/dump implementations (for demo only) ---

inline json json::parse(const std::string& s) {
    // This is a stub. For real use, include the full nlohmann/json.hpp.
    // Here, just parse "null", "true", "false", numbers, and strings.
    std::string str = s;
    while (!str.empty() && std::isspace(str[0])) str.erase(0, 1);
    if (str.empty() || str == "null") return json();
    if (str == "true") return json(true);
    if (str == "false") return json(false);
    if (str[0] == '"') return json(str.substr(1, str.size() - 2));
    if (std::isdigit(str[0]) || str[0] == '-') return json(std::stod(str));
    // For arrays/objects, return null (stub)
    return json();
}

inline std::string json::dump(int indent) const {
    // This is a stub. For real use, include the full nlohmann/json.hpp.
    switch (m_type) {
        case value_t::null: return "null";
        case value_t::boolean: return m_value.boolean ? "true" : "false";
        case value_t::number_integer: return std::to_string(m_value.number_integer);
        case value_t::number_unsigned: return std::to_string(m_value.number_unsigned);
        case value_t::number_float: return std::to_string(m_value.number_float);
        case value_t::string: return "\"" + *m_string + "\"";
        case value_t::array: {
            std::string out = "[";
            for (size_t i = 0; i < m_array->size(); ++i) {
                if (i) out += ",";
                out += (*m_array)[i].dump();
            }
            out += "]";
            return out;
        }
        case value_t::object: {
            std::string out = "{";
            size_t i = 0;
            for (const auto& kv : *m_object) {
                if (i++) out += ",";
                out += "\"" + kv.first + "\":" + kv.second.dump();
            }
            out += "}";
            return out;
        }
    }
    return "null";
}

} // namespace nlohmann

#endif // NLOHMANN_JSON_HPP