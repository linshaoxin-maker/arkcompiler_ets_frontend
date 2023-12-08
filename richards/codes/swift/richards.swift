//
//  richards.swift
//  richards
//
//  Created by 李 on 2023/11/7.
//

import Glibc

let isDebug : Bool = false
func DeBugLog(_ msg : Any , file: String = #file, line: Int = #line, fn: String = #function){
    if isDebug {
        let prefix = "\(msg)"
        print(prefix)
    }
}

var ID_IDLE       = 0
var ID_WORKER     = 1
var ID_HANDLER_A  = 2
var ID_HANDLER_B  = 3
var ID_DEVICE_A   = 4
var ID_DEVICE_B   = 5
var NUMBER_OF_IDS = 6
var KIND_DEVICE   = 0
var KIND_WORK     = 1

/**
 * The task is running and is currently scheduled.
 */
var STATE_RUNNING = 0;

/**
 * The task has packets left to process.
 */
var STATE_RUNNABLE = 1;

/**
 * The task is not currently running.  The task is not blocked as such and may
 * be started by the scheduler.
 */
var STATE_SUSPENDED = 2;

/**
 * The task is blocked and cannot be run until it is explicitly released.
 */
var STATE_HELD = 4;

var STATE_SUSPENDED_RUNNABLE = STATE_SUSPENDED | STATE_RUNNABLE;
var STATE_NOT_HELD = ~STATE_HELD;
var DATA_SIZE = 4;
var COUNT = 1000;

/**
 * These two constants specify how many times a packet is queued and
 * how many times a task is put on hold in a correct run of richards.
 * They don't have any meaning a such but are characteristic of a
 * correct run so if the actual queue or hold count is different from
 * the expected there must be a bug in the implementation.
 **/
var EXPECTED_QUEUE_COUNT = 2322;
var EXPECTED_HOLD_COUNT = 928;

class Benchmark {
    
    /// @Generator
    /// Base classes for different task types
    class BaseTask {
        /// @State
        var scheduler: Scheduler
        init(scheduler: Scheduler) {
            self.scheduler = scheduler
        }
        func run(packet: Packet?) -> TaskControlBlock? {
            return TaskControlBlock(link: nil, id: 0, priority: 0, queue: nil, task: BaseTask.self)
        }
        func toString()-> String {
            return "BaseTask"
        }
    }
    
    class IdleTask: BaseTask {
        var v1: Int
        var count: Int
        
        /// An idle task doesn't do any work itself but cycles control between the two device tasks.
        /// - Parameters:
        ///   - scheduler: the scheduler that manages this task
        ///   - v1: a seed value that controls how the device tasks are scheduled
        ///   - count: the number of times this task should be scheduled
        init(scheduler: Scheduler, v1: Int, count: Int) {
            self.v1 = v1
            self.count = count
            super.init(scheduler: scheduler)
        }
        
        override func run(packet: Packet?) -> TaskControlBlock? {
            count -= 1
            if count == 0 {
                return scheduler.holdCurrent()
            }
            if (v1 & 1) == 0 {
                v1 = v1 >> 1
                return scheduler.release(id: ID_DEVICE_A)
            } else{
                v1 = (v1 >> 1) ^ 0xD008
                return scheduler.release(id: ID_DEVICE_B)
            }
        }
        
        override func toString()-> String {
            return "IdleTask"
        }
    }
    
    class WorkerTask: BaseTask {
        var v1: Int
        var v2: Int
        
        /// A task that manipulates work packets.
        /// - Parameters:
        ///   - scheduler: the scheduler that manages this task
        ///   - v1:  a seed used to specify how work packets are manipulated
        ///   - v2: another seed used to specify how work packets are manipulated
        init(scheduler: Scheduler, v1: Int, v2: Int) {
            self.v1 = v1
            self.v2 = v2
            super.init(scheduler: scheduler)
        }
        
        override func run(packet: Packet? = nil)-> TaskControlBlock {
            if packet == nil {
                return scheduler.suspendCurrent()
            } else {
                if v1 == ID_HANDLER_A {
                    v1 = ID_HANDLER_B
                } else {
                    v1 = ID_HANDLER_A
                }
                packet?.id = v1
                packet?.a1 = 0
                for i in 0..<DATA_SIZE {
                    v2 += 1
                    if v2 > 26 {
                        v2 = 1
                    }
                    packet?.a2[i] = v2
                }
                return scheduler.queue(packet: packet!)
            }
        }
        
