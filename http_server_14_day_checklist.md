# C++ HTTP Server --- 14-Day Execution Checklist

> **Project goal:** Finish the core of a C++ HTTP/1.1 server that can
> serve a real static website, expose a small API, handle concurrent
> clients, pass meaningful tests, report benchmark results, and be
> demonstrated through a public URL.
>
> **Working approach:** Preserve the existing code. Use AI to accelerate
> implementation, but compile, test, review, and understand every
> important change.
>
> **Current known state** - \[x\] Project has `src/http/`,
> `src/server/`, `public/`, `tests/`, and `CMakeLists.txt`. - \[x\]
> Existing source includes `HttpParser`, `HttpRequest`, `HttpResponse`,
> and `Server` components. - \[x\] `public/` contains `index.html`,
> `style.css`, and `script.js`. - \[x\] A request to
> `http://localhost:4221/` returned `404 Not Found`; the server is
> reachable, but static-file serving is not implemented or not wired up
> yet. - \[ \] Confirm a clean build and current parser-test results in
> Ubuntu WSL. - \[ \] Inspect the current source before deciding exact
> implementation changes.
>
> **Status convention:** Check a box only after the requirement is
> implemented and verified. If blocked, add a note under the task.

------------------------------------------------------------------------

## 0. Project Safety and Baseline

### 0.1 Protect current work

-   [ ] Review `git status` and identify intentional changes.
    -   **Requirement:** Understand the existing deleted and untracked
        files before staging or committing anything.
-   [ ] Create a checkpoint commit or backup once the current state is
    understood.
    -   **Requirement:** Have a recoverable point before substantial
        edits. Do not blindly run `git add -A` if unrelated/deleted
        files are present.
-   [ ] Confirm the project root and current directory in WSL.
    -   **Requirement:** Commands should run against
        `/mnt/c/Users/yashg/Desktop/HTTP` or the actual project path.

### 0.2 Establish a reproducible build

-   [ ] Verify build tools in Ubuntu WSL.
    -   **Requirement:** `g++ --version`, `cmake --version`, and
        `make --version` work. Install missing tools only if needed.
-   [ ] Configure a separate WSL build directory.
    -   **Requirement:** Use `cmake -S . -B build-wsl` so the existing
        `build/` directory is not overwritten.
-   [ ] Build all targets.
    -   **Requirement:** `cmake --build build-wsl -j2` succeeds, or
        errors are recorded and fixed.
-   [ ] Run the current tests and record baseline results.
    -   **Requirement:**
        `ctest --test-dir build-wsl --output-on-failure` completes;
        record passing and failing tests.
-   [ ] Record current behavior of the server and client.
    -   **Requirement:** Note current port, startup output, existing
        endpoints, and known failures.

**Milestone 0:** We can build the current code from a clean directory
and know which tests currently pass.

------------------------------------------------------------------------

## 1. HTTP Request Parsing and Protocol Correctness

### 1.1 Request line

-   [ ] Parse method, request target, and HTTP version.
    -   **Requirement:** Reject incomplete or malformed request lines
        instead of accessing missing fields.
-   [ ] Validate supported HTTP versions.
    -   **Requirement:** Handle supported HTTP/1.0 and/or HTTP/1.1
        behavior deliberately; reject unsupported versions with an
        appropriate response.
-   [ ] Validate method and target syntax.
    -   **Requirement:** Malformed input must not crash the process or
        create an invalid request object.

### 1.2 Headers

-   [ ] Parse headers into a consistent representation.
    -   **Requirement:** Header names are handled case-insensitively;
        whitespace and malformed header lines are handled safely.
-   [ ] Handle duplicate or conflicting framing headers safely.
    -   **Requirement:** Reject ambiguous or conflicting
        `Content-Length` values. Do not interpret a request body
        differently in different components.
-   [ ] Validate `Host` according to the supported HTTP version.
    -   **Requirement:** For HTTP/1.1, missing or invalid `Host` is
        rejected appropriately.
-   [ ] Enforce request-line and header-size limits.
    -   **Requirement:** Very large headers or request lines cannot
        consume unbounded memory.

