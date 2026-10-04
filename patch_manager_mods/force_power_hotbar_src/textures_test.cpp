// Embedded texture table: every entry is a valid uncompressed 32-bit TGA of the expected size; names unique.
#include "HotbarLogic.h"
#include "HotbarTextures.inc"
#include "check.h"
#include <cstring>

struct Expect { const char* name; int w, h; };
static const Expect kExpected[] = {
    { "fph_slot", 128, 128 }, { "fph_slot_hi", 128, 128 }, { "fph_edit", 72, 128 }, { "fph_edit_hi", 72, 128 },
    { "fph_tgt_lowhp", 32, 32 }, { "fph_mode", 72, 128 }, { "fph_mode_hi", 72, 128 },
};

int main() {
  const int n = (int)(sizeof(EmbeddedTextures) / sizeof(EmbeddedTextures[0]));
  const int ne = (int)(sizeof(kExpected) / sizeof(kExpected[0]));
  for (int i = 0; i < n; i++) {
    const EmbeddedTexture& t = EmbeddedTextures[i];
    hb::TgaInfo info;
    KT_CHECK_EQ((int)hb::tgaInspect(t.data, t.size, &info), (int)hb::TGA_OK);
    for (int j = i + 1; j < n; j++) KT_CHECK(std::strcmp(t.resref, EmbeddedTextures[j].resref) != 0);
    for (int k = 0; k < ne; k++)
      if (!std::strcmp(t.resref, kExpected[k].name)) {
        KT_CHECK_EQ(info.w, kExpected[k].w);
        KT_CHECK_EQ(info.h, kExpected[k].h);
      }
  }
  for (int k = 0; k < ne; k++) {
    bool found = false;
    for (int i = 0; i < n; i++) found = found || !std::strcmp(EmbeddedTextures[i].resref, kExpected[k].name);
    KT_CHECK(found);
    if (!found) std::fprintf(stderr, "missing texture %s\n", kExpected[k].name);
  }
  KT_CHECK_EQ(n, ne);
  return kt::Report("textures_test");
}