        override func toString()-> String {
            return "WorkerTask"
        }
    }
    
    class HandlerTask: BaseTask {
        var v1: Packet?
        var v2: Packet?
        
        /// A task that manipulates work packets and then suspends itself.
        /// - Parameter packet: the scheduler that manages this task
        /// - Returns: TaskControlBlock
        override func run(packet: Packet?)-> TaskControlBlock {
            if packet != nil {
                if packet?.kind == KIND_WORK {
                    v1 = packet?.addTo(queue: v1)
                } else {
                    v2 = packet?.addTo(queue: v2)
                }
            }
            if v1 != nil {
                let count = v1?.a1
                var v = v2
                if count! < DATA_SIZE {
                    if v2 != nil {
                        v = v2
                        v2 = v2?.link
                        v?.a1 = (v1?.a2[count!])!
                        v1?.a1 = count! + 1
                        return scheduler.queue(packet: v!)
                    }
                } else {
                    v = v1
                    v1 = v1?.link
                    return scheduler.queue(packet: v!)
                }
            }
            return scheduler.suspendCurrent()
        }
        
        override func toString()-> String {
            return "HandlerTask"
        }
    }
    
    class DeviceTask: BaseTask {
        var v1: Packet?
        
        /// A task that suspends itself after each time it has been run to simulate waiting for data from an external device.
        /// - Parameter packet: the scheduler that manages this task
        /// - Returns: TaskControlBlock
        override func run(packet: Packet?)-> TaskControlBlock {
            if packet == nil {
                if v1 == nil {
                    return scheduler.suspendCurrent()
                }
                let v = v1
                v1 = nil
                return scheduler.queue(packet: v!)
            } else {
                v1 = packet
                return scheduler.holdCurrent()!
            }
        }
        
        override func toString()-> String {
            return "DeviceTask"
        }
    }
    
    class Packet {
        var link: Packet?
        var id: Int
        var kind: Int
        var a1: Int
        var a2 = [Int](repeating: 0, count: DATA_SIZE)
        
        /// A simple package of data that is manipulated by the tasks.  The exact layout
        /// of the payload data carried by a packet is not importaint, and neither is the
        /// nature of the work performed on packets by the tasks.
        /// - Parameters:
        ///   - link: the tail of the linked list of packets
        ///   - id: an ID for this packet
        ///   - kind: the type of this packet
        init(link: Packet? = nil, id: Int, kind: Int) {
            self.link = link
            self.id = id
            self.kind = kind
            self.a1 = 0
            self.a2 = [Int](repeating: 0, count: DATA_SIZE)
        }
        
        /// Add this packet to the end of a worklist, and return the worklist.
        /// - Parameter queue: the worklist to add this packet to
        /// - Returns: Packet
        func addTo(queue: Packet?)-> Packet {
            link = nil
            if queue == nil {
                return self
            }
            var peek = queue
            var next = queue
            while peek?.link != nil {
                next = peek?.link
                peek = next
            }
            next?.link = self
            return queue!
        }
        
        func toString()-> String {
            return "packet"
        }
    }
    
    class TaskControlBlock {
        var link: TaskControlBlock?
        var id: Int
        var priority: Int
        var queue: Packet?
        var task: BaseTask?
        var state: Int
        
        /// A task control block manages a task and the queue of work packages associated with it.
        /// - Parameters:
        ///   - link: the preceding block in the linked block list
        ///   - id: the id of this block
        ///   - priority: the priority of this block
        ///   - queue: the queue of packages to be processed by the task
        ///   - task: the task
        init(link: TaskControlBlock?, id: Int, priority: Int, queue: Packet?, task: Any) {
            self.link = link
            self.id = id
            self.priority = priority
            self.queue = queue
            self.task = (task as! Benchmark.BaseTask)
            if self.queue == nil {
                self.state = STATE_SUSPENDED
            } else {
                self.state = STATE_SUSPENDED_RUNNABLE
            }
        }
        
        func setRunning() {
            self.state = STATE_RUNNING
        }
        
        func markAsNotHeld() {
            self.state = self.state & STATE_NOT_HELD
        }
        
        func markAsHeld() {
            self.state = self.state | STATE_HELD
        }
        
        func isHeldOrSuspended() -> Bool {
            return ((self.state & STATE_HELD) != 0) || (self.state == STATE_SUSPENDED)
        }
        
