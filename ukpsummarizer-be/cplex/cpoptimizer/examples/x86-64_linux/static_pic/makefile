CCC = g++
CONCERTDIR = ../../../../concert
CPLEXDIR = ../../../../cplex
CFLAGS = -DIL_STD -O -DNDEBUG -I../../../include -I$(CONCERTDIR)/include -fPIC -fstrict-aliasing -pedantic -Wall -fexceptions -frounding-math -Wno-long-long -m64 -DILOUSEMT -D_REENTRANT -DILM_REENTRANT
LDFLAGS = -L../../../lib/x86-64_linux/static_pic -lcp -L$(CPLEXDIR)/lib/x86-64_linux/static_pic -lcplex -L$(CONCERTDIR)/lib/x86-64_linux/static_pic -lconcert  -lpthread -lm -ldl 
JAVA = $(JAVA_HOME)/bin/java
JAVAC = $(JAVA_HOME)/bin/javac
JAVAFLAGS = -d64
CPCLASSPATH = ../../../lib/ILOG.CP.jar
LIBRARYPATH = ../../../bin/x86-64_linux:$(CPLEXDIR)/bin/x86-64_linux
CPPEXDIR = ../../../examples/src/cpp
JAVAEXDIR = ../../../examples/src/java

all: cpp java

cpp: alloc color color_ex3 color_ex4 cpofileformat facility facility_ex3 facility_ex4 facility_explanations intro networkcfg plantlocation ppp sched_calendar sched_conflict sched_cumul sched_flowshop sched_goalprog sched_intro sched_jobshop sched_jobshopflex sched_learningeffect sched_openshop sched_optional sched_pflowshop sched_rcpsp sched_rcpspmm schedsearch_greedyandrelax schedsearch_optionalsettimes schedsearch_permutation schedsearch_sequencing schedsearch_settimes sched_sequence sched_setup sched_square sched_state sched_stochastic_jobshop sched_tcost sched_time searchphases smooth sports sports_strong steelmill steelmill_ex3 talent teambuilding truckfleet

java: Alloc.class Color.class CPOFileFormat.class FacilityExplanations.class Facility.class PlantLocation.class Ppp.class SchedCalendar.class SchedConflict.class SchedCumul.class SchedFlowShop.class SchedGoalProg.class SchedIntro.class SchedJobShopFlex.class SchedJobShop.class SchedLearningEffect.class SchedOpenShop.class SchedOptional.class SchedPFlowShop.class SchedRCPSP.class SchedRCPSPMM.class SchedSequence.class SchedSetup.class SchedSquare.class SchedState.class SchedStochasticJobShop.class SchedTCost.class SchedTime.class SearchPhases.class Sports.class SportsStrong.class Steelmill.class Talent.class Teambuilding.class Truckfleet.class

execute: execute_cpp execute_java

clean:
	rm -f alloc color color_ex3 color_ex4 cpofileformat facility facility_ex3 facility_ex4 facility_explanations intro networkcfg plantlocation ppp sched_calendar sched_conflict sched_cumul sched_flowshop sched_goalprog sched_intro sched_jobshop sched_jobshopflex sched_learningeffect sched_openshop sched_optional sched_pflowshop sched_rcpsp sched_rcpspmm schedsearch_greedyandrelax schedsearch_optionalsettimes schedsearch_permutation schedsearch_sequencing schedsearch_settimes sched_sequence sched_setup sched_square sched_state sched_stochastic_jobshop sched_tcost sched_time searchphases smooth sports sports_strong steelmill steelmill_ex3 talent teambuilding truckfleet *.class

FORCE:

$(JAVAC):
	@echo "####################################"
	@echo "Error: cannot find the javac command"
	@echo "Please ensure that JAVA_HOME is set correctly"
	@echo "Currently, JAVA_HOME = $(JAVA_HOME)"
	@exit 1
alloc:
	$(CCC) -o alloc $(CFLAGS) $(CPPEXDIR)/alloc.cpp $(LDFLAGS)
alloc.run: alloc FORCE
	./alloc
color:
	$(CCC) -o color $(CFLAGS) $(CPPEXDIR)/color.cpp $(LDFLAGS)