### 1.3 Body and fragmented TCP input

-   [ ] Accumulate bytes across multiple `recv()` calls.
    -   **Requirement:** A request split across several TCP reads is
        parsed correctly.
-   [ ] Read request bodies according to validated `Content-Length`.
    -   **Requirement:** Do not assume one `recv()` returns the complete
        request or body.
-   [ ] Enforce a maximum body size.
    -   **Requirement:** Oversized bodies are rejected with a suitable
        error and do not cause unbounded allocation.
-   [ ] Define behavior for unsupported transfer encodings.
    -   **Requirement:** If chunked request bodies are not implemented,
        reject them explicitly rather than parsing them incorrectly.
-   [ ] Define behavior for incomplete requests and client disconnects.
    -   **Requirement:** Incomplete requests do not hang indefinitely or
        crash the server.

### 1.4 Parser tests

-   [ ] Test valid GET, HEAD, and POST requests.
-   [ ] Test missing request-line fields and malformed headers.
-   [ ] Test header-name casing and whitespace.
-   [ ] Test body parsing and zero-length bodies.
-   [ ] Test requests split across input chunks.
-   [ ] Test missing, invalid, duplicate, and conflicting
    `Content-Length`.
-   [ ] Test oversized headers and bodies.
-   [ ] Test unsupported transfer encoding and incomplete requests.

**Requirements for completion:** Parser behavior is deterministic,
bounded, and covered by automated tests. Error cases return clear
failures instead of undefined behavior or process crashes.

**Milestone 1:** The parser handles expected input and rejects malformed
or ambiguous requests safely.

------------------------------------------------------------------------

## 2. HTTP Response Construction

### 2.1 Response format

-   [ ] Produce a valid HTTP status line.
    -   **Requirement:** Include the HTTP version, numeric status code,
        and reason phrase in the correct format.
-   [ ] Serialize headers correctly.
    -   **Requirement:** Use CRLF line endings and a blank line between
        headers and body.
-   [ ] Calculate `Content-Length` from the actual response body bytes.
    -   **Requirement:** Count bytes, not characters; do not include
        headers in the body length.
-   [ ] Set an appropriate `Content-Type`.
    -   **Requirement:** HTML, CSS, JavaScript, JSON, plain text, and
        images receive suitable types.
-   [ ] Make connection behavior explicit.
    -   **Requirement:** If the implementation closes each connection
        after a response, return `Connection: close` and close it
        consistently.
-   [ ] Support bodyless HEAD responses.
    -   **Requirement:** HEAD returns the headers that correspond to GET
        but sends no response body.
-   [ ] Handle empty and binary response bodies.
    -   **Requirement:** Binary files are sent byte-for-byte without
        text conversions or truncation.

### 2.2 Sending data safely

-   [ ] Handle partial socket sends.
    -   **Requirement:** Keep sending until all response bytes are
        written or a socket error occurs.
-   [ ] Handle client disconnects during response transmission.
    -   **Requirement:** A disconnected client must not crash the whole
        server.
-   [ ] Avoid unsafe global or shared response state.
    -   **Requirement:** Each request/response has clear ownership,
        especially when concurrency is introduced.

### 2.3 Response tests

-   [ ] Test status line and header serialization.
-   [ ] Test exact `Content-Length` for text and binary bodies.
-   [ ] Test HEAD responses contain no body.
-   [ ] Test partial-send/error handling where practical.
-   [ ] Test 400, 404, 405, 413, and 500 responses as applicable.

**Milestone 2:** Responses are valid, correctly framed, and reliably
transmitted.

------------------------------------------------------------------------

## 3. Static Website Serving

### 3.1 Document root and routing

-   [ ] Configure a document root.
    -   **Requirement:** Use a known directory, initially the project's
        `public/` folder. Avoid relying on the process's current working
        directory accidentally.
-   [ ] Map `/` to `index.html`.
    -   **Requirement:** `GET /` serves `public/index.html`.
