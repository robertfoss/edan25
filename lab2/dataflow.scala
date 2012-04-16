import scala.actors._
import scala.actors.Actor._
import scala.util._
import java.util.BitSet;
//import java.util.Thread;
// LAB 6: some case classes but you need additional ones too.
case class Start();
case class Stop();
case class Ready();
case class CheckIfRepeat();
case class GetInBitSet(v: Vertex);
case class GiveInBitSet(in: BitSet);
case class ActQueued(index: Int);
case class ActDone(index: Int);
case class Go();
case class GoContinue();
case class ChangeInBitSet(in: BitSet);	// might be useful...


class Controller(val cfg: Array[Vertex], print_debug: Boolean) extends Actor {
  var started = 0;
  var begin   = System.currentTimeMillis();
  var runningActors = 0;
  var allActorsStarted = false;
  var isStopped = false;
  var end = 0L;

  def act() {
    react {
      case Ready() => {
        started += 1;
        if (started == cfg.length) {
          for (u <- cfg)
            u ! new Go;
        }
        allActorsStarted = true;
        act();
      }

      case ActQueued(index: Int) => {
        runningActors += 1;
        if (print_debug) println(index + ": Controller: ActQueued()\trunningActors: " + runningActors);
        act();
      }
      case ActDone(index: Int) => {
        runningActors -= 1;
        if (print_debug) println(index + ": Controller: ActDone()\taas: " + allActorsStarted + "\trunningActors: " + runningActors);
        if (runningActors == 0 && allActorsStarted){
          for (u <- cfg){
              u ! new Stop;
          }
          end = System.currentTimeMillis();
          isStopped = true;
        } else {
          act();
        }
      }
    }
  } 
}

class Vertex(val index: Int, s: Int, val controller: Controller, print_debug: Boolean) extends Actor {
  var pred: List[Vertex]  = List();
  var succ: List[Vertex]  = List();
  var listed              = false;
  val uses                = new BitSet(s);
  val defs                = new BitSet(s);
  val out                 = new BitSet(s);
  var in                  = new BitSet(s);
  var old                 = new BitSet(s);
  var debug = print_debug;
  var outQuery = 0;

  def connect(that: Vertex)
  {
    //println(this.index + "->" + that.index);
    this.succ = that :: this.succ;
    that.pred = this :: that.pred;
  }

  def print() {  
    var i = 0;

    System.out.print("use[" + index + "] = { ");
    for (i <- 0 until uses.size())
      if (uses.get(i))
        System.out.print("" + i + " ");
    System.out.println("}");
    System.out.print("def[" + index + "] = { ");
    for (i <- 0 until defs.size())
      if (defs.get(i))
        System.out.print("" + i + " ");
    System.out.println("}\n");

    System.out.print("in[" + index + "] = { ");
    for (i <- 0 until in.size())
      if (in.get(i))
        System.out.print("" + i + " ");
    System.out.println("}");

    System.out.print("out[" + index + "] = { ");
    for (i <- 0 until out.size())
      if (out.get(i))
        System.out.print("" + i + " ");
    System.out.println("}\n");
  }

  def act() {
    react {
      case Start() => {
        // Tell controller we are ready to go.
        controller ! new Ready;
        if(debug) println(index + ": Start()");

        this ! CheckIfRepeat();

        act();
      }

        case CheckIfRepeat() => {
          if(debug) println(index + ": CheckIfRepeat()");
          if (!listed) {
              listed = true;
              controller ! ActQueued(index);
              this ! Go();
          }
          act();
        }

        case Go() => {
          if(debug) println(index + ": Go()");
          var v: Vertex = null;
          
          succ.foreach{ v =>
            outQuery += 1;
            v ! GetInBitSet(this);
          }
          if(succ.length == 0){
		        if(debug) println(index + ": Go(), No successors.");
            outQuery = 1;
            self ! GiveInBitSet(new BitSet());
          }
          act();
        }

        case GetInBitSet(v: Vertex) => {
          if(debug) println(index + ": GetInBitSet()");
          v ! GiveInBitSet(in);
          act();
        }

        case GiveInBitSet(in: BitSet) => {
          if(debug) println(index + ": GiveInBitSet()\tOutQuery: " + (outQuery-1));
          out.or(in);
          outQuery -= 1;
          if (outQuery == 0){
            this ! GoContinue();
            //println(index + ": Collected complete out bitset");
          }
          act();
        }

        case GoContinue() => {
          if(debug) println(index + ": GoContinue()");
          old = in;


          // in = use U (out - def)
          in = new BitSet();
          in.or(out);
          in.andNot(defs);
          in.or(uses);

          if (!in.equals(old)) {
            //println(index + ": GoContinue()\tin != old");
            pred.foreach{ v =>
                v ! CheckIfRepeat();
            }
          }
          //println(index + ": GoContinue()\tforeach done..");
          listed = false;
          controller ! ActDone(index);
          act();
        }

      // Controller tells us to terminate, ie do not call act().
      case Stop()  => { }

    }
  }
}

