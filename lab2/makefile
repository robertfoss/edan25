NSYM	= 10000
NVERTEX	= 1000
MAXSUCC	= 4
NACTIVE	= 100

SPATH = /usr/local/cs/scala/scala-2.9.1.final/bin

all:
	@mkdir -p classes
	#scalac -g:vars -d classes dataflow.scala
	#scalac -deprecation -d classes dataflow.scala
	${SPATH}/scalac -deprecation -optimise -d classes dataflow.scala
	${SPATH}/scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE)

run:
	${SPATH}/scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE)
