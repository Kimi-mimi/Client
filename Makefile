C_VER = -std=c99
FLAGS = -Wall -Werror -pedantic -g3 -gdwarf-2 -DDEBUG -g
LIBS = -lresolv
CUNIT = -lcunit

PROG = main

BUILD_DIR = build
TEST_DIR = $(BUILD_DIR)/test
TEXDIR = tex
TEXINCDIR = $(TEXDIR)/include
DOXDIR = doxygen
UDIR = utils
DOTDIR = dot

INCLUDES = $(wildcard autogen/*.h bytes/*.h client/*.h errors/*.h logger/*.h smtp/*.h)

MAKE2DOT = $(UDIR)/makefile2dot
FSM2DOT = $(UDIR)/fsm2dot
MAKESIMPLE = $(UDIR)/makesimple
CFLOW = cflow --level "0= "
CFLOW2DOT = $(UDIR)/cflow2dot
SIMPLECFLOW = grep -v -f cflow.ignore
SRC2TEX = $(UDIR)/src2tex
PDFLATEX = pdflatex -interaction=nonstopmode
DOXYGEN = doxygen

DOXYGEN_CONF_FILE = doxygen.cfg

REPORT_PDF_FILE = report.pdf
FSM_FILE = autogen/fsm.def

FSM_DOT_FILE = $(TEXINCDIR)/fsm.dot
FSM_TEX_FILE = $(TEXINCDIR)/fsm.tex
FSM_PDF_FILE = $(TEXINCDIR)/fsm.pdf

CFLOW_DOT_FILE = $(TEXINCDIR)/cflow.dot
CFLOW_TEX_FILE = $(TEXINCDIR)/cflow.tex
CFLOW_PDF_FILE = $(TEXINCDIR)/cflow.pdf

MAKEFILE_DOT_FILE = $(TEXINCDIR)/makefile.dot
MAKEFILE_TEX_FILE = $(TEXINCDIR)/makefile.tex
MAKEFILE_PDF_FILE = $(TEXINCDIR)/makefile.pdf

all: main test report

main: client.o client_errors.o logger.o smtp_command.o smtp_message.o
main: smtp_connection.o smtp_connection_list.o smtp_message_queue.o fsm_common.o fsm.o string.o bytes.o
main: create_build_dir main.o
	gcc $(C_VER) $(FLAGS) $(LIBS) -o $(BUILD_DIR)/main \
$(BUILD_DIR)/main.o \
$(BUILD_DIR)/client.o \
$(BUILD_DIR)/client_errors.o \
$(BUILD_DIR)/bytes.o \
$(BUILD_DIR)/logger.o \
$(BUILD_DIR)/string.o \
$(BUILD_DIR)/smtp_command.o \
$(BUILD_DIR)/smtp_message.o \
$(BUILD_DIR)/smtp_connection.o \
$(BUILD_DIR)/smtp_connection_list.o \
$(BUILD_DIR)/smtp_message_queue.o \
$(BUILD_DIR)/fsm_common.o \
$(BUILD_DIR)/fsm.o

create_build_dir:
	mkdir -p $(BUILD_DIR)

main.o: main.c client/client.h errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c main.c -o $(BUILD_DIR)/main.o

string.o: bytes/string.h bytes/string.c errors/client_errors.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c bytes/string.c -o $(BUILD_DIR)/string.o

bytes.o: bytes/bytes.h bytes/bytes.c errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c bytes/bytes.c -o $(BUILD_DIR)/bytes.o

client.o: client/client.h client/client.c errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c client/client.c -o $(BUILD_DIR)/client.o

logger.o: logger/logger.h logger/logger.c errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c logger/logger.c -o $(BUILD_DIR)/logger.o

smtp_command.o: smtp/smtp_command.h smtp/smtp_command.c errors/client_errors.h bytes/string.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_command.c -o $(BUILD_DIR)/smtp_command.o

smtp_message.o: smtp/smtp_message.h smtp/smtp_message.c errors/client_errors.h bytes/string.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_message.c -o $(BUILD_DIR)/smtp_message.o

smtp_connection.o: smtp/smtp_connection.c smtp/smtp_connection.h smtp/smtp_message.h
smtp_connection.o: errors/client_errors.h bytes/bytes.h bytes/string.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_connection.c -o $(BUILD_DIR)/smtp_connection.o

smtp_connection_list.o: smtp/smtp_connection_list.c smtp/smtp_connection_list.h smtp/smtp_connection.h
smtp_connection_list.o: errors/client_errors.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_connection_list.c -o $(BUILD_DIR)/smtp_connection_list.o

smtp_message_queue.o: smtp/smtp_message_queue.c smtp/smtp_message_queue.h smtp/smtp_message.h
smtp_message_queue.o: bytes/bytes.h bytes/string.h errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_message_queue.c -o $(BUILD_DIR)/smtp_message_queue.o

fsm_common.o: autogen/fsm-common.c autogen/fsm-common.h smtp/smtp_command.h smtp/smtp_connection.h smtp/smtp_connection_list.h
fsm_common.o: bytes/string.h logger/logger.h errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c autogen/fsm-common.c -o $(BUILD_DIR)/fsm_common.o

fsm.o: autogen/fsm-fsm.c autogen/fsm-fsm.h autogen/fsm-common.h smtp/smtp_command.h smtp/smtp_connection.h smtp/smtp_connection_list.h
fsm.o: bytes/string.h logger/logger.h errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c autogen/fsm-fsm.c -o $(BUILD_DIR)/fsm.o

client_errors.o: errors/client_errors.h errors/client_errors.c logger/logger.h
	gcc $(C_VER) $(FLAGS) -c errors/client_errors.c -o $(BUILD_DIR)/client_errors.o

# TEST

test: string.o bytes.o smtp_message.o smtp_message_queue.o smtp_connection.o smtp_connection_list.o
test: string_test.o bytes_test.o smtp_message_test.o smtp_message_queue_test.o smtp_connection_test.o smtp_connection_list_test.o
test: create_test_dir test.o
	gcc $(C_VER) $(FLAGS) $(CUNIT) $(LIBS) -o $(TEST_DIR)/test \
$(TEST_DIR)/test.o \
$(BUILD_DIR)/string.o \
$(TEST_DIR)/string_test.o \
$(BUILD_DIR)/bytes.o \
$(TEST_DIR)/bytes_test.o \
$(BUILD_DIR)/smtp_message.o \
$(TEST_DIR)/smtp_message_test.o \
$(BUILD_DIR)/smtp_message_queue.o \
$(TEST_DIR)/smtp_message_queue_test.o \
$(BUILD_DIR)/smtp_connection.o \
$(TEST_DIR)/smtp_connection_test.o \
$(BUILD_DIR)/smtp_connection_list.o \
$(TEST_DIR)/smtp_connection_list_test.o \

create_test_dir:
	mkdir -p $(TEST_DIR)

test.o: test.c bytes/test/string_test.h bytes/test/bytes_test.h
test.o: smtp/test/smtp_message_test.h
	gcc $(C_VER) $(FLAGS) -c test.c -o $(TEST_DIR)/test.o

bytes_test.o: bytes/test/bytes_test.c bytes/test/bytes_test.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c bytes/test/bytes_test.c -o $(TEST_DIR)/bytes_test.o

string_test.o: bytes/test/string_test.c bytes/test/string_test.h bytes/string.h
	gcc $(C_VER) $(FLAGS) -c bytes/test/string_test.c -o $(TEST_DIR)/string_test.o

smtp_message_test.o: smtp/test/smtp_message_test.h smtp/test/smtp_message_test.c
smtp_message_test.o: bytes/string.h smtp/smtp_message.h
	gcc $(C_VER) $(FLAGS) -c smtp/test/smtp_message_test.c -o $(TEST_DIR)/smtp_message_test.o

smtp_message_queue_test.o: smtp/test/smtp_message_queue_test.h smtp/test/smtp_message_queue_test.c
smtp_message_queue_test.o: smtp/smtp_message.h smtp/smtp_message_queue.h
	gcc $(C_VER) $(FLAGS) -c smtp/test/smtp_message_queue_test.c -o $(TEST_DIR)/smtp_message_queue_test.o

smtp_connection_test.o: smtp/test/smtp_connection_test.h smtp/test/smtp_connection_test.c
smtp_connection_test.o: bytes/string.h smtp/smtp_connection.h
	gcc $(C_VER) $(FLAGS) -c smtp/test/smtp_connection_test.c -o $(TEST_DIR)/smtp_connection_test.o

smtp_connection_list_test.o: smtp/test/smtp_connection_list_test.h smtp/test/smtp_connection_list_test.c
smtp_connection_list_test.o: bytes/string.h smtp/smtp_connection.h smtp/smtp_connection_list.h
	gcc $(C_VER) $(FLAGS) -c smtp/test/smtp_connection_list_test.c -o $(TEST_DIR)/smtp_connection_list_test.o

# Report
report: create_texinc_dir $(REPORT_PDF_FILE)

create_texinc_dir:
	mkdir -p $(TEXINCDIR)

$(FSM_DOT_FILE): $(FSM_FILE)
	$(FSM2DOT) $< > $@

# .dot -> _dot.tex
$(FSM_TEX_FILE): $(FSM_DOT_FILE)
	dot2tex -ftikz --autosize --crop  $< > $@

# _dot.tex -> _dot.pdf
$(FSM_PDF_FILE): $(FSM_TEX_FILE)
	$(PDFLATEX) -output-directory $(TEXINCDIR) $<

# cflow -> cflow.dot
$(CFLOW_DOT_FILE): client/client.c main.c
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

# cflow.dot -> cflow.tex
$(CFLOW_TEX_FILE): $(CFLOW_DOT_FILE)
	dot2tex -ftikz --autosize --crop  $< > $@

# cflow.tex -> cflow.pdf
$(CFLOW_PDF_FILE): $(CFLOW_TEX_FILE)
	$(PDFLATEX) -output-directory $(TEXINCDIR) $<

simplest.mk: Makefile
	sed 's/$$(INCLUDES)//'  Makefile | $(MAKESIMPLE)  > simplest.mk

# simplest.mk -> makefile.dot
$(MAKEFILE_DOT_FILE): $(MAKE2DOT) simplest.mk
	$(MAKE2DOT) $(PROG) < simplest.mk > $(MAKEFILE_DOT_FILE)

# makefile.dot -> makefile.tex
$(MAKEFILE_TEX_FILE): $(MAKEFILE_DOT_FILE)
	dot2tex -ftikz --autosize --crop  $< > $@

# makefile.tex -> makefile.pdf
$(MAKEFILE_PDF_FILE): $(MAKEFILE_TEX_FILE)
	$(PDFLATEX) -output-directory $(TEXINCDIR) $<

doxygen: $(DOXYGEN_CONF_FILE)
	$(DOXYGEN) $(DOXYGEN_CONF_FILE)

doxygen_pdf: doxygen
	cd tex/include/doxygen/latex && make && cd ../../../../

# report itself
$(REPORT_PDF_FILE): $(FSM_PDF_FILE) $(CFLOW_PDF_FILE) $(MAKEFILE_PDF_FILE) doxygen_pdf
	cd $(TEXDIR) && $(PDFLATEX) report.tex && $(PDFLATEX) report.tex && cp $(REPORT_PDF_FILE) ..
	#echo 1

# CLEAN

clean:
	rm -rf $(BUILD_DIR) *.log $(TEXINCDIR) *.pdf simplest.mk \
$(TEXDIR)/*.aux $(TEXDIR)/*.log $(TEXDIR)/*.out $(TEXDIR)/*.pdf $(TEXDIR)/*.toc