class Randoms(val seed: Int){
	var w = seed + 1;
	var z = seed * seed + seed + 2;

	def nextInt() = {
		z = 36969 * (z & 65535) + (z >> 16);
		w = 18000 * (w & 65535) + (w >> 16);

		((z << 16) + w);
	}
}


object Driver {
  val rand    = new Randoms(1);
  var nactive = 0;
  var nsym    = 0;
  var nvertex = 0;
  var maxsucc = 0;

  def makeCFG(cfg: Array[Vertex], maxsucc: Int, print_input: Boolean) {
  	cfg(0).connect(cfg(1));
    cfg(0).connect(cfg(2));

    for (i <- 2 until cfg.length) {
      if(print_input) print("[" + i + "] succ = {");
      val p = cfg(i);
      val q = (rand.nextInt() % maxsucc) + 1;
      for (j <- 0 until q) {
        val num = (rand.nextInt() % cfg.length).abs;
        val s = cfg(num);
        if(print_input) print(" " + num);
        p.connect(s);
      }
      if(print_input) print("}\n");
    }
  }

  def makeUseDef(cfg: Array[Vertex], print_input: Boolean) {
    for (i <- 0 until cfg.length) {
      if(print_input) print("[" + i + "] usedef = {");
      for (j <- 0 until nactive) {
        val s = (rand.nextInt() % nsym).abs;
        if (j % 4 != 0) {
          if (!cfg(i).defs.get(s)){
            if(print_input) print(" u " + s);
            cfg(i).uses.set(s);
          }
        } else {
          if (!cfg(i).uses.get(s)){
            if(print_input) print(" d " + s);
            cfg(i).defs.set(s);
          }
        }
      }
      if(print_input) print("}\n");
    }
  }

  def main(args: Array[String]) {
    nsym           	= args(0).toInt;
    nvertex        	= args(1).toInt;
    maxsucc        	= args(2).toInt;
    nactive        	= args(3).toInt;
    var print_output= args(4).toBoolean;
    var print_input	= args(5).toBoolean;
    var print_debug	= args(6).toBoolean;
    val cfg        	= new Array[Vertex](nvertex);
    //val nsucc       = new Array[Int](nvertex);
    val controller 	= new Controller(cfg, print_debug);

    controller.start;

    for (i <- 0 until nvertex) {
      //nsucc(i) = (rand.nextInt() % maxsucc).abs;
      cfg(i) = new Vertex(i, nsym, controller, print_debug);
    }

    //nsucc(0) = 2;
    //nsucc(1) = 0;

    makeCFG(cfg, maxsucc, print_input);
    makeUseDef(cfg, print_input);

    //println("starting " + nvertex + " actors...");

    for (i <- 0 until nvertex)
      cfg(i).start;

    for (i <- 0 until nvertex)
      cfg(i) ! new Start;

    while (!controller.isStopped){
      Thread.sleep(500);
      //println("main: Controller not stopped.");
    }
    //println("main: Controller stopped!½!");
  

    if (print_output){
      for (i <- 0 until nvertex)
        cfg(i).print();

      var sec = ((controller.end) - (controller.begin))/1000;
      var ms = ((controller.end-controller.begin)%1000);
      println("Runtime: " +sec + "." + ms);
      }
    }
}
