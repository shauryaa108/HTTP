#pragma once

#include "HttpRequest.h"

#include <string>

using namespace std;

enum class ParseResult {
    Incomplete,
    Complete,
    BadRequest
};

class HttpParser {
private:
    string buffer;
    HttpRequest request;

    static string trim(const string& str);
    static string toLower(string str);

public:
    ParseResult feed(const string& data);

    const HttpRequest& getRequest() const;

    void reset();
};