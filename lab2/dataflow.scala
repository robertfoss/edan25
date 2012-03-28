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
case class ActQueued();
case class ActDone();
case class Go();
case class GoContinue();
case class ChangeInBitSet(in: BitSet);	// might be useful...


class Controller(val cfg: Array[Vertex]) extends Actor {
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

      case ActQueued() => {
        runningActors += 1;
        //println("Controller: ActQueued()\trunningActors: " + runningActors);
        act();
      }
      case ActDone() => {
        runningActors -= 1;
        //println("Controller: ActDone()\taas: " + allActorsStarted + "\trunningActors: " + runningActors);
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

class Vertex(val index: Int, s: Int, val controller: Controller) extends Actor {
  var pred: List[Vertex]  = List();
  var succ: List[Vertex]  = List();
  var listed              = false;
  val uses                = new BitSet(s);
  val defs                = new BitSet(s);
  val out                 = new BitSet(s);
  var in                  = new BitSet(s);
  var old                 = new BitSet(s);
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
        //println(index + ": Start()");

        this ! CheckIfRepeat();

        act();
      }

        case CheckIfRepeat() => {
          //println(index + ": CheckIfRepeat()");
          if (!listed) {
              listed = true;
              controller ! ActQueued();
              this ! Go();
          }
          act();
        }

        case Go() => {
          //println(index + ": Go()");
          var v: Vertex = null;
          
          succ.foreach{ v =>
            outQuery += 1;
            v ! GetInBitSet(this);
          }
          act();
        }

        case GetInBitSet(v: Vertex) => {
          //println(index + ": GetInBitSet()");
          v ! GiveInBitSet(in);
          act();
        }

        case GiveInBitSet(in: BitSet) => {
          //println(index + ": GiveInBitSet()\tOutQuery: " + (outQuery-1));
          out.or(in);
          outQuery -= 1;
          if (outQuery == 0){
            this ! GoContinue();
            //println(index + ": Collected complete out bitset");
          }
          act();
        }

        case GoContinue() => {
          //println(index + ": GoContinue()");
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
          controller ! ActDone();
          act();
        }

      // Controller tells us to terminate, ie do not call act().
      case Stop()  => { }

    }
  }
}

object Driver {
  val rand    = new Random(1);
  var nactive = 0;
  var nsym    = 0;
  var nvertex = 0;
  var maxsucc = 0;

  def makeCFG(cfg: Array[Vertex], nsucc: Array[Int]) {

    cfg(0).connect(cfg(1));
    cfg(0).connect(cfg(2));

    for (i <- 2 until cfg.length) {
      val p = cfg(i);
      for (j <- 0 until nsucc(i)) {
        val s = cfg((rand.nextInt() % cfg.length).abs);
        p.connect(s);
      }
    }
  }

  def makeUseDef(cfg: Array[Vertex]) {
    for (i <- 0 until cfg.length) {
      for (j <- 0 until nactive) {
        val s = (rand.nextInt() % nsym).abs;
        if (j % 4 != 0) {
          if (!cfg(i).defs.get(s))
            cfg(i).uses.set(s);
        } else {
          if (!cfg(i).uses.get(s))
            cfg(i).defs.set(s);
        }
      }
    }
  }

  def main(args: Array[String]) {
    nsym           = args(0).toInt;
    nvertex        = args(1).toInt;
    maxsucc        = args(2).toInt;
    nactive        = args(3).toInt;
    var print	     = false;
    val cfg        = new Array[Vertex](nvertex);
    val nsucc      = new Array[Int](nvertex);
    val controller = new Controller(cfg);

    controller.start;

    for (i <- 0 until nvertex) {
      nsucc(i) = (rand.nextInt() % maxsucc).abs;
      cfg(i) = new Vertex(i, nsym, controller);
    }

    nsucc(0) = 2;
    nsucc(1) = 0;

    makeCFG(cfg, nsucc);
    makeUseDef(cfg);

    println("starting " + nvertex + " actors...");

    for (i <- 0 until nvertex)
      cfg(i).start;

    for (i <- 0 until nvertex)
      cfg(i) ! new Start;

    while (!controller.isStopped){
      Thread.sleep(500);
      //println("main: Controller not stopped.");
    }
    println("main: Controller stopped!½!");
  

    if (print)
      for (i <- 0 until nvertex)
        cfg(i).print();

    var sec = ((controller.end) - (controller.begin))/1000;
    var ms = ((controller.end-controller.begin)%1000);
    println("Runtime: " +sec + "." + ms); 
    }
}
