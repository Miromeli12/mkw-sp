#pragma once

extern "C" {
#include <Common.h>
}

namespace UI {

class MessageGroup {
public:
    void dt(s32 type);

private:
    u8 _00[0x14 - 0x00];
};

} // namespace UI
