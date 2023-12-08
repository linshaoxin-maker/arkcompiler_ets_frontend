let ID_IDLE = 0
let ID_WORKER = 1
let ID_HANDLER_A = 2
let ID_HANDLER_B = 3
let ID_DEVICE_A = 4
let ID_DEVICE_B = 5
let number_OF_IDS = 6

let KIND_DEVICE = 0
let KIND_WORK = 1

/**
 * The task is running and is currently scheduled.
 */
let STATE_RUNNING = 0;

/**
 * The task has packets left to process.
 */
let STATE_RUNNABLE = 1;

/**
 * The task is not currently running.  The task is not blocked as such and may
 * be started by the scheduler.
 */
let STATE_SUSPENDED = 2;

/**
 * The task is blocked and cannot be run until it is explicitly released.
 */
let STATE_HELD = 4;

let STATE_SUSPENDED_RUNNABLE = STATE_SUSPENDED | STATE_RUNNABLE;
let STATE_NOT_HELD = ~STATE_HELD;
let DATA_SIZE = 4;
let COUNT = 1000;

/**
 * These two constants specify how many times a packet is queued and
 * how many times a task is put on hold in a correct run of richards.
 * They don't have any meaning a such but are characteristic of a
 * correct run so if the actual queue or hold count is different from
 * the expected there must be a bug in the implementation.
 **/
let EXPECTED_QUEUE_COUNT = 2322;
let EXPECTED_HOLD_COUNT = 928;
//Generator
// Base classes for different task type
class BaseTask {
  // @State
  scheduler: Scheduler

  constructor(scheduler: Scheduler) {
    this.scheduler = scheduler
  }

  run(packet: Packet | null): TaskControlBlock {
    return new TaskControlBlock(null, 0, 0, packet, null)
  }

  toString(): String {
    return "BaseTask"
  }
}

class IdleTask extends BaseTask {
  v1: number
  count: number
  /**
   * An idle task doesn't do any work itself but cycles control between the two
   * device tasks.
   * @param {Scheduler} scheduler the scheduler that manages this task
   * @param {number} v1 a seed value that controls how the device tasks are scheduled
   * @param {number} count the number of times this task should be scheduled
   * @constructor
   */
  constructor(scheduler: Scheduler, v1: number, count: number) {
    super(scheduler)
    this.v1 = v1
    this.count = count
  }
  run(): TaskControlBlock {
    this.count -= 1
    if (this.count == 0) {
      return this.scheduler.holdCurrent()
    }
    if ((this.v1 & 1) == 0) {
      this.v1 = this.v1 >> 1
      return this.scheduler.release(ID_DEVICE_A)
    } else {
      this.v1 = (this.v1 >> 1) ^ 0xD008
      return this.scheduler.release(ID_DEVICE_B)
    }
  }

  toString(): String {
    return "IdleTask"
  }
}

class WorkerTask extends BaseTask {
  v1: number
  v2: number
  /**
   * A task that manipulates work packets.
   * @param {Scheduler} scheduler the scheduler that manages this task
   * @param {number} v1 a seed used to specify how work packets are manipulated
   * @param {number} v2 another seed used to specify how work packets are manipulated
   * @constructor
   */
  constructor(scheduler: Scheduler, v1: number, v2: number) {
    super(scheduler)
    this.v1 = v1
    this.v2 = v2
  }

  run(packet: Packet ): TaskControlBlock {
    if (packet == null) {
      return this.scheduler.suspendCurrent()
    } else {
      if (this.v1 == ID_HANDLER_A) {
        this.v1 = ID_HANDLER_B
      } else {
        this.v1 = ID_HANDLER_A
      }
      packet.id = this.v1
      packet.a1 = 0
      for (let i: number = 0; i < DATA_SIZE; i++) {
        this.v2 += 1
        if (this.v2 > 26) {
          this.v2 = 1
        }
        packet.a2[i] = this.v2
      }
      return this.scheduler.queue(packet)
    }

  }

  toString(): String {
    return "WorkerTask"
  }
}