color.run: color FORCE
	./color
color_ex3:
	$(CCC) -o color_ex3 $(CFLAGS) $(CPPEXDIR)/color_ex3.cpp $(LDFLAGS)
color_ex3.run: color_ex3 FORCE
	./color_ex3
color_ex4:
	$(CCC) -o color_ex4 $(CFLAGS) $(CPPEXDIR)/color_ex4.cpp $(LDFLAGS)
color_ex4.run: color_ex4 FORCE
	./color_ex4
cpofileformat:
	$(CCC) -o cpofileformat $(CFLAGS) $(CPPEXDIR)/cpofileformat.cpp $(LDFLAGS)
cpofileformat.run: cpofileformat FORCE
	./cpofileformat
facility:
	$(CCC) -o facility $(CFLAGS) $(CPPEXDIR)/facility.cpp $(LDFLAGS)
facility.run: facility FORCE
	./facility
facility_ex3:
	$(CCC) -o facility_ex3 $(CFLAGS) $(CPPEXDIR)/facility_ex3.cpp $(LDFLAGS)
facility_ex3.run: facility_ex3 FORCE
	./facility_ex3
facility_ex4:
	$(CCC) -o facility_ex4 $(CFLAGS) $(CPPEXDIR)/facility_ex4.cpp $(LDFLAGS)
facility_ex4.run: facility_ex4 FORCE
	./facility_ex4
facility_explanations:
	$(CCC) -o facility_explanations $(CFLAGS) $(CPPEXDIR)/facility_explanations.cpp $(LDFLAGS)
facility_explanations.run: facility_explanations FORCE
	./facility_explanations
intro:
	$(CCC) -o intro $(CFLAGS) $(CPPEXDIR)/intro.cpp $(LDFLAGS)
intro.run: intro FORCE
	./intro
networkcfg:
	$(CCC) -o networkcfg $(CFLAGS) $(CPPEXDIR)/networkcfg.cpp $(LDFLAGS)
networkcfg.run: networkcfg FORCE
	./networkcfg
plantlocation:
	$(CCC) -o plantlocation $(CFLAGS) $(CPPEXDIR)/plantlocation.cpp $(LDFLAGS)
plantlocation.run: plantlocation FORCE
	./plantlocation
ppp:
	$(CCC) -o ppp $(CFLAGS) $(CPPEXDIR)/ppp.cpp $(LDFLAGS)
ppp.run: ppp FORCE
	./ppp
sched_calendar:
	$(CCC) -o sched_calendar $(CFLAGS) $(CPPEXDIR)/sched_calendar.cpp $(LDFLAGS)
sched_calendar.run: sched_calendar FORCE
	./sched_calendar
sched_conflict:
	$(CCC) -o sched_conflict $(CFLAGS) $(CPPEXDIR)/sched_conflict.cpp $(LDFLAGS)
sched_conflict.run: sched_conflict FORCE
	./sched_conflict
sched_cumul:
	$(CCC) -o sched_cumul $(CFLAGS) $(CPPEXDIR)/sched_cumul.cpp $(LDFLAGS)
sched_cumul.run: sched_cumul FORCE
	./sched_cumul
sched_flowshop:
	$(CCC) -o sched_flowshop $(CFLAGS) $(CPPEXDIR)/sched_flowshop.cpp $(LDFLAGS)
sched_flowshop.run: sched_flowshop FORCE
	./sched_flowshop
sched_goalprog:
	$(CCC) -o sched_goalprog $(CFLAGS) $(CPPEXDIR)/sched_goalprog.cpp $(LDFLAGS)
sched_goalprog.run: sched_goalprog FORCE
	./sched_goalprog
sched_intro:
	$(CCC) -o sched_intro $(CFLAGS) $(CPPEXDIR)/sched_intro.cpp $(LDFLAGS)
sched_intro.run: sched_intro FORCE
	./sched_intro
sched_jobshop:
	$(CCC) -o sched_jobshop $(CFLAGS) $(CPPEXDIR)/sched_jobshop.cpp $(LDFLAGS)
sched_jobshop.run: sched_jobshop FORCE
	./sched_jobshop
