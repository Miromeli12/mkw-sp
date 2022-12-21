#pragma once

#include "game/race/Jugem.hh"

namespace Race {

class JugemManager {
public:
    REPLACE void calc();
    REPLACE void setVisible(u32 playerId, bool visible);

    static JugemManager *Instance();

private:
    u8 _00[0x14 - 0x00];
    Jugem *m_jugems[12]; // Extended
    u32 m_count; // Moved

    static JugemManager *s_instance;
};

} // namespace Race
