# Vocab Learner for Pebble

A minimalist, highly effective vocabulary learning application built natively for the Pebble ecosystem, optimized for the Pebble Time 2 (Emery) and backwards-compatible with all models.

[**Download Vocab Learner on the Rebble App Store!**](https://apps.rebble.io/en_US/application/6a38d2e88ba8e5000adbc6be)

## Features
- **Ultra-Minimalist UI**: Designed with a pure OLED-friendly black background. Distractions are removed so 55% of the screen is dedicated entirely to the definition, ensuring maximum readability without squinting.
- **Spaced Repetition System (SRS)**: Employs a true Leitner memory engine. The app learns what words you struggle with and spaces them out dynamically (from 10 minutes up to 3 days), ensuring optimal active recall.
- **Offline First**: Entirely self-contained C implementation. The vocabulary database is baked natively into the `.pbw` binary, using virtually no heap memory and working perfectly without your phone.
- **Background Wakeups**: Utilizes the Pebble Wakeup API to silently trigger a notification to remind you to review words.
- **Highly Configurable**: Control notification frequency and toggle haptic vibrations directly from the Pebble companion app.


## Build Instructions (For Developers)
To compile from source, you must use the Rebble Docker SDK:
```bash
docker run --rm -v $PWD:/pebble rebble/pebble-sdk:latest /bin/bash -c "cd /pebble && pebble build"
```
