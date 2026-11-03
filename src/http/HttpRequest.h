#pragma once

#include <string>
#include <unordered_map>

using namespace std;

class HttpRequest {
public:
    string method;
    string target;
    string version;

    unordered_map<string, string> headers;

    string body;

    bool hasHeader(const string& name) const;
    string getHeader(const string& name) const;
};