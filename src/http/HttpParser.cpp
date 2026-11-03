#include "HttpParser.h"

#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

string HttpParser::trim(const string& str) {
    size_t start = 0;
    size_t end = str.size();

    while (start < end &&
           isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }

    while (end > start &&
           isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }

    return str.substr(start, end - start);
}

string HttpParser::toLower(string str) {
    transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](unsigned char c) {
            return tolower(c);
        }
    );

    return str;
}

ParseResult HttpParser::feed(const string& data) {
    buffer += data;

    size_t headerEnd = buffer.find("\r\n\r\n");

    if (headerEnd == string::npos)
        return ParseResult::Incomplete;

    string headerPart = buffer.substr(0, headerEnd);

    istringstream stream(headerPart);
    string line;

    if (!getline(stream, line))
        return ParseResult::BadRequest;

    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    istringstream requestLine(line);

    if (!(requestLine >> request.method
                     >> request.target
                     >> request.version)) {
        return ParseResult::BadRequest;
    }

    string extra;

    if (requestLine >> extra)
        return ParseResult::BadRequest;

    if (request.version != "HTTP/1.1")
        return ParseResult::BadRequest;

    request.headers.clear();

    while (getline(stream, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        size_t colon = line.find(':');

        if (colon == string::npos)
            return ParseResult::BadRequest;

        string name = trim(line.substr(0, colon));
        string value = trim(line.substr(colon + 1));

        if (name.empty())
            return ParseResult::BadRequest;

        name = toLower(name);

        request.headers[name] = value;
    }

    size_t bodyStart = headerEnd + 4;
    size_t contentLength = 0;

    if (request.hasHeader("content-length")) {
        string length = request.getHeader("content-length");

        try {
            size_t pos = 0;

            unsigned long long value =
                stoull(length, &pos);

            if (pos != length.size())
                return ParseResult::BadRequest;

            contentLength = static_cast<size_t>(value);
        }
        catch (...) {
            return ParseResult::BadRequest;
        }
    }

    if (buffer.size() - bodyStart < contentLength)
        return ParseResult::Incomplete;

    request.body =
        buffer.substr(bodyStart, contentLength);

    return ParseResult::Complete;
}

const HttpRequest& HttpParser::getRequest() const {
    return request;
}

void HttpParser::reset() {
    buffer.clear();
    request = HttpRequest();
}