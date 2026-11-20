// QuickLootBezelCompose.h -- the six Quick Loot hardware bezels composed in C++ (doc 52 QL15, option A). A port of the bezel
// builders in .claude/research/quick_loot/render_hardware_bezels.py (build_computer_bezel, build_mitered_circuit_bezel,
// build_store_bezel, build_pause_gold_bezel, apply_finished_chassis_corners, make_tinted_crt, get_crt_pattern). Pure: no Win32,
// no engine calls, only <math.h> and the standard library, so the host test can run it.
//
// Pillow behaviours reproduced (the goal is parity with the Python output):
//  - Image.resize(.., LANCZOS): a = 3 kernel, support widened by the downscale factor, normalised weights, horizontal pass then
//    vertical pass, Pillow's 8-bit fixed-point path (22 fractional bits, round to nearest, clip after each pass). RGBA images
//    are resized premultiplied (Pillow converts RGBA -> RGBa -> resize -> RGBa -> RGBA), 'L' images are not.
//  - Image.alpha_composite: Pillow's integer src-over (7 extra precision bits, SHIFTFORDIV255 rounding); src alpha 0 keeps dst.
//  - Image.paste without a mask: plain replace including alpha.  Image.transpose with Pillow's definitions (ROTATE_90 is
//    counter-clockwise, ROTATE_270 clockwise).  Image.convert("L"): (19595 R + 38470 G + 7471 B + 0x8000) >> 16.
//  - numpy: float32 -> uint8 casts TRUNCATE; float32 steps stay float32, float64 steps stay double (the 0.45 chamfer darken and
//    the alpha mask are double in numpy, the CRT tint and the miter blend are float32).
// Out-of-range crop / paste is an error (false), never padded.
#pragma once
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include "QuickLootImage.h"

namespace qlbzc {

enum Key : uint8_t { KeyDatapad, KeyComputer, KeyUpgrade, KeyWorkbench, KeyStore, KeyPauseGold, KeyCount };
constexpr unsigned kOutSize = 512;

// pnl[KeyDatapad] = pnl_datapad, pnl[KeyComputer] = pnl_computer_pc, pnl[KeyUpgrade] = pnl_upgrade_pc,
// pnl[KeyWorkbench] = pnl_workbench_pc, pnl[KeyStore] = pnl_store_pc, pnl[KeyPauseGold] = pnl_pause_pc.
struct Sources { qlimg::Image pnl[KeyCount]; };

namespace detail {

typedef qlimg::Image Img;

constexpr unsigned kPanel = 2048;   // the generator's crop coordinates for the five panels are written in 2048 units
constexpr int kFixBits = 22;        // Pillow PRECISION_BITS for the 8-bit resample path (32 - 8 - 2)

inline bool valid(const Img& i) { return i.w != 0 && i.h != 0 && i.px.size() >= static_cast<size_t>(i.w) * i.h * 4; }
inline size_t ix(unsigned w, unsigned x, unsigned y) { return (static_cast<size_t>(y) * w + x) * 4; }
inline void makeImg(Img* o, unsigned w, unsigned h) { o->w = w; o->h = h; o->px.assign(static_cast<size_t>(w) * h * 4, 0); }
inline uint8_t clip8(int32_t v) { return static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v)); }

// ---- Lanczos resample (Pillow Resample.c) -------------------------------------------------------------------------------
inline double sincF(double x) {
    if (x == 0.0) return 1.0;
    x *= 3.14159265358979323846;
    return sin(x) / x;
}
inline double lanczosF(double x) { return (x >= -3.0 && x < 3.0) ? sincF(x) * sincF(x / 3.0) : 0.0; }

struct Coeffs {
    int ksize = 0;
    std::vector<int> xmin, n;
    std::vector<int32_t> k;
};

