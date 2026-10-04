#pragma once

class ScriptHost;

// Resolve and install the game-specific hooks after all AML mods have loaded.
// ScriptHost itself contains no game ABI, symbols, structures or offsets.
bool AttachGameSA(void* game, ScriptHost* host);
