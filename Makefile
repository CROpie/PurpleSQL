# OS-Specific Settings
ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # Not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
  else # In bash-like shell (e.g. Git Bash)
	CLEANUP = rm -f
	MKDIR = mkdir -p
  endif
	TARGET_EXTENSION = exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION = out
endif

# Paths
PATHU = unity/src/
PATHS = src/
PATHI = include/
PATHT = test/
PATHB = build/
PATHD = build/depends/
PATHO = build/objs/
PATHR = build/results/

BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHR)

# Tools & Flags
COMPILE = gcc -c
LINK = gcc
DEPEND = gcc -MM -MG -MF
CFLAGS = -I. -I$(PATHU) -I$(PATHI) -g -DTEST 

# Test File Setup
SRCT = $(wildcard $(PATHT)*.c)
RESULTS = $(patsubst $(PATHT)Test%.c,$(PATHR)Test%.txt,$(SRCT))

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

# Main app settings
MAIN_SRC = src/main.c
MAIN_OBJS = $(PATHO)main.o $(PATHO)database.o $(PATHO)repl.o
#MAIN_OBJS = $(wildcard $(PATHO)*.o)
MAIN_TARGET = main.$(TARGET_EXTENSION)
MAIN_OUTPUT = build/$(MAIN_TARGET)

# Default target — builds everything (but doesn't run tests or app)
all: $(BUILD_PATHS) $(RESULTS) $(MAIN_OUTPUT)

# Run unit tests
test: all
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# Run main application
run: $(MAIN_OUTPUT)
	./$<

# Create result files by executing test binaries
$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

# Link test binaries
$(PATHB)Test%.$(TARGET_EXTENSION): $(PATHO)Test%.o $(PATHO)repl.o $(PATHO)%.o $(PATHO)unity.o

# $(PATHB)Test%.$(TARGET_EXTENSION): $(PATHO)Test%.o $(PATHO)%.o $(PATHO)unity.o
	$(LINK) -o $@ $^

# Link main app
$(MAIN_OUTPUT): $(MAIN_OBJS)
	$(LINK) -o $@ $^

# Compile all sources to objects
$(PATHO)%.o: $(PATHT)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)%.o: $(PATHS)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)%.o: $(PATHU)%.c $(PATHU)%.h
	$(COMPILE) $(CFLAGS) $< -o $@

# Dependency file generation (optional)
$(PATHD)%.d: $(PATHT)%.c
	$(DEPEND) $@ $<

# Create build directories
$(PATHB) $(PATHD) $(PATHO) $(PATHR):
	$(MKDIR) $@

# Debug
debug: $(MAIN_OUTPUT) 
	gdb -tui $(MAIN_OUTPUT)

# Valgrind
val: $(MAIN_OUTPUT)
	valgrind --track-origins=yes --leak-check=full $(MAIN_OUTPUT)

# Clean target
clean:
	$(CLEANUP) $(PATHO)*.o
	$(CLEANUP) $(PATHB)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)*.txt

.PHONY: clean test run all
.PRECIOUS: $(PATHB)Test%.$(TARGET_EXTENSION) $(PATHD)%.d $(PATHO)%.o $(PATHR)%.txt