        func markAsSuspended() {
            self.state = self.state | STATE_SUSPENDED
        }
        
        func markAsRunnable() {
            self.state = self.state | STATE_RUNNABLE
        }
        
        /// Runs this task, if it is ready to be run, and returns the next task to run.
        func run() -> TaskControlBlock? {
            var packet: Packet?
            if state == STATE_SUSPENDED_RUNNABLE {
                packet = queue
                queue = packet?.link
                if queue == nil {
                    state = STATE_RUNNING
                } else {
                    state = STATE_RUNNABLE
                }
            } else {
                packet = nil
            }
            return self.task?.run(packet: packet)
        }
        
        /// Adds a packet to the worklist of this block's task, marks this as runnable if
        /// necessary, and returns the next runnable object to run (the one
        /// with the highest priority).
        func checkPriorityAdd(_ task: TaskControlBlock,_ packet: Packet) -> TaskControlBlock {
            if queue == nil {
                queue = packet
                markAsRunnable()
                if priority > task.priority {
                    return self
                }
            } else {
                queue = packet.addTo(queue: queue)
            }
            return task
        }
        
        func toString()-> String {
            return "tcb { \((task?.toString())!) @ \(state) }"
        }
    }
    
    /// A scheduler can be used to schedule a set of tasks based on their relative
    /// priorities.  Scheduling is done by maintaining a list of task control blocks
    /// which holds tasks and the data queue they are processing.
    class Scheduler {
        var queueCount = 0
        var holdCount: Int = 0
        var list: TaskControlBlock?
        var currentTcb: TaskControlBlock?
        var blocks:[Int: TaskControlBlock] = [:]
        var currentId: Int?
        
        /// Add an idle task to this scheduler.
        /// - Parameters:
        ///   - id: the identity of the task
        ///   - priority: the task's priority
        ///   - queue: the queue of work to be processed by the task
        ///   - count: the number of times to schedule the task
        func addIdleTask(_ id: Int, _ priority: Int,_ queue: Packet?,_ count: Int) {
            addRunningTask(id, priority, queue, IdleTask(scheduler: self, v1: 1, count: count))
        }
        
        /// Add a work task to this scheduler.
        /// - Parameters:
        ///   - id: the identity of the task
        ///   - priority: the task's priority
        ///   - queue: the queue of work to be processed by the task
        func addWorkerTask(_ id: Int, _ priority: Int,_ queue: Packet) {
            addTask(id, priority, queue, WorkerTask(scheduler: self, v1: ID_HANDLER_A, v2: 0))
        }
        
        /// Add a handler task to this scheduler.
        /// - Parameters:
        ///   - id: the identity of the task
        ///   - priority: the task's priority
        ///   - queue: the queue of work to be processed by the task
        func addhandlerTask(_ id: Int, _ priority: Int, _ queue: Packet) {
            addTask(id, priority, queue, HandlerTask(scheduler: self))
        }
        
        /// Add a handler task to this scheduler.
        /// - Parameters:
        ///   - id: the identity of the task
        ///   - priority: the task's priority
        ///   - queue: the queue of work to be processed by the task
        func addDeviceTask(_ id: Int, _ priority: Int, _ queue: Packet?) {
            addTask(id, priority, queue, DeviceTask(scheduler: self))
        }
        
        /// Add the specified task and mark it as running.
        /// - Parameters:
        ///   - id: the identity of the task
        ///   - priority: the task's priority
        ///   - queue: the queue of work to be processed by the task
        ///   - task: the task to add
        func addRunningTask(_ id: Int, _ priority: Int, _ queue: Packet?, _ task: IdleTask) {
            addTask(id, priority, queue, task)
            currentTcb?.setRunning()
        }
        
        /// Add the specified task to this scheduler.
        /// - Parameters:
        ///   - id: the identity of the task
        ///   - priority: the task's priority
        ///   - queue: the queue of work to be processed by the task
        ///   - task: the task to add
        func addTask(_ id: Int, _ priority: Int,_ queue: Packet?,_ task: Any) {
            currentTcb = TaskControlBlock(link: list, id: id, priority: priority, queue: queue, task: task)
            list = currentTcb
            blocks[id] = self.currentTcb
        }
        
        /// Block the currently executing task and return the next task control block
        /// to run.  The blocked task will not be made runnable until it is explicitly
        /// released, even if new work is added to it.
        func holdCurrent() -> TaskControlBlock? {
            holdCount += 1
            currentTcb?.markAsHeld()
            return currentTcb?.link
        }
        