inline void precompute(unsigned inSize, unsigned outSize, Coeffs* c) {
    const double scale = static_cast<double>(inSize) / static_cast<double>(outSize);
    const double fscale = scale < 1.0 ? 1.0 : scale;
    const double support = 3.0 * fscale;
    const int ksize = static_cast<int>(ceil(support)) * 2 + 1;
    c->ksize = ksize;
    c->xmin.assign(outSize, 0);
    c->n.assign(outSize, 0);
    c->k.assign(static_cast<size_t>(outSize) * ksize, 0);
    std::vector<double> tmp(static_cast<size_t>(ksize), 0.0);
    const double ss = 1.0 / fscale;
    for (unsigned xx = 0; xx < outSize; ++xx) {
        const double center = (xx + 0.5) * scale;
        int xmin = static_cast<int>(center - support + 0.5);
        if (xmin < 0) xmin = 0;
        int xmax = static_cast<int>(center + support + 0.5);
        if (xmax > static_cast<int>(inSize)) xmax = static_cast<int>(inSize);
        xmax -= xmin;
        if (xmax > ksize) xmax = ksize;
        double ww = 0.0;
        for (int x = 0; x < xmax; ++x) {
            const double w = lanczosF((x + xmin - center + 0.5) * ss);
            tmp[static_cast<size_t>(x)] = w;
            ww += w;
        }
        for (int x = 0; x < xmax; ++x) {
            if (ww != 0.0) tmp[static_cast<size_t>(x)] /= ww;
            const double v = tmp[static_cast<size_t>(x)] * static_cast<double>(1 << kFixBits);
            c->k[static_cast<size_t>(xx) * ksize + x] = static_cast<int32_t>(v < 0 ? -0.5 + v : 0.5 + v);
        }
        c->xmin[xx] = xmin;
        c->n[xx] = xmax;
    }
}

// Resample an 8-bit image with `ch` interleaved channels. Same size on an axis skips that pass (Pillow does the same).
inline bool resizeCh(const uint8_t* in, unsigned iw, unsigned ih, unsigned ch, unsigned ow, unsigned oh, std::vector<uint8_t>* out) {
    if (!in || iw == 0 || ih == 0 || ow == 0 || oh == 0 || !out) return false;
    std::vector<uint8_t> tmp;
    const uint8_t* cur = in;
    unsigned cw = iw;
    const int32_t half = 1 << (kFixBits - 1);
    if (ow != iw) {
        Coeffs c;
        precompute(iw, ow, &c);
        tmp.assign(static_cast<size_t>(ow) * ih * ch, 0);
        for (unsigned y = 0; y < ih; ++y)
            for (unsigned x = 0; x < ow; ++x) {
                const int32_t* k = &c.k[static_cast<size_t>(x) * c.ksize];
                const int x0 = c.xmin[x], n = c.n[x];
                for (unsigned q = 0; q < ch; ++q) {
                    int32_t s = half;
                    for (int i = 0; i < n; ++i) s += static_cast<int32_t>(in[(static_cast<size_t>(y) * iw + static_cast<unsigned>(x0 + i)) * ch + q]) * k[i];
                    tmp[(static_cast<size_t>(y) * ow + x) * ch + q] = clip8(s >> kFixBits);
                }
            }
        cur = tmp.data();
        cw = ow;
    }
    if (oh != ih) {
        Coeffs c;
        precompute(ih, oh, &c);
        std::vector<uint8_t> res(static_cast<size_t>(cw) * oh * ch, 0);
        for (unsigned y = 0; y < oh; ++y) {
            const int32_t* k = &c.k[static_cast<size_t>(y) * c.ksize];
            const int y0 = c.xmin[y], n = c.n[y];
            for (unsigned x = 0; x < cw; ++x)
                for (unsigned q = 0; q < ch; ++q) {
                    int32_t s = half;
                    for (int i = 0; i < n; ++i) s += static_cast<int32_t>(cur[(static_cast<size_t>(static_cast<unsigned>(y0 + i)) * cw + x) * ch + q]) * k[i];
                    res[(static_cast<size_t>(y) * cw + x) * ch + q] = clip8(s >> kFixBits);
                }
        }
        out->swap(res);
        return true;
    }
    out->assign(cur, cur + static_cast<size_t>(cw) * oh * ch);
    return true;
}

// Pillow MULDIV255 and the RGBA <-> RGBa conversions used around an RGBA resize.
inline uint32_t muldiv255(uint32_t a, uint32_t b) {
    const uint32_t t = a * b + 128;
    return ((t >> 8) + t) >> 8;
}

// ---- Image ops ---------------------------------------------------------------------------------------------------------
inline bool crop(const Img& s, unsigned l, unsigned t, unsigned r, unsigned b, Img* o) {
    if (!valid(s) || r <= l || b <= t || r > s.w || b > s.h) return false;
    Img res;
    makeImg(&res, r - l, b - t);
    for (unsigned y = 0; y < res.h; ++y) memcpy(&res.px[ix(res.w, 0, y)], &s.px[ix(s.w, l, t + y)], static_cast<size_t>(res.w) * 4);
    *o = res;
    return true;
}

