TARGET=interpreter.asm iolib.inc inner_functions.inc inner_words.inc
BIN=../mforth

all: $(BIN)

$(BIN): $(BIN).o
	ld -o $@ $^

$(BIN).o: $(TARGET)
	nasm -felf64 $< -o $@

clean: 
	rm ../mforth.o
