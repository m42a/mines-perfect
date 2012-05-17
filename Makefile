CPP:=g++
CFLAGS:=

CORE_OBJS:=core/board.o core/eqs.o core/logbook.o core/options.o core/perfana.o core/utils.o core/vargroup.o
GUI_OBJS:=gui/bevelctrl.o gui/bitmapctrl.o gui/boardctrl.o gui/buttonctrl.o gui/ctrl.o gui/gamectrl.o gui/lcdctrl.o gui/smileyctrl.o
WX_OBJS:=wxwin/api.o wxwin/app.o wxwin/dialogs.o

OBJS:=$(CORE_OBJS) $(GUI_OBJS) $(WX_OBJS)
DEPENDS:=$(OBJS:%.o=%.d)

mines-perfect: $(OBJS)
	g++ -o mines-perfect $(CFLAGS) $(OBJS)

-include $(DEPENDS)

$(OBJS): %.o: %.cpp
		$(CPP) -MMD -MP -c $(CFLAGS) $<

.PHONY: clean

clean:
	rm -f $(OBJS) $(DEPENDS) *~
