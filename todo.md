# C-actor TODO

- make sure to use atimic functions!!!!!!!
- make a benchmark where actors spawns other actors.
- add a tag when default is done.

## For when using speedup stack
- use a cond variable (semaphore does this better) to make sure that if a thread has not found work it just waits until work is added. (when sending the thread mst lock + signal that work has been added.)
- memory pool per message type so that you can use a static data size allocator that does not move data and keeps a freelist to reallocate.
- make sure that non-active actors are not checked by all the threads.
- look into this MPSC queue for mailboxes (https://github.com/grivet/mpsc-queue)
- use a MPSC queue that can be used for the mailboxes and maybe private thread work queue. (work stealing?)
Maybe for the thread it would be better to use an array that can grow because the thread needs to visit an actor and then put it back for future work.
