# Release Notes: v1.0.0

Welcome to the initial release of **Vocab Learner for Pebble**! 

This application was built natively for the Pebble smartwatch ecosystem, with specific optimizations for the Pebble Time 2 (Emery), to bring language mastery directly to your wrist. 

## What's Included in v1.0.0

### 🚀 Core Features
- **Leitner Spaced Repetition System (SRS)**: The app uses a built-in persistent memory engine to track your progress. Long-press `SELECT` when you know a word to advance its bucket, and the app will automatically schedule it for review in 10 minutes, 4 hours, or up to 3 days later!
- **Pure Minimalist Design**: The UI has been stripped of all distracting elements. With a deep black `GColorBlack` background and massive `24px` bold fonts, 55% of the screen is dedicated entirely to the definition, ensuring maximum readability at a glance.
- **Offline First**: The entire vocabulary database is baked natively into the lightweight C application. It requires absolutely zero internet connection or phone tethering to review your words.
- **Background Wakeups**: Utilizes the Pebble Wakeup API to quietly remind you when it's time for your next learning session.

### ⚙️ Companion App Settings
- Customize how often you receive background notifications (every 1, 4, or 24 hours).
- A brand new toggle to completely enable or disable haptic vibrations for silent studying.

We hope you enjoy building your vocabulary seamlessly throughout the day. Happy learning!
