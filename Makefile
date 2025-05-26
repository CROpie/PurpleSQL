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

# Application object files used by tests (add all your core src .o files here)
APP_OBJS = $(PATHO)database.o $(PATHO)repl.o $(PATHO)appmain.o $(PATHO)encoder.o

# Test source files (you can keep this for bulk test file discovery)
SRCT = $(wildcard $(PATHT)**/*.c)

# Result files for test outputs
RESULTS = $(patsubst $(PATHT)%.c,$(PATHR)%.txt,$(SRCT))

# Main app settings
MAIN_SRC = src/main.c
MAIN_OBJS = $(PATHO)main.o $(APP_OBJS)
MAIN_TARGET = main.$(TARGET_EXTENSION)
MAIN_OUTPUT = $(PATHB)$(MAIN_TARGET)

# Default target — builds everything (but doesn't run tests or app)
all: $(BUILD_PATHS) $(RESULTS) $(MAIN_OUTPUT)

# Run unit tests
test: all
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@grep -rs IGNORE $(PATHR) | awk -F':' 'NF>=4 && $$(NF-1) != "" && $$NF != "" {print $$(NF-1) ":" $$NF}' || echo "None"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@grep -rs FAIL $(PATHR) | awk -F':' 'NF>=4 && $$(NF-1) != "" && $$NF != "" {print $$(NF-1) ":" $$NF}' || echo "None"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@grep -rs PASS $(PATHR) | awk -F':' 'NF>=4 && $$(NF-1) != "" && $$NF != "" {print $$(NF-1) ":" $$NF}' || echo "None"
	@echo "\nDONE"

# Run main application
run: $(MAIN_OUTPUT)
	./$<

# Create result files by executing test binaries, ensuring parent dirs exist
$(PATHR)%.txt: $(PATHB)%.${TARGET_EXTENSION}
	@mkdir -p $(dir $@)
	-./$< > $@ 2>&1

# Link test executables with unity and app objects
$(PATHB)%.${TARGET_EXTENSION}: $(PATHO)%.o $(PATHO)unity.o $(APP_OBJS)
	@mkdir -p $(dir $@)
	$(LINK) -o $@ $^

# Link main app
$(MAIN_OUTPUT): $(MAIN_OBJS)
	@mkdir -p $(dir $@)
	$(LINK) -o $@ $^

# Compile all source files from src/
$(PATHO)%.o: $(PATHS)%.c
	@mkdir -p $(dir $@)
	$(COMPILE) $(CFLAGS) $< -o $@

# Compile all test source files from test/ (including subfolders)
$(PATHO)%.o: $(PATHT)%.c
	@mkdir -p $(dir $@)
	$(COMPILE) $(CFLAGS) $< -o $@

# Compile unity sources
$(PATHO)%.o: $(PATHU)%.c $(PATHU)%.h
	@mkdir -p $(dir $@)
	$(COMPILE) $(CFLAGS) $< -o $@

# Dependency file generation (optional)
$(PATHD)%.d: $(PATHT)%.c
	$(DEPEND) $@ $<

# Create build directories
$(BUILD_PATHS):
	$(MKDIR) $@

# Debug with gdb
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

.PHONY: clean test run all debug val
.PRECIOUS: $(PATHB)%.${TARGET_EXTENSION} $(PATHD)%.d $(PATHO)%.o $(PATHR)%.txt
