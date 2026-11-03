#include "HttpRequest.h"

using namespace std;

bool HttpRequest::hasHeader(const string& name) const {
    return headers.find(name) != headers.end();
}

string HttpRequest::getHeader(const string& name) const {
    auto it = headers.find(name);

    if (it == headers.end())
        return "";

    return it->second;
}