inline bool paste(Img* d, const Img& s, unsigned x, unsigned y) {
    if (!valid(*d) || !valid(s) || x + s.w > d->w || y + s.h > d->h) return false;
    for (unsigned r = 0; r < s.h; ++r) memcpy(&d->px[ix(d->w, x, y + r)], &s.px[ix(s.w, 0, r)], static_cast<size_t>(s.w) * 4);
    return true;
}

// Pillow alpha_composite(dst, src) at (x, y): integer src-over, PRECISION_BITS 7, rounded divisions (Pillow AlphaComposite.c).
inline bool alphaComposite(Img* d, const Img& s, unsigned x, unsigned y) {
    if (!valid(*d) || !valid(s) || x + s.w > d->w || y + s.h > d->h) return false;
    for (unsigned r = 0; r < s.h; ++r)
        for (unsigned c = 0; c < s.w; ++c) {
            const uint8_t* sp = &s.px[ix(s.w, c, r)];
            uint8_t* dp = &d->px[ix(d->w, x + c, y + r)];
            if (sp[3] == 0) continue;   // out = dst
            const uint32_t blend = static_cast<uint32_t>(dp[3]) * (255u - sp[3]);
            const uint32_t outa255 = static_cast<uint32_t>(sp[3]) * 255u + blend;
            const uint32_t coef1 = static_cast<uint32_t>(sp[3]) * 255u * 255u * 128u / outa255;
            const uint32_t coef2 = 255u * 128u - coef1;
            uint8_t o[4];
            for (int q = 0; q < 3; ++q) {
                uint32_t t = sp[q] * coef1 + dp[q] * coef2 + (0x80u << 7);
                t = (t + (t >> 8)) >> 8;
                o[q] = static_cast<uint8_t>(t >> 7);
            }
            const uint32_t a = outa255 + 0x80u;
            o[3] = static_cast<uint8_t>((a + (a >> 8)) >> 8);
            memcpy(dp, o, 4);
        }
    return true;
}

inline bool flipLR(const Img& s, Img* o) {
    if (!valid(s)) return false;
    Img res;
    makeImg(&res, s.w, s.h);
    for (unsigned y = 0; y < s.h; ++y)
        for (unsigned x = 0; x < s.w; ++x) memcpy(&res.px[ix(s.w, x, y)], &s.px[ix(s.w, s.w - 1 - x, y)], 4);
    *o = res;
    return true;
}

// Pillow ROTATE_90 (counter-clockwise): out(x', y') = src(W-1-y', x'); result is H wide, W tall.
inline bool rot90(const Img& s, Img* o) {
    if (!valid(s)) return false;
    Img res;
    makeImg(&res, s.h, s.w);
    for (unsigned y = 0; y < res.h; ++y)
        for (unsigned x = 0; x < res.w; ++x) memcpy(&res.px[ix(res.w, x, y)], &s.px[ix(s.w, s.w - 1 - y, x)], 4);
    *o = res;
    return true;
}

// Pillow ROTATE_270 (clockwise 90): out(x', y') = src(y', H-1-x').
inline bool rot270(const Img& s, Img* o) {
    if (!valid(s)) return false;
    Img res;
    makeImg(&res, s.h, s.w);
    for (unsigned y = 0; y < res.h; ++y)
        for (unsigned x = 0; x < res.w; ++x) memcpy(&res.px[ix(res.w, x, y)], &s.px[ix(s.w, y, s.h - 1 - x)], 4);
    *o = res;
    return true;
}

}  // namespace detail

// PIL Image.resize((w, h), Image.LANCZOS) for an RGBA image (premultiplied round trip, as Pillow does).
inline bool resizeLanczos(const qlimg::Image& in, unsigned w, unsigned h, qlimg::Image* out) {
    using namespace detail;
    if (!out || !valid(in) || w == 0 || h == 0) return false;
    if (in.w == w && in.h == h) {   // Pillow returns a copy before any premultiply
        Img c;
        c.w = w; c.h = h;
        c.px.assign(in.px.begin(), in.px.begin() + static_cast<size_t>(w) * h * 4);
        *out = c;
        return true;
    }
    const size_t n = static_cast<size_t>(in.w) * in.h;
    std::vector<uint8_t> pm(n * 4);
    for (size_t i = 0; i < n; ++i) {   // RGBA -> RGBa
        const uint32_t a = in.px[i * 4 + 3];
        for (int q = 0; q < 3; ++q) pm[i * 4 + q] = static_cast<uint8_t>(muldiv255(in.px[i * 4 + q], a));
        pm[i * 4 + 3] = static_cast<uint8_t>(a);
    }
    std::vector<uint8_t> rs;
    if (!resizeCh(pm.data(), in.w, in.h, 4, w, h, &rs)) return false;
    Img res;
    makeImg(&res, w, h);
    for (size_t i = 0; i < static_cast<size_t>(w) * h; ++i) {   // RGBa -> RGBA
        const uint32_t a = rs[i * 4 + 3];
        for (int q = 0; q < 3; ++q) {
            if (a == 255 || a == 0) res.px[i * 4 + q] = rs[i * 4 + q];
            else {
                const uint32_t p = static_cast<uint32_t>(rs[i * 4 + q]) * 255u / a;
                res.px[i * 4 + q] = static_cast<uint8_t>(p > 255u ? 255u : p);
            }
        }
        res.px[i * 4 + 3] = static_cast<uint8_t>(a);
    }
    *out = res;
    return true;
}

