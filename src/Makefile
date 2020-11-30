CC      = gcc
# -I /usr/include/ -L /home/dist/zlib-1.2.3/
PROF    = -O3 -ggdb
NOCRYPT =
C_FLAGS =  -Wall -Dlinux $(PROF) $(NOCRYPT)
L_FLAGS =  $(PROF)
RM	= rm -f
RM_CMD	= $(RM) *.CKP *.ln *.BAK *.bak *.o core errs ,* *~ *.a .emacs_* tags TAGS make.log MakeOut

ROT_FILES = act_comm.o act_enter.o act_info.o act_info2.o \
	act_move.o act_obj.o act_wiz.o alias.o arena.o \
	auction.o ban.o banks.o bit.o blade.o clans.o \
	clan_list.o clanskii.o class.o comm.o const.o curse.o darkside.o \
	db.o db2.o drunk.o effects.o emblazon.o fight.o \
	fight2.o finger.o flags.o forget.o games.o handler.o \
	healer.o hedit.o interp.o lookup.o magic.o magic2.o \
	magtype.o mccp.o mem.o mob_cmds.o mob_prog.o music.o \
	nclan.o new_clan_skills.o  note.o olc.o olc_act.o olc_mpcode.o olc_save.o \
	olevel.o ooc.o pload.o quest.o rand_obj.o ranger.o \
	recover.o recycle.o repent.o save.o scan.o  sign.o skills.o \
	songs.o spartan.o  special.o stock_market.o string.o summoner.o tables.o \
	update.o voodan.o who_list.o wizlist.o \

all:: rot

rot: $(ROT_FILES)
	rm -f rot
	$(CC) $(L_FLAGS) -o rot $(ROT_FILES) -lm -lz -lcrypt

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean::
	$(RM_CMD) \#*
