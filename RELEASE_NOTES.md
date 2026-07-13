# Release Notes: v2.1.0

This release removes the word-count ceiling. The vocabulary database moved out of the app binary (which is copied into the watch's limited RAM) and into the 256KB resource bank, read on demand — one word in memory at a time.

## What's New in v2.1.0

### 📦 Resource-Packed Vocabulary Database
- Words are now packed into a binary resource (`vocab_db.bin`) generated from `resources/vocab_db.json` and read with `resource_load_byte_range()`. App RAM usage no longer grows with the word list.
- **New capacity: ~1,800 words** on Pebble Time/2/Round (up from a few hundred), **400 words** on the original Pebble (up from 50). Growing the list is now just editing the JSON.
- SRS progress is stored in a packed 2-bytes-per-word format (schema v3) so even a 1,800-word deck fits Pebble's 4KB persistent-storage quota. Existing v2.0 (and v1.x) progress migrates automatically.

### Under the Hood
- New `src/c/vocab.c` runtime accessor (offset index + difficulty table in a few KB of heap; single 512-byte record buffer).
- `tools/generate_vocab.py` now emits the binary blob (plus a trimmed `~aplite` variant automatically when the list outgrows the original Pebble).
- Word content is unchanged in this release (234 words) — this is the scalability release; big word-pack drops can now follow without any code changes.

---

# Release Notes: v2.0.0

The biggest update yet: the spaced repetition engine now works both ways, the word database grew more than 10x, and the whole app got a battery and UX overhaul.

## What's New in v2.0.0

### 🧠 The SRS Finally Learns What You Struggle With
- **New grading action — HOLD `DOWN` = "I forgot"**: resets that word's Leitner bucket so it comes back for review immediately. Previously the engine could only promote words, never demote them; now it truly adapts to you.
- **Skipping respects your schedule**: `UP`/`DOWN` now show another random *due* word instead of walking the raw list, and always honor your difficulty filter.
- **"All caught up" screen**: when nothing is due, the app tells you — plus when your next word arrives — instead of force-feeding words ahead of schedule.

### 📚 234 Words (Up From 20)
- The vocabulary database grew to 234 curated words across 8 categories (General, Academic, Business, Science, Law, Arts, Nature, Emotions) and 3 difficulty levels.
- `resources/vocab_db.json` is now the single source of truth; `vocab.h` is generated at build time by `tools/generate_vocab.py`.
- Your existing progress on the original 20 words migrates automatically.

### 🔋 Battery & Correctness
- The app now wakes once per **minute** instead of once per second — a meaningful battery win on a wearable.
- **Timeline pins no longer duplicate**: pins use stable slot IDs that overwrite on refresh, are only pushed when you save settings, and are deleted when you disable notifications.
- SRS progress is stored in chunked, versioned records — safe past Pebble's 256-byte persist limit, with room to grow the word list further.
- Lifetime stats (Mastered / Reviewed / Graduated) now persist across launches.
- The settings page now shows your *actual* saved settings instead of defaults.

### ✨ Minimalist UI Upgrades
- **Leitner progress dots**: five tiny dots under the term show how close a word is to graduating.
- **Mode dots**: four dots at the bottom edge show which view you're on (meaning / example / origin / phonetic).
- **Grading flash**: a brief screen flash confirms a "mastered" grade registered.
- **Hints retire themselves**: button hints show for your first 10 launches, then disappear — pure black thereafter.
- Long definitions auto-shrink to fit; round (Chalk) displays get centered, inset text.

## Button Map
- `SELECT`: reveal / cycle meaning → example → origin → phonetic
- `HOLD SELECT`: I know this word (promote)
- `HOLD DOWN`: I forgot this word (reset)
- `UP` / `DOWN`: show another due word
- `HOLD UP`: learning stats