namespace detail {

inline bool rs(const Img& src, unsigned l, unsigned t, unsigned r, unsigned b, unsigned w, unsigned h, Img* out) {
    Img c;
    return crop(src, l, t, r, b, &c) && resizeLanczos(c, w, h, out);
}

// get_crt_pattern: pnl_datapad (512) crop (100, 80, 400, 420), convert("L"). Kept as 8-bit gray; crt_f = gray / 255 (float32).
constexpr unsigned kCrtW = 300, kCrtH = 340;
inline bool crtGray(const Img& dp512, std::vector<uint8_t>* gray) {
    Img c;
    if (!crop(dp512, 100, 80, 400, 420, &c)) return false;
    gray->assign(static_cast<size_t>(kCrtW) * kCrtH, 0);
    for (size_t i = 0; i < gray->size(); ++i) {
        const uint32_t r = c.px[i * 4], g = c.px[i * 4 + 1], b = c.px[i * 4 + 2];
        (*gray)[i] = static_cast<uint8_t>((r * 19595u + g * 38470u + b * 7471u + 0x8000u) >> 16);
    }
    return true;
}

// make_tinted_crt: gray -> float32 /255 -> *255 -> uint8 (truncate) -> LANCZOS to (w, h) -> float32 /255 -> per channel
// clip(gf * tint * 255 * 2.2, 0, 255) truncated to uint8, alpha 255.
inline bool makeTintedCrt(const std::vector<uint8_t>& gray, unsigned w, unsigned h, const float tint[3], Img* out) {
    std::vector<uint8_t> g8(gray.size());
    for (size_t i = 0; i < gray.size(); ++i) {
        const float f = static_cast<float>(gray[i]) / 255.0f;
        g8[i] = static_cast<uint8_t>(f * 255.0f);
    }
    std::vector<uint8_t> rsz;
    if (!resizeCh(g8.data(), kCrtW, kCrtH, 1, w, h, &rsz)) return false;
    Img res;
    makeImg(&res, w, h);
    for (size_t i = 0; i < static_cast<size_t>(w) * h; ++i) {
        const float gf = static_cast<float>(rsz[i]) / 255.0f;
        for (int c = 0; c < 3; ++c) {
            float v = gf * tint[c];
            v = v * 255.0f;
            v = v * 2.2f;
            res.px[i * 4 + c] = static_cast<uint8_t>(v < 0.0f ? 0.0f : (v > 255.0f ? 255.0f : v));
        }
        res.px[i * 4 + 3] = 255;
    }
    *out = res;
    return true;
}

// apply_finished_chassis_corners: 45-degree chamfer, alpha 0 inside, darkened (x0.45, truncated, double) alpha-255 ring.
inline void chassisCorners(Img* im, int c) {
    const int W = static_cast<int>(im->w), H = static_cast<int>(im->h);
    struct Ed {
        Img* im; int c;
        void at(int x, int y, int d) const {
            uint8_t* p = &im->px[ix(im->w, static_cast<unsigned>(x), static_cast<unsigned>(y))];
            if (d < c - 2) p[3] = 0;
            else if (d <= c) {
                for (int k = 0; k < 3; ++k) p[k] = static_cast<uint8_t>(p[k] * 0.45);
                p[3] = 255;
            }
        }
    } ed = {im, c};
    for (int y = 0; y < c; ++y)
        for (int x = 0; x < c; ++x) {
            ed.at(x, y, x + y);
            const int dx = (W - 1) - (W - c + x);
            ed.at(W - c + x, y, dx + y);
            const int dy = (H - 1) - (H - c + y);
            ed.at(x, H - c + y, x + dy);
            ed.at(W - c + x, H - c + y, dx + dy);
        }
}

// arr[..., 3] = min(arr[..., 3], clip((max(rgb) - 10.0) / 14.0 * 255.0, 0, 255).astype(uint8))   (double, truncated)
inline void lumaAlpha(Img* im) {
    for (size_t i = 0; i < static_cast<size_t>(im->w) * im->h; ++i) {
        uint8_t* p = &im->px[i * 4];
        const int m = p[0] > p[1] ? (p[0] > p[2] ? p[0] : p[2]) : (p[1] > p[2] ? p[1] : p[2]);
        double a = (m - 10.0) / 14.0 * 255.0;
        a = a < 0.0 ? 0.0 : (a > 255.0 ? 255.0 : a);
        const uint8_t a8 = static_cast<uint8_t>(a);
        if (a8 < p[3]) p[3] = a8;
    }
}

inline bool buildComputer(const Img& src, unsigned W, unsigned H, const std::vector<uint8_t>& crt, Img* res) {
    static const float tint[3] = {0.05f, 0.45f, 0.75f};
    if (!valid(src) || src.w != kPanel || src.h != kPanel) return false;
    Img out;
    if (!makeTintedCrt(crt, W, H, tint, &out)) return false;
    const double s = W / 512.0;
    const unsigned t = static_cast<unsigned>(32 * s);

    Img arr = src;
    // 1. fill the outer steel notch (y 950..1400, x 0..50 and 1998..2048) from steel_patch = rows 600..800 tiled; the dark
    // pixels (max rgb < 25) take all four channels of the tiled steel (the right side uses the horizontally mirrored tile).
    const unsigned kN = 450, kW = 50;
    std::vector<uint8_t> steel(static_cast<size_t>(kN) * kW * 4);
    for (unsigned r = 0; r < kN; ++r) memcpy(&steel[static_cast<size_t>(r) * kW * 4], &arr.px[ix(arr.w, 0, 600 + r % 200)], kW * 4);
    for (int side = 0; side < 2; ++side)
        for (unsigned r = 0; r < kN; ++r)
            for (unsigned c = 0; c < kW; ++c) {
                const unsigned px = side == 0 ? c : kPanel - kW + c;
                uint8_t* p = &arr.px[ix(arr.w, px, 950 + r)];
                if (p[0] < 25 && p[1] < 25 && p[2] < 25) memcpy(p, &steel[(static_cast<size_t>(r) * kW + (side == 0 ? c : kW - 1 - c)) * 4], 4);
            }
    lumaAlpha(&arr);

    // Uniform 130px corner crop in source space; right corners / rail are the FLIP_LEFT_RIGHT of the left ones.
    const unsigned C = 130;
    Img tl, tr, bl, br, topR, botR, leftR, rightR;
    if (!rs(arr, 0, 0, C, C, t, t, &tl) || !flipLR(tl, &tr)) return false;
    if (!rs(arr, 0, kPanel - C, C, kPanel, t, t, &bl) || !flipLR(bl, &br)) return false;
    if (!rs(arr, C, 0, kPanel - C, C, W - 2 * t, t, &topR)) return false;
    if (!rs(arr, C, kPanel - C, kPanel - C, kPanel, W - 2 * t, t, &botR)) return false;
    if (!rs(arr, 0, C, C, kPanel - C, t, H - 2 * t, &leftR) || !flipLR(leftR, &rightR)) return false;
    if (!alphaComposite(&out, tl, 0, 0) || !alphaComposite(&out, tr, W - t, 0)) return false;
    if (!alphaComposite(&out, bl, 0, H - t) || !alphaComposite(&out, br, W - t, H - t)) return false;
    if (!alphaComposite(&out, topR, t, 0) || !alphaComposite(&out, botR, t, H - t)) return false;
    if (!alphaComposite(&out, leftR, 0, t) || !alphaComposite(&out, rightR, W - t, t)) return false;
    chassisCorners(&out, 5);
    *res = out;
    return true;
}

// Corner miter: float32 blend a * m + b * (1 - m), truncated. mode 0 TL (y < x), 1 TR (y < t-1-x), 2 BL (y > t-1-x), 3 BR (y > x).
inline bool miter(const Img& a, const Img& b, unsigned t, int mode, Img* o) {
    if (!valid(a) || !valid(b) || a.w != t || a.h != t || b.w != t || b.h != t) return false;
    Img res;
    makeImg(&res, t, t);
    for (unsigned y = 0; y < t; ++y)
        for (unsigned x = 0; x < t; ++x) {
            const int d = (mode == 0 || mode == 3) ? static_cast<int>(x) : static_cast<int>(t) - 1 - static_cast<int>(x);
            const int yy = static_cast<int>(y);
            float m = 0.0f;
            if (mode < 2) m = yy < d ? 1.0f : (yy == d ? 0.5f : 0.0f);
            else m = yy > d ? 1.0f : (yy == d ? 0.5f : 0.0f);
            for (int q = 0; q < 4; ++q) {
                const float v = static_cast<float>(a.px[ix(t, x, y) + q]) * m + static_cast<float>(b.px[ix(t, x, y) + q]) * (1.0f - m);
                res.px[ix(t, x, y) + q] = static_cast<uint8_t>(v);
            }
        }
    *o = res;
    return true;
}

inline bool buildMitered(const Img& src, unsigned W, unsigned H, const float tint[3], const std::vector<uint8_t>& crt, unsigned t, Img* res) {
    if (!valid(src) || src.w != kPanel || src.h != kPanel) return false;
    Img leftSrc, rvl, rvr, rot, rht, rhb;
    if (!crop(src, 0, 350, 78, 1650, &leftSrc) || !resizeLanczos(leftSrc, t, H, &rvl) || !flipLR(rvl, &rvr)) return false;
    if (!rot270(rvl, &rot) || !resizeLanczos(rot, W, t, &rht)) return false;
    if (!rot90(rvl, &rot) || !resizeLanczos(rot, W, t, &rhb)) return false;
    Img out, c, c2, m;
    makeImg(&out, W, H);
    // centre viewport CRT (extends 6px under the rails)
    if (!makeTintedCrt(crt, W - 2 * (t - 6), H - 2 * (t - 6), tint, &c) || !paste(&out, c, t - 6, t - 6)) return false;
    if (!crop(rht, t, 0, W - t, t, &c) || !paste(&out, c, t, 0)) return false;
    if (!crop(rhb, t, 0, W - t, t, &c) || !paste(&out, c, t, H - t)) return false;
    if (!crop(rvl, 0, t, t, H - t, &c) || !paste(&out, c, 0, t)) return false;
    if (!crop(rvr, 0, t, t, H - t, &c) || !paste(&out, c, W - t, t)) return false;
    // TL: top rail (0,0) + left rail (0,0)
    if (!crop(rht, 0, 0, t, t, &c) || !crop(rvl, 0, 0, t, t, &c2) || !miter(c, c2, t, 0, &m) || !paste(&out, m, 0, 0)) return false;
    // TR: top rail (W-t,0) + right rail (0,0)
    if (!crop(rht, W - t, 0, W, t, &c) || !crop(rvr, 0, 0, t, t, &c2) || !miter(c, c2, t, 1, &m) || !paste(&out, m, W - t, 0)) return false;
    // BL: bottom rail (0,0) + left rail (0,H-t)
    if (!crop(rhb, 0, 0, t, t, &c) || !crop(rvl, 0, H - t, t, H, &c2) || !miter(c, c2, t, 2, &m) || !paste(&out, m, 0, H - t)) return false;
    // BR: bottom rail (W-t,0) + right rail (0,H-t)
    if (!crop(rhb, W - t, 0, W, t, &c) || !crop(rvr, 0, H - t, t, H, &c2) || !miter(c, c2, t, 3, &m) || !paste(&out, m, W - t, H - t)) return false;
    chassisCorners(&out, 5);
    *res = out;
    return true;
}

inline bool buildStore(const Img& src, unsigned W, unsigned H, const std::vector<uint8_t>& crt, Img* res) {
    static const float tint[3] = {0.45f, 0.30f, 0.08f};
    if (!valid(src) || src.w != kPanel || src.h != kPanel) return false;
    Img out;
    if (!makeTintedCrt(crt, W, H, tint, &out)) return false;
    const double s = W / 512.0, sy = H / 512.0;
    const unsigned Cw = static_cast<unsigned>(72 * s), ChTop = static_cast<unsigned>(60 * sy), ChBot = static_cast<unsigned>(52 * sy), Rw = static_cast<unsigned>(32 * s);
    Img tex = src;
    lumaAlpha(&tex);
    const unsigned CwS = 280, ChTs = 236, ChBs = 185, RwS = 125;
    const unsigned P = kPanel;
    Img tl, tr, bl, br, topR, botR, leftR, rightR;
    if (!rs(tex, 0, 0, CwS, ChTs, Cw, ChTop, &tl) || !rs(tex, P - CwS, 0, P, ChTs, Cw, ChTop, &tr)) return false;
    if (!rs(tex, 0, P - ChBs, CwS, P, Cw, ChBot, &bl) || !rs(tex, P - CwS, P - ChBs, P, P, Cw, ChBot, &br)) return false;
    if (!rs(tex, CwS, 0, P - CwS, ChTs, W - 2 * Cw, ChTop, &topR)) return false;
    if (!rs(tex, CwS, P - ChBs, P - CwS, P, W - 2 * Cw, ChBot, &botR)) return false;
    if (!rs(tex, 0, ChTs, RwS, P - ChBs, Rw, H - ChTop - ChBot, &leftR)) return false;
    if (!rs(tex, P - RwS, ChTs, P, P - ChBs, Rw, H - ChTop - ChBot, &rightR)) return false;
    if (!alphaComposite(&out, tl, 0, 0) || !alphaComposite(&out, tr, W - Cw, 0)) return false;
    if (!alphaComposite(&out, bl, 0, H - ChBot) || !alphaComposite(&out, br, W - Cw, H - ChBot)) return false;
    if (!alphaComposite(&out, topR, Cw, 0) || !alphaComposite(&out, botR, Cw, H - ChBot)) return false;
    if (!alphaComposite(&out, leftR, 0, ChTop) || !alphaComposite(&out, rightR, W - Rw, ChTop)) return false;
    chassisCorners(&out, 5);
    *res = out;
    return true;
}

inline bool buildPauseGold(const Img& src, unsigned W, unsigned H, const std::vector<uint8_t>& crt, Img* res) {
    static const float tint[3] = {0.50f, 0.40f, 0.12f};
    if (!valid(src) || src.w != kPanel || src.h != kPanel) return false;
    Img out;
    if (!makeTintedCrt(crt, W, H, tint, &out)) return false;
    const double s = W / 512.0, sy = H / 512.0;
    const unsigned Cw = static_cast<unsigned>(72 * s), ChTop = static_cast<unsigned>(48 * sy), ChBot = static_cast<unsigned>(48 * sy), Rw = static_cast<unsigned>(32 * s);
    Img tex = src;
    lumaAlpha(&tex);
    const unsigned CwS = 280, ChTs = 180, ChBs = 180, RwS = 115;
    const unsigned P = kPanel;
    Img tl, tr, bl, br, topR, botR;
    if (!rs(tex, 0, 0, CwS, ChTs, Cw, ChTop, &tl) || !rs(tex, P - CwS, 0, P, ChTs, Cw, ChTop, &tr)) return false;
    if (!rs(tex, 0, P - ChBs, CwS, P, Cw, ChBot, &bl) || !rs(tex, P - CwS, P - ChBs, P, P, Cw, ChBot, &br)) return false;
    if (!rs(tex, CwS, 0, P - CwS, ChTs, W - 2 * Cw, ChTop, &topR)) return false;
    if (!rs(tex, CwS, P - ChBs, P - CwS, P, W - 2 * Cw, ChBot, &botR)) return false;

    // side rails with 1:1 circular bosses
    const unsigned yTop = ChTop, yBot = H - ChBot;
    const unsigned bossH = static_cast<unsigned>(24 * sy);
    const unsigned yBoss = yBot - bossH - static_cast<unsigned>(10 * sy);
    if (yBoss <= yTop || yBot <= yBoss + bossH) return false;
    Img ribbed, boss, spacer, f;
    if (!rs(tex, 0, 500, RwS, 1200, Rw, yBoss - yTop, &ribbed)) return false;
    if (!rs(tex, 0, 1340, RwS, 1530, Rw, bossH, &boss)) return false;
    if (!rs(tex, 0, 1530, RwS, 1600, Rw, yBot - (yBoss + bossH), &spacer)) return false;

    if (!alphaComposite(&out, tl, 0, 0) || !alphaComposite(&out, tr, W - Cw, 0)) return false;
    if (!alphaComposite(&out, bl, 0, H - ChBot) || !alphaComposite(&out, br, W - Cw, H - ChBot)) return false;
    if (!alphaComposite(&out, topR, Cw, 0) || !alphaComposite(&out, botR, Cw, H - ChBot)) return false;
    if (!alphaComposite(&out, ribbed, 0, yTop) || !flipLR(ribbed, &f) || !alphaComposite(&out, f, W - Rw, yTop)) return false;
    if (!alphaComposite(&out, boss, 0, yBoss) || !flipLR(boss, &f) || !alphaComposite(&out, f, W - Rw, yBoss)) return false;
    if (!alphaComposite(&out, spacer, 0, yBoss + bossH) || !flipLR(spacer, &f) || !alphaComposite(&out, f, W - Rw, yBoss + bossH)) return false;
    chassisCorners(&out, 5);
    *res = out;
    return true;
}

// Bring a source to the size the generator's coordinates assume. Upscaling a 1024 px vanilla panel to 2048 first gives the
// vanilla-only path the same geometry as the 2048 px user-override panels.
inline bool normalise(const Img& in, unsigned size, Img* out) {
    if (!valid(in)) return false;
    if (in.w == size && in.h == size) { *out = in; return true; }
    return resizeLanczos(in, size, size, out);
}

}  // namespace detail

