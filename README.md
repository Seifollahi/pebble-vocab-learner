# Vocab Learner for Pebble

A minimalist, highly effective vocabulary learning application built natively for the Pebble ecosystem, optimized for the Pebble Time 2 (Emery) and backwards-compatible with all models.

## Features
- **Ultra-Minimalist UI**: Designed with a pure OLED-friendly black background. Distractions are removed so 55% of the screen is dedicated entirely to the definition, ensuring maximum readability without squinting.
- **Spaced Repetition System (SRS)**: Employs a true Leitner memory engine. The app learns what words you struggle with and spaces them out dynamically (from 10 minutes up to 3 days), ensuring optimal active recall.
- **Offline First**: Entirely self-contained C implementation. The vocabulary database is baked natively into the `.pbw` binary, using virtually no heap memory and working perfectly without your phone.
- **Background Wakeups**: Utilizes the Pebble Wakeup API to silently trigger a notification to remind you to review words.
- **Highly Configurable**: Control notification frequency and toggle haptic vibrations directly from the Pebble companion app.

## Publishing to Rebble App Store
This application is fully formatted and ready for the Rebble developer portal.
1. Download the generated `build/pebble.pbw` file.
2. Log into the [Rebble Developer Portal](https://dev-portal.rebble.io/).
3. Create a new App, name it **Vocab Learner**, and upload the `.pbw` file. The portal will automatically detect the UUID `445cd633-85d3-49ff-a7f3-b31cb61ec47f`.
4. Upload screenshots and the provided `menu_icon.png`.

## Build Instructions (For Developers)
To compile from source, you must use the Rebble Docker SDK:
```bash
docker run --rm -v $PWD:/pebble rebble/pebble-sdk:latest /bin/bash -c "cd /pebble && pebble build"
```
