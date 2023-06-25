# This makefile is used to generate svg files.

DOT = dot
PLANTUML = plantuml

SRC_DIRS = ./code
RELEASE_DIR := .

DOT_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.dot))
SVG_FROM_DOT_FILES = $(patsubst %.dot,%.svg, $(DOT_FILES))

PLANTUML_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.plantuml))
SVG_FROM_PLANTUML_FILES = $(patsubst %.plantuml,%.svg, $(PLANTUML_FILES))

$(info $(DOT_FILES))
$(info $(PLANTUML_FILES))

all: $(addprefix $(RELEASE_DIR)/,$(SVG_FROM_DOT_FILES)) $(addprefix $(RELEASE_DIR)/,$(SVG_FROM_PLANTUML_FILES))
.PHONY : clean all

$(RELEASE_DIR)/%.svg:%.dot
	$(DOT) -Tsvg $^ -o $@

$(RELEASE_DIR)/%.svg:%.plantuml
	$(PLANTUML) -tsvg $^ > $@

clean:
	rm -v *.svg