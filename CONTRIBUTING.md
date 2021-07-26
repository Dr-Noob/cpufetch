# cpufetch contributing guidelines

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [1. cpufetch does not accept pull requests](#1-cpufetch-does-not-accept-pull-requests)
- [2. Creating an issue](#2-creating-an-issue)
  - [2.1: I found a bug in cpufetch (the program provides incorrect / invalid information)](#21-i-found-a-bug-in-cpufetch-the-program-provides-incorrect--invalid-information)
  - [2.2: I found a bug in cpufetch (the program crashes / does not work properly)](#22-i-found-a-bug-in-cpufetch-the-program-crashes--does-not-work-properly)
    - [Stacktrace option 1 (best)](#stacktrace-option-1-best)
    - [Stacktrace option 2 (use this option if option 1 does not work)](#stacktrace-option-2-use-this-option-if-option-1-does-not-work)
  - [2.3: I have an idea for a new feature in cpufetch / I want to suggest a change in cpufetch](#23-i-have-an-idea-for-a-new-feature-in-cpufetch--i-want-to-suggest-a-change-in-cpufetch)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->
Thanks for your interest in contributing to cpufetch! Please, read this page carefully to understand how to contribute to cpufetch.

## 1. cpufetch does not accept pull requests
cpufetch is a small project, and I enjoy developing it. There are for sure some bugs and exciting features to add, but I prefer to make these
changes myself. For that reason, you should always use the issues page to report anything related to cpufetch. In the rare case that there is
a concise bug or feature that I am unable to implement myself, I will enable pull requests for this.

## 2. Creating an issue

### 2.1: I found a bug in cpufetch (the program provides incorrect / invalid information)
In the github issue **you must include**:
- Exact CPU model.
- Operating system.
- The output of `cpufetch`.
- The output of `cpufetch --debug`.

### 2.2: I found a bug in cpufetch (the program crashes / does not work properly)
- Exact CPU model.
- Operating system.
- The output of `cpufetch`.
- The output of `cpufetch --debug`.
- A stacktrace (if program crashes):

#### Stacktrace option 1 (best)
1. Build cpufetch with debug symbols (`make clean; make debug`).
2. Install valgrind (if it is not already installed)
3. Run cpufetch with valgrind (`valgrind ./cpufetch`)
4. Paste the complete output (preferably on a platform like pastebin)

#### Stacktrace option 2 (use this option if option 1 does not work)
1. Build cpufetch with debug symbols (`make clean; make debug`).
2. Install gdb (if it is not already installed)
3. Debug cpufetch with gdb (`gdb cpufetch`)
3. Run cpufetch (just r inside gdb console)
4. Paste the complete output (preferably on a platform like pastebin)

### 2.3: I have an idea for a new feature in cpufetch / I want to suggest a change in cpufetch
Just explain the feature in the issue and include references (links) to relevant sources if appropriate.
