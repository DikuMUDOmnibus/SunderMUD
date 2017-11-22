
LIBS     = -lcrypt -lz
FLAGS    = -O2 -Wall -g3 $(PROF) $(NOCRYPT) -DI3SUNDER
INCLUDES = -I.
PROF     =
C_FLAGS  = $(FLAGS) $(INCLUDES)
L_FLAGS  = $(FLAGS) -O2 $(PROF) $(LIBS) $(INCLUDES)
EXE		 = sundermud
CC       = gcc
O_DIR    = o/

##############
## If compiling on Cygwin, use the following line instead of the next.
#OSEXT    = .exe
OSEXT    =


FILES = act_click.c act_comm.c act_info.c act_magic.c act_move.c act_obj.c act_skill.c \
        act_wiz.c bit.c board.c channels.c calcfunc.c clan.c clients.c comm.c const.c convert.c db.c db2.c \
        db_area.c fight.c handler.c i3.c interp.c lease.c mccp.c mem.c mob_cmds.c mob_prog.c \
        notify.c olc.c olc_act.c olc_help.c olc_mpcode.c olc_save.c pfile.c quest.c \
        special.c spells.c spell2.c ssm.c string.c update.c

C_FILES = $(sort $(FILES))
O_FILES = $(patsubst %.c,$(O_DIR)%.o, $(C_FILES))

$(EXE): $(O_FILES)
	rm -f $(EXE)$(OSEXT)
	$(CC) -o $(EXE) $(L_FLAGS) $(O_FILES) $(LIBS)	
	mv -f ../bin/$(EXE)$(OSEXT) ../bin/$(EXE).backup
	mv -f $(EXE)$(OSEXT) ../bin/$(EXE)$(OSEXT)  

$(O_DIR)%.o: %.c
	$(CC) -c $(C_FLAGS) $< -o $@

clean:
	rm -f $(O_FILES) $(EXE)

 
