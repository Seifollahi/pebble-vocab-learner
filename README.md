# Vocab Learner for Pebble

A minimalist, highly effective vocabulary learning application built natively for the Pebble ecosystem, optimized for the Pebble Time 2 (Emery) and backwards-compatible with all models.

[**Download Vocab Learner on the Rebble App Store!**](https://apps.rebble.io/en_US/application/6a38d2e88ba8e5000adbc6be)

## Features
- **Ultra-Minimalist UI**: Pure OLED-friendly black background with most of the screen dedicated to the definition. Button hints teach you the controls for your first 10 launches, then vanish. Tiny progress dots show each word's Leitner level and which view (meaning / example / origin / phonetic) you're on.
- **True Spaced Repetition (SRS)**: A two-way Leitner memory engine. Hold `SELECT` when you know a word to space it out (10 minutes up to 3 days); hold `DOWN` when you've forgotten it to bring it back immediately. Skipping only draws from words that are actually due, and when nothing is due you get a satisfying "All caught up" screen with your next review time.
- **234-Word Database**: Curated words across 8 categories and 3 difficulty levels, baked natively into the `.pbw` binary. `resources/vocab_db.json` is the single source of truth — `src/c/vocab.h` is generated at build time.
- **Offline First**: Entirely self-contained C implementation. No internet or phone connection needed to review.
- **Battery Friendly**: Wakes the CPU once per minute, not once per second.
- **Timeline Notifications**: Deduplicated timeline pins (stable slot IDs) pushed only when you save settings, and cleaned up when you disable them.
- **Highly Configurable**: Notification frequency, difficulty filter (Basic / Intermediate / Advanced), and haptics — all from the companion app, which remembers your saved settings.

## Button Map
| Button | Action |
|---|---|
| `SELECT` | Reveal / cycle meaning → example → origin → phonetic |
| `HOLD SELECT` | I know this word (promote bucket) |
| `HOLD DOWN` | I forgot this word (reset bucket) |
| `UP` / `DOWN` | Show another due word |
| `HOLD UP` | Learning stats |

## Build Instructions (For Developers)
The vocabulary header is generated from JSON before compiling:
```bash
python3 tools/generate_vocab.py   # regenerates src/c/vocab.h
```
Then compile with the Rebble Docker SDK:
```bash
docker run --rm -v $PWD:/pebble rebble/pebble-sdk:latest /bin/bash -c "cd /pebble && python3 tools/generate_vocab.py && pebble build"
```
Or use the helper script (runs both steps): `./build.sh`

### Adding words
Edit `resources/vocab_db.json` (fields: `t` term, `m` meaning, `x` example, `e` etymology, `p` phonetic, `d` difficulty 1–3, `c` category) and rebuild. The persistence layer is versioned and chunked, so growing the list does not corrupt existing users' progress.
