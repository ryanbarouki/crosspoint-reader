#pragma once
#include <Utf8.h>

#include <cstdint>
#include <vector>

/// Returns true if the Unicode codepoint falls within the dict font's coverage —
/// phonetic, combining, Greek, and symbol ranges used in dictionary definitions.
/// Ranges covered:
///   U+0220–U+02FF  Latin Extended-B / IPA Extensions / Spacing Modifier Letters
///   U+1D00–U+1DBF  Phonetic Extensions / Supplement
///   U+0370–U+03FF  Greek and Coptic (etymology / definitions)
///   U+1E00–U+1FFF  Latin Extended Additional / Greek Extended
/// Combining marks are attached to the previous run by splitDictRuns().
static inline bool isDictCodepoint(uint32_t cp) {
  if (cp < 0x0220) return false;
  return (cp <= 0x02FF) || (cp >= 0x1D00 && cp <= 0x1DBF) || cp == 0x221A || cp == 0x2192 || cp == 0x261E ||
         (cp >= 0x2153 && cp <= 0x2154) || (cp >= 0x266D && cp <= 0x266F) || (cp >= 0x0370 && cp <= 0x03FF) ||
         (cp >= 0x1E00 && cp <= 0x1FFF);
}

struct DictTextSpan {
  bool isDictFont;
  uint32_t start = 0;
  uint16_t len = 0;
};

/// Split a UTF-8 string into runs of dict-font vs body-font codepoints.
/// Results are appended into `out`; caller must clear `out` before each call.
static inline void splitDictRuns(const char* text, std::vector<DictTextSpan>& out) {
  if (!text || !text[0]) return;
  uint16_t currentLen = 0;
  bool currentIsDictFont = false;
  bool first = true;
  const auto* p = reinterpret_cast<const uint8_t*>(text);
  uint32_t cp;
  uint32_t start = 0;
  while ((cp = utf8NextCodepoint(&p))) {
    const bool combining = utf8IsCombiningMark(cp);
    const bool isDict = combining ? currentIsDictFont : isDictCodepoint(cp);
    if (!first && !combining && isDict != currentIsDictFont) {
      uint16_t len = currentLen;
      out.push_back({currentIsDictFont, start, len});
      currentLen = 0;
      start += len;
    }
    currentIsDictFont = isDict;
    first = false;
    currentLen += utf8CodepointNumBytes(cp);
  }
  if (currentLen != 0) out.push_back({currentIsDictFont, start, currentLen});
}