-   [ ] Map URL paths to files under the document root.
    -   **Requirement:** `/style.css` maps to `public/style.css`; image
        and script paths work similarly.
-   [ ] Define behavior for directories and missing files.
    -   **Requirement:** Missing files return 404; directory requests do
        not expose directory listings unless deliberately implemented.
-   [ ] Support GET and HEAD for static files.
    -   **Requirement:** Other methods receive an appropriate
        method-not-allowed response when the route does not support
        them.

### 3.2 MIME types and file handling

-   [ ] Add a small MIME-type mapping.
    -   **Requirement:** At minimum support `.html`, `.css`, `.js`,
        `.json`, common image formats, `.svg`, and plain text.
-   [ ] Read and serve binary files correctly.
    -   **Requirement:** Images and other binary files are not corrupted
        by text-mode handling.
-   [ ] Handle file read failures.
    -   **Requirement:** Permission failures or I/O errors do not crash
        the server or leak filesystem details.
-   [ ] Avoid loading unbounded files into memory.
    -   **Requirement:** Set reasonable file-size limits or use a
        bounded/streaming approach if needed for the chosen scope.

### 3.3 Filesystem security

-   [ ] Prevent path traversal.
    -   **Requirement:** Requests containing traversal segments or
        encoded variants cannot access files outside the document root.
-   [ ] Normalize and validate URL paths safely.
    -   **Requirement:** Handle percent-encoding and path separators
        deliberately; reject malformed encodings where necessary.
-   [ ] Prevent accidental exposure of project files.
    -   **Requirement:** Only files beneath the configured public
        document root are eligible to be served.
-   [ ] Test symlink behavior.
    -   **Requirement:** Decide whether symlinks are allowed; ensure
        they cannot bypass the document-root boundary if the security
        design forbids that.
-   [ ] Test traversal and encoding edge cases.
    -   **Requirement:** Try `../`, encoded traversal, absolute paths,
        repeated separators, and paths to source/build/config files.

### 3.4 Static serving integration tests

-   [ ] `GET /` returns 200 and the expected HTML.
-   [ ] CSS, JavaScript, and image requests return the correct bytes and
    content types.
-   [ ] HEAD returns headers without a body.
-   [ ] Missing file returns 404.
-   [ ] Unsupported method returns 405 with an `Allow` header where
    appropriate.
-   [ ] Traversal attempts are rejected.
-   [ ] Run the site in a browser and verify all assets load.

**Milestone 3:** The complete `public/` website works through the C++
server, with filesystem boundaries enforced.

------------------------------------------------------------------------

## 4. Basic API and POST Support

### 4.1 Routing

-   [ ] Introduce the smallest useful dispatcher/router.
    -   **Requirement:** Separate route selection from socket handling
        without creating an unnecessarily elaborate framework.
-   [ ] Match routes by method and path.
    -   **Requirement:** A known path with the wrong method can return
        405; an unknown path returns 404.
-   [ ] Keep handler responsibilities clear.
    -   **Requirement:** Static-file logic and API logic do not become
        tangled in the low-level socket loop.

### 4.2 API endpoints

-   [ ] Add `GET /api/health`.
    -   **Requirement:** Return 200 and a small predictable JSON
        response indicating the server is responding.
-   [ ] Add `POST /api/echo` or another small POST endpoint.
    -   **Requirement:** Read the validated request body and return a
        defined response. Do not add a database unless separately
        justified.
-   [ ] Set JSON content types for API responses.
    -   **Requirement:** JSON responses include
        `Content-Type: application/json`.
-   [ ] Validate request bodies.
    -   **Requirement:** Missing, malformed, or oversized bodies receive
        appropriate error responses.
-   [ ] Define unsupported content-type behavior.
    -   **Requirement:** If an endpoint expects JSON, validate
        `Content-Type` and return a clear error for unsupported input.

### 4.3 API tests

-   [ ] Health endpoint returns the expected JSON and status.
-   [ ] POST endpoint handles valid bodies.
-   [ ] POST endpoint handles empty and oversized bodies.
-   [ ] Unknown routes return 404.
-   [ ] Wrong methods return 405.
-   [ ] Malformed input does not crash the server.

