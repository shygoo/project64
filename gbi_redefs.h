
// Redefinitions and extensions for gbi.h

// mk64 beta f3dex gbi
#ifdef F3DEX_PATCH_095

#undef gsSP1Quadrangle
#define gsSP1Quadrangle(v0, v1, v2, v3, flag) \
{ \
    _SHIFTL(0xB5, 24, 8),     \
    (_SHIFTL((v0)*2, 24, 8) | \
     _SHIFTL((v1)*2, 16, 8) | \
     _SHIFTL((v2)*2,  8, 8) | \
     _SHIFTL((v3)*2,  0, 8))  \
}

#endif // F3DEX_PATCH_095

// sm64 beta f3d gbi?
#ifdef F3D_GBI

#undef gsSPPerspNormalize
#define gsSPPerspNormalize(s) \
{ \
    _SHIFTL(0xB4, 24, 8), (s)\
}

#endif // F3D_GBI

#if defined(F3D_GBI) || defined(F3DEX_PATCH_095)
// patch for gsSPTextureRectangle (correct?)
#undef G_RDPHALF_1
#undef G_RDPHALF_2
#define G_RDPHALF_1 0xB3
#define G_RDPHALF_2 0xB2
#endif