class HandlerTask extends BaseTask {
  // scheduler: Scheduler
  v1: Packet | null=null
  v2: Packet | null=null
  constructor(scheduler:Scheduler) {
    // this.scheduler=scheduler
    super(scheduler)
  }
  /**
   * A task that manipulates work packets and then suspends itself.
   * @param {Scheduler} scheduler the scheduler that manages this task
   * @constructor
   */
  run(packet: Packet | null): TaskControlBlock {
    if (packet != null) {
      if (packet.kind == KIND_WORK) {
        this.v1 = packet.addTo(this.v1)
      } else {
        this.v2 = packet.addTo(this.v2)
      }
    }
    if (this.v1 != null) {
      let count = this.v1?.a1
      let v = this.v2
      if (count < DATA_SIZE) {
        if (this.v2 != null) {
          v = this.v2
          this.v2 = this.v2?.link
          v.a1 = (this.v1?.a2[count])
          this.v1.a1 = count + 1
          return this.scheduler.queue(v)
        }
      } else {
        v = this.v1
        this.v1 = this.v1?.link
        return this.scheduler.queue(v)
      }
    }
    return this.scheduler.suspendCurrent()
  }

  toString(): String {
    return "HandlerTask"
  }
}

/**
 * A task that suspends itself after each time it has been run to simulate
 * waiting for data from an external device.
 * @param {Scheduler} scheduler the scheduler that manages this task
 * @constructor
 */
//
class DeviceTask extends BaseTask{
  v1: Packet | null=null;
  // scheduler: Scheduler;

  constructor(scheduler:Scheduler) {
    super(scheduler)
  }
  /**
   * Runs this task, if it is ready to be run, and returns the next task to run.
   */
  run(packet: Packet | null): TaskControlBlock {
    if (packet == null) {
      if (this.v1 == null) {
        return this.scheduler.suspendCurrent()
      }
      let v = this.v1
      this.v1 = null
      return this.scheduler.queue(v)
    } else {
      this.v1 = packet
      return this.scheduler.holdCurrent()
    }
  }

  toString(): String {
    return "DeviceTask"
  }
}

class Packet {
  link: Packet | null;
  id: number
  kind: number
  a1: number=0
  a2: Array<number> = new Array(DATA_SIZE)



  /**
   * A simple package of data that is manipulated by the tasks.  The exact layout
   * of the payload data carried by a packet is not importaint, and neither is the
   * nature of the work performed on packets by the tasks.
   *
   * Besides carrying data, packets form linked lists and are hence used both as
   * data and worklists.
   * @param {Packet} link the tail of the linked list of packets
   * @param {number} id an ID for this packet
   * @param {number} kind the type of this packet
   * @constructor
   */
  constructor(link: Packet | null, id: number, kind: number,) {
    this.link = link;
    this.kind = kind;
    this.id = id;
  }

  /**
   * Add this packet to the end of a worklist, and return the worklist.
   * @param {Packet} queue the worklist to add this packet to
   */
  addTo(queue:Packet|null): Packet {
    this.link = null;
    if (queue == null) {
      return this;
    }
    let peek:Packet|null, next = queue;
    while ((peek = next.link) != null) {
      next = peek;
    }
    next.link = this;
    return queue;
  };

  toString(): String {
    return "packet"
  }
}

class TaskControlBlock {
  link: TaskControlBlock | null;
  id: number;
  priority: number;
  queue: Packet | null;
  task: BaseTask | null;
  state: number;

  /**
   * A task control block manages a task and the queue of work packages associated
   * with it.
   * @param {TaskControlBlock} link the preceding block in the linked block list
   * @param {number} id the id of this block
   * @param {number} priority the priority of this block
   * @param {Packet} queue the queue of packages to be processed by the task
   * @param {Task} task the task
   * @constructor
   */

  constructor(link: TaskControlBlock | null = null, id: number, priority: number, queue: Packet | null, task: BaseTask | null) {
    this.link = link
    this.id = id
    this.priority = priority
    this.queue = queue
    this.task = task
    if (this.queue == null) {
      this.state = 2
    } else {
      this.state = 2 | 1
    }
  }

  setRunning() {
    this.state = STATE_RUNNING
  }

  markAsNotHeld() {
    this.state = this.state & STATE_NOT_HELD
  }

  markAsHeld() {
    this.state = this.state | STATE_HELD
  }

  isHeldOrSuspended(): boolean {
    return ((this.state & STATE_HELD) != 0) || (this.state == STATE_SUSPENDED)
  }