**Milestone 4:** Static files and a small, tested API share the same
server.

------------------------------------------------------------------------

## 5. Concurrency, Socket Lifecycle, and Reliability

### 5.1 Choose and document a concurrency model

-   [ ] Inspect the current accept loop and choose the simplest safe
    model.
    -   **Requirement:** Prefer a bounded thread pool if compatible with
        the current code; do not introduce a complex event loop without
        a demonstrated need.
-   [ ] Define worker and queue limits.
    -   **Requirement:** Avoid creating unlimited threads or accepting
        unlimited queued work.
-   [ ] Define behavior when the queue is full.
    -   **Requirement:** Reject, defer, or otherwise handle overload
        deliberately rather than consuming unbounded resources.
-   [ ] Keep per-connection state isolated.
    -   **Requirement:** Request buffers, parsers, and responses are not
        accidentally shared across clients.

### 5.2 Resource ownership and shutdown

-   [ ] Apply RAII to socket and file resources where practical.
    -   **Requirement:** Resources close reliably on normal completion
        and error paths.
-   [ ] Make thread-pool shutdown safe.
    -   **Requirement:** Stop accepting work, wake workers, finish or
        cancel queued work according to the documented policy, and join
        threads.
-   [ ] Handle accept, read, write, and bind errors.
    -   **Requirement:** Log useful diagnostic messages without crashing
        unrelated requests.
-   [ ] Add socket/read/write timeouts or another bounded strategy.
    -   **Requirement:** Slow or stalled clients cannot occupy resources
        forever.
-   [ ] Handle client disconnects and server shutdown.
    -   **Requirement:** Disconnects are normal error paths, not fatal
        process errors.

### 5.3 Concurrency tests

-   [ ] Test several simultaneous clients.
-   [ ] Test repeated requests from many clients.
-   [ ] Test slow/incomplete clients.
-   [ ] Test client disconnects during request and response.
-   [ ] Test overload behavior when the queue is full.
-   [ ] Test clean shutdown while clients are connected.
-   [ ] Check for data races or shared-state bugs using suitable tooling
    where available.

**Milestone 5:** The server handles concurrent clients within documented
resource limits and shuts down predictably.

------------------------------------------------------------------------

## 6. Security and Operational Limits

-   [ ] Set maximum request-line, header, body, and file sizes.
    -   **Requirement:** All limits are documented and enforced before
        unbounded allocation.
-   [ ] Validate all input that affects parsing, routing, or filesystem
    access.
    -   **Requirement:** Untrusted requests cannot cause out-of-bounds
        access, path escape, or uncontrolled resource consumption.
-   [ ] Avoid leaking sensitive error details.
    -   **Requirement:** Client responses are concise; diagnostic
        details stay in server logs.
-   [ ] Avoid logging sensitive request bodies by default.
    -   **Requirement:** Logs do not accidentally expose credentials or
        private data.
-   [ ] Handle malformed and unexpected requests without terminating the
    process.
    -   **Requirement:** One bad client should not take down the server.
-   [ ] Document limitations.
    -   **Requirement:** Clearly state that the project is a
        learning/portfolio server and not a hardened replacement for a
        mature production server.
-   [ ] Review public-exposure risks before opening the tunnel.
    -   **Requirement:** Serve only the intended document root, use safe
        limits, and do not expose private files or development secrets.

**Milestone 6:** The server has explicit limits, safe failure behavior,
and documented security boundaries.

------------------------------------------------------------------------

## 7. Build System and Code Quality

-   [ ] Confirm the project builds from a clean build directory.
    -   **Requirement:** A fresh CMake configure and build succeeds in
        the documented environment.
-   [ ] Keep Windows and Linux build artifacts separate.
    -   **Requirement:** Do not reuse a CMake cache configured for
        another OS or compiler.
-   [ ] Improve CMake targets only where needed.
    -   **Requirement:** Server, client, and tests build predictably;
        avoid unrelated build-system refactors.