sched_jobshopflex:
	$(CCC) -o sched_jobshopflex $(CFLAGS) $(CPPEXDIR)/sched_jobshopflex.cpp $(LDFLAGS)
sched_jobshopflex.run: sched_jobshopflex FORCE
	./sched_jobshopflex
sched_learningeffect:
	$(CCC) -o sched_learningeffect $(CFLAGS) $(CPPEXDIR)/sched_learningeffect.cpp $(LDFLAGS)
sched_learningeffect.run: sched_learningeffect FORCE
	./sched_learningeffect
sched_openshop:
	$(CCC) -o sched_openshop $(CFLAGS) $(CPPEXDIR)/sched_openshop.cpp $(LDFLAGS)
sched_openshop.run: sched_openshop FORCE
	./sched_openshop
sched_optional:
	$(CCC) -o sched_optional $(CFLAGS) $(CPPEXDIR)/sched_optional.cpp $(LDFLAGS)
sched_optional.run: sched_optional FORCE
	./sched_optional
sched_pflowshop:
	$(CCC) -o sched_pflowshop $(CFLAGS) $(CPPEXDIR)/sched_pflowshop.cpp $(LDFLAGS)
sched_pflowshop.run: sched_pflowshop FORCE
	./sched_pflowshop
sched_rcpsp:
	$(CCC) -o sched_rcpsp $(CFLAGS) $(CPPEXDIR)/sched_rcpsp.cpp $(LDFLAGS)
sched_rcpsp.run: sched_rcpsp FORCE
	./sched_rcpsp
sched_rcpspmm:
	$(CCC) -o sched_rcpspmm $(CFLAGS) $(CPPEXDIR)/sched_rcpspmm.cpp $(LDFLAGS)
sched_rcpspmm.run: sched_rcpspmm FORCE
	./sched_rcpspmm
schedsearch_greedyandrelax:
	$(CCC) -o schedsearch_greedyandrelax $(CFLAGS) $(CPPEXDIR)/schedsearch_greedyandrelax.cpp $(LDFLAGS)
schedsearch_greedyandrelax.run: schedsearch_greedyandrelax FORCE
	./schedsearch_greedyandrelax
schedsearch_optionalsettimes:
	$(CCC) -o schedsearch_optionalsettimes $(CFLAGS) $(CPPEXDIR)/schedsearch_optionalsettimes.cpp $(LDFLAGS)
schedsearch_optionalsettimes.run: schedsearch_optionalsettimes FORCE
	./schedsearch_optionalsettimes
schedsearch_permutation:
	$(CCC) -o schedsearch_permutation $(CFLAGS) $(CPPEXDIR)/schedsearch_permutation.cpp $(LDFLAGS)
schedsearch_permutation.run: schedsearch_permutation FORCE
	./schedsearch_permutation
schedsearch_sequencing:
	$(CCC) -o schedsearch_sequencing $(CFLAGS) $(CPPEXDIR)/schedsearch_sequencing.cpp $(LDFLAGS)
schedsearch_sequencing.run: schedsearch_sequencing FORCE
	./schedsearch_sequencing
schedsearch_settimes:
	$(CCC) -o schedsearch_settimes $(CFLAGS) $(CPPEXDIR)/schedsearch_settimes.cpp $(LDFLAGS)
schedsearch_settimes.run: schedsearch_settimes FORCE
	./schedsearch_settimes
sched_sequence:
	$(CCC) -o sched_sequence $(CFLAGS) $(CPPEXDIR)/sched_sequence.cpp $(LDFLAGS)
sched_sequence.run: sched_sequence FORCE
	./sched_sequence
sched_setup:
	$(CCC) -o sched_setup $(CFLAGS) $(CPPEXDIR)/sched_setup.cpp $(LDFLAGS)
sched_setup.run: sched_setup FORCE
	./sched_setup
sched_square:
	$(CCC) -o sched_square $(CFLAGS) $(CPPEXDIR)/sched_square.cpp $(LDFLAGS)
sched_square.run: sched_square FORCE
	./sched_square
sched_state:
	$(CCC) -o sched_state $(CFLAGS) $(CPPEXDIR)/sched_state.cpp $(LDFLAGS)
sched_state.run: sched_state FORCE
	./sched_state
