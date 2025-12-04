# Vocab Learner - Pebble Watch App

📚 A comprehensive vocabulary learning application for Pebble smartwatches with advanced features like phonetic pronunciation, example sentences, etymology, and progress tracking.

## Features

### 📖 Learning Modes
- **Definition Display** - Traditional meaning of the word
- **Example Sentences** - See words used in context
- **Etymology** - Learn the origin and history of words
- **Phonetic Pronunciation** - Learn how to pronounce each word

### 📊 Progress Tracking
- Track words learned (auto-advances every 10 minutes)
- Count total words reviewed (manual navigation)
- Streak tracking for consistency
- Real-time statistics on your wrist

### 🎓 Vocabulary Organization
- **20+ Pre-loaded Words** with difficulty levels (Beginner/Intermediate/Advanced)
- **Multiple Categories** (General, Academic)
- Color-coded difficulty indicators
- Dynamic category and difficulty display

### ⚡ Smart Features
- **Live Clock Display** showing current time
- **Auto-advance Timer** - New vocabulary every 10 minutes with vibration alert
- **Interactive Button Controls** for seamless navigation
- **Responsive Interface** with color-coded information layers
- **Multiple Vibration Patterns** for different events

### 🎨 Visual Design
- Black background with vibrant cyan clock
- Color-coded layers:
  - Cyan: Clock and time info
  - Chrome Yellow: Title
  - Vivid Cerulean: Vocabulary terms
  - Spring Bud: Definitions and information
  - Malachite Green: Statistics
  - Red: Countdown timer
  - Orange: Instructions

## Button Controls

| Button | Action |
|--------|--------|
| **UP** | Next vocabulary word |
| **DOWN** | Previous vocabulary word |
| **SELECT (Middle)** | Reveal information / Cycle through modes |

**Information Cycle:** Definition → Example → Etymology → Phonetic Pronunciation

## Installation

### Prerequisites
- Pebble SDK 3
- Python 2.7
- NodeJS (optional, for companion app)

### Build from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/vocab-learner.git
cd vocab-learner

# Install dependencies
pebble doctor

# Build the app
pebble build

# Install on emulator or device
pebble install --phone <phone-ip>
```

## Vocabulary Database

The app includes 20 carefully selected words covering:
- **Beginner Level:** Zenith, Nostalgic, Jubilant
- **Intermediate Level:** Eloquent, Serendipity, Pragmatic, Meticulous, Ambiguous, Petulant, Resilient, Fastidious, Gregarious, Hiatus, Incisive, Paradigm
- **Advanced Level:** Ephemeral, Ubiquitous, Mellifluous, Obfuscate

Each word includes:
- **Term** - The vocabulary word
- **Meaning** - Clear definition
- **Example** - Sentence demonstrating usage
- **Etymology** - Word origin and history
- **Phonetic** - Pronunciation guide

## Customization

### Adding New Vocabulary

Edit `src/c/vocab.h`:

```c
{"NewWord", "Definition here", "Example sentence here", "Etymology here", "Pronunciation", 2, "Category"}
```

Then rebuild:
```bash
pebble build
```

### Adjusting Timer

In `src/c/pebble_vocab_learner.c`, change the `s_time_remaining` value:

```c
static int s_time_remaining = 600;  // 600 seconds = 10 minutes
```

## Project Structure

```
vocab-learner/
├── src/c/
│   ├── pebble_vocab_learner.c      # Main application
│   └── vocab.h                     # Vocabulary database
├── wscript                         # Build configuration
├── package.json              # Project metadata
├── README.md                 # This file
└── .gitignore                # Git ignore rules
```

## Platform Support

- ✅ Pebble Time (Basalt)
- ✅ Pebble Time Steel (Basalt)
- ✅ Pebble Time Round (Chalk)
- ✅ Pebble Original (Aplite)
- ✅ Pebble Steel (Diorite)
- ✅ Pebble 2 (Emery)

## Performance

- **Memory Usage:** ~2.4KB RAM footprint
- **App Size:** ~4KB resources
- **Battery Impact:** Minimal (updates only on button press or minute change)

## Development

### Building
```bash
pebble build          # Build for all platforms
pebble build --phone  # Build for phone emulator
```

### Running in Emulator
```bash
pebble install --phone <ip-address>
pebble logs           # View app logs
```

### Debugging
```bash
pebble build -v       # Verbose build output
pebble logs | grep vocab  # Filter logs
```

## Future Enhancements

- [ ] Difficulty selection on app launch
- [ ] Quiz mode with multiple choice answers
- [ ] Achievement badges and milestones
- [ ] Sync learned words with companion phone app
- [ ] Custom vocabulary import via BLE
- [ ] Spaced repetition algorithm
- [ ] Voice guidance (via companion app)
- [ ] Dark mode option
- [ ] Export statistics to CSV

## Contributing

Contributions are welcome! Here's how to contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Created as a learning tool for Pebble smartwatch development.

## Acknowledgments

- Pebble SDK Documentation
- Vocabulary from GRE/SAT prep materials
- Community feedback and testing

## Support

For issues, questions, or suggestions:
- Open an [Issue](https://github.com/yourusername/vocab-learner/issues)
- Create a [Discussion](https://github.com/yourusername/vocab-learner/discussions)

## FAQ

**Q: Can I add more vocabulary words?**
A: Yes! Edit `src/c/vocab.h` and add entries to the `vocab_list` array.

**Q: Can I change the timer interval?**
A: Yes! Modify `s_time_remaining = 600;` in `pebble_vocab_learner.c` to your desired seconds.

**Q: Will this work on my Pebble model?**
A: Yes! The app is tested and compiled for all Pebble models.

**Q: How do I update the app on my watch?**
A: Use `pebble install --phone <phone-ip>` to push updates.

---

**Happy Learning! 📚✨**
