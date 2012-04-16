NSYM	= 100
NVERTEX	= 8
MAXSUCC	= 4
NACTIVE	= 10
NTHREADS = 4
PRINT=true

#SPATH=/usr/local/cs/scala/scala-2.9.1.final/bin/
SPATH=
all:
	@mkdir -p classes
	#scalac -g:vars -d classes dataflow.scala
	#scalac -deprecation -d classes dataflow.scala
	rm -f *.txt
	${SPATH}scalac -deprecation -optimise -d classes dataflow.scala
	${SPATH}scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) ${PRINT} > print_input.txt
	rm -f *.class
	rm -rf classes

	javac DataflowCorrect.java && java -Xmx1024M Dataflow $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(NTHREADS) ${PRINT} > jonasinpt.txt
	rm -f *.class
	diff jonasinpt.txt print_input > diff.txt
	

run:
	${SPATH}/scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE)
