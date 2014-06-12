
# Makefile for the gpc++ kernel

include		../Makefile.ini



TARGET		= $(GPLIBFILE)

GPKERNEL= 	code.o
 

OBJS		= $(GPKERNEL) 

CPPFLAGS	= $(OVERALLCPPFLAGS) -I$(GPINCLUDE) -I/usr/lib/jvm/java-8-oracle/include/ \
			-I/usr/lib/jvm/java-8-oracle/include/linux/

# Implicit rule how to compile
.cc.o:
	$(CCC) $(CPPFLAGS) -c $<



# Create the GP library. Use either ld (for dynamic libraries) or ar
# and ranlib.
$(TARGET) : $(OBJS) 
#	$(AR) -r $(TARGET) $(OBJS)
#	$(RANLIB) $(TARGET)
	$(LD) -r -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) *~

superclean: clean
	rm -f $(TARGET)

strip:
	strip $(OBJS)
	make



# File dependencies
code.o: code.cc ../include/code.h