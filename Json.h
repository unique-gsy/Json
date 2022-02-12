//
// Created by 花火 on 2022/1/22.
//

#ifndef JSON_JSON_H
#define JSON_JSON_H
#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>
#include <memory>

namespace myjson{
    static const int max_depth = 200;

    class JsonValue;

    class Json final {
    public:
        enum Type { NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT };

        typedef std::vector<Json> array;
        typedef std::unordered_map<std::string, Json> object;

        Json() noexcept;
        explicit Json(bool value);
        explicit Json(const std::string &value);
        explicit Json(const object &value);
        explicit Json(int value);
        explicit Json(double value);
        explicit Json(const array &value);

        static Json parse(const std::string& input, std::string &err);
        void dump(std::string& result) const;
        std::string dump() const {
            std::string result;
            dump(result);
            return result;
        }

        Type type() const ;
        bool is_null() const { return type() == NUL; }
        bool is_number() const { return type() == NUMBER; }
        bool is_string() const { return type() == STRING; }
        bool is_array() const { return type() == ARRAY; }
        bool is_object() const { return type() == OBJECT; }

        bool bool_value() const;
        int int_value() const;
        double double_value() const;
        const std::string &string_value() const;
        const Json::array &array_value() const;
        const Json::object &object_value() const;
        int size() const;

    private:
        std::shared_ptr<JsonValue> value_ptr;
    };

    struct Paser final {
        std::string source;
        std::string err;
        size_t i;
        bool failed;

        template<typename T>
        T fail(const std::string &msg, const T state) {
            if (!failed)
                err = msg;
            failed = true;
            return state;
        }

        void skip_whitespace() {
            while (source[i] == ' ' || source[i] == '\n' || source[i] == '\r' || source[i] == '\t' || source[i] == '\0')
                ++i;
        }

        char token_next_char() {
            skip_whitespace();
            if (i == source.size()) {
                return fail("end of json", static_cast<char>(0));
            }
            return source[i++];
        }

        std::string parse_string();

        Json parse_number();

        Json expect(const std::string &expect, Json result);

        Json parse(int depth);
    };

    class JsonValue {
    protected:
        friend class Json;

        virtual Json::Type type() const = 0;

        virtual void dump(std::string& result) const = 0;

        virtual bool bool_value() const;
        virtual int int_value() const;
        virtual double double_value() const;
        virtual const std::string &string_value() const;
        virtual const Json::array &array_items() const;
        virtual const Json::object &object_items() const;
        virtual int size() const;

        virtual ~JsonValue() {}
    };

    template <Json::Type tag, typename T>
    class Value : public JsonValue {
    protected:
        explicit Value(const T &value) : save_value(value) {}
        const T save_value;

        void dump(std::string &result) const override;

        Json::Type type() const override { return tag; }
    };

    struct NullStruct {
        bool operator==(NullStruct) const { return true; }
        bool operator<(NullStruct) const { return false; }
    };

    class JsonNULL final : public Value<Json::NUL, NullStruct > {
    public:
        JsonNULL() : Value<Json::NUL, NullStruct>({}) {}
    };

    class JsonInt final : public Value<Json::NUMBER, int> {
        int int_value() const override { return save_value; }
    public:
        explicit JsonInt(int value) : Value<Json::NUMBER, int>(value) {}
    };

    class JsonDouble final : public Value<Json::NUMBER, double> {
        double double_value() const override { return save_value; }
    public:
        explicit JsonDouble(double value) : Value<Json::NUMBER, double>(value) {}
    };

    class JsonBoolean final : public Value<Json::BOOL, bool> {
        bool bool_value() const override { return save_value; }
    public:
        explicit JsonBoolean(bool value) : Value<Json::BOOL, bool>(value) {}
    };

    class JsonString final : public Value<Json::STRING, std::string> {
        const std::string &string_value() const override { return save_value; }
    public:
        explicit JsonString(const std::string &value) : Value(value) {}
    };

    class JsonArray final : public Value<Json::ARRAY, Json::array > {
        const Json::array &array_items() const override { return save_value; }
        int size() const override { return save_value.size(); }
    public:
        explicit JsonArray(const Json::array &value) : Value(value) {}
    };

    class JsonObject final : public Value<Json::OBJECT, Json::object> {
        const Json::object &object_items() const override { return save_value; }
        int size() const override { return save_value.size(); }
    public:
        explicit JsonObject(const Json::object &value) : Value(value) {}
    };
}

#endif //JSON_JSON_H
