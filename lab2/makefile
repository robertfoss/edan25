NSYM	= 100
NVERTEX	= 8
MAXSUCC	= 4
NACTIVE	= 10

SPATH = /usr/local/cs/scala/scala-2.9.1.final/bin

all:
	@mkdir -p classes
	#scalac -g:vars -d classes dataflow.scala
	#scalac -deprecation -d classes dataflow.scala
	${SPATH}/scalac -deprecation -optimise -d classes dataflow.scala
	${SPATH}/scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) > print_input
	#javac ../lab1/DataflowJonas.java && java -Xmx1024M Dataflow $S $V $U $A $T $P > jonassmall.txt
	#diff output jonas.txt

run:
	${SPATH}/scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE)
