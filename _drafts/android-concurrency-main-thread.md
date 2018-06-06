---
layout: post
title: 'Android Concurrency: The Main Thread'
comments: True
---

This is the first post of a series I am writing on concurrency in the Android
platform. We'll start from the basics by examining how multithreading is used
and implemented in the underlying system; then branch into what facilities are
offered by both Java and Android for helping us develop concurrent capabilities
in our own apps.

By the end of this series we will have a good understanding of concurrency in
Android, and be armed with the right knowledge for implementing multithreaded
applications when needed. My goal is to at least prepare you so you won't be
intimidated next time you come across things like _Handlers_, _Loopers_,
_Executors_, _ThreadPools_, and whatnot. Upon doing so we'll delve into the
Android platform source code to take a closer look at how these concepts
were built, and will sometimes write our own code.

<span class="more"/>

Let's start with this quote straight from the [Android guide on processes and
threads](http://developer.android.com/guide/components/processes-and-threads.html).

> When an application component starts and the application does not have any
> other components running, the Android system _starts a new Linux process_ for
> the application with a _single thread of execution_.

![Off of the main thread!](/assets/asynctasks-vs-handlerthreads-01.jpg)
<p class="caption"> Reading that was like reading genesis. Let there be light!
</p>

In the beginning your application is nothing but a process with a thread. A
process as in a [Linux
process](http://www.tldp.org/LDP/tlk/kernel/processes.html), and a thread as in
a [_pthread_](https://en.wikipedia.org/wiki/POSIX_Threads). I will assume a
certain familiarity of the reader with the topics of Linux threads and
processes, and move on to how these concepts are applied in Android.

Though the
only thing you really need to know to understand this text is that threads are
different 

## The Main Thread

So what is this one thread we have when our application starts? The _main
thread_, also commonly called the _UI thread_, is responsible for all interface
and interaction events happening in our application, including drawing the
views in your activity and dispatching touch events to the appropriate widgets.
Besides drawing and input events, application code is also executed on the main
thread by default.

We will get to the details of what the main thread really is in a future post
in this very same series (spoiler: it's a _thread with a_
[_Looper_](https://www.youtube.com/watch?v=adPLIAnx9og)). Meanwhile let's try
to simplify things and think of the main thread as being a thread with a list
of tasks. This thread then goes through the list handling tasks one by one, top
to bottom. In the process of solving a task the main thread might add new tasks
to the list. Tasks may also be added by other threads.

Building on this simplified concept there are _two inherent rules_ to be
followed when dealing with the main thread.

### Don't overburden the main thread

Have a look at the following imaginary scenario that has probably happened to
you:

- The main thread starts processing the next task _screen touched_.
- Screen touch generates a new task _dispatch touch event_, which is added to
  the main thread list.
- At some point the main thread will get to that task, figure out the touch
  event corresponds to a _button_ in your application, and execute the
_onClick_ callback associated with the button widget listener.

All this sounds fine. That is, until you realize that the _onClick_ method
makes a file upload request to a remote web server. Now what happens to the
tasks in our main thread list while this file is being read and uploaded to
your web server through a mobile network? _They wait_.

Remember the main thread tasks include drawing and input event handling. All
this will be waiting for your file access and network request. This means the
application will not redraw itself, nor will it respond to user interaction
while that operation is ongoing. From a user's perspective your application is
_ice-frozen_, when in fact it is just very busy sending data through the
network.

This explains why it is so well known among mobile developers that whenever you
have to do a large amount of work, such as a database query or an API access,
you definitely want to deal with that workload off of the main thread.

UI drawing is scheduled every 16ms, when you don't offload this large work from
the main thread and it ends up taking longer than those 16ms, your application
will start to lag. Furthermore, if you block the main thread list even longer
the Android system will punish you with the dreaded _application not
responding_ (ANR) dialog, giving your user the options of either terminating
your application right then and there, or waiting. I don't want to be there to
see what she's gonna choose.

The same is also true for adding _way too many tasks into the main thread
list_. Although not as bad as having long running executions, many small tasks
might still delay screen redraw or user input processing, and in turn make your
application look laggy.

### All UI manipulation must happen at the main thread

Alright, let's not execute long running operations in the main thread. Cool, I
can implement my own thread subclass, or spawn off a normal thread and pass it
a _Runnable_ encapsulating my long running background operation. Is that all
there is to know about the main thread? Not quite yet. And the
spawning-off-a-thread solution might not work as you wanted.

The android implementation of UI widgets is not thread-safe. That is, several
threads accessing the same widget in a concurrent environment might end up
leaving your views in invalid and unpredictable states. How does Android handle
this?  Simple. All UI updates _must be done in the main thread_ (which is why
many people call it the UI thread).

Since all work in the main thread is sequentially accomplished by going over
that list we talked about, we won't run into concurrency issues when
manipulating our widgets.

Now what if my long running background operation needs to update the user
interface when it's done? If you're managing your own background threads you
will have to push a new task into the main thread list (remember other threads
can give tasks to the main thread). But there probably are easier ways to do so
than implementing the thing yourself.

That said, let's implement the thing ourselves.

## AsyncTasks

The `AsyncTask` class provides an easy way to execute some work on a separate
thread *and* report back to the main thread once it's finished. Once background
computation results are reported, the user interface can be update. All this
comes pretty much for free, saving you from the hassle of having to deal with
threads, thread pools, executors, futures, and handlers. This allows you to
focus solely on your background computation and logic instead of spending time
on particularities of how it will be executed.

Although this sounds nice and will certainly do the trick 90% of the times,
there are a few drawbacks and caveats around `AsyncTask` that you better keep
in mind when developing Android apps.

`AsyncTask` is a *good and easy* way of managing background workloads so long as:

* ### The workload isn't *really long*
By default all `AsyncTasks` are **executed sequentially** on a separate thread.
The `AsyncTask` class implements a serial executor that polls tasks from a
queue sequentially. Although the specific thread that will in fact do the work
comes from a thread pool, the serial executor ensures tasks are executed
strictly in *FIFO* order.
* ### There aren't *many* jobs to be executed
Since `AsyncTasks` will be executed sequentially, some sort of queuing
mechanism is required to manage which tasks are waiting and what should be
executed next. Another default limitation of `AsyncTasks` is that this queue is
implemented through a *capacity-bounded* [`BlockingQueue`](). This means that
trying to spin off **too many `AsyncTasks` might eventually block the main
thread**. Which will happen if the queue is full and `AsyncTask.execute()` is
called to start a new task.
* ### Computation results are supposed to *affect* the user interface
This point is a direct implication of the previous two. The implicit back and
forward between main thread and background processing is the main appeal of
`AsyncTasks`. If your background operation won't later be used to update the
UI, you're probably overdoing it. What is worse is since execution is
sequential and the waiting queue is capacity-bounded, having this long running
background operation in the `AsyncTask` waiting queue might stall or delay
other `AsyncTasks` that will actually update the UI with useful information.
{{ begincomment }}
So if you have a long workload that is for any reason detached from the user
interface, make sure to execute it
{{ endcomment }}


$$
\frac{\delta^2 E_{x}}{\delta t^2} = \frac{\delta}{\delta t} \Big(\frac{\delta
Ex}{\delta t}\Big)= f^{\prime\prime}(z - ct)\Big(\frac{\delta(z-ct)}{\delta
t}\Big) = c^2*f^{\prime\prime}(z - ct)
$$


## HandlerThread




















