OBJS = main.o socket.o http.o 
PROG = server 
TEMP_FILES = *.tmp 
HTML_FILES = *.html 
DEBUG_FLAG = -g

%.o: %.c
	gcc -c $(DEBUG_FLAG) -o $@ $<

$(PROG): $(OBJS)
	gcc $(DEBUG_FLAG) -o $@ $^

clean:
	rm -rf $(OBJS) $(PROG) $(TEMP_FILES) $(HTML_FILES)
