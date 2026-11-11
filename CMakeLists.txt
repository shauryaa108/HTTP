cmake_minimum_required(VERSION 3.16)

project(HTTPServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server
    src/main.cpp

    src/server/server.cpp
    src/server/ClientConnection.cpp

    src/http/HttpParser.cpp
    src/http/HttpRequest.cpp
    src/http/HttpResponse.cpp
)

add_executable(client
    src/client.cpp
)

add_executable(parser_test
    tests/parser_test.cpp
    src/http/HttpParser.cpp
    src/http/HttpRequest.cpp
)

enable_testing()

add_test(
    NAME ParserTests
    COMMAND parser_test
)