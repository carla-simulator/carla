# TimerManager -- Callback-Based Event Handling {#timer_manager}

[TOC]

In some places, OMNeT++ provides mechanisms to react to some sort of event, that are harder to use than they should.
It is perfectly possible to implement the reaction to the event, but it involves a lot of boilerplate code and no real abstraction.

One example for this are self messages, which are often used to implement timers (one shot or recurring) for individual modules.
To implement a single timer, one typically has to:

- define a class member to contain the pointer to the cMessage instance.
- create the cMessage instance
- schedule the cMessage instance
- detect the arrival of that particular cMessage, usually in some if-cascade in or called by handleMessage
- perform the actual action
- possibly re-schedule the message, if the event is recurring
- delete the cMessage when done and/or the object gets destroyed

```{.cpp}
// Example module with a one-shot timer firing 2.5 seconds after initialize and a recurring timer firing every 1 second
class WitoutTimers: public cSimpleModule {
protected:
    // define class member to contain the pointer to the cMessage instance
    cMessage* recurringTrigger;
    cMessage* oneshotTrigger;
    void initialize() override {
        // create the cMessage instance
        recurringTrigger = new cMessage("recurring trigger");
        oneshotTrigger = new cMessage("one-shot trigger");
        // schedule the cMessage instance
        scheduleAt(1, recurringTrigger);
        scheduleAt(2.5, oneshotTrigger);
        // ...
    }
    void handleMessage(cMessage* msg) override {
        if (msg->isSelfMessage()) {
            // detect the arrival of a particular cMessage
            if (msg == recurringTrigger) {
                // perform actual action
                std::cerr << "recurring Timer called from " << this->getFullPath() << std::endl;
                // re-schedule the message
                scheduleAt(1, recurringTrigger);
            }
            if (msg == oneshotTrigger) {
                std::cerr << "one-shot timer fired from " << this->getFullPath() << " at " << simTime() << std::endl;
            }
        }
    }
public:
    virtual ~WithoutTimers() {
        // delete the cMessage when the object gets destroyed
        if(recurringTrigger) {
            cancelAndDelete(recurringTrigger);
            recurringTrigger = nullptr;
        }
        if(oneshotTrigger) {
            cancelAndDelete(oneshotTrigger);
            oneshotTrigger = nullptr;
        }
    }
};
```

This is highly repetitive and scatters code all over the class, which is bad.
Furthermore, it has to be repeated for every single timer.

With the [TimerManager] suite, most of the boilerplate is abstracted away.
The [TimerManager](@ref veins::TimerManager) contains all of it and provides an interface with clear semantics.
Now the only steps left are:

- per-module setup of the [TimerManager] instance (only necessary once per module):
    - define and instantiate the [TimerManager]
    - forward all self-messages to the [TimerManager] instance (in handleMessage)
- define the individual timers:
  - encode the reaction to the timer firing in a functor, e.g., a lambda
  - specify when and how often a timer shall fire by constructing a [TimerSpecification]
  - register the timer with the [TimerManager] instance

```{.cpp}
// Example module with same functionalty as above
class WithTimers: public cSimpleModule {
protected:
    veins::TimerManager timerManager(this);  // define and instantiate the TimerManager
    void handleMessage(cMessage* msg) override {
        if (msg->isSelfMessage()) {
            timerManager.handleMessage(msg);  // forward all self-messages to the TimerManager instance
        }
        // ...
    }
    void initialize() override {
        // encode the reaction to the timer firing with a lambda
        auto recurringCallback = [this](){
            std::cerr << "recurring Timer called from " << this->getFullPath() << std::endl;
        };
        auto oneshotCallback = [this](){
            std::cerr << "one-shot timer fired from " << this->getFullPath() << " at " << simTime() << std::endl;
        };
        // specify when and how ofthen a timer shall fire
        auto recurringTimerSpec = veins::TimerSpecification(recurringCallback).interval(1);
        auto oneshotTimerSpec = veins::TimerSpecification(oneshotCallback).oneshotin(2.5);
        // register the timer with the TimerManager instance
        timerManager.create(recurringTimerSpec, "recurring timer");
        timerManager.create(oneshotTimerSpec, "one-shot timer");
    }
};
```

Note: the [TimerManager] works fine with self-messages not handled by it.
The return code of [TimerManager::handleMessage] indicates whether the message passed to it could be handled by a timer.
```{.cpp}
// Alternative implementation for co-existence with other self-messages
    void handleMessage(cMessage* msg) override {
        if (msg->isSelfMessage() && timerManager.handleMessage(msg)) return;
        // ...
    }
```

All cleanup and re-scheduling chores are done by the [TimerManager]
As the [TimerManager] instance can be value member (and not a pointer to a heap-allocated object like cMessage), there is no need for a custom destructor.

When using inheritance trees, the [TimerManager] can easily used by different levels (as long as it is a protected or public member or accessible otherwise).
Even multiple instances coule be used, as long as message delegation is correctly implemented.

[TimerManager]: @ref veins::TimerManager "TimerManager"
[TimerSpecification]: @ref veins::TimerSpecification "TimerSpecification"
[TimerManager::handleMessage]: @ref veins::TimerManager::handleMessage "TimerManager::handleMessage()"
