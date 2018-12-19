# To "make"       run in terminal: "make client" or "make server"
# To "make clean" run in terminal: "make client_clean" or "make server_clean"
player:
	make -f makefile.player
player_clean:
	make clean -f makefile.player
server:
	make -f makefile.server
server_clean:
	make clean -f makefile.server