  markAsSuspended() {
    this.state = this.state | STATE_SUSPENDED
  }

  markAsRunnable() {
    this.state = this.state | STATE_RUNNABLE
  }
  /**
   * Runs this task, if it is ready to be run, and returns the next task to run.
   */
  run(): TaskControlBlock | null {
    let packet: Packet | null
    if (this.state == STATE_SUSPENDED_RUNNABLE) {
      packet = this.queue
      this.queue = packet!.link
      if (this.queue == null) {
        this.state = STATE_RUNNING
      } else {
        this.state = STATE_RUNNABLE
      }
    } else {
      packet = null
    }
    return this.task!.run(packet)
  }
  /**
   * Adds a packet to the worklist of this block's task, marks this as runnable if
   * necessary, and returns the next runnable object to run (the one
   * with the highest priority).
   */
  checkPriorityAdd(task: TaskControlBlock, packet: Packet): TaskControlBlock {
    if (this.queue == null) {
      this.queue = packet
      this.markAsRunnable()
      if (this.priority > task.priority) {
        return this
      }
    } else {
      this.queue = packet.addTo(this.queue)
    }
    return task
  }

  toString(): String {
    let d: String = ''
    if (this.task) {
      d = `tcb {  (${String(this.task.toString())}) @ \(${String(this.state)}) }`
    }
    return d
  }
}

class Scheduler {
  queueCount: number
  holdCount: number
  list: TaskControlBlock | null
  currentTcb: TaskControlBlock | null
  blocks: Array<TaskControlBlock>
  currentId: number

  constructor(queueCount: number, holdCount: number, currentId: number, list: TaskControlBlock | null, currentTcb: TaskControlBlock | null, blocks: Array<TaskControlBlock>) {
    this.queueCount = queueCount | 0
    this.holdCount = holdCount | 0
    this.currentId = currentId
    this.list = list
    this.currentTcb = currentTcb
    this.blocks = blocks
  }
  /**
   * Add an idle task to this scheduler.
   * @param {number} id the identity of the task
   * @param {number} priority the task's priority
   * @param {Packet} queue the queue of work to be processed by the task
   * @param {number} count the number of times to schedule the task
   */
  addIdleTask(id: number, priority: number, queue: Packet | null , count: number) {
    this.addRunningTask(id, priority, queue, new IdleTask(this, 1, count))
  }
  /**
   * Add a work task to this scheduler.
   * @param {number} id the identity of the task
   * @param {number} priority the task's priority
   * @param {Packet} queue the queue of work to be processed by the task
   */
  addWorkerTask(id: number, priority: number, queue: Packet | null) {
    this.addTask(id, priority, queue, new WorkerTask(this, ID_HANDLER_A, 0))
  }

  /**
   * Add a handler task to this scheduler.
   * @param {number} id the identity of the task
   * @param {number} priority the task's priority
   * @param {Packet} queue the queue of work to be processed by the task
   */
  addHandlerTask(id: number, priority: number, queue: Packet | null) {
    this.addTask(id, priority, queue, new HandlerTask(this))
  }
  /**
   * Add a handler task to this scheduler.
   * @param {number} id the identity of the task
   * @param {number} priority the task's priority
   * @param {Packet} queue the queue of work to be processed by the task
   */
  addDeviceTask(id: number, priority: number, queue: Packet | null) {
    this.addTask(id, priority, queue, new DeviceTask(this))
  }
  /**
   * Add the specified task and mark it as running.
   * @param {number} id the identity of the task
   * @param {number} priority the task's priority
   * @param {Packet} queue the queue of work to be processed by the task
   * @param {Task} task the task to add
   */
  addRunningTask(id: number, priority: number, queue: Packet|null , task: IdleTask) {
    this.addTask(id, priority, queue, task)
    this.currentTcb!.setRunning()
  }
  /**
   * Add the specified task to this scheduler.
   * @param {number} id the identity of the task
   * @param {number} priority the task's priority
   * @param {Packet} queue the queue of work to be processed by the task
   * @param {Task} task the task to add
   */
  addTask(id: number, priority: number, queue: Packet | null, task: BaseTask) {
    this.currentTcb = new TaskControlBlock(this.list, id, priority, queue, task)
    this.list = this.currentTcb
    this.blocks[id] = this.currentTcb
  }

