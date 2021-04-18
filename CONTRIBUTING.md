# cpufetch contributing guidelines

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->


- [Creating a pull request](#creating-a-pull-request)
  - [Case 1: I found a bug in cpufetch](#case-1-i-found-a-bug-in-cpufetch)
  - [Case 2: I have an idea for a new feature in cpufetch / I want to suggest a change in cpufetch](#case-2-i-have-an-idea-for-a-new-feature-in-cpufetch--i-want-to-suggest-a-change-in-cpufetch)
  - [Case 3: I want to make changes to the Makefile](#case-3-i-want-to-make-changes-to-the-makefile)
- [Creating an issue](#creating-an-issue)
  - [cpufetch fails / crashes with a segmentation fault / ends without any output](#cpufetch-fails--crashes-with-a-segmentation-fault--ends-without-any-output)
    - [Option 1 (best)](#option-1-best)
    - [Option 2 (use this option if you can't work with option 1)](#option-2-use-this-option-if-you-cant-work-with-option-1)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Creating a pull request

Thanks for your interest in contributing to cpufetch!

What kind of contribution are you going to propose?

### Case 1: I found a bug in cpufetch
If you found a bug, please don't open a pull request; open an issue instead, even if you know the solution. I appreciate people finding bugs, but I generally prefer to fix them myself.

### Case 2: I have an idea for a new feature in cpufetch / I want to suggest a change in cpufetch
Great! Make a PR and make sure to explain how did you implement your new feature on the github conversation page.

### Case 3: I want to make changes to the Makefile
Don't open a pull request; open an issue instead and suggest your changes in the Makefile there. Except for extraordinary cases, I will not accept changes in the Makefile.

## Creating an issue

If you are going to report a bug or problem, always report the CPU model and OS. If possible, also paste the output of `cpufetch` and `cpufetch --debug`.

### cpufetch fails / crashes with a segmentation fault / ends without any output
You need to provide additional information in the github issue:

#### Option 1 (best)
1. Build cpufetch with debug symbols (`make clean; make debug`).
2. Install valgrind (if it is not already installed)
3. Run cpufetch with valgrind (`valgrind ./cpufetch`)
4. Paste the complete output (preferably on a platform like pastebin)

#### Option 2 (use this option if you can't work with option 1)
1. Build cpufetch with debug symbols (`make clean; make debug`).
2. Install gdb (if it is not already installed)
3. Debug cpufetch with gdb (`gdb cpufetch`)
3. Run cpufetch (just r inside gdb console)
4. Paste the complete output (preferably on a platform like pastebin)
