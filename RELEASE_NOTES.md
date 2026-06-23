# Release Notes: v1.1.0

Welcome to the newest release of **Vocab Learner for Pebble**! 

This update brings more control over your learning experience and significantly improves how background notifications work.

## What's New in v1.1.0

### ⏰ True Background Notifications (Timeline Pins)
- We have completely rewritten the background reminder system. The app now pushes **Pebble Timeline Pins** directly to your watch instead of using the local Wakeup API. 
- You will now receive proper popup notifications reminding you to review your vocabulary. You can simply dismiss the notification to review it later, or select "Open App" to jump right into a review session!
- *Note: You must enable "Timeline" permissions for this app when installing.*

### 🎚️ Difficulty Levels Configuration
- You can now filter which words you want to study directly from the Companion App Settings! Choose to study Basic, Intermediate, Advanced words, or any combination of the three.

## Core Features
- **Leitner Spaced Repetition System (SRS)**: The app uses a built-in persistent memory engine to track your progress. Long-press `SELECT` when you know a word to advance its bucket, and the app will automatically schedule it for review in 10 minutes, 4 hours, or up to 3 days later!
- **Pure Minimalist Design**: The UI has been stripped of all distracting elements. With a deep black `GColorBlack` background and massive `24px` bold fonts, 55% of the screen is dedicated entirely to the definition.
- **Offline First**: The entire vocabulary database is baked natively into the lightweight C application. It requires absolutely zero internet connection or phone tethering to review your words.
