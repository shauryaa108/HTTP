#pragma once

#include <string>
#include <unordered_map>

using namespace std;

class HttpResponse {
private:
    int statusCode;
    string statusText;

    unordered_map<string, string> headers;

    string body;

public:
    HttpResponse();

    void setStatus(int code, const string& text);
    void setHeader(const string& name, const string& value);
    void setBody(const string& body);

    string toString(bool headRequest = false) const;

    static HttpResponse ok(const string& body);
    static HttpResponse badRequest();
    static HttpResponse notFound();
    static HttpResponse methodNotAllowed();
    static HttpResponse internalServerError();
};