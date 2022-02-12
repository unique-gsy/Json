//
// Created by 花火 on 2022/1/22.
//

#include "Json.h"

namespace myjson {

    static inline bool in_range(long x, long lower, long upper) {
        return x >= lower && x <= upper;
    }
    static void dump(NullStruct, std::string& result) {
        result += "null";
    }
    static void dump(bool value, std::string& result) {
        result += value ? "true" : "false";
    }
    static void dump(int value, std::string& result) {
        result += std::to_string(value);
    }
    static void dump(double value, std::string& result) {
        result += std::to_string(value);
    }
    static void dump(const std::string& value, std::string& result) {
        result += "\"" + value + "\"";
    }
    static void dump(const Json::array& value, std::string& result) {
        result += '[';
        bool first = true;
        for (int i = 0; i < value.size(); ++i) {
            if (!first)
                result += ',';
            first = false;
            value[i].dump(result);
        }
        result += ']';
    }
    static void dump(const Json::object& value, std::string& result) {
        result += '{';
        bool first = true;
        for (const auto &tmp : value) {
            if (!first)
                result += ',';
            first = false;
            dump(tmp.first,result);
            result += ":";
            tmp.second.dump(result);
        }
        result += '}';
    }

    Json::Json() noexcept : value_ptr(std::make_shared<JsonNULL>()) {}

    Json::Json(const std::string &value) : value_ptr(make_shared<JsonString>(value)) {}
    Json::Json(const object &values) : value_ptr(make_shared<JsonObject>(values)) {}
    Json::Json(bool value): value_ptr(std::make_shared<JsonBoolean>(value)) {}
    Json::Json(int value): value_ptr(std::make_shared<JsonInt>(value)) {}
    Json::Json(double value): value_ptr(std::make_shared<JsonDouble>(value)) {}
    Json::Json(const array &value): value_ptr(std::make_shared<JsonArray>(value)) {}

    Json Json::parse(const std::string &input, std::string &err) {
        Paser parser{input, err, 0, false};
        Json result = parser.parse(0);
        return result;
    }
    void Json::dump(std::string &result) const {
        return value_ptr ->dump(result);
    }


    Json::Type Json::type() const { return value_ptr -> type(); }
    int Json::size() const { return value_ptr -> size(); }

    bool Json::bool_value() const { return value_ptr -> bool_value(); }
    int Json::int_value() const { return value_ptr -> int_value(); }
    double Json::double_value() const { return value_ptr -> double_value(); }
    const std::string &Json::string_value() const { return value_ptr -> string_value(); }
    const Json::array &Json::array_value() const { return value_ptr -> array_items(); }
    const Json::object &Json::object_value() const { return value_ptr -> object_items(); }

    bool JsonValue::bool_value() const { return false; }
    int JsonValue::int_value() const { return 0; }
    double JsonValue::double_value() const { return 0; }

    struct Statics {
        const std::string empty_string;
        const Json::array empty_vector;
        const Json::object empty_object;
        Statics() = default;
    };
    static const Statics& statics() {
        static const Statics test {};
        return test;
    };

    const std::string &JsonValue::string_value() const { return statics().empty_string; }
    const Json::array &JsonValue::array_items() const { return statics().empty_vector; }
    const Json::object &JsonValue::object_items() const { return statics().empty_object; }
    int JsonValue::size() const { return 0; }

    template<Json::Type tag, typename T>
    void Value<tag, T>::dump(std::string &result) const {
        myjson::dump(save_value, result);
    }

    Json Paser::expect(const std::string &expect, Json result) {
        if (source.compare(i, expect.size(), expect) == 0) {
            i += expect.size();
            return result;
        } else {
            return fail("parse error: expect " + expect + ", got : " + source.substr(i, expect.size()), Json());
        }
    }

    Json Paser::parse(int depth) {
        if (depth > max_depth)
            return fail("exceeded max depth", Json());
        char ch = token_next_char();
        if (ch == 'n') {
            --i;
            return expect("null", Json());
        }
        if (ch == 't') {
            --i;
            return expect("true", Json(true));
        }
        if (ch == 'f') {
            --i;
            return expect("false", Json(false));
        }
        if (ch == '-' || in_range(ch, '0', '9')) {
            --i;
            return parse_number();
        }
        if (ch == '\"') {
            return Json(parse_string());
        }
        if (ch == '[') {
            std::vector<Json> result;
            ch = token_next_char();
            if (ch == ']') {
                return Json(result);
            }
            while (true) {
                --i;
                result.emplace_back(parse(depth + 1));
                if (failed)
                    return {};
                ch = token_next_char();
                if (ch == ']')
                    break;
                if (ch != ',') {
                    return fail(std::string ("parse error: expect ',' , got ") + ch, Json{});
                }

                ch = token_next_char();
            }
            return Json(result);
        }
        if (ch == '{') {
            Json::object result;
            ch = token_next_char();
            if (ch == '}')
                return Json(result);
            while (true) {
                if (ch != '"')
                    return fail(std::string ("parse error: expect '\"', got: ") + ch, Json());
                std::string key = parse_string();
                if (failed) return Json{};

                ch = token_next_char();
                if (ch != ':')
                    return fail(std::string ("parse error: expect ':' , got: ") + ch, Json());

                result[std::move(key)] = parse(depth + 1);

                if (failed) return Json{};

                ch = token_next_char();
                if (ch == '}') break;
                if (ch != ',') {
                    return fail(std::string ("parse error: expect ',' , got: ") + ch, Json());
                }
                ch = token_next_char();
            }
            return Json(result);
        }


        return Json();
    }

    std::string Paser::parse_string() {
        std::string result;

        if (i >= source.size())
            return fail("end of json", "");
        char ch = source[i++];
        for (; ch != '\"' && i < source.size(); ch = source[i++]) {
            result += ch;
        }
        if (ch == '\0') ++i;
        return result;
    }

    Json Paser::parse_number() {
        if (i >= source.size())
            return fail("end of json", Json());
        size_t start = i;
        bool sign = false;
        bool sign_double = false;
        if (source[i] == '-') {
            sign = true;
            ++i;
        }
        if (source[i] == '+')
            ++i;
        skip_whitespace();
        if (!in_range(source[i], '0', '9'))
            fail(std::string ("parse number err, got") + source[i - 1], Json());
        for (;i < source.size() && in_range(source[i], '0', '9'); ++i) {}
        if (source[i] == '.') {   //double
            sign_double = true;
            ++i;
            if (!in_range(source[i], '0', '9'))
                fail(std::string ("parse number err, got") + source[i - 1], Json());
            for (;i < source.size() && in_range(source[i], '0', '9'); ++i) {}
        }
        if (sign_double) {
            double number = std::stod(source.substr(start, i));
            return Json(number);
        } else {
            int number = std::stoi(source.substr(start, i));
            return Json(number);
        }
    }
}