sched_stochastic_jobshop:
	$(CCC) -o sched_stochastic_jobshop $(CFLAGS) $(CPPEXDIR)/sched_stochastic_jobshop.cpp $(LDFLAGS)
sched_stochastic_jobshop.run: sched_stochastic_jobshop FORCE
	./sched_stochastic_jobshop
sched_tcost:
	$(CCC) -o sched_tcost $(CFLAGS) $(CPPEXDIR)/sched_tcost.cpp $(LDFLAGS)
sched_tcost.run: sched_tcost FORCE
	./sched_tcost
sched_time:
	$(CCC) -o sched_time $(CFLAGS) $(CPPEXDIR)/sched_time.cpp $(LDFLAGS)
sched_time.run: sched_time FORCE
	./sched_time
searchphases:
	$(CCC) -o searchphases $(CFLAGS) $(CPPEXDIR)/searchphases.cpp $(LDFLAGS)
searchphases.run: searchphases FORCE
	./searchphases
smooth:
	$(CCC) -o smooth $(CFLAGS) $(CPPEXDIR)/smooth.cpp $(LDFLAGS)
smooth.run: smooth FORCE
	./smooth
sports:
	$(CCC) -o sports $(CFLAGS) $(CPPEXDIR)/sports.cpp $(LDFLAGS)
sports.run: sports FORCE
	./sports
sports_strong:
	$(CCC) -o sports_strong $(CFLAGS) $(CPPEXDIR)/sports_strong.cpp $(LDFLAGS)
sports_strong.run: sports_strong FORCE
	./sports_strong
steelmill:
	$(CCC) -o steelmill $(CFLAGS) $(CPPEXDIR)/steelmill.cpp $(LDFLAGS)
steelmill.run: steelmill FORCE
	./steelmill
steelmill_ex3:
	$(CCC) -o steelmill_ex3 $(CFLAGS) $(CPPEXDIR)/steelmill_ex3.cpp $(LDFLAGS)
steelmill_ex3.run: steelmill_ex3 FORCE
	./steelmill_ex3
talent:
	$(CCC) -o talent $(CFLAGS) $(CPPEXDIR)/talent.cpp $(LDFLAGS)
talent.run: talent FORCE
	./talent
teambuilding:
	$(CCC) -o teambuilding $(CFLAGS) $(CPPEXDIR)/teambuilding.cpp $(LDFLAGS)
teambuilding.run: teambuilding FORCE
	./teambuilding
truckfleet:
	$(CCC) -o truckfleet $(CFLAGS) $(CPPEXDIR)/truckfleet.cpp $(LDFLAGS)
truckfleet.run: truckfleet FORCE
	./truckfleet
