#ifndef VOCAB_H
#define VOCAB_H

typedef struct {
  const char *term;
  const char *meaning;
  const char *example;
  const char *etymology;
  const char *phonetic;
  int difficulty;  // 1=beginner, 2=intermediate, 3=advanced
  const char *category;
} VocabEntry;

static const VocabEntry vocab_list[] = {
  {"Eloquent", "Fluent and persuasive in speaking", "The speaker was eloquent and captivating.", "From Latin: eloquens", "EL-uh-kwunt", 2, "General"},
  {"Ephemeral", "Lasting for a very short time", "The beauty of cherry blossoms is ephemeral.", "From Greek: ephemeros", "uh-FEM-er-ul", 3, "General"},
  {"Ubiquitous", "Present everywhere at once", "Smartphones are ubiquitous in modern society.", "From Latin: ubique", "yoo-BIK-wi-tus", 3, "General"},
  {"Serendipity", "Finding good things by chance", "Meeting her was pure serendipity.", "From Persian: Serendip", "ser-en-dip-i-tee", 2, "General"},
  {"Pragmatic", "Dealing with things in a practical way", "Take a pragmatic approach to problem-solving.", "From Greek: pragmatikos", "prag-MAT-ik", 2, "General"},
  {"Mellifluous", "Sweet or musical sounding", "Her mellifluous voice enchanted the audience.", "From Latin: mel + fluere", "muh-LIF-loo-us", 3, "General"},
  {"Obfuscate", "To make something unclear", "Don't obfuscate the issue with vague language.", "From Latin: obfuscare", "ob-FUS-kayt", 3, "Advanced"},
  {"Paradigm", "A typical example or pattern", "This discovery changed the scientific paradigm.", "From Greek: paradeigma", "PAIR-uh-dym", 2, "Academic"},
  {"Zenith", "The highest point or peak", "His career reached its zenith at age 40.", "From Arabic: samt", "ZEE-nith", 1, "General"},
  {"Nostalgic", "Sentimental about the past", "The old song made her feel nostalgic.", "From Greek: nostos + algos", "nos-TAL-jik", 1, "General"},
  {"Resilient", "Able to recover quickly", "The community proved resilient after the disaster.", "From Latin: resilire", "ruh-ZIL-yunt", 2, "General"},
  {"Meticulous", "Very careful and precise", "She is meticulous in her attention to detail.", "From Latin: meticulosus", "muh-TIK-yuh-lus", 2, "General"},
  {"Ambiguous", "Open to more than one meaning", "His statement was ambiguous and confusing.", "From Latin: ambiguus", "am-BIG-yuh-us", 2, "General"},
  {"Petulant", "Childishly sulky or bad-tempered", "The child's petulant behavior annoyed everyone.", "From Latin: petulans", "PET-yuh-lunt", 2, "General"},
  {"Eloquence", "Fluent and persuasive speech", "The eloquence of her argument won the debate.", "From Latin: eloquens", "EL-uh-kwents", 2, "General"},
  {"Fastidious", "Very particular about details", "He is fastidious about cleanliness.", "From Latin: fastidiosus", "fa-STID-ee-us", 2, "General"},
  {"Gregarious", "Fond of being in crowds", "Humans are naturally gregarious creatures.", "From Latin: gregarius", "gruh-GAIR-ee-us", 2, "General"},
  {"Hiatus", "A pause or gap in continuity", "The company took a hiatus from production.", "From Latin: hiatus", "hy-AY-tus", 2, "General"},
  {"Incisive", "Intelligently keen and analytical", "The critic's incisive review was spot-on.", "From Latin: incidere", "in-SY-siv", 2, "General"},
  {"Jubilant", "Joyful and triumphant", "The crowd was jubilant after winning the game.", "From Latin: jubilans", "JOO-buh-lunt", 1, "General"},
};

#define VOCAB_COUNT (sizeof(vocab_list) / sizeof(vocab_list[0]))

#endif