-   [ ] Enable useful compiler warnings.
    -   **Requirement:** Address meaningful warnings introduced by the
        project without spending days on irrelevant toolchain noise.
-   [ ] Ensure tests are discoverable through CTest.
    -   **Requirement:** The documented test command runs the intended
        suite.
-   [ ] Remove accidental generated artifacts from version control.
    -   **Requirement:** Add appropriate ignore rules for build output
        and local files without deleting existing work.
-   [ ] Review naming, error messages, and comments.
    -   **Requirement:** Improve clarity where it helps maintenance and
        explanation; avoid cosmetic rewrites.

**Milestone 7:** A new checkout can be configured, built, and tested
using documented commands.

------------------------------------------------------------------------

## 8. Benchmarking and Load Testing

### 8.1 Establish a repeatable benchmark

-   [ ] Select an available HTTP benchmarking tool.
    -   **Requirement:** Use a tool that can generate repeatable
        requests against the local server; document installation and
        invocation.
-   [ ] Benchmark a small static file.
    -   **Requirement:** Record the request count, concurrency,
        duration, throughput, and latency.
-   [ ] Benchmark a larger static file or a representative payload.
    -   **Requirement:** Observe whether file size changes throughput
        and latency.
-   [ ] Run concurrent-client tests.
    -   **Requirement:** Test more than one concurrency level, including
        a modest level such as 10 and a higher level such as 50 or 100
        if the machine supports it.
-   [ ] Record environment details.
    -   **Requirement:** Include CPU, OS/WSL context, compiler/build
        type, tool, request size, concurrency, and test duration.
-   [ ] Repeat runs and report observed values honestly.
    -   **Requirement:** Do not claim an unmeasured requests-per-second
        figure or compare results from different conditions as if
        equivalent.

### 8.2 Evaluate results

-   [ ] Record throughput (requests/second).
-   [ ] Record latency (preferably median and tail latency if the tool
    supports it).
-   [ ] Record error counts and failed requests.
-   [ ] Identify one clear bottleneck or limitation.
-   [ ] Make a small, justified optimization only if time allows.
-   [ ] Rerun the same benchmark after the optimization.

**Milestone 8:** The README contains reproducible benchmark results and
clearly stated limitations.

------------------------------------------------------------------------

## 9. Public Demo Using the Existing Laptop

### 9.1 Local readiness

-   [ ] Start the server from the documented command.
-   [ ] Verify `GET /` locally.
-   [ ] Verify CSS, JavaScript, images, and API routes locally.
-   [ ] Confirm the server is bound to the intended local
    interface/port.
-   [ ] Confirm the request limits and path-safety checks are in place
    before public exposure.

### 9.2 Tunnel setup

-   [ ] Install `cloudflared` on Windows using official instructions.
    -   **Requirement:** Use the official Cloudflare installation
        documentation for the selected Windows installation method.
-   [ ] Start the C++ server on its configured port, for example `4221`.
-   [ ] Start a Quick Tunnel in a second PowerShell terminal.
    -   **Requirement:** If `cloudflared.exe` is on PATH, run:

        ``` powershell
        cloudflared tunnel --url http://localhost:4221
        ```

    -   **Requirement:** If it is not on PATH, use the verified full
        path to `cloudflared.exe`.
-   [ ] Copy the generated public `trycloudflare.com` URL.
    -   **Requirement:** Treat it as temporary; Quick Tunnel URLs can
        change between runs.
-   [ ] Test from a different device or network.
    -   **Requirement:** Verify that the public URL loads the site and
        that assets/API routes work.
-   [ ] Document how to start and stop the server and tunnel.
    -   **Requirement:** Explain that the laptop must remain awake and
        connected, and both processes must keep running.

### 9.3 Public demo boundaries

-   [ ] Do not expose source code, build directories, configuration
    files, or unrelated folders.
-   [ ] Do not put credentials or secrets in the public directory.
-   [ ] Do not treat a temporary tunnel as a guaranteed production
    deployment.
-   [ ] Record any network, firewall, or WSL-related constraints
    encountered.

