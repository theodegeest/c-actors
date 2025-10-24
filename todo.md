# C-actor TODO

- make a list sorting benchmark that spawn children actors in nodes until a cutoff.
- write a MPSC que that can be used for the mailboxes and maybe private thread work queue. (work stealing?)
Maybe for the thread it would be better to use an array that can grow because the thread needs to visit an actor and then put it back for future work.



## For when using speedup stack
- make sure that non-active actors are not checked by all the threads.