        /// Suspend the currently executing task and return the next task control block
        /// to run.  If new work is added to the suspended task it will be made runnable.
        func suspendCurrent() -> TaskControlBlock {
            currentTcb?.markAsSuspended()
            return currentTcb!
        }
        
        /// Add the specified packet to the end of the worklist used by the task
        /// associated with the packet and make the task runnable if it is currently suspended.
        /// - Parameter packet: the packet to add
        /// - Returns: TaskControlBlock
        func queue(packet: Packet) -> TaskControlBlock {
            let t = blocks[packet.id]
            if t == nil {
                return t!
            }
            queueCount += 1
            packet.link = nil
            packet.id = currentId!
            return (t?.checkPriorityAdd(currentTcb!, packet))!
        }
        
        /// Execute the tasks managed by this scheduler.
        func schedule() {
            currentTcb = list!
            while (currentTcb != nil) {
                DeBugLog(currentTcb!.toString())
                if (currentTcb!.isHeldOrSuspended()) {
                    currentTcb = currentTcb?.link!
                } else {
                    currentId = currentTcb!.id
                    self.currentTcb = currentTcb?.run()
                }
            }
        }
        
        /// Release a task that is currently blocked and return the next block to run.
        /// - Parameter id: the id of the task to suspend
        /// - Returns: TaskControlBlock
        func release(id: Int) -> TaskControlBlock {
            let tcb = blocks[id]
            if tcb == nil {
                return tcb!
            }
            tcb?.markAsNotHeld()
            let tcbF: Int = tcb!.priority
            let tcbC: Int = currentTcb!.priority
            if tcbF > tcbC {
                return tcb!
            } else {
                return currentTcb!
            }
        }
    }
    
    /// The Richards benchmark simulates the task dispatcher of an operating system.
    func runRichards() {
        let scheduler = Scheduler()
        scheduler.addIdleTask(ID_IDLE, 0, nil, COUNT)

        var queue = Packet(link: nil, id: ID_WORKER, kind: KIND_WORK)
        queue = Packet(link: queue, id: ID_WORKER, kind: KIND_WORK)
        scheduler.addWorkerTask(KIND_WORK, 1000, queue)
        
        queue = Packet(link: nil, id: ID_DEVICE_A, kind: KIND_DEVICE)
        queue = Packet(link: queue, id: ID_DEVICE_A, kind: KIND_DEVICE)
        queue = Packet(link: queue, id: ID_DEVICE_A, kind: KIND_DEVICE)
        scheduler.addhandlerTask(ID_HANDLER_A, 2000, queue)
        
        queue = Packet(link: nil, id: ID_DEVICE_B, kind: KIND_DEVICE)
        queue = Packet(link: queue, id: ID_DEVICE_B, kind: KIND_DEVICE)
        queue = Packet(link: queue, id: ID_DEVICE_B, kind: KIND_DEVICE)
        scheduler.addhandlerTask(ID_HANDLER_B, 3000, queue)
        
        scheduler.addDeviceTask(ID_DEVICE_A, 4000, nil)
        
        scheduler.addDeviceTask(ID_DEVICE_B, 5000, nil)
        
        scheduler.schedule()
        
        if (scheduler.queueCount != EXPECTED_QUEUE_COUNT ||
            scheduler.holdCount != EXPECTED_HOLD_COUNT) {
            let msg =
            "Error during execution: queueCount = \(scheduler.queueCount) , holdCount = \(scheduler.holdCount).";
            DeBugLog(msg)
        }
        DeBugLog("queueCount = \(scheduler.queueCount) ; holdCount = \(scheduler.holdCount) ")
    }
    
    /// @Benchmark
    func runIteration() {
        for _ in 0..<50 {
            runRichards()
        }
    }
}

class Timer {
    private let CLOCK_REALTIME = 0
    private var time_spec = timespec()

    func getTime() -> Double {
        clock_gettime(Int32(CLOCK_REALTIME),&time_spec)
        return Double(time_spec.tv_sec * 1_000_000 + time_spec.tv_nsec / 1_000)
    }
}

func runIterationTime() {
    let start = Timer().getTime()
    let richard = Benchmark()
    for _ in 0..<120 {
        richard.runIteration()
    }
    let end = Timer().getTime()
    let duration = (end - start) / 1000
    print("richards: ms = \(duration)")
}
runIterationTime()
