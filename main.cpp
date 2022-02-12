#include <iostream>
#include <string>
#include "Json.h"

using namespace std;
using namespace myjson;

int test_count = 0;
int test_pass = 0;
int test_wrong = 0;

string err;

#define EXPECT_EQ_BASE(equality, expect, actual) \
    do{\
        ++test_count;\
        if (equality) { ++test_pass; }\
        else{\
            cout << __FILE__ << ":" << __LINE__ << " wrong : expect:" << (expect) << " actual: " << (actual) << endl;\
            ++test_wrong;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual)
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual)

static void test_parse_null() {
    EXPECT_EQ_INT(Json::Type::NUL, Json::parse("null", err).type());
}

static void test_parse_true() {
    EXPECT_EQ_INT(Json::Type::BOOL, Json::parse("true", err).type());
    EXPECT_EQ_INT(true, Json::parse("true", err).bool_value());
}

static void test_parse_false() {
    EXPECT_EQ_INT(Json::Type::BOOL, Json::parse("false", err).type());
    EXPECT_EQ_INT(false, Json::parse("false", err).bool_value());
}

#define PARSE_INT(expect, json) \
    do { \
        EXPECT_EQ_INT(Json::Type::NUMBER, Json::parse(json, err).type()); \
        EXPECT_EQ_INT(expect, Json::parse(json, err).int_value());\
    } while (0)
#define PARSE_DOUBLE(expect, json) \
    do { \
        EXPECT_EQ_INT(Json::Type::NUMBER, Json::parse(json, err).type()); \
        EXPECT_EQ_DOUBLE(expect, Json::parse(json, err).double_value());\
    } while(0)

static void test_parse_number() {
    PARSE_INT(1, "1");
    PARSE_INT(-23, "-23");
    PARSE_INT(456, "456");
    PARSE_INT(-7890, "-7890");
    PARSE_INT(1234567890, "1234567890");
    PARSE_INT(123, "123");
    PARSE_DOUBLE(1.2, "1.2");
    PARSE_DOUBLE(-3.4, "-3.4");
    PARSE_DOUBLE(56.78, "56.78");
    PARSE_DOUBLE(90.09, "90.09");
    PARSE_DOUBLE(1.0000000000000002, "1.0000000000000002");
}

#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual)
#define PARSE_STRING(expect, json) do {\
    EXPECT_EQ_INT(Json::Type::STRING, Json::parse(json, err).type()); \
    EXPECT_EQ_STRING(expect, Json::parse(json, err).string_value());\
} while (0)

static void test_parse_string() {
    PARSE_STRING("", "\"\"");
    PARSE_STRING("Hello\nWorld", "\"Hello\nWorld\"");
    PARSE_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

#define EXPECT_EQ_SIZE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual)

static void test_parse_array() {
    EXPECT_EQ_INT(Json::Type::ARRAY, Json::parse("[ ]", err).type());
    EXPECT_EQ_SIZE(0, Json::parse("[ ]", err).size());

    EXPECT_EQ_INT(Json::Type::ARRAY, Json::parse("[  null , false , true , 123 , \"abc\"  ]", err).type());
    EXPECT_EQ_SIZE(5, Json::parse("[  null , false , true , 123 , \"abc\"  ]", err).size());
}

static void test_parse_objece() {
    EXPECT_EQ_INT(Json::Type::OBJECT, Json::parse("{ }", err).type());
    EXPECT_EQ_SIZE(0, Json::parse("{ }", err).size());

    EXPECT_EQ_INT(Json::Type::OBJECT, Json::parse("{\"key1\": 12.34, \"key2\": false, \"key3\": [1, 2, 3]}", err).type());
    EXPECT_EQ_SIZE(3, Json::parse("{\"key1\": 12.34, \"key2\": false, \"key3\": [1, 2, 3]}", err).size());
}

static void test_parse() {
    test_parse_null();
    test_parse_false();
    test_parse_true();
    test_parse_number();
    test_parse_string();
    test_parse_array();
    test_parse_objece();
}

int main() {
    test_parse();
    cout << "通过" << test_pass << "例, 错误" << test_wrong << "例" << endl;

    return 0;
}