// Streaming form (peak memory: one 2048 px panel at a time, not six). prepareDatapad() once with pnl_datapad, then
// composeOne() per key with that key's own panel; the caller frees each panel after its bezel is composed.
struct Datapad {
    qlimg::Image dp;            // pnl_datapad at 512x512 (the KeyDatapad bezel itself)
    std::vector<uint8_t> crt;   // grayscale CRT backing derived from it
};

inline bool prepareDatapad(const qlimg::Image& datapad, Datapad* d) {
    using namespace detail;
    if (!normalise(datapad, kOutSize, &d->dp)) return false;
    return crtGray(d->dp, &d->crt);
}

// One 512x512 bezel. For KeyDatapad `panel` is ignored. False if the panel is empty or a crop falls outside it.
inline bool composeOne(Key k, const Datapad& d, const qlimg::Image& panel, qlimg::Image* out) {
    using namespace detail;
    static const float kUpg[3] = {0.04f, 0.35f, 0.55f};
    static const float kWb[3] = {0.04f, 0.40f, 0.30f};
    if (k == KeyDatapad) { *out = d.dp; return true; }
    if (k >= KeyCount) return false;
    Img scaled;
    const Img* p = &panel;
    if (valid(panel) && (panel.w != kPanel || panel.h != kPanel)) {   // 1024 px vanilla panels: see normalise()
        if (!normalise(panel, kPanel, &scaled)) return false;
        p = &scaled;
    }
    if (!valid(*p)) return false;
    Img res;
    bool ok = false;
    switch (k) {
        case KeyComputer: ok = buildComputer(*p, kOutSize, kOutSize, d.crt, &res); break;
        case KeyUpgrade: ok = buildMitered(*p, kOutSize, kOutSize, kUpg, d.crt, 32, &res); break;
        case KeyWorkbench: ok = buildMitered(*p, kOutSize, kOutSize, kWb, d.crt, 32, &res); break;
        case KeyStore: ok = buildStore(*p, kOutSize, kOutSize, d.crt, &res); break;
        case KeyPauseGold: ok = buildPauseGold(*p, kOutSize, kOutSize, d.crt, &res); break;
        default: break;
    }
    if (!ok) return false;
    *out = res;
    return true;
}