  /**
   * Block the currently executing task and return the next task control block
   * to run.  The blocked task will not be made runnable until it is explicitly
   * released, even if new work is added to it.
   */
  holdCurrent(): TaskControlBlock {
    this.holdCount++
    this.currentTcb!.markAsHeld()
    return this.currentTcb!.link!
  }
  /**
   * Suspend the currently executing task and return the next task control block
   * to run.  If new work is added to the suspended task it will be made runnable.
   */
  suspendCurrent(): TaskControlBlock {
    this.currentTcb!.markAsSuspended()
    return this.currentTcb!
  }
  /**
   * Add the specified packet to the end of the worklist used by the task
   * associated with the packet and make the task runnable if it is currently
   * suspended.
   * @param {Packet} packet the packet to add
   */
  queue(packet: Packet): TaskControlBlock {
    let t = this.blocks[packet.id]
    if (t == null) {
      return t
    }
    this.queueCount++
    packet.link = null
    packet.id = this.currentId
    return t.checkPriorityAdd(this.currentTcb!, packet!)
  }
  /**
   * Execute the tasks managed by this scheduler.
   */
  schedule() {
    this.currentTcb = this.list
    while (this.currentTcb != null) {
      if(debug){
        log(`${this.currentTcb.toString()}`)
      }
      if (this.currentTcb.isHeldOrSuspended()) {
        this.currentTcb = this.currentTcb?.link
      } else {
        this.currentId = this.currentTcb.id
        this.currentTcb = this.currentTcb.run()
      }
    }
  }
  /**
   * Release a task that is currently blocked and return the next block to run.
   * @param {int} id the id of the task to suspend
   */
  release(id: number): TaskControlBlock {
    let tcb = this.blocks[id]
    if (tcb == null) {
      return tcb
    }
    tcb?.markAsNotHeld()
    if (tcb.priority > this.currentTcb!.priority) {
      return tcb;
    } else {
      return this.currentTcb!;
    }
  }
}

class Benchmark {
  // @Benchmark
  runIteration() {
    for (let i = 0; i < 50; ++i)
      runRichards();
  }
}
function runRichards() {
  let scheduler = new Scheduler(0, 0, 0, null, null, new Array(number_OF_IDS))
  scheduler.addIdleTask(ID_IDLE, 0, null, COUNT)

  let queue = new Packet(null, ID_WORKER, KIND_WORK)
  queue = new Packet(queue, ID_WORKER, KIND_WORK)
  scheduler.addWorkerTask(KIND_WORK, 1000, queue)

  queue = new Packet(null, ID_DEVICE_A, KIND_DEVICE)
  queue = new Packet(queue, ID_DEVICE_A, KIND_DEVICE)
  queue = new Packet(queue, ID_DEVICE_A, KIND_DEVICE)
  scheduler.addHandlerTask(ID_HANDLER_A, 2000, queue)

  queue = new Packet(null, ID_DEVICE_B, KIND_DEVICE)
  queue = new Packet(queue, ID_DEVICE_B, KIND_DEVICE)
  queue = new Packet(queue, ID_DEVICE_B, KIND_DEVICE)
  scheduler.addHandlerTask(ID_HANDLER_B, 3000, queue)

  scheduler.addDeviceTask(ID_DEVICE_A, 4000, null)

  scheduler.addDeviceTask(ID_DEVICE_B, 5000, null)

  scheduler.schedule()

  if (scheduler.queueCount != EXPECTED_QUEUE_COUNT ||

    scheduler.holdCount != EXPECTED_HOLD_COUNT) {
    let msg = `Error during execution: queueCount = \(${scheduler.queueCount}) , holdCount = \(${scheduler.holdCount}).`;
    throw new Error(msg)
  }
  else {
    log(`queueCount : ${scheduler.queueCount} ; holdCount : ${scheduler.holdCount}`)
  }
}

let debug:boolean=false
function log(msg:string):void{
  if(debug){
    print(msg)
  }
}
function  runIterationTime(){
  let start=new Date().getTime()
  for (let j = 0; j < 120; j++) {
    for (let i = 0; i < 50; i++){
      runRichards();
    }
  }
  let end=new Date().getTime()
  print(`richards: ms = ${end-start}`)
}
runIterationTime()