execute_cpp: alloc.run color.run color_ex3.run color_ex4.run cpofileformat.run facility.run facility_ex3.run facility_ex4.run facility_explanations.run intro.run networkcfg.run plantlocation.run ppp.run sched_calendar.run sched_conflict.run sched_cumul.run sched_flowshop.run sched_goalprog.run sched_intro.run sched_jobshop.run sched_jobshopflex.run sched_learningeffect.run sched_openshop.run sched_optional.run sched_pflowshop.run sched_rcpsp.run sched_rcpspmm.run schedsearch_greedyandrelax.run schedsearch_optionalsettimes.run schedsearch_permutation.run schedsearch_sequencing.run schedsearch_settimes.run sched_sequence.run sched_setup.run sched_square.run sched_state.run sched_stochastic_jobshop.run sched_tcost.run sched_time.run searchphases.run smooth.run sports.run sports_strong.run steelmill.run steelmill_ex3.run talent.run teambuilding.run truckfleet.run
Alloc.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Alloc.java
Alloc.run: Alloc.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Alloc
Color.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Color.java
Color.run: Color.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Color
CPOFileFormat.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/CPOFileFormat.java
CPOFileFormat.run: CPOFileFormat.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. CPOFileFormat
FacilityExplanations.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/FacilityExplanations.java
FacilityExplanations.run: FacilityExplanations.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. FacilityExplanations
Facility.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Facility.java
Facility.run: Facility.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Facility
PlantLocation.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/PlantLocation.java
PlantLocation.run: PlantLocation.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. PlantLocation
Ppp.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Ppp.java
Ppp.run: Ppp.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Ppp
SchedCalendar.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedCalendar.java
SchedCalendar.run: SchedCalendar.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedCalendar
SchedConflict.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedConflict.java
SchedConflict.run: SchedConflict.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedConflict
SchedCumul.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedCumul.java
SchedCumul.run: SchedCumul.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedCumul
SchedFlowShop.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedFlowShop.java
SchedFlowShop.run: SchedFlowShop.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedFlowShop
SchedGoalProg.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedGoalProg.java
SchedGoalProg.run: SchedGoalProg.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedGoalProg
SchedIntro.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedIntro.java
SchedIntro.run: SchedIntro.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedIntro
SchedJobShopFlex.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedJobShopFlex.java
SchedJobShopFlex.run: SchedJobShopFlex.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedJobShopFlex
SchedJobShop.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedJobShop.java
SchedJobShop.run: SchedJobShop.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedJobShop
SchedLearningEffect.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedLearningEffect.java
SchedLearningEffect.run: SchedLearningEffect.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedLearningEffect
SchedOpenShop.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedOpenShop.java
SchedOpenShop.run: SchedOpenShop.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedOpenShop
SchedOptional.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedOptional.java
SchedOptional.run: SchedOptional.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedOptional
SchedPFlowShop.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedPFlowShop.java
SchedPFlowShop.run: SchedPFlowShop.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedPFlowShop
SchedRCPSP.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedRCPSP.java
SchedRCPSP.run: SchedRCPSP.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedRCPSP
SchedRCPSPMM.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedRCPSPMM.java
SchedRCPSPMM.run: SchedRCPSPMM.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedRCPSPMM
SchedSequence.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedSequence.java
SchedSequence.run: SchedSequence.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedSequence
SchedSetup.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedSetup.java
SchedSetup.run: SchedSetup.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedSetup
SchedSquare.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedSquare.java
SchedSquare.run: SchedSquare.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedSquare
SchedState.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedState.java
SchedState.run: SchedState.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedState
SchedStochasticJobShop.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedStochasticJobShop.java
SchedStochasticJobShop.run: SchedStochasticJobShop.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedStochasticJobShop
SchedTCost.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedTCost.java
SchedTCost.run: SchedTCost.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedTCost
SchedTime.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SchedTime.java
SchedTime.run: SchedTime.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SchedTime
SearchPhases.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SearchPhases.java
SearchPhases.run: SearchPhases.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SearchPhases
Sports.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Sports.java
Sports.run: Sports.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Sports
SportsStrong.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/SportsStrong.java
SportsStrong.run: SportsStrong.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. SportsStrong
Steelmill.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Steelmill.java
Steelmill.run: Steelmill.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Steelmill
Talent.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Talent.java
Talent.run: Talent.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Talent
Teambuilding.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Teambuilding.java
Teambuilding.run: Teambuilding.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Teambuilding
Truckfleet.class: $(JAVAC)
	$(JAVAC) -d . -classpath $(CPCLASSPATH) $(JAVAEXDIR)/Truckfleet.java
Truckfleet.run: Truckfleet.class FORCE
	LD_LIBRARY_PATH=$(LIBRARYPATH):$$LD_LIBRARY_PATH $(JAVA) $(JAVAFLAGS) -classpath $(CPCLASSPATH):. Truckfleet
execute_java: Alloc.run Color.run CPOFileFormat.run FacilityExplanations.run Facility.run PlantLocation.run Ppp.run SchedCalendar.run SchedConflict.run SchedCumul.run SchedFlowShop.run SchedGoalProg.run SchedIntro.run SchedJobShopFlex.run SchedJobShop.run SchedLearningEffect.run SchedOpenShop.run SchedOptional.run SchedPFlowShop.run SchedRCPSP.run SchedRCPSPMM.run SchedSequence.run SchedSetup.run SchedSquare.run SchedState.run SchedStochasticJobShop.run SchedTCost.run SchedTime.run SearchPhases.run Sports.run SportsStrong.run Steelmill.run Talent.run Teambuilding.run Truckfleet.run
