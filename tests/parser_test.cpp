#include "../src/http/HttpParser.h"

#include <cassert>
#include <iostream>

using namespace std;

void testRequestLine() {
    HttpParser parser;

    ParseResult result = parser.feed(
        "GET /hello HTTP/1.1\r\n"
        "\r\n"
    );

    assert(result == ParseResult::Complete);

    const HttpRequest& request =
        parser.getRequest();

    assert(request.method == "GET");
    assert(request.target == "/hello");
    assert(request.version == "HTTP/1.1");
}

void testHeaders() {
    HttpParser parser;

    ParseResult result = parser.feed(
        "GET /hello HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: curl/8.0\r\n"
        "Accept: */*\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
    );

    assert(result == ParseResult::Complete);

    const HttpRequest& request =
        parser.getRequest();

    assert(request.getHeader("host") == "localhost");
    assert(request.getHeader("user-agent") == "curl/8.0");
    assert(request.getHeader("accept") == "*/*");
    assert(request.getHeader("connection") == "keep-alive");
}

void testPartialRequest() {
    HttpParser parser;

    ParseResult result = parser.feed(
        "GET /hello HTTP/1.1\r\nHos"
    );

    assert(result == ParseResult::Incomplete);

    result = parser.feed(
        "t: localhost\r\n\r\n"
    );

    assert(result == ParseResult::Complete);

    const HttpRequest& request =
        parser.getRequest();

    assert(request.method == "GET");
    assert(request.target == "/hello");
    assert(request.getHeader("host") == "localhost");
}

void testContentLength() {
    HttpParser parser;

    ParseResult result = parser.feed(
        "POST /hello HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello"
    );

    assert(result == ParseResult::Complete);

    const HttpRequest& request =
        parser.getRequest();

    assert(request.method == "POST");
    assert(request.body == "hello");
}

void testPartialBody() {
    HttpParser parser;

    ParseResult result = parser.feed(
        "POST /hello HTTP/1.1\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hel"
    );

    assert(result == ParseResult::Incomplete);

    result = parser.feed("lo");

    assert(result == ParseResult::Complete);

    assert(parser.getRequest().body == "hello");
}

void testBadRequest() {
    HttpParser parser;

    ParseResult result = parser.feed(
        "GET /hello\r\n"
        "\r\n"
    );

    assert(result == ParseResult::BadRequest);
}

int main() {
    testRequestLine();
    testHeaders();
    testPartialRequest();
    testContentLength();
    testPartialBody();
    testBadRequest();

    cout << "All parser tests passed\n";

    return 0;
}