**Milestone 9:** The website is publicly reachable through a temporary
URL while the laptop and tunnel are running.

------------------------------------------------------------------------

## 10. Documentation and Final Demo

-   [ ] Write a clear README overview.
    -   **Requirement:** Explain what the server does, the motivation,
        and the current feature set.
-   [ ] Document architecture and request flow.
    -   **Requirement:** Explain the listener, connection handling,
        parser, dispatcher, handlers, and response construction.
-   [ ] Document build and run instructions.
    -   **Requirement:** A reviewer can build and run the project from a
        clean checkout.
-   [ ] Document test commands and coverage.
    -   **Requirement:** Include the CMake/CTest commands and important
        test categories.
-   [ ] Document security choices and limitations.
    -   **Requirement:** Explain path traversal protection, size limits,
        concurrency limits, and unsupported HTTP features.
-   [ ] Add reproducible benchmark results.
    -   **Requirement:** Include test conditions and avoid unsupported
        performance claims.
-   [ ] Add a short demo walkthrough.
    -   **Requirement:** Demonstrate static serving, an API request, an
        error case, and the public URL if available.
-   [ ] Prepare interview explanations.
    -   **Requirement:** Be able to explain ownership/RAII, TCP
        read/write behavior, HTTP framing, concurrency model, security
        decisions, and measured bottlenecks.
-   [ ] Review Git status and commit the final core project.
    -   **Requirement:** Include intentional source, tests,
        documentation, and configuration; exclude generated build output
        and secrets.

**Milestone 10:** The project is demonstrable, reproducible, documented,
and explainable.

------------------------------------------------------------------------

## Suggested 14-Day Schedule

  -----------------------------------------------------------------------
  Day                                 Primary deliverable
  ----------------------------------- -----------------------------------
  1                                   Baseline build, tests, Git safety,
                                      code audit

  2                                   Parser and response gap analysis;
                                      fix highest-priority correctness
                                      issues

  3                                   Request parsing and malformed-input
                                      tests

  4                                   Response framing, body handling,
                                      partial sends

  5                                   Document root, `/` → `index.html`,
                                      basic static files

  6                                   MIME types, HEAD, path traversal
                                      protection, static integration
                                      tests

  7                                   Minimal router and API health
                                      endpoint

  8                                   POST endpoint, body validation, API
                                      tests

  9                                   Choose and implement bounded
                                      concurrency model

  10                                  Timeouts, shutdown, disconnects,
                                      concurrency tests

  11                                  Security/limits review and
                                      regression tests

  12                                  Benchmarks and public tunnel demo

  13                                  Fix critical issues; build/test
                                      instructions and README

  14                                  Final regression, demo, interview
                                      explanations, checkpoint commit
  -----------------------------------------------------------------------

> **Schedule rule:** If a day slips, do not automatically extend the
> workday or rewrite the plan. Prioritize HTTP correctness, safe static
> serving, tests, and a demonstrable deployment. Advanced optimization
> and optional polish can move to the post-sprint backlog.

------------------------------------------------------------------------

## Progress Update Template

Copy this section into your update whenever you want to report progress.
You can also simply tell me what you did and what failed; I can help
update the checklist.

``` text
Date / Day:
Today's intended deliverable:
Completed tasks:
Tests run and results:
Current build status:
Current blocker:
Files changed:
What I understand now:
Next smallest step:
Energy today (low / medium / high):
```

## Post-Sprint Backlog --- Not Required for the 14-Day Core

-   [ ] Keep-alive and multiple requests per connection.
-   [ ] Chunked transfer encoding.
-   [ ] More complete HTTP/1.1 compliance.
-   [ ] HTTPS termination or a reverse-proxy deployment.
-   [ ] More advanced non-blocking I/O/event-loop architecture.
-   [ ] Sanitizers, fuzz testing, and deeper performance profiling.
-   [ ] More comprehensive API routing and request-body formats.
-   [ ] Long-running service management and uptime monitoring.

These are intentionally optional. Do not start them until the core
milestones are passing.
