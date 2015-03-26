###############################################################################
# NoSpark - OpenEVSE charger firmware
# Copyright (C) 2015 Andre Eisenbach
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# See LICENSE for a copy of the GNU General Public License or see
# it online at <http://www.gnu.org/licenses/>.
###############################################################################

include Makefile.inc

LIBS = 

CXXFLAGS = $(PRJCXXFLAGS) -I.

SRCDIR = .
DEPDIR = .dep
OBJDIR = obj

SOURCE = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)%, $(OBJDIR)%, $(SOURCE:.cpp=.o))
DEPS = $(patsubst $(OBJDIR)%, $(DEPDIR)%, $(OBJS:.o=.d))
LIBA = $(patsubst %, $(OBJDIR)/lib%.a, $(LIBS))
LIBL = -L$(OBJDIR) $(patsubst %, -l%, $(LIBS))

TARGET = $(notdir $(CURDIR)).hex
ELF = $(notdir $(CURDIR)).elf

all: $(TARGET)

upload: $(TARGET)
	@echo [UPL] $<
	@$(AVRDUDE) $(DUDEFLAGS) -U flash:w:$<

clean:
	-@rm $(TARGET)
	-@rm -rf .dep
	-@rm -rf obj
	-@for lib in $(LIBS); do (cd $$lib; $(MAKE) clean); done

$(DEPDIR)/%.d: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MM $< > $@
	@sed -i -e "s,$*.o:,$(OBJDIR)/$*.o $@: ,g" $@

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo [C++] $(notdir $<)
	@$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.a: force_look
	@cd $(patsubst lib%, %, $(basename $(notdir $@)) ; $(MAKE) $(MFLAGS))

$(OBJDIR)/$(ELF): $(LIBA) $(OBJS)
	@echo [LNK] $(notdir $@)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBL) -o $@
	@echo
	@$(SIZE) $(SIZEFLAGS) $@

%.hex: $(OBJDIR)/%.elf
	@echo [HEX] $@
	@$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

force_look:
	@true

-include $(DEPS)
