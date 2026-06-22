# Contributing to Vocab Learner

First off, thank you for considering contributing to Vocab Learner! It's people like you that make the open source community such a great place to learn, inspire, and create.

## Getting Started

This application is built natively for the Pebble Smartwatch platform using C and PebbleKit JS.

### Prerequisites
Since the official Pebble SDK is no longer actively maintained by Pebble, we use the community-maintained Rebble Docker image to compile the application reliably across platforms.

1. Ensure you have [Docker](https://docs.docker.com/get-docker/) installed and running.
2. Clone this repository:
   ```bash
   git clone https://github.com/Seifollahi/pebble-vocab-learner.git
   cd pebble-vocab-learner
   ```

### Building the Project
To compile the `.pbw` binary for all supported platforms (Aplite, Basalt, Chalk, Diorite, Emery), run the following Docker command from the root of the project:

```bash
docker run --rm -v $PWD:/pebble rebble/pebble-sdk:latest /bin/bash -c "cd /pebble && pebble build"
```
The compiled application will be generated at `build/vocab_learner.pbw`.

## Project Structure
- `src/c/pebble_vocab_learner.c` - The main application entry point and message router.
- `src/c/state.c` - The core logic for the Leitner Spaced Repetition System (SRS) and persistent storage.
- `src/c/ui.c` - The minimalist OLED-optimized graphics routines.
- `src/pkjs/index.js` - The PebbleKit JS companion code for configuration and communication.
- `resources/vocab_db.json` - The static offline vocabulary database.

## Making Changes
1. **Fork** the repository and create your branch from `main`.
2. Ensure your C code follows standard formatting without excessive heap allocations (Pebble devices are memory constrained!).
3. Build and test your changes.
4. Issue a Pull Request with a clear description of the modifications.

## License
By contributing to Vocab Learner, you agree that your contributions will be licensed under its MIT License.
