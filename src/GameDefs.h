#pragma once

// Game window dimensions
constexpr int WINDOW_WIDTH{ 800 };
constexpr int WINDOW_HEIGHT{ 800 };

// Player parameters
constexpr const char* PLAYER_NODE_NAME{ "PlayerShip" };
constexpr const char* BULLET_NODE_NAME{ "Bullet" };
constexpr unsigned int NUM_LIVES{ 3 };// Initial number of lives
constexpr float PLAYER_MOVE_SPEED{ 4.0f };// Movement speed as world units per second
constexpr float BULLET_MOVE_SPEED{ 8.0f };
constexpr float SHOOT_COOLDOWN{ 0.1f };// Shooting cooldown time in seconds
constexpr float SHOOT_GROUPING_SIZE{ 10.0f };

// Seeker parameters
constexpr const char* SEEKER_NODE_NAME{ "Seeker" };
constexpr float SEEKER_MOVE_SPEED{ 2.0f };
constexpr float SEEKER_SPAWN_INTERVAL{ 1.0f };
constexpr float SEEKER_INACTIVITY_PERIOD{ 2.0f };
static const StringHash VAR_SEEKER_INACTIVITY_TIMER{ "InactivityTimer" };
static const StringHash SEEKER_RESPAWN_TIMER{ "SeekerRespawnTimer" };

// Wanderer parameters
constexpr const char* WANDERER_NODE_NAME{ "Wanderer" };
constexpr float WANDERER_MOVE_SPEED{ 2.0f };
constexpr float WANDERER_ROLL_SPEED{ 3.0f };
constexpr float WANDERER_EVADE_RADIUS{ 1.0f };
constexpr float WANDERER_SPAWN_INTERVAL{ 1.0f };
constexpr unsigned int SCORES_FOR_WANDERER{ 10 };
static const StringHash WANDERER_RESPAWN_TIMER{ "WandererRespawnTimer" };

// Black hole parameters
constexpr const char* BLACKHOLE_NODE_NAME{ "Black Hole" };
constexpr float BLACKHOLE_ROLL_SPEED{ 3.0f };
constexpr float BLACKHOLE_SPAWN_INTERVAL{ 2.0f };
static const StringHash VAR_BLACKHOLE_HP{ "BlackHoleHP" };
constexpr unsigned int BLACKHOLE_HP{ 10 };
static const StringHash BLACKHOLE_RESPAWN_TIMER{ "BlackHoleRespawnTimer" };

// UI Element names
static const String UI_SCORES_TEXT{ "ScoresText" };
static const String UI_HSCORES_TEXT{ "HScoresText" };
static const String UI_LIVES_TEXT{ "LivesText" };

static const StringHash RECYCLE_DEPLETED_PARTICLE_EMITTERS_TIMER { "RecycleDepletedParticleEmitters" };