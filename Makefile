####################################################
#
# CMPSC311 - Fall 2017
# Assignment #2 Makefile
# Author   : Patrick McDaniel
# Created  : Sat Sep 2nd 08:56:10 EDT 2017
#
####################################################
#
# STUDENTS MUST ADD COMMENTS BELOW
#

# Define Make environment
# Variables
ARCHIVE=ar		# Create and update static library files
CC=gcc			# Overriding ‘cc’ to gcc 
LINK=gcc		# Set link to gcc
CFLAGS=-c -Wall -I. -fpic -g	# Set the flags to be used with the default compiler
LINKFLAGS=-L. -g	# Specify which shell link structures	
LIBFLAGS=-shared -Wall	# Pass flags to the linker
LINKLIBS=-lcrud -lgcrypt 	# Link to the libraries
DEPFILE=Makefile.dep		# makefile dependency

# Files to build
# The 'hdd_sim' target has dependency 'hdd_sim.o' and 'hdd_file_io.o' 
HDD_SIM_OBJFILES=	hdd_sim.o \
					hdd_file_io.o
# Target file name
TARGETS=	hdd_sim 
					
# Suffix rules
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS)  -o $@ $<

# Productions
# Top level target hdd_sim
all : $(TARGETS) 
	
hdd_sim : $(HDD_SIM_OBJFILES) libcrud.a
	$(LINK) $(LINKFLAGS) -o $@ $(HDD_SIM_OBJFILES) $(LINKLIBS) 

# Do dependency generation
# Specify dependencies
depend : $(DEPFILE)

$(DEPFILE) : $(HDD_SIM_OBJFILES:.o=.c)
	gcc -MM $(CFLAGS) $(HDD_SIM_OBJFILES:.o=.c) > $(DEPFILE)
        
# Cleanup 
# Delete all files that are created by make command
clean:
	rm -f $(TARGETS) $(HDD_SIM_OBJFILES) 
  
# Dependancies

