#include "IniReader.h"

IniRange IniRange_create(const char *s, size_t len) {
    return (IniRange){
        .mString = StringRange_create(s, len),
        .currentSection = StringView_create(""),
        .lineNum = 0,
        .state = 0,
    };
}
enum {
    STATE_NONE,
    STATE_WANT_SECTION,
    STATE_WANT_SECTION_END,
    STATE_IN_COMMENT,
    STATE_WANT_DELIM,
    STATE_WANT_VALUE,
    STATE_WANT_VALUE_END,
    STATE_IN_COMMENT_WANT_VALUE,
    STATE_POST_WANT_VALUE,
};
static bool IniRange_WantsValueEnd(int state) {
    switch (state) {
    case STATE_WANT_VALUE_END:
    case STATE_WANT_VALUE /* Empty value */:
    case STATE_IN_COMMENT_WANT_VALUE:
        return true;
    }

    return false;
}
bool IniRange_next(IniRange *self, IniProperty *prop) {
    assert(self != NULL);
    assert(prop != NULL);

    const char *token = NULL;
    const char *lastNonSpace = NULL;

    StringView key = StringView_create("");

    for (const char *it = self->mString.mView.s + self->mString.mPos;
            it < self->mString.mView.s + self->mString.mView.len;
            ++it, ++self->mString.mPos) {
        if (self->state == STATE_POST_WANT_VALUE) {
            ++self->lineNum;
            self->state = STATE_NONE;
        }
        switch (*it) {
        case ' ':
            if (self->state == STATE_NONE || self->state == STATE_WANT_DELIM ||
                    self->state == STATE_WANT_VALUE) {
                break;
            }
            // Treat as identifier
            goto Identifier;
            break;
        case '[':
            if (self->state == STATE_NONE) {
                token = it;
                self->state = STATE_WANT_SECTION;
                break;
            }
            // Treat as identifier
            goto Identifier;
            break;
        case ']':
            if (self->state == STATE_WANT_SECTION_END ||
                    self->state == STATE_WANT_SECTION /* Empty section tag */) {
                self->currentSection = (StringView){ .s = token, .len = it - token };
                self->state = STATE_NONE;
                break;
            }
            // Treat as identifier
            goto Identifier;
            break;
        case '\n':
            if (self->state == STATE_NONE || self->state == STATE_IN_COMMENT) {
                self->state = STATE_NONE;
                ++self->lineNum;
                break;
            }
            if (IniRange_WantsValueEnd(self->state)) {
                ++self->mString.mPos;
            StateWantValue:
                self->state = STATE_POST_WANT_VALUE;
                const StringView value = (StringView){
                    .s = token,
                    .len = lastNonSpace + 1 - token,
                };
                *prop = (IniProperty){
                    .section = self->currentSection,
                    .key = key,
                    .value = value,
                };
                return true;
            }
            // Error
            return false;
        case '#':
        case ';':
            if (self->state == STATE_NONE) {
                self->state = STATE_IN_COMMENT;
                break;
            }
            if (IniRange_WantsValueEnd(self->state)) {
                self->state = STATE_IN_COMMENT_WANT_VALUE;
                break;
            }
            // Error
            return false;
        case '=':
            if (self->state == STATE_WANT_DELIM) {
                self->state = STATE_WANT_VALUE;
                key = (StringView){
                    .s = token,
                    .len = lastNonSpace + 1 - token,
                };
                break;
            }
            // Error
            return false;
        default:
            if (self->state == STATE_IN_COMMENT) {
                break;
            }
            if (self->state == STATE_NONE) {
                self->state = STATE_WANT_DELIM;
                token = it;
                break;
            }
            // Treat as identifier
        Identifier:
            if (self->state == STATE_WANT_SECTION) {
                token = it;
                self->state = STATE_WANT_SECTION_END;
            }
            if (self->state == STATE_WANT_VALUE) {
                token = it;
                self->state = STATE_WANT_VALUE_END;
            }
            if (self->state == STATE_IN_COMMENT_WANT_VALUE) {
                break;
            }
            lastNonSpace = it;
            break;
        }
    }
    if (IniRange_WantsValueEnd(self->state)) {
        goto StateWantValue;
    }
    if (self->state == STATE_NONE || self->state == STATE_IN_COMMENT) {
        return true;
    }
    return false;
}
