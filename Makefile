# COMPILER

#IBM cluster compiler and edu215 compiler
CXX= gcc 

# FILES

OBJS =	 src/Main.o      
	
# CPLEX VERSION (LIBS and INCLUDE files)


CPLEXLIBDIR =  /Users/fabiofurini/Applications/IBM/ILOG/CPLEX_Studio126/cplex/lib/x86-64_osx/static_pic
CPLEXINCDIR= /Users/fabiofurini/Applications/IBM/ILOG/CPLEX_Studio126/cplex/include/ilcplex

#CPLEXLIBDIR =  /home/fabio/ILOG/CPLEX_Studio_AcademicResearch125/cplex/lib/x86-64_sles10_4.1/static_pic/
#CPLEXINCDIR= /home/fabio/ILOG/CPLEX_Studio_AcademicResearch125/cplex/include/ilcplex/


# Nothing should be changed

LP_LIBS =  -L$(CPLEXLIBDIR) -lilocplex -lcplex -lm -lpthread   


DBG= -O3
#DBG= -g

INCDIR = -I. -I$(CPLEXINCDIR)  -I$(INCPATHS) -I.

#COMPILER FLAGS

CXXFLAGS =  $(DBG) $(INCDIR) 

.c.o:
	gcc -c $(CXXFLAGS) $< -o $@ $(LDOPT)

LDLIBS = $(LP_LIBS)

all: KP

KP: $(OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS) $(LDOPT)

$(OBJS): Makefile

clean:
	rm -f $(OBJS) rm KP
