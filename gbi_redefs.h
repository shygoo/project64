
// mk64 quad
#define gsSP1Quadrangle(v0, v1, v2, v3, flag) \
{ \
    _SHIFTL(0xB5, 24, 8),   \
    (_SHIFTL((v0), 24, 8) | \
     _SHIFTL((v1), 16, 8) | \
     _SHIFTL((v2),  8, 8) | \
     _SHIFTL((v3),  0, 8))  \
}
