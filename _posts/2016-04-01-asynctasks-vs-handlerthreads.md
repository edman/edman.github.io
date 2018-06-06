---
layout: post
title: AsyncTask vs HandlerThread
comments: True
---

![Off of the main thread!](/assets/asynctasks-vs-handlerthreads-01.jpg)
<p class="caption"> Don't overburden the UI thread when other resources are
available </p>

It is well known among mobile developers that whenever you have to do a large
amount of work, such as a database query or an API access, you definitely want
to deal with that workload off of the UI thread. In Android that mostly means
an
[`AsyncTask`](http://developer.android.com/reference/android/os/AsyncTask.html)
will come into play.

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

<span class="more"/>

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




















