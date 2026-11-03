#include "HttpResponse.h"

#include <sstream>

using namespace std;

HttpResponse::HttpResponse()
    : statusCode(200),
      statusText("OK") {
}

void HttpResponse::setStatus(
    int code,
    const string& text
) {
    statusCode = code;
    statusText = text;
}

void HttpResponse::setHeader(
    const string& name,
    const string& value
) {
    headers[name] = value;
}

void HttpResponse::setBody(const string& body) {
    this->body = body;
}

string HttpResponse::toString(bool headRequest) const {
    ostringstream response;

    response << "HTTP/1.1 "
             << statusCode
             << " "
             << statusText
             << "\r\n";

    for (const auto& [name, value] : headers) {
        response << name << ": "
                 << value
                 << "\r\n";
    }

    response << "Content-Length: "
             << body.size()
             << "\r\n";

    response << "Connection: close\r\n";
    response << "\r\n";

    if (!headRequest)
        response << body;

    return response.str();
}

HttpResponse HttpResponse::ok(const string& body) {
    HttpResponse response;

    response.setStatus(200, "OK");
    response.setHeader("Content-Type", "text/plain");
    response.setBody(body);

    return response;
}

HttpResponse HttpResponse::badRequest() {
    HttpResponse response;

    response.setStatus(400, "Bad Request");
    response.setHeader("Content-Type", "text/plain");
    response.setBody("400 Bad Request\n");

    return response;
}

HttpResponse HttpResponse::notFound() {
    HttpResponse response;

    response.setStatus(404, "Not Found");
    response.setHeader("Content-Type", "text/plain");
    response.setBody("404 Not Found\n");

    return response;
}

HttpResponse HttpResponse::methodNotAllowed() {
    HttpResponse response;

    response.setStatus(405, "Method Not Allowed");
    response.setHeader("Content-Type", "text/plain");
    response.setBody("405 Method Not Allowed\n");

    return response;
}

HttpResponse HttpResponse::internalServerError() {
    HttpResponse response;

    response.setStatus(500, "Internal Server Error");
    response.setHeader("Content-Type", "text/plain");
    response.setBody("500 Internal Server Error\n");

    return response;
}