// The six 512x512 bezels, in Key order. False if any source is empty or a crop falls outside a panel.
inline bool composeAll(const Sources& src, qlimg::Image out[KeyCount]) {
    Datapad d;
    if (!prepareDatapad(src.pnl[KeyDatapad], &d)) return false;
    qlimg::Image res[KeyCount];
    for (int k = 0; k < KeyCount; ++k)
        if (!composeOne(static_cast<Key>(k), d, src.pnl[k], &res[k])) return false;
    for (int k = 0; k < KeyCount; ++k) out[k] = res[k];
    return true;
}

// Uncompressed 32-bit true-colour TGA, bottom-left origin, 8 alpha bits (descriptor 0x08), BGRA, bottom row first, no footer.
inline void encodeTga32(const qlimg::Image& img, std::vector<uint8_t>* out) {
    out->clear();
    if (!detail::valid(img) || img.w > 0xFFFFu || img.h > 0xFFFFu) return;
    out->reserve(18 + static_cast<size_t>(img.w) * img.h * 4);
    uint8_t h[18] = {0};
    h[2] = 2;
    h[12] = static_cast<uint8_t>(img.w);
    h[13] = static_cast<uint8_t>(img.w >> 8);
    h[14] = static_cast<uint8_t>(img.h);
    h[15] = static_cast<uint8_t>(img.h >> 8);
    h[16] = 32;
    h[17] = 0x08;
    out->insert(out->end(), h, h + 18);
    for (unsigned y = img.h; y-- > 0;)
        for (unsigned x = 0; x < img.w; ++x) {
            const uint8_t* p = &img.px[detail::ix(img.w, x, y)];
            out->push_back(p[2]);
            out->push_back(p[1]);
            out->push_back(p[0]);
            out->push_back(p[3]);
        }
}

}  // namespace qlbzc
